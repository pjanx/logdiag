/*
 * ld-diagram-symbol.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
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

G_DEFINE_TYPE (LdDiagramSymbol, ld_diagram_symbol, LD_TYPE_DIAGRAM_OBJECT);

static void ld_diagram_symbol_finalize (GObject *gobject);


static void
ld_diagram_symbol_class_init (LdDiagramSymbolClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_diagram_symbol_finalize;

	/* TODO: A property for the class. */

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


/**
 * ld_diagram_symbol_new:
 * @klass: The class of the symbol (symbol identifier).
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


/* TODO: gtk-doc comments. */
const gchar *
ld_diagram_symbol_get_class (LdDiagramSymbol *self)
{
	g_return_val_if_fail (LD_IS_DIAGRAM_SYMBOL (self), NULL);
	return self->priv->klass;
}

void
ld_diagram_symbol_set_class (LdDiagramSymbol *self, const gchar *klass)
{
	g_return_if_fail (LD_IS_DIAGRAM_SYMBOL (self));

	if (self->priv->klass)
		g_free (self->priv->klass);
	self->priv->klass = g_strdup (klass);
}
