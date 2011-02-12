/*
 * logdiag.c -- logdiag main source file.
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010 - 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>
#include <locale.h>

#include "config.h"

#include "ld-window-main.h"


int
main (int argc, char *argv[])
{
	GtkWidget *wnd;

	setlocale (LC_ALL, "");

	bindtextdomain (GETTEXT_DOMAIN, GETTEXT_DIRNAME);
	bind_textdomain_codeset (GETTEXT_DOMAIN, "UTF-8");
	textdomain (GETTEXT_DOMAIN);

	/* For custom command line arguments, see:
	 * http://git.gnome.org/browse/glade3/tree/src/main.c
	 */

	gtk_init (&argc, &argv);
	gtk_window_set_default_icon_name (PROJECT_NAME);
	wnd = ld_window_main_new ();
	gtk_main ();

	return 0;
}

