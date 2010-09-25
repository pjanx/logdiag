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

#include "ld-library.h"
#include "ld-symbol-category.h"
#include "ld-symbol.h"
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

	g_return_val_if_fail (LD_IS_LIBRARY (self), NULL);
	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	if (!g_file_test (path, G_FILE_TEST_IS_DIR))
		return NULL;

	icon_file = g_build_filename (path, "icon.svg", NULL);
	if (!g_file_test (icon_file, G_FILE_TEST_IS_REGULAR))
	{
		g_warning ("The category in %s has no icon.", path);
		g_free (icon_file);
		return NULL;
	}

	/* TODO: Search for category.json and read the category name from it. */
	/* TODO: Search for xyz.lua and load the objects into the category. */

	cat = ld_symbol_category_new (self);
	cat->name = g_strdup (name);
	cat->image_path = icon_file;
	return cat;
}

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
	GDir *dir;
	const gchar *item;
	gboolean changed = FALSE;

	g_return_val_if_fail (LD_IS_LIBRARY (self), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);

	dir = g_dir_open (path, 0, NULL);
	if (!dir)
		return FALSE;

	while ((item = g_dir_read_name (dir)))
	{
		LdSymbolCategory *cat;
		gchar *categ_path;

		categ_path = g_build_filename (path, item, NULL);
		cat = load_category (self, categ_path, item);
		if (cat)
			g_hash_table_insert (self->categories, cat->name, cat);
		g_free (categ_path);

		changed = TRUE;
	}
	g_dir_close (dir);

	if (changed)
		g_signal_emit (self,
			LD_LIBRARY_GET_CLASS (self)->changed_signal, 0);

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

