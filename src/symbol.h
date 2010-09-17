/*
 * symbol.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __SYMBOL_H__
#define __SYMBOL_H__

G_BEGIN_DECLS


#define LOGDIAG_TYPE_SYMBOL (logdiag_symbol_get_type ())
#define LOGDIAG_SYMBOL(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LOGDIAG_TYPE_SYMBOL, LogdiagSymbol))
#define LOGDIAG_SYMBOL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LOGDIAG_TYPE_SYMBOL, LogdiagSymbolClass))
#define LOGDIAG_IS_SYMBOL(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LOGDIAG_TYPE_SYMBOL))
#define LOGDIAG_IS_SYMBOL_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LOGDIAG_TYPE_SYMBOL))
#define LOGDIAG_SYMBOL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LOGDIAG_SYMBOL, LogdiagSymbolClass))

typedef struct _LogdiagSymbol LogdiagSymbol;
typedef struct _LogdiagSymbolPrivate LogdiagSymbolPrivate;
typedef struct _LogdiagSymbolClass LogdiagSymbolClass;


/**
 * LogdiagSymbol:
 * @name: The name of this symbol.
 */
struct _LogdiagSymbol
{
/*< private >*/
	GObject parent_instance;
	LogdiagSymbolPrivate *priv;

/*< public >*/
	gchar *name;
};

struct _LogdiagSymbolClass
{
	GObjectClass parent_class;
};


GType logdiag_symbol_get_type (void) G_GNUC_CONST;

LogdiagSymbol *logdiag_symbol_new (LogdiagSymbolLibrary *library,
	const gchar *filename);
char *logdiag_symbol_build_identifier (LogdiagSymbol *self);
void logdiag_symbol_draw (LogdiagSymbol *self, cairo_t *surface,
	GHashTable *param, gint x, gint y, gdouble zoom);

/* TODO: Funkce pro získání terminálů. */



G_END_DECLS

#endif /* ! __SYMBOL_H__ */

