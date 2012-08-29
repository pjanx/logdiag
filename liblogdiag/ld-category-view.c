/*
 * ld-category-view.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-category-view
 * @short_description: A category view widget
 * @see_also: #LdCategory
 *
 * #LdCategoryView enables the user to drag symbols from #LdLibrary
 * onto #LdDiagramView.
 */

/*
 * LdCategoryViewPrivate:
 * @category: a category object assigned as a model.
 * @expander_prefix: a string to prepend to subcategory labels in expanders.
 */
struct _LdCategoryViewPrivate
{
	LdCategory *category;
	gchar *expander_prefix;
};

enum
{
	PROP_0,
	PROP_CATEGORY
};

static void ld_category_view_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_category_view_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);
static void ld_category_view_dispose (GObject *gobject);

static void reload_category (LdCategoryView *self);
static void load_category_cb (gpointer data, gpointer user_data);


G_DEFINE_TYPE (LdCategoryView, ld_category_view, GTK_TYPE_VBOX);

static void
ld_category_view_class_init (LdCategoryViewClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_category_view_get_property;
	object_class->set_property = ld_category_view_set_property;
	object_class->dispose = ld_category_view_dispose;

/**
 * LdCategoryView:category:
 *
 * The #LdCategory this widget retrieves content from.
 */
	pspec = g_param_spec_object ("category", "Category",
		"The symbol category that is shown by this widget.",
		LD_TYPE_LIBRARY, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_CATEGORY, pspec);

	g_type_class_add_private (klass, sizeof (LdCategoryViewPrivate));
}

static void
ld_category_view_init (LdCategoryView *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_CATEGORY_VIEW, LdCategoryViewPrivate);
}

static void
ld_category_view_dispose (GObject *gobject)
{
	LdCategoryView *self;

	self = LD_CATEGORY_VIEW (gobject);
	ld_category_view_set_category (self, NULL);

	g_free (self->priv->expander_prefix);
	self->priv->expander_prefix = NULL;

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_category_view_parent_class)->dispose (gobject);
}

static void
ld_category_view_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdCategoryView *self;

	self = LD_CATEGORY_VIEW (object);
	switch (property_id)
	{
	case PROP_CATEGORY:
		g_value_set_object (value, ld_category_view_get_category (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_category_view_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdCategoryView *self;

	self = LD_CATEGORY_VIEW (object);
	switch (property_id)
	{
	case PROP_CATEGORY:
		ld_category_view_set_category (self,
			LD_CATEGORY (g_value_get_object (value)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}


/**
 * ld_category_view_new:
 *
 * Create an instance.
 */
GtkWidget *
ld_category_view_new (void)
{
	return g_object_new (LD_TYPE_CATEGORY_VIEW, NULL);
}

/**
 * ld_category_view_set_category:
 * @self: an #LdCategoryView object.
 * @category: (allow-none): the category to be assigned to the widget.
 *
 * Assign an #LdCategory object to the widget.
 */
void
ld_category_view_set_category (LdCategoryView *self, LdCategory *category)
{
	g_return_if_fail (LD_IS_CATEGORY_VIEW (self));
	g_return_if_fail (LD_IS_CATEGORY (category) || category == NULL);

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
		g_object_ref (category);
	}
	reload_category (self);
	g_object_notify (G_OBJECT (self), "category");
}

/**
 * ld_category_view_get_category:
 * @self: an #LdCategoryView object.
 *
 * Return value: (transfer none): the #LdCategory object
 *               assigned to the widget.
 */
LdCategory *
ld_category_view_get_category (LdCategoryView *self)
{
	g_return_val_if_fail (LD_IS_CATEGORY_VIEW (self), NULL);
	return self->priv->category;
}

/**
 * ld_category_view_set_expander_prefix:
 * @self: an #LdCategoryView object.
 * @category: (allow-none): the new prefix.
 *
 * Set the prefix for inner #GtkExpander labels.
 */
void
ld_category_view_set_expander_prefix (LdCategoryView *self,
	const gchar *prefix)
{
	g_return_if_fail (LD_IS_CATEGORY_VIEW (self));
	g_free (self->priv->expander_prefix);

	if (prefix)
		self->priv->expander_prefix = g_strdup (prefix);
	else
		self->priv->expander_prefix = NULL;

	reload_category (self);
}

/**
 * ld_category_view_get_expander_prefix:
 * @self: an #LdCategoryView object.
 *
 * Return value: the prefix for inner expander labels.
 */
const gchar *
ld_category_view_get_expander_prefix (LdCategoryView *self)
{
	g_return_val_if_fail (LD_IS_CATEGORY_VIEW (self), NULL);
	return self->priv->expander_prefix;
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
reload_category (LdCategoryView *self)
{
	g_return_if_fail (LD_IS_CATEGORY_VIEW (self));

	/* Clear the toolbar first, if there was already something in it. */
	gtk_container_foreach (GTK_CONTAINER (self),
		(GtkCallback) gtk_widget_destroy, NULL);

	if (self->priv->category)
	{
		GSList *children;

		/* TODO: Also show the symbols. */

		children = (GSList *) ld_category_get_children (self->priv->category);
		if (children)
			g_slist_foreach (children, load_category_cb, self);
		else
			/* TODO: Don't show this if there are any symbols. */
			gtk_box_pack_start (GTK_BOX (self),
				create_empty_label (), FALSE, FALSE, 0);
	}
}

static void
load_category_cb (gpointer data, gpointer user_data)
{
	LdCategoryView *self;
	LdCategory *cat;
	GtkWidget *expander, *child;
	const gchar *name;
	gchar *label, *label_markup;

	g_return_if_fail (LD_IS_CATEGORY_VIEW (user_data));
	g_return_if_fail (LD_IS_CATEGORY (data));

	self = user_data;
	cat = data;

	name = ld_category_get_human_name (cat);
	if (self->priv->expander_prefix)
	{
		/* It's the least I can do to make it not look bad right now. */
		gchar *prefix_escaped, *name_escaped;

		prefix_escaped = g_markup_escape_text (self->priv->expander_prefix, -1);
		name_escaped = g_markup_escape_text (name, -1);

		label = g_strdup_printf ("%s: %s", self->priv->expander_prefix, name);
		label_markup = g_strdup_printf ("<small>%s:</small> %s",
			prefix_escaped, name_escaped);

		g_free (name_escaped);
		g_free (prefix_escaped);
	}
	else
	{
		label = g_strdup (name);
		label_markup = g_markup_escape_text (name, -1);
	}

	expander = gtk_expander_new (label_markup);
	gtk_expander_set_expanded (GTK_EXPANDER (expander), TRUE);
	gtk_expander_set_use_markup (GTK_EXPANDER (expander), TRUE);

	child = ld_category_view_new ();
	ld_category_view_set_expander_prefix (LD_CATEGORY_VIEW (child), label);
	ld_category_view_set_category (LD_CATEGORY_VIEW (child), cat);

	gtk_container_add (GTK_CONTAINER (expander), child);
	gtk_box_pack_start (GTK_BOX (self), expander, FALSE, FALSE, 0);

	g_free (label);
	g_free (label_markup);
}

