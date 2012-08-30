/*
 * ld-category-view.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_CATEGORY_VIEW_H__
#define __LD_CATEGORY_VIEW_H__

G_BEGIN_DECLS


#define LD_TYPE_CATEGORY_VIEW (ld_category_view_get_type ())
#define LD_CATEGORY_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_CATEGORY_VIEW, LdCategoryView))
#define LD_CATEGORY_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_CATEGORY_VIEW, LdCategoryViewClass))
#define LD_IS_CATEGORY_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_CATEGORY_VIEW))
#define LD_IS_CATEGORY_VIEW_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_CATEGORY_VIEW))
#define LD_CATEGORY_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_CATEGORY_VIEW, LdCategoryViewClass))

typedef struct _LdCategoryView LdCategoryView;
typedef struct _LdCategoryViewPrivate LdCategoryViewPrivate;
typedef struct _LdCategoryViewClass LdCategoryViewClass;


/**
 * LdCategoryView:
 */
struct _LdCategoryView
{
/*< private >*/
	GtkVBox parent_instance;
	LdCategoryViewPrivate *priv;
};

struct _LdCategoryViewClass
{
/*< private >*/
	GtkVBoxClass parent_class;
};


GType ld_category_view_get_type (void) G_GNUC_CONST;

GtkWidget *ld_category_view_new (void);

void ld_category_view_set_category (LdCategoryView *self, LdCategory *category);
LdCategory *ld_category_view_get_category (LdCategoryView *self);


G_END_DECLS

#endif /* ! __LD_CATEGORY_VIEW_H__ */
