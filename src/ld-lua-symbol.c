/*
 * ld-lua-symbol.c
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

#include "ld-lua.h"
#include "ld-lua-symbol.h"

#include "ld-lua-symbol-private.h"


/**
 * SECTION:ld-lua-symbol
 * @short_description: A symbol.
 * @see_also: #LdSymbol
 *
 * #LdLuaSymbol is an implementation of #LdSymbol.
 */

G_DEFINE_TYPE (LdLuaSymbol, ld_lua_symbol, LD_TYPE_SYMBOL);

static void ld_lua_symbol_finalize (GObject *gobject);

static const gchar *ld_lua_symbol_get_name (LdSymbol *symbol);
static const gchar *ld_lua_symbol_get_human_name (LdSymbol *symbol);
static void ld_lua_symbol_get_area (LdSymbol *symbol, LdSymbolArea *area);
static void ld_lua_symbol_draw (LdSymbol *symbol, cairo_t *cr);


static void
ld_lua_symbol_class_init (LdLuaSymbolClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_lua_symbol_finalize;

	klass->parent_class.get_name = ld_lua_symbol_get_name;
	klass->parent_class.get_human_name = ld_lua_symbol_get_human_name;
	klass->parent_class.get_area = ld_lua_symbol_get_area;
	klass->parent_class.draw = ld_lua_symbol_draw;

	g_type_class_add_private (klass, sizeof (LdLuaSymbolPrivate));
}

static void
ld_lua_symbol_init (LdLuaSymbol *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_LUA_SYMBOL, LdLuaSymbolPrivate);
}

static void
ld_lua_symbol_finalize (GObject *gobject)
{
	LdLuaSymbol *self;

	self = LD_LUA_SYMBOL (gobject);

	ld_lua_private_unregister (self->priv->lua, self);
	g_object_unref (self->priv->lua);

	if (self->priv->name)
		g_free (self->priv->name);
	if (self->priv->human_name)
		g_free (self->priv->human_name);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_lua_symbol_parent_class)->finalize (gobject);
}


const gchar *
ld_lua_symbol_get_name (LdSymbol *symbol)
{
	g_return_val_if_fail (LD_IS_LUA_SYMBOL (symbol), NULL);
	return LD_LUA_SYMBOL (symbol)->priv->name;
}

const gchar *
ld_lua_symbol_get_human_name (LdSymbol *symbol)
{
	g_return_val_if_fail (LD_IS_LUA_SYMBOL (symbol), NULL);
	return LD_LUA_SYMBOL (symbol)->priv->human_name;
}

void
ld_lua_symbol_get_area (LdSymbol *symbol, LdSymbolArea *area)
{
	LdLuaSymbol *self;

	g_return_if_fail (LD_IS_LUA_SYMBOL (symbol));
	g_return_if_fail (area != NULL);

	self = LD_LUA_SYMBOL (symbol);
	*area = self->priv->area;
}

static void
ld_lua_symbol_draw (LdSymbol *symbol, cairo_t *cr)
{
	LdLuaSymbol *self;

	g_return_if_fail (LD_IS_LUA_SYMBOL (symbol));
	g_return_if_fail (cr != NULL);

	self = LD_LUA_SYMBOL (symbol);
	ld_lua_private_draw (self->priv->lua, self, cr);
}

