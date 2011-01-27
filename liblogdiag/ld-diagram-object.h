/*
 * ld-diagram-object.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010 - 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_DIAGRAM_OBJECT_H__
#define __LD_DIAGRAM_OBJECT_H__

G_BEGIN_DECLS


#define LD_TYPE_DIAGRAM_OBJECT (ld_diagram_object_get_type ())
#define LD_DIAGRAM_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_DIAGRAM_OBJECT, LdDiagramObject))
#define LD_DIAGRAM_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_DIAGRAM_OBJECT, LdDiagramObjectClass))
#define LD_IS_DIAGRAM_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_DIAGRAM_OBJECT))
#define LD_IS_DIAGRAM_OBJECT_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_DIAGRAM_OBJECT))
#define LD_DIAGRAM_OBJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_DIAGRAM_OBJECT, LdDiagramObjectClass))

typedef struct _LdDiagramObject LdDiagramObject;
typedef struct _LdDiagramObjectPrivate LdDiagramObjectPrivate;
typedef struct _LdDiagramObjectClass LdDiagramObjectClass;


/**
 * LdDiagramObject:
 */
struct _LdDiagramObject
{
/*< private >*/
	GObject parent_instance;
	LdDiagramObjectPrivate *priv;
};

/**
 * LdDiagramObjectClass:
 */
struct _LdDiagramObjectClass
{
/*< private >*/
	GObjectClass parent_class;

	guint data_changed_signal;
};


GType ld_diagram_object_get_type (void) G_GNUC_CONST;

LdDiagramObject *ld_diagram_object_new (JsonObject *storage);
JsonObject *ld_diagram_object_get_storage (LdDiagramObject *self);
void ld_diagram_object_set_storage (LdDiagramObject *self, JsonObject *storage);

gboolean ld_diagram_object_get_data (LdDiagramObject *self,
	GValue *data, GType type, const gchar *first_element, ...);
gboolean ld_diagram_object_get_data_valist (LdDiagramObject *self,
	GValue *data, GType type, const gchar *first_element, va_list var_args);
gboolean ld_diagram_object_get_datav (LdDiagramObject *self,
	GValue *data, GType type, const gchar **elements);
void ld_diagram_object_set_data (LdDiagramObject *self,
	const GValue *data, const gchar *first_element, ...);
void ld_diagram_object_set_data_valist (LdDiagramObject *self,
	const GValue *data, const gchar *first_element, va_list var_args);
void ld_diagram_object_set_datav (LdDiagramObject *self,
	const GValue *data, const gchar **elements);

void ld_diagram_object_get_data_for_param (LdDiagramObject *self,
	GValue *data, GParamSpec *pspec);
void ld_diagram_object_set_data_for_param (LdDiagramObject *self,
	const GValue *data, GParamSpec *pspec);

gdouble ld_diagram_object_get_x (LdDiagramObject *self);
gdouble ld_diagram_object_get_y (LdDiagramObject *self);
void ld_diagram_object_set_x (LdDiagramObject *self, gdouble x);
void ld_diagram_object_set_y (LdDiagramObject *self, gdouble y);


G_END_DECLS

#endif /* ! __LD_DIAGRAM_OBJECT_H__ */

