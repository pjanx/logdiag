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
#include <gdk/gdkkeysyms.h>

#include "config.h"

#include "ld-marshal.h"
#include "ld-diagram-object.h"
#include "ld-diagram-symbol.h"
#include "ld-diagram.h"
#include "ld-symbol.h"
#include "ld-library.h"
#include "ld-canvas.h"


/**
 * SECTION:ld-canvas
 * @short_description: A canvas.
 * @see_also: #LdDiagram
 *
 * #LdCanvas displays and enables the user to manipulate with an #LdDiagram.
 */

/* Milimetres per inch. */
#define MM_PER_INCH 25.4

/* Tolerance on all sides of symbols for strokes. */
#define SYMBOL_AREA_CLIP_TOLERANCE 0.5

/* The default screen resolution in DPI units. */
#define DEFAULT_SCREEN_RESOLUTION 96

/*
 * OperationEnd:
 *
 * Called upon ending an operation.
 */
typedef void (*OperationEnd) (LdCanvas *self);

struct _AddObjectData
{
	LdDiagramObject *object;
	gboolean visible;
};

typedef struct _AddObjectData AddObjectData;


/*
 * LdCanvasPrivate:
 * @diagram: A diagram object assigned to this canvas as a model.
 * @library: A library object assigned to this canvas as a model.
 * @adjustment_h: An adjustment object for the horizontal axis, if any.
 * @adjustment_v: An adjustment object for the vertical axis, if any.
 * @x: The X coordinate of the center of view.
 * @y: The Y coordinate of the center of view.
 * @zoom: The current zoom of the canvas.
 * @operation: The current operation.
 * @operation_data: Data related to the current operation.
 * @operation_end: A callback to end the operation.
 */
struct _LdCanvasPrivate
{
	LdDiagram *diagram;
	LdLibrary *library;

	GtkAdjustment *adjustment_h;
	GtkAdjustment *adjustment_v;

	gdouble x;
	gdouble y;
	gdouble zoom;

	gint operation;
	OperationEnd operation_end;
	union
	{
		AddObjectData add_object;
	}
	operation_data;
};

#define OPER_DATA(self, member) ((self)->priv->operation_data.member)

G_DEFINE_TYPE (LdCanvas, ld_canvas, GTK_TYPE_DRAWING_AREA);

enum
{
	PROP_0,
	PROP_DIAGRAM,
	PROP_LIBRARY
};

enum
{
	OPER_0,
	OPER_ADD_OBJECT
};

/*
 * DrawData:
 * @self: Our #LdCanvas.
 * @cr: A cairo context to draw on.
 * @exposed_rect: The area that is to be redrawn.
 * @scale: Computed size of one diagram unit in pixels.
 */
struct _DrawData
{
	LdCanvas *self;
	cairo_t *cr;
	cairo_rectangle_t exposed_rect;
	gdouble scale;
};

typedef struct _DrawData DrawData;


static void ld_canvas_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_canvas_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);
static void ld_canvas_finalize (GObject *gobject);

static void ld_canvas_real_set_scroll_adjustments
	(LdCanvas *self, GtkAdjustment *horizontal, GtkAdjustment *vertical);
static void on_adjustment_value_changed
	(GtkAdjustment *adjustment, LdCanvas *self);
static void on_size_allocate (GtkWidget *widget, GtkAllocation *allocation,
	gpointer user_data);

static gdouble ld_canvas_get_base_unit_in_px (GtkWidget *self);
static gdouble ld_canvas_get_scale_in_px (LdCanvas *self);

static gboolean on_motion_notify (GtkWidget *widget, GdkEventMotion *event,
	gpointer user_data);
static gboolean on_leave_notify (GtkWidget *widget, GdkEventCrossing *event,
	gpointer user_data);
static gboolean on_button_press (GtkWidget *widget, GdkEventButton *event,
	gpointer user_data);
static gboolean on_button_release (GtkWidget *widget, GdkEventButton *event,
	gpointer user_data);

static void move_object_to_widget_coords (LdCanvas *self,
	LdDiagramObject *object, gdouble x, gdouble y);
static LdSymbol *resolve_diagram_symbol (LdCanvas *self,
	LdDiagramSymbol *diagram_symbol);
static gboolean get_symbol_clip_area_on_widget (LdCanvas *self,
	LdDiagramSymbol *diagram_symbol, gdouble *x, gdouble *y,
	gdouble *width, gdouble *height);
static void queue_object_redraw (LdCanvas *self, LdDiagramObject *object);

static void ld_canvas_real_cancel_operation (LdCanvas *self);
static void ld_canvas_add_object_end (LdCanvas *self);

static gboolean on_expose_event (GtkWidget *widget, GdkEventExpose *event,
	gpointer user_data);
static void draw_grid (GtkWidget *widget, DrawData *data);
static void draw_diagram (GtkWidget *widget, DrawData *data);
static void draw_object (LdDiagramObject *diagram_object, DrawData *data);
static void draw_symbol (LdDiagramSymbol *diagram_symbol, DrawData *data);


static void
ld_canvas_class_init (LdCanvasClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkBindingSet *binding_set;
	GParamSpec *pspec;

	widget_class = GTK_WIDGET_CLASS (klass);

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_canvas_get_property;
	object_class->set_property = ld_canvas_set_property;
	object_class->finalize = ld_canvas_finalize;

	klass->set_scroll_adjustments = ld_canvas_real_set_scroll_adjustments;
	klass->cancel_operation = ld_canvas_real_cancel_operation;

	binding_set = gtk_binding_set_by_class (klass);
	gtk_binding_entry_add_signal (binding_set, GDK_Escape, 0,
		"cancel-operation", 0);

/**
 * LdCanvas:diagram:
 *
 * The underlying #LdDiagram object of this canvas.
 */
	pspec = g_param_spec_object ("diagram", "Diagram",
		"The underlying diagram object of this canvas.",
		LD_TYPE_DIAGRAM, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_DIAGRAM, pspec);

/**
 * LdCanvas:library:
 *
 * The #LdLibrary that this canvas retrieves symbols from.
 */
	pspec = g_param_spec_object ("library", "Library",
		"The library that this canvas retrieves symbols from.",
		LD_TYPE_LIBRARY, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_LIBRARY, pspec);

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

/**
 * LdCanvas::cancel-operation:
 *
 * Cancel any current operation.
 */
	klass->cancel_operation_signal = g_signal_new
		("cancel-operation", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (LdCanvasClass, cancel_operation), NULL, NULL,
		g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);

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

	g_signal_connect (self, "motion-notify-event",
		G_CALLBACK (on_motion_notify), NULL);
	g_signal_connect (self, "leave-notify-event",
		G_CALLBACK (on_leave_notify), NULL);
	g_signal_connect (self, "button-press-event",
		G_CALLBACK (on_button_press), NULL);
	g_signal_connect (self, "button-release-event",
		G_CALLBACK (on_button_release), NULL);

	g_object_set (self, "can-focus", TRUE, NULL);

	gtk_widget_add_events (GTK_WIDGET (self),
		GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK
		| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
		| GDK_LEAVE_NOTIFY_MASK);
}

static void
ld_canvas_finalize (GObject *gobject)
{
	LdCanvas *self;

	self = LD_CANVAS (gobject);

	ld_canvas_real_set_scroll_adjustments (self, NULL, NULL);

	if (self->priv->diagram)
		g_object_unref (self->priv->diagram);
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
	case PROP_DIAGRAM:
		g_value_set_object (value, ld_canvas_get_diagram (self));
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
	case PROP_DIAGRAM:
		ld_canvas_set_diagram (self, LD_DIAGRAM (g_value_get_object (value)));
		break;
	case PROP_LIBRARY:
		ld_canvas_set_library (self, LD_LIBRARY (g_value_get_object (value)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_canvas_real_set_scroll_adjustments (LdCanvas *self,
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


/* ===== Generic interface etc. ============================================ */

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
 * ld_canvas_set_diagram:
 * @self: An #LdCanvas object.
 * @diagram: The #LdDiagram to be assigned to the canvas.
 *
 * Assign an #LdDiagram object to the canvas.
 */
void
ld_canvas_set_diagram (LdCanvas *self, LdDiagram *diagram)
{
	g_return_if_fail (LD_IS_CANVAS (self));
	g_return_if_fail (LD_IS_DIAGRAM (diagram));

	if (self->priv->diagram)
		g_object_unref (self->priv->diagram);

	self->priv->diagram = diagram;
	g_object_ref (diagram);

	g_object_notify (G_OBJECT (self), "diagram");
}

/**
 * ld_canvas_get_diagram:
 * @self: An #LdCanvas object.
 *
 * Get the #LdDiagram object assigned to this canvas.
 * The reference count on the diagram is not incremented.
 */
LdDiagram *
ld_canvas_get_diagram (LdCanvas *self)
{
	g_return_val_if_fail (LD_IS_CANVAS (self), NULL);
	return self->priv->diagram;
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
	g_return_if_fail (LD_IS_CANVAS (self));
	g_return_if_fail (LD_IS_LIBRARY (library));

	if (self->priv->library)
		g_object_unref (self->priv->library);

	self->priv->library = library;
	g_object_ref (library);

	g_object_notify (G_OBJECT (self), "library");
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
	g_return_val_if_fail (LD_IS_CANVAS (self), NULL);
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
	gdouble resolution;

	g_return_val_if_fail (GTK_IS_WIDGET (self), 1);

	resolution = gdk_screen_get_resolution (gtk_widget_get_screen (self));
	if (resolution == -1)
		resolution = DEFAULT_SCREEN_RESOLUTION;

	/* XXX: It might look better if the unit was rounded to a whole number. */
	return resolution / MM_PER_INCH * LD_CANVAS_BASE_UNIT_LENGTH;
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
 * ld_canvas_widget_to_diagram_coords:
 * @self: An #LdCanvas object.
 * @wx: The X coordinate to be translated.
 * @wy: The Y coordinate to be translated.
 * @dx: (out): The translated X coordinate.
 * @dy: (out): The translated Y coordinate.
 *
 * Translate coordinates located inside the canvas window
 * into diagram coordinates.
 */
void
ld_canvas_widget_to_diagram_coords (LdCanvas *self,
	gdouble wx, gdouble wy, gdouble *dx, gdouble *dy)
{
	GtkWidget *widget;
	gdouble scale;

	g_return_if_fail (LD_IS_CANVAS (self));
	g_return_if_fail (dx != NULL);
	g_return_if_fail (dy != NULL);

	widget = GTK_WIDGET (self);
	scale = ld_canvas_get_scale_in_px (self);

	/* We know diagram coordinates of the center of the canvas, so we may
	 * translate the given X and Y coordinates to this center and then scale
	 * them by dividing them by the current scale.
	 */
	*dx = self->priv->x + (wx - (widget->allocation.width  * 0.5)) / scale;
	*dy = self->priv->y + (wy - (widget->allocation.height * 0.5)) / scale;
}

/**
 * ld_canvas_diagram_to_widget_coords:
 * @self: An #LdCanvas object.
 * @dx: The X coordinate to be translated.
 * @dy: The Y coordinate to be translated.
 * @wx: (out): The translated X coordinate.
 * @wy: (out): The translated Y coordinate.
 *
 * Translate diagram coordinates into canvas coordinates.
 */
void
ld_canvas_diagram_to_widget_coords (LdCanvas *self,
	gdouble dx, gdouble dy, gdouble *wx, gdouble *wy)
{
	GtkWidget *widget;
	gdouble scale;

	g_return_if_fail (LD_IS_CANVAS (self));
	g_return_if_fail (wx != NULL);
	g_return_if_fail (wy != NULL);

	widget = GTK_WIDGET (self);
	scale = ld_canvas_get_scale_in_px (self);

	/* Just the reversal of ld_canvas_widget_to_diagram_coords(). */
	*wx = scale * (dx - self->priv->x) + 0.5 * widget->allocation.width;
	*wy = scale * (dy - self->priv->y) + 0.5 * widget->allocation.height;
}


/* ===== Operations ======================================================== */

static void
ld_canvas_real_cancel_operation (LdCanvas *self)
{
	g_return_if_fail (LD_IS_CANVAS (self));

	if (self->priv->operation)
	{
		if (self->priv->operation_end)
			self->priv->operation_end (self);
		self->priv->operation = OPER_0;
		self->priv->operation_end = NULL;
	}
}

/**
 * ld_canvas_add_object_begin:
 * @self: An #LdCanvas object.
 * @object: (transfer full): The object to be added to the diagram.
 *
 * Begin an operation for adding an object into the diagram.
 */
void
ld_canvas_add_object_begin (LdCanvas *self, LdDiagramObject *object)
{
	AddObjectData *data;

	g_return_if_fail (LD_IS_CANVAS (self));
	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (object));

	ld_canvas_real_cancel_operation (self);

	self->priv->operation = OPER_ADD_OBJECT;
	self->priv->operation_end = ld_canvas_add_object_end;

	data = &OPER_DATA (self, add_object);
	data->object = object;
}

static void
ld_canvas_add_object_end (LdCanvas *self)
{
	AddObjectData *data;

	data = &OPER_DATA (self, add_object);
	if (data->object)
	{
		queue_object_redraw (self, data->object);
		g_object_unref (data->object);
		data->object = NULL;
	}
}


/* ===== Events, rendering ================================================= */

static void
move_object_to_widget_coords (LdCanvas *self, LdDiagramObject *object,
	gdouble x, gdouble y)
{
	gdouble dx, dy;

	ld_canvas_widget_to_diagram_coords (self, x, y, &dx, &dy);
	ld_diagram_object_set_x (object, floor (dx + 0.5));
	ld_diagram_object_set_y (object, floor (dy + 0.5));
}

static LdSymbol *
resolve_diagram_symbol (LdCanvas *self, LdDiagramSymbol *diagram_symbol)
{
	if (!self->priv->library)
		return NULL;

	return ld_library_find_symbol (self->priv->library,
		ld_diagram_symbol_get_class (diagram_symbol));
}

static gboolean
get_symbol_clip_area_on_widget (LdCanvas *self, LdDiagramSymbol *diagram_symbol,
	gdouble *x, gdouble *y, gdouble *width, gdouble *height)
{
	LdSymbol *symbol;
	LdSymbolArea area;
	gdouble x1, x2;
	gdouble y1, y2;
	gdouble object_x, object_y;

	symbol = resolve_diagram_symbol (self, diagram_symbol);

	object_x = ld_diagram_object_get_x (LD_DIAGRAM_OBJECT (diagram_symbol));
	object_y = ld_diagram_object_get_y (LD_DIAGRAM_OBJECT (diagram_symbol));

	if (symbol)
		ld_symbol_get_area (symbol, &area);
	else
		return FALSE;

	/* TODO: Rotate the space for other orientations. */
	ld_canvas_diagram_to_widget_coords (self,
		object_x + area.x - SYMBOL_AREA_CLIP_TOLERANCE,
		object_y + area.y - SYMBOL_AREA_CLIP_TOLERANCE,
		&x1, &y1);
	ld_canvas_diagram_to_widget_coords (self,
		object_x + area.x + area.width  + SYMBOL_AREA_CLIP_TOLERANCE,
		object_y + area.y + area.height + SYMBOL_AREA_CLIP_TOLERANCE,
		&x2, &y2);

	*x = x1;
	*y = y1;
	*width  = x2 - x1;
	*height = y2 - y1;
	return TRUE;
}

static void
queue_object_redraw (LdCanvas *self, LdDiagramObject *object)
{
	if (LD_IS_DIAGRAM_SYMBOL (object))
	{
		gdouble x, y, width, height;

		if (!get_symbol_clip_area_on_widget (self, LD_DIAGRAM_SYMBOL (object),
			&x, &y, &width, &height))
			return;
		gtk_widget_queue_draw_area (GTK_WIDGET (self),
			floor (x), floor (y), ceil (width), ceil (height));
	}
}

static gboolean
on_motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	LdCanvas *self;

	self = LD_CANVAS (widget);
	switch (self->priv->operation)
	{
		AddObjectData *data;

	case OPER_ADD_OBJECT:
		data = &OPER_DATA (self, add_object);
		data->visible = TRUE;

		queue_object_redraw (self, data->object);
		move_object_to_widget_coords (self, data->object, event->x, event->y);
		queue_object_redraw (self, data->object);
		break;
	}
	return FALSE;
}

static gboolean
on_leave_notify (GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
	LdCanvas *self;

	self = LD_CANVAS (widget);
	switch (self->priv->operation)
	{
		AddObjectData *data;

	case OPER_ADD_OBJECT:
		data = &OPER_DATA (self, add_object);
		data->visible = FALSE;

		queue_object_redraw (self, data->object);
		break;
	}
	return FALSE;
}

static gboolean
on_button_press (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	LdCanvas *self;

	if (!gtk_widget_has_focus (widget))
		gtk_widget_grab_focus (widget);

	self = LD_CANVAS (widget);
	switch (self->priv->operation)
	{
		AddObjectData *data;

	case OPER_ADD_OBJECT:
		data = &OPER_DATA (self, add_object);

		queue_object_redraw (self, data->object);
		move_object_to_widget_coords (self, data->object, event->x, event->y);

		if (self->priv->diagram)
			ld_diagram_insert_object (self->priv->diagram, data->object, -1);

		/* XXX: "cancel" causes confusion. */
		ld_canvas_real_cancel_operation (self);
		break;
	}
	return FALSE;
}

static gboolean
on_button_release (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	return FALSE;
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

	gdk_cairo_rectangle (data.cr, &event->area);
	cairo_clip (data.cr);

	/* Paint the background white. */
	cairo_set_source_rgb (data.cr, 1, 1, 1);
	cairo_paint (data.cr);

	draw_grid (widget, &data);
	draw_diagram (widget, &data);

	cairo_destroy (data.cr);
	return FALSE;
}

static void
draw_grid (GtkWidget *widget, DrawData *data)
{
	gdouble x_init, y_init;
	gdouble x, y;

	cairo_set_source_rgb (data->cr, 0.5, 0.5, 0.5);
	cairo_set_line_width (data->cr, 1);
	cairo_set_line_cap (data->cr, CAIRO_LINE_CAP_ROUND);

	/* Get coordinates of the top-left point. */
	ld_canvas_widget_to_diagram_coords (data->self,
		data->exposed_rect.x, data->exposed_rect.y, &x_init, &y_init);
	ld_canvas_diagram_to_widget_coords (data->self,
		ceil (x_init), ceil (y_init), &x_init, &y_init);

	/* Iterate over all the points. */
	for     (x = x_init; x <= data->exposed_rect.x + data->exposed_rect.width;
			 x += data->scale)
	{
		for (y = y_init; y <= data->exposed_rect.y + data->exposed_rect.height;
			 y += data->scale)
		{
			cairo_move_to (data->cr, x, y);
			cairo_line_to (data->cr, x, y);
		}
	}
	cairo_stroke (data->cr);
}

static void
draw_diagram (GtkWidget *widget, DrawData *data)
{
	GSList *objects;

	if (!data->self->priv->diagram)
		return;

	cairo_save (data->cr);

	cairo_set_source_rgb (data->cr, 0, 0, 0);
	cairo_set_line_width (data->cr, 1 / data->scale);

	/* Draw objects from the diagram. */
	objects = ld_diagram_get_objects (data->self->priv->diagram);
	g_slist_foreach (objects, (GFunc) draw_object, data);

	switch (data->self->priv->operation)
	{
		AddObjectData *op_data;

	case OPER_ADD_OBJECT:
		op_data = &OPER_DATA (data->self, add_object);
		if (op_data->visible)
			draw_object (op_data->object, data);
		break;
	}

	cairo_restore (data->cr);
}

static void
draw_object (LdDiagramObject *diagram_object, DrawData *data)
{
	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (diagram_object));
	g_return_if_fail (data != NULL);

	if (LD_IS_DIAGRAM_SYMBOL (diagram_object))
		draw_symbol (LD_DIAGRAM_SYMBOL (diagram_object), data);
}

static void
draw_symbol (LdDiagramSymbol *diagram_symbol, DrawData *data)
{
	LdSymbol *symbol;
	gdouble x, y, width, height;

	symbol = resolve_diagram_symbol (data->self, diagram_symbol);

	/* TODO: Resolve this better; draw a cross or whatever. */
	if (!symbol)
	{
		g_warning ("Cannot find symbol %s in the library.",
			ld_diagram_symbol_get_class (diagram_symbol));
		return;
	}

	if (!get_symbol_clip_area_on_widget (data->self, diagram_symbol,
		&x, &y, &width, &height))
		return;
	if    (x > data->exposed_rect.x + data->exposed_rect.width
		|| y > data->exposed_rect.y + data->exposed_rect.height
		|| x + width  < data->exposed_rect.x
		|| y + height < data->exposed_rect.y)
		return;

	cairo_save (data->cr);

	cairo_rectangle (data->cr, x, y, width, height);
	cairo_clip (data->cr);

	/* TODO: Rotate the space for other orientations. */
	ld_canvas_diagram_to_widget_coords (data->self,
		ld_diagram_object_get_x (LD_DIAGRAM_OBJECT (diagram_symbol)),
		ld_diagram_object_get_y (LD_DIAGRAM_OBJECT (diagram_symbol)),
		&x, &y);
	cairo_translate (data->cr, x, y);
	cairo_scale (data->cr, data->scale, data->scale);
	ld_symbol_draw (symbol, data->cr);

	cairo_restore (data->cr);
}
