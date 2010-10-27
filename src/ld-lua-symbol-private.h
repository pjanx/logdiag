/*
 * ld-lua-symbol-private.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_LUA_SYMBOL_PRIVATE_H__
#define __LD_LUA_SYMBOL_PRIVATE_H__

G_BEGIN_DECLS


/*< private_header >*/

/*
 * LdLuaSymbolPrivate:
 * @lua: Parent #LdLua object.
 * @name: Name of this symbol.
 * @human_name: Localized human name of this symbol.
 * @area: Area of this symbol.
 */
struct _LdLuaSymbolPrivate
{
	LdLua *lua;
	gchar *name;
	gchar *human_name;
	LdSymbolArea area;
};


G_END_DECLS

#endif /* ! __LD_LUA_SYMBOL_PRIVATE_H__ */

