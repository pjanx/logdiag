/*
 * ld-category-tree-view.h
 *
 * This file is a part of logdiag.
 * Copyright 2011, 2012 Přemysl Eric Janouch
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_CATEGORY_TREE_VIEW_H__
#define __LD_CATEGORY_TREE_VIEW_H__

G_BEGIN_DECLS


#define LD_TYPE_CATEGORY_TREE_VIEW (ld_category_tree_view_get_type ())
#define LD_CATEGORY_TREE_VIEW(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), \
		LD_TYPE_CATEGORY_TREE_VIEW, \
		LdCategoryTreeView))
#define LD_CATEGORY_TREE_VIEW_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
		LD_TYPE_CATEGORY_TREE_VIEW, \
		LdCategoryTreeViewClass))
#define LD_IS_CATEGORY_TREE_VIEW(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), LD_TYPE_CATEGORY_TREE_VIEW))
#define LD_IS_CATEGORY_TREE_VIEW_CLASS(klass) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((klass), LD_TYPE_CATEGORY_TREE_VIEW))
#define LD_CATEGORY_TREE_VIEW_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
		LD_CATEGORY_TREE_VIEW, \
		LdCategoryTreeViewClass))

typedef struct _LdCategoryTreeView LdCategoryTreeView;
typedef struct _LdCategoryTreeViewPrivate LdCategoryTreeViewPrivate;
typedef struct _LdCategoryTreeViewClass LdCategoryTreeViewClass;


/**
 * LdCategoryTreeView:
 */
struct _LdCategoryTreeView
{
/*< private >*/
	GtkBox parent_instance;
	LdCategoryTreeViewPrivate *priv;
};

struct _LdCategoryTreeViewClass
{
/*< private >*/
	GtkBoxClass parent_class;
};


GType ld_category_tree_view_get_type (void) G_GNUC_CONST;

GtkWidget *ld_category_tree_view_new (LdCategory *category);


G_END_DECLS

#endif /* ! __LD_CATEGORY_TREE_VIEW_H__ */
