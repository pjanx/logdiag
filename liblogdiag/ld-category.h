/*
 * ld-category.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_CATEGORY_H__
#define __LD_CATEGORY_H__

G_BEGIN_DECLS


#define LD_TYPE_CATEGORY (ld_category_get_type ())
#define LD_CATEGORY(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_CATEGORY, LdCategory))
#define LD_CATEGORY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_CATEGORY, LdCategoryClass))
#define LD_IS_CATEGORY(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_CATEGORY))
#define LD_IS_CATEGORY_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_CATEGORY))
#define LD_CATEGORY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_CATEGORY, LdCategoryClass))

typedef struct _LdCategory LdCategory;
typedef struct _LdCategoryPrivate LdCategoryPrivate;
typedef struct _LdCategoryClass LdCategoryClass;


/**
 * LdCategory:
 */
struct _LdCategory
{
/*< private >*/
	GObject parent_instance;
	LdCategoryPrivate *priv;
};

/* TODO: If required sometime, categories (and maybe symbols) should implement
 *       a "changed" signal. This can be somewhat tricky. The library might be
 *       a good candidate for what they call a proxy. See GtkUIManager.
 */
struct _LdCategoryClass
{
/*< private >*/
	GObjectClass parent_class;
};


GType ld_category_get_type (void) G_GNUC_CONST;

LdCategory *ld_category_new (const gchar *name, const gchar *human_name);

void ld_category_set_name (LdCategory *self, const gchar *name);
const gchar *ld_category_get_name (LdCategory *self);
void ld_category_set_human_name (LdCategory *self, const gchar *human_name);
const gchar *ld_category_get_human_name (LdCategory *self);

gboolean ld_category_insert_symbol (LdCategory *self,
	LdSymbol *symbol, gint pos);
void ld_category_remove_symbol (LdCategory *self, LdSymbol *symbol);
const GSList *ld_category_get_symbols (LdCategory *self);

gboolean ld_category_add_child (LdCategory *self, LdCategory *category);
void ld_category_remove_child (LdCategory *self, LdCategory *category);
const GSList *ld_category_get_children (LdCategory *self);


G_END_DECLS

#endif /* ! __LD_CATEGORY_H__ */

