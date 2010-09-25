/*
 * ld-lua.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_LUA_H__
#define __LD_LUA_H__

G_BEGIN_DECLS


lua_State *
ld_lua_init (void);

void
ld_lua_destroy (lua_State *L);


G_END_DECLS

#endif /* ! __LD_LUA_H__ */

