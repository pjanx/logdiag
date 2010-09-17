/*
 * logdiag.c -- logdiag main source file.
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>

#include "config.h"

#include "ld-window-main.h"


int main (int argc, char *argv[])
{
	GtkWidget *wnd;

#ifdef HAVE_GETTEXT
	setlocale (LC_ALL, "");

	bindtextdomain (GETTEXT_DOMAIN, GETTEXT_DIRNAME);
	textdomain (GETTEXT_DOMAIN);
#endif

	/* For custom command line arguments, see:
	 * http://git.gnome.org/browse/glade3/tree/src/main.c
	 */

	gtk_init (&argc, &argv);
	wnd = ld_window_main_new ();
	gtk_main ();

	return 0;
}

