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
 * @lua: Parent Lua object.
 */
struct _LdLuaSymbolPrivate
{
	LdLua *lua;
};

G_DEFINE_TYPE (LdLuaSymbol, ld_lua_symbol, LD_TYPE_SYMBOL);

static void ld_lua_symbol_finalize (GObject *gobject);


static void
ld_lua_symbol_class_init (LdLuaSymbolClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_lua_symbol_finalize;

	/* TODO: Override the "draw" method of LdSymbol. */

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

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_lua_symbol_parent_class)->finalize (gobject);
}


/**
 * ld_symbol_new:
 * @library: A library object.
 * @filename: The file from which the symbol will be loaded.
 *
 * Load a symbol from a file into the library.
 */
LdSymbol *
ld_lua_symbol_new (LdSymbolCategory *parent, LdLua *lua)
{
	LdLuaSymbol *symbol;

	symbol = g_object_new (LD_TYPE_LUA_SYMBOL, NULL);

	/* TODO: Create a separate ld-symbol-private.h, include it in this file
	 *       and then assign and ref the parent category here.
	 */

	symbol->priv->lua = lua;
	g_object_ref (lua);
}

