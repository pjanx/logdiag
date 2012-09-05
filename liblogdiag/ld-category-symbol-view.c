/*
 * ld-category-symbol-view.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-category-symbol-view
 * @short_description: A widget that displays symbols in a category
 * @see_also: #LdCategory, #LdDiagramView
 *
 * #LdCategorySymbolView allows the user to drag symbols from an #LdCategory
 * onto #LdDiagramView.
 */

/*
 * LdCategorySymbolViewPrivate:
 * @category: a category object assigned as a model.
 */
struct _LdCategorySymbolViewPrivate
{
	LdCategory *category;
	guint height_negotiation : 1;
};

enum
{
	PROP_0,
	PROP_CATEGORY
};

static void ld_category_symbol_view_get_property (GObject *object,
	guint property_id, GValue *value, GParamSpec *pspec);
static void ld_category_symbol_view_set_property (GObject *object,
	guint property_id, const GValue *value, GParamSpec *pspec);
static void ld_category_symbol_view_finalize (GObject *gobject);

static void on_size_request (GtkWidget *widget, GtkRequisition *requisition,
	gpointer user_data);
static void on_size_allocate (GtkWidget *widget, GdkRectangle *allocation,
	gpointer user_data);
static gboolean on_expose_event (GtkWidget *widget, GdkEventExpose *event,
	gpointer user_data);


G_DEFINE_TYPE (LdCategorySymbolView,
	ld_category_symbol_view, GTK_TYPE_DRAWING_AREA);

static void
ld_category_symbol_view_class_init (LdCategorySymbolViewClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_category_symbol_view_get_property;
	object_class->set_property = ld_category_symbol_view_set_property;
	object_class->finalize = ld_category_symbol_view_finalize;

/**
 * LdCategorySymbolView:category:
 *
 * The underlying #LdCategory object of this view.
 */
	pspec = g_param_spec_object ("category", "Category",
		"The underlying category object of this view.",
		LD_TYPE_CATEGORY, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_CATEGORY, pspec);

	g_type_class_add_private (klass, sizeof (LdCategorySymbolViewPrivate));
}

static void
ld_category_symbol_view_init (LdCategorySymbolView *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_CATEGORY_SYMBOL_VIEW, LdCategorySymbolViewPrivate);

	g_signal_connect (self, "size-allocate",
		G_CALLBACK (on_size_allocate), NULL);
	g_signal_connect (self, "size-request",
		G_CALLBACK (on_size_request), NULL);
	g_signal_connect (self, "expose-event",
		G_CALLBACK (on_expose_event), NULL);

	gtk_widget_add_events (GTK_WIDGET (self),
		GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK
		| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
		| GDK_LEAVE_NOTIFY_MASK);
}

static void
ld_category_symbol_view_finalize (GObject *gobject)
{
	LdCategorySymbolView *self;

	self = LD_CATEGORY_SYMBOL_VIEW (gobject);

	if (self->priv->category)
		g_object_unref (self->priv->category);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_category_symbol_view_parent_class)->finalize (gobject);
}

static void
ld_category_symbol_view_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdCategorySymbolView *self;

	self = LD_CATEGORY_SYMBOL_VIEW (object);
	switch (property_id)
	{
	case PROP_CATEGORY:
		g_value_set_object (value, ld_category_symbol_view_get_category (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_category_symbol_view_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdCategorySymbolView *self;

	self = LD_CATEGORY_SYMBOL_VIEW (object);
	switch (property_id)
	{
	case PROP_CATEGORY:
		ld_category_symbol_view_set_category (self,
			LD_CATEGORY (g_value_get_object (value)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
on_size_request (GtkWidget *widget, GtkRequisition *requisition,
	gpointer user_data)
{
	LdCategorySymbolView *self;

	self = LD_CATEGORY_SYMBOL_VIEW (widget);

	requisition->width = 10;

	if (self->priv->height_negotiation)
	{
		GtkAllocation alloc;

		gtk_widget_get_allocation (widget, &alloc);
		requisition->height = 5000 / alloc.width;
	}
	else
		requisition->height = 10;
}

static void
on_size_allocate (GtkWidget *widget, GdkRectangle *allocation,
	gpointer user_data)
{
	LdCategorySymbolView *self;

	self = LD_CATEGORY_SYMBOL_VIEW (widget);

	if (self->priv->height_negotiation)
		self->priv->height_negotiation = FALSE;
	else
	{
		self->priv->height_negotiation = TRUE;
		gtk_widget_queue_resize (widget);
	}
}

static gboolean
on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	cairo_t *cr;

	cr = gdk_cairo_create (gtk_widget_get_window (widget));
	gdk_cairo_rectangle (cr, &event->area);
	cairo_clip (cr);

	gdk_cairo_set_source_color (cr,
		&gtk_widget_get_style (widget)->base[GTK_STATE_NORMAL]);
	cairo_paint (cr);

	cairo_destroy (cr);
	return FALSE;
}

/* ===== Generic interface etc. ============================================ */

/**
 * ld_category_symbol_view_new:
 *
 * Create an instance.
 */
GtkWidget *
ld_category_symbol_view_new (void)
{
	return g_object_new (LD_TYPE_CATEGORY_SYMBOL_VIEW, NULL);
}

/**
 * ld_category_symbol_view_set_category:
 * @self: an #LdCategorySymbolView object.
 * @category: the #LdCategory to be assigned to the view.
 *
 * Assign an #LdCategory object to the view.
 */
void
ld_category_symbol_view_set_category (LdCategorySymbolView *self,
	LdCategory *category)
{
	g_return_if_fail (LD_IS_CATEGORY_SYMBOL_VIEW (self));
	g_return_if_fail (LD_IS_CATEGORY (category));

	if (self->priv->category)
		g_object_unref (self->priv->category);

	self->priv->category = category;
	g_object_ref (category);

	g_object_notify (G_OBJECT (self), "category");
}

/**
 * ld_category_symbol_view_get_category:
 * @self: an #LdCategorySymbolView object.
 *
 * Get the #LdCategory object assigned to this view.
 * The reference count on the category is not incremented.
 */
LdCategory *
ld_category_symbol_view_get_category (LdCategorySymbolView *self)
{
	g_return_val_if_fail (LD_IS_CATEGORY_SYMBOL_VIEW (self), NULL);
	return self->priv->category;
}
