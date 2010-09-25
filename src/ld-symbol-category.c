/*
 * ld-symbol-category.c
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
 * SECTION:ld-symbol-category
 * @short_description: A category of symbols.
 * @see_also: #LdSymbol, #LdLibrary
 *
 * #LdSymbolCategory represents a category of #LdSymbol objects.
 */

G_DEFINE_TYPE (LdSymbolCategory, ld_symbol_category, G_TYPE_OBJECT);

static void
ld_symbol_category_finalize (GObject *gobject);


static void
ld_symbol_category_class_init (LdSymbolCategoryClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_symbol_category_finalize;
}

static void
ld_symbol_category_init (LdSymbolCategory *self)
{
	/* TODO: use _new_full, correct equal and specify destroy functions. */
	/* XXX: How's the situation with subcategory names and symbol names
	 *      within the same hashtable?
	 */
	self->children = g_hash_table_new (g_str_hash, g_str_equal);
}

static void
ld_symbol_category_finalize (GObject *gobject)
{
	LdSymbolCategory *self;

	self = LD_SYMBOL_CATEGORY (gobject);

	if (self->name)
		g_free (self->name);
	if (self->image_path)
		g_free (self->image_path);

	g_object_unref (self->parent);
	g_hash_table_destroy (self->children);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_symbol_category_parent_class)->finalize (gobject);
}

/**
 * ld_symbol_category_new:
 * @parent: The parent library for this category.
 *
 * Create an instance.
 */
LdSymbolCategory *
ld_symbol_category_new (LdLibrary *parent)
{
	LdSymbolCategory *cat;

	cat = g_object_new (LD_TYPE_SYMBOL_CATEGORY, NULL);

	cat->parent = parent;
	g_object_ref (parent);

	return cat;
}

