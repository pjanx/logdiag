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

#include "ld-symbol.h"
#include "ld-symbol-category.h"
#include "ld-library.h"


/**
 * SECTION:ld-symbol-category
 * @short_description: A category of symbols.
 * @see_also: #LdSymbol, #LdLibrary
 *
 * #LdSymbolCategory represents a category of #LdSymbol objects.
 */

/*
 * LdSymbolCategoryPrivate:
 * @name: The name of this category.
 * @image_path: Path to the image for this category.
 * @children: Children of this category.
 */
struct _LdSymbolCategoryPrivate
{
	gchar *name;
	gchar *image_path;
	GSList *children;
};

G_DEFINE_TYPE (LdSymbolCategory, ld_symbol_category, G_TYPE_OBJECT);

static void
ld_symbol_category_finalize (GObject *gobject);


static void
ld_symbol_category_class_init (LdSymbolCategoryClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_symbol_category_finalize;

	g_type_class_add_private (klass, sizeof (LdSymbolCategoryPrivate));
}

static void
ld_symbol_category_init (LdSymbolCategory *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_SYMBOL_CATEGORY, LdSymbolCategoryPrivate);
}

static void
ld_symbol_category_finalize (GObject *gobject)
{
	LdSymbolCategory *self;

	self = LD_SYMBOL_CATEGORY (gobject);

	if (self->priv->name)
		g_free (self->priv->name);
	if (self->priv->image_path)
		g_free (self->priv->image_path);

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
ld_symbol_category_new (const gchar *name)
{
	LdSymbolCategory *cat;

	cat = g_object_new (LD_TYPE_SYMBOL_CATEGORY, NULL);
	cat->priv->name = g_strdup (name);

	return cat;
}

/**
 * ld_symbol_category_set_name:
 * @self: An #LdSymbolCategory object.
 * @name: The new name for this category.
 */
void
ld_symbol_category_set_name (LdSymbolCategory *self, const gchar *name)
{
	g_return_if_fail (LD_IS_SYMBOL_CATEGORY (self));
	g_return_if_fail (name != NULL);

	if (self->priv->name)
		g_free (self->priv->name);
	self->priv->name = g_strdup (name);
}

/**
 * ld_symbol_category_get_name:
 * @self: An #LdSymbolCategory object.
 *
 * Return the name of this category.
 */
const gchar *
ld_symbol_category_get_name (LdSymbolCategory *self)
{
	g_return_val_if_fail (LD_IS_SYMBOL_CATEGORY (self), NULL);
	return self->priv->name;
}

/**
 * ld_symbol_category_set_image_path:
 * @self: An #LdSymbolCategory object.
 * @image_path: The new path to the image for this category. May be NULL.
 */
void
ld_symbol_category_set_image_path (LdSymbolCategory *self,
	const gchar *image_path)
{
	g_return_if_fail (LD_IS_SYMBOL_CATEGORY (self));

	if (self->priv->image_path)
		g_free (self->priv->image_path);
	self->priv->image_path = g_strdup (image_path);
}

/**
 * ld_symbol_category_get_image_path:
 * @self: An #LdSymbolCategory object.
 *
 * Return the filesystem path to the image for this category. May be NULL.
 */
const gchar *
ld_symbol_category_get_image_path (LdSymbolCategory *self)
{
	g_return_val_if_fail (LD_IS_SYMBOL_CATEGORY (self), NULL);
	return self->priv->image_path;
}

