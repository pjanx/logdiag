/*
 * ld-category.c
 *
 * This file is a part of logdiag.
 * Copyright 2010, 2011, 2012 Přemysl Eric Janouch
 *
 * See the file LICENSE for licensing information.
 *
 */

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-category
 * @short_description: A category of symbols
 * @see_also: #LdSymbol, #LdLibrary
 *
 * #LdCategory represents a category of #LdSymbol objects.
 */

/*
 * LdCategoryPrivate:
 * @parent: the parent of this category.
 * @name: the name of this category.
 * @human_name: the localized human-readable name of this category.
 * @symbols: (element-type LdSymbol *): symbols in this category.
 * @subcategories: (element-type LdCategory *) children of this category.
 */
struct _LdCategoryPrivate
{
	LdCategory *parent;
	gchar *name;
	gchar *human_name;
	GSList *symbols;
	GSList *subcategories;
};

enum
{
	PROP_0,
	PROP_PARENT,
	PROP_NAME,
	PROP_HUMAN_NAME
};

static void ld_category_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_category_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);
static void ld_category_finalize (GObject *gobject);

static void on_category_notify_name (LdCategory *category,
	GParamSpec *pspec, gpointer user_data);


G_DEFINE_TYPE (LdCategory, ld_category, G_TYPE_OBJECT)

static void
ld_category_class_init (LdCategoryClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_category_get_property;
	object_class->set_property = ld_category_set_property;
	object_class->finalize = ld_category_finalize;

/**
 * LdCategory:parent:
 *
 * The parent of this symbol category.
 */
	pspec = g_param_spec_string ("parent", "Parent",
		"The parent of this symbol category.",
		"", G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_PARENT, pspec);

/**
 * LdCategory:name:
 *
 * The name of this symbol category.
 */
	pspec = g_param_spec_string ("name", "Name",
		"The name of this symbol category.",
		"", G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_NAME, pspec);

/**
 * LdCategory:human-name:
 *
 * The localized human name of this symbol category.
 */
	pspec = g_param_spec_string ("human-name", "Human name",
		"The localized human name of this symbol category.",
		"", G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_HUMAN_NAME, pspec);

/**
 * LdCategory::symbols-changed:
 *
 * The list of symbols has changed.
 */
	klass->symbols_changed_signal = g_signal_new
		("symbols-changed", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST, 0, NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

/**
 * LdCategory::children-changed:
 *
 * The list of subcategory children has changed.
 */
	klass->children_changed_signal = g_signal_new
		("children-changed", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST, 0, NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	g_type_class_add_private (klass, sizeof (LdCategoryPrivate));
}

static void
ld_category_init (LdCategory *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_CATEGORY, LdCategoryPrivate);
}

static void
ld_category_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdCategory *self;

	self = LD_CATEGORY (object);
	switch (property_id)
	{
	case PROP_NAME:
		g_value_set_string (value, ld_category_get_name (self));
		break;
	case PROP_HUMAN_NAME:
		g_value_set_string (value, ld_category_get_human_name (self));
		break;
	case PROP_PARENT:
		g_value_set_object (value, ld_category_get_parent (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_category_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdCategory *self;

	self = LD_CATEGORY (object);
	switch (property_id)
	{
	case PROP_NAME:
		ld_category_set_name (self, g_value_get_string (value));
		break;
	case PROP_HUMAN_NAME:
		ld_category_set_human_name (self, g_value_get_string (value));
		break;
	case PROP_PARENT:
		ld_category_set_parent (self, g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
uninstall_category_cb (LdCategory *category, LdCategory *self)
{
	g_signal_handlers_disconnect_by_func (category,
		on_category_notify_name, self);
	if (ld_category_get_parent (category) == self)
		ld_category_set_parent (category, NULL);
	g_object_unref (category);
}

static void
parent_weak_notify (gpointer data, GObject *object)
{
	LdCategory *self;

	/* In practice this should never happen, for it would mean that
	 * we have a parent that have us as its child.
	 */
	self = (LdCategory *) data;
	if (self->priv->parent)
	{
		self->priv->parent = NULL;
		g_object_notify (G_OBJECT (self), "parent");
	}
}

static void
ld_category_finalize (GObject *gobject)
{
	LdCategory *self;

	self = LD_CATEGORY (gobject);

	if (self->priv->parent)
		g_object_weak_unref
			(G_OBJECT (self->priv->parent), parent_weak_notify, self);

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
	G_OBJECT_CLASS (ld_category_parent_class)->finalize (gobject);
}


/**
 * ld_category_new:
 * @name: the name of the new category.
 * @human_name: the localized human name of the new category.
 *
 * Create an instance.
 */
LdCategory *
ld_category_new (const gchar *name, const gchar *human_name)
{
	LdCategory *cat;

	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (human_name != NULL, NULL);

	cat = g_object_new (LD_TYPE_CATEGORY, NULL);
	cat->priv->name = g_strdup (name);
	cat->priv->human_name = g_strdup (human_name);

	return cat;
}

/**
 * ld_category_set_name:
 * @self: an #LdCategory object.
 * @name: the new name for this category.
 */
void
ld_category_set_name (LdCategory *self, const gchar *name)
{
	g_return_if_fail (LD_IS_CATEGORY (self));
	g_return_if_fail (name != NULL);

	if (self->priv->name)
		g_free (self->priv->name);
	self->priv->name = g_strdup (name);

	g_object_notify (G_OBJECT (self), "name");
}

/**
 * ld_category_get_name:
 * @self: an #LdCategory object.
 *
 * Return the name of this category.
 */
const gchar *
ld_category_get_name (LdCategory *self)
{
	g_return_val_if_fail (LD_IS_CATEGORY (self), NULL);
	return self->priv->name;
}

/**
 * ld_category_set_human_name:
 * @self: an #LdCategory object.
 * @human_name: the new localized human name for this category.
 */
void
ld_category_set_human_name (LdCategory *self, const gchar *human_name)
{
	g_return_if_fail (LD_IS_CATEGORY (self));
	g_return_if_fail (human_name != NULL);

	if (self->priv->human_name)
		g_free (self->priv->human_name);
	self->priv->human_name = g_strdup (human_name);

	g_object_notify (G_OBJECT (self), "human-name");
}

/**
 * ld_category_get_human_name:
 * @self: an #LdCategory object.
 *
 * Return the localized human name of this category.
 */
const gchar *
ld_category_get_human_name (LdCategory *self)
{
	g_return_val_if_fail (LD_IS_CATEGORY (self), NULL);
	return self->priv->human_name;
}

/**
 * ld_category_insert_symbol:
 * @self: an #LdCategory object.
 * @symbol: the symbol to be inserted.
 * @pos: the position at which the symbol will be inserted.
 *       Negative values will append to the end of list.
 *
 * Insert a symbol into the category.
 *
 * Return value: %TRUE if successful (no name collisions).
 */
gboolean
ld_category_insert_symbol (LdCategory *self, LdSymbol *symbol, gint pos)
{
	const gchar *name;
	const GSList *iter;

	g_return_val_if_fail (LD_IS_CATEGORY (self), FALSE);
	g_return_val_if_fail (LD_IS_SYMBOL (symbol), FALSE);

	/* Check for name collisions. */
	name = ld_symbol_get_name (symbol);
	for (iter = self->priv->symbols; iter; iter = iter->next)
	{
		if (!strcmp (name, ld_symbol_get_name (iter->data)))
		{
			g_warning ("attempted to insert multiple `%s' symbols into"
				" category `%s'", name, ld_category_get_name (self));
			return FALSE;
		}
	}

	self->priv->symbols = g_slist_insert (self->priv->symbols, symbol, pos);
	g_object_ref (symbol);

	g_signal_emit (self,
		LD_CATEGORY_GET_CLASS (self)->symbols_changed_signal, 0);
	return TRUE;
}

/**
 * ld_category_remove_symbol:
 * @self: an #LdCategory object.
 * @symbol: the symbol to be removed.
 *
 * Removes a symbol from the category.
 */
void
ld_category_remove_symbol (LdCategory *self, LdSymbol *symbol)
{
	GSList *link;

	g_return_if_fail (LD_IS_CATEGORY (self));
	g_return_if_fail (LD_IS_SYMBOL (symbol));

	if ((link = g_slist_find (self->priv->symbols, symbol)))
	{
		self->priv->symbols = g_slist_delete_link (self->priv->symbols, link);
		g_object_unref (symbol);

		g_signal_emit (self,
			LD_CATEGORY_GET_CLASS (self)->symbols_changed_signal, 0);
	}
}

/**
 * ld_category_get_symbols:
 * @self: an #LdCategory object.
 *
 * Return value: (element-type LdSymbol *): a list of symbols.  Do not modify.
 */
const GSList *
ld_category_get_symbols (LdCategory *self)
{
	g_return_val_if_fail (LD_IS_CATEGORY (self), NULL);
	return self->priv->symbols;
}

/**
 * ld_category_set_parent:
 * @self: an #LdCategory object.
 * @parent: the new parent category.
 *
 * Set the parent of this category.
 */
void
ld_category_set_parent (LdCategory *self, LdCategory *parent)
{
	g_return_if_fail (LD_IS_CATEGORY (self));
	g_return_if_fail (parent == NULL || LD_IS_CATEGORY (parent));

	if (self->priv->parent)
		g_object_weak_unref
			(G_OBJECT (self->priv->parent), parent_weak_notify, self);

	self->priv->parent = parent;

	if (parent)
		g_object_weak_ref (G_OBJECT (parent), parent_weak_notify, self);

	g_object_notify (G_OBJECT (self), "parent");
}

/**
 * ld_category_get_parent:
 * @self: an #LdCategory object.
 *
 * Return value: the parent of this category.
 */
LdCategory *
ld_category_get_parent (LdCategory *self)
{
	g_return_val_if_fail (LD_IS_CATEGORY (self), NULL);
	return self->priv->parent;
}

/**
 * ld_category_get_path:
 * @self: an #LdCategory object.
 *
 * Return value: the path to this category within the library.
 */
gchar *
ld_category_get_path (LdCategory *self)
{
	LdCategory *iter;
	gchar *path = NULL, *new_path;

	g_return_val_if_fail (LD_IS_CATEGORY (self), NULL);

	for (iter = self; iter; iter = ld_category_get_parent (iter))
	{
		const gchar *name;

		/* Stop at the root category. */
		name = ld_category_get_name (iter);
		if (!strcmp (name, LD_LIBRARY_IDENTIFIER_SEPARATOR))
			break;

		new_path = g_build_path
			(LD_LIBRARY_IDENTIFIER_SEPARATOR, name, path, NULL);
		g_free (path);
		path = new_path;
	}

	return path;
}

static void
on_category_notify_name (LdCategory *category,
	GParamSpec *pspec, gpointer user_data)
{
	LdCategory *self;

	self = (LdCategory *) user_data;
	g_warning ("name of a library subcategory has changed");

	/* The easy way of handling it. */
	g_object_ref (category);
	ld_category_remove_child (self, category);
	ld_category_add_child (self, category);
	g_object_unref (category);
}

/**
 * ld_category_add_child:
 * @self: an #LdCategory object.
 * @category: the category to be inserted.
 *
 * Insert a subcategory into the category.
 *
 * Return value: %TRUE if successful (no name collisions).
 */
gboolean
ld_category_add_child (LdCategory *self, LdCategory *category)
{
	const gchar *name;
	GSList *iter;

	g_return_val_if_fail (LD_IS_CATEGORY (self), FALSE);
	g_return_val_if_fail (LD_IS_CATEGORY (category), FALSE);

	name = ld_category_get_name (category);
	for (iter = self->priv->subcategories; iter; iter = iter->next)
	{
		gint comp;

		comp = g_utf8_collate (name, ld_category_get_name (iter->data));
		if (!comp)
		{
			g_warning ("attempted to insert multiple `%s' subcategories into"
				" category `%s'", name, ld_category_get_name (self));
			return FALSE;
		}
		if (comp < 0)
			break;
	}

	g_signal_connect (category, "notify::name",
		G_CALLBACK (on_category_notify_name), self);
	self->priv->subcategories = g_slist_insert_before
		(self->priv->subcategories, iter, category);
	ld_category_set_parent (category, self);
	g_object_ref (category);

	g_signal_emit (self,
		LD_CATEGORY_GET_CLASS (self)->children_changed_signal, 0);
	return TRUE;
}

/**
 * ld_category_remove_child:
 * @self: an #LdCategory object.
 * @category: the category to be removed.
 *
 * Removes a subcategory from the category.
 */
void
ld_category_remove_child (LdCategory *self, LdCategory *category)
{
	GSList *link;

	g_return_if_fail (LD_IS_CATEGORY (self));
	g_return_if_fail (LD_IS_CATEGORY (category));

	if ((link = g_slist_find (self->priv->subcategories, category)))
	{
		self->priv->subcategories
			= g_slist_delete_link (self->priv->subcategories, link);
		uninstall_category_cb (category, self);

		g_signal_emit (self,
			LD_CATEGORY_GET_CLASS (self)->children_changed_signal, 0);
	}
}

/**
 * ld_category_get_children:
 * @self: an #LdCategory object.
 *
 * Return value: (element-type LdCategory *):
 *               a list of subcategories.  Do not modify.
 */
const GSList *
ld_category_get_children (LdCategory *self)
{
	g_return_val_if_fail (LD_IS_CATEGORY (self), NULL);
	return self->priv->subcategories;
}
