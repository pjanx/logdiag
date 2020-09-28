/*
 * ld-symbol.h
 *
 * This file is a part of logdiag.
 * Copyright 2010, 2011 Přemysl Eric Janouch
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


struct _LdSymbol
{
/*< private >*/
	GObject parent_instance;
	LdSymbolPrivate *priv;
};

/**
 * LdSymbolClass:
 * @get_name: get the name of the symbol.
 * @get_human_name: get the localized human name of the symbol.
 * @get_area: get the area of the symbol.
 * @get_terminals: get a list of symbol terminals.
 * @draw: draw the symbol on a Cairo surface.
 */
struct _LdSymbolClass
{
/*< private >*/
	GObjectClass parent_class;

/*< public >*/
	const gchar *(*get_name) (LdSymbol *self);
	const gchar *(*get_human_name) (LdSymbol *self);
	void (*get_area) (LdSymbol *self, LdRectangle *area);
	const LdPointArray *(*get_terminals) (LdSymbol *self);
	void (*draw) (LdSymbol *self, cairo_t *cr);
};


GType ld_symbol_get_type (void) G_GNUC_CONST;

const gchar *ld_symbol_get_name (LdSymbol *self);
const gchar *ld_symbol_get_human_name (LdSymbol *self);
void ld_symbol_get_area (LdSymbol *self, LdRectangle *area);
const LdPointArray *ld_symbol_get_terminals (LdSymbol *self);
void ld_symbol_draw (LdSymbol *self, cairo_t *cr);


G_END_DECLS

#endif /* ! __LD_SYMBOL_H__ */

