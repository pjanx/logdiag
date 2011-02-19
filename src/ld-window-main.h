/*
 * ld-window-main.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_WINDOW_MAIN_H__
#define __LD_WINDOW_MAIN_H__

G_BEGIN_DECLS


#define LD_TYPE_WINDOW_MAIN (ld_window_main_get_type ())
#define LD_WINDOW_MAIN(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_WINDOW_MAIN, LdWindowMain))
#define LD_WINDOW_MAIN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_WINDOW_MAIN, LdWindowMainClass))
#define LD_IS_WINDOW_MAIN(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_WINDOW_MAIN))
#define LD_IS_WINDOW_MAIN_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_WINDOW_MAIN))
#define LD_WINDOW_MAIN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_WINDOW_MAIN, LdWindowMainClass))

typedef struct _LdWindowMain LdWindowMain;
typedef struct _LdWindowMainPrivate LdWindowMainPrivate;
typedef struct _LdWindowMainClass LdWindowMainClass;


struct _LdWindowMain
{
/*< private >*/
	GtkWindow parent_instance;
	LdWindowMainPrivate *priv;
};

struct _LdWindowMainClass
{
/*< private >*/
	GtkWindowClass parent_class;
};


GType ld_window_main_get_type (void) G_GNUC_CONST;

GtkWidget *ld_window_main_new (const gchar *filename);


G_END_DECLS

#endif /* ! __LD_WINDOW_MAIN_H__ */

