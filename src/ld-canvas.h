/*
 * ld-canvas.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_CANVAS_H__
#define __LD_CANVAS_H__

G_BEGIN_DECLS


#define LD_TYPE_CANVAS (ld_canvas_get_type ())
#define LD_CANVAS(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_CANVAS, LdCanvas))
#define LD_CANVAS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_CANVAS, LdCanvasClass))
#define LD_IS_CANVAS(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_CANVAS))
#define LD_IS_CANVAS_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_CANVAS))
#define LD_CANVAS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_CANVAS, LdCanvasClass))

typedef struct _LdCanvas LdCanvas;
typedef struct _LdCanvasPrivate LdCanvasPrivate;
typedef struct _LdCanvasClass LdCanvasClass;


/**
 * LdCanvas:
 */
struct _LdCanvas
{
/*< private >*/
	GtkDrawingArea parent_instance;
	LdCanvasPrivate *priv;

/*< public >*/
};

struct _LdCanvasClass
{
/*< private >*/
	GtkDrawingAreaClass parent_class;

	guint cancel_operation_signal;

	void (*set_scroll_adjustments) (LdCanvas *self,
		GtkAdjustment *horizontal, GtkAdjustment *vertical);
	void (*cancel_operation) (LdCanvas *self);
};


/**
 * LD_CANVAS_BASE_UNIT:
 *
 * Length of the base unit in milimetres.
 */
#define LD_CANVAS_BASE_UNIT_LENGTH 2.5


GType ld_canvas_get_type (void) G_GNUC_CONST;

LdCanvas *ld_canvas_new (void);

void ld_canvas_set_diagram (LdCanvas *self, LdDiagram *diagram);
LdDiagram *ld_canvas_get_diagram (LdCanvas *self);
void ld_canvas_set_library (LdCanvas *self, LdLibrary *library);
LdLibrary *ld_canvas_get_library (LdCanvas *self);

void ld_canvas_widget_to_diagram_coords (LdCanvas *self,
	gdouble wx, gdouble wy, gdouble *dx, gdouble *dy);
void ld_canvas_diagram_to_widget_coords (LdCanvas *self,
	gdouble dx, gdouble dy, gdouble *wx, gdouble *wy);

void ld_canvas_add_object_begin (LdCanvas *self, LdDiagramObject *object);

/* TODO: The rest of the interface. */


G_END_DECLS

#endif /* ! __LD_CANVAS_H__ */
