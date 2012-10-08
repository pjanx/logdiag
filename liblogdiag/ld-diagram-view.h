/*
 * ld-diagram-view.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010, 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_DIAGRAM_VIEW_H__
#define __LD_DIAGRAM_VIEW_H__

G_BEGIN_DECLS


#define LD_TYPE_DIAGRAM_VIEW (ld_diagram_view_get_type ())
#define LD_DIAGRAM_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_DIAGRAM_VIEW, LdDiagramView))
#define LD_DIAGRAM_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_DIAGRAM_VIEW, LdDiagramViewClass))
#define LD_IS_DIAGRAM_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_DIAGRAM_VIEW))
#define LD_IS_DIAGRAM_VIEW_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_DIAGRAM_VIEW))
#define LD_DIAGRAM_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_DIAGRAM_VIEW, LdDiagramViewClass))

typedef struct _LdDiagramView LdDiagramView;
typedef struct _LdDiagramViewPrivate LdDiagramViewPrivate;
typedef struct _LdDiagramViewClass LdDiagramViewClass;


/**
 * LdDiagramView:
 */
struct _LdDiagramView
{
/*< private >*/
	GtkDrawingArea parent_instance;
	LdDiagramViewPrivate *priv;
};

struct _LdDiagramViewClass
{
/*< private >*/
	GtkDrawingAreaClass parent_class;

	guint cancel_operation_signal;
	guint move_signal;

	void (*set_scroll_adjustments) (LdDiagramView *self,
		GtkAdjustment *horizontal, GtkAdjustment *vertical);
	void (*cancel_operation) (LdDiagramView *self);
	void (*move) (LdDiagramView *self, gdouble dx, gdouble dy);
};


/**
 * LD_DIAGRAM_VIEW_BASE_UNIT_LENGTH:
 *
 * Length of the base unit in milimetres.
 */
#define LD_DIAGRAM_VIEW_BASE_UNIT_LENGTH 2.5


GType ld_diagram_view_get_type (void) G_GNUC_CONST;

GtkWidget *ld_diagram_view_new (void);

void ld_diagram_view_set_diagram (LdDiagramView *self, LdDiagram *diagram);
LdDiagram *ld_diagram_view_get_diagram (LdDiagramView *self);
void ld_diagram_view_set_library (LdDiagramView *self, LdLibrary *library);
LdLibrary *ld_diagram_view_get_library (LdDiagramView *self);

void ld_diagram_view_widget_to_diagram_coords (LdDiagramView *self,
	gdouble wx, gdouble wy, gdouble *dx, gdouble *dy);
void ld_diagram_view_diagram_to_widget_coords (LdDiagramView *self,
	gdouble dx, gdouble dy, gdouble *wx, gdouble *wy);

gdouble ld_diagram_view_get_x (LdDiagramView *self);
void ld_diagram_view_set_x (LdDiagramView *self, gdouble x);
gdouble ld_diagram_view_get_y (LdDiagramView *self);
void ld_diagram_view_set_y (LdDiagramView *self, gdouble y);

gdouble ld_diagram_view_get_zoom (LdDiagramView *self);
void ld_diagram_view_set_zoom (LdDiagramView *self, gdouble zoom);
gboolean ld_diagram_view_can_zoom_in (LdDiagramView *self);
void ld_diagram_view_zoom_in (LdDiagramView *self);
gboolean ld_diagram_view_can_zoom_out (LdDiagramView *self);
void ld_diagram_view_zoom_out (LdDiagramView *self);

gboolean ld_diagram_view_get_show_grid (LdDiagramView *self);
void ld_diagram_view_set_show_grid (LdDiagramView *self, gboolean show_grid);

void ld_diagram_view_add_object_begin (LdDiagramView *self,
	LdDiagramObject *object);


G_END_DECLS

#endif /* ! __LD_DIAGRAM_VIEW_H__ */
