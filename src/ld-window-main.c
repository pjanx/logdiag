/*
 * ld-window-main.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010 - 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>

#include "config.h"

#include "ld-window-main.h"

#include "ld-types.h"
#include "ld-symbol.h"
#include "ld-symbol-category.h"
#include "ld-library.h"

#include "ld-diagram-object.h"
#include "ld-diagram-symbol.h"
#include "ld-diagram.h"

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

/*
 * SymbolMenuItem:
 *
 * Data related to a symbol in an open symbol menu.
 */
typedef struct _SymbolMenuItem SymbolMenuItem;

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
typedef struct _SymbolMenuData SymbolMenuData;

struct _SymbolMenuData
{
	gulong expose_handler;
	gulong motion_notify_handler;
	gulong button_press_handler;
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
	GtkActionGroup *action_group;

	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *menu;
	GtkWidget *toolbar;
	GtkWidget *library_toolbar;

	LdLibrary *library;

	LdDiagram *diagram;
	gchar *filename;

	GtkWidget *canvas_window;
	LdCanvas *canvas;

	GtkWidget *statusbar;
	guint statusbar_menu_context_id;

	SymbolMenuData symbol_menu;
};

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
static gboolean on_canvas_button_press (GtkWidget *widget,
	GdkEventButton *event, gpointer user_data);
static gboolean on_canvas_button_release (GtkWidget *widget,
	GdkEventButton *event, gpointer user_data);

static void on_ui_proxy_connected (GtkUIManager *ui, GtkAction *action,
	GtkWidget *proxy, LdWindowMain *window);
static void on_ui_proxy_disconnected (GtkUIManager *ui, GtkAction *action,
	GtkWidget *proxy, LdWindowMain *window);

static void on_menu_item_selected (GtkWidget *item, LdWindowMain *window);
static void on_menu_item_deselected (GtkItem *item, LdWindowMain *window);

static gboolean on_delete (LdWindowMain *self, GdkEvent *event,
	gpointer user_data);
static void update_title (LdWindowMain *self);
static void action_set_sensitive (LdWindowMain *self, const gchar *name,
	gboolean sensitive);

static gchar *diagram_get_name (LdWindowMain *self);
static void diagram_set_filename (LdWindowMain *self, gchar *filename);
static void diagram_new (LdWindowMain *self);
static void diagram_save (LdWindowMain *self);

static GtkFileFilter *diagram_get_file_filter (void);
static void diagram_show_open_dialog (LdWindowMain *self);
static void diagram_show_save_as_dialog (LdWindowMain *self);

static gboolean may_close_diagram (LdWindowMain *self,
	const gchar *dialog_message);
static gboolean may_quit (LdWindowMain *self);

static void on_action_new (GtkAction *action, LdWindowMain *self);
static void on_action_open (GtkAction *action, LdWindowMain *self);
static void on_action_save (GtkAction *action, LdWindowMain *self);
static void on_action_save_as (GtkAction *action, LdWindowMain *self);
static void on_action_quit (GtkAction *action, LdWindowMain *self);
static void on_action_about (GtkAction *action, LdWindowMain *self);


/* ===== Local variables =================================================== */

/* Actions for menus, toolbars, accelerators. */
static GtkActionEntry wm_action_entries[] =
{
	{"FileMenu", NULL, Q_("_File"), NULL, NULL, NULL},
		{"New", GTK_STOCK_NEW, Q_("_New"), "<Ctrl>N",
			Q_("Create a new diagram"),
			G_CALLBACK (on_action_new)},
		{"Open", GTK_STOCK_OPEN, Q_("_Open..."), "<Ctrl>O",
			Q_("Open a diagram"),
			G_CALLBACK (on_action_open)},
		{"Save", GTK_STOCK_SAVE, Q_("_Save"), "<Ctrl>S",
			Q_("Save the current diagram"),
			G_CALLBACK (on_action_save)},
		{"SaveAs", GTK_STOCK_SAVE_AS, Q_("Save _As..."), "<Shift><Ctrl>S",
			Q_("Save the current diagram with another name"),
			G_CALLBACK (on_action_save_as)},
		{"Export", NULL, Q_("_Export"), NULL,
			Q_("Export the diagram"),
			NULL},
		{"Quit", GTK_STOCK_QUIT, Q_("_Quit"), "<Ctrl>Q",
			Q_("Quit the application"),
			G_CALLBACK (on_action_quit)},

	{"EditMenu", NULL, Q_("_Edit"), NULL, NULL, NULL},
		/* XXX: Don't implement these yet: */
/*
		{"Cut", GTK_STOCK_CUT, Q_("Cu_t"), "<Ctrl>X", NULL, NULL},
		{"Copy", GTK_STOCK_COPY, Q_("_Copy"), "<Ctrl>C", NULL, NULL},
		{"Paste", GTK_STOCK_PASTE, Q_("_Paste"), "<Ctrl>V", NULL, NULL},
 */
		{"Delete", GTK_STOCK_DELETE, Q_("_Delete"), "Delete",
			Q_("Delete the contents of the selection"),
			NULL},
		{"SelectAll", GTK_STOCK_SELECT_ALL, Q_("Select _All"), "<Ctrl>A",
			Q_("Select all objects in the diagram"),
			NULL},

	/* TODO: View menu (zooming). */

	{"HelpMenu", NULL, Q_("_Help"), NULL, NULL, NULL},
		{"About", GTK_STOCK_ABOUT, Q_("_About"), NULL,
			Q_("Show a dialog about this application"),
			G_CALLBACK (on_action_about)}
};


/* ===== Generic widget methods ============================================ */

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

G_DEFINE_TYPE (LdWindowMain, ld_window_main, GTK_TYPE_WINDOW);

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
	GError *error;

	self->priv = priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_WINDOW_MAIN, LdWindowMainPrivate);

	/* Construct menu and toolbar. */
	priv->ui_manager = gtk_ui_manager_new ();

	g_signal_connect (priv->ui_manager, "connect-proxy",
		G_CALLBACK (on_ui_proxy_connected), self);
	g_signal_connect (priv->ui_manager, "disconnect-proxy",
		G_CALLBACK (on_ui_proxy_disconnected), self);

	priv->action_group = gtk_action_group_new ("MainActions");
	gtk_action_group_add_actions (priv->action_group, wm_action_entries,
		G_N_ELEMENTS (wm_action_entries), self);
	gtk_ui_manager_insert_action_group (priv->ui_manager,
		priv->action_group, 0);

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
	g_signal_connect (self, "delete-event", G_CALLBACK (on_delete), NULL);

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
	priv->symbol_menu.button_press_handler = g_signal_connect (priv->canvas,
		"button-press-event", G_CALLBACK (on_canvas_button_press), self);
	priv->symbol_menu.button_release_handler = g_signal_connect (priv->canvas,
		"button-release-event", G_CALLBACK (on_canvas_button_release), self);

	/* Don't process the signals yet. */
	g_signal_handler_block (priv->canvas,
		priv->symbol_menu.expose_handler);
	g_signal_handler_block (priv->canvas,
		priv->symbol_menu.motion_notify_handler);
	g_signal_handler_block (priv->canvas,
		priv->symbol_menu.button_press_handler);
	g_signal_handler_block (priv->canvas,
		priv->symbol_menu.button_release_handler);

	/* Initialize the backend. */
	priv->diagram = ld_diagram_new ();

	g_signal_connect_data (priv->diagram, "changed",
		G_CALLBACK (update_title), self,
		NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);

	priv->library = ld_library_new ();
	ld_library_load (priv->library, PROJECT_SHARE_DIR "library");

	ld_canvas_set_diagram (priv->canvas, priv->diagram);
	ld_canvas_set_library (priv->canvas, priv->library);

	load_library_toolbar (self);
	diagram_set_filename (self, NULL);

	action_set_sensitive (self, "Export", FALSE);
	action_set_sensitive (self, "Delete", FALSE);
	action_set_sensitive (self, "SelectAll", FALSE);

	gtk_widget_grab_focus (GTK_WIDGET (priv->canvas));

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
	g_object_unref (self->priv->diagram);
	g_object_unref (self->priv->ui_manager);
	g_object_unref (self->priv->action_group);

	if (self->priv->filename)
		g_free (self->priv->filename);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_window_main_parent_class)->finalize (gobject);
}

/*
 * on_delete:
 *
 * Handle requests to close the window.
 */
static gboolean
on_delete (LdWindowMain *self, GdkEvent *event, gpointer user_data)
{
	return !may_quit (self);
}

/*
 * update_title:
 *
 * Update the title of the window.
 */
static void
update_title (LdWindowMain *self)
{
	gchar *title;
	gchar *name;

	g_return_if_fail (LD_IS_WINDOW_MAIN (self));

	name = diagram_get_name (self);
	title = g_strdup_printf ("%s%s - %s",
		ld_diagram_get_modified (self->priv->diagram) ? "*" : "",
		name, PROJECT_NAME);
	gtk_window_set_title (GTK_WINDOW (self), title);

	g_free (title);
	g_free (name);
}

/*
 * action_set_sensitive:
 * @sensitive: The sensitivity state.
 *
 * Set sensitivity of an action.
 */
static void
action_set_sensitive (LdWindowMain *self, const gchar *name, gboolean sensitive)
{
	GtkAction *action;

	g_return_if_fail (LD_IS_WINDOW_MAIN (self));
	g_return_if_fail (name != NULL);

	action = gtk_action_group_get_action (self->priv->action_group, name);
	g_return_if_fail (action != NULL);

	gtk_action_set_sensitive (action, sensitive);
}


/* ===== Library toolbar and symbol menu =================================== */

/*
 * load_library_toolbar:
 *
 * Load symbols from the library into the library toolbar.
 */
static void
load_library_toolbar (LdWindowMain *self)
{
	GSList *categories;

	g_return_if_fail (LD_IS_WINDOW_MAIN (self));

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

	/* Don't steal focus from the canvas. */
	g_object_set (button, "can-focus", FALSE, NULL);

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
			priv->symbol_menu.button_press_handler);
		g_signal_handler_block (priv->canvas,
			priv->symbol_menu.button_release_handler);

		g_object_unref (data->active_button);
		data->active_button = NULL;

		/* Ashes to ashes, NULL to NULL. */
		for (i = 0; i < data->n_items; i++)
			g_object_unref (data->items[i].symbol);

		g_free (data->items);
		data->items = NULL;

		gtk_grab_remove (GTK_WIDGET (self->priv->canvas));
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
			priv->symbol_menu.button_press_handler);
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
			LdRectangle area;

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

		gtk_grab_add (GTK_WIDGET (self->priv->canvas));
	}
	redraw_symbol_menu (self);
}

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

	if (widget->window != event->window
		|| event->x < 0 || event->y < data->menu_y
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
on_canvas_button_press (GtkWidget *widget, GdkEventButton *event,
	gpointer user_data)
{
	LdWindowMain *self;
	SymbolMenuData *data;

	self = LD_WINDOW_MAIN (user_data);
	data = &self->priv->symbol_menu;

	/* If the event occured elsewhere, cancel the menu and put the event
	 * back into the queue.
	 */
	if (widget->window != event->window && data->active_button)
	{
		gtk_toggle_button_set_active (data->active_button, FALSE);
		gdk_event_put ((GdkEvent *) event);
	}
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

	if (data->active_item != -1)
	{
		LdDiagramSymbol *symbol;
		const gchar *category_name, *symbol_name;
		gchar *klass;

		category_name = ld_symbol_category_get_name
			(g_object_get_data (G_OBJECT (data->active_button), "category"));
		symbol_name = ld_symbol_get_name
			(data->items[data->active_item].symbol);

		klass = g_build_path (LD_LIBRARY_IDENTIFIER_SEPARATOR,
			category_name, symbol_name, NULL);
		symbol = ld_diagram_symbol_new (klass);
		g_free (klass);

		ld_canvas_add_object_begin (self->priv->canvas,
			LD_DIAGRAM_OBJECT (symbol));
	}

	/* We've either chosen a symbol or canceled the menu, so hide it. */
	if (data->active_button)
		gtk_toggle_button_set_active (data->active_button, FALSE);

	return FALSE;
}


/* ===== Menu items processing ============================================= */

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


/* ===== Diagram handling ================================================== */

/*
 * diagram_get_name:
 *
 * Get the name of the currently opened diagram.
 */
static gchar *
diagram_get_name (LdWindowMain *self)
{
	g_return_val_if_fail (LD_IS_WINDOW_MAIN (self), NULL);

	if (self->priv->filename)
		return g_path_get_basename (self->priv->filename);
	else
		return g_strdup (_("Unsaved Diagram"));
}

/*
 * diagram_set_filename:
 * @filename: The new filename. May be NULL for a new, yet unsaved, file.
 *
 * Set the filename corresponding to the currently opened diagram.
 * The function takes ownership of the string.
 */
static void
diagram_set_filename (LdWindowMain *self, gchar *filename)
{
	g_return_if_fail (LD_IS_WINDOW_MAIN (self));

	if (self->priv->filename)
		g_free (self->priv->filename);
	self->priv->filename = filename;

	update_title (self);
}

/*
 * diagram_new:
 *
 * Create a new diagram.
 */
static void
diagram_new (LdWindowMain *self)
{
	g_return_if_fail (LD_IS_WINDOW_MAIN (self));

	if (!may_close_diagram (self, "Save the changes to diagram \"%s\" before"
		" closing it and creating a new one?"))
		return;

	/* TODO: Reset canvas view to the center. */
	ld_diagram_clear (self->priv->diagram);
	ld_diagram_set_modified (self->priv->diagram, FALSE);

	diagram_set_filename (self, NULL);
}

/*
 * diagram_save:
 *
 * Save the current diagram.
 */
static void
diagram_save (LdWindowMain *self)
{
	GError *error;

	g_return_if_fail (LD_IS_WINDOW_MAIN (self));

	if (!self->priv->filename)
	{
		diagram_show_save_as_dialog (self);
		return;
	}

	error = NULL;
	ld_diagram_save_to_file (self->priv->diagram,
		self->priv->filename, &error);
	if (error)
	{
		GtkWidget *message_dialog;

		g_warning ("Saving failed: %s", error->message);
		g_error_free (error);

		message_dialog = gtk_message_dialog_new (GTK_WINDOW (self),
			GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
			"Failed to save the diagram");
		gtk_message_dialog_format_secondary_text
			(GTK_MESSAGE_DIALOG (message_dialog),
			"Try again or save it under another name.");
		gtk_dialog_run (GTK_DIALOG (message_dialog));
		gtk_widget_destroy (message_dialog);
	}
	else
	{
		ld_diagram_set_modified (self->priv->diagram, FALSE);
		update_title (self);
	}
}

/*
 * diagram_get_file_filter:
 *
 * Return value: A new #GtkFileFilter object for diagrams.
 */
static GtkFileFilter *
diagram_get_file_filter (void)
{
	GtkFileFilter *filter;

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, "Logdiag Diagrams");
	gtk_file_filter_add_pattern (filter, "*.ldd");
	return filter;
}

/*
 * diagram_show_open_dialog:
 *
 * Show a dialog for opening a diagram.
 */
static void
diagram_show_open_dialog (LdWindowMain *self)
{
	GtkWidget *dialog;

	g_return_if_fail (LD_IS_WINDOW_MAIN (self));

	if (!may_close_diagram (self, "Save the changes to diagram \"%s\" before"
		" closing it and opening another one?"))
		return;

	dialog = gtk_file_chooser_dialog_new ("Open...", GTK_WINDOW (self),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog),
		diagram_get_file_filter ());

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *filename;
		GError *error;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		error = NULL;
		ld_diagram_load_from_file (self->priv->diagram, filename, &error);
		if (error)
		{
			GtkWidget *message_dialog;

			g_warning ("Loading failed: %s", error->message);
			g_error_free (error);

			message_dialog = gtk_message_dialog_new (GTK_WINDOW (self),
				GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				"Failed to open the file");
			gtk_message_dialog_format_secondary_text
				(GTK_MESSAGE_DIALOG (message_dialog),
				"The file is probably corrupted.");
			gtk_dialog_run (GTK_DIALOG (message_dialog));
			gtk_widget_destroy (message_dialog);
		}
		else
		{
			ld_diagram_set_modified (self->priv->diagram, FALSE);
			diagram_set_filename (self, filename);
		}
	}
	gtk_widget_destroy (dialog);
}

/*
 * diagram_show_save_as_dialog:
 *
 * Show a dialog for saving the diagram.
 */
static void
diagram_show_save_as_dialog (LdWindowMain *self)
{
	GtkWidget *dialog;

	g_return_if_fail (LD_IS_WINDOW_MAIN (self));

	dialog = gtk_file_chooser_dialog_new ("Save As...", GTK_WINDOW (self),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);
	g_object_set (dialog, "do-overwrite-confirmation", TRUE, NULL);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog),
		diagram_get_file_filter ());

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		diagram_set_filename (self, gtk_file_chooser_get_filename
			(GTK_FILE_CHOOSER (dialog)));
		diagram_save (self);
	}
	gtk_widget_destroy (dialog);
}

/*
 * may_close_diagram:
 * @dialog_message: The dialog message to display to the user if the diagram
 *                  has been modified.
 *
 * When no changes have been made to the current diagram, the function
 * lets the caller proceed. Otherwise the user is asked for further actions.
 * If he chooses to save the diagram, the function will handle this action.
 *
 * Return value: FALSE if the current action should be cancelled.
 *               TRUE if the caller may proceed.
 */
static gboolean
may_close_diagram (LdWindowMain *self, const gchar *dialog_message)
{
	GtkWidget *message_dialog;
	gchar *name;
	gint result;

	g_return_val_if_fail (LD_IS_WINDOW_MAIN (self), TRUE);
	g_return_val_if_fail (dialog_message != NULL, TRUE);

	if (!ld_diagram_get_modified (self->priv->diagram))
		return TRUE;

	name = diagram_get_name (self);

	/* TODO: Show the time since the diagram was last saved.
	 *       (Record the event with g_get_current_time().)
	 */
	message_dialog = gtk_message_dialog_new (GTK_WINDOW (self),
		GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
		dialog_message, name);
	gtk_message_dialog_format_secondary_text
		(GTK_MESSAGE_DIALOG (message_dialog),
		"If you don't save, changes will be permanently lost.");
	gtk_dialog_add_buttons (GTK_DIALOG (message_dialog),
		"Close _without Saving", GTK_RESPONSE_NO,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_YES,
		NULL);

	result = gtk_dialog_run (GTK_DIALOG (message_dialog));
	gtk_widget_destroy (message_dialog);
	g_free (name);

	switch (result)
	{
	case GTK_RESPONSE_NO:
		return TRUE;
	case GTK_RESPONSE_YES:
		diagram_save (self);
		return TRUE;
	case GTK_RESPONSE_CANCEL:
	case GTK_RESPONSE_DELETE_EVENT:
		return FALSE;
	default:
		g_assert_not_reached ();
	}
}

/*
 * may_quit:
 *
 * A variant on may_close_diagram() for the occasion of closing
 * the whole application.
 *
 * Return value: TRUE if the application may quit, FALSE otherwise.
 */
static gboolean
may_quit (LdWindowMain *self)
{
	g_return_val_if_fail (LD_IS_WINDOW_MAIN (self), TRUE);

	return may_close_diagram (self,
		"Save the changes to diagram \"%s\" before closing?");
}


/* ===== User interface actions ============================================ */

static void
on_action_new (GtkAction *action, LdWindowMain *self)
{
	diagram_new (self);
}

static void
on_action_open (GtkAction *action, LdWindowMain *self)
{
	diagram_show_open_dialog (self);
}

static void
on_action_save (GtkAction *action, LdWindowMain *self)
{
	diagram_save (self);
}

static void
on_action_save_as (GtkAction *action, LdWindowMain *self)
{
	diagram_show_save_as_dialog (self);
}

static void
on_action_quit (GtkAction *action, LdWindowMain *self)
{
	if (may_quit (self))
		gtk_widget_destroy (GTK_WIDGET (self));
}

static void
on_action_about (GtkAction *action, LdWindowMain *self)
{
	gtk_show_about_dialog (GTK_WINDOW (self),
		"program-name", PROJECT_NAME,
		"version", PROJECT_VERSION,
		"copyright", "Copyright Přemysl Janouch 2010 - 2011",
		NULL);
}

