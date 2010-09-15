/*
 * window-main.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>

#include "config.h"

#include "window-main.h"
#include "symbol-library.h"
#include "symbol-category.h"


/**
 * SECTION:window-main
 * @short_description: The main application window.
 *
 * #LogdiagWindowMain is the main window of the application.
 */
/* NOTE: The main window should not maybe be included in either
 * the documentation or the static library.
 */


/* Private members of the window. */
struct _LogdiagWindowMainPrivate
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *menu;
	GtkWidget *toolbar;

	LogdiagSymbolLibrary *library;

	GtkWidget *statusbar;
	guint statusbar_menu_context_id;
};

/* Define the type. */
G_DEFINE_TYPE (LogdiagWindowMain, logdiag_window_main, GTK_TYPE_WINDOW);


/* ===== Local functions =================================================== */

/*
 * cb_load_category:
 *
 * A hashtable foreach callback for adding categories into the toolbar.
 */
static void
cb_load_category (gpointer key, gpointer value, gpointer user_data);

/*
 * load_toolbar:
 *
 * Load symbols from the library into the toolbar.
 */
static void
load_toolbar (LogdiagWindowMain *self);

/*
 * cb_ui_proxy_connected:
 *
 * An item was connected to the manager.
 */
static void
cb_ui_proxy_connected (GtkUIManager *ui, GtkAction *action,
	GtkWidget *proxy, LogdiagWindowMain *window);

/*
 * cb_ui_proxy_disconnected:
 *
 * An item was disconnected from the manager.
 */
static void
cb_ui_proxy_disconnected (GtkUIManager *ui, GtkAction *action,
	GtkWidget *proxy, LogdiagWindowMain *window);

/* A menu item was selected. */
static void
cb_menu_item_selected (GtkWidget *item, LogdiagWindowMain *window);

/* A menu item was deselected. */
static void
cb_menu_item_deselected (GtkItem *item, LogdiagWindowMain *window);

/* Show the about dialog. */
static void
cb_show_about_dialog (GtkAction *action, LogdiagWindowMain *window);


/* ===== Local variables =================================================== */

/* Actions for menus, toolbars, accelerators. */
static GtkActionEntry mw_actionEntries[] =
{
	{"FileMenu", NULL, Q_("_File")},
		{"New", GTK_STOCK_NEW, NULL, NULL,
			Q_("Create a new document"), NULL},
		{"Open", GTK_STOCK_OPEN, NULL, NULL,
			Q_("Open a document"), NULL},
		{"Save", GTK_STOCK_SAVE, NULL, NULL,
			Q_("Save the current document"), NULL},
		{"SaveAs", GTK_STOCK_SAVE_AS, NULL, NULL,
			Q_("Save the current document with another name"), NULL},
		{"Export", NULL, Q_("_Export"), NULL,
			Q_("Export the document"), NULL},
		{"Quit", GTK_STOCK_QUIT, NULL, NULL,
			Q_("Quit the program"), NULL},

	{"EditMenu", NULL, Q_("_Edit")},
		{"Cut", GTK_STOCK_CUT, NULL, NULL, NULL, NULL},
		{"Copy", GTK_STOCK_COPY, NULL, NULL, NULL, NULL},
		{"Paste", GTK_STOCK_PASTE, NULL, NULL, NULL, NULL},
		{"Delete", GTK_STOCK_DELETE, NULL, NULL, NULL, NULL},
		{"SelectAll", GTK_STOCK_SELECT_ALL, NULL, NULL, NULL, NULL},

	{"HelpMenu", NULL, Q_("_Help")},
		{"About", GTK_STOCK_ABOUT, NULL, NULL, NULL,
			G_CALLBACK(cb_show_about_dialog)}
};



/**
 * logdiag_window_main_new:
 *
 * Create an instance.
 */
GtkWidget *
logdiag_window_main_new (void)
{
	return g_object_new (LOGDIAG_TYPE_WINDOW_MAIN, NULL);
}

static void
logdiag_window_main_class_init (LogdiagWindowMainClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);

	g_type_class_add_private (klass, sizeof (LogdiagWindowMainPrivate));
}

static void
logdiag_window_main_init (LogdiagWindowMain *self)
{
	LogdiagWindowMainPrivate *priv;
	GtkActionGroup *action_group;
	GtkUIManager *ui_manager;
	GError *error;

	self->priv = priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LOGDIAG_TYPE_WINDOW_MAIN, LogdiagWindowMainPrivate);

	priv->vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (self), priv->vbox);


	ui_manager = gtk_ui_manager_new ();

	/* TODO: Show tooltips in the statusbar:
	 * http://git.gnome.org/browse/glade3/tree/src/glade-window.c : 2165
	 */
	g_signal_connect (ui_manager, "connect-proxy",
		G_CALLBACK (cb_ui_proxy_connected), self);
	g_signal_connect (ui_manager, "disconnect-proxy",
		G_CALLBACK (cb_ui_proxy_disconnected), self);

	/* Prepare our actions. */
	action_group = gtk_action_group_new ("MainActions");
	gtk_action_group_add_actions (action_group,
		mw_actionEntries, G_N_ELEMENTS (mw_actionEntries), self);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);

	error = NULL;
	gtk_ui_manager_add_ui_from_file
		(ui_manager, PROJECT_SHARE_DIR "gui/window-main.ui", &error);
	if (error)
	{
		g_message (_("Building UI failed: %s"), error->message);
		g_error_free (error);
	}

	/* Load keyboard accelerators into the window. */
	gtk_window_add_accel_group
		(GTK_WINDOW (self), gtk_ui_manager_get_accel_group (ui_manager));

	priv->menu = gtk_ui_manager_get_widget (ui_manager, "/MenuBar");
	gtk_box_pack_start (GTK_BOX (priv->vbox), priv->menu, FALSE, FALSE, 0);

	priv->hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start (GTK_BOX (priv->vbox), priv->hbox, TRUE, TRUE, 0);

	/* Add the symbol toolbar. */
	priv->toolbar = gtk_toolbar_new ();
	/* NOTE: For GTK 2.16+, s/toolbar/orientable/ */
	gtk_toolbar_set_orientation
		(GTK_TOOLBAR (priv->toolbar), GTK_ORIENTATION_VERTICAL);
	gtk_toolbar_set_icon_size
		(GTK_TOOLBAR (priv->toolbar), GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_toolbar_set_style
		(GTK_TOOLBAR (priv->toolbar), GTK_TOOLBAR_ICONS);

	gtk_box_pack_start (GTK_BOX (priv->hbox), priv->toolbar, FALSE, FALSE, 0);

	/* Symbol library. */
	priv->library = logdiag_symbol_library_new ();
	logdiag_symbol_library_load (priv->library, PROJECT_SHARE_DIR "library");

	load_toolbar (self);

	/* TODO: GtkHPaned */

	priv->statusbar = gtk_statusbar_new ();
	priv->statusbar_menu_context_id = gtk_statusbar_get_context_id
		(GTK_STATUSBAR (priv->statusbar), "menu");
	gtk_box_pack_end (GTK_BOX (priv->vbox), priv->statusbar, FALSE, FALSE, 0);


	/* Do this on disposal. */
	/* g_object_unref(ui_manager); */

	/* Proceed to showing the window. */
	g_signal_connect (self, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	gtk_window_set_default_size (GTK_WINDOW (self), 500, 400);
	gtk_widget_show_all (GTK_WIDGET (self));
}

static void
cb_load_category (gpointer key, gpointer value, gpointer user_data)
{
	const gchar *name;
	LogdiagSymbolCategory *cat;
	LogdiagWindowMain *self;
	GdkPixbuf *pbuf;
	GtkWidget *img;
	GtkToolItem *item;

	name = key;
	cat = value;
	self = user_data;

	g_return_if_fail (key != NULL);
	g_return_if_fail (LOGDIAG_IS_SYMBOL_CATEGORY (cat));

	/* XXX: Hardcoded icon width, unref? */
	pbuf = gdk_pixbuf_new_from_file_at_size (cat->image_path, 32, -1, NULL);
	if (!pbuf)
		return;
	img = gtk_image_new_from_pixbuf (pbuf);
	g_object_unref (pbuf);

	item = gtk_tool_button_new (img, name);
	gtk_tool_item_set_tooltip_text (item, name);
	gtk_toolbar_insert (GTK_TOOLBAR (self->priv->toolbar), item, 0);
}

static void
load_toolbar (LogdiagWindowMain *self)
{
	/* TODO: Clear the toolbar first, if there was already something in it. */

	g_hash_table_foreach (self->priv->library->categories,
		cb_load_category, self);
}

static void
cb_ui_proxy_connected (GtkUIManager *ui, GtkAction *action,
	GtkWidget *proxy, LogdiagWindowMain *window)
{
	if (GTK_IS_MENU_ITEM (proxy))
	{
		g_signal_connect (proxy, "select",
			G_CALLBACK (cb_menu_item_selected), window);
		g_signal_connect (proxy, "deselect",
			G_CALLBACK (cb_menu_item_deselected), window);
	}
}

static void
cb_ui_proxy_disconnected (GtkUIManager *ui, GtkAction *action,
	GtkWidget *proxy, LogdiagWindowMain *window)
{
	if (GTK_IS_MENU_ITEM (proxy))
	{
		g_signal_handlers_disconnect_by_func
			(proxy, G_CALLBACK (cb_menu_item_selected), window);
		g_signal_handlers_disconnect_by_func
			(proxy, G_CALLBACK (cb_menu_item_deselected), window);
	}
}

static void
cb_menu_item_selected (GtkWidget *item, LogdiagWindowMain *window)
{
	GtkAction *action;
	gchar *tooltip;

	action = gtk_activatable_get_related_action (GTK_ACTIVATABLE (item));
	g_object_get (G_OBJECT (action), "tooltip", &tooltip, NULL);

	if (tooltip != NULL)
		gtk_statusbar_push (GTK_STATUSBAR (window->priv->statusbar),
			window->priv->statusbar_menu_context_id, tooltip);

	g_free (tooltip);
}

static void
cb_menu_item_deselected (GtkItem *item, LogdiagWindowMain *window)
{
	gtk_statusbar_pop (GTK_STATUSBAR (window->priv->statusbar),
		window->priv->statusbar_menu_context_id);
}

static void
cb_show_about_dialog (GtkAction *action, LogdiagWindowMain *window)
{
	gtk_show_about_dialog (GTK_WINDOW (window),
		"program-name", PROJECT_NAME,
		"version", PROJECT_VERSION,
		"copyright", "Copyright Přemysl Janouch 2010",
		NULL);
}

