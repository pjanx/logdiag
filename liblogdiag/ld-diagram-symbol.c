/*
 * ld-diagram-symbol.c
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
 * SECTION:ld-diagram-symbol
 * @short_description: A symbol object.
 * @see_also: #LdDiagramObject
 *
 * #LdDiagramSymbol is an implementation of #LdDiagramObject.
 */

enum
{
	PROP_0,
	PROP_CLASS
};

static void ld_diagram_symbol_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_diagram_symbol_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);


G_DEFINE_TYPE (LdDiagramSymbol, ld_diagram_symbol, LD_TYPE_DIAGRAM_OBJECT);

static void
ld_diagram_symbol_class_init (LdDiagramSymbolClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_diagram_symbol_get_property;
	object_class->set_property = ld_diagram_symbol_set_property;

/**
 * LdDiagramSymbol:class:
 *
 * The class of this symbol.
 */
	pspec = g_param_spec_string ("class", "Class",
		"The class of this symbol.",
		"", G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_CLASS, pspec);
}

static void
ld_diagram_symbol_init (LdDiagramSymbol *self)
{
}

static void
ld_diagram_symbol_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdDiagramObject *self;

	self = LD_DIAGRAM_OBJECT (object);
	switch (property_id)
	{
	case PROP_CLASS:
		ld_diagram_object_get_data_for_param (self, value, pspec);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_diagram_symbol_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdDiagramObject *self;

	self = LD_DIAGRAM_OBJECT (object);
	switch (property_id)
	{
	case PROP_CLASS:
		ld_diagram_object_set_data_for_param (self, value, pspec);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}


/**
 * ld_diagram_symbol_new:
 * @storage: A storage backend.
 *
 * Return value: A new #LdDiagramSymbol object.
 */
LdDiagramSymbol *
ld_diagram_symbol_new (JsonObject *storage)
{
	LdDiagramSymbol *self;

	self = g_object_new (LD_TYPE_DIAGRAM_SYMBOL, "storage", storage, NULL);
	return self;
}

/**
 * ld_diagram_symbol_get_class:
 * @self: An #LdDiagramSymbol object.
 *
 * Return value: The class of the symbol.
 */
const gchar *
ld_diagram_symbol_get_class (LdDiagramSymbol *self)
{
	const gchar *klass;

	g_return_val_if_fail (LD_IS_DIAGRAM_SYMBOL (self), NULL);
	g_object_get (self, "class", &klass, NULL);
	return klass;
}

/**
 * ld_diagram_symbol_set_class:
 * @self: An #LdDiagramSymbol object.
 * @klass: The class.
 *
 * Set the class of the symbol.
 */
void
ld_diagram_symbol_set_class (LdDiagramSymbol *self, const gchar *klass)
{
	g_return_if_fail (LD_IS_DIAGRAM_SYMBOL (self));
	g_object_set (self, "class", klass, NULL);
}
