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

#include "ld-symbol.h"
#include "ld-library.h"

#include "ld-lua.h"
#include "ld-lua-symbol.h"

#include "ld-lua-private.h"
#include "ld-lua-symbol-private.h"


/**
 * SECTION:ld-lua
 * @short_description: Lua symbol engine.
 * @see_also: #LdLuaSymbol
 *
 * #LdLua is a symbol engine that uses Lua scripts to manage symbols.
 */

/* How does the application call the function for rendering?
 *   registry.logdiag_symbols
 *     -> table indexed by pointers to LdLuaSymbol objects
 *   registry.logdiag_symbols.object.render(cr)
 *     -> rendering function
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
 * @load_callback: A callback for newly registered symbols.
 * @load_user_data: User data to be passed to the callback.
 *
 * Full user data to be stored in Lua registry.
 */
typedef struct
{
	LdLua *self;
	LdLuaLoadCallback load_callback;
	gpointer load_user_data;
}
LdLuaData;

#define LD_LUA_LIBRARY_NAME "logdiag"
#define LD_LUA_DATA_INDEX LD_LUA_LIBRARY_NAME "_data"
#define LD_LUA_SYMBOLS_INDEX LD_LUA_LIBRARY_NAME "_symbols"


typedef struct
{
	LdLuaSymbol *symbol;
	cairo_t *cr;
}
LdLuaDrawData;

static int ld_lua_private_draw_cb (lua_State *L);
static int ld_lua_private_unregister_cb (lua_State *L);


static int ld_lua_logdiag_register (lua_State *L);
static gchar *get_translation (lua_State *L, int index);
static void read_symbol_area (lua_State *L, int index, LdSymbolArea *area);

static luaL_Reg ld_lua_logdiag_lib[] =
{
	{"register", ld_lua_logdiag_register},
	{NULL, NULL}
};


static void push_cairo_object (lua_State *L, cairo_t *cr);

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
	ud = lua_newuserdata (L, sizeof (LdLuaData));
	ud->self = self;
	ud->load_callback = NULL;
	ud->load_user_data = NULL;

	lua_setfield (L, LUA_REGISTRYINDEX, LD_LUA_DATA_INDEX);

	/* Create an empty symbols table. */
	lua_newtable (L);
	lua_setfield (L, LUA_REGISTRYINDEX, LD_LUA_SYMBOLS_INDEX);
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
gboolean
ld_lua_check_file (LdLua *self, const gchar *filename)
{
	g_return_val_if_fail (LD_IS_LUA (self), FALSE);
	return g_str_has_suffix (filename, ".lua")
		&& g_file_test (filename, G_FILE_TEST_IS_REGULAR);
}

/**
 * ld_lua_load_file:
 * @self: An #LdLua object.
 * @filename: The file to be loaded.
 * @callback: A callback for newly registered symbols.
 * @user_data: User data to be passed to the callback.
 *
 * Loads a file and creates #LdLuaSymbol objects for contained symbols.
 *
 * Returns: TRUE if no error has occured, FALSE otherwise.
 */
gboolean
ld_lua_load_file (LdLua *self, const gchar *filename,
	LdLuaLoadCallback callback, gpointer user_data)
{
	gint retval;
	LdLuaData *ud;

	g_return_val_if_fail (LD_IS_LUA (self), FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);
	g_return_val_if_fail (callback != NULL, FALSE);

	/* XXX: If something from the following fails, Lua will call exit(). */
	lua_getfield (self->priv->L, LUA_REGISTRYINDEX, LD_LUA_DATA_INDEX);
	ud = lua_touserdata (self->priv->L, -1);
	lua_pop (self->priv->L, 1);
	g_return_val_if_fail (ud != NULL, FALSE);

	ud->load_callback = callback;
	ud->load_user_data = user_data;

	retval = luaL_loadfile (self->priv->L, filename);
	if (retval)
		goto ld_lua_lftc_fail;

	retval = lua_pcall (self->priv->L, 0, 0, 0);
	if (retval)
		goto ld_lua_lftc_fail;

	ud->load_callback = NULL;
	ud->load_user_data = NULL;
	return TRUE;

ld_lua_lftc_fail:
	g_warning ("Lua error: %s", lua_tostring (self->priv->L, -1));
	lua_remove (self->priv->L, -1);

	ud->load_callback = NULL;
	ud->load_user_data = NULL;
	return FALSE;
}

/* ===== LdLuaSymbol callbacks ============================================= */

/**
 * ld_lua_private_draw:
 * @self: An #LdLua object.
 * @symbol: A symbol to be drawn.
 * @cr: A Cairo context to be drawn onto.
 *
 * Draw a symbol onto a Cairo context.
 */
void
ld_lua_private_draw (LdLua *self, LdLuaSymbol *symbol, cairo_t *cr)
{
	LdLuaDrawData data;

	g_return_if_fail (LD_IS_LUA (self));
	g_return_if_fail (LD_IS_LUA_SYMBOL (symbol));

	data.symbol = symbol;
	data.cr = cr;

	if (lua_cpcall (self->priv->L, ld_lua_private_draw_cb, &data))
	{
		g_warning ("Lua error: %s", lua_tostring (self->priv->L, -1));
		lua_remove (self->priv->L, -1);
	}
}

static int
ld_lua_private_draw_cb (lua_State *L)
{
	LdLuaDrawData *data;

	data = lua_touserdata (L, -1);

	/* Retrieve the function for rendering from the registry. */
	lua_getfield (L, LUA_REGISTRYINDEX, LD_LUA_SYMBOLS_INDEX);
	lua_pushlightuserdata (L, data->symbol);
	lua_gettable (L, -2);

	luaL_checktype (L, -1, LUA_TTABLE);
	lua_getfield (L, -1, "render");
	luaL_checktype (L, -1, LUA_TFUNCTION);

	/* Call the function do draw the symbol. */
	push_cairo_object (L, data->cr);
	lua_pcall (L, 1, 0, 0);
}

/**
 * ld_lua_private_unregister:
 * @self: An #LdLua object.
 * @symbol: A symbol to be unregistered.
 *
 * Unregister a symbol from the internal Lua state.
 */
void
ld_lua_private_unregister (LdLua *self, LdLuaSymbol *symbol)
{
	g_return_if_fail (LD_IS_LUA (self));
	g_return_if_fail (LD_IS_LUA_SYMBOL (symbol));

	if (lua_cpcall (self->priv->L, ld_lua_private_unregister_cb, symbol))
	{
		g_warning ("Lua error: %s", lua_tostring (self->priv->L, -1));
		lua_remove (self->priv->L, -1);
	}
}

static int
ld_lua_private_unregister_cb (lua_State *L)
{
	/* Set the entry in the symbol table to nil. */
	lua_getfield (L, LUA_REGISTRYINDEX, LD_LUA_SYMBOLS_INDEX);
	lua_insert (L, -2);
	lua_pushnil (L);
	lua_settable (L, -3);
	return 0;
}

/* ===== Application library =============================================== */

/* XXX: This function is damn too long. */
static int
ld_lua_logdiag_register (lua_State *L)
{
	LdLuaData *ud;
	LdLuaSymbol *symbol;
	const gchar *name;
	gchar *human_name;

	lua_getfield (L, LUA_REGISTRYINDEX, LD_LUA_DATA_INDEX);
	ud = lua_touserdata (L, -1);
	lua_pop (L, 1);
	g_return_val_if_fail (ud != NULL, 0);

	/* Check and retrieve arguments. */
	name = lua_tostring (L, 1);
	if (!name)
		luaL_error (L, "register: bad or missing argument #%d", 1);
	if (!lua_istable (L, 2))
		luaL_error (L, "register: bad or missing argument #%d", 2);
	if (!lua_istable (L, 3))
		luaL_error (L, "register: bad or missing argument #%d", 3);
	if (!lua_istable (L, 4))
		luaL_error (L, "register: bad or missing argument #%d", 4);
	if (!lua_isfunction (L, 5))
		luaL_error (L, "register: bad or missing argument #%d", 5);

	/* Create a symbol using the given parameters. */
	/* XXX: If an error occurs, this object will not be freed. */
	symbol = g_object_new (LD_TYPE_LUA_SYMBOL, NULL);
	symbol->priv->lua = ud->self;
	g_object_ref (ud->self);

	symbol->priv->name = g_strdup (name);

	human_name = get_translation (L, 2);
	if (!human_name)
		human_name = g_strdup (name);
	symbol->priv->human_name = human_name;

	/* TODO: Check the values. */
	read_symbol_area (L, 3, &symbol->priv->area);

	/* TODO: Read and set the terminals (they're in a table). */
	lua_pushvalue (L, 4);

	/* Create an entry in the symbol table. */
	lua_getfield (L, LUA_REGISTRYINDEX, LD_LUA_SYMBOLS_INDEX);
	lua_pushlightuserdata (L, symbol);

	lua_newtable (L);
	lua_pushvalue (L, 5);
	lua_setfield (L, -2, "render");

	lua_settable (L, -3);

	/* The caller is responsible for referencing the symbol. */
	ud->load_callback (LD_SYMBOL (symbol), ud->load_user_data);
	g_object_unref (symbol);

	return 0;
}

/*
 * get_translation:
 * @L: A Lua state.
 * @index: Stack index of the table.
 *
 * Select an applicable translation from a table.
 * The return value has to be freed with g_free().
 *
 * Return value: The translation, if found. If none was found, returns NULL.
 */
static gchar *
get_translation (lua_State *L, int index)
{
	const gchar *const *lang;
	gchar *result;

	for (lang = g_get_language_names (); *lang; lang++)
	{
		lua_getfield (L, 2, *lang);
		if (lua_isstring (L, -1))
		{
			result = g_strdup (lua_tostring (L, -1));
			lua_pop (L, 1);
			return result;
		}
		lua_pop (L, 1);
	}
	return NULL;
}

/*
 * read_symbol_area:
 * @L: A Lua state.
 * @index: Stack index of the table.
 * @area: Where the area will be returned.
 *
 * Read a symbol area from a Lua table.
 */
static void
read_symbol_area (lua_State *L, int index, LdSymbolArea *area)
{
	if (lua_objlen (L, index) != 4)
		return;

	lua_rawgeti (L, index, 1);
	area->x1 = lua_tonumber (L, -1);

	lua_rawgeti (L, index, 2);
	area->y1 = lua_tonumber (L, -1);

	lua_rawgeti (L, index, 3);
	area->x2 = lua_tonumber (L, -1);

	lua_rawgeti (L, index, 4);
	area->y2 = lua_tonumber (L, -1);
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



