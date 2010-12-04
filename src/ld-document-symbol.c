/*
 * ld-document-symbol.c
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
#include "ld-document-symbol.h"


/**
 * SECTION:ld-document-symbol
 * @short_description: A symbol object.
 * @see_also: #LdDocumentObject
 *
 * #LdDocumentSymbol is an implementation of #LdDocumentObject.
 */

/*
 * LdDocumentSymbolPrivate:
 * @klass: The class of this symbol.
 */
struct _LdDocumentSymbolPrivate
{
	gchar *klass;
};

G_DEFINE_TYPE (LdDocumentSymbol, ld_document_symbol, LD_TYPE_DOCUMENT_OBJECT);

static void ld_document_symbol_finalize (GObject *gobject);


static void
ld_document_symbol_class_init (LdDocumentSymbolClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_document_symbol_finalize;

	/* TODO: A property for the class. */

	g_type_class_add_private (klass, sizeof (LdDocumentSymbolPrivate));
}

static void
ld_document_symbol_init (LdDocumentSymbol *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_DOCUMENT_SYMBOL, LdDocumentSymbolPrivate);
}

static void
ld_document_symbol_finalize (GObject *gobject)
{
	LdDocumentSymbol *self;

	self = LD_DOCUMENT_SYMBOL (gobject);

	if (self->priv->klass)
		g_free (self->priv->klass);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_document_symbol_parent_class)->finalize (gobject);
}


/**
 * ld_document_symbol_new:
 * @klass: The class of the symbol (symbol identifier).
 *
 * Return value: A new #LdDocumentSymbol object.
 */
LdDocumentSymbol *
ld_document_symbol_new (const gchar *klass)
{
	LdDocumentSymbol *self;

	self = g_object_new (LD_TYPE_DOCUMENT_SYMBOL, NULL);
	ld_document_symbol_set_class (self, klass);
	return self;
}


const gchar *
ld_document_symbol_get_class (LdDocumentSymbol *self)
{
	g_return_val_if_fail (LD_IS_DOCUMENT_SYMBOL (self), NULL);
	return self->priv->klass;
}

void
ld_document_symbol_set_class (LdDocumentSymbol *self, const gchar *klass)
{
	g_return_if_fail (LD_IS_DOCUMENT_SYMBOL (self));

	if (self->priv->klass)
		g_free (self->priv->klass);
	self->priv->klass = g_strdup (klass);
}
