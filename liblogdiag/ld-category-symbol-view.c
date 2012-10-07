/*
 * ld-category-symbol-view.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-category-symbol-view
 * @short_description: A widget that displays symbols in a category
 * @see_also: #LdCategory, #LdDiagramView
 *
 * #LdCategorySymbolView allows the user to drag symbols from an #LdCategory
 * onto #LdDiagramView.
 */

/* Milimetres per inch. */
#define MM_PER_INCH 25.4
/* The default screen resolution in DPI units. */
#define DEFAULT_SCREEN_RESOLUTION 96

#define SYMBOL_WIDTH    50   /* Width  of a symbol. */
#define SYMBOL_HEIGHT   40   /* Height of a symbol. */
#define SYMBOL_SPACING  10   /* Spacing between symbols, and also borders. */

typedef struct
{
	LdSymbol *symbol;        /* The associated symbol, ref'ed. */
	gchar *path;             /* Path to the symbol. */

	GdkRectangle rect;       /* Clipping rectangle. */
	gdouble scale;           /* Scale to draw the symbol in. */
	gdouble dx, dy;          /* Delta into .rect. */
}
SymbolData;

/*
 * LdCategorySymbolViewPrivate:
 * @category: a category object assigned as a model.
 * @path: path to the category within the library.
 * @layout: (element-type SymbolData *): current layout of symbols.
 * @preselected: currently preselected symbol.
 * @height_negotiation: whether we are negotiating height right now.
 */
struct _LdCategorySymbolViewPrivate
{
	LdCategory *category;
	gchar *path;
	GSList *layout;
	SymbolData *preselected;
	guint height_negotiation : 1;
};

enum
{
	PROP_0,
	PROP_CATEGORY
};

static void ld_category_symbol_view_get_property (GObject *object,
	guint property_id, GValue *value, GParamSpec *pspec);
static void ld_category_symbol_view_set_property (GObject *object,
	guint property_id, const GValue *value, GParamSpec *pspec);
static void ld_category_symbol_view_finalize (GObject *gobject);

static void on_size_request (GtkWidget *widget, GtkRequisition *requisition,
	gpointer user_data);
static void on_size_allocate (GtkWidget *widget, GdkRectangle *allocation,
	gpointer user_data);
static gboolean on_expose_event (GtkWidget *widget, GdkEventExpose *event,
	gpointer user_data);


G_DEFINE_TYPE (LdCategorySymbolView,
	ld_category_symbol_view, GTK_TYPE_DRAWING_AREA);

static void
ld_category_symbol_view_class_init (LdCategorySymbolViewClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_category_symbol_view_get_property;
	object_class->set_property = ld_category_symbol_view_set_property;
	object_class->finalize = ld_category_symbol_view_finalize;

/**
 * LdCategorySymbolView:category:
 *
 * The underlying #LdCategory object of this view.
 */
	pspec = g_param_spec_object ("category", "Category",
		"The underlying category object of this view.",
		LD_TYPE_CATEGORY, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_CATEGORY, pspec);

	g_type_class_add_private (klass, sizeof (LdCategorySymbolViewPrivate));
}

static void
symbol_redraw (LdCategorySymbolView *self, SymbolData *symbol)
{
	gtk_widget_queue_draw_area (GTK_WIDGET (self),
		symbol->rect.x,
		symbol->rect.y,
		symbol->rect.width,
		symbol->rect.height);
}

static void
symbol_deselect (LdCategorySymbolView *self)
{
	if (!self->priv->preselected)
		return;

	symbol_redraw (self, self->priv->preselected);
	self->priv->preselected = NULL;
	gtk_drag_source_unset (GTK_WIDGET (self));
}

static gboolean
on_leave_notify (GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
	symbol_deselect (LD_CATEGORY_SYMBOL_VIEW (widget));
	return FALSE;
}

static gboolean
on_motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	LdCategorySymbolView *self;
	GSList *iter;

	if (event->state & GDK_BUTTON1_MASK)
		return FALSE;

	self = LD_CATEGORY_SYMBOL_VIEW (widget);
	for (iter = self->priv->layout; iter; iter = iter->next)
	{
		SymbolData *data;

		data = iter->data;
		if (event->x <  data->rect.x
		 || event->y <  data->rect.y
		 || event->x >= data->rect.x + data->rect.width
		 || event->y >= data->rect.y + data->rect.height)
			continue;

		if (data != self->priv->preselected)
		{
			GtkTargetEntry target = {"ld-symbol", GTK_TARGET_SAME_APP, 0};

			symbol_deselect (self);
			self->priv->preselected = data;
			symbol_redraw (self, data);

			gtk_drag_source_set (widget,
				GDK_BUTTON1_MASK, &target, 1, GDK_ACTION_COPY);
		}
		return FALSE;
	}

	symbol_deselect (self);
	return FALSE;
}

static void
on_drag_data_get
(GtkWidget *widget, GdkDragContext *ctx, GtkSelectionData *selection_data,
	guint target_type, guint time, gpointer user_data)
{
	LdCategorySymbolView *self;

	self = LD_CATEGORY_SYMBOL_VIEW (widget);
	g_return_if_fail (self->priv->preselected != NULL);

	gtk_selection_data_set (selection_data,
		gtk_selection_data_get_target (selection_data),
		8, (guchar *) self->priv->preselected->path,
		strlen (self->priv->preselected->path));
}

static void
on_drag_begin (GtkWidget *widget, GdkDragContext *ctx, gpointer user_data)
{
	LdCategorySymbolView *self;
	GdkPixbuf *pbuf;

	self = LD_CATEGORY_SYMBOL_VIEW (widget);
	g_return_if_fail (self->priv->preselected != NULL);

	/* Some of the larger previews didn't work, and we have to get rid of
	 * the icon later when we're hovering above LdDiagramView anyway. */
	pbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, 1, 1);
	gdk_pixbuf_fill (pbuf, 0x00000000);
	gtk_drag_set_icon_pixbuf (ctx, pbuf, 0, 0);
	g_object_unref (pbuf);
}

static void
on_drag_end (GtkWidget *widget, GdkDragContext *ctx, gpointer user_data)
{
	symbol_deselect (LD_CATEGORY_SYMBOL_VIEW (widget));
}

static void
ld_category_symbol_view_init (LdCategorySymbolView *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_CATEGORY_SYMBOL_VIEW, LdCategorySymbolViewPrivate);

	g_signal_connect (self, "size-allocate",
		G_CALLBACK (on_size_allocate), NULL);
	g_signal_connect (self, "size-request",
		G_CALLBACK (on_size_request), NULL);
	g_signal_connect (self, "expose-event",
		G_CALLBACK (on_expose_event), NULL);

	g_signal_connect (self, "motion-notify-event",
		G_CALLBACK (on_motion_notify), NULL);
	g_signal_connect (self, "leave-notify-event",
		G_CALLBACK (on_leave_notify), NULL);

	g_signal_connect (self, "drag-begin",
		G_CALLBACK (on_drag_begin), NULL);
	g_signal_connect (self, "drag-data-get",
		G_CALLBACK (on_drag_data_get), NULL);
	g_signal_connect (self, "drag-end",
		G_CALLBACK (on_drag_end), NULL);

	gtk_widget_add_events (GTK_WIDGET (self),
		GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK
		| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
		| GDK_LEAVE_NOTIFY_MASK);
}

static void
symbol_data_free (SymbolData *self)
{
	g_object_unref (self->symbol);
	g_free (self->path);
	g_slice_free (SymbolData, self);
}

static void
layout_destroy (LdCategorySymbolView *self)
{
	g_slist_foreach (self->priv->layout, (GFunc) symbol_data_free, NULL);
	g_slist_free (self->priv->layout);
	self->priv->layout = NULL;
	self->priv->preselected = NULL;
}

static void
ld_category_symbol_view_finalize (GObject *gobject)
{
	LdCategorySymbolView *self;

	self = LD_CATEGORY_SYMBOL_VIEW (gobject);

	layout_destroy (self);
	if (self->priv->category)
		g_object_unref (self->priv->category);
	g_free (self->priv->path);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_category_symbol_view_parent_class)->finalize (gobject);
}

static void
ld_category_symbol_view_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdCategorySymbolView *self;

	self = LD_CATEGORY_SYMBOL_VIEW (object);
	switch (property_id)
	{
	case PROP_CATEGORY:
		g_value_set_object (value, ld_category_symbol_view_get_category (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_category_symbol_view_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdCategorySymbolView *self;

	self = LD_CATEGORY_SYMBOL_VIEW (object);
	switch (property_id)
	{
	case PROP_CATEGORY:
		ld_category_symbol_view_set_category (self,
			LD_CATEGORY (g_value_get_object (value)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}


typedef struct
{
	guint total_height;      /* Total height required to show the symbols. */
	guint max_width;         /* Width available to the widget. */

	GSList *cur_row;         /* Current row of symbols. */
	guint cur_width;         /* Current width of the row. */
	guint cur_height_up;     /* Current max. upper height of symbols. */
	guint cur_height_down;   /* Current max. lower height of symbols. */
}
LayoutContext;

static GSList *
layout_finish_row (LayoutContext *ctx)
{
	GSList *item, *result;
	gint row_height, h_delta;

	row_height = SYMBOL_SPACING + ctx->cur_height_up + ctx->cur_height_down;
	h_delta = (ctx->max_width - ctx->cur_width) / 2;

	for (item = ctx->cur_row; item; item = item->next)
	{
		SymbolData *data;

		data = item->data;
		data->rect.x += h_delta;
		data->rect.height = row_height;
		data->dy = SYMBOL_SPACING * 0.5 + ctx->cur_height_up;
	}

	result = g_slist_reverse (ctx->cur_row);

	ctx->cur_row = NULL;
	ctx->total_height += row_height;

	ctx->cur_width = SYMBOL_SPACING;
	ctx->cur_height_up = 0;
	ctx->cur_height_down = 0;

	return result;
}

static gint
layout_for_width (LdCategorySymbolView *self, gint width)
{
	GSList *symbols, *iter;
	LayoutContext ctx = {SYMBOL_SPACING, 0, NULL, SYMBOL_SPACING, 0, 0};

	layout_destroy (self);
	ctx.max_width = width;

	symbols = (GSList *) ld_category_get_symbols (self->priv->category);
	for (iter = symbols; iter; iter = iter->next)
	{
		SymbolData *data;
		LdRectangle area;
		LdSymbol *symbol;
		gint real_width, height_up, height_down;

		symbol = LD_SYMBOL (iter->data);
		ld_symbol_get_area (symbol, &area);

		data = g_slice_new (SymbolData);
		data->symbol = g_object_ref (symbol);
		data->path = g_build_path (LD_LIBRARY_IDENTIFIER_SEPARATOR,
			self->priv->path, ld_symbol_get_name (symbol), NULL);

		/* Compute the scale to fit the symbol to an area of
		 * SYMBOL_WIDTH * SYMBOL_HEIGHT, vertically centred. */
		data->scale = SYMBOL_HEIGHT * 0.5
			/ MAX (ABS (area.y), ABS (area.y + area.height)) * 0.5;
		if (data->scale * area.width > SYMBOL_WIDTH)
			data->scale = SYMBOL_WIDTH / area.width;

		real_width = data->scale * area.width + 0.5;
		data->rect.width = real_width + SYMBOL_SPACING;
		/* Now I have no idea what this does but it worked before.
		 * When I do, I have to write it in here. */
		data->dx = data->rect.width * 0.5 + data->scale
			* (area.width * 0.5 - ABS (area.x + area.width));

		if (ctx.cur_width + real_width + SYMBOL_SPACING > ctx.max_width
			&& ctx.cur_row != NULL)
		{
			self->priv->layout = g_slist_concat (self->priv->layout,
				layout_finish_row (&ctx));
		}

		/* Half of the spacing is included on each side of the rect. */
		data->rect.x = ctx.cur_width    - SYMBOL_SPACING / 2;
		data->rect.y = ctx.total_height - SYMBOL_SPACING / 2;

		height_up   = data->scale * ABS (area.y);
		height_down = data->scale * ABS (area.y + area.height);

		if (height_up   > ctx.cur_height_up)
			ctx.cur_height_up   = height_up;
		if (height_down > ctx.cur_height_down)
			ctx.cur_height_down = height_down;

		ctx.cur_row = g_slist_prepend (ctx.cur_row, data);
		ctx.cur_width += real_width + SYMBOL_SPACING;
	}

	if (ctx.cur_row != NULL)
		self->priv->layout = g_slist_concat (self->priv->layout,
			layout_finish_row (&ctx));

	return ctx.total_height;
}


static gboolean
on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
	LdCategorySymbolView *self;
	cairo_t *cr;
	GSList *iter;

	self = LD_CATEGORY_SYMBOL_VIEW (widget);
	cr = gdk_cairo_create (gtk_widget_get_window (widget));
	gdk_cairo_rectangle (cr, &event->area);
	cairo_clip (cr);

	gdk_cairo_set_source_color (cr,
		&gtk_widget_get_style (widget)->base[GTK_STATE_NORMAL]);
	cairo_paint (cr);

	for (iter = self->priv->layout; iter; iter = iter->next)
	{
		SymbolData *data;

		data = iter->data;
		if (!gdk_rectangle_intersect (&data->rect, &event->area, NULL))
			continue;

		cairo_save (cr);
		gdk_cairo_rectangle (cr, &data->rect);
		cairo_clip (cr);

		gdk_cairo_set_source_color (cr,
			&gtk_widget_get_style (widget)->text[GTK_STATE_NORMAL]);

		if (data == self->priv->preselected)
			cairo_paint_with_alpha (cr, 0.1);

		cairo_translate (cr, data->rect.x + data->dx, data->rect.y + data->dy);
		cairo_scale (cr, data->scale, data->scale);

		cairo_set_line_width (cr, 1 / data->scale);
		ld_symbol_draw (data->symbol, cr);

		cairo_restore (cr);
	}

	cairo_destroy (cr);
	return FALSE;
}

static void
on_size_request (GtkWidget *widget, GtkRequisition *requisition,
	gpointer user_data)
{
	LdCategorySymbolView *self;

	self = LD_CATEGORY_SYMBOL_VIEW (widget);

	if (!self->priv->category
	 || !ld_category_get_symbols (self->priv->category))
	{
		requisition->width  = 0;
		requisition->height = 0;
		return;
	}

	requisition->width = SYMBOL_WIDTH + 2 * SYMBOL_SPACING;

	if (self->priv->height_negotiation)
	{
		GtkAllocation alloc;

		gtk_widget_get_allocation (widget, &alloc);
		requisition->height = layout_for_width (self, alloc.width);
	}
	else
		requisition->height = SYMBOL_HEIGHT + 2 * SYMBOL_SPACING;
}

static void
on_size_allocate (GtkWidget *widget, GdkRectangle *allocation,
	gpointer user_data)
{
	LdCategorySymbolView *self;

	self = LD_CATEGORY_SYMBOL_VIEW (widget);

	if (self->priv->height_negotiation)
		self->priv->height_negotiation = FALSE;
	else
	{
		self->priv->height_negotiation = TRUE;
		gtk_widget_queue_resize (widget);
	}
}

/* ===== Generic interface etc. ============================================ */

/**
 * ld_category_symbol_view_new:
 * @category: (allow-none): a category to be assigned to the widget.
 *
 * Create an instance.
 */
GtkWidget *
ld_category_symbol_view_new (LdCategory *category)
{
	LdCategorySymbolView *self;

	self = g_object_new (LD_TYPE_CATEGORY_SYMBOL_VIEW, NULL);
	ld_category_symbol_view_set_category (self, category);
	return GTK_WIDGET (self);
}

/**
 * ld_category_symbol_view_set_category:
 * @self: an #LdCategorySymbolView object.
 * @category: the #LdCategory to be assigned to the view.
 *
 * Assign an #LdCategory object to the view.
 */
void
ld_category_symbol_view_set_category (LdCategorySymbolView *self,
	LdCategory *category)
{
	g_return_if_fail (LD_IS_CATEGORY_SYMBOL_VIEW (self));
	g_return_if_fail (LD_IS_CATEGORY (category));

	if (self->priv->category)
	{
		g_object_unref (self->priv->category);

		g_free (self->priv->path);
		self->priv->path = NULL;
	}

	/* XXX: We should rebuild the path if the name changes but it shouldn't
	 *      happen and we would have to track the parents, too. */
	self->priv->path = ld_category_get_path (category);

	self->priv->category = category;
	g_object_ref (category);

	g_object_notify (G_OBJECT (self), "category");
	gtk_widget_queue_resize (GTK_WIDGET (self));
}

/**
 * ld_category_symbol_view_get_category:
 * @self: an #LdCategorySymbolView object.
 *
 * Get the #LdCategory object assigned to this view.
 * The reference count on the category is not incremented.
 */
LdCategory *
ld_category_symbol_view_get_category (LdCategorySymbolView *self)
{
	g_return_val_if_fail (LD_IS_CATEGORY_SYMBOL_VIEW (self), NULL);
	return self->priv->category;
}
