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
 * @human_name: the localized human-readable name of this category.
 * @symbols: (element-type LdSymbol *): symbols in this category.
 * @subcategories: (element-type LdSymbolCategory *) children of this category.
 */
struct _LdSymbolCategoryPrivate
{
	gchar *name;
	gchar *human_name;
	GSList *symbols;
	GSList *subcategories;
};

enum
{
	PROP_0,
	PROP_NAME,
	PROP_HUMAN_NAME
};

static void ld_symbol_category_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_symbol_category_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);
static void ld_symbol_category_finalize (GObject *gobject);

static void on_category_notify_name (LdSymbolCategory *category,
	GParamSpec *pspec, gpointer user_data);


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
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
uninstall_category_cb (LdSymbolCategory *category, LdSymbolCategory *self)
{
	g_signal_handlers_disconnect_by_func (category,
		on_category_notify_name, self);
	g_object_unref (category);
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

	g_slist_foreach (self->priv->symbols, (GFunc) g_object_unref, NULL);
	g_slist_free (self->priv->symbols);

	g_slist_foreach (self->priv->subcategories,
		(GFunc) uninstall_category_cb, self);
	g_slist_free (self->priv->subcategories);

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
 * ld_symbol_category_insert_symbol:
 * @self: an #LdSymbolCategory object.
 * @symbol: the symbol to be inserted.
 * @pos: the position at which the symbol will be inserted.
 *       Negative values will append to the end of list.
 *
 * Insert a symbol into the category.
 *
 * Return value: %TRUE if successful (no name collisions).
 */
gboolean
ld_symbol_category_insert_symbol (LdSymbolCategory *self,
	LdSymbol *symbol, gint pos)
{
	const gchar *name;
	const GSList *iter;

	g_return_val_if_fail (LD_IS_SYMBOL_CATEGORY (self), FALSE);
	g_return_val_if_fail (LD_IS_SYMBOL (symbol), FALSE);

	/* Check for name collisions. */
	name = ld_symbol_get_name (symbol);
	for (iter = self->priv->symbols; iter; iter = iter->next)
	{
		if (!strcmp (name, ld_symbol_get_name (iter->data)))
		{
			g_warning ("attempted to insert multiple `%s' symbols into"
				" category `%s'", name, ld_symbol_category_get_name (self));
			return FALSE;
		}
	}

	self->priv->symbols = g_slist_insert (self->priv->symbols, symbol, pos);
	g_object_ref (symbol);
	return TRUE;
}

/**
 * ld_symbol_category_remove_symbol:
 * @self: an #LdSymbolCategory object.
 * @symbol: the symbol to be removed.
 *
 * Removes a symbol from the category.
 */
void
ld_symbol_category_remove_symbol (LdSymbolCategory *self,
	LdSymbol *symbol)
{
	g_return_if_fail (LD_IS_SYMBOL_CATEGORY (self));
	g_return_if_fail (LD_IS_SYMBOL (symbol));

	if (g_slist_find (self->priv->symbols, symbol))
	{
		self->priv->symbols = g_slist_remove (self->priv->symbols, symbol);
		g_object_unref (symbol);
	}
}

/**
 * ld_symbol_category_get_symbols:
 * @self: an #LdSymbolCategory object.
 *
 * Return value: (element-type LdSymbol *): a list of symbols.  Do not modify.
 */
const GSList *
ld_symbol_category_get_symbols (LdSymbolCategory *self)
{
	g_return_val_if_fail (LD_IS_SYMBOL_CATEGORY (self), NULL);
	return self->priv->symbols;
}


static void
on_category_notify_name (LdSymbolCategory *category,
	GParamSpec *pspec, gpointer user_data)
{
	LdSymbolCategory *self;

	self = (LdSymbolCategory *) user_data;
	g_warning ("name of a library subcategory has changed");

	/* The easy way of handling it. */
	g_object_ref (category);
	ld_symbol_category_remove_child (self, category);
	ld_symbol_category_add_child (self, category);
	g_object_unref (category);
}

/**
 * ld_symbol_category_add_child:
 * @self: an #LdSymbolCategory object.
 * @category: the category to be inserted.
 *
 * Insert a subcategory into the category.
 *
 * Return value: %TRUE if successful (no name collisions).
 */
gboolean
ld_symbol_category_add_child (LdSymbolCategory *self,
	LdSymbolCategory *category)
{
	const gchar *name;
	GSList *iter;

	g_return_val_if_fail (LD_IS_SYMBOL_CATEGORY (self), FALSE);
	g_return_val_if_fail (LD_IS_SYMBOL_CATEGORY (category), FALSE);

	name = ld_symbol_category_get_name (category);
	for (iter = self->priv->subcategories; iter; iter = iter->next)
	{
		gint comp;

		comp = g_utf8_collate (name, ld_symbol_category_get_name (iter->data));
		if (!comp)
		{
			g_warning ("attempted to insert multiple `%s' subcategories into"
				" category `%s'", name, ld_symbol_category_get_name (self));
			return FALSE;
		}
		if (comp < 0)
			break;
	}

	g_signal_connect (category, "notify::name",
		G_CALLBACK (on_category_notify_name), self);
	self->priv->subcategories = g_slist_insert_before
		(self->priv->subcategories, iter, category);
	g_object_ref (category);
	return TRUE;
}

/**
 * ld_symbol_category_remove_child:
 * @self: an #LdSymbolCategory object.
 * @category: the category to be removed.
 *
 * Removes a subcategory from the category.
 */
void
ld_symbol_category_remove_child (LdSymbolCategory *self,
	LdSymbolCategory *category)
{
	g_return_if_fail (LD_IS_SYMBOL_CATEGORY (self));
	g_return_if_fail (LD_IS_SYMBOL_CATEGORY (category));

	if (g_slist_find (self->priv->subcategories, category))
	{
		g_signal_handlers_disconnect_by_func (category,
			on_category_notify_name, self);
		self->priv->subcategories
			= g_slist_remove (self->priv->subcategories, category);
		g_object_unref (category);
	}
}

/**
 * ld_symbol_category_get_children:
 * @self: an #LdSymbolCategory object.
 *
 * Return value: (element-type LdSymbolCategory *):
 *               a list of subcategories.  Do not modify.
 */
const GSList *
ld_symbol_category_get_children (LdSymbolCategory *self)
{
	g_return_val_if_fail (LD_IS_SYMBOL_CATEGORY (self), NULL);
	return self->priv->subcategories;
}

