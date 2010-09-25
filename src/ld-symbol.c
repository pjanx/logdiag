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
 * @library: The parent LdLibrary.
 * The library contains the real function for rendering.
 */
struct _LdSymbolPrivate
{
	LdLibrary *library;
};

G_DEFINE_TYPE (LdSymbol, ld_symbol, G_TYPE_OBJECT);

static void
ld_symbol_finalize (GObject *gobject);


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
		(self, LD_TYPE_SYMBOL_LIBRARY, LdSymbolPrivate);
}

static void
ld_symbol_finalize (GObject *gobject)
{
	LdSymbol *self;

	self = LD_SYMBOL (gobject);
	g_object_unref (self->priv->library);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_symbol_parent_class)->finalize (gobject);
}

/**
 * ld_symbol_new:
 * @library: A library object.
 * @filename: The file from which the symbol will be loaded.
 *
 * Load a symbol from a file into the library.
 */
LdSymbol *ld_symbol_new (LdLibrary *library)
{
	LdSymbol *symbol;

	symbol = g_object_new (LD_TYPE_SYMBOL, NULL);

	symbol->priv->library = library;
	g_object_ref (library);
}

/**
 * ld_symbol_build_identifier:
 * @self: A symbol object.
 *
 * Build an identifier for the symbol.
 * The identifier is in the format "Category/Category/Symbol".
 */
char *
ld_symbol_build_identifier (LdSymbol *self)
{
	return NULL;
}

/**
 * ld_symbol_draw:
 * @self: A symbol object.
 * @cr: A cairo surface to be drawn on.
 * @param: Parameters for the symbol in a table.
 *
 * Draw the symbol onto a Cairo surface.
 */
void
ld_symbol_draw (LdSymbol *self, cairo_t *cr, GHashTable *param)
{
	return;
}
