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


#define LD_TYPE_SYMBOL_AREA (ld_symbol_area_get_type ())

typedef struct _LdSymbolArea LdSymbolArea;

/**
 * LdSymbolArea:
 * @x1: Left-top X coordinate.
 * @y1: Left-top Y coordinate.
 * @x2: Right-bottom X coordinate.
 * @y2: Right-bottom Y coordinate.
 *
 * Defines the area of the symbol relative to the center of the symbol,
 * which is at the (0, 0) coordinates.
 */
struct _LdSymbolArea
{
	gdouble x1, y1;
	gdouble x2, y2;
};


GType ld_symbol_area_get_type (void) G_GNUC_CONST;

LdSymbolArea *ld_symbol_area_copy (const LdSymbolArea *self);
void ld_symbol_area_free (LdSymbolArea *self);


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
};

/**
 * LdSymbolClass:
 * @parent_class: The parent class.
 * @get_name: Get the name of the symbol.
 * @get_human_name: Get the localized human name of the symbol.
 * @get_area: Get the area of the symbol.
 * @draw: Draw the symbol on a Cairo surface.
 */
struct _LdSymbolClass
{
	GObjectClass parent_class;

	const gchar *(*get_name) (LdSymbol *self);
	const gchar *(*get_human_name) (LdSymbol *self);
	void (*get_area) (LdSymbol *self, LdSymbolArea *area);
	void (*draw) (LdSymbol *self, cairo_t *cr);
};


GType ld_symbol_get_type (void) G_GNUC_CONST;

const gchar *ld_symbol_get_name (LdSymbol *self);
const gchar *ld_symbol_get_human_name (LdSymbol *self);
void ld_symbol_get_area (LdSymbol *self, LdSymbolArea *area);
void ld_symbol_draw (LdSymbol *self, cairo_t *cr);

/* TODO: Interface for terminals.
 *       Something like a list of gdouble pairs (-> a new structure).
 */


G_END_DECLS

#endif /* ! __LD_SYMBOL_H__ */

