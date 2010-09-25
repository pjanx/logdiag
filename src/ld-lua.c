/*
 * ld-lua.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "config.h"

#include "ld-symbol-library.h"
#include "ld-lua.h"


/* TODO */
/* A virtual superclass can be made for this. */

/* Lua state belongs to the library.
 * One Lua file should be able to register multiple symbols.
 */
/* How does the application call the function for rendering?
 *   logdiag.symbols -- readonly table (from lua) -- this can be probably
 *     accomplished using a custom metatable that errors out on newindex,
 *     items will be added to this table only in C.
 *   logdiag.symbols[ident].render(cr) -- here "ident" is the full path
 *     to this symbol
 *   logdiag.symbols[ident].names[lang, area, terminals] -- these
 *     subarrays need not be in this array
 *
 */

static void *
ld_lua_alloc (void *ud, void *ptr, size_t osize, size_t nsize);


static int
ld_lua_logdiag_register (lua_State *L);

static luaL_Reg ld_lua_logdiag_lib[] =
{
	{"register", ld_lua_logdiag_register},
	{NULL, NULL}
};


static int
ld_lua_cairo_move_to (lua_State *L);
static int
ld_lua_cairo_line_to (lua_State *L);
static int
ld_lua_cairo_stroke (lua_State *L);
static int
ld_lua_cairo_stroke_preserve (lua_State *L);
static int
ld_lua_cairo_fill (lua_State *L);
static int
ld_lua_cairo_fill_preserve (lua_State *L);

static luaL_Reg ld_lua_cairo_table[] =
{
	{"move_to", ld_lua_cairo_move_to},
	{"line_to", ld_lua_cairo_line_to},
	{"stroke", ld_lua_cairo_stroke},
	{"stroke_preserve", ld_lua_cairo_stroke_preserve},
	{"fill", ld_lua_cairo_fill},
	{"fill_preserve", ld_lua_cairo_fill_preserve},
	{NULL, NULL}
};

/* ===== Generic =========================================================== */

lua_State *
ld_lua_init (void)
{
	lua_State *L;

	L = lua_newstate (ld_lua_alloc, NULL);
	g_return_val_if_fail (L != NULL, NULL);

	/* TODO: lua_atpanic () */

	/* Load some safe libraries. */
	lua_pushcfunction (L, luaopen_string);
	lua_call (L, 0, 0);

	lua_pushcfunction (L, luaopen_table);
	lua_call (L, 0, 0);

	lua_pushcfunction (L, luaopen_math);
	lua_call (L, 0, 0);

	/* Load the application library. */
	luaL_register (L, "logdiag", ld_lua_logdiag_lib);
}

void
ld_lua_destroy (lua_State *L)
{
	lua_close (L);
}

static void *
ld_lua_alloc (void *ud, void *ptr, size_t osize, size_t nsize)
{
	if (!nsize)
	{
		g_free (ptr);
		return NULL;
	}
	else
		return g_try_realloc (ptr, nsize);
}

/* ===== Application library =============================================== */

static int
ld_lua_logdiag_register (lua_State *L)
{
	/* TODO: Create a symbol. */
	/* XXX: Shouldn't this function be a closure with LdLibrary userdata?
	 *      It is also possible to have the userdata in the "logdiag" table.
	 */

#if 0
	lua_newtable (L);

	/* TODO: Push a function. */
	lua_call (L, 1, 0);
#endif /* 0 */
}

/* ===== Cairo ============================================================= */

static void
push_cairo_object (lua_State *L, cairo_t *cr)
{
	luaL_Reg *fn;

	/* Create a table. */
	lua_newtable (L);

	/* Add methods. */
	for (fn = ld_lua_cairo_table; fn->name; fn++)
	{
		lua_pushlightuserdata (L, cr);
		lua_pushcclosure (L, fn->func, 1);
		lua_setfield (L, -2, fn->name);
	}
}

static int
ld_lua_cairo_move_to (lua_State *L)
{
	return 0;
}

static int
ld_lua_cairo_line_to (lua_State *L)
{
	return 0;
}

static int
ld_lua_cairo_stroke (lua_State *L)
{
	return 0;
}

static int
ld_lua_cairo_stroke_preserve (lua_State *L)
{
	return 0;
}

static int
ld_lua_cairo_fill (lua_State *L)
{
	return 0;
}

static int
ld_lua_cairo_fill_preserve (lua_State *L)
{
	return 0;
}



