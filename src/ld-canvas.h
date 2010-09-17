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
	GtkDrawingAreaClass parent_class;
};


GType ld_canvas_get_type (void) G_GNUC_CONST;

LdCanvas *ld_canvas_new (void);


G_END_DECLS

#endif /* ! __LD_CANVAS_H__ */
