/*
 * ld-undo-action.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2011
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_UNDO_ACTION_H__
#define __LD_UNDO_ACTION_H__

G_BEGIN_DECLS


#define LD_TYPE_UNDO_ACTION (ld_undo_action_get_type ())
#define LD_UNDO_ACTION(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_UNDO_ACTION, LdUndoAction))
#define LD_UNDO_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_UNDO_ACTION, LdUndoActionClass))
#define LD_IS_UNDO_ACTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_UNDO_ACTION))
#define LD_IS_UNDO_ACTION_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_UNDO_ACTION))
#define LD_UNDO_ACTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_UNDO_ACTION, LdUndoActionClass))

typedef struct _LdUndoAction LdUndoAction;
typedef struct _LdUndoActionPrivate LdUndoActionPrivate;
typedef struct _LdUndoActionClass LdUndoActionClass;


/**
 * LdUndoAction:
 */
struct _LdUndoAction
{
/*< private >*/
	GObject parent_instance;
	LdUndoActionPrivate *priv;
};

/**
 * LdUndoActionClass:
 */
struct _LdUndoActionClass
{
/*< private >*/
	GObjectClass parent_class;
};


/**
 * LdUndoActionFunc:
 * @user_data: user data passed to ld_undo_action_new().
 *
 * A callback function prototype for actions.
 */
typedef void (*LdUndoActionFunc) (gpointer user_data);


GType ld_undo_action_get_type (void) G_GNUC_CONST;

/* TODO: Extend the methods (with eg. a string description). */
LdUndoAction *ld_undo_action_new (LdUndoActionFunc undo_func,
	LdUndoActionFunc redo_func, LdUndoActionFunc destroy_func,
	gpointer user_data);
void ld_undo_action_undo (LdUndoAction *self);
void ld_undo_action_redo (LdUndoAction *self);


G_END_DECLS

#endif /* ! __LD_UNDO_ACTION_H__ */

