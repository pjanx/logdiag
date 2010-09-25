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


#define LD_TYPE_LUA (ld_lua_get_type ())
#define LD_LUA(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_LUA, LdLua))
#define LD_LUA_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_LUA, LdLuaClass))
#define LD_IS_LUA(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_LUA))
#define LD_IS_LUA_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_LUA))
#define LD_LUA_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_LUA, LdLuaClass))

typedef struct _LdLua LdLua;
typedef struct _LdLuaPrivate LdLuaPrivate;
typedef struct _LdLuaClass LdLuaClass;


struct _LdLua
{
/*< private >*/
	GObject parent_instance;
	LdLuaPrivate *priv;

/*< public >*/
	gchar *name;
};

/* TODO: A virtual superclass, so other engines can be used. */
struct _LdLuaClass
{
	GObjectClass parent_class;
};


GType ld_lua_get_type (void) G_GNUC_CONST;

LdLua *ld_lua_new (void);
/* TODO: Implement the following: */
gboolean ld_lua_check_file (LdLua *lua, const gchar *filename);
gboolean ld_lua_load_file_to_category (LdLua *lua, const gchar *filename,
	LdSymbolCategory *category);


G_END_DECLS

#endif /* ! __LD_LUA_H__ */

