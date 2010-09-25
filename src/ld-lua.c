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

#include "ld-library.h"
#include "ld-symbol-category.h"
#include "ld-lua.h"


/**
 * SECTION:ld-lua
 * @short_description: Lua symbol engine.
 * @see_also: #LdSymbol
 *
 * #LdLua is a symbol engine that uses Lua scripts to manage symbols.
 */
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

/*
 * LdLuaPrivate:
 * @L: Lua state.
 *
 * The library contains the real function for rendering.
 */
struct _LdLuaPrivate
{
	lua_State *L;
};

G_DEFINE_TYPE (LdLua, ld_lua, G_TYPE_OBJECT);

static void ld_lua_finalize (GObject *gobject);

static void *ld_lua_alloc (void *ud, void *ptr, size_t osize, size_t nsize);


static int ld_lua_logdiag_register (lua_State *L);

static luaL_Reg ld_lua_logdiag_lib[] =
{
	{"register", ld_lua_logdiag_register},
	{NULL, NULL}
};


static int ld_lua_cairo_move_to (lua_State *L);
static int ld_lua_cairo_line_to (lua_State *L);
static int ld_lua_cairo_stroke (lua_State *L);
static int ld_lua_cairo_stroke_preserve (lua_State *L);
static int ld_lua_cairo_fill (lua_State *L);
static int ld_lua_cairo_fill_preserve (lua_State *L);

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

static void
ld_lua_class_init (LdLuaClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_lua_finalize;

	g_type_class_add_private (klass, sizeof (LdLuaPrivate));
}

static void
ld_lua_init (LdLua *self)
{
	lua_State *L;

	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_LUA, LdLuaPrivate);

	L = self->priv->L = lua_newstate (ld_lua_alloc, NULL);
	g_return_if_fail (L != NULL);

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

static void
ld_lua_finalize (GObject *gobject)
{
	LdLua *self;

	self = LD_LUA (gobject);
	lua_close (self->priv->L);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_lua_parent_class)->finalize (gobject);
}

/**
 * ld_lua_new:
 * @library: A library object.
 * @filename: The file from which the symbol will be loaded.
 *
 * Load a symbol from a file into the library.
 */
LdLua *
ld_lua_new (void)
{
	return g_object_new (LD_TYPE_LUA, NULL);
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
	return 0;
}

/* ===== Cairo ============================================================= */

static void
push_cairo_object (lua_State *L, cairo_t *cr)
{
	luaL_Reg *fn;

	/* Create a table. */
	lua_newtable (L);

	/* Add methods. */
	/* XXX: The light user data pointer gets invalid after the end of
	 *      "render" function invocation. If the script stores the "cr" object
	 *      in some global variable and then tries to reuse it the next time,
	 *      the application will go SIGSEGV.
	 */
	for (fn = ld_lua_cairo_table; fn->name; fn++)
	{
		lua_pushlightuserdata (L, cr);
		lua_pushcclosure (L, fn->func, 1);
		lua_setfield (L, -2, fn->name);
	}
}

/* TODO: Implement the functions. */
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



