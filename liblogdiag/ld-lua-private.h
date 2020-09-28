/*
 * ld-lua-private.h
 *
 * This file is a part of logdiag.
 * Copyright 2010 Přemysl Eric Janouch
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_LUA_PRIVATE_H__
#define __LD_LUA_PRIVATE_H__

G_BEGIN_DECLS


/*< private_header >*/

void ld_lua_private_unregister (LdLua *self, LdLuaSymbol *symbol);
void ld_lua_private_draw (LdLua *self, LdLuaSymbol *symbol, cairo_t *cr);


G_END_DECLS

#endif /* ! __LD_LUA_PRIVATE_H__ */

