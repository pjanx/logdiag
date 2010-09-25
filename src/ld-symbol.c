/*
 * ld-symbol.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>

#include "config.h"

#include "ld-library.h"
#include "ld-symbol-category.h"
#include "ld-symbol.h"


/**
 * SECTION:ld-symbol
 * @short_description: A symbol.
 * @see_also: #LdDocument, #LdCanvas
 *
 * #LdSymbol represents a symbol in the #LdDocument that is in turn
 * drawn onto the #LdCanvas.
 */

/*
 * LdSymbolPrivate:
 * @parent: The parent LdSymbolCategory. It is used to identify
 * the object within it's library.
 */
struct _LdSymbolPrivate
{
	LdSymbolCategory *parent;
};

G_DEFINE_ABSTRACT_TYPE (LdSymbol, ld_symbol, G_TYPE_OBJECT);

static void ld_symbol_finalize (GObject *gobject);


static void
ld_symbol_class_init (LdSymbolClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_symbol_finalize;

	g_type_class_add_private (klass, sizeof (LdSymbolPrivate));
}

static void
ld_symbol_init (LdSymbol *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_SYMBOL, LdSymbolPrivate);
}

static void
ld_symbol_finalize (GObject *gobject)
{
	LdSymbol *self;

	self = LD_SYMBOL (gobject);
	g_object_unref (self->priv->parent);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_symbol_parent_class)->finalize (gobject);
}

/**
 * ld_symbol_build_identifier:
 * @self: A symbol object.
 *
 * Build an identifier for the symbol.
 * The identifier is in the format "Category/Category/Symbol".
 */
gchar *
ld_symbol_build_identifier (LdSymbol *self)
{
	/* TODO: Implement. */
	return NULL;
}

/**
 * ld_symbol_draw:
 * @self: A symbol object.
 * @cr: A cairo surface to be drawn on.
 *
 * Draw the symbol onto a Cairo surface.
 */
void
ld_symbol_draw (LdSymbol *self, cairo_t *cr)
{
	LdSymbolClass *klass;

	g_return_if_fail (LD_IS_SYMBOL (self));
	g_return_if_fail (cr != NULL);

	klass = LD_SYMBOL_GET_CLASS (self);
	if (klass->draw)
		klass->draw (self, cr);
}
