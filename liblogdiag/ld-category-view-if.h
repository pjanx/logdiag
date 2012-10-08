/*
 * ld-category-view-if.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2012. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_CATEGORY_VIEW_IF_H__
#define __LD_CATEGORY_VIEW_IF_H__

G_BEGIN_DECLS


#define LD_TYPE_CATEGORY_VIEW_IF (ld_category_view_if_get_type ())
#define LD_CATEGORY_VIEW_IF(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_CATEGORY_VIEW_IF, LdCategoryViewIf))
#define LD_IS_CATEGORY_VIEW_IF(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_CATEGORY_VIEW_IF))
#define LD_CATEGORY_VIEW_IF_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE \
	((inst), LD_TYPE_CATEGORY_VIEW_IF, LdCategoryViewIfInterface))

typedef struct _LdCategoryViewIf LdCategoryViewIf;
typedef struct _LdCategoryViewIfInterface LdCategoryViewIfInterface;

/**
 * LdCategoryViewIf:
 */
struct _LdCategoryViewIf
{
	/* Just to remind gtk-doc that this really exists. */
};

struct _LdCategoryViewIfInterface
{
/*< private >*/
	GTypeInterface parent;

	guint symbol_selected_signal;
	guint symbol_deselected_signal;

	void (*set_category) (LdCategoryViewIf *self, LdCategory *category);
	LdCategory *(*get_category) (LdCategoryViewIf *self);
};


GType ld_category_view_if_get_type (void) G_GNUC_CONST;

void ld_category_view_if_set_category (LdCategoryViewIf *self,
	LdCategory *category);
LdCategory *ld_category_view_if_get_category (LdCategoryViewIf *self);


G_END_DECLS

#endif /* ! __LD_CATEGORY_VIEW_IF_H__ */
