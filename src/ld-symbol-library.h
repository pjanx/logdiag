/*
 * ld-symbol-library.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_SYMBOL_LIBRARY_H__
#define __LD_SYMBOL_LIBRARY_H__

G_BEGIN_DECLS


#define LD_TYPE_SYMBOL_LIBRARY (ld_symbol_library_get_type ())
#define LD_SYMBOL_LIBRARY(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_SYMBOL_LIBRARY, LdSymbolLibrary))
#define LD_SYMBOL_LIBRARY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_SYMBOL_LIBRARY, LdSymbolLibraryClass))
#define LD_IS_SYMBOL_LIBRARY(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_SYMBOL_LIBRARY))
#define LD_IS_SYMBOL_LIBRARY_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_SYMBOL_LIBRARY))
#define LD_SYMBOL_LIBRARY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_SYMBOL_LIBRARY, LdSymbolLibraryClass))

typedef struct _LdSymbolLibrary LdSymbolLibrary;
typedef struct _LdSymbolLibraryPrivate LdSymbolLibraryPrivate;
typedef struct _LdSymbolLibraryClass LdSymbolLibraryClass;


/**
 * LdSymbolLibrary:
 * @categories: Lists all the categories (#LdSymbolCategory).
 *
 * Object structure.
 */
struct _LdSymbolLibrary
{
/*< private >*/
	GObject parent_instance;
	LdSymbolLibraryPrivate *priv;

/*< public >*/
	GHashTable *categories;
};

struct _LdSymbolLibraryClass
{
/*< private >*/
	GObjectClass parent_class;
	guint changed_signal;
};


GType ld_symbol_library_get_type (void) G_GNUC_CONST;

LdSymbolLibrary *ld_symbol_library_new (void);
gboolean ld_symbol_library_load (LdSymbolLibrary *self,
	const gchar *directory);
void ld_symbol_library_clear (LdSymbolLibrary *self);


G_END_DECLS

#endif /* ! __LD_SYMBOL_LIBRARY_H__ */

