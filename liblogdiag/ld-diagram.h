/*
 * ld-diagram.h
 *
 * This file is a part of logdiag.
 * Copyright 2010, 2011 Přemysl Eric Janouch
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_DIAGRAM_H__
#define __LD_DIAGRAM_H__

G_BEGIN_DECLS


#define LD_TYPE_DIAGRAM (ld_diagram_get_type ())
#define LD_DIAGRAM(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_DIAGRAM, LdDiagram))
#define LD_DIAGRAM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_DIAGRAM, LdDiagramClass))
#define LD_IS_DIAGRAM(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_DIAGRAM))
#define LD_IS_DIAGRAM_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_DIAGRAM))
#define LD_DIAGRAM_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_DIAGRAM, LdDiagramClass))

typedef struct _LdDiagram LdDiagram;
typedef struct _LdDiagramClass LdDiagramClass;
typedef struct _LdDiagramPrivate LdDiagramPrivate;


/**
 * LdDiagram:
 *
 * A diagram object.
 */
struct _LdDiagram
{
/*< private >*/
	GObject parent_instance;
	LdDiagramPrivate *priv;
};

struct _LdDiagramClass
{
/*< private >*/
	GObjectClass parent_class;

	guint changed_signal;
	guint selection_changed_signal;

	void (*changed) (LdDiagram *self);
	void (*selection_changed) (LdDiagram *self);
};


GQuark ld_diagram_error_quark (void);

/**
 * LD_DIAGRAM_ERROR:
 *
 * Uset to get the #GError quark for #LdDiagram errors.
 */
#define LD_DIAGRAM_ERROR (ld_diagram_error_quark ())

/**
 * LdDiagramError:
 * @LD_DIAGRAM_ERROR_DIAGRAM_CORRUPT: The input diagram is corrupt.
 *
 * These identify errors that can occur while calling #LdDiagram functions.
 */
typedef enum
{
	LD_DIAGRAM_ERROR_DIAGRAM_CORRUPT
}
LdDiagramError;


GType ld_diagram_get_type (void) G_GNUC_CONST;

LdDiagram *ld_diagram_new (void);
void ld_diagram_clear (LdDiagram *self);
gboolean ld_diagram_load_from_file (LdDiagram *self,
	const gchar *filename, GError **error);
gboolean ld_diagram_save_to_file (LdDiagram *self,
	const gchar *filename, GError **error);

gboolean ld_diagram_get_modified (LdDiagram *self);
void ld_diagram_set_modified (LdDiagram *self, gboolean value);

gboolean ld_diagram_can_undo (LdDiagram *self);
gboolean ld_diagram_can_redo (LdDiagram *self);
void ld_diagram_undo (LdDiagram *self);
void ld_diagram_redo (LdDiagram *self);
void ld_diagram_begin_user_action (LdDiagram *self);
void ld_diagram_end_user_action (LdDiagram *self);

GList *ld_diagram_get_objects (LdDiagram *self);
void ld_diagram_insert_object (LdDiagram *self,
	LdDiagramObject *object, gint pos);
void ld_diagram_remove_object (LdDiagram *self,
	LdDiagramObject *object);

GList *ld_diagram_get_selection (LdDiagram *self);
void ld_diagram_remove_selection (LdDiagram *self);
void ld_diagram_select (LdDiagram *self, LdDiagramObject *object);
void ld_diagram_select_all (LdDiagram *self);
void ld_diagram_unselect (LdDiagram *self, LdDiagramObject *object);
void ld_diagram_unselect_all (LdDiagram *self);


G_END_DECLS

#endif /* ! __LD_DIAGRAM_H__ */

