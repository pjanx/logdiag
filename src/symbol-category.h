/*
 * symbol-category.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __SYMBOL_CATEGORY_H__
#define __SYMBOL_CATEGORY_H__

G_BEGIN_DECLS


#define LOGDIAG_TYPE_SYMBOL_CATEGORY (logdiag_symbol_category_get_type ())
#define LOGDIAG_SYMBOL_CATEGORY(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LOGDIAG_TYPE_SYMBOL_CATEGORY, LogdiagSymbolCategory))
#define LOGDIAG_SYMBOL_CATEGORY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LOGDIAG_TYPE_SYMBOL_CATEGORY, LogdiagSymbolCategoryClass))
#define LOGDIAG_IS_SYMBOL_CATEGORY(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LOGDIAG_TYPE_SYMBOL_CATEGORY))
#define LOGDIAG_IS_SYMBOL_CATEGORY_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LOGDIAG_TYPE_SYMBOL_CATEGORY))
#define LOGDIAG_SYMBOL_CATEGORY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LOGDIAG_SYMBOL_CATEGORY, LogdiagSymbolCategoryClass))

typedef struct _LogdiagSymbolCategory LogdiagSymbolCategory;
typedef struct _LogdiagSymbolCategoryClass LogdiagSymbolCategoryClass;


/**
 * LogdiagSymbolCategory:
 * @parent: The parent object, may be LogdiagSymbolLibrary
 * or another LogdiagSymbolCategory.
 * @name: The name of the category.
 * @image_path: Path to the image of the category.
 * @children: Children of this category.
 */
struct _LogdiagSymbolCategory
{
/*< private >*/
	GObject parent_instance;

/*< public >*/
	gpointer parent;
	gchar *name;
	gchar *image_path;
	GHashTable *children;
};

struct _LogdiagSymbolCategoryClass
{
	GtkObjectClass parent_class;
};


GType logdiag_symbol_category_get_type (void) G_GNUC_CONST;

LogdiagSymbolCategory *
logdiag_symbol_category_new (LogdiagSymbolLibrary *parent);


G_END_DECLS

#endif /* ! __SYMBOL_CATEGORY_H__ */

