/*
 * ld-library.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>

#include "config.h"

#include "ld-symbol.h"
#include "ld-symbol-category.h"
#include "ld-library.h"

#include "ld-lua.h"


/**
 * SECTION:ld-library
 * @short_description: A symbol library.
 * @see_also: #LdSymbol, #LdSymbolCategory
 *
 * #LdLibrary is used for loading symbols from their files.
 */

/*
 * LdLibraryPrivate:
 * @script_state: State of the scripting language.
 */
struct _LdLibraryPrivate
{
	LdLua *lua;
};

G_DEFINE_TYPE (LdLibrary, ld_library, G_TYPE_OBJECT);

static void
ld_library_finalize (GObject *gobject);

static LdSymbolCategory *
load_category (LdLibrary *self, const char *path, const char *name);

static gboolean
foreach_dir (const gchar *path,
	gboolean (*callback) (const gchar *, const gchar *, gpointer),
	gpointer userdata, GError **error);
static gboolean
load_category_cb (const gchar *base,
	const gchar *filename, gpointer userdata);
static gboolean
ld_library_load_cb (const gchar *base,
	const gchar *filename, gpointer userdata);


static void
ld_library_class_init (LdLibraryClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_library_finalize;

/**
 * LdLibrary::changed:
 * @library: The library object.
 *
 * Contents of the library have changed.
 */
	klass->changed_signal = g_signal_new
		("changed", G_TYPE_FROM_CLASS (object_class),
		G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
		0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	g_type_class_add_private (klass, sizeof (LdLibraryPrivate));
}

static void
ld_library_init (LdLibrary *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_LIBRARY, LdLibraryPrivate);

	self->priv->lua = ld_lua_new ();

	self->categories = g_hash_table_new_full (g_str_hash, g_str_equal,
		(GDestroyNotify) g_free, (GDestroyNotify) g_object_unref);
}

static void
ld_library_finalize (GObject *gobject)
{
	LdLibrary *self;

	self = LD_LIBRARY (gobject);

	g_object_unref (self->priv->lua);
	g_hash_table_destroy (self->categories);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_library_parent_class)->finalize (gobject);
}

/**
 * ld_library_new:
 *
 * Create an instance.
 */
LdLibrary *
ld_library_new (void)
{
	return g_object_new (LD_TYPE_LIBRARY, NULL);
}

/*
 * foreach_dir:
 *
 * Call a user-defined function for each file within a directory.
 */
static gboolean
foreach_dir (const gchar *path,
	gboolean (*callback) (const gchar *, const gchar *, gpointer),
	gpointer userdata, GError **error)
{
	GDir *dir;
	const gchar *item;

	/* FIXME: We don't set an error. */
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (callback != NULL, FALSE);

	dir = g_dir_open (path, 0, error);
	if (!dir)
		return FALSE;

	while ((item = g_dir_read_name (dir)))
	{
		gchar *filename;

		filename = g_build_filename (path, item, NULL);
		if (!callback (item, filename, userdata))
			break;
		g_free (filename);
	}
	g_dir_close (dir);
	return TRUE;
}

/*
 * LoadCategoryData:
 *
 * Data shared between load_category() and load_category_cb().
 */
typedef struct
{
	LdLibrary *self;
	LdSymbolCategory *cat;
}
LoadCategoryData;

/*
 * load_category:
 * @self: A symbol library object.
 * @path: The path to the category.
 * @name: The default name of the category.
 *
 * Loads a category into the library.
 */
static LdSymbolCategory *
load_category (LdLibrary *self, const char *path, const char *name)
{
	LdSymbolCategory *cat;
	gchar *icon_file;
	LoadCategoryData data;

	g_return_val_if_fail (LD_IS_LIBRARY (self), NULL);
	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	if (!g_file_test (path, G_FILE_TEST_IS_DIR))
		goto load_category_fail_1;

	icon_file = g_build_filename (path, "icon.svg", NULL);
	if (!g_file_test (icon_file, G_FILE_TEST_IS_REGULAR))
	{
		g_warning ("The category in %s has no icon.", path);
		goto load_category_fail_2;
	}

	/* TODO: Search for category.json and read the category name from it. */

	cat = ld_symbol_category_new (name);
	ld_symbol_category_set_image_path (cat, icon_file);

	data.self = self;
	data.cat = cat;
	foreach_dir (path, load_category_cb, &data, NULL);

	g_free (icon_file);
	return cat;

load_category_fail_2:
	g_free (icon_file);
load_category_fail_1:
	return NULL;
}

/*
 * load_category_cb:
 *
 * Load script files from a directory into a symbol category.
 */
static gboolean
load_category_cb (const gchar *base, const gchar *filename, gpointer userdata)
{
	LoadCategoryData *data;

	g_return_val_if_fail (base != NULL, FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);
	g_return_val_if_fail (userdata != NULL, FALSE);

	data = (LoadCategoryData *) userdata;

	if (ld_lua_check_file (data->self->priv->lua, filename))
		ld_lua_load_file_to_category
			(data->self->priv->lua, filename, data->cat);

	return TRUE;
}

/*
 * LibraryLoadData:
 *
 * Data shared between ld_library_load() and ld_library_load_cb().
 */
typedef struct
{
	LdLibrary *self;
	gboolean changed;
}
LibraryLoadData;

/**
 * ld_library_load:
 * @self: A symbol library object.
 * @directory: A directory to be loaded.
 *
 * Load the contents of a directory into the library.
 */
gboolean
ld_library_load (LdLibrary *self, const char *path)
{
	LibraryLoadData data;

	g_return_val_if_fail (LD_IS_LIBRARY (self), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);

	data.self = self;
	data.changed = FALSE;
	foreach_dir (path, ld_library_load_cb, &data, NULL);

	if (data.changed)
		g_signal_emit (self, LD_LIBRARY_GET_CLASS (self)->changed_signal, 0);

	return TRUE;
}

/*
 * ld_library_load_cb:
 *
 * A callback that's called for each file in the root directory.
 */
static gboolean
ld_library_load_cb (const gchar *base, const gchar *filename, gpointer userdata)
{
	LdSymbolCategory *cat;
	gchar *categ_path;
	LibraryLoadData *data;

	g_return_val_if_fail (base != NULL, FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);
	g_return_val_if_fail (userdata != NULL, FALSE);

	data = (LibraryLoadData *) userdata;

	cat = load_category (data->self, filename, base);
	if (cat)
		g_hash_table_insert (data->self->categories,
			g_strdup (ld_symbol_category_get_name (cat)), cat);

	data->changed = TRUE;
	return TRUE;
}

/**
 * ld_library_clear:
 * @self: A symbol library object.
 *
 * Clears all the contents.
 */
void
ld_library_clear (LdLibrary *self)
{
	g_return_if_fail (LD_IS_LIBRARY (self));

	g_hash_table_remove_all (self->categories);

	g_signal_emit (self,
		LD_LIBRARY_GET_CLASS (self)->changed_signal, 0);
}

