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
#include "ld-canvas.h"
#include "ld-document.h"

/* http://www.gnomejournal.org/article/34/writing-a-widget-using-cairo-and-gtk28 */

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
};

G_DEFINE_TYPE (LdCanvas, ld_canvas, GTK_TYPE_DRAWING_AREA);

static void
ld_canvas_finalize (GObject *gobject);


static void
ld_canvas_class_init (LdCanvasClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_canvas_finalize;

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
		0, // G_STRUCT_OFFSET (LdCanvasClass, ...)
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
}

static void
ld_canvas_finalize (GObject *gobject)
{
	LdCanvas *self;

	self = LD_CANVAS (gobject);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_canvas_parent_class)->finalize (gobject);
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
