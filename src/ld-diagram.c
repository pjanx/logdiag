/*
 * ld-diagram.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
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
 * #LdDiagram is a model for storing diagrams.
 */

/*
 * LdDiagramPrivate:
 * @modified: Whether the diagram has been modified.
 * @objects: All the objects in the diagram.
 * @selection: All currently selected objects.
 * @connections: Connections between objects.
 */
struct _LdDiagramPrivate
{
	gboolean modified;

	GSList *objects;
	GSList *selection;
	GSList *connections;
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

static void ld_diagram_real_changed (LdDiagram *self);
static void ld_diagram_clear_internal (LdDiagram *self);


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
 * Do the same what ld_diagram_clear() does but don't emit signals.
 */
static void
ld_diagram_clear_internal (LdDiagram *self)
{
	g_slist_free (self->priv->connections);
	self->priv->connections = NULL;

	g_slist_foreach (self->priv->selection, (GFunc) g_object_unref, NULL);
	g_slist_free (self->priv->selection);
	self->priv->selection = NULL;

	g_slist_foreach (self->priv->objects, (GFunc) g_object_unref, NULL);
	g_slist_free (self->priv->objects);
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
	JsonGenerator *generator;
	JsonNode *root;
	GError *json_error;

	g_return_val_if_fail (LD_IS_DIAGRAM (self), FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);

	/* TODO: Implement saving for real. This is just a stub. */
	generator = json_generator_new ();
	g_object_set (generator, "pretty", TRUE, NULL);

	/* XXX: json-glib dislikes empty objects. */
	root = json_node_new (JSON_NODE_OBJECT);
	json_generator_set_root (generator, root);
	json_node_free (root);

	json_error = NULL;
	json_generator_to_file (generator, filename, &json_error);
	if (json_error)
	{
		g_propagate_error (error, json_error);
		g_object_unref (generator);
		return FALSE;
	}
	g_object_unref (generator);
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
 * Get a list of objects in the diagram.
 * You mustn't make any changes to the list.
 */
GSList *
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

	if (!g_slist_find (self->priv->objects, object))
	{
		self->priv->objects =
			g_slist_insert (self->priv->objects, object, pos);
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

	if (g_slist_find (self->priv->objects, object))
	{
		ld_diagram_selection_remove (self, object);

		self->priv->objects = g_slist_remove (self->priv->objects, object);
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
 * You mustn't make any changes to the list.
 */
GSList *
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

	if (!g_slist_find (self->priv->selection, object)
		&& g_slist_find (self->priv->objects, object))
	{
		self->priv->selection =
			g_slist_insert (self->priv->selection, object, pos);
		g_object_ref (object);

		g_signal_emit (self,
			LD_DIAGRAM_GET_CLASS (self)->changed_signal, 0);
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

	if (g_slist_find (self->priv->selection, object))
	{
		self->priv->selection = g_slist_remove (self->priv->selection, object);
		g_object_unref (object);

		g_signal_emit (self,
			LD_DIAGRAM_GET_CLASS (self)->changed_signal, 0);
	}
}
