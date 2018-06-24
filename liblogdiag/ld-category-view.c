/*
 * ld-category-view.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2012
 *
 * See the file LICENSE for licensing information.
 *
 */

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-category-view
 * @short_description: Interface for objects displaying categories
 * @see_also: #LdCategory
 *
 * #LdCategoryView defines objects displaying contents of #LdCategory
 * hierarchies.
 */

G_DEFINE_INTERFACE (LdCategoryView, ld_category_view, 0);

static void
ld_category_view_default_init (LdCategoryViewInterface *iface)
{
	GParamSpec *pspec;

/**
 * LdCategoryView::symbol-selected:
 * @self: an #LdCategoryView object.
 * @symbol: the selected #LdSymbol object.
 * @path: location of the symbol within the library.
 *
 * A symbol has been selected.
 */
	iface->symbol_selected_signal = g_signal_new
		("symbol-selected", G_TYPE_FROM_INTERFACE (iface),
		G_SIGNAL_RUN_LAST, 0, NULL, NULL,
		ld_marshal_VOID__OBJECT_STRING,
		G_TYPE_NONE, 2, LD_TYPE_SYMBOL,
		G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

/**
 * LdCategoryView::symbol-deselected:
 * @self: an #LdCategoryView object.
 * @symbol: the deselected #LdSymbol object.
 * @path: location of the symbol within the library.
 *
 * A symbol has been deselected.
 */
	iface->symbol_deselected_signal = g_signal_new
		("symbol-deselected", G_TYPE_FROM_INTERFACE (iface),
		G_SIGNAL_RUN_LAST, 0, NULL, NULL,
		ld_marshal_VOID__OBJECT_STRING,
		G_TYPE_NONE, 2, LD_TYPE_SYMBOL,
		G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

/**
 * LdCategoryView:category:
 *
 * The #LdCategory this object retrieves content from.
 */
	pspec = g_param_spec_object ("category", "Category",
		"The symbol category that is shown by this object.",
		LD_TYPE_CATEGORY, G_PARAM_READWRITE);
	g_object_interface_install_property (iface, pspec);
}

/**
 * ld_category_view_set_category:
 * @self: an #LdCategoryView object.
 * @category: the #LdCategory to be assigned to the view.
 *
 * Assign an #LdCategory object to the view.
 */
void
ld_category_view_set_category (LdCategoryView *self,
	LdCategory *category)
{
	g_return_if_fail (LD_IS_CATEGORY_VIEW (self));
	LD_CATEGORY_VIEW_GET_INTERFACE (self)->set_category (self, category);
}

/**
 * ld_category_view_get_category:
 * @self: an #LdCategoryView object.
 *
 * Get the #LdCategory object assigned to this view.
 * The reference count on the category is not incremented.
 */
LdCategory *
ld_category_view_get_category (LdCategoryView *self)
{
	g_return_val_if_fail (LD_IS_CATEGORY_VIEW (self), NULL);
	return LD_CATEGORY_VIEW_GET_INTERFACE (self)->get_category (self);
}
