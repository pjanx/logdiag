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

#include "ld-library.h"
#include "ld-symbol.h"
#include "ld-symbol-category.h"
#include "ld-lua.h"
#include "ld-lua-symbol.h"


/**
 * SECTION:ld-lua-symbol
 * @short_description: A symbol.
 * @see_also: #LdSymbol
 *
 * #LdLuaSymbol is an implementation of #LdSymbol.
 */

/*
 * LdLuaSymbolPrivate:
 * @lua: Parent #LdLua object.
 * @ident: Identifier for the symbol.
 */
struct _LdLuaSymbolPrivate
{
	LdLua *lua;
	gchar *ident;
};

G_DEFINE_TYPE (LdLuaSymbol, ld_lua_symbol, LD_TYPE_SYMBOL);

static void ld_lua_symbol_finalize (GObject *gobject);

static void ld_lua_symbol_draw (LdSymbol *self, cairo_t *cr);


static void
ld_lua_symbol_class_init (LdLuaSymbolClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_lua_symbol_finalize;

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
	g_object_unref (self->priv->lua);

	g_free (self->priv->ident);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_lua_symbol_parent_class)->finalize (gobject);
}


/**
 * ld_symbol_new:
 * @lua: An #LdLua object.
 * @ident: Identifier for the symbol.
 *
 * Load a symbol from a file into the library.
 */
LdSymbol *
ld_lua_symbol_new (LdLua *lua, const gchar *ident)
{
	LdLuaSymbol *self;

	g_return_val_if_fail (LD_IS_LUA (lua), NULL);
	g_return_val_if_fail (ident != NULL, NULL);

	self = g_object_new (LD_TYPE_LUA_SYMBOL, NULL);

	self->priv->lua = lua;
	g_object_ref (lua);

	self->priv->ident = g_strdup (ident);
	return LD_SYMBOL (self);
}

static void
ld_lua_symbol_draw (LdSymbol *self, cairo_t *cr)
{
	g_return_if_fail (LD_IS_SYMBOL (self));
	g_return_if_fail (cr != NULL);

	/* TODO: Implement. */
	/* Retrieve the function for rendering from the registry or wherever
	 * it's going to end up, and call it.
	 */
}

