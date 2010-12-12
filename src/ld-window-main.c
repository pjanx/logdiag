/*
 * ld-window-main.c
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

#include "ld-symbol.h"
#include "ld-symbol-category.h"
#include "ld-library.h"

#include "ld-document-object.h"
#include "ld-document-symbol.h"
#include "ld-document.h"

#include "ld-canvas.h"


/**
 * SECTION:ld-window-main
 * @short_description: The main application window.
 *
 * #LdWindowMain is the main window of the application.
 */
/* NOTE: The main window should not maybe be included in either
 *       the documentation or the static library.
 */


typedef struct _SymbolMenuItem SymbolMenuItem;
typedef struct _SymbolMenuData SymbolMenuData;

/*
 * SymbolMenuItem:
 *
 * Data related to a symbol in an open symbol menu.
 */
struct _SymbolMenuItem
{
	LdSymbol *symbol;

	gint width;
	gdouble scale;
};

/*
 * SymbolMenuData:
 *
 * Data related to the currently opened symbol menu.
 */
struct _SymbolMenuData
{
	gulong expose_handler;
	gulong motion_notify_handler;
	gulong button_release_handler;

	GtkToggleButton *active_button;

	SymbolMenuItem *items;
	gint n_items;
	gint active_item;

	gint menu_width;
	gint menu_height;
	gint menu_y;
};

struct _LdWindowMainPrivate
{
	GtkUIManager *ui_manager;

	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *menu;
	GtkWidget *toolbar;
	GtkWidget *library_toolbar;

	LdLibrary *library;

	GtkWidget *canvas_window;
	LdCanvas *canvas;

	GtkWidget *statusbar;
	guint statusbar_menu_context_id;

	SymbolMenuData symbol_menu;
};

/* Define the type. */
G_DEFINE_TYPE (LdWindowMain, ld_window_main, GTK_TYPE_WINDOW);

#define LIBRARY_TOOLBAR_ICON_WIDTH 32


/* ===== Local functions =================================================== */

static void ld_window_main_finalize (GObject *gobject);

static void load_library_toolbar (LdWindowMain *self);
static void load_category_cb (gpointer data, gpointer user_data);

static void redraw_symbol_menu (LdWindowMain *self);
static void on_category_toggle (GtkToggleButton *toggle_button,
	gpointer user_data);
static gboolean on_canvas_exposed (GtkWidget *widget,
	GdkEventExpose *event, gpointer user_data);
static gboolean on_canvas_motion_notify (GtkWidget *widget,
	GdkEventMotion *event, gpointer user_data);
static gboolean on_canvas_button_release (GtkWidget *widget,
	GdkEventButton *event, gpointer user_data);

static void on_ui_proxy_connected (GtkUIManager *ui, GtkAction *action,
	GtkWidget *proxy, LdWindowMain *window);
static void on_ui_proxy_disconnected (GtkUIManager *ui, GtkAction *action,
	GtkWidget *proxy, LdWindowMain *window);

static void on_menu_item_selected (GtkWidget *item, LdWindowMain *window);
static void on_menu_item_deselected (GtkItem *item, LdWindowMain *window);

static void on_action_about (GtkAction *action, LdWindowMain *self);


/* ===== Local variables =================================================== */

/* Actions for menus, toolbars, accelerators. */
static GtkActionEntry wm_action_entries[] =
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
			Q_("Quit the application"),
			G_CALLBACK (gtk_main_quit)},

	{"EditMenu", NULL, Q_("_Edit")},
		/* XXX: Don't implement these yet: */
/*
		{"Cut", GTK_STOCK_CUT, NULL, NULL, NULL, NULL},
		{"Copy", GTK_STOCK_COPY, NULL, NULL, NULL, NULL},
		{"Paste", GTK_STOCK_PASTE, NULL, NULL, NULL, NULL},
 */
		{"Delete", GTK_STOCK_DELETE, NULL, NULL,
			Q_("Delete the contents of the selection"), NULL},
		{"SelectAll", GTK_STOCK_SELECT_ALL, NULL, NULL,
			Q_("Select all objects in the document"), NULL},

	/* TODO: View menu (zooming). */

	{"HelpMenu", NULL, Q_("_Help")},
		{"About", GTK_STOCK_ABOUT, NULL, NULL,
			Q_("Show a dialog about this application"),
			G_CALLBACK (on_action_about)}
};



/**
 * ld_window_main_new:
 *
 * Create an instance.
 */
GtkWidget *
ld_window_main_new (void)
{
	return g_object_new (LD_TYPE_WINDOW_MAIN, NULL);
}

static void
ld_window_main_class_init (LdWindowMainClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_window_main_finalize;

	widget_class = GTK_WIDGET_CLASS (klass);

	g_type_class_add_private (klass, sizeof (LdWindowMainPrivate));
}

static void
ld_window_main_init (LdWindowMain *self)
{
	LdWindowMainPrivate *priv;
	GtkActionGroup *action_group;
	GError *error;

	self->priv = priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_WINDOW_MAIN, LdWindowMainPrivate);

	/* Construct menu and toolbar. */
	priv->ui_manager = gtk_ui_manager_new ();

	g_signal_connect (priv->ui_manager, "connect-proxy",
		G_CALLBACK (on_ui_proxy_connected), self);
	g_signal_connect (priv->ui_manager, "disconnect-proxy",
		G_CALLBACK (on_ui_proxy_disconnected), self);

	action_group = gtk_action_group_new ("MainActions");
	gtk_action_group_add_actions (action_group, wm_action_entries,
		G_N_ELEMENTS (wm_action_entries), self);
	gtk_ui_manager_insert_action_group (priv->ui_manager, action_group, 0);

	error = NULL;
	gtk_ui_manager_add_ui_from_file
		(priv->ui_manager, PROJECT_SHARE_DIR "gui/window-main.ui", &error);
	if (error)
	{
		g_message (_("Building UI failed: %s"), error->message);
		g_error_free (error);
	}

	priv->menu = gtk_ui_manager_get_widget (priv->ui_manager, "/MenuBar");
	priv->toolbar = gtk_ui_manager_get_widget (priv->ui_manager, "/Toolbar");

	/* Create the remaining widgets. */
	priv->library_toolbar = gtk_toolbar_new ();
	/* XXX: For GTK 2.16+, s/toolbar/orientable/ */
	gtk_toolbar_set_orientation (GTK_TOOLBAR (priv->library_toolbar),
		GTK_ORIENTATION_VERTICAL);

	priv->canvas = ld_canvas_new ();
	priv->canvas_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (priv->canvas_window),
		GTK_WIDGET (priv->canvas));

	priv->statusbar = gtk_statusbar_new ();
	priv->statusbar_menu_context_id = gtk_statusbar_get_context_id
		(GTK_STATUSBAR (priv->statusbar), "menu");

	/* Pack all widgets into the window. */
	priv->hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (priv->hbox), priv->library_toolbar,
		FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (priv->hbox), priv->canvas_window,
		TRUE, TRUE, 0);

	priv->vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (priv->vbox), priv->menu, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (priv->vbox), priv->toolbar, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (priv->vbox), priv->hbox, TRUE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (priv->vbox), priv->statusbar, FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER (self), priv->vbox);

	/* Configure the window. */
	g_signal_connect (self, "destroy", G_CALLBACK (gtk_main_quit), NULL);

	gtk_window_add_accel_group (GTK_WINDOW (self),
		gtk_ui_manager_get_accel_group (priv->ui_manager));
	gtk_window_set_default_size (GTK_WINDOW (self), 500, 400);
	gtk_window_set_position (GTK_WINDOW (self), GTK_WIN_POS_CENTER);

	/* Hook canvas signals. */
	/* XXX: To be able to draw a symbol menu over the canvas, we may:
	 *   1. Hook the expose-event and button-{press,release}-event signals.
	 *   2. Create a hook mechanism in the LdCanvas object.
	 *     + The cairo context would not have to be created twice.
	 *     - More complex API.
	 */
	priv->symbol_menu.expose_handler = g_signal_connect (priv->canvas,
		"expose-event", G_CALLBACK (on_canvas_exposed), self);
	priv->symbol_menu.motion_notify_handler = g_signal_connect (priv->canvas,
		"motion-notify-event", G_CALLBACK (on_canvas_motion_notify), self);
	priv->symbol_menu.button_release_handler = g_signal_connect (priv->canvas,
		"button-release-event", G_CALLBACK (on_canvas_button_release), self);

	/* Don't process the signals yet. */
	g_signal_handler_block (priv->canvas,
		priv->symbol_menu.expose_handler);
	g_signal_handler_block (priv->canvas,
		priv->symbol_menu.motion_notify_handler);
	g_signal_handler_block (priv->canvas,
		priv->symbol_menu.button_release_handler);



	/* Initialize the backend. */
	priv->library = ld_library_new ();
	ld_library_load (priv->library, PROJECT_SHARE_DIR "library");

	ld_canvas_set_library (priv->canvas, priv->library);

	load_library_toolbar (self);

	/* Realize the window. */
	gtk_widget_show_all (GTK_WIDGET (self));
}

/*
 * ld_window_main_finalize:
 *
 * Dispose of all the resources owned by this window.
 */
static void
ld_window_main_finalize (GObject *gobject)
{
	LdWindowMain *self;

	self = LD_WINDOW_MAIN (gobject);

	/* Dispose of objects. Note that GtkObject has floating ref. by default
	 * and gtk_object_destroy () should be used for it.
	 */
	g_object_unref (self->priv->library);
	g_object_unref (self->priv->ui_manager);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_window_main_parent_class)->finalize (gobject);
}

/*
 * load_library_toolbar:
 *
 * Load symbols from the library into the library toolbar.
 */
static void
load_library_toolbar (LdWindowMain *self)
{
	GSList *categories;

	/* Clear the toolbar first, if there was already something in it. */
	gtk_container_foreach (GTK_CONTAINER (self->priv->library_toolbar),
		(GtkCallback) gtk_widget_destroy, NULL);

	categories = (GSList *) ld_library_get_children (self->priv->library);
	g_slist_foreach (categories, load_category_cb, self);
}

/*
 * load_category_cb:
 *
 * A foreach callback for adding categories into the library toolbar.
 */
static void
load_category_cb (gpointer data, gpointer user_data)
{
	LdWindowMain *self;
	LdSymbolCategory *cat;
	const gchar *human_name;
	GdkPixbuf *pbuf;
	GtkWidget *img;
	GtkToolItem *item;
	GtkWidget *button;

	g_return_if_fail (LD_IS_WINDOW_MAIN (user_data));
	g_return_if_fail (LD_IS_SYMBOL_CATEGORY (data));

	self = user_data;
	cat = data;

	human_name = ld_symbol_category_get_human_name (cat);

	pbuf = gdk_pixbuf_new_from_file_at_size	(ld_symbol_category_get_image_path
		(cat), LIBRARY_TOOLBAR_ICON_WIDTH, -1, NULL);
	g_return_if_fail (pbuf != NULL);

	img = gtk_image_new_from_pixbuf (pbuf);
	g_object_unref (pbuf);

	item = gtk_tool_item_new ();
	button = gtk_toggle_button_new ();
	gtk_container_add (GTK_CONTAINER (button), img);
	gtk_container_add (GTK_CONTAINER (item), button);

	/* Assign the category to the toggle button. */
	g_object_ref (cat);
	g_object_set_data_full (G_OBJECT (button),
		"category", cat, (GDestroyNotify) g_object_unref);

	/* Hook toggling of the button. */
	g_signal_connect (button, "toggled", G_CALLBACK (on_category_toggle), self);

	gtk_tool_item_set_tooltip_text (item, human_name);
	gtk_toolbar_insert (GTK_TOOLBAR (self->priv->library_toolbar), item, 0);
}

/*
 * redraw_symbol_menu:
 *
 * Make the area for symbol menu redraw itself.
 */
static void
redraw_symbol_menu (LdWindowMain *self)
{
	SymbolMenuData *data;

	g_return_if_fail (LD_IS_WINDOW_MAIN (self));
	data = &self->priv->symbol_menu;

	gtk_widget_queue_draw_area (GTK_WIDGET (self->priv->canvas),
		0, data->menu_y - 1, data->menu_width + 2, data->menu_height + 2);
}

/*
 * on_category_toggle:
 *
 * Show or hide a symbol menu.
 */
static void
on_category_toggle (GtkToggleButton *toggle_button, gpointer user_data)
{
	LdWindowMain *self;
	LdWindowMainPrivate *priv;
	LdSymbolCategory *cat;
	SymbolMenuData *data;

	g_return_if_fail (LD_IS_WINDOW_MAIN (user_data));

	cat = g_object_get_data (G_OBJECT (toggle_button), "category");
	self = LD_WINDOW_MAIN (user_data);
	priv = self->priv;
	data = &priv->symbol_menu;

	/* First untoggle any active button. */
	if (data->active_button)
		gtk_toggle_button_set_active (data->active_button, FALSE);

	/* And toggle signal handlers that enable the user to add a symbol. */
	if (data->active_button == toggle_button)
	{
		gint i;

		g_signal_handler_block (priv->canvas,
			priv->symbol_menu.expose_handler);
		g_signal_handler_block (priv->canvas,
			priv->symbol_menu.motion_notify_handler);
		g_signal_handler_block (priv->canvas,
			priv->symbol_menu.button_release_handler);

		g_object_unref (data->active_button);
		data->active_button = NULL;

		/* Ashes to ashes, NULL to NULL. */
		for (i = 0; i < data->n_items; i++)
			g_object_unref (data->items[i].symbol);

		g_free (data->items);
		data->items = NULL;
	}
	else
	{
		const GSList *children, *symbol_iter;
		SymbolMenuItem *item;
		gint x, y, menu_width;

		g_return_if_fail (gtk_widget_translate_coordinates (GTK_WIDGET
			(toggle_button), GTK_WIDGET (priv->canvas), 0, 0, &x, &y));

		data->menu_y = y;
		data->menu_height = GTK_WIDGET (toggle_button)->allocation.height;

		g_signal_handler_unblock (priv->canvas,
			priv->symbol_menu.expose_handler);
		g_signal_handler_unblock (priv->canvas,
			priv->symbol_menu.motion_notify_handler);
		g_signal_handler_unblock (priv->canvas,
			priv->symbol_menu.button_release_handler);

		data->active_button = toggle_button;
		g_object_ref (data->active_button);

		children = ld_symbol_category_get_children (cat);

		data->n_items = g_slist_length ((GSList *) children);
		data->items = g_new (SymbolMenuItem, data->n_items);
		data->active_item = -1;

		item = data->items;
		menu_width = 0;
		for (symbol_iter = children; symbol_iter;
			symbol_iter = symbol_iter->next)
		{
			LdSymbolArea area;

			item->symbol = LD_SYMBOL (symbol_iter->data);
			g_object_ref (item->symbol);

			ld_symbol_get_area (item->symbol, &area);

			/* This is the height when the center of the symbol is
			 * in the center of it's symbol menu item.
			 */
			item->scale = data->menu_height * 0.5
				/ MAX (ABS (area.y), ABS (area.y + area.height)) / 2;
			/* FIXME: The width is probably wrong (related to the center). */
			item->width = item->scale * area.width
				+ data->menu_height * 0.5;

			menu_width += item++->width;
		}
		data->menu_width = menu_width;
	}
	redraw_symbol_menu (self);
}

/*
 * on_canvas_exposed:
 *
 * Draw a symbol menu.
 */
static gboolean
on_canvas_exposed (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	cairo_t *cr;
	LdWindowMain *self;
	SymbolMenuData *data;
	gint i, x;

	cr = gdk_cairo_create (widget->window);
	self = LD_WINDOW_MAIN (user_data);
	data = &self->priv->symbol_menu;

	/* Draw some border. */
	cairo_set_line_width (cr, 1);

	cairo_rectangle (cr, 0, data->menu_y, data->menu_width, data->menu_height);
	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_fill (cr);

	/* Draw all symbols from that category. */
	for (x = i = 0; i < data->n_items; i++)
	{
		SymbolMenuItem *item;

		item = data->items + i;
		cairo_save (cr);

		cairo_rectangle (cr, x, data->menu_y, item->width, data->menu_height);
		cairo_clip (cr);

		if (i == data->active_item)
		{
			cairo_set_source_rgb (cr, 0.9, 0.9, 0.9);
			cairo_paint (cr);
		}

		cairo_translate (cr, x + (gdouble) item->width / 2,
			data->menu_y + (gdouble) data->menu_height / 2);
		cairo_scale (cr, item->scale, item->scale);

		cairo_set_source_rgb (cr, 0, 0, 0);
		cairo_set_line_width (cr, 1 / item->scale);
		ld_symbol_draw (item->symbol, cr);

		cairo_restore (cr);
		x += item->width;
	}

	cairo_rectangle (cr, 0, data->menu_y, data->menu_width, data->menu_height);
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_stroke (cr);

	cairo_destroy (cr);
	return FALSE;
}



static gboolean
on_canvas_motion_notify (GtkWidget *widget, GdkEventMotion *event,
	gpointer user_data)
{
	LdWindowMain *self;
	SymbolMenuData *data;
	gint i, x;

	self = LD_WINDOW_MAIN (user_data);
	data = &self->priv->symbol_menu;

	if (event->x < 0 || event->y < data->menu_y
		|| event->y >= data->menu_y + data->menu_height)
	{
		data->active_item = -1;
		redraw_symbol_menu (self);
		return FALSE;
	}

	for (x = i = 0; i < data->n_items; i++)
	{
		x += data->items[i].width;
		if (event->x < x)
		{
			/* TODO: Show the human name of this symbol in status bar. */
			data->active_item = i;
			redraw_symbol_menu (self);
			return FALSE;
		}
	}
	data->active_item = -1;
	redraw_symbol_menu (self);
	return FALSE;
}

static gboolean
on_canvas_button_release (GtkWidget *widget, GdkEventButton *event,
	gpointer user_data)
{
	LdWindowMain *self;
	SymbolMenuData *data;

	self = LD_WINDOW_MAIN (user_data);
	data = &self->priv->symbol_menu;

	if (event->button != 1)
		return FALSE;

	/* TODO: Add the selected symbol into the document on the position. */

	/* We've either chosen a symbol or canceled the menu, so hide it. */
	if (data->active_button)
		gtk_toggle_button_set_active (data->active_button, FALSE);

	return FALSE;
}

/*
 * on_ui_proxy_connected:
 *
 * An item was connected to the manager.
 */
static void
on_ui_proxy_connected (GtkUIManager *ui, GtkAction *action,
	GtkWidget *proxy, LdWindowMain *window)
{
	if (GTK_IS_MENU_ITEM (proxy))
	{
		g_signal_connect (proxy, "select",
			G_CALLBACK (on_menu_item_selected), window);
		g_signal_connect (proxy, "deselect",
			G_CALLBACK (on_menu_item_deselected), window);
	}
}

/*
 * on_ui_proxy_disconnected:
 *
 * An item was disconnected from the manager.
 */
static void
on_ui_proxy_disconnected (GtkUIManager *ui, GtkAction *action,
	GtkWidget *proxy, LdWindowMain *window)
{
	if (GTK_IS_MENU_ITEM (proxy))
	{
		g_signal_handlers_disconnect_by_func
			(proxy, G_CALLBACK (on_menu_item_selected), window);
		g_signal_handlers_disconnect_by_func
			(proxy, G_CALLBACK (on_menu_item_deselected), window);
	}
}

static void
on_menu_item_selected (GtkWidget *item, LdWindowMain *window)
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
on_menu_item_deselected (GtkItem *item, LdWindowMain *window)
{
	gtk_statusbar_pop (GTK_STATUSBAR (window->priv->statusbar),
		window->priv->statusbar_menu_context_id);
}

static void
on_action_about (GtkAction *action, LdWindowMain *self)
{
	gtk_show_about_dialog (GTK_WINDOW (self),
		"program-name", PROJECT_NAME,
		"version", PROJECT_VERSION,
		"copyright", "Copyright Přemysl Janouch 2010",
		NULL);
}

