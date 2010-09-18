/*
 * ld-symbol.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_SYMBOL_H__
#define __LD_SYMBOL_H__

G_BEGIN_DECLS


#define LD_TYPE_SYMBOL (ld_symbol_get_type ())
#define LD_SYMBOL(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_SYMBOL, LdSymbol))
#define LD_SYMBOL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_SYMBOL, LdSymbolClass))
#define LD_IS_SYMBOL(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_SYMBOL))
#define LD_IS_SYMBOL_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_SYMBOL))
#define LD_SYMBOL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_SYMBOL, LdSymbolClass))

typedef struct _LdSymbol LdSymbol;
typedef struct _LdSymbolPrivate LdSymbolPrivate;
typedef struct _LdSymbolClass LdSymbolClass;


/**
 * LdSymbol:
 * @name: The name of this symbol.
 */
struct _LdSymbol
{
/*< private >*/
	GObject parent_instance;
	LdSymbolPrivate *priv;

/*< public >*/
	gchar *name;
};

struct _LdSymbolClass
{
	GObjectClass parent_class;
};


GType ld_symbol_get_type (void) G_GNUC_CONST;

LdSymbol *ld_symbol_new (LdSymbolLibrary *library,
	const gchar *filename);
gchar *ld_symbol_build_identifier (LdSymbol *self);
void ld_symbol_draw (LdSymbol *self, cairo_t *surface,
	GHashTable *param, gint x, gint y, gdouble zoom);

/* TODO: An interface for symbol terminals. */


G_END_DECLS

#endif /* ! __LD_SYMBOL_H__ */

