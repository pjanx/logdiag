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

#include <liblogdiag/liblogdiag.h>
#include "config.h"

#include "ld-window-main.h"


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
	guint statusbar_symbol_context_id;
	guint statusbar_menu_context_id;
};


/* ===== Local functions =================================================== */

static void ld_window_main_finalize (GObject *gobject);

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

static void on_canvas_zoom_changed (LdCanvas *canvas,
	GParamSpec *pspec, LdWindowMain *self);

static void on_diagram_changed (LdDiagram *diagram, LdWindowMain *self);
static void on_diagram_history_changed (LdDiagram *diagram,
	GParamSpec *pspec, LdWindowMain *self);
static void on_diagram_selection_changed (LdDiagram *diagram,
	LdWindowMain *self);

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

static void on_symbol_selected (LdLibraryToolbar *toolbar, LdSymbol *symbol,
	const gchar *klass, LdWindowMain *self);
static void on_symbol_deselected (LdLibraryToolbar *toolbar, LdSymbol *symbol,
	const gchar *klass, LdWindowMain *self);
static void on_symbol_chosen (LdLibraryToolbar *toolbar, LdSymbol *symbol,
	const gchar *klass, LdWindowMain *self);

static void on_action_new (GtkAction *action, LdWindowMain *self);
static void on_action_open (GtkAction *action, LdWindowMain *self);
static void on_action_save (GtkAction *action, LdWindowMain *self);
static void on_action_save_as (GtkAction *action, LdWindowMain *self);
static void on_action_quit (GtkAction *action, LdWindowMain *self);
static void on_action_about (GtkAction *action, LdWindowMain *self);

static void on_action_undo (GtkAction *action, LdWindowMain *self);
static void on_action_redo (GtkAction *action, LdWindowMain *self);
static void on_action_delete (GtkAction *action, LdWindowMain *self);
static void on_action_select_all (GtkAction *action, LdWindowMain *self);

static void on_action_zoom_in (GtkAction *action, LdWindowMain *self);
static void on_action_zoom_out (GtkAction *action, LdWindowMain *self);
static void on_action_normal_size (GtkAction *action, LdWindowMain *self);


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
/*
 *		{"Export", NULL, Q_("_Export"), NULL,
 *			Q_("Export the diagram"),
 *			NULL},
 */
		{"Quit", GTK_STOCK_QUIT, Q_("_Quit"), "<Ctrl>Q",
			Q_("Quit the application"),
			G_CALLBACK (on_action_quit)},

	{"EditMenu", NULL, Q_("_Edit"), NULL, NULL, NULL},
		{"Undo", GTK_STOCK_UNDO, Q_("_Undo"), "<Ctrl>Z",
			Q_("Undo the last action"),
			G_CALLBACK (on_action_undo)},
		{"Redo", GTK_STOCK_REDO, Q_("_Redo"), "<Shift><Ctrl>Z",
			Q_("Redo the last undone action"),
			G_CALLBACK (on_action_redo)},
/*
 *		{"Cut", GTK_STOCK_CUT, Q_("Cu_t"), "<Ctrl>X", NULL, NULL},
 *		{"Copy", GTK_STOCK_COPY, Q_("_Copy"), "<Ctrl>C", NULL, NULL},
 *		{"Paste", GTK_STOCK_PASTE, Q_("_Paste"), "<Ctrl>V", NULL, NULL},
 */
		{"Delete", GTK_STOCK_DELETE, Q_("_Delete"), "Delete",
			Q_("Delete the contents of the selection"),
			G_CALLBACK (on_action_delete)},
		{"SelectAll", GTK_STOCK_SELECT_ALL, Q_("Select _All"), "<Ctrl>A",
			Q_("Select all objects in the diagram"),
			G_CALLBACK (on_action_select_all)},

	{"ViewMenu", NULL, Q_("_View"), NULL, NULL, NULL},
		{"ZoomIn", GTK_STOCK_ZOOM_IN, Q_("_Zoom In"), "<Ctrl>plus",
			Q_("Zoom into the diagram"),
			G_CALLBACK (on_action_zoom_in)},
		{"ZoomOut", GTK_STOCK_ZOOM_OUT, Q_("Zoom _Out"), "<Ctrl>minus",
			Q_("Zoom out of the diagram"),
			G_CALLBACK (on_action_zoom_out)},
		{"NormalSize", GTK_STOCK_ZOOM_100, Q_("_Normal Size"), "<Ctrl>0",
			Q_("Reset zoom level back to the default"),
			G_CALLBACK (on_action_normal_size)},

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
	priv->library_toolbar = ld_library_toolbar_new ();
	/* XXX: For GTK 2.16+, s/toolbar/orientable/ */
	gtk_toolbar_set_orientation (GTK_TOOLBAR (priv->library_toolbar),
		GTK_ORIENTATION_VERTICAL);

	priv->canvas = LD_CANVAS (ld_canvas_new ());
	priv->canvas_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (priv->canvas_window),
		GTK_WIDGET (priv->canvas));

	priv->statusbar = gtk_statusbar_new ();
	priv->statusbar_menu_context_id = gtk_statusbar_get_context_id
		(GTK_STATUSBAR (priv->statusbar), "menu");
	priv->statusbar_symbol_context_id = gtk_statusbar_get_context_id
		(GTK_STATUSBAR (priv->statusbar), "symbol");

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

	/* Initialize the backend. */
	priv->diagram = ld_diagram_new ();

	g_signal_connect_after (priv->diagram, "changed",
		G_CALLBACK (on_diagram_changed), self);
	g_signal_connect (priv->diagram, "notify::can-undo",
		G_CALLBACK (on_diagram_history_changed), self);
	g_signal_connect (priv->diagram, "notify::can-redo",
		G_CALLBACK (on_diagram_history_changed), self);
	g_signal_connect_after (priv->diagram, "selection-changed",
		G_CALLBACK (on_diagram_selection_changed), self);

	priv->library = ld_library_new ();
	ld_library_load (priv->library, PROJECT_SHARE_DIR "library");

	ld_canvas_set_diagram (priv->canvas, priv->diagram);
	ld_canvas_set_library (priv->canvas, priv->library);

	g_signal_connect (priv->canvas, "notify::zoom",
		G_CALLBACK (on_canvas_zoom_changed), self);

	ld_library_toolbar_set_library (LD_LIBRARY_TOOLBAR (priv->library_toolbar),
		priv->library);
	ld_library_toolbar_set_canvas (LD_LIBRARY_TOOLBAR (priv->library_toolbar),
		priv->canvas);

	g_signal_connect_after (priv->library_toolbar, "symbol-selected",
		G_CALLBACK (on_symbol_selected), self);
	g_signal_connect_after (priv->library_toolbar, "symbol-deselected",
		G_CALLBACK (on_symbol_deselected), self);
	g_signal_connect_after (priv->library_toolbar, "symbol-chosen",
		G_CALLBACK (on_symbol_chosen), self);

	diagram_set_filename (self, NULL);

	action_set_sensitive (self, "Undo", FALSE);
	action_set_sensitive (self, "Redo", FALSE);
	action_set_sensitive (self, "Delete", FALSE);
	action_set_sensitive (self, "NormalSize", FALSE);

	gtk_widget_grab_focus (GTK_WIDGET (priv->canvas));

	/* Realize the window. */
	gtk_widget_show_all (GTK_WIDGET (self));
}

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
 * @sensitive: the sensitivity state.
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

static void
on_diagram_changed (LdDiagram *diagram, LdWindowMain *self)
{
	update_title (self);
}

static void
on_diagram_history_changed (LdDiagram *diagram,
	GParamSpec *pspec, LdWindowMain *self)
{
	action_set_sensitive (self, "Undo", ld_diagram_can_undo (diagram));
	action_set_sensitive (self, "Redo", ld_diagram_can_redo (diagram));
}

static void
on_diagram_selection_changed (LdDiagram *diagram, LdWindowMain *self)
{
	gboolean selection_empty;

	selection_empty = !ld_diagram_get_selection (diagram);
	action_set_sensitive (self, "Delete", !selection_empty);
}

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
 * @filename: (allow-none): the new filename. %NULL for a new file.
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

	ld_diagram_clear (self->priv->diagram);
	ld_diagram_set_modified (self->priv->diagram, FALSE);

	/* TODO: Reset canvas view to the center. */
	ld_canvas_set_zoom (self->priv->canvas, 1);

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

	/* FIXME: If this fails, we still retain the filename. */
	error = NULL;
	ld_diagram_save_to_file (self->priv->diagram,
		self->priv->filename, &error);
	if (error)
	{
		GtkWidget *message_dialog;

		g_warning ("saving failed: %s", error->message);
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
 * Return value: a new #GtkFileFilter object for diagrams.
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

			g_warning ("loading failed: %s", error->message);
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
 * @dialog_message: the dialog message to display to the user if the diagram
 *                  has been modified.
 *
 * When no changes have been made to the current diagram, the function
 * lets the caller proceed. Otherwise the user is asked for further actions.
 * If he chooses to save the diagram, the function will handle this action.
 *
 * Return value: %FALSE if the current action should be cancelled.
 *               %TRUE if the caller may proceed.
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
 * Return value: %TRUE if the application may quit, %FALSE otherwise.
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
on_symbol_selected (LdLibraryToolbar *toolbar, LdSymbol *symbol,
	const gchar *klass, LdWindowMain *self)
{
	const gchar *symbol_name;

	symbol_name = ld_symbol_get_human_name (symbol);
	gtk_statusbar_push (GTK_STATUSBAR (self->priv->statusbar),
		self->priv->statusbar_menu_context_id, symbol_name);
}

static void
on_symbol_deselected (LdLibraryToolbar *toolbar, LdSymbol *symbol,
	const gchar *klass, LdWindowMain *self)
{
	gtk_statusbar_pop (GTK_STATUSBAR (self->priv->statusbar),
		self->priv->statusbar_menu_context_id);
}

static void
on_symbol_chosen (LdLibraryToolbar *toolbar, LdSymbol *symbol,
	const gchar *klass, LdWindowMain *self)
{
	LdDiagramSymbol *diagram_symbol;

	diagram_symbol = ld_diagram_symbol_new (NULL);
	ld_diagram_symbol_set_class (diagram_symbol, klass);

	ld_canvas_add_object_begin (self->priv->canvas,
		LD_DIAGRAM_OBJECT (diagram_symbol));
}

static void
on_canvas_zoom_changed (LdCanvas *canvas, GParamSpec *pspec, LdWindowMain *self)
{
	action_set_sensitive (self, "ZoomIn",
		ld_canvas_can_zoom_in (self->priv->canvas));
	action_set_sensitive (self, "ZoomOut",
		ld_canvas_can_zoom_out (self->priv->canvas));
	action_set_sensitive (self, "NormalSize",
		ld_canvas_get_zoom (self->priv->canvas) != 1);
}

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
		"website", PROJECT_URL,
		NULL);
}

static void
on_action_undo (GtkAction *action, LdWindowMain *self)
{
	ld_diagram_undo (self->priv->diagram);
}

static void
on_action_redo (GtkAction *action, LdWindowMain *self)
{
	ld_diagram_redo (self->priv->diagram);
}

static void
on_action_delete (GtkAction *action, LdWindowMain *self)
{
	ld_diagram_remove_selection (self->priv->diagram);
}

static void
on_action_select_all (GtkAction *action, LdWindowMain *self)
{
	ld_diagram_select_all (self->priv->diagram);
}

static void
on_action_zoom_in (GtkAction *action, LdWindowMain *self)
{
	ld_canvas_zoom_in (self->priv->canvas);
}

static void
on_action_zoom_out (GtkAction *action, LdWindowMain *self)
{
	ld_canvas_zoom_out (self->priv->canvas);
}

static void
on_action_normal_size (GtkAction *action, LdWindowMain *self)
{
	ld_canvas_set_zoom (self->priv->canvas, 1);
}
