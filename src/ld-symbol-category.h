/*
 * ld-symbol-category.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_SYMBOL_CATEGORY_H__
#define __LD_SYMBOL_CATEGORY_H__

G_BEGIN_DECLS


#define LD_TYPE_SYMBOL_CATEGORY (ld_symbol_category_get_type ())
#define LD_SYMBOL_CATEGORY(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_SYMBOL_CATEGORY, LdSymbolCategory))
#define LD_SYMBOL_CATEGORY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_SYMBOL_CATEGORY, LdSymbolCategoryClass))
#define LD_IS_SYMBOL_CATEGORY(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_SYMBOL_CATEGORY))
#define LD_IS_SYMBOL_CATEGORY_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_SYMBOL_CATEGORY))
#define LD_SYMBOL_CATEGORY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_SYMBOL_CATEGORY, LdSymbolCategoryClass))

typedef struct _LdSymbolCategory LdSymbolCategory;
typedef struct _LdSymbolCategoryClass LdSymbolCategoryClass;


/**
 * LdSymbolCategory:
 * @parent: The parent object, may be LdSymbolLibrary
 * or another LdSymbolCategory.
 * @name: The name of the category.
 * @image_path: Path to the image for this category.
 * @children: Children of this category.
 */
struct _LdSymbolCategory
{
/*< private >*/
	GObject parent_instance;

/*< public >*/
	gpointer parent;
	gchar *name;
	gchar *image_path;
	GHashTable *children;
};

struct _LdSymbolCategoryClass
{
	GObjectClass parent_class;
};


GType ld_symbol_category_get_type (void) G_GNUC_CONST;

LdSymbolCategory *
ld_symbol_category_new (LdSymbolLibrary *parent);


G_END_DECLS

#endif /* ! __LD_SYMBOL_CATEGORY_H__ */

