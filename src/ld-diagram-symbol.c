/*
 * ld-diagram-symbol.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010 - 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>

#include "config.h"

#include "ld-diagram-object.h"
#include "ld-diagram-symbol.h"


/**
 * SECTION:ld-diagram-symbol
 * @short_description: A symbol object.
 * @see_also: #LdDiagramObject
 *
 * #LdDiagramSymbol is an implementation of #LdDiagramObject.
 */

/*
 * LdDiagramSymbolPrivate:
 * @klass: The class of this symbol.
 */
struct _LdDiagramSymbolPrivate
{
	gchar *klass;
};

enum
{
	PROP_0,
	PROP_CLASS
};

static void ld_diagram_symbol_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_diagram_symbol_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);
static void ld_diagram_symbol_finalize (GObject *gobject);


G_DEFINE_TYPE (LdDiagramSymbol, ld_diagram_symbol, LD_TYPE_DIAGRAM_OBJECT);

static void
ld_diagram_symbol_class_init (LdDiagramSymbolClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_diagram_symbol_get_property;
	object_class->set_property = ld_diagram_symbol_set_property;
	object_class->finalize = ld_diagram_symbol_finalize;

/**
 * LdDiagramSymbol:class:
 *
 * The class of this symbol.
 */
	pspec = g_param_spec_string ("class", "Class",
		"The class of this symbol.",
		"", G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_CLASS, pspec);

	g_type_class_add_private (klass, sizeof (LdDiagramSymbolPrivate));
}

static void
ld_diagram_symbol_init (LdDiagramSymbol *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_DIAGRAM_SYMBOL, LdDiagramSymbolPrivate);
}

static void
ld_diagram_symbol_finalize (GObject *gobject)
{
	LdDiagramSymbol *self;

	self = LD_DIAGRAM_SYMBOL (gobject);

	if (self->priv->klass)
		g_free (self->priv->klass);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_diagram_symbol_parent_class)->finalize (gobject);
}

static void
ld_diagram_symbol_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdDiagramSymbol *self;

	self = LD_DIAGRAM_SYMBOL (object);
	switch (property_id)
	{
	case PROP_CLASS:
		g_value_set_string (value, ld_diagram_symbol_get_class (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_diagram_symbol_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdDiagramSymbol *self;

	self = LD_DIAGRAM_SYMBOL (object);
	switch (property_id)
	{
	case PROP_CLASS:
		ld_diagram_symbol_set_class (self, g_value_get_string (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}


/**
 * ld_diagram_symbol_new:
 * @klass: The class of the new symbol.
 *
 * Return value: A new #LdDiagramSymbol object.
 */
LdDiagramSymbol *
ld_diagram_symbol_new (const gchar *klass)
{
	LdDiagramSymbol *self;

	self = g_object_new (LD_TYPE_DIAGRAM_SYMBOL, NULL);
	ld_diagram_symbol_set_class (self, klass);
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
	g_return_val_if_fail (LD_IS_DIAGRAM_SYMBOL (self), NULL);
	return self->priv->klass;
}

/**
 * ld_diagram_symbol_get_class:
 * @self: An #LdDiagramSymbol object.
 * @klass: The class.
 *
 * Set the class of the symbol.
 */
void
ld_diagram_symbol_set_class (LdDiagramSymbol *self, const gchar *klass)
{
	g_return_if_fail (LD_IS_DIAGRAM_SYMBOL (self));

	if (self->priv->klass)
		g_free (self->priv->klass);
	self->priv->klass = g_strdup (klass);
}
