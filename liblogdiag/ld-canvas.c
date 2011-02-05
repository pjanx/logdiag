/*
 * ld-canvas.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010 - 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <math.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-canvas
 * @short_description: A canvas
 * @see_also: #LdDiagram
 *
 * #LdCanvas displays and enables the user to manipulate with an #LdDiagram.
 */

/* Milimetres per inch. */
#define MM_PER_INCH 25.4
/* The default screen resolution in DPI units. */
#define DEFAULT_SCREEN_RESOLUTION 96

/* The maximal, minimal and default values of zoom. */
#define ZOOM_MIN 0.01
#define ZOOM_MAX 100
#define ZOOM_DEFAULT 1
/* Multiplication factor for zooming. */
#define ZOOM_STEP 1.4

/* When drawing is requested, extend all sides of
 * the rectangle to be drawn by this number of pixels.
 */
#define QUEUE_DRAW_EXTEND 3
/* Cursor tolerance for object borders. */
#define OBJECT_BORDER_TOLERANCE 3
/* Tolerance on all sides of symbols for strokes. */
#define SYMBOL_CLIP_TOLERANCE 5

/* Size of a highlighted terminal. */
#define TERMINAL_RADIUS 5
/* Tolerance around terminal points. */
#define TERMINAL_HOVER_TOLERANCE 8

/*
 * OperationEnd:
 *
 * Called upon ending an operation.
 */
typedef void (*OperationEnd) (LdCanvas *self);

enum
{
	OPER_0,
	OPER_ADD_OBJECT,
	OPER_SELECT,
	OPER_MOVE_SELECTION
};

typedef struct _AddObjectData AddObjectData;
typedef struct _SelectData SelectData;
typedef struct _MoveSelectionData MoveSelectionData;

struct _AddObjectData
{
	LdDiagramObject *object;
	gboolean visible;
};

struct _SelectData
{
	LdPoint drag_last_pos;
};

struct _MoveSelectionData
{
	LdPoint move_origin;
};

enum
{
	COLOR_BASE,
	COLOR_GRID,
	COLOR_OBJECT,
	COLOR_SELECTION,
	COLOR_TERMINAL,
	COLOR_COUNT
};

typedef struct _LdCanvasColor LdCanvasColor;

struct _LdCanvasColor
{
	gdouble r;
	gdouble g;
	gdouble b;
	gdouble a;
};

/*
 * LdCanvasPrivate:
 * @diagram: a diagram object assigned to this canvas as a model.
 * @library: a library object assigned to this canvas as a model.
 * @adjustment_h: an adjustment object for the horizontal axis, if any.
 * @adjustment_v: an adjustment object for the vertical axis, if any.
 * @x: the X coordinate of the center of view.
 * @y: the Y coordinate of the center of view.
 * @zoom: the current zoom of the canvas.
 * @terminal: position of the highlighted terminal.
 * @terminal_highlighted: whether a terminal is highlighted.
 * @drag_start_pos: position of the mouse pointer when dragging started.
 * @drag_operation: the operation to start when dragging starts.
 * @operation: the current operation.
 * @operation_data: data related to the current operation.
 * @operation_end: a callback to end the operation.
 * @palette: colors used by the widget.
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

	LdPoint terminal;
	gboolean terminal_highlighted;

	LdPoint drag_start_pos;
	gint drag_operation;

	gint operation;
	union
	{
		AddObjectData add_object;
		SelectData select;
		MoveSelectionData move_selection;
	}
	operation_data;
	OperationEnd operation_end;

	LdCanvasColor palette[COLOR_COUNT];
};

#define OPER_DATA(self, member) ((self)->priv->operation_data.member)
#define COLOR_GET(self, name) (&(self)->priv->palette[name])

/*
 * DrawData:
 * @self: our #LdCanvas.
 * @cr: a cairo context to draw on.
 * @exposed_rect: the area that is to be redrawn.
 * @scale: computed size of one diagram unit in pixels.
 */
typedef struct _DrawData DrawData;

struct _DrawData
{
	LdCanvas *self;
	cairo_t *cr;
	LdRectangle exposed_rect;
	gdouble scale;
};

enum
{
	PROP_0,
	PROP_DIAGRAM,
	PROP_LIBRARY,
	PROP_ZOOM
};

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
static void update_adjustments (LdCanvas *self);
static void ld_canvas_real_move (LdCanvas *self, gdouble dx, gdouble dy);

static void diagram_connect_signals (LdCanvas *self);
static void diagram_disconnect_signals (LdCanvas *self);

static gdouble ld_canvas_get_base_unit_in_px (GtkWidget *self);
static gdouble ld_canvas_get_scale_in_px (LdCanvas *self);

static void ld_canvas_color_set (LdCanvasColor *color,
	gdouble r, gdouble g, gdouble b, gdouble a);
static void ld_canvas_color_apply (LdCanvasColor *color, cairo_t *cr);

static void move_selection (LdCanvas *self, gdouble dx, gdouble dy);
static void move_object_to_coords (LdCanvas *self, LdDiagramObject *object,
	gdouble x, gdouble y);
static LdDiagramObject *get_object_at_coords (LdCanvas *self,
	gdouble x, gdouble y);
static gboolean is_object_selected (LdCanvas *self, LdDiagramObject *object);
static LdSymbol *resolve_diagram_symbol (LdCanvas *self,
	LdDiagramSymbol *diagram_symbol);
static gboolean get_symbol_area (LdCanvas *self, LdDiagramSymbol *symbol,
	LdRectangle *rect);
static gboolean get_symbol_clip_area (LdCanvas *self, LdDiagramSymbol *symbol,
	LdRectangle *rect);
static gboolean get_object_area (LdCanvas *self, LdDiagramObject *object,
	LdRectangle *rect);
static gboolean object_hit_test (LdCanvas *self, LdDiagramObject *object,
	gdouble x, gdouble y);
static void check_terminals (LdCanvas *self, gdouble x, gdouble y);
static void hide_terminals (LdCanvas *self);
static void queue_draw (LdCanvas *self, LdRectangle *rect);
static void queue_object_draw (LdCanvas *self, LdDiagramObject *object);
static void queue_terminal_draw (LdCanvas *self, LdPoint *terminal);

static void ld_canvas_real_cancel_operation (LdCanvas *self);
static void oper_add_object_end (LdCanvas *self);

static void oper_select_begin (LdCanvas *self, gdouble x, gdouble y);
static void oper_select_end (LdCanvas *self);
static void oper_select_get_rectangle (LdCanvas *self, LdRectangle *rect);
static void oper_select_queue_draw (LdCanvas *self);
static void oper_select_draw (GtkWidget *widget, DrawData *data);
static void oper_select_motion (LdCanvas *self, gdouble x, gdouble y);

static void oper_move_selection_begin (LdCanvas *self, gdouble x, gdouble y);
static void oper_move_selection_end (LdCanvas *self);
static void oper_move_selection_motion (LdCanvas *self, gdouble x, gdouble y);

static void simulate_motion (LdCanvas *self);
static gboolean on_motion_notify (GtkWidget *widget, GdkEventMotion *event,
	gpointer user_data);
static gboolean on_leave_notify (GtkWidget *widget, GdkEventCrossing *event,
	gpointer user_data);
static gboolean on_button_press (GtkWidget *widget, GdkEventButton *event,
	gpointer user_data);
static gboolean on_button_release (GtkWidget *widget, GdkEventButton *event,
	gpointer user_data);
static gboolean on_scroll (GtkWidget *widget, GdkEventScroll *event,
	gpointer user_data);

static gboolean on_expose_event (GtkWidget *widget, GdkEventExpose *event,
	gpointer user_data);
static void draw_grid (GtkWidget *widget, DrawData *data);
static void draw_diagram (GtkWidget *widget, DrawData *data);
static void draw_terminal (GtkWidget *widget, DrawData *data);
static void draw_object (LdDiagramObject *diagram_object, DrawData *data);
static void draw_symbol (LdDiagramSymbol *diagram_symbol, DrawData *data);


G_DEFINE_TYPE (LdCanvas, ld_canvas, GTK_TYPE_DRAWING_AREA);

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
	klass->move = ld_canvas_real_move;

	binding_set = gtk_binding_set_by_class (klass);
	gtk_binding_entry_add_signal (binding_set, GDK_Escape, 0,
		"cancel-operation", 0);
	gtk_binding_entry_add_signal (binding_set, GDK_Left, 0,
		"move", 2, G_TYPE_DOUBLE, (gdouble) -1, G_TYPE_DOUBLE, (gdouble) 0);
	gtk_binding_entry_add_signal (binding_set, GDK_Right, 0,
		"move", 2, G_TYPE_DOUBLE, (gdouble) 1, G_TYPE_DOUBLE, (gdouble) 0);
	gtk_binding_entry_add_signal (binding_set, GDK_Up, 0,
		"move", 2, G_TYPE_DOUBLE, (gdouble) 0, G_TYPE_DOUBLE, (gdouble) -1);
	gtk_binding_entry_add_signal (binding_set, GDK_Down, 0,
		"move", 2, G_TYPE_DOUBLE, (gdouble) 0, G_TYPE_DOUBLE, (gdouble) 1);

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
 * LdCanvas:zoom:
 *
 * The zoom of this canvas.
 */
	pspec = g_param_spec_double ("zoom", "Zoom",
		"The zoom of this canvas.",
		ZOOM_MIN, ZOOM_MAX, ZOOM_DEFAULT, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_ZOOM, pspec);

/**
 * LdCanvas::set-scroll-adjustments:
 * @self: an #LdCanvas object.
 * @horizontal: the horizontal #GtkAdjustment.
 * @vertical: the vertical #GtkAdjustment.
 *
 * Set scroll adjustments for the canvas.
 */
	widget_class->set_scroll_adjustments_signal = g_signal_new
		("set-scroll-adjustments", G_TYPE_FROM_CLASS (widget_class),
		G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (LdCanvasClass, set_scroll_adjustments),
		NULL, NULL,
		ld_marshal_VOID__OBJECT_OBJECT,
		G_TYPE_NONE, 2, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);

/**
 * LdCanvas::cancel-operation:
 * @self: an #LdCanvas object.
 *
 * Cancel any current operation.
 */
	klass->cancel_operation_signal = g_signal_new
		("cancel-operation", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (LdCanvasClass, cancel_operation), NULL, NULL,
		g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);

/**
 * LdCanvas::move:
 * @self: an #LdCanvas object.
 * @dx: The difference by which to move on the horizontal axis.
 * @dy: The difference by which to move on the vertical axis.
 *
 * Move the selection, if any, or the document.
 */
	klass->move_signal = g_signal_new
		("move", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (LdCanvasClass, move), NULL, NULL,
		ld_marshal_VOID__DOUBLE_DOUBLE,
		G_TYPE_NONE, 2, G_TYPE_DOUBLE, G_TYPE_DOUBLE);

	g_type_class_add_private (klass, sizeof (LdCanvasPrivate));
}

static void
ld_canvas_init (LdCanvas *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_CANVAS, LdCanvasPrivate);

	self->priv->x = 0;
	self->priv->y = 0;
	self->priv->zoom = ZOOM_DEFAULT;

	ld_canvas_color_set (COLOR_GET (self, COLOR_BASE), 1, 1, 1, 1);
	ld_canvas_color_set (COLOR_GET (self, COLOR_GRID), 0.5, 0.5, 0.5, 1);
	ld_canvas_color_set (COLOR_GET (self, COLOR_OBJECT), 0, 0, 0, 1);
	ld_canvas_color_set (COLOR_GET (self, COLOR_SELECTION), 0, 0, 1, 1);
	ld_canvas_color_set (COLOR_GET (self, COLOR_TERMINAL), 1, 0.5, 0.5, 1);

	g_signal_connect (self, "size-allocate",
		G_CALLBACK (on_size_allocate), NULL);
	g_signal_connect (self, "expose-event",
		G_CALLBACK (on_expose_event), NULL);

	g_signal_connect (self, "motion-notify-event",
		G_CALLBACK (on_motion_notify), NULL);
	g_signal_connect (self, "leave-notify-event",
		G_CALLBACK (on_leave_notify), NULL);
	g_signal_connect (self, "button-press-event",
		G_CALLBACK (on_button_press), NULL);
	g_signal_connect (self, "button-release-event",
		G_CALLBACK (on_button_release), NULL);
	g_signal_connect (self, "scroll-event",
		G_CALLBACK (on_scroll), NULL);

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
	{
		diagram_disconnect_signals (self);
		g_object_unref (self->priv->diagram);
	}
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
	case PROP_ZOOM:
		g_value_set_double (value, ld_canvas_get_zoom (self));
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
	case PROP_ZOOM:
		ld_canvas_set_zoom (self, g_value_get_double (value));
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

	self = LD_CANVAS (widget);

	/* FIXME: If the new allocation is bigger, we may see more than
	 *        what we're supposed to be able to see -> adjust X and Y.
	 *
	 *        If the visible area is so large that we simply must see more,
	 *        let's disable the scrollbars in question.
	 */
	update_adjustments (self);
}

static void
update_adjustments (LdCanvas *self)
{
	gdouble scale;

	scale = ld_canvas_get_scale_in_px (self);

	if (self->priv->adjustment_h)
	{
		self->priv->adjustment_h->page_size
			= GTK_WIDGET (self)->allocation.width  / scale;
		self->priv->adjustment_h->value
			= self->priv->x - self->priv->adjustment_h->page_size / 2;
		gtk_adjustment_changed (self->priv->adjustment_h);
	}
	if (self->priv->adjustment_v)
	{
		self->priv->adjustment_v->page_size
			= GTK_WIDGET (self)->allocation.height / scale;
		self->priv->adjustment_v->value
			= self->priv->y - self->priv->adjustment_v->page_size / 2;
		gtk_adjustment_changed (self->priv->adjustment_v);
	}
}

static void
ld_canvas_real_move (LdCanvas *self, gdouble dx, gdouble dy)
{
	LdDiagram *diagram;

	diagram = self->priv->diagram;
	if (!diagram)
		return;

	/* TODO: Check/move boundaries, also implement normal
	 *       getters and setters for priv->x and priv->y.
	 */
	if (ld_diagram_get_selection (diagram))
		move_selection (self, dx, dy);
	else
	{
		self->priv->x += dx;
		self->priv->y += dy;

		simulate_motion (self);
		update_adjustments (self);
	}
	gtk_widget_queue_draw (GTK_WIDGET (self));
}


/* ===== Generic interface etc. ============================================ */

/**
 * ld_canvas_new:
 *
 * Create an instance.
 */
GtkWidget *
ld_canvas_new (void)
{
	return g_object_new (LD_TYPE_CANVAS, NULL);
}

/**
 * ld_canvas_set_diagram:
 * @self: an #LdCanvas object.
 * @diagram: the #LdDiagram to be assigned to the canvas.
 *
 * Assign an #LdDiagram object to the canvas.
 */
void
ld_canvas_set_diagram (LdCanvas *self, LdDiagram *diagram)
{
	g_return_if_fail (LD_IS_CANVAS (self));
	g_return_if_fail (LD_IS_DIAGRAM (diagram));

	if (self->priv->diagram)
	{
		diagram_disconnect_signals (self);
		g_object_unref (self->priv->diagram);
	}

	self->priv->diagram = diagram;
	diagram_connect_signals (self);
	g_object_ref (diagram);

	g_object_notify (G_OBJECT (self), "diagram");
}

/**
 * ld_canvas_get_diagram:
 * @self: an #LdCanvas object.
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

static void
diagram_connect_signals (LdCanvas *self)
{
	g_return_if_fail (LD_IS_DIAGRAM (self->priv->diagram));

	g_signal_connect_swapped (self->priv->diagram, "changed",
		G_CALLBACK (gtk_widget_queue_draw), self);
	g_signal_connect_swapped (self->priv->diagram, "selection-changed",
		G_CALLBACK (gtk_widget_queue_draw), self);
}

static void
diagram_disconnect_signals (LdCanvas *self)
{
	g_return_if_fail (LD_IS_DIAGRAM (self->priv->diagram));

	g_signal_handlers_disconnect_matched (self->priv->diagram,
		G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA, 0, 0, NULL,
		gtk_widget_queue_draw, self);
}

/**
 * ld_canvas_set_library:
 * @self: an #LdCanvas object.
 * @library: the #LdLibrary to be assigned to the canvas.
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
 * @self: an #LdCanvas object.
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
 * @self: a #GtkWidget object to retrieve DPI from (indirectly).
 *
 * Return value: length of the base unit in pixels.
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
 * @self: an #LdCanvas object.
 *
 * Return value: displayed length of the base unit in pixels.
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
 * @self: an #LdCanvas object.
 * @wx: the X coordinate to be translated.
 * @wy: the Y coordinate to be translated.
 * @dx: (out): the translated X coordinate.
 * @dy: (out): the translated Y coordinate.
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
 * @self: an #LdCanvas object.
 * @dx: the X coordinate to be translated.
 * @dy: the Y coordinate to be translated.
 * @wx: (out): the translated X coordinate.
 * @wy: (out): the translated Y coordinate.
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

/**
 * ld_canvas_get_zoom:
 * @self: an #LdCanvas object.
 *
 * Return value: zoom of the canvas.
 */
gdouble
ld_canvas_get_zoom (LdCanvas *self)
{
	g_return_val_if_fail (LD_IS_CANVAS (self), -1);
	return self->priv->zoom;
}

/**
 * ld_canvas_set_zoom:
 * @self: an #LdCanvas object.
 * @zoom: the zoom.
 *
 * Set zoom of the canvas.
 */
void
ld_canvas_set_zoom (LdCanvas *self, gdouble zoom)
{
	gdouble clamped_zoom;

	g_return_if_fail (LD_IS_CANVAS (self));

	clamped_zoom = CLAMP (zoom, ZOOM_MIN, ZOOM_MAX);
	if (self->priv->zoom == clamped_zoom)
		return;

	self->priv->zoom = clamped_zoom;

	simulate_motion (self);
	update_adjustments (self);
	gtk_widget_queue_draw (GTK_WIDGET (self));

	g_object_notify (G_OBJECT (self), "zoom");
}

/**
 * ld_canvas_can_zoom_in:
 * @self: an #LdCanvas object.
 *
 * Return value: %TRUE if the view can be zoomed in.
 */
gboolean
ld_canvas_can_zoom_in (LdCanvas *self)
{
	g_return_val_if_fail (LD_IS_CANVAS (self), FALSE);
	return self->priv->zoom < ZOOM_MAX;
}

/**
 * ld_canvas_can_zoom_out:
 * @self: an #LdCanvas object.
 *
 * Return value: %TRUE if the view can be zoomed out.
 */
gboolean
ld_canvas_can_zoom_out (LdCanvas *self)
{
	g_return_val_if_fail (LD_IS_CANVAS (self), FALSE);
	return self->priv->zoom > ZOOM_MIN;
}

/**
 * ld_canvas_zoom_in:
 * @self: an #LdCanvas object.
 *
 * Zoom the view in.
 */
void
ld_canvas_zoom_in (LdCanvas *self)
{
	g_return_if_fail (LD_IS_CANVAS (self));
	ld_canvas_set_zoom (self, self->priv->zoom * ZOOM_STEP);
}

/**
 * ld_canvas_zoom_out:
 * @self: an #LdCanvas object.
 *
 * Zoom the view out.
 */
void
ld_canvas_zoom_out (LdCanvas *self)
{
	g_return_if_fail (LD_IS_CANVAS (self));
	ld_canvas_set_zoom (self, self->priv->zoom / ZOOM_STEP);
}


/* ===== Helper functions ================================================== */

static void
ld_canvas_color_set (LdCanvasColor *color,
	gdouble r, gdouble g, gdouble b, gdouble a)
{
	color->r = r;
	color->g = g;
	color->b = b;
	color->a = a;
}

static void
ld_canvas_color_apply (LdCanvasColor *color, cairo_t *cr)
{
	cairo_set_source_rgba (cr, color->r, color->g, color->b, color->a);
}

static void
move_selection (LdCanvas *self, gdouble dx, gdouble dy)
{
	LdDiagram *diagram;
	GList *selection, *iter;

	diagram = self->priv->diagram;
	if (!diagram)
		return;

	selection = ld_diagram_get_selection (diagram);
	if (!selection)
		return;

	ld_diagram_begin_user_action (diagram);
	for (iter = selection; iter; iter = g_list_next (iter))
	{
		gdouble x, y;

		g_object_get (iter->data, "x", &x, "y", &y, NULL);
		x += dx;
		y += dy;
		g_object_set (iter->data, "x", x, "y", y, NULL);
	}
	ld_diagram_end_user_action (diagram);
}

static void
move_object_to_coords (LdCanvas *self, LdDiagramObject *object,
	gdouble x, gdouble y)
{
	gdouble dx, dy;

	ld_canvas_widget_to_diagram_coords (self, x, y, &dx, &dy);
	g_object_set (object, "x", floor (dx + 0.5), "y", floor (dy + 0.5), NULL);
}

static LdDiagramObject *
get_object_at_coords (LdCanvas *self, gdouble x, gdouble y)
{
	GList *objects, *iter;

	/* Iterate from the top object downwards. */
	objects = (GList *) ld_diagram_get_objects (self->priv->diagram);
	for (iter = g_list_last (objects); iter; iter = g_list_previous (iter))
	{
		LdDiagramObject *object;

		object = LD_DIAGRAM_OBJECT (iter->data);
		if (object_hit_test (self, object, x, y))
			return object;
	}
	return NULL;
}

static gboolean
is_object_selected (LdCanvas *self, LdDiagramObject *object)
{
	return g_list_find (ld_diagram_get_selection (self->priv->diagram),
		object) != NULL;
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
get_symbol_area (LdCanvas *self, LdDiagramSymbol *symbol, LdRectangle *rect)
{
	gdouble object_x, object_y;
	LdSymbol *library_symbol;
	LdRectangle area;
	gdouble x1, x2;
	gdouble y1, y2;

	g_object_get (symbol, "x", &object_x, "y", &object_y, NULL);

	library_symbol = resolve_diagram_symbol (self, symbol);
	if (library_symbol)
		ld_symbol_get_area (library_symbol, &area);
	else
		return FALSE;

	/* TODO: Rotate the rectangle for other orientations. */
	ld_canvas_diagram_to_widget_coords (self,
		object_x + area.x,
		object_y + area.y,
		&x1, &y1);
	ld_canvas_diagram_to_widget_coords (self,
		object_x + area.x + area.width,
		object_y + area.y + area.height,
		&x2, &y2);

	rect->x = x1;
	rect->y = y1;
	rect->width  = x2 - x1;
	rect->height = y2 - y1;
	return TRUE;
}

static gboolean
get_symbol_clip_area (LdCanvas *self, LdDiagramSymbol *symbol,
	LdRectangle *rect)
{
	LdRectangle object_rect;

	if (!get_object_area (self, LD_DIAGRAM_OBJECT (symbol), &object_rect))
		return FALSE;

	*rect = object_rect;
	ld_rectangle_extend (rect, SYMBOL_CLIP_TOLERANCE);
	return TRUE;
}

static gboolean
get_object_area (LdCanvas *self, LdDiagramObject *object, LdRectangle *rect)
{
	if (LD_IS_DIAGRAM_SYMBOL (object))
		return get_symbol_area (self, LD_DIAGRAM_SYMBOL (object), rect);
	return FALSE;
}

static gboolean
object_hit_test (LdCanvas *self, LdDiagramObject *object, gdouble x, gdouble y)
{
	LdRectangle rect;

	if (!get_object_area (self, object, &rect))
		return FALSE;
	ld_rectangle_extend (&rect, OBJECT_BORDER_TOLERANCE);
	return ld_rectangle_contains (&rect, x, y);
}

static void
check_terminals (LdCanvas *self, gdouble x, gdouble y)
{
	GList *objects, *iter;
	LdDiagramSymbol *closest_symbol = NULL;
	gdouble closest_distance = TERMINAL_HOVER_TOLERANCE;
	LdPoint closest_terminal;

	objects = (GList *) ld_diagram_get_objects (self->priv->diagram);
	for (iter = objects; iter; iter = g_list_next (iter))
	{
		LdDiagramObject *diagram_object;
		gdouble object_x, object_y;
		LdDiagramSymbol *diagram_symbol;
		LdSymbol *symbol;
		const LdPointArray *terminals;
		guint i;

		if (!LD_IS_DIAGRAM_SYMBOL (iter->data))
			continue;

		diagram_symbol = LD_DIAGRAM_SYMBOL (iter->data);
		symbol = resolve_diagram_symbol (self, diagram_symbol);
		if (!symbol)
			continue;

		diagram_object = LD_DIAGRAM_OBJECT (iter->data);
		g_object_get (diagram_object, "x", &object_x, "y", &object_y, NULL);

		terminals = ld_symbol_get_terminals (symbol);

		for (i = 0; i < terminals->length; i++)
		{
			LdPoint cur_term;
			gdouble distance;

			cur_term = terminals->points[i];
			cur_term.x += object_x;
			cur_term.y += object_y;
			ld_canvas_diagram_to_widget_coords (self,
				cur_term.x, cur_term.y, &cur_term.x, &cur_term.y);

			distance = ld_point_distance (&cur_term, x, y);
			if (distance <= closest_distance)
			{
				closest_symbol = diagram_symbol;
				closest_distance = distance;
				closest_terminal = cur_term;
			}
		}
	}

	hide_terminals (self);

	if (closest_symbol)
	{
		self->priv->terminal_highlighted = TRUE;
		self->priv->terminal = closest_terminal;
		queue_terminal_draw (self, &closest_terminal);
	}
}

static void
hide_terminals (LdCanvas *self)
{
	if (self->priv->terminal_highlighted)
	{
		self->priv->terminal_highlighted = FALSE;
		queue_terminal_draw (self, &self->priv->terminal);
	}
}

static void
queue_draw (LdCanvas *self, LdRectangle *rect)
{
	LdRectangle area;

	area = *rect;
	ld_rectangle_extend (&area, QUEUE_DRAW_EXTEND);
	gtk_widget_queue_draw_area (GTK_WIDGET (self),
		area.x, area.y, area.width, area.height);
}

static void
queue_object_draw (LdCanvas *self, LdDiagramObject *object)
{
	if (LD_IS_DIAGRAM_SYMBOL (object))
	{
		LdRectangle rect;

		if (!get_symbol_clip_area (self, LD_DIAGRAM_SYMBOL (object), &rect))
			return;
		queue_draw (self, &rect);
	}
}

static void
queue_terminal_draw (LdCanvas *self, LdPoint *terminal)
{
	LdRectangle rect;

	rect.x = terminal->x - TERMINAL_RADIUS;
	rect.y = terminal->y - TERMINAL_RADIUS;
	rect.width  = 2 * TERMINAL_RADIUS;
	rect.height = 2 * TERMINAL_RADIUS;
	queue_draw (self, &rect);
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
 * @self: an #LdCanvas object.
 * @object: (transfer full): the object to be added to the diagram.
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
	self->priv->operation_end = oper_add_object_end;

	data = &OPER_DATA (self, add_object);
	data->object = object;
}

static void
oper_add_object_end (LdCanvas *self)
{
	AddObjectData *data;

	data = &OPER_DATA (self, add_object);
	if (data->object)
	{
		queue_object_draw (self, data->object);
		g_object_unref (data->object);
		data->object = NULL;
	}
}

static void
oper_select_begin (LdCanvas *self, gdouble x, gdouble y)
{
	SelectData *data;

	ld_canvas_real_cancel_operation (self);

	self->priv->operation = OPER_SELECT;
	self->priv->operation_end = oper_select_end;

	data = &OPER_DATA (self, select);
	data->drag_last_pos.x = self->priv->drag_start_pos.x;
	data->drag_last_pos.y = self->priv->drag_start_pos.y;

	oper_select_motion (self, x, y);
}

static void
oper_select_end (LdCanvas *self)
{
	oper_select_queue_draw (self);
}

static void
oper_select_get_rectangle (LdCanvas *self, LdRectangle *rect)
{
	SelectData *data;

	data = &OPER_DATA (self, select);
	rect->x = MIN (self->priv->drag_start_pos.x, data->drag_last_pos.x);
	rect->y = MIN (self->priv->drag_start_pos.y, data->drag_last_pos.y);
	rect->width  = ABS (self->priv->drag_start_pos.x - data->drag_last_pos.x);
	rect->height = ABS (self->priv->drag_start_pos.y - data->drag_last_pos.y);
}

static void
oper_select_queue_draw (LdCanvas *self)
{
	LdRectangle rect;
	SelectData *data;

	data = &OPER_DATA (self, select);
	oper_select_get_rectangle (self, &rect);
	queue_draw (self, &rect);
}

static void
oper_select_draw (GtkWidget *widget, DrawData *data)
{
	static const double dashes[] = {3, 5};
	SelectData *select_data;

	g_return_if_fail (data->self->priv->operation == OPER_SELECT);

	ld_canvas_color_apply (COLOR_GET (data->self, COLOR_GRID), data->cr);
	cairo_set_line_width (data->cr, 1);
	cairo_set_line_cap (data->cr, CAIRO_LINE_CAP_SQUARE);
	cairo_set_dash (data->cr, dashes, G_N_ELEMENTS (dashes), 0);

	select_data = &OPER_DATA (data->self, select);

	cairo_rectangle (data->cr,
		data->self->priv->drag_start_pos.x - 0.5,
		data->self->priv->drag_start_pos.y - 0.5,
		select_data->drag_last_pos.x - data->self->priv->drag_start_pos.x + 1,
		select_data->drag_last_pos.y - data->self->priv->drag_start_pos.y + 1);
	cairo_stroke (data->cr);
}

static void
oper_select_motion (LdCanvas *self, gdouble x, gdouble y)
{
	SelectData *data;
	GList *objects, *iter;
	LdRectangle selection_rect, object_rect;

	data = &OPER_DATA (self, select);

	oper_select_queue_draw (self);
	data->drag_last_pos.x = x;
	data->drag_last_pos.y = y;
	oper_select_queue_draw (self);

	oper_select_get_rectangle (self, &selection_rect);
	objects = (GList *) ld_diagram_get_objects (self->priv->diagram);

	for (iter = objects; iter; iter = g_list_next (iter))
	{
		LdDiagramObject *object;

		object = LD_DIAGRAM_OBJECT (iter->data);
		if (!get_object_area (self, object, &object_rect))
			continue;

		ld_rectangle_extend (&object_rect, OBJECT_BORDER_TOLERANCE);
		if (ld_rectangle_intersects (&object_rect, &selection_rect))
			ld_diagram_select (self->priv->diagram, object);
		else
			ld_diagram_unselect (self->priv->diagram, object);
	}
}

static void
oper_move_selection_begin (LdCanvas *self, gdouble x, gdouble y)
{
	MoveSelectionData *data;

	ld_canvas_real_cancel_operation (self);

	self->priv->operation = OPER_MOVE_SELECTION;
	self->priv->operation_end = oper_move_selection_end;

	ld_diagram_begin_user_action (self->priv->diagram);

	data = &OPER_DATA (self, move_selection);
	data->move_origin.x = self->priv->drag_start_pos.x;
	data->move_origin.y = self->priv->drag_start_pos.y;

	oper_move_selection_motion (self, x, y);
}

static void
oper_move_selection_end (LdCanvas *self)
{
	ld_diagram_end_user_action (self->priv->diagram);
}

static void
oper_move_selection_motion (LdCanvas *self, gdouble x, gdouble y)
{
	MoveSelectionData *data;
	gdouble scale, dx, dy, move_x, move_y;
	gdouble move = FALSE;

	scale = ld_canvas_get_scale_in_px (self);
	data = &OPER_DATA (self, move_selection);

	dx = x - data->move_origin.x;
	dy = y - data->move_origin.y;

	move_x = dx < 0 ? ceil (dx / scale) : floor (dx / scale);
	move_y = dy < 0 ? ceil (dy / scale) : floor (dy / scale);

	if (ABS (move_x) >= 1)
	{
		data->move_origin.x += move_x * scale;
		move = TRUE;
	}
	if (ABS (move_y) >= 1)
	{
		data->move_origin.y += move_y * scale;
		move = TRUE;
	}

	if (move)
		move_selection (self, move_x, move_y);
}


/* ===== Events, rendering ================================================= */

static void
simulate_motion (LdCanvas *self)
{
	GdkEventMotion event;
	GtkWidget *widget;
	gint x, y;
	GdkModifierType state;

	widget = GTK_WIDGET (self);

	if (gdk_window_get_pointer (widget->window, &x, &y, &state)
		!= widget->window)
		return;

	memset (&event, 0, sizeof (event));
	event.type = GDK_MOTION_NOTIFY;
	event.window = widget->window;
	event.x = x;
	event.y = y;
	event.state = state;

	on_motion_notify (widget, &event, NULL);
}

static gboolean
on_motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	LdCanvas *self;
	AddObjectData *add_data;

	self = LD_CANVAS (widget);
	switch (self->priv->operation)
	{
	case OPER_ADD_OBJECT:
		add_data = &OPER_DATA (self, add_object);
		add_data->visible = TRUE;

		queue_object_draw (self, add_data->object);
		move_object_to_coords (self, add_data->object, event->x, event->y);
		queue_object_draw (self, add_data->object);
		break;
	case OPER_SELECT:
		oper_select_motion (self, event->x, event->y);
		break;
	case OPER_MOVE_SELECTION:
		oper_move_selection_motion (self, event->x, event->y);
		break;
	case OPER_0:
		if (event->state & GDK_BUTTON1_MASK
			&& (event->x != self->priv->drag_start_pos.x
			|| event->y != self->priv->drag_start_pos.y))
		{
			switch (self->priv->drag_operation)
			{
			case OPER_SELECT:
				oper_select_begin (self, event->x, event->y);
				break;
			case OPER_MOVE_SELECTION:
				oper_move_selection_begin (self, event->x, event->y);
				break;
			}
		}
		check_terminals (self, event->x, event->y);
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

		queue_object_draw (self, data->object);
		break;
	}
	return FALSE;
}

static gboolean
on_button_press (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	LdCanvas *self;
	AddObjectData *data;
	LdDiagramObject *object;

	if (event->button != 1)
		return FALSE;
	if (!gtk_widget_has_focus (widget))
		gtk_widget_grab_focus (widget);

	self = LD_CANVAS (widget);
	if (!self->priv->diagram)
		return FALSE;

	self->priv->drag_operation = OPER_0;
	switch (self->priv->operation)
	{
	case OPER_ADD_OBJECT:
		data = &OPER_DATA (self, add_object);

		queue_object_draw (self, data->object);
		move_object_to_coords (self, data->object, event->x, event->y);
		ld_diagram_insert_object (self->priv->diagram, data->object, -1);

		/* XXX: "cancel" causes confusion. */
		ld_canvas_real_cancel_operation (self);
		break;
	case OPER_0:
		self->priv->drag_start_pos.x = event->x;
		self->priv->drag_start_pos.y = event->y;

		object = get_object_at_coords (self, event->x, event->y);
		if (!object)
		{
			ld_diagram_unselect_all (self->priv->diagram);
			self->priv->drag_operation = OPER_SELECT;
		}
		else if (!is_object_selected (self, object))
		{
			if (event->state != GDK_SHIFT_MASK)
				ld_diagram_unselect_all (self->priv->diagram);
			ld_diagram_select (self->priv->diagram, object);
			self->priv->drag_operation = OPER_MOVE_SELECTION;
		}
		else
			self->priv->drag_operation = OPER_MOVE_SELECTION;
		break;
	}
	return FALSE;
}

static gboolean
on_button_release (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	LdCanvas *self;
	LdDiagramObject *object;

	if (event->button != 1)
		return FALSE;

	self = LD_CANVAS (widget);
	if (!self->priv->diagram)
		return FALSE;

	switch (self->priv->operation)
	{
	case OPER_SELECT:
	case OPER_MOVE_SELECTION:
		ld_canvas_real_cancel_operation (self);
		break;
	case OPER_0:
		object = get_object_at_coords (self, event->x, event->y);
		if (object && is_object_selected (self, object))
		{
			if (!(event->state & GDK_SHIFT_MASK))
				ld_diagram_unselect_all (self->priv->diagram);
			ld_diagram_select (self->priv->diagram, object);
		}
		break;
	}
	return FALSE;
}

static gboolean
on_scroll (GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
	gdouble prev_x, prev_y;
	gdouble new_x, new_y;
	LdCanvas *self;

	self = LD_CANVAS (widget);

	ld_canvas_widget_to_diagram_coords (self,
		event->x, event->y, &prev_x, &prev_y);

	switch (event->direction)
	{
	case GDK_SCROLL_UP:
		ld_canvas_zoom_in (self);
		break;
	case GDK_SCROLL_DOWN:
		ld_canvas_zoom_out (self);
		break;
	default:
		return FALSE;
	}

	ld_canvas_widget_to_diagram_coords (self,
		event->x, event->y, &new_x, &new_y);

	/* Focus on the point under the cursor. */
	self->priv->x += prev_x - new_x;
	self->priv->y += prev_y - new_y;

	check_terminals (self, event->x, event->y);
	return TRUE;
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

	ld_canvas_color_apply (COLOR_GET (data.self, COLOR_BASE), data.cr);
	cairo_paint (data.cr);

	draw_grid (widget, &data);
	draw_diagram (widget, &data);
	draw_terminal (widget, &data);

	if (data.self->priv->operation == OPER_SELECT)
		oper_select_draw (widget, &data);

	cairo_destroy (data.cr);
	return FALSE;
}

static void
draw_grid (GtkWidget *widget, DrawData *data)
{
	gdouble grid_step;
	gint grid_factor;
	gdouble x_init, y_init;
	gdouble x, y;
	gdouble x_round, y_round;

	grid_step = data->scale;
	grid_factor = 1;
	while (grid_step < 5)
	{
		grid_step *= 5;
		grid_factor *= 5;
	}

	ld_canvas_color_apply (COLOR_GET (data->self, COLOR_GRID), data->cr);
	cairo_set_line_width (data->cr, 1);
	cairo_set_line_cap (data->cr, CAIRO_LINE_CAP_ROUND);

	/* Get coordinates of the top-left point. */
	ld_canvas_widget_to_diagram_coords (data->self,
		data->exposed_rect.x, data->exposed_rect.y, &x_init, &y_init);

	x_init = ceil (x_init);
	x_init = x_init - (gint) x_init % grid_factor;
	y_init = ceil (y_init);
	y_init = y_init - (gint) y_init % grid_factor;

	ld_canvas_diagram_to_widget_coords (data->self,
		x_init, y_init, &x_init, &y_init);

	/* Iterate over all the points. */
	for     (x = x_init; x <= data->exposed_rect.x + data->exposed_rect.width;
			 x += grid_step)
	{
		for (y = y_init; y <= data->exposed_rect.y + data->exposed_rect.height;
			 y += grid_step)
		{
			x_round = floor (x) + 0.5;
			y_round = floor (y) + 0.5;
			cairo_move_to (data->cr, x_round, y_round);
			cairo_line_to (data->cr, x_round, y_round);
		}
	}
	cairo_stroke (data->cr);
}

static void
draw_terminal (GtkWidget *widget, DrawData *data)
{
	LdCanvasPrivate *priv;

	priv = data->self->priv;
	if (!priv->terminal_highlighted)
		return;

	ld_canvas_color_apply (COLOR_GET (data->self, COLOR_TERMINAL), data->cr);
	cairo_set_line_width (data->cr, 1);

	cairo_new_path (data->cr);
	cairo_arc (data->cr, priv->terminal.x, priv->terminal.y,
		TERMINAL_RADIUS, 0, 2 * G_PI);
	cairo_stroke (data->cr);
}

static void
draw_diagram (GtkWidget *widget, DrawData *data)
{
	GList *objects, *iter;

	if (!data->self->priv->diagram)
		return;

	cairo_save (data->cr);
	cairo_set_line_width (data->cr, 1 / data->scale);

	/* Draw objects from the diagram, from bottom to top. */
	objects = (GList *) ld_diagram_get_objects (data->self->priv->diagram);
	for (iter = objects; iter; iter = g_list_next (iter))
		draw_object (LD_DIAGRAM_OBJECT (iter->data), data);

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

	if (is_object_selected (data->self, diagram_object))
		ld_canvas_color_apply (COLOR_GET (data->self,
			COLOR_SELECTION), data->cr);
	else
		ld_canvas_color_apply (COLOR_GET (data->self,
			COLOR_OBJECT), data->cr);

	if (LD_IS_DIAGRAM_SYMBOL (diagram_object))
		draw_symbol (LD_DIAGRAM_SYMBOL (diagram_object), data);
}

static void
draw_symbol (LdDiagramSymbol *diagram_symbol, DrawData *data)
{
	LdSymbol *symbol;
	LdRectangle clip_rect;
	gdouble x, y;

	symbol = resolve_diagram_symbol (data->self, diagram_symbol);

	/* TODO: Resolve this better; draw a cross or whatever. */
	if (!symbol)
	{
		g_warning ("cannot find symbol `%s' in the library",
			ld_diagram_symbol_get_class (diagram_symbol));
		return;
	}

	if (!get_symbol_clip_area (data->self, diagram_symbol, &clip_rect)
		|| !ld_rectangle_intersects (&clip_rect, &data->exposed_rect))
		return;

	cairo_save (data->cr);

	cairo_rectangle (data->cr, clip_rect.x, clip_rect.y,
		clip_rect.width, clip_rect.height);
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
