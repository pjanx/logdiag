/*
 * ld-symbol-category.c
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
 * SECTION:ld-symbol-category
 * @short_description: A category of symbols
 * @see_also: #LdSymbol, #LdLibrary
 *
 * #LdSymbolCategory represents a category of #LdSymbol objects.
 */

/*
 * LdSymbolCategoryPrivate:
 * @name: the name of this category.
 * @image_path: path to the image for this category.
 * @children: children of this category.
 */
struct _LdSymbolCategoryPrivate
{
	gchar *name;
	gchar *human_name;
	gchar *image_path;
	GSList *children;
};

enum
{
	PROP_0,
	PROP_NAME,
	PROP_HUMAN_NAME,
	PROP_IMAGE_PATH
};

static void ld_symbol_category_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_symbol_category_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);
static void ld_symbol_category_finalize (GObject *gobject);


G_DEFINE_TYPE (LdSymbolCategory, ld_symbol_category, G_TYPE_OBJECT);

static void
ld_symbol_category_class_init (LdSymbolCategoryClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_symbol_category_get_property;
	object_class->set_property = ld_symbol_category_set_property;
	object_class->finalize = ld_symbol_category_finalize;

/**
 * LdSymbolCategory:name:
 *
 * The name of this symbol category.
 */
	pspec = g_param_spec_string ("name", "Name",
		"The name of this symbol category.",
		"", G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_NAME, pspec);

/**
 * LdSymbolCategory:human-name:
 *
 * The localized human name of this symbol category.
 */
	pspec = g_param_spec_string ("human-name", "Human name",
		"The localized human name of this symbol category.",
		"", G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_HUMAN_NAME, pspec);

/**
 * LdSymbolCategory:image-path:
 *
 * Path to an image file representing this category.
 */
	pspec = g_param_spec_string ("image-path", "Image path",
		"Path to an image file representing this category.",
		"", G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_IMAGE_PATH, pspec);

	g_type_class_add_private (klass, sizeof (LdSymbolCategoryPrivate));
}

static void
ld_symbol_category_init (LdSymbolCategory *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_SYMBOL_CATEGORY, LdSymbolCategoryPrivate);
}

static void
ld_symbol_category_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdSymbolCategory *self;

	self = LD_SYMBOL_CATEGORY (object);
	switch (property_id)
	{
	case PROP_NAME:
		g_value_set_string (value, ld_symbol_category_get_name (self));
		break;
	case PROP_HUMAN_NAME:
		g_value_set_string (value, ld_symbol_category_get_human_name (self));
		break;
	case PROP_IMAGE_PATH:
		g_value_set_string (value, ld_symbol_category_get_image_path (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_symbol_category_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdSymbolCategory *self;

	self = LD_SYMBOL_CATEGORY (object);
	switch (property_id)
	{
	case PROP_NAME:
		ld_symbol_category_set_name (self, g_value_get_string (value));
		break;
	case PROP_HUMAN_NAME:
		ld_symbol_category_set_human_name (self, g_value_get_string (value));
		break;
	case PROP_IMAGE_PATH:
		ld_symbol_category_set_image_path (self, g_value_get_string (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_symbol_category_finalize (GObject *gobject)
{
	LdSymbolCategory *self;

	self = LD_SYMBOL_CATEGORY (gobject);

	if (self->priv->name)
		g_free (self->priv->name);
	if (self->priv->human_name)
		g_free (self->priv->human_name);
	if (self->priv->image_path)
		g_free (self->priv->image_path);

	g_slist_foreach (self->priv->children, (GFunc) g_object_unref, NULL);
	g_slist_free (self->priv->children);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_symbol_category_parent_class)->finalize (gobject);
}


/**
 * ld_symbol_category_new:
 * @name: the name of the new category.
 * @human_name: the localized human name of the new category.
 *
 * Create an instance.
 */
LdSymbolCategory *
ld_symbol_category_new (const gchar *name, const gchar *human_name)
{
	LdSymbolCategory *cat;

	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (human_name != NULL, NULL);

	cat = g_object_new (LD_TYPE_SYMBOL_CATEGORY, NULL);
	cat->priv->name = g_strdup (name);
	cat->priv->human_name = g_strdup (human_name);

	return cat;
}

/**
 * ld_symbol_category_set_name:
 * @self: an #LdSymbolCategory object.
 * @name: the new name for this category.
 */
void
ld_symbol_category_set_name (LdSymbolCategory *self, const gchar *name)
{
	g_return_if_fail (LD_IS_SYMBOL_CATEGORY (self));
	g_return_if_fail (name != NULL);

	if (self->priv->name)
		g_free (self->priv->name);
	self->priv->name = g_strdup (name);

	g_object_notify (G_OBJECT (self), "name");
}

/**
 * ld_symbol_category_get_name:
 * @self: an #LdSymbolCategory object.
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
 * ld_symbol_category_set_human_name:
 * @self: an #LdSymbolCategory object.
 * @human_name: the new localized human name for this category.
 */
void
ld_symbol_category_set_human_name (LdSymbolCategory *self,
	const gchar *human_name)
{
	g_return_if_fail (LD_IS_SYMBOL_CATEGORY (self));
	g_return_if_fail (human_name != NULL);

	if (self->priv->human_name)
		g_free (self->priv->human_name);
	self->priv->human_name = g_strdup (human_name);

	g_object_notify (G_OBJECT (self), "human-name");
}

/**
 * ld_symbol_category_get_human_name:
 * @self: an #LdSymbolCategory object.
 *
 * Return the localized human name of this category.
 */
const gchar *
ld_symbol_category_get_human_name (LdSymbolCategory *self)
{
	g_return_val_if_fail (LD_IS_SYMBOL_CATEGORY (self), NULL);
	return self->priv->human_name;
}

/**
 * ld_symbol_category_set_image_path:
 * @self: an #LdSymbolCategory object.
 * @image_path: (allow-none): The new path to the image for this category.
 */
void
ld_symbol_category_set_image_path (LdSymbolCategory *self,
	const gchar *image_path)
{
	g_return_if_fail (LD_IS_SYMBOL_CATEGORY (self));

	if (self->priv->image_path)
		g_free (self->priv->image_path);
	self->priv->image_path = g_strdup (image_path);

	g_object_notify (G_OBJECT (self), "image-path");
}

/**
 * ld_symbol_category_get_image_path:
 * @self: an #LdSymbolCategory object.
 *
 * Return value: (allow-none): filesystem path to the image for this category.
 */
const gchar *
ld_symbol_category_get_image_path (LdSymbolCategory *self)
{
	g_return_val_if_fail (LD_IS_SYMBOL_CATEGORY (self), NULL);
	return self->priv->image_path;
}

/**
 * ld_symbol_category_insert_child:
 * @self: an #LdSymbolCategory object.
 * @child: the child to be inserted.
 * @pos: the position at which the child will be inserted.
 *       Negative values will append to the end of list.
 *
 * Insert a child into the category.
 */
void
ld_symbol_category_insert_child (LdSymbolCategory *self,
	GObject *child, gint pos)
{
	g_return_if_fail (LD_IS_SYMBOL_CATEGORY (self));
	g_return_if_fail (G_IS_OBJECT (child));

	g_object_ref (child);
	self->priv->children = g_slist_insert (self->priv->children, child, pos);
}

/**
 * ld_symbol_category_remove_child:
 * @self: an #LdSymbolCategory object.
 * @child: the child to be removed.
 *
 * Removes a child from the category.
 */
void
ld_symbol_category_remove_child (LdSymbolCategory *self,
	GObject *child)
{
	g_return_if_fail (LD_IS_SYMBOL_CATEGORY (self));
	g_return_if_fail (G_IS_OBJECT (child));

	g_object_unref (child);
	self->priv->children = g_slist_remove (self->priv->children, child);
}

/**
 * ld_symbol_category_get_children:
 * @self: an #LdSymbolCategory object.
 *
 * Return value: (element-type GObject): a list of children. Do not modify.
 */
const GSList *
ld_symbol_category_get_children (LdSymbolCategory *self)
{
	g_return_val_if_fail (LD_IS_SYMBOL_CATEGORY (self), NULL);
	return self->priv->children;
}

