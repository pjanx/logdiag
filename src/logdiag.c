/*
 * logdiag.c -- logdiag main source file.
 *
 * This file is a part of logdiag.
 * Copyright 2010, 2011 Přemysl Eric Janouch
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <locale.h>

#include "config.h"

#include "ld-window-main.h"


#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>

/*
 * get_utf8_args:
 * @argc: where the number of arguments will be stored.
 * @argv: where the actual array of arguments will be stored.
 *        Use g_strfreev() to free the array.
 *
 * Retrieve program arguments in UTF-8 encoding on Windows.
 *
 * Return value: %TRUE if the function has succeeded.
 */
static gboolean
get_utf8_args (int *argc, char ***argv)
{
	LPWSTR *argv_wide;
	int i, argc_local, buff_size;
	char **argv_local, *arg;

	argv_wide = CommandLineToArgvW (GetCommandLineW (), &argc_local);
	if (!argv_wide)
		return FALSE;

	argv_local = g_malloc ((argc_local + 1) * sizeof (char *));
	for (i = 0; i < argc_local; i++)
	{
		buff_size = WideCharToMultiByte (CP_UTF8, 0, argv_wide[i], -1,
			NULL, 0, NULL, NULL);
		if (!buff_size)
			goto get_utf8_args_fail;

		argv_local[i] = g_malloc (buff_size);
		if (!WideCharToMultiByte (CP_UTF8, 0, argv_wide[i], -1,
			argv_local[i], buff_size, NULL, NULL))
		{
			g_free (argv_local[i]);
			goto get_utf8_args_fail;
		}
	}
	argv_local[i] = NULL;
	LocalFree (argv_wide);

	if (argc)
		*argc = argc_local;
	if (argv)
		*argv = argv_local;
	return TRUE;

get_utf8_args_fail:
	while (i--)
		g_free (argv_local[i]);
	g_free (argv_local);

	LocalFree (argv_wide);
	return FALSE;
}
#endif

static gint ld_active_windows = 0;

static void
window_on_destroyed (GtkWidget *object, gpointer user_data)
{
	if (--ld_active_windows <= 0)
		gtk_main_quit ();
}

static void
window_create (const gchar *file)
{
	GtkWidget *wm;

	wm = ld_window_main_new (file);
	g_signal_connect (wm, "destroy", G_CALLBACK (window_on_destroyed), NULL);
	ld_active_windows++;
}

int
main (int argc, char *argv[])
{
	gchar **iter, **files = NULL;
	GOptionEntry option_entries[] =
	{
		{G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files,
			NULL, N_("[FILE...]")},
		{NULL}
	};

	GError *error;
#ifdef _WIN32
	gboolean argv_overriden;
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

#ifdef PROJECT_GSETTINGS_DIR
	/* This is enabled when the build is set up for development,
	 * so that the application can find its schema. It might also find use
	 * when installing the application into a location that's missing from
	 * g_get_system_data_dirs(), for example /usr/local or ~/.local.
	 */
	g_setenv ("GSETTINGS_SCHEMA_DIR", PROJECT_GSETTINGS_DIR, 0);
#endif /* PROJECT_GSETTINGS_DIR */

#ifdef _WIN32
	/* Don't be unneccessarily limited by the system's ANSI codepage. */
	/* g_win32_get_command_line() should replace this code for GLib >= 2.40. */
	argv_overriden = get_utf8_args (&argc, &argv);
	if (argv_overriden)
		_putenv ("CHARSET=UTF-8");
#endif

	error = NULL;
	gtk_init_with_args (&argc, &argv, N_("- Schematic editor"),
		option_entries, GETTEXT_DOMAIN, &error);
	if (error)
	{
		g_warning ("%s", error->message);
		g_error_free (error);
		return 1;
	}

#ifdef _WIN32
	if (argv_overriden)
	{
		_putenv ("CHARSET=");
		g_strfreev (argv);
	}
#endif

#ifdef OPTION_NOINSTALL
	gtk_icon_theme_prepend_search_path (gtk_icon_theme_get_default (),
		PROJECT_SHARE_DIR "icons");
#endif

	gtk_window_set_default_icon_name (PROJECT_NAME);

	if (files)
	{
		for (iter = files; *iter; iter++)
			window_create (*iter);
		g_strfreev (files);
	}
	else
		window_create (NULL);

	gtk_main ();
	return 0;
}
