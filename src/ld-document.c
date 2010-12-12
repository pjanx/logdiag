/*
 * ld-document.c
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

#include "ld-document-object.h"
#include "ld-document.h"


/**
 * SECTION:ld-document
 * @short_description: A document object.
 * @see_also: #LdCanvas
 *
 * #LdDocument is a model for storing documents.
 */

/*
 * LdDocumentPrivate:
 * @modified: Whether the document has been modified.
 * @objects: All the objects in the document.
 * @selection: All currently selected objects.
 * @connections: Connections between objects.
 */
struct _LdDocumentPrivate
{
	gboolean modified;

	GSList *objects;
	GSList *selection;
	GSList *connections;
};

G_DEFINE_TYPE (LdDocument, ld_document, G_TYPE_OBJECT);

enum
{
	PROP_0,
	PROP_MODIFIED
};

static void ld_document_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_document_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);
static void ld_document_dispose (GObject *gobject);
static void ld_document_finalize (GObject *gobject);

static void ld_document_real_changed (LdDocument *self);
static void ld_document_clear_internal (LdDocument *self);


static void
ld_document_class_init (LdDocumentClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_document_get_property;
	object_class->set_property = ld_document_set_property;
	object_class->dispose = ld_document_dispose;
	object_class->finalize = ld_document_finalize;

	klass->changed = ld_document_real_changed;

/**
 * LdDocument:modified:
 *
 * Whether the document has been modified.
 */
	pspec = g_param_spec_boolean ("modified", "Modified",
		"Whether the document has been modified.",
		FALSE, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_MODIFIED, pspec);

/**
 * LdDocument::changed:
 * @document: The document object.
 *
 * Contents of the document have changed.
 */
	klass->changed_signal = g_signal_new
		("changed", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (LdDocumentClass, changed), NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	g_type_class_add_private (klass, sizeof (LdDocumentPrivate));
}

static void
ld_document_init (LdDocument *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_DOCUMENT, LdDocumentPrivate);
}

static void
ld_document_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdDocument *self;

	self = LD_DOCUMENT (object);
	switch (property_id)
	{
	case PROP_MODIFIED:
		g_value_set_boolean (value, ld_document_get_modified (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_document_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdDocument *self;

	self = LD_DOCUMENT (object);
	switch (property_id)
	{
	case PROP_MODIFIED:
		ld_document_set_modified (self, g_value_get_boolean (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_document_dispose (GObject *gobject)
{
	LdDocument *self;

	self = LD_DOCUMENT (gobject);
	ld_document_clear_internal (self);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_document_parent_class)->dispose (gobject);
}

static void
ld_document_finalize (GObject *gobject)
{
	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_document_parent_class)->finalize (gobject);
}

static void
ld_document_real_changed (LdDocument *self)
{
	g_return_if_fail (LD_IS_DOCUMENT (self));

	ld_document_set_modified (self, TRUE);
}


/**
 * ld_document_new:
 *
 * Create an instance.
 */
LdDocument *
ld_document_new (void)
{
	return g_object_new (LD_TYPE_DOCUMENT, NULL);
}

/**
 * ld_document_clear:
 * @self: An #LdDocument object.
 *
 * Clear the whole document with it's objects and selection.
 */
void
ld_document_clear (LdDocument *self)
{
	g_return_if_fail (LD_IS_DOCUMENT (self));

	ld_document_clear_internal (self);

	g_signal_emit (self,
		LD_DOCUMENT_GET_CLASS (self)->changed_signal, 0);
}

/*
 * ld_document_clear_internal:
 * @self: An #LdDocument object.
 *
 * Do the same what ld_document_clear() does but don't emit signals.
 */
static void
ld_document_clear_internal (LdDocument *self)
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
 * ld_document_load_from_file:
 * @self: An #LdDocument object.
 * @filename: A filename.
 * @error: Return location for a GError, or NULL.
 *
 * Load a file into the document.
 *
 * Return value: TRUE if the file could be loaded, FALSE otherwise.
 */
gboolean
ld_document_load_from_file (LdDocument *self,
	const gchar *filename, GError **error)
{
	JsonParser *parser;
	GError *json_error;

	g_return_val_if_fail (LD_IS_DOCUMENT (self), FALSE);
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

	ld_document_clear (self);
	g_object_unref (parser);
	return TRUE;
}

/**
 * ld_document_save_to_file:
 * @self: An #LdDocument object.
 * @filename: A filename.
 * @error: Return location for a GError, or NULL.
 *
 * Save the document into a file.
 *
 * Return value: TRUE if the document could be saved, FALSE otherwise.
 */
gboolean
ld_document_save_to_file (LdDocument *self,
	const gchar *filename, GError **error)
{
	JsonGenerator *generator;
	JsonNode *root;
	GError *json_error;

	g_return_val_if_fail (LD_IS_DOCUMENT (self), FALSE);
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
 * ld_document_get_modified:
 * @self: An #LdDocument object.
 *
 * Return value: The modification status of document.
 */
gboolean
ld_document_get_modified (LdDocument *self)
{
	g_return_val_if_fail (LD_IS_DOCUMENT (self), FALSE);
	return self->priv->modified;
}

/**
 * ld_document_set_modified:
 * @self: An #LdDocument object.
 * @value: Whether the document has been modified.
 *
 * Set the modification status of document.
 */
void
ld_document_set_modified (LdDocument *self, gboolean value)
{
	g_return_if_fail (LD_IS_DOCUMENT (self));
	self->priv->modified = value;

	g_object_notify (G_OBJECT (self), "modified");
}

/**
 * ld_document_get_objects:
 * @self: An #LdDocument object.
 *
 * Get a list of objects in the document.
 * You mustn't make any changes to the list.
 */
GSList *
ld_document_get_objects (LdDocument *self)
{
	g_return_val_if_fail (LD_IS_DOCUMENT (self), NULL);
	return self->priv->objects;
}

/**
 * ld_document_insert_object:
 * @self: An #LdDocument object.
 * @object: The object to be inserted.
 * @pos: The position at which the object is to be inserted.
 *       Negative values will append to the end.
 *
 * Insert an object into the document.
 */
void
ld_document_insert_object (LdDocument *self, LdDocumentObject *object, gint pos)
{
	g_return_if_fail (LD_IS_DOCUMENT (self));
	g_return_if_fail (LD_IS_DOCUMENT_OBJECT (object));

	if (!g_slist_find (self->priv->objects, object))
	{
		self->priv->objects =
			g_slist_insert (self->priv->objects, object, pos);
		g_object_ref (object);

		g_signal_emit (self,
			LD_DOCUMENT_GET_CLASS (self)->changed_signal, 0);
	}
}

/**
 * ld_document_remove_object:
 * @self: An #LdDocument object.
 * @object: The object to be removed.
 *
 * Remove an object from the document.
 */
void
ld_document_remove_object (LdDocument *self, LdDocumentObject *object)
{
	g_return_if_fail (LD_IS_DOCUMENT (self));
	g_return_if_fail (LD_IS_DOCUMENT_OBJECT (object));

	if (g_slist_find (self->priv->objects, object))
	{
		ld_document_selection_remove (self, object);

		self->priv->objects = g_slist_remove (self->priv->objects, object);
		g_object_unref (object);

		g_signal_emit (self,
			LD_DOCUMENT_GET_CLASS (self)->changed_signal, 0);
	}
}

/**
 * ld_document_get_selection:
 * @self: An #LdDocument object.
 *
 * Get a list of objects that are currently selected in the document.
 * You mustn't make any changes to the list.
 */
GSList *
ld_document_get_selection (LdDocument *self)
{
	g_return_val_if_fail (LD_IS_DOCUMENT (self), NULL);
	return self->priv->selection;
}

/**
 * ld_document_selection_add:
 * @self: An #LdDocument object.
 * @object: The object to be added to the selection.
 * @pos: The position at which the object is to be inserted.
 *       Negative values will append to the end.
 *
 * Add an object to selection.
 */
void
ld_document_selection_add (LdDocument *self, LdDocumentObject *object, gint pos)
{
	g_return_if_fail (LD_IS_DOCUMENT (self));
	g_return_if_fail (LD_IS_DOCUMENT_OBJECT (object));

	if (!g_slist_find (self->priv->selection, object)
		&& g_slist_find (self->priv->objects, object))
	{
		self->priv->selection =
			g_slist_insert (self->priv->selection, object, pos);
		g_object_ref (object);

		g_signal_emit (self,
			LD_DOCUMENT_GET_CLASS (self)->changed_signal, 0);
	}
}

/**
 * ld_document_selection_remove:
 * @self: An #LdDocument object.
 * @object: The object to be removed from the selection.
 *
 * Remove an object from the selection.
 */
void
ld_document_selection_remove (LdDocument *self, LdDocumentObject *object)
{
	g_return_if_fail (LD_IS_DOCUMENT (self));
	g_return_if_fail (LD_IS_DOCUMENT_OBJECT (object));

	if (g_slist_find (self->priv->selection, object))
	{
		self->priv->selection = g_slist_remove (self->priv->selection, object);
		g_object_unref (object);

		g_signal_emit (self,
			LD_DOCUMENT_GET_CLASS (self)->changed_signal, 0);
	}
}
