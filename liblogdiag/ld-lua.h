/*
 * ld-lua.h
 *
 * This file is a part of logdiag.
 * Copyright 2010 Přemysl Eric Janouch
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_LUA_H__
#define __LD_LUA_H__

G_BEGIN_DECLS


#define LD_TYPE_LUA (ld_lua_get_type ())
#define LD_LUA(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), LD_TYPE_LUA, LdLua))
#define LD_LUA_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), LD_TYPE_LUA, LdLuaClass))
#define LD_IS_LUA(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LD_TYPE_LUA))
#define LD_IS_LUA_CLASS(klass) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((klass), LD_TYPE_LUA))
#define LD_LUA_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), LD_LUA, LdLuaClass))

typedef struct _LdLua LdLua;
typedef struct _LdLuaPrivate LdLuaPrivate;
typedef struct _LdLuaClass LdLuaClass;


struct _LdLua
{
/*< private >*/
	GObject parent_instance;
	LdLuaPrivate *priv;
};

/* TODO: A virtual superclass, so other engines can be used. */
struct _LdLuaClass
{
/*< private >*/
	GObjectClass parent_class;
};


/**
 * LdLuaLoadCallback:
 * @symbol: the symbol that has been created.
 * @user_data: user data passed to ld_lua_load_file().
 *
 * A callback function that is called when a symbol is created.
 */
typedef void (*LdLuaLoadCallback) (LdSymbol *symbol, gpointer user_data);


GType ld_lua_get_type (void) G_GNUC_CONST;

LdLua *ld_lua_new (void);
gboolean ld_lua_check_file (LdLua *self, const gchar *filename);
gboolean ld_lua_load_file (LdLua *self, const gchar *filename,
	LdLuaLoadCallback callback, gpointer user_data);


G_END_DECLS

#endif /* ! __LD_LUA_H__ */
