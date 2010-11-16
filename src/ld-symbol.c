/*
 * ld-symbol.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>

#include "config.h"

#include "ld-symbol.h"
#include "ld-symbol-category.h"
#include "ld-library.h"


/**
 * ld_symbol_area_copy:
 * @self: An #LdSymbolArea structure.
 *
 * Makes a copy of the structure.
 * The result must be freed by ld_symbol_area_free().
 *
 * Return value: A copy of @self.
 **/
LdSymbolArea *
ld_symbol_area_copy (const LdSymbolArea *self)
{
	LdSymbolArea *new_area;

	g_return_val_if_fail (self != NULL, NULL);

	new_area = g_slice_new (LdSymbolArea);
	*new_area = *self;
	return new_area;
}

/**
 * ld_symbol_area_free:
 * @self: An #LdSymbolArea structure.
 *
 * Frees the structure created with ld_symbol_area_copy().
 **/
void
ld_symbol_area_free (LdSymbolArea *self)
{
	g_return_if_fail (self != NULL);

	g_slice_free (LdSymbolArea, self);
}

GType
ld_symbol_area_get_type (void)
{
	static GType our_type = 0;

	if (our_type == 0)
		our_type = g_boxed_type_register_static
			(g_intern_static_string ("LdSymbolArea"),
			(GBoxedCopyFunc) ld_symbol_area_copy,
			(GBoxedFreeFunc) ld_symbol_area_free);
	return our_type;
}


/**
 * SECTION:ld-symbol
 * @short_description: A symbol.
 * @see_also: #LdDocument, #LdCanvas
 *
 * #LdSymbol represents a symbol in the #LdDocument that is in turn
 * drawn onto the #LdCanvas.
 *
 * All implementations of this abstract class are required to use
 * cairo_save() and cairo_restore() when drawing to store the state.
 */

G_DEFINE_ABSTRACT_TYPE (LdSymbol, ld_symbol, G_TYPE_OBJECT);

enum
{
	PROP_0,
	PROP_NAME,
	PROP_HUMAN_NAME,
	PROP_AREA
};

static void
ld_symbol_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);

static void
ld_symbol_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);

static void ld_symbol_finalize (GObject *gobject);


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
		LD_TYPE_SYMBOL_AREA, G_PARAM_READABLE);
	g_object_class_install_property (object_class, PROP_AREA, pspec);
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
			LdSymbolArea area;

			ld_symbol_get_area (self, &area);
			g_value_set_boxed (value, &area);
		}
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
 * @self: An #LdSymbol object.
 *
 * Return value: The name of the symbol.
 */
const gchar *
ld_symbol_get_name (LdSymbol *self)
{
	LdSymbolClass *klass;

	g_return_if_fail (LD_IS_SYMBOL (self));

	klass = LD_SYMBOL_GET_CLASS (self);
	g_return_val_if_fail (klass->get_name != NULL, NULL);
	return klass->get_name (self);
}

/**
 * ld_symbol_get_human_name:
 * @self: An #LdSymbol object.
 *
 * Return value: The localised human name of the symbol.
 */
const gchar *
ld_symbol_get_human_name (LdSymbol *self)
{
	LdSymbolClass *klass;

	g_return_if_fail (LD_IS_SYMBOL (self));

	klass = LD_SYMBOL_GET_CLASS (self);
	g_return_val_if_fail (klass->get_human_name != NULL, NULL);
	return klass->get_human_name (self);
}

/**
 * ld_symbol_get_area:
 * @self: A symbol object.
 * @area: Where the area of the symbol will be returned.
 *
 * Get the area of the symbol.
 */
void
ld_symbol_get_area (LdSymbol *self, LdSymbolArea *area)
{
	LdSymbolClass *klass;

	g_return_if_fail (LD_IS_SYMBOL (self));
	g_return_if_fail (area != NULL);

	klass = LD_SYMBOL_GET_CLASS (self);
	g_return_if_fail (klass->get_area != NULL);
	klass->get_area (self, area);
}

/**
 * ld_symbol_draw:
 * @self: A symbol object.
 * @cr: A cairo surface to be drawn on.
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
