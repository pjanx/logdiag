/*
 * ld-diagram-symbol.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010, 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_DIAGRAM_SYMBOL_H__
#define __LD_DIAGRAM_SYMBOL_H__

G_BEGIN_DECLS


#define LD_TYPE_DIAGRAM_SYMBOL (ld_diagram_symbol_get_type ())
#define LD_DIAGRAM_SYMBOL(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_DIAGRAM_SYMBOL, LdDiagramSymbol))
#define LD_DIAGRAM_SYMBOL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_DIAGRAM_SYMBOL, LdDiagramSymbolClass))
#define LD_IS_DIAGRAM_SYMBOL(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_DIAGRAM_SYMBOL))
#define LD_IS_DIAGRAM_SYMBOL_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_DIAGRAM_SYMBOL))
#define LD_DIAGRAM_SYMBOL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_DIAGRAM_SYMBOL, LdDiagramSymbolClass))

typedef struct _LdDiagramSymbol LdDiagramSymbol;
typedef struct _LdDiagramSymbolPrivate LdDiagramSymbolPrivate;
typedef struct _LdDiagramSymbolClass LdDiagramSymbolClass;


/**
 * LdDiagramSymbol:
 */
struct _LdDiagramSymbol
{
/*< private >*/
	LdDiagramObject parent_instance;
};

/**
 * LdDiagramSymbolClass:
 */
struct _LdDiagramSymbolClass
{
/*< private >*/
	LdDiagramObjectClass parent_class;
};


enum
{
	LD_DIAGRAM_SYMBOL_ROTATION_0,
	LD_DIAGRAM_SYMBOL_ROTATION_90,
	LD_DIAGRAM_SYMBOL_ROTATION_180,
	LD_DIAGRAM_SYMBOL_ROTATION_270
};

GType ld_diagram_symbol_get_type (void) G_GNUC_CONST;

LdDiagramSymbol *ld_diagram_symbol_new (JsonObject *storage);
gchar *ld_diagram_symbol_get_class (LdDiagramSymbol *self);
void ld_diagram_symbol_set_class (LdDiagramSymbol *self, const gchar *klass);
gint ld_diagram_symbol_get_rotation (LdDiagramSymbol *self);
void ld_diagram_symbol_set_rotation (LdDiagramSymbol *self, gint rotation);


G_END_DECLS

#endif /* ! __LD_DIAGRAM_SYMBOL_H__ */

