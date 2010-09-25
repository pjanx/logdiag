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
#include "ld-symbol.h"
#include "ld-lua.h"


/**
 * SECTION:ld-lua
 * @short_description: Lua symbol engine.
 * @see_also: #LdSymbol
 *
 * #LdLua is a symbol engine that uses Lua scripts to manage symbols.
 */
/* How does the application call the function for rendering?
 *   logdiag.symbols -- readonly table (from lua) -- this can be probably
 *     accomplished using a custom metatable that errors out on newindex,
 *     items will be added to this table only in C.
 *     It can also be placed into the Lua registry.
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


/*
 * LdLuaData:
 * @self: A reference to self.
 * @category: A reference to parent category of the currently processed file.
 *
 * Full user data to be stored in Lua registry.
 */
typedef struct
{
	LdLua *self;
	LdSymbolCategory *category;
}
LdLuaData;

#define LD_LUA_LIBRARY_NAME "logdiag"
#define LD_LUA_DATA_INDEX LD_LUA_LIBRARY_NAME "_data"

#define LD_LUA_RETRIEVE_DATA(L) \
( \
	lua_pushliteral ((L), LD_LUA_DATA_INDEX), \
	lua_gettable ((L), LUA_REGISTRYINDEX), \
	lua_touserdata ((L), -1) \
)


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
	LdLuaData *ud;

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
	luaL_register (L, LD_LUA_LIBRARY_NAME, ld_lua_logdiag_lib);

	/* Store user data to the registry. */
	lua_pushliteral (L, LD_LUA_DATA_INDEX);

	ud = lua_newuserdata (L, sizeof (LdLuaData));
	ud->self = self;
	ud->category = NULL;

	lua_settable (L, LUA_REGISTRYINDEX);
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
 *
 * Create an instance of #LdLua.
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

/**
 * ld_lua_check_file:
 * @self: An #LdLua object.
 * @filename: The file to be checked.
 *
 * Check if the given filename can be loaded by #LdLua.
 */
gboolean ld_lua_check_file (LdLua *self, const gchar *filename)
{
	g_return_val_if_fail (LD_IS_LUA (self), FALSE);
	return g_str_has_suffix (filename, ".lua");
}

/**
 * ld_lua_load_file_to_category:
 * @self: An #LdLua object.
 * @filename: The file to be loaded.
 * @category: An #LdSymbolCategory object.
 *
 * Loads a file and appends contained symbols into the category.
 *
 * Returns: TRUE if no error has occured, FALSE otherwise.
 */
gboolean ld_lua_load_file_to_category (LdLua *self, const gchar *filename,
	LdSymbolCategory *category)
{
	gint retval;
	LdLuaData *ud;

	g_return_val_if_fail (LD_IS_LUA (self), FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);
	g_return_val_if_fail (LD_IS_SYMBOL_CATEGORY (category), FALSE);

	/* TODO: Error reporting. */

	ud = LD_LUA_RETRIEVE_DATA (self->priv->L);
	g_return_val_if_fail (ud != NULL, FALSE);

	ud->category = category;

	retval = luaL_loadfile (self->priv->L, filename);
	if (retval)
		goto ld_lua_lftc_fail;

	retval = lua_pcall (self->priv->L, 0, 0, 0);
	if (retval)
		goto ld_lua_lftc_fail;

	ud->category = NULL;
	return TRUE;

ld_lua_lftc_fail:
	ud->category = NULL;
	return FALSE;
}

/* ===== Application library =============================================== */

static int
ld_lua_logdiag_register (lua_State *L)
{
	LdLuaData *ud;
	LdSymbol *symbol;

	ud = LD_LUA_RETRIEVE_DATA (L);
	g_return_val_if_fail (ud != NULL, 0);

	/* TODO: Create a symbol. */
	/* XXX: Does ld_lua_symbol_new really need to be passed the category here?
	 *      The symbol can have just a weak reference to the category.
	 */

/*
	symbol = ld_lua_symbol_new (ud->category, ud->self);
	ld_symbol_category_insert (ud->category, symbol, -1);
	g_object_unref (symbol);
*/
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

/* TODO: More functions. Possibly put it into another file
 *       and generate it automatically.
 */
static int
ld_lua_cairo_move_to (lua_State *L)
{
	cairo_t *cr;
	lua_Number x, y;

	cr = lua_touserdata (L, lua_upvalueindex (1));

	x = luaL_checknumber (L, 1);
	y = luaL_checknumber (L, 2);

	cairo_move_to (cr, x, y);
	return 0;
}

static int
ld_lua_cairo_line_to (lua_State *L)
{
	cairo_t *cr;
	lua_Number x, y;

	cr = lua_touserdata (L, lua_upvalueindex (1));
	x = luaL_checknumber (L, 1);
	y = luaL_checknumber (L, 2);
	cairo_line_to (cr, x, y);
	return 0;
}

static int
ld_lua_cairo_stroke (lua_State *L)
{
	cairo_t *cr;

	cr = lua_touserdata (L, lua_upvalueindex (1));
	cairo_stroke (cr);
	return 0;
}

static int
ld_lua_cairo_stroke_preserve (lua_State *L)
{
	cairo_t *cr;

	cr = lua_touserdata (L, lua_upvalueindex (1));
	cairo_stroke_preserve (cr);
	return 0;
}

static int
ld_lua_cairo_fill (lua_State *L)
{
	cairo_t *cr;

	cr = lua_touserdata (L, lua_upvalueindex (1));
	cairo_fill (cr);
	return 0;
}

static int
ld_lua_cairo_fill_preserve (lua_State *L)
{
	cairo_t *cr;

	cr = lua_touserdata (L, lua_upvalueindex (1));
	cairo_fill_preserve (cr);
	return 0;
}



