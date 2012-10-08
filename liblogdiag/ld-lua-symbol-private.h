/*
 * ld-lua-symbol-private.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010, 2011. All rights reserved.
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
 * @lua: parent #LdLua object.
 * @name: name of this symbol.
 * @human_name: localized human name of this symbol.
 * @area: area of this symbol.
 * @terminals: terminals of this symbol.
 */
struct _LdLuaSymbolPrivate
{
	LdLua *lua;
	gchar *name;
	gchar *human_name;
	LdRectangle area;
	LdPointArray *terminals;
};


G_END_DECLS

#endif /* ! __LD_LUA_SYMBOL_PRIVATE_H__ */

