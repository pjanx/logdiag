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
typedef struct _LdSymbolCategoryPrivate LdSymbolCategoryPrivate;
typedef struct _LdSymbolCategoryClass LdSymbolCategoryClass;


/**
 * LdSymbolCategory:
 */
struct _LdSymbolCategory
{
/*< private >*/
	GObject parent_instance;
	LdSymbolCategoryPrivate *priv;
};

/* TODO: If required sometime, categories (and maybe symbols) should implement
 *       a "changed" signal. This can be somewhat tricky. The library might be
 *       a good candidate for what they call a proxy. See GtkUIManager.
 */
struct _LdSymbolCategoryClass
{
/*< private >*/
	GObjectClass parent_class;
};


GType ld_symbol_category_get_type (void) G_GNUC_CONST;

LdSymbolCategory *ld_symbol_category_new (const gchar *name,
	const gchar *human_name);

void ld_symbol_category_set_name (LdSymbolCategory *self, const gchar *name);
const gchar *ld_symbol_category_get_name (LdSymbolCategory *self);
void ld_symbol_category_set_human_name (LdSymbolCategory *self,
	const gchar *human_name);
const gchar *ld_symbol_category_get_human_name (LdSymbolCategory *self);

gboolean ld_symbol_category_insert_symbol (LdSymbolCategory *self,
	LdSymbol *symbol, gint pos);
void ld_symbol_category_remove_symbol (LdSymbolCategory *self,
	LdSymbol *symbol);
const GSList *ld_symbol_category_get_symbols (LdSymbolCategory *self);

gboolean ld_symbol_category_insert_subcategory (LdSymbolCategory *self,
	LdSymbolCategory *category, gint pos);
void ld_symbol_category_remove_subcategory (LdSymbolCategory *self,
	LdSymbolCategory *category);
const GSList *ld_symbol_category_get_subcategories (LdSymbolCategory *self);


G_END_DECLS

#endif /* ! __LD_SYMBOL_CATEGORY_H__ */

