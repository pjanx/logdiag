/*
 * ld-canvas.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __CANVAS_H__
#define __CANVAS_H__

G_BEGIN_DECLS


#define LOGDIAG_TYPE_CANVAS (logdiag_canvas_get_type ())
#define LOGDIAG_CANVAS(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LOGDIAG_TYPE_CANVAS, LogdiagCanvas))
#define LOGDIAG_CANVAS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LOGDIAG_TYPE_CANVAS, LogdiagCanvasClass))
#define LOGDIAG_IS_CANVAS(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LOGDIAG_TYPE_CANVAS))
#define LOGDIAG_IS_CANVAS_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LOGDIAG_TYPE_CANVAS))
#define LOGDIAG_CANVAS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LOGDIAG_CANVAS, LogdiagCanvasClass))

typedef struct _LogdiagCanvas LogdiagCanvas;
typedef struct _LogdiagCanvasPrivate LogdiagCanvasPrivate;
typedef struct _LogdiagCanvasClass LogdiagCanvasClass;


/**
 * LogdiagCanvas:
 */
struct _LogdiagCanvas
{
/*< private >*/
	GtkDrawingArea parent_instance;
	LogdiagCanvasPrivate *priv;

/*< public >*/
};

struct _LogdiagCanvasClass
{
	GtkDrawingAreaClass parent_class;
};


GType logdiag_canvas_get_type (void) G_GNUC_CONST;

LogdiagCanvas *logdiag_canvas_new (void);


G_END_DECLS

#endif /* ! __CANVAS_H__ */
