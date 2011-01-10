/*
 * ld-diagram.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010 - 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "config.h"

#include "ld-diagram-object.h"
#include "ld-diagram.h"


/**
 * SECTION:ld-diagram
 * @short_description: A diagram object.
 * @see_also: #LdCanvas
 *
 * #LdDiagram is a model used for storing diagrams.
 */

/*
 * LdDiagramPrivate:
 * @modified: Whether the diagram has been modified.
 * @objects: All objects in the diagram.
 * @selection: All currently selected objects.
 * @connections: Connections between objects.
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

static gboolean write_signature (GOutputStream *stream, GError **error);

static void ld_diagram_real_changed (LdDiagram *self);
static void ld_diagram_clear_internal (LdDiagram *self);
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
 * @diagram: The diagram object.
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
 * @diagram: The diagram object.
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
	ld_diagram_clear_internal (self);

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
 * @self: An #LdDiagram object.
 *
 * Clear the whole diagram with it's objects and selection.
 */
void
ld_diagram_clear (LdDiagram *self)
{
	g_return_if_fail (LD_IS_DIAGRAM (self));

	ld_diagram_clear_internal (self);

	g_signal_emit (self,
		LD_DIAGRAM_GET_CLASS (self)->changed_signal, 0);
}

/*
 * ld_diagram_clear_internal:
 * @self: An #LdDiagram object.
 *
 * Do the same as ld_diagram_clear() does but don't emit signals.
 */
static void
ld_diagram_clear_internal (LdDiagram *self)
{
	ld_diagram_unselect_all (self);

	g_list_free (self->priv->connections);
	self->priv->connections = NULL;

	g_list_foreach (self->priv->objects, (GFunc) g_object_unref, NULL);
	g_list_free (self->priv->objects);
	self->priv->objects = NULL;
}

/**
 * ld_diagram_load_from_file:
 * @self: An #LdDiagram object.
 * @filename: A filename.
 * @error: Return location for a GError, or NULL.
 *
 * Load a file into the diagram.
 *
 * Return value: TRUE if the file could be loaded, FALSE otherwise.
 */
gboolean
ld_diagram_load_from_file (LdDiagram *self,
	const gchar *filename, GError **error)
{
	JsonParser *parser;
	GError *json_error;

	g_return_val_if_fail (LD_IS_DIAGRAM (self), FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);

	/* TODO: Implement loading for real. This is just a stub. */
	parser = json_parser_new ();

	json_error = NULL;
	json_parser_load_from_file (parser, filename, &json_error);
	if (json_error)
	{
		g_propagate_error (error, json_error);
		g_object_unref (parser);
		return FALSE;
	}

	ld_diagram_clear (self);
	g_object_unref (parser);
	return TRUE;
}

/**
 * ld_diagram_save_to_file:
 * @self: An #LdDiagram object.
 * @filename: A filename.
 * @error: Return location for a GError, or NULL.
 *
 * Save the diagram into a file.
 *
 * Return value: TRUE if the diagram could be saved, FALSE otherwise.
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

	/* TODO: Implement saving for real. This is just a stub. */
	generator = json_generator_new ();
	g_object_set (generator, "pretty", TRUE, NULL);

	/* XXX: json-glib dislikes empty objects. */
	root = json_node_new (JSON_NODE_OBJECT);
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

/**
 * ld_diagram_get_modified:
 * @self: An #LdDiagram object.
 *
 * Return value: The modification status of diagram.
 */
gboolean
ld_diagram_get_modified (LdDiagram *self)
{
	g_return_val_if_fail (LD_IS_DIAGRAM (self), FALSE);
	return self->priv->modified;
}

/**
 * ld_diagram_set_modified:
 * @self: An #LdDiagram object.
 * @value: Whether the diagram has been modified.
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

/**
 * ld_diagram_get_objects:
 * @self: An #LdDiagram object.
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
 * @self: An #LdDiagram object.
 * @object: The object to be inserted.
 * @pos: The position at which the object is to be inserted.
 *       Negative values will append to the end.
 *
 * Insert an object into the diagram.
 */
void
ld_diagram_insert_object (LdDiagram *self, LdDiagramObject *object, gint pos)
{
	g_return_if_fail (LD_IS_DIAGRAM (self));
	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (object));

	if (!g_list_find (self->priv->objects, object))
	{
		self->priv->objects =
			g_list_insert (self->priv->objects, object, pos);
		g_object_ref (object);

		g_signal_emit (self,
			LD_DIAGRAM_GET_CLASS (self)->changed_signal, 0);
	}
}

/**
 * ld_diagram_remove_object:
 * @self: An #LdDiagram object.
 * @object: The object to be removed.
 *
 * Remove an object from the diagram.
 */
void
ld_diagram_remove_object (LdDiagram *self, LdDiagramObject *object)
{
	g_return_if_fail (LD_IS_DIAGRAM (self));
	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (object));

	if (g_list_find (self->priv->objects, object))
	{
		ld_diagram_selection_remove (self, object);

		self->priv->objects = g_list_remove (self->priv->objects, object);
		g_object_unref (object);

		g_signal_emit (self,
			LD_DIAGRAM_GET_CLASS (self)->changed_signal, 0);
	}
}

/**
 * ld_diagram_get_selection:
 * @self: An #LdDiagram object.
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
 * ld_diagram_selection_add:
 * @self: An #LdDiagram object.
 * @object: The object to be added to the selection.
 * @pos: The position at which the object is to be inserted.
 *       Negative values will append to the end.
 *
 * Add an object to selection.
 */
void
ld_diagram_selection_add (LdDiagram *self, LdDiagramObject *object, gint pos)
{
	g_return_if_fail (LD_IS_DIAGRAM (self));
	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (object));

	g_return_if_fail (g_list_find (self->priv->objects, object) != NULL);

	if (!g_list_find (self->priv->selection, object))
	{
		self->priv->selection =
			g_list_insert (self->priv->selection, object, pos);
		g_object_ref (object);

		g_signal_emit (self,
			LD_DIAGRAM_GET_CLASS (self)->selection_changed_signal, 0);
	}
}

/**
 * ld_diagram_selection_remove:
 * @self: An #LdDiagram object.
 * @object: The object to be removed from the selection.
 *
 * Remove an object from the selection.
 */
void
ld_diagram_selection_remove (LdDiagram *self, LdDiagramObject *object)
{
	g_return_if_fail (LD_IS_DIAGRAM (self));
	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (object));

	if (g_list_find (self->priv->selection, object))
	{
		self->priv->selection = g_list_remove (self->priv->selection, object);
		g_object_unref (object);

		g_signal_emit (self,
			LD_DIAGRAM_GET_CLASS (self)->selection_changed_signal, 0);
	}
}

/**
 * ld_diagram_select_all:
 * @self: An #LdDiagram object.
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
 * @self: An #LdDiagram object.
 *
 * Remove all objects from the current selection.
 */
void
ld_diagram_unselect_all (LdDiagram *self)
{
	g_return_if_fail (LD_IS_DIAGRAM (self));

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
