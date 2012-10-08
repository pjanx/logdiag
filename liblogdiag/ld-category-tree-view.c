/*
 * ld-category-tree-view.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2011, 2012. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-category-tree-view
 * @short_description: A category tree view widget
 * @see_also: #LdCategory
 *
 * #LdCategoryTreeView enables the user to drag symbols from #LdLibrary
 * onto #LdDiagramView.
 */

/*
 * LdCategoryTreeViewPrivate:
 * @category: a category object assigned as a model.
 * @expander_prefix: a string to prepend to subcategory labels in expanders.
 */
struct _LdCategoryTreeViewPrivate
{
	LdCategory *category;
	gchar *expander_prefix;
};

enum
{
	PROP_0,
	PROP_CATEGORY
};

static void ld_category_tree_view_get_property (GObject *object,
	guint property_id, GValue *value, GParamSpec *pspec);
static void ld_category_tree_view_set_property (GObject *object,
	guint property_id, const GValue *value, GParamSpec *pspec);
static void ld_category_tree_view_dispose (GObject *gobject);

static void ld_category_tree_view_set_category (LdCategoryView *iface,
	LdCategory *category);
static LdCategory *ld_category_tree_view_get_category (LdCategoryView *iface);


static void
ld_category_view_init (LdCategoryViewInterface *iface)
{
	iface->set_category = ld_category_tree_view_set_category;
	iface->get_category = ld_category_tree_view_get_category;
}

G_DEFINE_TYPE_WITH_CODE (LdCategoryTreeView,
	ld_category_tree_view, GTK_TYPE_VBOX,
	G_IMPLEMENT_INTERFACE (LD_TYPE_CATEGORY_VIEW, ld_category_view_init));

static void
ld_category_tree_view_class_init (LdCategoryTreeViewClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_category_tree_view_get_property;
	object_class->set_property = ld_category_tree_view_set_property;
	object_class->dispose = ld_category_tree_view_dispose;

	g_object_class_override_property (object_class, PROP_CATEGORY, "category");

	g_type_class_add_private (klass, sizeof (LdCategoryTreeViewPrivate));
}

static void
ld_category_tree_view_init (LdCategoryTreeView *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_CATEGORY_TREE_VIEW, LdCategoryTreeViewPrivate);
}

static void
ld_category_tree_view_dispose (GObject *gobject)
{
	LdCategoryTreeView *self;

	self = LD_CATEGORY_TREE_VIEW (gobject);
	ld_category_view_set_category (LD_CATEGORY_VIEW (self), NULL);

	g_free (self->priv->expander_prefix);
	self->priv->expander_prefix = NULL;

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_category_tree_view_parent_class)->dispose (gobject);
}

static void
ld_category_tree_view_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	switch (property_id)
	{
	case PROP_CATEGORY:
		g_value_set_object (value,
			ld_category_view_get_category (LD_CATEGORY_VIEW (object)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_category_tree_view_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	switch (property_id)
	{
	case PROP_CATEGORY:
		ld_category_view_set_category (LD_CATEGORY_VIEW (object),
			LD_CATEGORY (g_value_get_object (value)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}


static GtkWidget *
create_empty_label (void)
{
	GtkWidget *label;
	PangoAttrList *attr;

	label = gtk_label_new (_("Empty"));
	gtk_widget_set_sensitive (label, FALSE);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), 5, 0);

	attr = pango_attr_list_new ();
	pango_attr_list_change (attr, pango_attr_style_new (PANGO_STYLE_ITALIC));
	gtk_label_set_attributes (GTK_LABEL (label), attr);
	pango_attr_list_unref (attr);

	return label;
}

static void
reconstruct_prefix (LdCategoryTreeView *self)
{
	LdCategory *iter;
	gchar *start, *end;

	start = g_strdup ("");
	end   = g_strdup ("");

	for (iter = self->priv->category; iter;
		iter = ld_category_get_parent (iter))
	{
		const gchar *name;
		gchar *new_start, *new_end, *name_escaped;

		/* Stop at the root category. */
		if (!strcmp (ld_category_get_name (iter),
			LD_LIBRARY_IDENTIFIER_SEPARATOR))
			break;

		name = ld_category_get_human_name (iter);
		name_escaped = g_markup_escape_text (name, -1);

		new_start = g_strconcat (start, "<small>", NULL);
		new_end   = g_strconcat (name_escaped, ":</small> ", end, NULL);

		g_free (name_escaped);
		g_free (start);
		g_free (end);

		start = new_start;
		end   = new_end;
	}

	g_free (self->priv->expander_prefix);
	self->priv->expander_prefix = g_strconcat (start, end, NULL);
	g_free (start);
	g_free (end);
}

static void
on_symbol_selected (GObject *source,
	LdSymbol *symbol, const gchar *path, LdCategoryTreeView *self)
{
	g_signal_emit (self, LD_CATEGORY_VIEW_GET_INTERFACE (self)->
		symbol_selected_signal, 0, symbol, path);
}

static void
on_symbol_deselected (GObject *source,
	LdSymbol *symbol, const gchar *path, LdCategoryTreeView *self)
{
	g_signal_emit (self, LD_CATEGORY_VIEW_GET_INTERFACE (self)->
		symbol_deselected_signal, 0, symbol, path);
}

static void
load_category_cb (gpointer data, gpointer user_data)
{
	LdCategoryTreeView *self;
	LdCategory *cat;
	GtkWidget *expander, *child;
	gchar *name, *label_markup;

	g_return_if_fail (LD_IS_CATEGORY_TREE_VIEW (user_data));
	g_return_if_fail (LD_IS_CATEGORY (data));

	self = user_data;
	cat = data;

	name = g_markup_escape_text (ld_category_get_human_name (cat), -1);
	label_markup = g_strconcat (self->priv->expander_prefix, name, NULL);
	g_free (name);

	expander = gtk_expander_new (label_markup);
	gtk_expander_set_expanded (GTK_EXPANDER (expander), TRUE);
	gtk_expander_set_use_markup (GTK_EXPANDER (expander), TRUE);
	g_free (label_markup);

	child = ld_category_tree_view_new (cat);
	gtk_container_add (GTK_CONTAINER (expander), child);
	gtk_box_pack_start (GTK_BOX (self), expander, FALSE, FALSE, 0);

	g_signal_connect_after (child, "symbol-selected",
		G_CALLBACK (on_symbol_selected), self);
	g_signal_connect_after (child, "symbol-deselected",
		G_CALLBACK (on_symbol_deselected), self);
}

static void
reload_category (LdCategoryTreeView *self)
{
	g_return_if_fail (LD_IS_CATEGORY_TREE_VIEW (self));

	/* Clear the container first, if there is already something in it. */
	gtk_container_foreach (GTK_CONTAINER (self),
		(GtkCallback) gtk_widget_destroy, NULL);

	/* XXX: We might want to disconnect signal handlers. */

	if (self->priv->category)
	{
		GSList *symbols, *children;

		symbols  = (GSList *) ld_category_get_symbols  (self->priv->category);
		children = (GSList *) ld_category_get_children (self->priv->category);

		if (symbols)
		{
			GtkWidget *symbol_view;

			symbol_view = ld_category_symbol_view_new (self->priv->category);
			gtk_box_pack_start (GTK_BOX (self), symbol_view, FALSE, FALSE, 0);

			g_signal_connect_after (symbol_view, "symbol-selected",
				G_CALLBACK (on_symbol_selected), self);
			g_signal_connect_after (symbol_view, "symbol-deselected",
				G_CALLBACK (on_symbol_deselected), self);
		}

		if (children)
		{
			reconstruct_prefix (self);
			g_slist_foreach (children, load_category_cb, self);
		}
		else if (!symbols)
			gtk_box_pack_start (GTK_BOX (self),
				create_empty_label (), FALSE, FALSE, 0);
	}
}

/* ===== Interface ========================================================= */

/**
 * ld_category_tree_view_new:
 * @category: (allow-none): a category to be assigned to the widget.
 *
 * Create an instance.
 */
GtkWidget *
ld_category_tree_view_new (LdCategory *category)
{
	LdCategoryTreeView *self;

	self = g_object_new (LD_TYPE_CATEGORY_TREE_VIEW, NULL);
	ld_category_view_set_category (LD_CATEGORY_VIEW (self), category);
	return GTK_WIDGET (self);
}

static void
ld_category_tree_view_set_category (LdCategoryView *iface, LdCategory *category)
{
	LdCategoryTreeView *self;

	g_return_if_fail (LD_IS_CATEGORY_TREE_VIEW (iface));
	g_return_if_fail (LD_IS_CATEGORY (category) || category == NULL);

	self = LD_CATEGORY_TREE_VIEW (iface);
	if (self->priv->category)
	{
		g_signal_handlers_disconnect_by_func (self->priv->category,
			reload_category, self);
		g_object_unref (self->priv->category);
	}

	self->priv->category = category;

	if (category)
	{
		g_signal_connect_data (category, "children-changed",
			G_CALLBACK (reload_category), self,
			NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);
		g_signal_connect_data (category, "notify::parent",
			G_CALLBACK (reload_category), self,
			NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);
		g_object_ref (category);
	}
	reload_category (self);
	g_object_notify (G_OBJECT (self), "category");
}

static LdCategory *
ld_category_tree_view_get_category (LdCategoryView *iface)
{
	g_return_val_if_fail (LD_IS_CATEGORY_TREE_VIEW (iface), NULL);
	return LD_CATEGORY_TREE_VIEW (iface)->priv->category;
}
