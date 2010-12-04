/*
 * ld-canvas.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

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

/*
 * LdCanvasPrivate:
 * @document: A document object assigned to this canvas as a model.
 */
struct _LdCanvasPrivate
{
	LdDocument *document;
	LdLibrary *library;
};

G_DEFINE_TYPE (LdCanvas, ld_canvas, GTK_TYPE_DRAWING_AREA);

enum
{
	PROP_0,
	PROP_DOCUMENT,
	PROP_LIBRARY
};

static void
ld_canvas_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);

static void
ld_canvas_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);

static void
ld_canvas_finalize (GObject *gobject);


static gboolean
on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer user_data);

static void
draw_grid (GtkWidget *widget, cairo_t *cr);

static void
canvas_paint (GtkWidget *widget, cairo_t *cr);


static void
ld_canvas_class_init (LdCanvasClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GParamSpec *pspec;

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

	widget_class = GTK_WIDGET_CLASS (klass);

/* TODO: Scrolling support; make the comment bellow a gtk-doc comment then. */
/*
 * LdCanvas::set-scroll-adjustments:
 * @canvas: The canvas object.
 *
 * Contents of the library have changed.
 */
/*
	widget_class->set_scroll_adjustments_signal = g_signal_new
		("set-scroll-adjustments", G_TYPE_FROM_CLASS (widget_class),
		G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (LdCanvasClass, set_scroll_adjustments),
		NULL, NULL,
		g_cclosure_user_marshal_VOID__OBJECT_OBJECT,
		G_TYPE_NONE, 2, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);
*/
	g_type_class_add_private (klass, sizeof (LdCanvasPrivate));
}

static void
ld_canvas_init (LdCanvas *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_CANVAS, LdCanvasPrivate);

	g_signal_connect (self, "expose-event", G_CALLBACK (on_expose_event), NULL);
}

static void
ld_canvas_finalize (GObject *gobject)
{
	LdCanvas *self;

	self = LD_CANVAS (gobject);

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

static gboolean
on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	cairo_t *cr;

	cr = gdk_cairo_create (widget->window);

	cairo_rectangle (cr, event->area.x, event->area.y,
		event->area.width, event->area.height);
	cairo_clip (cr);

	canvas_paint (widget, cr);

	cairo_destroy (cr);

	return FALSE;
}

static void
draw_grid (GtkWidget *widget, cairo_t *cr)
{
	int x, y;

	/* Drawing points:
	 * http://lists.freedesktop.org/archives/cairo/2009-June/017459.html
	 */
	cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
	cairo_set_line_width (cr, 1);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);

	/* FIXME: Invariable grid size; it's slow because it draws whole area. */
	for (x = 0; x < widget->allocation.width; x += 20)
	{
		for (y = 0; y < widget->allocation.height; y += 20)
		{
			/* Drawing sharp lines (also applies to these dots):
			 * http://www.cairographics.org/FAQ/#sharp_lines
			 */
			cairo_move_to (cr, x + 0.5, y + 0.5);
			cairo_close_path (cr);
			cairo_stroke (cr);
		}
	}
}

static void
canvas_paint (GtkWidget *widget, cairo_t *cr)
{
	/* Paint a white background. */
	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_paint (cr);

	draw_grid (widget, cr);
}

