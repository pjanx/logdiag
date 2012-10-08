/*
 * ld-category-symbol-view.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_CATEGORY_SYMBOL_VIEW_H__
#define __LD_CATEGORY_SYMBOL_VIEW_H__

G_BEGIN_DECLS


#define LD_TYPE_CATEGORY_SYMBOL_VIEW (ld_category_symbol_view_get_type ())
#define LD_CATEGORY_SYMBOL_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_CATEGORY_SYMBOL_VIEW, LdCategorySymbolView))
#define LD_CATEGORY_SYMBOL_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_CATEGORY_SYMBOL_VIEW, LdCategorySymbolViewClass))
#define LD_IS_CATEGORY_SYMBOL_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_CATEGORY_SYMBOL_VIEW))
#define LD_IS_CATEGORY_SYMBOL_VIEW_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_CATEGORY_SYMBOL_VIEW))
#define LD_CATEGORY_SYMBOL_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_CATEGORY_SYMBOL_VIEW, LdCategorySymbolViewClass))

typedef struct _LdCategorySymbolView LdCategorySymbolView;
typedef struct _LdCategorySymbolViewPrivate LdCategorySymbolViewPrivate;
typedef struct _LdCategorySymbolViewClass LdCategorySymbolViewClass;


/**
 * LdCategorySymbolView:
 */
struct _LdCategorySymbolView
{
/*< private >*/
	GtkDrawingArea parent_instance;
	LdCategorySymbolViewPrivate *priv;
};

struct _LdCategorySymbolViewClass
{
/*< private >*/
	GtkDrawingAreaClass parent_class;
};


GType ld_category_symbol_view_get_type (void) G_GNUC_CONST;

GtkWidget *ld_category_symbol_view_new (LdCategory *category);


G_END_DECLS

#endif /* ! __LD_CATEGORY_SYMBOL_VIEW_H__ */
