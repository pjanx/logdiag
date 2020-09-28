/*
 * ld-category-view.h
 *
 * This file is a part of logdiag.
 * Copyright 2012 Přemysl Eric Janouch
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
#define LD_IS_CATEGORY_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_CATEGORY_VIEW))
#define LD_CATEGORY_VIEW_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE \
	((inst), LD_TYPE_CATEGORY_VIEW, LdCategoryViewInterface))

typedef struct _LdCategoryView LdCategoryView;
typedef struct _LdCategoryViewInterface LdCategoryViewInterface;

/**
 * LdCategoryView:
 */
struct _LdCategoryView
{
	/* Just to remind gtk-doc that this really exists. */
};

struct _LdCategoryViewInterface
{
/*< private >*/
	GTypeInterface parent;

	guint symbol_selected_signal;
	guint symbol_deselected_signal;

	void (*set_category) (LdCategoryView *self, LdCategory *category);
	LdCategory *(*get_category) (LdCategoryView *self);
};


GType ld_category_view_get_type (void) G_GNUC_CONST;

void ld_category_view_set_category (LdCategoryView *self,
	LdCategory *category);
LdCategory *ld_category_view_get_category (LdCategoryView *self);


G_END_DECLS

#endif /* ! __LD_CATEGORY_VIEW_H__ */
