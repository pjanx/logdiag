/*
 * ld-diagram.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010 - 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-diagram
 * @short_description: A model for diagrams
 * @see_also: #LdCanvas
 *
 * #LdDiagram is a model used for storing diagrams.
 */

/*
 * LdDiagramPrivate:
 * @modified: whether the diagram has been modified.
 * @objects: all objects in the diagram.
 * @selection: all currently selected objects.
 * @connections: connections between objects.
 */
struct _LdDiagramPrivate
{
	gboolean modified;

	GList *objects;
	GList *selection;
	GList *connections;
};

enum
{
	PROP_0,
	PROP_MODIFIED
};

static void ld_diagram_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_diagram_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);
static void ld_diagram_dispose (GObject *gobject);
static void ld_diagram_finalize (GObject *gobject);

static void on_object_data_changed (LdDiagramObject *self,
	gchar **path, GValue *old_value, GValue *new_value, gpointer user_data);

static gboolean write_signature (GOutputStream *stream, GError **error);

static gboolean check_node (JsonNode *node, JsonNodeType type,
	const gchar *id, GError **error);
static gboolean deserialize_diagram (LdDiagram *self, JsonNode *root,
	GError **error);
static LdDiagramObject *deserialize_object (JsonObject *object_storage);

static JsonNode *serialize_diagram (LdDiagram *self);
static JsonNode *serialize_object (LdDiagramObject *object);
static const gchar *get_object_class_string (GType type);

static void install_object (LdDiagramObject *object, LdDiagram *self);
static void uninstall_object (LdDiagramObject *object, LdDiagram *self);
static void ld_diagram_real_changed (LdDiagram *self);
static void ld_diagram_unselect_all_internal (LdDiagram *self);


G_DEFINE_TYPE (LdDiagram, ld_diagram, G_TYPE_OBJECT);

static void
ld_diagram_class_init (LdDiagramClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_diagram_get_property;
	object_class->set_property = ld_diagram_set_property;
	object_class->dispose = ld_diagram_dispose;
	object_class->finalize = ld_diagram_finalize;

	klass->changed = ld_diagram_real_changed;

/**
 * LdDiagram:modified:
 *
 * Whether the diagram has been modified.
 */
	pspec = g_param_spec_boolean ("modified", "Modified",
		"Whether the diagram has been modified.",
		FALSE, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_MODIFIED, pspec);

/**
 * LdDiagram::changed:
 * @self: an #LdDiagram object.
 *
 * Contents of the diagram have changed.
 */
	klass->changed_signal = g_signal_new
		("changed", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (LdDiagramClass, changed), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

/**
 * LdDiagram::selection-changed:
 * @self: an #LdDiagram object.
 *
 * The current selection has changed.
 */
	klass->selection_changed_signal = g_signal_new
		("selection-changed", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (LdDiagramClass, selection_changed), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	g_type_class_add_private (klass, sizeof (LdDiagramPrivate));
}

static void
ld_diagram_init (LdDiagram *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_DIAGRAM, LdDiagramPrivate);
}

static void
ld_diagram_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdDiagram *self;

	self = LD_DIAGRAM (object);
	switch (property_id)
	{
	case PROP_MODIFIED:
		g_value_set_boolean (value, ld_diagram_get_modified (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_diagram_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdDiagram *self;

	self = LD_DIAGRAM (object);
	switch (property_id)
	{
	case PROP_MODIFIED:
		ld_diagram_set_modified (self, g_value_get_boolean (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_diagram_dispose (GObject *gobject)
{
	LdDiagram *self;

	self = LD_DIAGRAM (gobject);
	ld_diagram_clear (self);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_diagram_parent_class)->dispose (gobject);
}

static void
ld_diagram_finalize (GObject *gobject)
{
	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_diagram_parent_class)->finalize (gobject);
}

static void
ld_diagram_real_changed (LdDiagram *self)
{
	g_return_if_fail (LD_IS_DIAGRAM (self));

	ld_diagram_set_modified (self, TRUE);
}


/**
 * ld_diagram_error_quark:
 *
 * Registers an error quark for #LdDiagram if necessary.
 *
 * Return value: the error quark used for #LdDiagram errors.
 */
GQuark
ld_diagram_error_quark (void)
{
	return g_quark_from_static_string ("ld-diagram-error-quark");
}

/**
 * ld_diagram_new:
 *
 * Create an instance.
 */
LdDiagram *
ld_diagram_new (void)
{
	return g_object_new (LD_TYPE_DIAGRAM, NULL);
}

/**
 * ld_diagram_clear:
 * @self: an #LdDiagram object.
 *
 * Clear the whole diagram with it's objects and selection.
 */
void
ld_diagram_clear (LdDiagram *self)
{
	gboolean changed = FALSE;
	gboolean selection_changed = FALSE;

	g_return_if_fail (LD_IS_DIAGRAM (self));

	if (self->priv->selection)
	{
		ld_diagram_unselect_all_internal (self);
		selection_changed = TRUE;
	}

	if (self->priv->connections)
	{
		g_list_free (self->priv->connections);
		self->priv->connections = NULL;
		changed = TRUE;
	}
	if (self->priv->objects)
	{
		g_list_foreach (self->priv->objects, (GFunc) uninstall_object, self);
		g_list_free (self->priv->objects);
		self->priv->objects = NULL;
		changed = TRUE;
	}

	if (changed)
		g_signal_emit (self,
			LD_DIAGRAM_GET_CLASS (self)->changed_signal, 0);
	if (selection_changed)
		g_signal_emit (self,
			LD_DIAGRAM_GET_CLASS (self)->selection_changed_signal, 0);
}

/**
 * ld_diagram_load_from_file:
 * @self: an #LdDiagram object.
 * @filename: a filename.
 * @error: (allow-none): return location for a #GError, or %NULL.
 *
 * Load a file into the diagram.
 *
 * Return value: %TRUE if the file could be loaded, %FALSE otherwise.
 */
gboolean
ld_diagram_load_from_file (LdDiagram *self,
	const gchar *filename, GError **error)
{
	JsonParser *parser;
	GError *local_error;

	g_return_val_if_fail (LD_IS_DIAGRAM (self), FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);

	parser = json_parser_new ();

	local_error = NULL;
	json_parser_load_from_file (parser, filename, &local_error);
	if (local_error)
	{
		g_propagate_error (error, local_error);
		g_object_unref (parser);
		return FALSE;
	}

	ld_diagram_clear (self);

	local_error = NULL;
	deserialize_diagram (self, json_parser_get_root (parser), &local_error);
	g_object_unref (parser);
	if (local_error)
	{
		g_propagate_error (error, local_error);
		return FALSE;
	}
	return TRUE;
}

/**
 * ld_diagram_save_to_file:
 * @self: an #LdDiagram object.
 * @filename: a filename.
 * @error: (allow-none): return location for a #GError, or %NULL.
 *
 * Save the diagram into a file.
 *
 * Return value: %TRUE if the diagram could be saved, %FALSE otherwise.
 */
gboolean
ld_diagram_save_to_file (LdDiagram *self,
	const gchar *filename, GError **error)
{
	GFile *file;
	GFileOutputStream *file_stream;
	JsonGenerator *generator;
	JsonNode *root;
	GError *local_error;

	g_return_val_if_fail (LD_IS_DIAGRAM (self), FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);

	file = g_file_new_for_path (filename);

	local_error = NULL;
	file_stream = g_file_replace (file, NULL, FALSE,
		G_FILE_CREATE_NONE, NULL, &local_error);
	g_object_unref (file);

	if (local_error)
	{
		g_propagate_error (error, local_error);
		return FALSE;
	}

	local_error = NULL;
	write_signature (G_OUTPUT_STREAM (file_stream), &local_error);
	if (local_error)
	{
		g_object_unref (file_stream);
		g_propagate_error (error, local_error);
		return FALSE;
	}

	generator = json_generator_new ();
	g_object_set (generator, "pretty", TRUE, NULL);

	root = serialize_diagram (self);
	json_generator_set_root (generator, root);
	json_node_free (root);

	local_error = NULL;
	json_generator_to_stream (generator, G_OUTPUT_STREAM (file_stream),
		NULL, &local_error);
	g_object_unref (file_stream);
	g_object_unref (generator);

	if (local_error)
	{
		g_propagate_error (error, local_error);
		return FALSE;
	}
	return TRUE;
}

static gboolean
write_signature (GOutputStream *stream, GError **error)
{
	static const gchar signature[] = "/* logdiag diagram */\n";
	GError *local_error = NULL;

	g_output_stream_write (stream, signature, sizeof (signature) - 1,
		NULL, &local_error);
	if (local_error)
	{
		g_propagate_error (error, local_error);
		return FALSE;
	}
	return TRUE;
}

static gboolean
check_node (JsonNode *node, JsonNodeType type, const gchar *id, GError **error)
{
	if (!node)
	{
		g_set_error (error, LD_DIAGRAM_ERROR, LD_DIAGRAM_ERROR_DIAGRAM_CORRUPT,
			"%s is missing", id);
		return FALSE;
	}
	if (!JSON_NODE_HOLDS (node, type))
	{
		g_set_error (error, LD_DIAGRAM_ERROR, LD_DIAGRAM_ERROR_DIAGRAM_CORRUPT,
			"%s is of wrong type", id);
		return FALSE;
	}
	return TRUE;
}

static gboolean
deserialize_diagram (LdDiagram *self, JsonNode *root, GError **error)
{
	JsonObject *root_object;
	JsonNode *objects_node;
	GList *iter;

	if (!check_node (root, JSON_NODE_OBJECT, "the root node", error))
		return FALSE;

	root_object = json_node_get_object (root);
	objects_node = json_object_get_member (root_object, "objects");
	if (!check_node (objects_node, JSON_NODE_ARRAY,
		"the `objects' array", error))
		return FALSE;

	iter = json_array_get_elements (json_node_get_array (objects_node));
	for (; iter; iter = g_list_next (iter))
	{
		GError *node_error = NULL;

		check_node (iter->data, JSON_NODE_OBJECT, "object node", &node_error);
		if (node_error)
		{
			g_warning ("%s", node_error->message);
			g_error_free (node_error);
		}
		else
			/* FIXME: Appending is slow. */
			ld_diagram_insert_object (self,
				deserialize_object (json_node_get_object (iter->data)), -1);
	}
	return TRUE;
}

static LdDiagramObject *
deserialize_object (JsonObject *object_storage)
{
	JsonNode *object_type_node;
	const gchar *type;

	json_object_ref (object_storage);
	object_type_node = json_object_get_member (object_storage, "type");

	if (!object_type_node || !JSON_NODE_HOLDS_VALUE (object_type_node))
		goto deserialize_object_default;

	type = json_node_get_string (object_type_node);
	if (!g_strcmp0 ("symbol", type))
		return LD_DIAGRAM_OBJECT (ld_diagram_symbol_new (object_storage));

deserialize_object_default:
	/* Anything we can't identify is just an indefinite object. */
	return ld_diagram_object_new (object_storage);
}

static JsonNode *
serialize_diagram (LdDiagram *self)
{
	JsonNode *root_node;
	JsonObject *root_object;
	JsonArray *objects_array;
	GList *iter;

	root_node = json_node_new (JSON_NODE_OBJECT);
	root_object = json_object_new ();
	json_node_take_object (root_node, root_object);

	objects_array = json_array_new ();
	for (iter = self->priv->objects; iter; iter = g_list_next (iter))
		json_array_add_element (objects_array,
			serialize_object (LD_DIAGRAM_OBJECT (iter->data)));

	json_object_set_int_member (root_object, "version", 1);
	json_object_set_array_member (root_object, "objects", objects_array);
	return root_node;
}

static JsonNode *
serialize_object (LdDiagramObject *object)
{
	JsonNode *object_node, *object_type_node;
	JsonObject *object_storage;

	object_node = json_node_new (JSON_NODE_OBJECT);
	object_storage = ld_diagram_object_get_storage (object);

	object_type_node = json_object_get_member (object_storage, "type");
	if (!object_type_node || !JSON_NODE_HOLDS_VALUE (object_type_node))
		json_object_set_string_member (object_storage,
			"type", get_object_class_string (G_OBJECT_TYPE (object)));

	json_node_set_object (object_node, object_storage);
	return object_node;
}

static const gchar *
get_object_class_string (GType type)
{
	if (type == LD_TYPE_DIAGRAM_SYMBOL)
		return "symbol";
	if (type != LD_TYPE_DIAGRAM_OBJECT)
		/* We don't know our own type, that's just plain wrong. */
		g_warn_if_reached ();
	return "object";
}

/**
 * ld_diagram_get_modified:
 * @self: an #LdDiagram object.
 *
 * Return value: the modification status of diagram.
 */
gboolean
ld_diagram_get_modified (LdDiagram *self)
{
	g_return_val_if_fail (LD_IS_DIAGRAM (self), FALSE);
	return self->priv->modified;
}

/**
 * ld_diagram_set_modified:
 * @self: an #LdDiagram object.
 * @value: whether the diagram has been modified.
 *
 * Set the modification status of diagram.
 */
void
ld_diagram_set_modified (LdDiagram *self, gboolean value)
{
	g_return_if_fail (LD_IS_DIAGRAM (self));
	self->priv->modified = value;

	g_object_notify (G_OBJECT (self), "modified");
}

static void
on_object_data_changed (LdDiagramObject *self, gchar **path,
	GValue *old_value, GValue *new_value, gpointer user_data)
{
}

static void
install_object (LdDiagramObject *object, LdDiagram *self)
{
	g_signal_connect (object, "data-changed",
		G_CALLBACK (on_object_data_changed), self);
	g_object_ref (object);
}

static void
uninstall_object (LdDiagramObject *object, LdDiagram *self)
{
	g_signal_handlers_disconnect_by_func (object, on_object_data_changed, self);
	g_object_unref (object);
}

/**
 * ld_diagram_get_objects:
 * @self: an #LdDiagram object.
 *
 * Get a list of objects in the diagram. Do not modify.
 */
GList *
ld_diagram_get_objects (LdDiagram *self)
{
	g_return_val_if_fail (LD_IS_DIAGRAM (self), NULL);
	return self->priv->objects;
}

/**
 * ld_diagram_insert_object:
 * @self: an #LdDiagram object.
 * @object: the object to be inserted.
 * @pos: the position at which the object is to be inserted.
 *       Negative values will append to the end.
 *
 * Insert an object into the diagram.
 */
void
ld_diagram_insert_object (LdDiagram *self, LdDiagramObject *object, gint pos)
{
	g_return_if_fail (LD_IS_DIAGRAM (self));
	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (object));

	if (g_list_find (self->priv->objects, object))
		return;

	self->priv->objects = g_list_insert (self->priv->objects, object, pos);
	install_object (object, self);

	g_signal_emit (self,
		LD_DIAGRAM_GET_CLASS (self)->changed_signal, 0);
}

/**
 * ld_diagram_remove_object:
 * @self: an #LdDiagram object.
 * @object: the object to be removed.
 *
 * Remove an object from the diagram.
 */
void
ld_diagram_remove_object (LdDiagram *self, LdDiagramObject *object)
{
	g_return_if_fail (LD_IS_DIAGRAM (self));
	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (object));

	if (!g_list_find (self->priv->objects, object))
		return;

	ld_diagram_unselect (self, object);

	self->priv->objects = g_list_remove (self->priv->objects, object);
	uninstall_object (object, self);

	g_signal_emit (self,
		LD_DIAGRAM_GET_CLASS (self)->changed_signal, 0);
}

/**
 * ld_diagram_get_selection:
 * @self: an #LdDiagram object.
 *
 * Get a list of objects that are currently selected in the diagram.
 * Do not modify.
 */
GList *
ld_diagram_get_selection (LdDiagram *self)
{
	g_return_val_if_fail (LD_IS_DIAGRAM (self), NULL);
	return self->priv->selection;
}

/**
 * ld_diagram_remove_selection:
 * @self: an #LdDiagram object.
 *
 * Remove selected objects from the diagram.
 */
void
ld_diagram_remove_selection (LdDiagram *self)
{
	LdDiagramObject *object;
	gboolean changed;
	GList *iter;

	g_return_if_fail (LD_IS_DIAGRAM (self));

	for (iter = self->priv->selection; iter; iter = g_list_next (iter))
	{
		object = LD_DIAGRAM_OBJECT (iter->data);
		g_object_unref (object);

		self->priv->objects = g_list_remove (self->priv->objects, object);
		uninstall_object (object, self);
	}

	changed = self->priv->selection != NULL;
	g_list_free (self->priv->selection);
	self->priv->selection = NULL;

	if (changed)
	{
		g_signal_emit (self,
			LD_DIAGRAM_GET_CLASS (self)->changed_signal, 0);
		g_signal_emit (self,
			LD_DIAGRAM_GET_CLASS (self)->selection_changed_signal, 0);
	}
}

/**
 * ld_diagram_select:
 * @self: an #LdDiagram object.
 * @object: the object to be added to the selection.
 *
 * Add an object to selection.
 */
void
ld_diagram_select (LdDiagram *self, LdDiagramObject *object)
{
	g_return_if_fail (LD_IS_DIAGRAM (self));
	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (object));
	g_return_if_fail (g_list_find (self->priv->objects, object) != NULL);

	if (g_list_find (self->priv->selection, object))
		return;

	self->priv->selection = g_list_insert (self->priv->selection, object, 0);
	g_object_ref (object);

	g_signal_emit (self,
		LD_DIAGRAM_GET_CLASS (self)->selection_changed_signal, 0);
}

/**
 * ld_diagram_unselect:
 * @self: an #LdDiagram object.
 * @object: the object to be removed from the selection.
 *
 * Remove an object from the selection.
 */
void
ld_diagram_unselect (LdDiagram *self, LdDiagramObject *object)
{
	g_return_if_fail (LD_IS_DIAGRAM (self));
	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (object));

	if (!g_list_find (self->priv->selection, object))
		return;

	self->priv->selection = g_list_remove (self->priv->selection, object);
	g_object_unref (object);

	g_signal_emit (self,
		LD_DIAGRAM_GET_CLASS (self)->selection_changed_signal, 0);
}

/**
 * ld_diagram_select_all:
 * @self: an #LdDiagram object.
 *
 * Include all objects in the document to the selection.
 */
void
ld_diagram_select_all (LdDiagram *self)
{
	g_return_if_fail (LD_IS_DIAGRAM (self));

	ld_diagram_unselect_all_internal (self);
	self->priv->selection = g_list_copy (self->priv->objects);
	g_list_foreach (self->priv->selection, (GFunc) g_object_ref, NULL);

	g_signal_emit (self,
		LD_DIAGRAM_GET_CLASS (self)->selection_changed_signal, 0);
}

/**
 * ld_diagram_unselect_all:
 * @self: an #LdDiagram object.
 *
 * Remove all objects from the current selection.
 */
void
ld_diagram_unselect_all (LdDiagram *self)
{
	g_return_if_fail (LD_IS_DIAGRAM (self));

	if (!self->priv->selection)
		return;

	ld_diagram_unselect_all_internal (self);

	g_signal_emit (self,
		LD_DIAGRAM_GET_CLASS (self)->selection_changed_signal, 0);
}

static void
ld_diagram_unselect_all_internal (LdDiagram *self)
{
	g_list_foreach (self->priv->selection, (GFunc) g_object_unref, NULL);
	g_list_free (self->priv->selection);
	self->priv->selection = NULL;
}
