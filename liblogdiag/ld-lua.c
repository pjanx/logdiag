/*
 * ld-lua.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010, 2011, 2012. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "liblogdiag.h"
#include "config.h"

#include "ld-lua-private.h"
#include "ld-lua-symbol-private.h"


/**
 * SECTION:ld-lua
 * @short_description: Lua symbol engine
 * @see_also: #LdLuaSymbol
 *
 * #LdLua is a symbol engine that uses Lua scripts to manage symbols.
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

/* registry.logdiag_symbols
 *   -> A table indexed by pointers to LdLuaSymbol objects
 * registry.logdiag_symbols.object.render(cr)
 *   -> The rendering function
 */

#define LD_LUA_LIBRARY_NAME   "logdiag"
#define LD_LUA_DATA_INDEX     LD_LUA_LIBRARY_NAME "_data"
#define LD_LUA_SYMBOLS_INDEX  LD_LUA_LIBRARY_NAME "_symbols"
#define LD_LUA_META_INDEX     LD_LUA_LIBRARY_NAME "_meta"

/*
 * LdLuaData:
 * @self: a reference to self.
 * @load_callback: a callback for newly registered symbols.
 * @load_user_data: user data to be passed to the callback.
 *
 * Full user data to be stored in Lua registry.
 */
typedef struct _LdLuaData LdLuaData;

struct _LdLuaData
{
	LdLua *self;
	LdLuaLoadCallback load_callback;
	gpointer load_user_data;
};

typedef struct _LdLuaDrawData LdLuaDrawData;

struct _LdLuaDrawData
{
	LdLuaSymbol *symbol;
	cairo_t *cr;
	unsigned save_count;
};

static void ld_lua_finalize (GObject *gobject);

static void *ld_lua_alloc (void *ud, void *ptr, size_t osize, size_t nsize);

static int ld_lua_private_draw_cb (lua_State *L);
static int ld_lua_private_unregister_cb (lua_State *L);

static int ld_lua_logdiag_register (lua_State *L);
static int process_registration (lua_State *L);
static gchar *get_translation (lua_State *L, int index);
static gboolean read_symbol_area (lua_State *L, int index, LdRectangle *area);
static gboolean read_terminals (lua_State *L, int index,
	LdPointArray **terminals);

static gdouble get_cairo_scale (cairo_t *cr);
static int ld_lua_cairo_save (lua_State *L);
static int ld_lua_cairo_restore (lua_State *L);
static int ld_lua_cairo_get_line_width (lua_State *L);
static int ld_lua_cairo_set_line_width (lua_State *L);
static int ld_lua_cairo_translate (lua_State *L);
static int ld_lua_cairo_scale (lua_State *L);
static int ld_lua_cairo_rotate (lua_State *L);
static int ld_lua_cairo_move_to (lua_State *L);
static int ld_lua_cairo_line_to (lua_State *L);
static int ld_lua_cairo_curve_to (lua_State *L);
static int ld_lua_cairo_arc (lua_State *L);
static int ld_lua_cairo_arc_negative (lua_State *L);
static int ld_lua_cairo_new_path (lua_State *L);
static int ld_lua_cairo_new_sub_path (lua_State *L);
static int ld_lua_cairo_close_path (lua_State *L);
static int ld_lua_cairo_stroke (lua_State *L);
static int ld_lua_cairo_stroke_preserve (lua_State *L);
static int ld_lua_cairo_fill (lua_State *L);
static int ld_lua_cairo_fill_preserve (lua_State *L);
static int ld_lua_cairo_clip (lua_State *L);
static int ld_lua_cairo_clip_preserve (lua_State *L);
static int ld_lua_cairo_show_text (lua_State *L);


static luaL_Reg ld_lua_logdiag_lib[] =
{
	{"register", ld_lua_logdiag_register},
	{NULL, NULL}
};

static luaL_Reg ld_lua_cairo_table[] =
{
	{"save", ld_lua_cairo_save},
	{"restore", ld_lua_cairo_restore},
	{"get_line_width", ld_lua_cairo_get_line_width},
	{"set_line_width", ld_lua_cairo_set_line_width},
	{"translate", ld_lua_cairo_translate},
	{"scale", ld_lua_cairo_scale},
	{"rotate", ld_lua_cairo_rotate},
	{"move_to", ld_lua_cairo_move_to},
	{"line_to", ld_lua_cairo_line_to},
	{"curve_to", ld_lua_cairo_curve_to},
	{"arc", ld_lua_cairo_arc},
	{"arc_negative", ld_lua_cairo_arc_negative},
	{"new_path", ld_lua_cairo_new_path},
	{"new_sub_path", ld_lua_cairo_new_sub_path},
	{"close_path", ld_lua_cairo_close_path},
	{"stroke", ld_lua_cairo_stroke},
	{"stroke_preserve", ld_lua_cairo_stroke_preserve},
	{"fill", ld_lua_cairo_fill},
	{"fill_preserve", ld_lua_cairo_fill_preserve},
	{"clip", ld_lua_cairo_clip},
	{"clip_preserve", ld_lua_cairo_clip_preserve},
	{"show_text", ld_lua_cairo_show_text},
	{NULL, NULL}
};


/* ===== Generic =========================================================== */

G_DEFINE_TYPE (LdLua, ld_lua, G_TYPE_OBJECT);

static void
ld_lua_class_init (LdLuaClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_lua_finalize;

	g_type_class_add_private (klass, sizeof (LdLuaPrivate));
}

static void
push_cairo_metatable (lua_State *L)
{
	luaL_Reg *fn;

	luaL_newmetatable (L, LD_LUA_META_INDEX);

	/* Create a method table. */
	lua_newtable (L);
	for (fn = ld_lua_cairo_table; fn->name; fn++)
	{
		lua_pushcfunction (L, fn->func);
		lua_setfield (L, -2, fn->name);
	}

	lua_setfield (L, -2, "__index");
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

	/* XXX: Might not be a bad idea to use lua_atpanic(). */

	/* Load some safe libraries. */
	luaL_requiref (L, "string", luaopen_string, TRUE);
	luaL_requiref (L, "table",  luaopen_table,  TRUE);
	luaL_requiref (L, "math",   luaopen_math,   TRUE);
	lua_pop (L, 3);

	/* Load the application library. */
	luaL_newlib (L, ld_lua_logdiag_lib);
	lua_setglobal (L, LD_LUA_LIBRARY_NAME);

	/* Store user data to the registry. */
	ud = lua_newuserdata (L, sizeof *ud);
	ud->self = self;
	ud->load_callback = NULL;
	ud->load_user_data = NULL;

	lua_setfield (L, LUA_REGISTRYINDEX, LD_LUA_DATA_INDEX);

	/* Create an empty symbol table. */
	lua_newtable (L);
	lua_setfield (L, LUA_REGISTRYINDEX, LD_LUA_SYMBOLS_INDEX);

	push_cairo_metatable (L);
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
 * @self: an #LdLua object.
 * @filename: the file to be checked.
 *
 * Check if the given filename can be loaded by #LdLua.
 */
gboolean
ld_lua_check_file (LdLua *self, const gchar *filename)
{
	g_return_val_if_fail (LD_IS_LUA (self), FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);

	return g_str_has_suffix (filename, ".lua")
		&& g_file_test (filename, G_FILE_TEST_IS_REGULAR);
}

/**
 * ld_lua_load_file:
 * @self: an #LdLua object.
 * @filename: the file to be loaded.
 * @callback: a callback for newly registered symbols.
 * The callee is responsible for referencing the symbol.
 * @user_data: user data to be passed to the callback.
 *
 * Loads a file and creates #LdLuaSymbol objects for contained symbols.
 *
 * Returns: %TRUE if no error has occured, %FALSE otherwise.
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

	/* XXX: If something from the following fails, Lua will panic. */
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
 * @self: an #LdLua object.
 * @symbol: a symbol to be drawn.
 * @cr: a Cairo context to be drawn onto.
 *
 * Draw a symbol onto a Cairo context.
 */
void
ld_lua_private_draw (LdLua *self, LdLuaSymbol *symbol, cairo_t *cr)
{
	LdLuaDrawData data;

	g_return_if_fail (LD_IS_LUA (self));
	g_return_if_fail (LD_IS_LUA_SYMBOL (symbol));
	g_return_if_fail (cr != NULL);

	data.symbol = symbol;
	data.cr = cr;
	data.save_count = 0;

	lua_pushcfunction (self->priv->L, ld_lua_private_draw_cb);
	lua_pushlightuserdata (self->priv->L, &data);
	if (lua_pcall (self->priv->L, 1, 0, 0))
	{
		g_warning ("Lua error: %s", lua_tostring (self->priv->L, -1));
		lua_pop (self->priv->L, 1);
	}

	while (data.save_count--)
		cairo_restore (cr);
}

static int
ld_lua_private_draw_cb (lua_State *L)
{
	LdLuaDrawData *data, *luadata;

	data = lua_touserdata (L, -1);

	/* Retrieve the function for rendering from the registry. */
	lua_getfield (L, LUA_REGISTRYINDEX, LD_LUA_SYMBOLS_INDEX);
	lua_pushlightuserdata (L, data->symbol);
	lua_gettable (L, -2);

	luaL_checktype (L, -1, LUA_TTABLE);
	lua_getfield (L, -1, "render");
	luaL_checktype (L, -1, LUA_TFUNCTION);

	/* Create a Cairo wrapper object. */
	luadata = lua_newuserdata (L, sizeof *data);
	memcpy (luadata, data, sizeof *data);
	lua_getfield (L, LUA_REGISTRYINDEX, LD_LUA_META_INDEX);
	lua_setmetatable (L, -2);

	/* Force it to stay alive for a bit longer. */
	lua_pushvalue (L, -1);
	lua_insert (L, 1);

	/* Draw the symbol. */
	if (lua_pcall (L, 1, 0, 0))
	{
		g_warning ("Lua error: %s", lua_tostring (L, -1));
		lua_pop (L, 1);
	}

	/* Copy the userdata back and invalidate it, so that malicious Lua
	 * scripts won't succeed at drawing onto a long invalid Cairo context.
	 */
	memcpy (data, luadata, sizeof *data);
	memset (luadata, 0, sizeof *data);
	return 0;
}

/**
 * ld_lua_private_unregister:
 * @self: an #LdLua object.
 * @symbol: a symbol to be unregistered.
 *
 * Unregister a symbol from the internal Lua state.
 */
void
ld_lua_private_unregister (LdLua *self, LdLuaSymbol *symbol)
{
	g_return_if_fail (LD_IS_LUA (self));
	g_return_if_fail (LD_IS_LUA_SYMBOL (symbol));

	lua_pushcfunction (self->priv->L, ld_lua_private_unregister_cb);
	lua_pushlightuserdata (self->priv->L, symbol);
	if (lua_pcall (self->priv->L, 1, 0, 0))
	{
		g_warning ("Lua error: %s", lua_tostring (self->priv->L, -1));
		lua_pop (self->priv->L, 1);
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

static int
ld_lua_logdiag_register (lua_State *L)
{
	LdLuaData *ud;
	LdLuaSymbol *symbol;

	lua_getfield (L, LUA_REGISTRYINDEX, LD_LUA_DATA_INDEX);
	ud = lua_touserdata (L, -1);
	lua_pop (L, 1);
	g_return_val_if_fail (ud != NULL, 0);

	/* Use a protected environment, so script errors won't cause leaking
	 * of the symbol object. Only a failure of the last three function calls
	 * before lua_pcall() may cause the symbol to leak.
	 */
	lua_checkstack (L, 3);
	symbol = g_object_new (LD_TYPE_LUA_SYMBOL, NULL);

	lua_pushlightuserdata (L, symbol);
	lua_pushcclosure (L, process_registration, 1);
	lua_insert (L, 1);

	/* On the stack, there are function arguments plus the function itself. */
	if (lua_pcall (L, lua_gettop (L) - 1, 0, 0))
	{
		luaL_where (L, 1);
		lua_insert (L, -2);
		lua_concat (L, 2);

		g_warning ("Lua symbol registration failed: %s",
			lua_tostring (L, -1));
		lua_pushboolean (L, FALSE);
	}
	else
	{
		/* We don't want an extra LdLua reference either. */
		symbol->priv->lua = ud->self;
		g_object_ref (ud->self);

		ud->load_callback (LD_SYMBOL (symbol), ud->load_user_data);
		lua_pushboolean (L, TRUE);
	}
	g_object_unref (symbol);

	return 1;
}

/*
 * process_registration:
 * @L: a Lua state.
 *
 * Parse arguments, write them to a symbol object and register the object.
 */
static int
process_registration (lua_State *L)
{
	LdLuaSymbol *symbol;
	const gchar *name;
	gchar *human_name;

	int i, type, types[] =
		{LUA_TSTRING, LUA_TTABLE, LUA_TTABLE, LUA_TTABLE, LUA_TFUNCTION};
	int n_args_needed = sizeof (types) / sizeof (int);

	if (lua_gettop (L) < n_args_needed)
		return luaL_error (L, "Too few arguments.");

	for (i = 0; i < n_args_needed; i++)
		if ((type = lua_type (L, i + 1)) != types[i])
			return luaL_error (L, "Bad type of argument #%d."
				" Expected %s, got %s.", i + 1,
				lua_typename (L, types[i]), lua_typename (L, type));

	symbol = LD_LUA_SYMBOL (lua_touserdata (L, lua_upvalueindex (1)));
	name = lua_tostring (L, 1);
	if (g_strstr_len (name, -1, LD_LIBRARY_IDENTIFIER_SEPARATOR))
		return luaL_error (L, "Invalid symbol name.");
	symbol->priv->name = g_strdup (name);

	human_name = get_translation (L, 2);
	if (!human_name)
		human_name = g_strdup (symbol->priv->name);
	symbol->priv->human_name = human_name;

	if (!read_symbol_area (L, 3, &symbol->priv->area))
		return luaL_error (L, "Malformed symbol area array.");
	if (!read_terminals (L, 4, &symbol->priv->terminals))
		return luaL_error (L, "Malformed terminals array.");

	lua_getfield (L, LUA_REGISTRYINDEX, LD_LUA_SYMBOLS_INDEX);
	lua_pushlightuserdata (L, symbol);

	lua_newtable (L);
	lua_pushvalue (L, 5);
	lua_setfield (L, -2, "render");

	lua_settable (L, -3);
	return 0;
}

/*
 * get_translation:
 * @L: a Lua state.
 * @index: stack index of the table.
 *
 * Select an applicable translation from a table.
 * The return value has to be freed with g_free().
 *
 * Return value: the translation, if found. If none was found, returns %NULL.
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
 * @L: a Lua state.
 * @index: stack index of the table.
 * @area: where the area will be returned.
 *
 * Read a symbol area from a Lua table.
 *
 * Return value: %TRUE on success, %FALSE on failure.
 */
static gboolean
read_symbol_area (lua_State *L, int index, LdRectangle *area)
{
	lua_Number x1, x2, y1, y2;

	if (lua_rawlen (L, index) != 4)
		return FALSE;

	lua_rawgeti (L, index, 1);
	if (!lua_isnumber (L, -1))
		return FALSE;
	x1 = lua_tonumber (L, -1);

	lua_rawgeti (L, index, 2);
	if (!lua_isnumber (L, -1))
		return FALSE;
	y1 = lua_tonumber (L, -1);

	lua_rawgeti (L, index, 3);
	if (!lua_isnumber (L, -1))
		return FALSE;
	x2 = lua_tonumber (L, -1);

	lua_rawgeti (L, index, 4);
	if (!lua_isnumber (L, -1))
		return FALSE;
	y2 = lua_tonumber (L, -1);

	area->x = MIN (x1, x2);
	area->y = MIN (y1, y2);
	area->width  = ABS (x2 - x1);
	area->height = ABS (y2 - y1);

	lua_pop (L, 4);
	return TRUE;
}

/*
 * read_terminals:
 * @L: a Lua state.
 * @index: stack index of the table.
 * @area: where the point array will be returned.
 *
 * Read symbol terminals from a Lua table.
 *
 * Return value: %TRUE on success, %FALSE on failure.
 */
static gboolean
read_terminals (lua_State *L, int index, LdPointArray **terminals)
{
	LdPointArray *points;
	size_t num_points;

	num_points = lua_rawlen (L, index);
	points = ld_point_array_sized_new (num_points);

	lua_pushnil (L);
	while (lua_next (L, index) != 0)
	{
		g_assert (points->length < points->size);

		if (!lua_istable (L, -1) || lua_rawlen (L, -1) != 2)
			goto read_terminals_fail;

		lua_rawgeti (L, -1, 1);
		if (!lua_isnumber (L, -1))
			goto read_terminals_fail;
		points->points[points->length].x = lua_tonumber (L, -1);
		lua_pop (L, 1);

		lua_rawgeti (L, -1, 2);
		if (!lua_isnumber (L, -1))
			goto read_terminals_fail;
		points->points[points->length].y = lua_tonumber (L, -1);

		lua_pop (L, 2);
		points->length++;
	}
	*terminals = points;
	return TRUE;

read_terminals_fail:
	ld_point_array_free (points);
	*terminals = NULL;
	return FALSE;
}


/* ===== Cairo ============================================================= */

static gdouble
get_cairo_scale (cairo_t *cr)
{
	double dx = 1, dy = 0;

	cairo_user_to_device_distance (cr, &dx, &dy);
	return dx;
}

#define LD_LUA_CAIRO_GET_DATA \
	data = luaL_checkudata (L, 1, LD_LUA_META_INDEX); \
	if (!data->cr) \
		return luaL_error (L, "Tried to use an invalid Cairo object");

#define LD_LUA_CAIRO_BEGIN(name) \
static int \
ld_lua_cairo_ ## name (lua_State *L) \
{ \
	LdLuaDrawData *data; \

#define LD_LUA_CAIRO_END(n_values) \
	return (n_values); \
}

#define LD_LUA_CAIRO_TRIVIAL(name) \
LD_LUA_CAIRO_BEGIN (name) \
	LD_LUA_CAIRO_GET_DATA \
	cairo_ ## name (data->cr); \
LD_LUA_CAIRO_END (0)

LD_LUA_CAIRO_TRIVIAL (new_path)
LD_LUA_CAIRO_TRIVIAL (new_sub_path)
LD_LUA_CAIRO_TRIVIAL (close_path)

LD_LUA_CAIRO_TRIVIAL (stroke)
LD_LUA_CAIRO_TRIVIAL (stroke_preserve)
LD_LUA_CAIRO_TRIVIAL (fill)
LD_LUA_CAIRO_TRIVIAL (fill_preserve)
LD_LUA_CAIRO_TRIVIAL (clip)
LD_LUA_CAIRO_TRIVIAL (clip_preserve)

LD_LUA_CAIRO_BEGIN (save)
	LD_LUA_CAIRO_GET_DATA
	if (data->save_count + 1)
	{
		data->save_count++;
		cairo_save (data->cr);
	}
LD_LUA_CAIRO_END (0)

LD_LUA_CAIRO_BEGIN (restore)
	LD_LUA_CAIRO_GET_DATA
	if (data->save_count)
	{
		data->save_count--;
		cairo_restore (data->cr);
	}
LD_LUA_CAIRO_END (0)

LD_LUA_CAIRO_BEGIN (get_line_width)
	LD_LUA_CAIRO_GET_DATA
	lua_pushnumber (L, cairo_get_line_width (data->cr)
		* get_cairo_scale (data->cr));
LD_LUA_CAIRO_END (1)

LD_LUA_CAIRO_BEGIN (set_line_width)
	LD_LUA_CAIRO_GET_DATA
	cairo_set_line_width (data->cr, luaL_checknumber (L, 2)
		/ get_cairo_scale (data->cr));
LD_LUA_CAIRO_END (0)

LD_LUA_CAIRO_BEGIN (translate)
	lua_Number x, y;

	LD_LUA_CAIRO_GET_DATA
	x = luaL_checknumber (L, 2);
	y = luaL_checknumber (L, 3);

	cairo_translate (data->cr, x, y);
LD_LUA_CAIRO_END (0)

LD_LUA_CAIRO_BEGIN (scale)
	lua_Number sx, sy;

	LD_LUA_CAIRO_GET_DATA
	sx = luaL_checknumber (L, 2);
	sy = luaL_checknumber (L, 3);

	cairo_scale (data->cr, sx, sy);
LD_LUA_CAIRO_END (0)

LD_LUA_CAIRO_BEGIN (rotate)
	lua_Number angle;

	LD_LUA_CAIRO_GET_DATA
	angle = luaL_checknumber (L, 2);
	cairo_rotate (data->cr, angle);
LD_LUA_CAIRO_END (0)

LD_LUA_CAIRO_BEGIN (move_to)
	lua_Number x, y;

	LD_LUA_CAIRO_GET_DATA
	x = luaL_checknumber (L, 2);
	y = luaL_checknumber (L, 3);

	cairo_move_to (data->cr, x, y);
LD_LUA_CAIRO_END (0)

LD_LUA_CAIRO_BEGIN (line_to)
	lua_Number x, y;

	LD_LUA_CAIRO_GET_DATA
	x = luaL_checknumber (L, 2);
	y = luaL_checknumber (L, 3);

	cairo_line_to (data->cr, x, y);
LD_LUA_CAIRO_END (0)

LD_LUA_CAIRO_BEGIN (curve_to)
	lua_Number x1, y1, x2, y2, x3, y3;

	LD_LUA_CAIRO_GET_DATA
	x1 = luaL_checknumber (L, 2);
	y1 = luaL_checknumber (L, 3);
	x2 = luaL_checknumber (L, 4);
	y2 = luaL_checknumber (L, 5);
	x3 = luaL_checknumber (L, 6);
	y3 = luaL_checknumber (L, 7);

	cairo_curve_to (data->cr, x1, y1, x2, y2, x3, y3);
LD_LUA_CAIRO_END (0)

LD_LUA_CAIRO_BEGIN (arc)
	lua_Number xc, yc, radius, angle1, angle2;

	LD_LUA_CAIRO_GET_DATA
	xc = luaL_checknumber (L, 2);
	yc = luaL_checknumber (L, 3);
	radius = luaL_checknumber (L, 4);
	angle1 = luaL_checknumber (L, 5);
	angle2 = luaL_checknumber (L, 6);

	cairo_arc (data->cr, xc, yc, radius, angle1, angle2);
LD_LUA_CAIRO_END (0)

LD_LUA_CAIRO_BEGIN (arc_negative)
	lua_Number xc, yc, radius, angle1, angle2;

	LD_LUA_CAIRO_GET_DATA
	xc = luaL_checknumber (L, 2);
	yc = luaL_checknumber (L, 3);
	radius = luaL_checknumber (L, 4);
	angle1 = luaL_checknumber (L, 5);
	angle2 = luaL_checknumber (L, 6);

	cairo_arc_negative (data->cr, xc, yc, radius, angle1, angle2);
LD_LUA_CAIRO_END (0)

LD_LUA_CAIRO_BEGIN (show_text)
	const char *text;
	GtkStyle *style;
	PangoLayout *layout;
	int width, height;
	double x, y;

	LD_LUA_CAIRO_GET_DATA
	text = luaL_checkstring (L, 2);

	layout = pango_cairo_create_layout (data->cr);
	pango_layout_set_text (layout, text, -1);

	style = gtk_style_new ();
	pango_font_description_set_size (style->font_desc, 1 * PANGO_SCALE);
	pango_layout_set_font_description (layout, style->font_desc);
	g_object_unref (style);

	pango_layout_get_size (layout, &width, &height);
	cairo_get_current_point (data->cr, &x, &y);
	x -= (double) width  / PANGO_SCALE / 2;
	y -= (double) height / PANGO_SCALE / 2;

	cairo_save (data->cr);
	cairo_move_to (data->cr, x, y);
	pango_cairo_show_layout (data->cr, layout);
	cairo_restore (data->cr);

	g_object_unref (layout);
LD_LUA_CAIRO_END (0)

