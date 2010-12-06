/*
 * ld-canvas.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <math.h>
#include <gtk/gtk.h>

#include "config.h"

#include "ld-marshal.h"
#include "ld-document-object.h"
#include "ld-document.h"
#include "ld-symbol.h"
#include "ld-library.h"
#include "ld-canvas.h"


/**
 * SECTION:ld-canvas
 * @short_description: A canvas.
 * @see_also: #LdDocument
 *
 * #LdCanvas is used for displaying #LdDocument objects.
 */

/* Milimetres per inch. */
#define MM_PER_INCH 25.4

/*
 * LdCanvasPrivate:
 * @document: A document object assigned to this canvas as a model.
 * @library: A library object assigned to this canvas as a model.
 * @adjustment_h: An adjustment object for the horizontal axis, if any.
 * @adjustment_v: An adjustment object for the vertical axis, if any.
 * @x: The X coordinate of the center of view.
 * @y: The Y coordinate of the center of view.
 * @zoom: The current zoom of the canvas.
 */
struct _LdCanvasPrivate
{
	LdDocument *document;
	LdLibrary *library;

	GtkAdjustment *adjustment_h;
	GtkAdjustment *adjustment_v;

	gdouble x;
	gdouble y;
	gdouble zoom;
};

G_DEFINE_TYPE (LdCanvas, ld_canvas, GTK_TYPE_DRAWING_AREA);

enum
{
	PROP_0,
	PROP_DOCUMENT,
	PROP_LIBRARY
};

typedef struct _DrawData DrawData;

/*
 * DrawData:
 * @self: Our #LdCanvas.
 * @cr: A cairo context to draw on.
 * @exposed_rect: The area that is to be redrawn.
 * @scale: Computed size of one document unit in pixels.
 */
struct _DrawData
{
	LdCanvas *self;
	cairo_t *cr;
	cairo_rectangle_t exposed_rect;
	gdouble scale;
};

static void ld_canvas_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_canvas_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);
static void ld_canvas_finalize (GObject *gobject);

static void on_adjustment_value_changed
	(GtkAdjustment *adjustment, LdCanvas *self);
static void ld_canvas_set_scroll_adjustments
	(LdCanvas *self, GtkAdjustment *horizontal, GtkAdjustment *vertical);

static gdouble ld_canvas_get_base_unit_in_px (GtkWidget *self);
static gdouble ld_canvas_get_scale_in_px (LdCanvas *self);

static void on_size_allocate (GtkWidget *widget, GtkAllocation *allocation,
	gpointer user_data);
static gboolean on_expose_event (GtkWidget *widget, GdkEventExpose *event,
	gpointer user_data);
static void draw_grid (GtkWidget *widget, DrawData *data);
static void draw_document (GtkWidget *widget, DrawData *data);


static void
ld_canvas_class_init (LdCanvasClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GParamSpec *pspec;

	widget_class = GTK_WIDGET_CLASS (klass);

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_canvas_get_property;
	object_class->set_property = ld_canvas_set_property;
	object_class->finalize = ld_canvas_finalize;

/**
 * LdCanvas:document:
 *
 * The underlying #LdDocument object of this canvas.
 */
	pspec = g_param_spec_object ("document", "Document",
		"The underlying document object of this canvas.",
		LD_TYPE_DOCUMENT, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_DOCUMENT, pspec);

/**
 * LdCanvas:library:
 *
 * The #LdLibrary that this canvas retrieves symbols from.
 */
	pspec = g_param_spec_object ("library", "Library",
		"The library that this canvas retrieves symbols from.",
		LD_TYPE_LIBRARY, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_DOCUMENT, pspec);

	klass->set_scroll_adjustments = ld_canvas_set_scroll_adjustments;

/**
 * LdCanvas::set-scroll-adjustments:
 * @horizontal: The horizontal #GtkAdjustment.
 * @vertical: The vertical #GtkAdjustment.
 *
 * Set scroll adjustments for the canvas.
 */
	widget_class->set_scroll_adjustments_signal = g_signal_new
		("set-scroll-adjustments", G_TYPE_FROM_CLASS (widget_class),
		G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (LdCanvasClass, set_scroll_adjustments),
		NULL, NULL,
		g_cclosure_user_marshal_VOID__OBJECT_OBJECT,
		G_TYPE_NONE, 2, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);

	g_type_class_add_private (klass, sizeof (LdCanvasPrivate));
}

static void
ld_canvas_init (LdCanvas *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_CANVAS, LdCanvasPrivate);

	self->priv->x = 0;
	self->priv->y = 0;
	self->priv->zoom = 1;

	g_signal_connect (self, "expose-event",
		G_CALLBACK (on_expose_event), NULL);
	g_signal_connect (self, "size-allocate",
		G_CALLBACK (on_size_allocate), NULL);
}

static void
ld_canvas_finalize (GObject *gobject)
{
	LdCanvas *self;

	self = LD_CANVAS (gobject);

	ld_canvas_set_scroll_adjustments (self, NULL, NULL);

	if (self->priv->document)
		g_object_unref (self->priv->document);
	if (self->priv->library)
		g_object_unref (self->priv->library);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_canvas_parent_class)->finalize (gobject);
}

static void
ld_canvas_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdCanvas *self;

	self = LD_CANVAS (object);
	switch (property_id)
	{
	case PROP_DOCUMENT:
		g_value_set_object (value, ld_canvas_get_document (self));
		break;
	case PROP_LIBRARY:
		g_value_set_object (value, ld_canvas_get_library (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_canvas_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdCanvas *self;

	self = LD_CANVAS (object);
	switch (property_id)
	{
	case PROP_DOCUMENT:
		ld_canvas_set_document (self, LD_DOCUMENT (g_value_get_object (value)));
		break;
	case PROP_LIBRARY:
		ld_canvas_set_library (self, LD_LIBRARY (g_value_get_object (value)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_canvas_set_scroll_adjustments (LdCanvas *self,
	GtkAdjustment *horizontal, GtkAdjustment *vertical)
{
	/* TODO: Infinite canvas. */
	GtkWidget *widget;
	gdouble scale;

	widget = GTK_WIDGET (self);
	scale = ld_canvas_get_scale_in_px (self);

	if (horizontal != self->priv->adjustment_h)
	{
		if (self->priv->adjustment_h)
		{
			g_signal_handlers_disconnect_by_func (self->priv->adjustment_h,
				on_adjustment_value_changed, self);
			g_object_unref (self->priv->adjustment_h);

			self->priv->adjustment_h = NULL;
		}
		if (horizontal)
		{
			g_object_ref (horizontal);
			g_signal_connect (horizontal, "value-changed",
				G_CALLBACK (on_adjustment_value_changed), self);

			horizontal->upper = 100;
			horizontal->lower = -100;
			horizontal->step_increment = 0.5;
			horizontal->page_increment = 5;
			horizontal->page_size = widget->allocation.width / scale;
			horizontal->value = -horizontal->page_size / 2;

			self->priv->adjustment_h = horizontal;
		}
	}

	if (vertical != self->priv->adjustment_v)
	{
		if (self->priv->adjustment_v)
		{
			g_signal_handlers_disconnect_by_func (self->priv->adjustment_v,
				on_adjustment_value_changed, self);
			g_object_unref (self->priv->adjustment_v);

			self->priv->adjustment_v = NULL;
		}
		if (vertical)
		{
			g_object_ref (vertical);
			g_signal_connect (vertical, "value-changed",
				G_CALLBACK (on_adjustment_value_changed), self);

			vertical->upper = 100;
			vertical->lower = -100;
			vertical->step_increment = 0.5;
			vertical->page_increment = 5;
			vertical->page_size = widget->allocation.height / scale;
			vertical->value = -vertical->page_size / 2;

			self->priv->adjustment_v = vertical;
		}
	}
}

static void
on_adjustment_value_changed (GtkAdjustment *adjustment, LdCanvas *self)
{
	GtkWidget *widget;
	gdouble scale;

	widget = GTK_WIDGET (self);
	scale = ld_canvas_get_scale_in_px (self);

	if (adjustment == self->priv->adjustment_h)
	{
		self->priv->x = adjustment->value
			+ widget->allocation.width / scale / 2;
		gtk_widget_queue_draw (widget);
	}
	else if (adjustment == self->priv->adjustment_v)
	{
		self->priv->y = adjustment->value
			+ widget->allocation.height / scale / 2;
		gtk_widget_queue_draw (widget);
	}
}

static void
on_size_allocate (GtkWidget *widget, GtkAllocation *allocation,
	gpointer user_data)
{
	LdCanvas *self;
	gdouble scale;

	self = LD_CANVAS (widget);
	scale = ld_canvas_get_scale_in_px (self);

	/* FIXME: If the new allocation is bigger, we may see more than
	 *        what we're supposed to be able to see -> adjust X and Y.
	 *
	 *        If the visible area is just so large that we must see more,
	 *        let's disable the scrollbars in question.
	 */
	if (self->priv->adjustment_h)
	{
		self->priv->adjustment_h->page_size = allocation->width / scale;
		self->priv->adjustment_h->value
			= self->priv->x - self->priv->adjustment_h->page_size / 2;
		gtk_adjustment_changed (self->priv->adjustment_h);
	}

	if (self->priv->adjustment_v)
	{
		self->priv->adjustment_v->page_size = allocation->height / scale;
		self->priv->adjustment_v->value
			= self->priv->y - self->priv->adjustment_v->page_size / 2;
		gtk_adjustment_changed (self->priv->adjustment_v);
	}
}

/**
 * ld_canvas_new:
 *
 * Create an instance.
 */
LdCanvas *
ld_canvas_new (void)
{
	return g_object_new (LD_TYPE_CANVAS, NULL);
}

/**
 * ld_canvas_set_document:
 * @self: An #LdCanvas object.
 * @document: The #LdDocument to be assigned to the canvas.
 *
 * Assign an #LdDocument object to the canvas.
 */
void
ld_canvas_set_document (LdCanvas *self, LdDocument *document)
{
	if (self->priv->document)
		g_object_unref (self->priv->document);

	self->priv->document = document;
	g_object_ref (document);
}

/**
 * ld_canvas_get_document:
 * @self: An #LdCanvas object.
 *
 * Get the #LdDocument object assigned to this canvas.
 * The reference count on the document is not incremented.
 */
LdDocument *
ld_canvas_get_document (LdCanvas *self)
{
	return self->priv->document;
}

/**
 * ld_canvas_set_library:
 * @self: An #LdCanvas object.
 * @library: The #LdLibrary to be assigned to the canvas.
 *
 * Assign an #LdLibrary object to the canvas.
 */
void
ld_canvas_set_library (LdCanvas *self, LdLibrary *library)
{
	if (self->priv->library)
		g_object_unref (self->priv->library);

	self->priv->library = library;
	g_object_ref (library);
}

/**
 * ld_canvas_get_library:
 * @self: An #LdCanvas object.
 *
 * Get the #LdLibrary object assigned to this canvas.
 * The reference count on the library is not incremented.
 */
LdLibrary *
ld_canvas_get_library (LdCanvas *self)
{
	return self->priv->library;
}

/*
 * ld_canvas_get_base_unit_in_px:
 * @self: A #GtkWidget object to retrieve DPI from (indirectly).
 *
 * Return value: The length of the base unit in pixels.
 */
static gdouble
ld_canvas_get_base_unit_in_px (GtkWidget *self)
{
	g_return_val_if_fail (GTK_IS_WIDGET (self), 1);

	/* XXX: It might look better if the unit was rounded to a whole number. */
	return gdk_screen_get_resolution (gtk_widget_get_screen (self))
		/ MM_PER_INCH * LD_CANVAS_BASE_UNIT_LENGTH;
}

/*
 * ld_canvas_get_scale_in_px:
 * @self: An #LdCanvas object.
 *
 * Return value: The displayed length of the base unit in pixels.
 */
static gdouble
ld_canvas_get_scale_in_px (LdCanvas *self)
{
	g_return_val_if_fail (LD_IS_CANVAS (self), 1);

	return ld_canvas_get_base_unit_in_px (GTK_WIDGET (self))
		* self->priv->zoom;
}

/**
 * ld_canvas_translate_canvas_coordinates:
 * @self: An #LdCanvas object.
 * @x: The X coordinate to be translated.
 * @y: The Y coordinate to be translated.
 *
 * Translate coordinates located inside the canvas window
 * into document coordinates.
 */
void
ld_canvas_translate_canvas_coordinates (LdCanvas *self, gdouble *x, gdouble *y)
{
	GtkWidget *widget;
	gdouble scale;

	g_return_if_fail (LD_IS_CANVAS (self));

	widget = GTK_WIDGET (self);
	scale = ld_canvas_get_scale_in_px (self);

	/* We know document coordinates of the center of the canvas, so we may
	 * translate the given X and Y coordinates to this center and then scale
	 * them by dividing them by the length of the base unit in pixels
	 * times zoom of the canvas.
	 */
	*x = self->priv->x + (*x - (widget->allocation.width  * 0.5)) / scale;
	*y = self->priv->y + (*y - (widget->allocation.height * 0.5)) / scale;
}

/**
 * ld_canvas_translate_document_coordinates:
 * @self: An #LdCanvas object.
 * @x: The X coordinate to be translated.
 * @y: The Y coordinate to be translated.
 *
 * Translate document coordinates into canvas coordinates.
 */
void
ld_canvas_translate_document_coordinates (LdCanvas *self,
	gdouble *x, gdouble *y)
{
	GtkWidget *widget;
	gdouble scale;

	g_return_if_fail (LD_IS_CANVAS (self));

	widget = GTK_WIDGET (self);
	scale = ld_canvas_get_scale_in_px (self);

	/* Just the reversal of ld_canvas_translate_canvas_coordinates(). */
	*x = scale * (*x - self->priv->x) + 0.5 * widget->allocation.width;
	*y = scale * (*y - self->priv->y) + 0.5 * widget->allocation.height;
}

static gboolean
on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	DrawData data;

	data.cr = gdk_cairo_create (widget->window);
	data.self = LD_CANVAS (widget);
	data.scale = ld_canvas_get_scale_in_px (data.self);
	data.exposed_rect.x = event->area.x;
	data.exposed_rect.y = event->area.y;
	data.exposed_rect.width = event->area.width;
	data.exposed_rect.height = event->area.height;

	cairo_rectangle (data.cr, data.exposed_rect.x, data.exposed_rect.y,
		data.exposed_rect.width, data.exposed_rect.height);
	cairo_clip (data.cr);

	/* Paint the background white. */
	cairo_set_source_rgb (data.cr, 1, 1, 1);
	cairo_paint (data.cr);

	draw_grid (widget, &data);
	draw_document (widget, &data);

	cairo_destroy (data.cr);
	return FALSE;
}

static void
draw_grid (GtkWidget *widget, DrawData *data)
{
	gdouble x_top, y_top;
	gdouble x, y;

	cairo_set_source_rgb (data->cr, 0.5, 0.5, 0.5);
	cairo_set_line_width (data->cr, 1);
	cairo_set_line_cap (data->cr, CAIRO_LINE_CAP_ROUND);

	/* Get coordinates of the most top-left grid point. */
	x_top = data->exposed_rect.x;
	y_top = data->exposed_rect.y;
	ld_canvas_translate_canvas_coordinates (data->self, &x_top, &y_top);
	x_top = ceil (x_top);
	y_top = ceil (y_top);
	ld_canvas_translate_document_coordinates (data->self, &x_top, &y_top);

	/* Iterate over all the points. */
	for     (x = x_top; x <= data->exposed_rect.x + data->exposed_rect.width;
	         x += data->scale)
	{
		for (y = y_top; y <= data->exposed_rect.y + data->exposed_rect.height;
		     y += data->scale)
		{
			cairo_move_to (data->cr, x, y);
			cairo_line_to (data->cr, x, y);
		}
	}
	cairo_stroke (data->cr);
}

static void
draw_document (GtkWidget *widget, DrawData *data)
{
	/* TODO: Draw symbols from the document. */
}

