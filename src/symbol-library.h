/*
 * symbol-library.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __SYMBOL_LIBRARY_H__
#define __SYMBOL_LIBRARY_H__

G_BEGIN_DECLS


#define LOGDIAG_TYPE_SYMBOL_LIBRARY (logdiag_symbol_library_get_type ())
#define LOGDIAG_SYMBOL_LIBRARY(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LOGDIAG_TYPE_SYMBOL_LIBRARY, LogdiagSymbolLibrary))
#define LOGDIAG_SYMBOL_LIBRARY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LOGDIAG_TYPE_SYMBOL_LIBRARY, LogdiagSymbolLibraryClass))
#define LOGDIAG_IS_SYMBOL_LIBRARY(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LOGDIAG_TYPE_SYMBOL_LIBRARY))
#define LOGDIAG_IS_SYMBOL_LIBRARY_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LOGDIAG_TYPE_SYMBOL_LIBRARY))
#define LOGDIAG_SYMBOL_LIBRARY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LOGDIAG_SYMBOL_LIBRARY, LogdiagSymbolLibraryClass))

typedef struct _LogdiagSymbolLibrary LogdiagSymbolLibrary;
typedef struct _LogdiagSymbolLibraryPrivate LogdiagSymbolLibraryPrivate;
typedef struct _LogdiagSymbolLibraryClass LogdiagSymbolLibraryClass;


/**
 * LogdiagSymbolLibrary:
 * @categories: Lists all the categories (#LogdiagSymbolCategory).
 *
 * Object structure.
 */
struct _LogdiagSymbolLibrary
{
/*< private >*/
	GObject parent_instance;
	LogdiagSymbolLibraryPrivate *priv;

/*< public >*/
	GHashTable *categories;
};

struct _LogdiagSymbolLibraryClass
{
/*< private >*/
	GObjectClass parent_class;
	guint changed_signal;
};


GType logdiag_symbol_library_get_type (void) G_GNUC_CONST;

LogdiagSymbolLibrary *logdiag_symbol_library_new (void);
gboolean logdiag_symbol_library_load (LogdiagSymbolLibrary *self,
	const char *directory);
void logdiag_symbol_library_clear (LogdiagSymbolLibrary *self);


G_END_DECLS

#endif /* ! __SYMBOL_LIBRARY_H__ */

