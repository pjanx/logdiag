/*
 * canvas.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>

#include "config.h"

#include "canvas.h"
#include "document.h"

/* http://www.gnomejournal.org/article/34/writing-a-widget-using-cairo-and-gtk28 */

/**
 * SECTION:canvas
 * @short_description: A canvas.
 * @see_also: #LogdiagDocument
 *
 * #LogdiagCanvas is used for displaying #LogdiagDocument objects.
 */

/*
 * LogdiagCanvasPrivate:
 * @document: A document object assigned to this canvas as a model.
 */
struct _LogdiagCanvasPrivate
{
	LogdiagDocument *document;
};

G_DEFINE_TYPE (LogdiagCanvas, logdiag_canvas, GTK_TYPE_DRAWING_AREA);

static void
logdiag_canvas_finalize (GObject *gobject);


static void
logdiag_canvas_class_init (LogdiagCanvasClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = logdiag_canvas_finalize;

	widget_class = GTK_WIDGET_CLASS (klass);

/**
 * LogdiagCanvas::set-scroll-adjustments:
 * @canvas: The canvas object.
 *
 * Contents of the library have changed.
 */
/*
	widget_class->set_scroll_adjustments_signal = g_signal_new
		("set-scroll-adjustments", G_TYPE_FROM_CLASS (widget_class),
		G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		0, // G_STRUCT_OFFSET (LogdiagCanvasClass, ...)
		NULL, NULL,
		gtk_marshal_NONE__POINTER_POINTER,
		G_TYPE_NONE, 2, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);
*/
	g_type_class_add_private (klass, sizeof (LogdiagCanvasPrivate));
}

static void
logdiag_canvas_init (LogdiagCanvas *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LOGDIAG_TYPE_CANVAS, LogdiagCanvasPrivate);
}

static void
logdiag_canvas_finalize (GObject *gobject)
{
	LogdiagCanvas *self;

	self = LOGDIAG_CANVAS (gobject);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (logdiag_canvas_parent_class)->finalize (gobject);
}

/**
 * logdiag_canvas_new:
 *
 * Create an instance.
 */
LogdiagCanvas *
logdiag_canvas_new (void)
{
	return g_object_new (LOGDIAG_TYPE_CANVAS, NULL);
}
