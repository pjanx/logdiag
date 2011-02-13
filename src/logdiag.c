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
	GtkWidget *window;
	GError *error;
#ifdef _WIN32
	gchar *install_dir;

	install_dir = g_win32_get_package_installation_directory_of_module (NULL);
	if (install_dir)
	{
		g_chdir (install_dir);
		g_free (install_dir);
	}
#endif

	setlocale (LC_ALL, "");

	bindtextdomain (GETTEXT_DOMAIN, GETTEXT_DIRNAME);
	bind_textdomain_codeset (GETTEXT_DOMAIN, "UTF-8");
	textdomain (GETTEXT_DOMAIN);

	error = NULL;
	gtk_init_with_args (&argc, &argv,
		N_("[FILE] - Schematic editor"), NULL, GETTEXT_DOMAIN, &error);
	if (error)
	{
		g_warning ("%s", error->message);
		g_error_free (error);
		return 1;
	}

	/* TODO: Open the file in the parameter, if present. */
	gtk_window_set_default_icon_name (PROJECT_NAME);
	window = ld_window_main_new ();
	gtk_main ();

	return 0;
}

