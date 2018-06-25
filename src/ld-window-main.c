/*
 * ld-window-main.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010, 2011, 2012
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <liblogdiag/liblogdiag.h>
#include "config.h"

#include "ld-window-main.h"


struct _LdWindowMainPrivate
{
	GSettings *settings;
	GtkUIManager *ui_manager;
	GtkActionGroup *action_group;

	GtkWidget *vbox;
	GtkWidget *paned;
	GtkWidget *menu;
	GtkWidget *toolbar;

	GtkWidget *library_view;
	GtkWidget *lv_window;
	GtkWidget *lv_viewport;

	LdLibrary *library;

	LdDiagram *diagram;
	gchar *filename;

	GtkWidget *scrolled_window;
	LdDiagramView *view;

	GtkWidget *statusbar;
	GtkWidget *zoom_label;
	guint statusbar_symbol_context_id;
	guint statusbar_menu_context_id;
	guint statusbar_hint_context_id;

	guint statusbar_hint_drag;
};


/* ===== Local functions =================================================== */

static void ld_window_main_finalize (GObject *gobject);
static void load_library_directories (LdLibrary *library);

static void on_ui_proxy_connected (GtkUIManager *ui, GtkAction *action,
	GtkWidget *proxy, LdWindowMain *window);
static void on_ui_proxy_disconnected (GtkUIManager *ui, GtkAction *action,
	GtkWidget *proxy, LdWindowMain *window);

static void on_menu_item_selected (GtkWidget *item, LdWindowMain *window);
static void on_menu_item_deselected (GtkMenuItem *item, LdWindowMain *window);

static gboolean on_delete (LdWindowMain *self, GdkEvent *event,
	gpointer user_data);
static void update_title (LdWindowMain *self);
static void action_set_sensitive (LdWindowMain *self, const gchar *name,
	gboolean sensitive);

static void on_view_zoom_changed (LdDiagramView *view,
	GParamSpec *pspec, LdWindowMain *self);

static void on_diagram_changed (LdDiagram *diagram, LdWindowMain *self);
static void on_diagram_history_changed (LdDiagram *diagram,
	GParamSpec *pspec, LdWindowMain *self);
static void on_diagram_selection_changed (LdDiagram *diagram,
	LdWindowMain *self);

static gchar *diagram_get_name (LdWindowMain *self);
static void diagram_set_filename (LdWindowMain *self, gchar *filename);
static void diagram_new (LdWindowMain *self);
static gboolean diagram_open (LdWindowMain *self, const gchar *filename);
static gboolean diagram_save (LdWindowMain *self, GtkWindow *dialog_parent,
	const gchar *filename);

static GtkFileFilter *diagram_get_file_filter (void);
static void diagram_show_open_dialog (LdWindowMain *self);
static void diagram_show_save_as_dialog (LdWindowMain *self);

static gboolean may_close_diagram (LdWindowMain *self,
	const gchar *dialog_message);
static gboolean may_quit (LdWindowMain *self);

static void on_symbol_selected (LdCategoryView *view,
	LdSymbol *symbol, const gchar *path, LdWindowMain *self);
static void on_symbol_deselected (LdCategoryView *view,
	LdSymbol *symbol, const gchar *path, LdWindowMain *self);

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

static void on_action_main_toolbar (GtkToggleAction *action,
	LdWindowMain *self);
static void on_action_library_pane (GtkToggleAction *action,
	LdWindowMain *self);
static void on_action_grid (GtkToggleAction *action, LdWindowMain *self);

static void on_action_zoom_in (GtkAction *action, LdWindowMain *self);
static void on_action_zoom_out (GtkAction *action, LdWindowMain *self);
static void on_action_normal_size (GtkAction *action, LdWindowMain *self);


/* ===== Local variables =================================================== */

/* Actions for menus, toolbars, accelerators. */
static GtkActionEntry wm_action_entries[] =
{
	{"FileMenu", NULL, N_("_File"), NULL, NULL, NULL},
		{"New", GTK_STOCK_NEW, N_("_New"), "<Ctrl>N",
			N_("Create a new diagram"),
			G_CALLBACK (on_action_new)},
		{"Open", GTK_STOCK_OPEN, N_("_Open..."), "<Ctrl>O",
			N_("Open a diagram"),
			G_CALLBACK (on_action_open)},
		{"Save", GTK_STOCK_SAVE, N_("_Save"), "<Ctrl>S",
			N_("Save the current diagram"),
			G_CALLBACK (on_action_save)},
		{"SaveAs", GTK_STOCK_SAVE_AS, N_("Save _As..."), "<Shift><Ctrl>S",
			N_("Save the current diagram with another name"),
			G_CALLBACK (on_action_save_as)},
/*
 *		{"Export", NULL, N_("_Export"), NULL,
 *			N_("Export the diagram"),
 *			NULL},
 */
		{"Quit", GTK_STOCK_QUIT, N_("_Quit"), "<Ctrl>Q",
			N_("Quit the application"),
			G_CALLBACK (on_action_quit)},

	{"EditMenu", NULL, N_("_Edit"), NULL, NULL, NULL},
		{"Undo", GTK_STOCK_UNDO, N_("_Undo"), "<Ctrl>Z",
			N_("Undo the last action"),
			G_CALLBACK (on_action_undo)},
		{"Redo", GTK_STOCK_REDO, N_("_Redo"), "<Shift><Ctrl>Z",
			N_("Redo the last undone action"),
			G_CALLBACK (on_action_redo)},
/*
 *		{"Cut", GTK_STOCK_CUT, N_("Cu_t"), "<Ctrl>X", NULL, NULL},
 *		{"Copy", GTK_STOCK_COPY, N_("_Copy"), "<Ctrl>C", NULL, NULL},
 *		{"Paste", GTK_STOCK_PASTE, N_("_Paste"), "<Ctrl>V", NULL, NULL},
 */
		{"Delete", GTK_STOCK_DELETE, N_("_Delete"), "Delete",
			N_("Delete the contents of the selection"),
			G_CALLBACK (on_action_delete)},
		{"SelectAll", GTK_STOCK_SELECT_ALL, N_("Select _All"), "<Ctrl>A",
			N_("Select all objects in the diagram"),
			G_CALLBACK (on_action_select_all)},

	{"ViewMenu", NULL, N_("_View"), NULL, NULL, NULL},
		{"ZoomIn", GTK_STOCK_ZOOM_IN, N_("_Zoom In"), "<Ctrl>plus",
			N_("Zoom into the diagram"),
			G_CALLBACK (on_action_zoom_in)},
		{"ZoomOut", GTK_STOCK_ZOOM_OUT, N_("Zoom _Out"), "<Ctrl>minus",
			N_("Zoom out of the diagram"),
			G_CALLBACK (on_action_zoom_out)},
		{"NormalSize", GTK_STOCK_ZOOM_100, N_("_Normal Size"), "<Ctrl>0",
			N_("Reset zoom level back to the default"),
			G_CALLBACK (on_action_normal_size)},

	{"HelpMenu", NULL, N_("_Help"), NULL, NULL, NULL},
		{"About", GTK_STOCK_ABOUT, N_("_About"), NULL,
			N_("Show a dialog about this application"),
			G_CALLBACK (on_action_about)}
};

static GtkToggleActionEntry wm_toggle_action_entries[] =
{
	{"MainToolbar", NULL, N_("_Main Toolbar"), NULL,
		N_("Toggle displaying of the main toolbar"),
		G_CALLBACK (on_action_main_toolbar), TRUE},
	{"LibraryPane", NULL, N_("_Library Pane"), NULL,
		N_("Toggle displaying of the library pane"),
		G_CALLBACK (on_action_library_pane), TRUE},
	{"ShowGrid", NULL, N_("Show _Grid"), NULL,
		N_("Toggle displaying of the grid"),
		G_CALLBACK (on_action_grid), TRUE}
};


/* ===== Generic widget methods ============================================ */

/**
 * ld_window_main_new:
 * @filename: (allow-none): a file to open.
 *
 * Create an instance.
 */
GtkWidget *
ld_window_main_new (const gchar *filename)
{
	GtkWidget *self;
	self = g_object_new (LD_TYPE_WINDOW_MAIN, NULL);

	if (filename)
		diagram_open (LD_WINDOW_MAIN (self), filename);

	return self;
}

G_DEFINE_TYPE (LdWindowMain, ld_window_main, GTK_TYPE_WINDOW);

static void
ld_window_main_class_init (LdWindowMainClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_window_main_finalize;

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
	gtk_action_group_set_translation_domain
		(priv->action_group, GETTEXT_DOMAIN);
	gtk_action_group_add_actions
		(priv->action_group, wm_action_entries,
		G_N_ELEMENTS (wm_action_entries), self);
	gtk_action_group_add_toggle_actions
		(priv->action_group, wm_toggle_action_entries,
		G_N_ELEMENTS (wm_toggle_action_entries), self);
	gtk_ui_manager_insert_action_group (priv->ui_manager,
		priv->action_group, 0);

	error = NULL;
	gtk_ui_manager_add_ui_from_file
		(priv->ui_manager, PROJECT_SHARE_DIR "gui/window-main.ui", &error);
	if (error)
	{
		g_message ("building UI failed: %s", error->message);
		g_error_free (error);
	}

	priv->menu = gtk_ui_manager_get_widget (priv->ui_manager, "/MenuBar");
	priv->toolbar = gtk_ui_manager_get_widget (priv->ui_manager, "/Toolbar");

	/* Create the remaining widgets. */
	priv->library_view = ld_category_tree_view_new (NULL);

	priv->view = LD_DIAGRAM_VIEW (ld_diagram_view_new ());
	priv->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (priv->scrolled_window),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (priv->scrolled_window),
		GTK_WIDGET (priv->view));

	priv->statusbar = gtk_statusbar_new ();
	priv->statusbar_menu_context_id = gtk_statusbar_get_context_id
		(GTK_STATUSBAR (priv->statusbar), "menu");
	priv->statusbar_symbol_context_id = gtk_statusbar_get_context_id
		(GTK_STATUSBAR (priv->statusbar), "symbol");
	priv->statusbar_hint_context_id = gtk_statusbar_get_context_id
		(GTK_STATUSBAR (priv->statusbar), "hint");

	priv->lv_viewport = gtk_viewport_new (NULL, NULL);
	gtk_viewport_set_shadow_type
		(GTK_VIEWPORT (priv->lv_viewport), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (priv->lv_viewport), priv->library_view);

	priv->lv_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (priv->lv_window),
		GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (priv->lv_window), priv->lv_viewport);

	priv->paned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_paned_pack1 (GTK_PANED (priv->paned),
		priv->lv_window, FALSE, FALSE);
	gtk_paned_pack2 (GTK_PANED (priv->paned),
		priv->scrolled_window, TRUE, TRUE);
	gtk_paned_set_position (GTK_PANED (priv->paned), 180);

	/* Pack all widgets into the window. */
	priv->vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start (GTK_BOX (priv->vbox), priv->menu, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (priv->vbox), priv->toolbar, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (priv->vbox), priv->paned, TRUE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (priv->vbox), priv->statusbar, FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER (self), priv->vbox);

	/* Configure the window. */
	g_signal_connect (self, "delete-event", G_CALLBACK (on_delete), NULL);

	gtk_window_add_accel_group (GTK_WINDOW (self),
		gtk_ui_manager_get_accel_group (priv->ui_manager));
	gtk_window_set_default_size (GTK_WINDOW (self), 640, 440);
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
	load_library_directories (priv->library);

	ld_diagram_view_set_diagram (priv->view, priv->diagram);
	ld_diagram_view_set_library (priv->view, priv->library);

	ld_category_view_set_category (LD_CATEGORY_VIEW (priv->library_view),
		ld_library_get_root (priv->library));

	g_signal_connect_after (priv->library_view, "symbol-selected",
		G_CALLBACK (on_symbol_selected), self);
	g_signal_connect_after (priv->library_view, "symbol-deselected",
		G_CALLBACK (on_symbol_deselected), self);

	diagram_set_filename (self, NULL);

	priv->statusbar_hint_drag = gtk_statusbar_push
		(GTK_STATUSBAR (priv->statusbar), priv->statusbar_hint_context_id,
		_("Drag symbols from the library pane to add them to the diagram."));

	priv->zoom_label = gtk_label_new ("");
	gtk_label_set_single_line_mode (GTK_LABEL (priv->zoom_label), TRUE);
	gtk_box_pack_end (GTK_BOX (gtk_statusbar_get_message_area
		(GTK_STATUSBAR (priv->statusbar))), priv->zoom_label, FALSE, FALSE, 0);

	g_signal_connect (priv->view, "notify::zoom",
		G_CALLBACK (on_view_zoom_changed), self);
	g_object_notify (G_OBJECT (priv->view), "zoom");

	action_set_sensitive (self, "Undo", FALSE);
	action_set_sensitive (self, "Redo", FALSE);
	action_set_sensitive (self, "Delete", FALSE);
	action_set_sensitive (self, "NormalSize", FALSE);

	gtk_widget_grab_focus (GTK_WIDGET (priv->view));

	/* Realize the window. */
	gtk_widget_show_all (GTK_WIDGET (self));

	/* Set up GSettings. */
	priv->settings = g_settings_new ("org." PROJECT_NAME);

	g_settings_bind (priv->settings, "show-main-toolbar",
		gtk_action_group_get_action (priv->action_group,
			"MainToolbar"), "active", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (priv->settings, "show-library-pane",
		gtk_action_group_get_action (priv->action_group,
			"LibraryPane"), "active", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (priv->settings, "show-grid",
		gtk_action_group_get_action (priv->action_group,
			"ShowGrid"), "active", G_SETTINGS_BIND_DEFAULT);
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
	g_object_unref (self->priv->settings);

	if (self->priv->filename)
		g_free (self->priv->filename);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_window_main_parent_class)->finalize (gobject);
}

static void
load_library_directories (LdLibrary *library)
{
	GFile *file_program, *file_user;
	const gchar *program_dir;
	gchar *user_dir;

	program_dir = PROJECT_SHARE_DIR "library";
	user_dir = g_build_filename (g_get_user_data_dir (),
		PROJECT_NAME, "library", NULL);

	file_program = g_file_new_for_path (program_dir);
	file_user    = g_file_new_for_path (user_dir);

	ld_library_load (library, program_dir);

	/* Don't try to load the same directory twice. */
	if (!g_file_equal (file_program, file_user))
		ld_library_load (library, user_dir);

	g_object_unref (file_user);
	g_object_unref (file_program);

	g_free (user_dir);
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
on_menu_item_deselected (GtkMenuItem *item, LdWindowMain *window)
{
	gtk_statusbar_pop (GTK_STATUSBAR (window->priv->statusbar),
		window->priv->statusbar_menu_context_id);
}


/* ===== Diagram handling ================================================== */

static void
on_diagram_changed (LdDiagram *diagram, LdWindowMain *self)
{
	update_title (self);

	if (self->priv->statusbar_hint_drag)
	{
		gtk_statusbar_remove (GTK_STATUSBAR (self->priv->statusbar),
			self->priv->statusbar_hint_context_id,
			self->priv->statusbar_hint_drag);
		self->priv->statusbar_hint_drag = 0;
	}
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
		return g_filename_display_basename (self->priv->filename);
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

	if (!may_close_diagram (self, _("Save the changes to diagram \"%s\" before"
		" closing it and creating a new one?")))
		return;

	ld_diagram_clear (self->priv->diagram);
	ld_diagram_set_modified (self->priv->diagram, FALSE);

	ld_diagram_view_set_x (self->priv->view, 0);
	ld_diagram_view_set_y (self->priv->view, 0);
	ld_diagram_view_set_zoom (self->priv->view, 1);

	diagram_set_filename (self, NULL);
}

/*
 * diagram_save:
 *
 * Save the current diagram.
 */
static gboolean
diagram_save (LdWindowMain *self, GtkWindow *dialog_parent,
	const gchar *filename)
{
	GError *error;

	g_return_val_if_fail (LD_IS_WINDOW_MAIN (self), FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);

	error = NULL;
	ld_diagram_save_to_file (self->priv->diagram, filename, &error);
	if (error)
	{
		GtkWidget *message_dialog;

		g_warning ("saving failed: %s", error->message);
		g_error_free (error);

		message_dialog = gtk_message_dialog_new (dialog_parent,
			GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
			_("Failed to save the diagram"));
		gtk_message_dialog_format_secondary_text
			(GTK_MESSAGE_DIALOG (message_dialog),
			_("Try again or save it under another name."));
		gtk_dialog_run (GTK_DIALOG (message_dialog));
		gtk_widget_destroy (message_dialog);
		return FALSE;
	}
	else
	{
		ld_diagram_set_modified (self->priv->diagram, FALSE);
		update_title (self);
		return TRUE;
	}
}

/*
 * diagram_open:
 *
 * Open a diagram from a file.
 */
static gboolean
diagram_open (LdWindowMain *self, const gchar *filename)
{
	GError *error;

	error = NULL;
	ld_diagram_load_from_file (self->priv->diagram, filename, &error);
	if (error)
	{
		GtkWidget *message_dialog;

		g_warning ("loading failed: %s", error->message);

		message_dialog = gtk_message_dialog_new (GTK_WINDOW (self),
			GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
			_("Failed to open the file"));

		if (error->domain != G_FILE_ERROR)
		{
			gchar *display_filename;

			display_filename = g_filename_display_name (filename);
			gtk_message_dialog_format_secondary_text
				(GTK_MESSAGE_DIALOG (message_dialog),
				_("Failed to open file `%s': Invalid contents."), filename);
			g_free (display_filename);
		}
		else
			gtk_message_dialog_format_secondary_text
				(GTK_MESSAGE_DIALOG (message_dialog),
				"%s", error->message);

		gtk_dialog_run (GTK_DIALOG (message_dialog));
		gtk_widget_destroy (message_dialog);

		g_error_free (error);
		return FALSE;
	}

	ld_diagram_set_modified (self->priv->diagram, FALSE);
	diagram_set_filename (self, g_strdup (filename));
	return TRUE;
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
	gtk_file_filter_set_name (filter, _("Logdiag Diagrams (*.ldd)"));
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

	if (!may_close_diagram (self, _("Save the changes to diagram \"%s\" before"
		" closing it and opening another one?")))
		return;

	dialog = gtk_file_chooser_dialog_new (_("Open..."), GTK_WINDOW (self),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog),
		diagram_get_file_filter ());

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		diagram_open (self, filename);
		g_free (filename);
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

	dialog = gtk_file_chooser_dialog_new (_("Save As..."), GTK_WINDOW (self),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);
	g_object_set (dialog, "do-overwrite-confirmation", TRUE, NULL);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog),
		diagram_get_file_filter ());

	if (self->priv->filename)
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog),
			self->priv->filename);

	while (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		if (diagram_save (self, GTK_WINDOW (dialog), filename))
		{
			diagram_set_filename (self, filename);
			break;
		}
		g_free (filename);
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
		_("If you don't save, changes will be permanently lost."));
	gtk_dialog_add_buttons (GTK_DIALOG (message_dialog),
		_("Close _without Saving"), GTK_RESPONSE_NO,
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
		on_action_save (NULL, self);
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
		_("Save the changes to diagram \"%s\" before closing?"));
}


/* ===== User interface actions ============================================ */

static void
on_symbol_selected (LdCategoryView *view,
	LdSymbol *symbol, const gchar *path, LdWindowMain *self)
{
	const gchar *symbol_name;

	symbol_name = ld_symbol_get_human_name (symbol);
	gtk_statusbar_push (GTK_STATUSBAR (self->priv->statusbar),
		self->priv->statusbar_menu_context_id, symbol_name);
}

static void
on_symbol_deselected (LdCategoryView *view,
	LdSymbol *symbol, const gchar *path, LdWindowMain *self)
{
	gtk_statusbar_pop (GTK_STATUSBAR (self->priv->statusbar),
		self->priv->statusbar_menu_context_id);
}

static void
on_view_zoom_changed (LdDiagramView *view, GParamSpec *pspec,
	LdWindowMain *self)
{
	gchar *zoom;

	action_set_sensitive (self, "ZoomIn",
		ld_diagram_view_can_zoom_in (self->priv->view));
	action_set_sensitive (self, "ZoomOut",
		ld_diagram_view_can_zoom_out (self->priv->view));
	action_set_sensitive (self, "NormalSize",
		ld_diagram_view_get_zoom (self->priv->view) != 1);

	zoom = g_strdup_printf (_("%d%%"),
		(gint) (ld_diagram_view_get_zoom (self->priv->view) * 100 + 0.5));
	gtk_label_set_text (GTK_LABEL (self->priv->zoom_label), zoom);
	g_free (zoom);
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
	if (!self->priv->filename
	 || !diagram_save (self, GTK_WINDOW (self), self->priv->filename))
		diagram_show_save_as_dialog (self);
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
		"logo-icon-name", PROJECT_NAME,
		"version", PROJECT_VERSION,
		"translator-credits", _("translator-credits"),
		"copyright", "Copyright Přemysl Janouch 2010 - 2018",
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
on_action_main_toolbar (GtkToggleAction *action, LdWindowMain *self)
{
	gtk_widget_set_visible (self->priv->toolbar,
		gtk_toggle_action_get_active (action));
}

static void
on_action_library_pane (GtkToggleAction *action, LdWindowMain *self)
{
	gtk_widget_set_visible (self->priv->lv_window,
		gtk_toggle_action_get_active (action));
}

static void
on_action_grid (GtkToggleAction *action, LdWindowMain *self)
{
	ld_diagram_view_set_show_grid (self->priv->view,
		gtk_toggle_action_get_active (action));
}

static void
on_action_zoom_in (GtkAction *action, LdWindowMain *self)
{
	ld_diagram_view_zoom_in (self->priv->view);
}

static void
on_action_zoom_out (GtkAction *action, LdWindowMain *self)
{
	ld_diagram_view_zoom_out (self->priv->view);
}

static void
on_action_normal_size (GtkAction *action, LdWindowMain *self)
{
	ld_diagram_view_set_zoom (self->priv->view, 1);
}
