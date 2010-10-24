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
 * SECTION:ld-symbol
 * @short_description: A symbol.
 * @see_also: #LdDocument, #LdCanvas
 *
 * #LdSymbol represents a symbol in the #LdDocument that is in turn
 * drawn onto the #LdCanvas.
 */

/*
 * LdSymbolPrivate:
 * @name: The name of this symbol.
 */
struct _LdSymbolPrivate
{
	gchar *name;
};

G_DEFINE_ABSTRACT_TYPE (LdSymbol, ld_symbol, G_TYPE_OBJECT);

enum
{
	PROP_0,
	PROP_NAME
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
	object_class->finalize = ld_symbol_finalize;

/**
 * LdSymbol:name:
 *
 * The name of this symbol.
 */
	pspec = g_param_spec_string ("name", "Name",
		"The name of this symbol.",
		"", G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_NAME, pspec);

	g_type_class_add_private (klass, sizeof (LdSymbolPrivate));
}

static void
ld_symbol_init (LdSymbol *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_SYMBOL, LdSymbolPrivate);
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
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_symbol_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdSymbol *self;

	self = LD_SYMBOL (object);
	switch (property_id)
	{
	case PROP_NAME:
		ld_symbol_set_name (self, g_value_get_string (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_symbol_finalize (GObject *gobject)
{
	LdSymbol *self;

	self = LD_SYMBOL (gobject);

	if (self->priv->name)
		g_free (self->priv->name);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_symbol_parent_class)->finalize (gobject);
}

/**
 * ld_symbol_set_name:
 * @self: An #LdSymbol object.
 * @name: A new name for the symbol.
 *
 * Set the name of a symbol.
 */
void
ld_symbol_set_name (LdSymbol *self, const gchar *name)
{
	g_return_if_fail (LD_IS_SYMBOL (self));
	g_return_if_fail (name != NULL);

	if (self->priv->name)
		g_free (self->priv->name);
	self->priv->name = g_strdup (name);
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
	g_return_if_fail (LD_IS_SYMBOL (self));
	return self->priv->name;
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
	if (klass->draw)
		klass->draw (self, cr);
}
