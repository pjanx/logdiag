/*
 * ld-undo-action.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2011
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <string.h>

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-undo-action
 * @short_description: An action that can be undone
 * @see_also: #LdDiagram, #LdDiagramObject
 *
 * #LdUndoAction represents an action that can be reverted.
 */

/*
 * LdUndoActionPrivate:
 * @undo_func: a callback to undo the action.
 * @redo_func: a callback to redo the action.
 * @destroy_func: a callback to destroy user data.
 * @user_data: data given by the user.
 */
struct _LdUndoActionPrivate
{
	LdUndoActionFunc undo_func;
	LdUndoActionFunc redo_func;
	LdUndoActionFunc destroy_func;
	gpointer user_data;
};

static void ld_undo_action_finalize (GObject *gobject);


G_DEFINE_TYPE (LdUndoAction, ld_undo_action, G_TYPE_OBJECT);

static void
ld_undo_action_class_init (LdUndoActionClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_undo_action_finalize;

	g_type_class_add_private (klass, sizeof (LdUndoActionPrivate));
}

static void
ld_undo_action_init (LdUndoAction *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_UNDO_ACTION, LdUndoActionPrivate);
}

static void
ld_undo_action_finalize (GObject *gobject)
{
	LdUndoAction *self;

	self = LD_UNDO_ACTION (gobject);
	if (self->priv->destroy_func)
		self->priv->destroy_func (self->priv->user_data);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_undo_action_parent_class)->finalize (gobject);
}


/**
 * ld_undo_action_new:
 * @undo_func: a callback to undo the action.
 * @redo_func: a callback to redo the action.
 * @destroy_func: (allow-none): a callback to destroy user data.
 * @user_data: user data passed to callbacks.
 *
 * Return value: a new #LdUndoAction object.
 */
LdUndoAction *
ld_undo_action_new (LdUndoActionFunc undo_func,
	LdUndoActionFunc redo_func, LdUndoActionFunc destroy_func,
	gpointer user_data)
{
	LdUndoAction *self;

	g_return_val_if_fail (undo_func != NULL, NULL);
	g_return_val_if_fail (redo_func != NULL, NULL);

	self = g_object_new (LD_TYPE_UNDO_ACTION, NULL);
	self->priv->undo_func = undo_func;
	self->priv->redo_func = redo_func;
	self->priv->destroy_func = destroy_func;
	self->priv->user_data = user_data;
	return self;
}

/**
 * ld_undo_action_undo:
 * @self: an #LdUndoAction object.
 *
 * Undo the action handled by this object.
 */
void
ld_undo_action_undo (LdUndoAction *self)
{
	g_return_if_fail (LD_IS_UNDO_ACTION (self));
	self->priv->undo_func (self->priv->user_data);
}

/**
 * ld_undo_action_redo:
 * @self: an #LdUndoAction object.
 *
 * Redo the action handled by this object.
 */
void
ld_undo_action_redo (LdUndoAction *self)
{
	g_return_if_fail (LD_IS_UNDO_ACTION (self));
	self->priv->redo_func (self->priv->user_data);
}
