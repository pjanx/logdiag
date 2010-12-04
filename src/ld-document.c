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
 * @objects: All the objects in the document.
 * @selection: All currently selected objects.
 * @connections: Connections between objects.
 */
struct _LdDocumentPrivate
{
	GSList *objects;
	GSList *selection;
	GSList *connections;
};

G_DEFINE_TYPE (LdDocument, ld_document, G_TYPE_OBJECT);

static void
ld_document_finalize (GObject *gobject);


static void
ld_document_class_init (LdDocumentClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_document_finalize;

/**
 * LdDocument::changed:
 * @document: The document object.
 *
 * Contents of the document have changed.
 */
	klass->changed_signal = g_signal_new
		("changed", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
		0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	g_type_class_add_private (klass, sizeof (LdDocumentPrivate));
}

static void
ld_document_init (LdDocument *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_DOCUMENT, LdDocumentPrivate);
}

static void
ld_document_finalize (GObject *gobject)
{
	LdDocument *self;

	self = LD_DOCUMENT (gobject);
	ld_document_clear (self);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_document_parent_class)->finalize (gobject);
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

	g_slist_free (self->priv->connections);
	self->priv->connections = NULL;

	g_slist_foreach (self->priv->selection, (GFunc) g_object_unref, NULL);
	g_slist_free (self->priv->selection);
	self->priv->selection = NULL;

	g_slist_foreach (self->priv->objects, (GFunc) g_object_unref, NULL);
	g_slist_free (self->priv->objects);
	self->priv->objects = NULL;

	g_signal_emit (self,
		LD_DOCUMENT_GET_CLASS (self)->changed_signal, 0);
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
	const gchar *filename, GError *error)
{
	g_return_val_if_fail (LD_IS_DOCUMENT (self), FALSE);

	/* TODO */
	return FALSE;
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
	const gchar *filename, GError *error)
{
	g_return_val_if_fail (LD_IS_DOCUMENT (self), FALSE);

	/* TODO */
	return FALSE;
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
	}
	g_signal_emit (self,
		LD_DOCUMENT_GET_CLASS (self)->changed_signal, 0);
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
	}
	g_signal_emit (self,
		LD_DOCUMENT_GET_CLASS (self)->changed_signal, 0);
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
	}
	g_signal_emit (self,
		LD_DOCUMENT_GET_CLASS (self)->changed_signal, 0);
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
	}
	g_signal_emit (self,
		LD_DOCUMENT_GET_CLASS (self)->changed_signal, 0);
}
