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
	gchar **files = NULL;
	GOptionEntry option_entries[] =
	{
		{G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files,
			NULL, N_("[FILE]")},
		{NULL}
	};

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
	gtk_init_with_args (&argc, &argv, N_("- Schematic editor"),
		option_entries, GETTEXT_DOMAIN, &error);
	if (error)
	{
		g_warning ("%s", error->message);
		g_error_free (error);
		return 1;
	}

	gtk_window_set_default_icon_name (PROJECT_NAME);

	/* TODO: Be able to open multiple files. */
	if (files)
	{
		ld_window_main_new (files[0]);
		g_strfreev (files);
	}
	else
		ld_window_main_new (NULL);

	gtk_main ();
	return 0;
}

