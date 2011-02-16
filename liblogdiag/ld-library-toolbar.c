/*
 * ld-library-toolbar.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-library-toolbar
 * @short_description: A library toolbar
 * @see_also: #LdLibrary
 *
 * #LdLibraryToolbar enables the user to choose symbols from an #LdLibrary.
 */

#define LIBRARY_TOOLBAR_ICON_WIDTH 32

/*
 * SymbolMenuItem:
 *
 * Data related to a symbol in an open symbol menu.
 */
typedef struct _SymbolMenuItem SymbolMenuItem;

struct _SymbolMenuItem
{
	LdSymbol *symbol;
	gchar *klass;

	gint width;
	gdouble dx;
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
	GtkToggleButton *active_button;

	SymbolMenuItem *items;
	gint n_items;
	gint active_item;

	gint menu_width;
	gint menu_height;
	gint menu_y;
};

enum
{
	VIEW_HANDLER_EXPOSE,
	VIEW_HANDLER_MOTION_NOTIFY,
	VIEW_HANDLER_BUTTON_PRESS,
	VIEW_HANDLER_BUTTON_RELEASE,
	VIEW_HANDLER_COUNT
};

/*
 * LdLibraryToolbarPrivate:
 * @library: a library object assigned as a model.
 * @view: a view widget for showing symbol menus.
 * @view_handlers: signal handlers that hook the view.
 * @symbol_menu: data related to menus.
 */
struct _LdLibraryToolbarPrivate
{
	LdLibrary *library;
	LdDiagramView *view;
	gulong view_handlers[VIEW_HANDLER_COUNT];
	SymbolMenuData symbol_menu;
};

enum
{
	PROP_0,
	PROP_LIBRARY,
	PROP_VIEW
};

static void ld_library_toolbar_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_library_toolbar_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);
static void ld_library_toolbar_dispose (GObject *gobject);

static void reload_library (LdLibraryToolbar *self);
static void load_category_cb (gpointer data, gpointer user_data);
static GdkPixbuf *recolor_pixbuf (GdkPixbuf *pbuf, GdkColor *color);

static void redraw_symbol_menu (LdLibraryToolbar *self);
static void emit_symbol_signal (LdLibraryToolbar *self,
	guint signal_id, gint menu_index);
static void on_category_toggle (GtkToggleButton *toggle_button,
	gpointer user_data);

static inline void block_view_handlers (LdLibraryToolbar *self);
static inline void unblock_view_handlers (LdLibraryToolbar *self);
static inline void disconnect_view_handlers (LdLibraryToolbar *self);
static gboolean on_view_exposed (GtkWidget *widget,
	GdkEventExpose *event, gpointer user_data);
static gboolean on_view_motion_notify (GtkWidget *widget,
	GdkEventMotion *event, gpointer user_data);
static gboolean on_view_button_press (GtkWidget *widget,
	GdkEventButton *event, gpointer user_data);
static gboolean on_view_button_release (GtkWidget *widget,
	GdkEventButton *event, gpointer user_data);


G_DEFINE_TYPE (LdLibraryToolbar, ld_library_toolbar, GTK_TYPE_TOOLBAR);

static void
ld_library_toolbar_class_init (LdLibraryToolbarClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GParamSpec *pspec;

	widget_class = GTK_WIDGET_CLASS (klass);

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_library_toolbar_get_property;
	object_class->set_property = ld_library_toolbar_set_property;
	object_class->dispose = ld_library_toolbar_dispose;

/**
 * LdLibraryToolbar:library:
 *
 * The #LdLibrary that this toolbar retrieves symbols from.
 */
	pspec = g_param_spec_object ("library", "Library",
		"The library that this toolbar retrieves symbols from.",
		LD_TYPE_LIBRARY, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_LIBRARY, pspec);

/**
 * LdLibraryToolbar:view:
 *
 * The #LdDiagramView widget misused for showing symbol menus.
 */
	pspec = g_param_spec_object ("view", "View",
		"The view widget misused for showing symbol menus.",
		LD_TYPE_DIAGRAM_VIEW, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_VIEW, pspec);

/**
 * LdLibraryToolbar::symbol-chosen:
 * @self: an #LdLibraryToolbar object.
 * @symbol: the chosen #LdSymbol object.
 * @klass: location of the symbol within the library.
 *
 * A symbol has been chosen.
 */
	klass->symbol_chosen_signal = g_signal_new
		("symbol-chosen", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST, 0, NULL, NULL,
		ld_marshal_VOID__OBJECT_STRING,
		G_TYPE_NONE, 2, LD_TYPE_SYMBOL,
		G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

/**
 * LdLibraryToolbar::symbol-selected:
 * @self: an #LdLibraryToolbar object.
 * @symbol: the selected #LdSymbol object.
 * @klass: location of the symbol within the library.
 *
 * A symbol has been selected.
 */
	klass->symbol_selected_signal = g_signal_new
		("symbol-selected", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST, 0, NULL, NULL,
		ld_marshal_VOID__OBJECT_STRING,
		G_TYPE_NONE, 2, LD_TYPE_SYMBOL,
		G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

/**
 * LdLibraryToolbar::symbol-deselected:
 * @self: an #LdLibraryToolbar object.
 * @symbol: the deselected #LdSymbol object.
 * @klass: location of the symbol within the library.
 *
 * A symbol has been deselected.
 */
	klass->symbol_deselected_signal = g_signal_new
		("symbol-deselected", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST, 0, NULL, NULL,
		ld_marshal_VOID__OBJECT_STRING,
		G_TYPE_NONE, 2, LD_TYPE_SYMBOL,
		G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

	g_type_class_add_private (klass, sizeof (LdLibraryToolbarPrivate));
}

static void
ld_library_toolbar_init (LdLibraryToolbar *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_LIBRARY_TOOLBAR, LdLibraryToolbarPrivate);
}

static void
ld_library_toolbar_dispose (GObject *gobject)
{
	LdLibraryToolbar *self;

	self = LD_LIBRARY_TOOLBAR (gobject);

	ld_library_toolbar_set_library (self, NULL);
	ld_library_toolbar_set_view (self, NULL);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_library_toolbar_parent_class)->dispose (gobject);
}

static void
ld_library_toolbar_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdLibraryToolbar *self;

	self = LD_LIBRARY_TOOLBAR (object);
	switch (property_id)
	{
	case PROP_LIBRARY:
		g_value_set_object (value, ld_library_toolbar_get_library (self));
		break;
	case PROP_VIEW:
		g_value_set_object (value, ld_library_toolbar_get_view (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_library_toolbar_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdLibraryToolbar *self;

	self = LD_LIBRARY_TOOLBAR (object);
	switch (property_id)
	{
	case PROP_LIBRARY:
		ld_library_toolbar_set_library (self,
			LD_LIBRARY (g_value_get_object (value)));
		break;
	case PROP_VIEW:
		ld_library_toolbar_set_view (self,
			LD_DIAGRAM_VIEW (g_value_get_object (value)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}


/**
 * ld_library_toolbar_new:
 *
 * Create an instance.
 */
GtkWidget *
ld_library_toolbar_new (void)
{
	return g_object_new (LD_TYPE_LIBRARY_TOOLBAR, NULL);
}

/**
 * ld_library_toolbar_set_library:
 * @self: an #LdLibraryToolbar object.
 * @library: (allow-none): the library to be assigned to the toolbar.
 *
 * Assign an #LdLibrary object to the toolbar.
 */
void
ld_library_toolbar_set_library (LdLibraryToolbar *self, LdLibrary *library)
{
	g_return_if_fail (LD_IS_LIBRARY_TOOLBAR (self));
	g_return_if_fail (LD_IS_LIBRARY (library) || library == NULL);

	if (self->priv->library)
	{
		g_signal_handlers_disconnect_by_func (self->priv->library,
			reload_library, self);
		g_object_unref (self->priv->library);
	}

	self->priv->library = library;

	if (library)
	{
		g_signal_connect_data (library, "changed",
			G_CALLBACK (reload_library), self,
			NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);
		g_object_ref (library);
	}
	reload_library (self);
	g_object_notify (G_OBJECT (self), "library");
}

/**
 * ld_library_toolbar_get_library:
 * @self: an #LdLibraryToolbar object.
 *
 * Return value: (transfer: none): the #LdLibrary object
 *               assigned to the toolbar.
 */
LdLibrary *
ld_library_toolbar_get_library (LdLibraryToolbar *self)
{
	g_return_val_if_fail (LD_IS_LIBRARY_TOOLBAR (self), NULL);
	return self->priv->library;
}

/**
 * ld_library_toolbar_set_view:
 * @self: an #LdLibraryToolbar object.
 * @view: (allow-none): the widget to be assigned to the toolbar.
 *
 * Assign an #LdDiagramView widget to the toolbar.
 */
void
ld_library_toolbar_set_view (LdLibraryToolbar *self, LdDiagramView *view)
{
	g_return_if_fail (LD_IS_LIBRARY_TOOLBAR (self));
	g_return_if_fail (LD_IS_DIAGRAM_VIEW (view) || view == NULL);

	if (self->priv->view)
	{
		disconnect_view_handlers (self);
		g_object_unref (self->priv->view);
	}

	self->priv->view = view;

	if (view)
	{
		self->priv->view_handlers[VIEW_HANDLER_EXPOSE]
			= g_signal_connect (view, "expose-event",
			G_CALLBACK (on_view_exposed), self);
		self->priv->view_handlers[VIEW_HANDLER_MOTION_NOTIFY]
			= g_signal_connect (view, "motion-notify-event",
			G_CALLBACK (on_view_motion_notify), self);
		self->priv->view_handlers[VIEW_HANDLER_BUTTON_PRESS]
			= g_signal_connect (view, "button-press-event",
			G_CALLBACK (on_view_button_press), self);
		self->priv->view_handlers[VIEW_HANDLER_BUTTON_RELEASE]
			= g_signal_connect (view, "button-release-event",
			G_CALLBACK (on_view_button_release), self);

		block_view_handlers (self);
		g_object_ref (view);
	}
	g_object_notify (G_OBJECT (self), "view");
}

/**
 * ld_library_toolbar_get_view:
 * @self: an #LdLibraryToolbar object.
 *
 * Return value: (transfer: none): the #LdDiagramView widget
 *               assigned to the toolbar.
 */
LdDiagramView *
ld_library_toolbar_get_view (LdLibraryToolbar *self)
{
	g_return_val_if_fail (LD_IS_LIBRARY_TOOLBAR (self), NULL);
	return self->priv->view;
}

static void
reload_library (LdLibraryToolbar *self)
{
	g_return_if_fail (LD_IS_LIBRARY_TOOLBAR (self));

	/* Clear the toolbar first, if there was already something in it. */
	gtk_container_foreach (GTK_CONTAINER (self),
		(GtkCallback) gtk_widget_destroy, NULL);

	if (self->priv->library)
	{
		GSList *categories;

		categories = (GSList *) ld_library_get_children (self->priv->library);
		g_slist_foreach (categories, load_category_cb, self);
	}
}

static void
load_category_cb (gpointer data, gpointer user_data)
{
	LdLibraryToolbar *self;
	LdSymbolCategory *cat;
	const gchar *human_name;
	GdkPixbuf *pbuf, *new_pbuf;
	GtkWidget *img;
	GtkToolItem *item;
	GtkWidget *button;
	GtkStyle *style;

	g_return_if_fail (LD_IS_LIBRARY_TOOLBAR (user_data));
	g_return_if_fail (LD_IS_SYMBOL_CATEGORY (data));

	self = user_data;
	cat = data;

	pbuf = gdk_pixbuf_new_from_file_at_size	(ld_symbol_category_get_image_path
		(cat), LIBRARY_TOOLBAR_ICON_WIDTH, -1, NULL);
	g_return_if_fail (pbuf != NULL);

	button = gtk_toggle_button_new ();
	style = gtk_rc_get_style (button);

	/* TODO: Handle all states. */
	new_pbuf = recolor_pixbuf (pbuf, &style->fg[GTK_STATE_NORMAL]);
	if (new_pbuf)
	{
		g_object_unref (pbuf);
		pbuf = new_pbuf;
	}

	img = gtk_image_new_from_pixbuf (pbuf);
	g_object_unref (pbuf);

	item = gtk_tool_item_new ();
	gtk_container_add (GTK_CONTAINER (button), img);
	gtk_container_add (GTK_CONTAINER (item), button);

	/* Don't steal focus from the view. */
	g_object_set (button, "can-focus", FALSE, NULL);

	/* Assign the category to the toggle button. */
	/* TODO: Move this to the data parameter for the signal handler.
	 *       Use g_signal_connect_data() to set up destroy notification.
	 */
	g_object_ref (cat);
	g_object_set_data_full (G_OBJECT (button),
		"category", cat, (GDestroyNotify) g_object_unref);

	/* Hook toggling of the button. */
	g_signal_connect (button, "toggled", G_CALLBACK (on_category_toggle), self);

	human_name = ld_symbol_category_get_human_name (cat);
	gtk_tool_item_set_tooltip_text (item, human_name);
	gtk_toolbar_insert (GTK_TOOLBAR (self), item, 0);
}

static GdkPixbuf *
recolor_pixbuf (GdkPixbuf *pbuf, GdkColor *color)
{
	gint width, height;
	GdkPixbuf *new_pbuf;
	cairo_surface_t *cr_surface;
	cairo_t *cr;

	width  = gdk_pixbuf_get_width  (pbuf);
	height = gdk_pixbuf_get_height (pbuf);

	new_pbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, width, height);
	g_return_val_if_fail (new_pbuf != NULL, NULL);

	cr_surface = cairo_image_surface_create_for_data
		(gdk_pixbuf_get_pixels (new_pbuf), CAIRO_FORMAT_ARGB32,
		width, height, gdk_pixbuf_get_rowstride (new_pbuf));
	cr = cairo_create (cr_surface);

	/* Change the color of all pixels but leave the alpha channel intact. */
	gdk_cairo_set_source_color (cr, color);
	cairo_paint (cr);

	cairo_set_operator (cr, CAIRO_OPERATOR_DEST_IN);
	gdk_cairo_set_source_pixbuf (cr, pbuf, 0, 0);
	cairo_paint (cr);

	cairo_destroy (cr);
	cairo_surface_destroy (cr_surface);
	return new_pbuf;
}

static void
redraw_symbol_menu (LdLibraryToolbar *self)
{
	SymbolMenuData *data;

	g_return_if_fail (LD_IS_LIBRARY_TOOLBAR (self));
	data = &self->priv->symbol_menu;

	gtk_widget_queue_draw_area (GTK_WIDGET (self->priv->view),
		0, data->menu_y - 1, data->menu_width + 2, data->menu_height + 2);
}

static void
emit_symbol_signal (LdLibraryToolbar *self, guint signal_id, gint menu_index)
{
	SymbolMenuData *data;
	SymbolMenuItem *item;

	data = &self->priv->symbol_menu;
	if (menu_index == -1)
		menu_index = data->active_item;
	if (menu_index != -1)
	{
		item = &data->items[menu_index];
		g_signal_emit (self, signal_id, 0, item->symbol, item->klass);
	}
}

static void
on_category_toggle (GtkToggleButton *toggle_button, gpointer user_data)
{
	LdLibraryToolbar *self;
	LdLibraryToolbarPrivate *priv;
	LdSymbolCategory *cat;
	SymbolMenuData *data;
	const gchar *category_name, *symbol_name;

	cat = g_object_get_data (G_OBJECT (toggle_button), "category");
	self = LD_LIBRARY_TOOLBAR (user_data);
	priv = self->priv;
	data = &priv->symbol_menu;

	/* First untoggle any active button. */
	if (data->active_button)
		gtk_toggle_button_set_active (data->active_button, FALSE);

	/* And toggle signal handlers that enable the user to add a symbol. */
	if (data->active_button == toggle_button)
	{
		gint i;

		block_view_handlers (self);

		g_object_unref (data->active_button);
		data->active_button = NULL;

		emit_symbol_signal (self, LD_LIBRARY_TOOLBAR_GET_CLASS (self)
			->symbol_deselected_signal, -1);

		/* Ashes to ashes, NULL to NULL. */
		for (i = 0; i < data->n_items; i++)
		{
			g_object_unref (data->items[i].symbol);
			g_free (data->items[i].klass);
		}

		g_free (data->items);
		data->items = NULL;

		gtk_grab_remove (GTK_WIDGET (self->priv->view));
	}
	else
	{
		const GSList *children, *symbol_iter;
		SymbolMenuItem *item;
		gint x, y, menu_width;

		g_return_if_fail (gtk_widget_translate_coordinates (GTK_WIDGET
			(toggle_button), GTK_WIDGET (priv->view), 0, 0, &x, &y));

		data->menu_y = y;
		data->menu_height = GTK_WIDGET (toggle_button)->allocation.height;

		unblock_view_handlers (self);

		data->active_button = toggle_button;
		g_object_ref (data->active_button);

		category_name = ld_symbol_category_get_name (cat);
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

			symbol_name = ld_symbol_get_name (item->symbol);
			item->klass = g_build_path (LD_LIBRARY_IDENTIFIER_SEPARATOR,
				category_name, symbol_name, NULL);

			ld_symbol_get_area (item->symbol, &area);

			/* This is the height when the center of the symbol is
			 * in the center of it's symbol menu item.
			 */
			item->scale = data->menu_height * 0.5
				/ MAX (ABS (area.y), ABS (area.y + area.height)) * 0.5;
			if (item->scale * area.width > 1.5 * data->menu_height)
				item->scale = 1.5 * data->menu_height / area.width;
			item->width = data->menu_height * 0.5 + item->scale * area.width;
			item->dx = item->width * 0.5 + item->scale
				* (area.width * 0.5 - ABS (area.x + area.width));

			menu_width += item++->width;
		}
		data->menu_width = menu_width;

		gtk_grab_add (GTK_WIDGET (self->priv->view));
	}
	redraw_symbol_menu (self);
}

#define DEFINE_VIEW_HANDLER_FUNC(name) \
static inline void \
name ## _view_handlers (LdLibraryToolbar *self) \
{ \
	gint i; \
	g_return_if_fail (LD_IS_DIAGRAM_VIEW (self->priv->view)); \
	for (i = 0; i < VIEW_HANDLER_COUNT; i++) \
		g_signal_handler_ ## name (self->priv->view, \
			self->priv->view_handlers[i]); \
}

DEFINE_VIEW_HANDLER_FUNC (block)
DEFINE_VIEW_HANDLER_FUNC (unblock)
DEFINE_VIEW_HANDLER_FUNC (disconnect)

static gboolean
on_view_exposed (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	cairo_t *cr;
	LdLibraryToolbar *self;
	SymbolMenuData *data;
	gint i, x;

	cr = gdk_cairo_create (widget->window);
	self = LD_LIBRARY_TOOLBAR (user_data);
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

		if (i)
		{
			cairo_move_to (cr, x - 0.5, data->menu_y + 1);
			cairo_line_to (cr, x - 0.5, data->menu_y + data->menu_height);
			cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
			cairo_stroke (cr);
		}

		item = data->items + i;
		cairo_save (cr);

		cairo_rectangle (cr, x, data->menu_y, item->width, data->menu_height);
		cairo_clip (cr);

		if (i == data->active_item)
		{
			cairo_set_source_rgb (cr, 0.9, 0.9, 0.9);
			cairo_paint (cr);
		}

		cairo_translate (cr, x + item->dx,
			data->menu_y + data->menu_height * 0.5);
		cairo_scale (cr, item->scale, item->scale);

		cairo_set_source_rgb (cr, 0, 0, 0);
		cairo_set_line_width (cr, 1 / item->scale);
		ld_symbol_draw (item->symbol, cr);

		cairo_restore (cr);
		x += item->width;
	}

	cairo_rectangle (cr, 0.5, data->menu_y + 0.5,
		data->menu_width, data->menu_height);
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_stroke (cr);

	cairo_destroy (cr);
	return FALSE;
}

static gboolean
on_view_motion_notify (GtkWidget *widget, GdkEventMotion *event,
	gpointer user_data)
{
	LdLibraryToolbar *self;
	SymbolMenuData *data;
	gint i, x, at_cursor = -1;

	self = LD_LIBRARY_TOOLBAR (user_data);
	data = &self->priv->symbol_menu;

	if (widget->window != event->window
		|| event->x < 0 || event->y < data->menu_y
		|| event->y >= data->menu_y + data->menu_height)
		goto on_view_motion_notify_end;

	for (x = i = 0; i < data->n_items; i++)
	{
		x += data->items[i].width;
		if (event->x < x)
		{
			at_cursor = i;
			break;
		}
	}

on_view_motion_notify_end:
	if (data->active_item != at_cursor)
	{
		emit_symbol_signal (self, LD_LIBRARY_TOOLBAR_GET_CLASS (self)
			->symbol_deselected_signal, -1);

		if (at_cursor != -1)
			emit_symbol_signal (self, LD_LIBRARY_TOOLBAR_GET_CLASS (self)
				->symbol_selected_signal, at_cursor);
	}
	data->active_item = at_cursor;
	redraw_symbol_menu (self);
	return FALSE;
}

static gboolean
on_view_button_press (GtkWidget *widget, GdkEventButton *event,
	gpointer user_data)
{
	LdLibraryToolbar *self;
	SymbolMenuData *data;

	self = LD_LIBRARY_TOOLBAR (user_data);
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
on_view_button_release (GtkWidget *widget, GdkEventButton *event,
	gpointer user_data)
{
	LdLibraryToolbar *self;
	SymbolMenuData *data;

	self = LD_LIBRARY_TOOLBAR (user_data);
	data = &self->priv->symbol_menu;

	if (event->button != 1)
		return FALSE;

	emit_symbol_signal (self, LD_LIBRARY_TOOLBAR_GET_CLASS (self)
		->symbol_chosen_signal, -1);

	/* We've either chosen a symbol or canceled the menu, so hide it. */
	if (data->active_button)
		gtk_toggle_button_set_active (data->active_button, FALSE);

	return FALSE;
}
