/*
 * ld-symbol.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010 - 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-symbol
 * @short_description: A symbol
 * @see_also: #LdDiagramSymbol, #LdDiagramView
 *
 * #LdSymbol represents a symbol to be drawn by #LdDiagramView.
 *
 * All implementations of this abstract class are required to use
 * cairo_save() and cairo_restore() when drawing to store the state.
 */

enum
{
	PROP_0,
	PROP_NAME,
	PROP_HUMAN_NAME,
	PROP_AREA,
	PROP_TERMINALS
};

static void ld_symbol_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_symbol_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);


G_DEFINE_ABSTRACT_TYPE (LdSymbol, ld_symbol, G_TYPE_OBJECT);

static void
ld_symbol_class_init (LdSymbolClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_symbol_get_property;
	object_class->set_property = ld_symbol_set_property;

/**
 * LdSymbol:name:
 *
 * The name of this symbol.
 */
	pspec = g_param_spec_string ("name", "Name",
		"The name of this symbol.",
		"", G_PARAM_READABLE);
	g_object_class_install_property (object_class, PROP_NAME, pspec);

/**
 * LdSymbol:human-name:
 *
 * The localized human name of this symbol.
 */
	pspec = g_param_spec_string ("human-name", "Human name",
		"The localized human name of this symbol.",
		"", G_PARAM_READABLE);
	g_object_class_install_property (object_class, PROP_HUMAN_NAME, pspec);

/**
 * LdSymbol:area:
 *
 * The area of this symbol.
 */
	pspec = g_param_spec_boxed ("area", "Area",
		"The area of this symbol.",
		LD_TYPE_RECTANGLE, G_PARAM_READABLE);
	g_object_class_install_property (object_class, PROP_AREA, pspec);

/**
 * LdSymbol:terminals:
 *
 * A point array that specifies terminals of this symbol.
 */
	pspec = g_param_spec_boxed ("terminals", "Terminals",
		"A point array that specifies terminals of this symbol.",
		LD_TYPE_POINT_ARRAY, G_PARAM_READABLE);
	g_object_class_install_property (object_class, PROP_TERMINALS, pspec);
}

static void
ld_symbol_init (LdSymbol *self)
{
}

static void
ld_symbol_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdSymbol *self;

	self = LD_SYMBOL (object);
	switch (property_id)
	{
	case PROP_NAME:
		g_value_set_string (value, ld_symbol_get_name (self));
		break;
	case PROP_HUMAN_NAME:
		g_value_set_string (value, ld_symbol_get_human_name (self));
		break;
	case PROP_AREA:
		{
			LdRectangle area;

			ld_symbol_get_area (self, &area);
			g_value_set_boxed (value, &area);
		}
		break;
	case PROP_TERMINALS:
		g_value_set_boxed (value, ld_symbol_get_terminals (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_symbol_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}


/**
 * ld_symbol_get_name:
 * @self: an #LdSymbol object.
 *
 * Return value: the name of the symbol.
 */
const gchar *
ld_symbol_get_name (LdSymbol *self)
{
	LdSymbolClass *klass;

	g_return_val_if_fail (LD_IS_SYMBOL (self), NULL);

	klass = LD_SYMBOL_GET_CLASS (self);
	g_return_val_if_fail (klass->get_name != NULL, NULL);
	return klass->get_name (self);
}

/**
 * ld_symbol_get_human_name:
 * @self: an #LdSymbol object.
 *
 * Return value: the localised human name of the symbol.
 */
const gchar *
ld_symbol_get_human_name (LdSymbol *self)
{
	LdSymbolClass *klass;

	g_return_val_if_fail (LD_IS_SYMBOL (self), NULL);

	klass = LD_SYMBOL_GET_CLASS (self);
	g_return_val_if_fail (klass->get_human_name != NULL, NULL);
	return klass->get_human_name (self);
}

/**
 * ld_symbol_get_area:
 * @self: an #LdSymbol object.
 * @area: where the area of the symbol will be returned.
 *
 * Get the area of the symbol.
 */
void
ld_symbol_get_area (LdSymbol *self, LdRectangle *area)
{
	LdSymbolClass *klass;

	g_return_if_fail (LD_IS_SYMBOL (self));
	g_return_if_fail (area != NULL);

	klass = LD_SYMBOL_GET_CLASS (self);
	g_return_if_fail (klass->get_area != NULL);
	klass->get_area (self, area);
}

/**
 * ld_symbol_get_terminals:
 * @self: an #LdSymbol object.
 *
 * Get a list of symbol terminals.
 *
 * Return value: an #LdPointArray structure.
 */
const LdPointArray *
ld_symbol_get_terminals (LdSymbol *self)
{
	LdSymbolClass *klass;

	g_return_val_if_fail (LD_IS_SYMBOL (self), NULL);

	klass = LD_SYMBOL_GET_CLASS (self);
	g_return_val_if_fail (klass->get_terminals != NULL, NULL);
	return klass->get_terminals (self);
}

/**
 * ld_symbol_draw:
 * @self: an #LdSymbol object.
 * @cr: a cairo surface to be drawn on.
 *
 * Draw the symbol onto a Cairo surface.
 */
void
ld_symbol_draw (LdSymbol *self, cairo_t *cr)
{
	LdSymbolClass *klass;

	g_return_if_fail (LD_IS_SYMBOL (self));
	g_return_if_fail (cr != NULL);

	klass = LD_SYMBOL_GET_CLASS (self);
	g_return_if_fail (klass->draw != NULL);
	klass->draw (self, cr);
}
