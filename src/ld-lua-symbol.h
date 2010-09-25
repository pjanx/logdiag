/*
 * ld-lua-symbol.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_LUA_SYMBOL_H__
#define __LD_LUA_SYMBOL_H__

G_BEGIN_DECLS


#define LD_TYPE_LUA_SYMBOL (ld_lua_symbol_get_type ())
#define LD_LUA_SYMBOL(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_LUA_SYMBOL, LdLuaSymbol))
#define LD_LUA_SYMBOL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_LUA_SYMBOL, LdLuaSymbolClass))
#define LD_IS_LUA_SYMBOL(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_LUA_SYMBOL))
#define LD_IS_LUA_SYMBOL_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_LUA_SYMBOL))
#define LD_LUA_SYMBOL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_LUA_SYMBOL, LdLuaSymbolClass))

typedef struct _LdLuaSymbol LdLuaSymbol;
typedef struct _LdLuaSymbolPrivate LdLuaSymbolPrivate;
typedef struct _LdLuaSymbolClass LdLuaSymbolClass;


/**
 * LdLuaSymbol:
 * @name: The name of this symbol.
 */
struct _LdLuaSymbol
{
/*< private >*/
	LdSymbol parent_instance;
	LdLuaSymbolPrivate *priv;
};

/**
 * LdLuaSymbolClass:
 * @parent_class: The parent class.
 */
struct _LdLuaSymbolClass
{
	LdSymbolClass parent_class;
};


GType ld_lua_symbol_get_type (void) G_GNUC_CONST;

LdSymbol *ld_lua_symbol_new (LdSymbolCategory *parent, LdLua *lua);


G_END_DECLS

#endif /* ! __LD_LUA_SYMBOL_H__ */

