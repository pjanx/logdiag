/*
 * window-main.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __WINDOW_MAIN_H__
#define __WINDOW_MAIN_H__

G_BEGIN_DECLS


#define LOGDIAG_TYPE_WINDOW_MAIN (logdiag_window_main_get_type ())
#define LOGDIAG_WINDOW_MAIN(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LOGDIAG_TYPE_WINDOW_MAIN, LogdiagWindowMain))
#define LOGDIAG_WINDOW_MAIN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LOGDIAG_TYPE_WINDOW_MAIN, LogdiagWindowMainClass))
#define LOGDIAG_IS_WINDOW_MAIN(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LOGDIAG_TYPE_WINDOW_MAIN))
#define LOGDIAG_IS_WINDOW_MAIN_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LOGDIAG_TYPE_WINDOW_MAIN))
#define LOGDIAG_WINDOW_MAIN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LOGDIAG_WINDOW_MAIN, LogdiagWindowMainClass))

typedef struct _LogdiagWindowMain LogdiagWindowMain;
typedef struct _LogdiagWindowMainPrivate LogdiagWindowMainPrivate;
typedef struct _LogdiagWindowMainClass LogdiagWindowMainClass;


/**
 * LogdiagWindowMain:
 *
 * Object structure.
 */
struct _LogdiagWindowMain
{
/*< private >*/
	GtkWindow parent_instance;
	LogdiagWindowMainPrivate *priv;
};

struct _LogdiagWindowMainClass
{
	GtkWindowClass parent_class;
};


GType logdiag_window_main_get_type (void) G_GNUC_CONST;

GtkWidget *logdiag_window_main_new (void);


G_END_DECLS

#endif /* ! __WINDOW_MAIN_H__ */

