/*
 * ld-symbol-library.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>
#include <lua.h>
/* #include <lauxlib.h> */

#include "config.h"

#include "ld-symbol-library.h"
#include "ld-symbol-category.h"
#include "ld-symbol.h"

/* ===== Symbol library ==================================================== */

/**
 * SECTION:ld-symbol-library
 * @short_description: A symbol library.
 * @see_also: #LdSymbol, #LdSymbolCategory
 *
 * #LdSymbolLibrary is used for loading symbols from their files.
 */

/*
 * LdSymbolLibraryPrivate:
 * @lua_state: Lua state.
 */
struct _LdSymbolLibraryPrivate
{
	lua_State *lua_state;
};

G_DEFINE_TYPE (LdSymbolLibrary, ld_symbol_library, G_TYPE_OBJECT);

static void
ld_symbol_library_finalize (GObject *gobject);


static void
ld_symbol_library_class_init (LdSymbolLibraryClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_symbol_library_finalize;

/**
 * LdSymbolLibrary::changed:
 * @library: The library object.
 *
 * Contents of the library have changed.
 */
	klass->changed_signal = g_signal_new
		("changed", G_TYPE_FROM_CLASS (object_class),
		G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
		0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	g_type_class_add_private (klass, sizeof (LdSymbolLibraryPrivate));
}

static void
ld_symbol_library_init (LdSymbolLibrary *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_SYMBOL_LIBRARY, LdSymbolLibraryPrivate);

	/* TODO: lua */
	self->priv->lua_state = NULL;

	self->categories = g_hash_table_new_full (g_str_hash, g_str_equal,
		(GDestroyNotify) g_free, (GDestroyNotify) g_object_unref);
}

static void
ld_symbol_library_finalize (GObject *gobject)
{
	LdSymbolLibrary *self;

	self = LD_SYMBOL_LIBRARY (gobject);

	g_hash_table_destroy (self->categories);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_symbol_library_parent_class)->finalize (gobject);
}

/**
 * ld_symbol_library_new:
 *
 * Create an instance.
 */
LdSymbolLibrary *
ld_symbol_library_new (void)
{
	return g_object_new (LD_TYPE_SYMBOL_LIBRARY, NULL);
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
load_category (LdSymbolLibrary *self, const char *path, const char *name)
{
	LdSymbolCategory *cat;
	gchar *icon_file;

	g_return_val_if_fail (LD_IS_SYMBOL_LIBRARY (self), NULL);
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
 * ld_symbol_library_load:
 * @self: A symbol library object.
 * @directory: A directory to be loaded.
 *
 * Load the contents of a directory into the library.
 */
gboolean
ld_symbol_library_load (LdSymbolLibrary *self, const char *path)
{
	GDir *dir;
	const gchar *item;

	g_return_val_if_fail (LD_IS_SYMBOL_LIBRARY (self), FALSE);
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
	}
	g_dir_close (dir);
	return TRUE;
}

/**
 * ld_symbol_library_clear:
 * @self: A symbol library object.
 *
 * Clears all the contents.
 */
void
ld_symbol_library_clear (LdSymbolLibrary *self)
{
	g_return_if_fail (LD_IS_SYMBOL_LIBRARY (self));

	g_hash_table_remove_all (self->categories);
	return;
}


/* ===== Symbol category =================================================== */

/**
 * SECTION:ld-symbol-category
 * @short_description: A category of symbols.
 * @see_also: #LdSymbol, #LdSymbolLibrary
 *
 * #LdSymbolCategory represents a category of #LdSymbol objects.
 */

G_DEFINE_TYPE (LdSymbolCategory, ld_symbol_category, G_TYPE_OBJECT);

static void
ld_symbol_category_finalize (GObject *gobject);


static void
ld_symbol_category_class_init (LdSymbolCategoryClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_symbol_category_finalize;
}

static void
ld_symbol_category_init (LdSymbolCategory *self)
{
	/* TODO: use _new_full, correct equal and specify destroy functions. */
	/* XXX: How's the situation with subcategory names and symbol names
	 *      within the same hashtable?
	 */
	self->children = g_hash_table_new (g_str_hash, g_str_equal);
}

static void
ld_symbol_category_finalize (GObject *gobject)
{
	LdSymbolCategory *self;

	self = LD_SYMBOL_CATEGORY (gobject);

	if (self->name)
		g_free (self->name);
	if (self->image_path)
		g_free (self->image_path);

	g_object_unref (self->parent);
	g_hash_table_destroy (self->children);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_symbol_category_parent_class)->finalize (gobject);
}

/**
 * ld_symbol_category_new:
 * @parent: The parent library for this category.
 *
 * Create an instance.
 */
LdSymbolCategory *
ld_symbol_category_new (LdSymbolLibrary *parent)
{
	LdSymbolCategory *cat;

	cat = g_object_new (LD_TYPE_SYMBOL_CATEGORY, NULL);

	cat->parent = parent;
	g_object_ref (parent);

	return cat;
}


/* ===== Symbol ============================================================ */

/**
 * SECTION:ld-symbol
 * @short_description: A symbol.
 * @see_also: #LdDocument, #LdCanvas
 *
 * #LdSymbol represents a symbol in the #LdDocument that is in turn
 * drawn onto the #LdCanvas.
 */

/*
 * LdSymbolPrivate:
 * @library: The parent LdSymbolLibrary.
 * The library contains the real function for rendering.
 */
struct _LdSymbolPrivate
{
	LdSymbolLibrary *library;
};

G_DEFINE_TYPE (LdSymbol, ld_symbol, G_TYPE_OBJECT);

static void
ld_symbol_finalize (GObject *gobject);


static void
ld_symbol_class_init (LdSymbolClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_symbol_finalize;

	g_type_class_add_private (klass, sizeof (LdSymbolPrivate));
}

static void
ld_symbol_init (LdSymbol *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_SYMBOL_LIBRARY, LdSymbolPrivate);
}

static void
ld_symbol_finalize (GObject *gobject)
{
	LdSymbol *self;

	self = LD_SYMBOL (gobject);
	g_object_unref (self->priv->library);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_symbol_parent_class)->finalize (gobject);
}

/**
 * ld_symbol_new:
 * @library: A library object.
 * @filename: The file from which the symbol will be loaded.
 *
 * Load a symbol from a file into the library.
 */
LdSymbol *ld_symbol_new (LdSymbolLibrary *library,
	const gchar *filename)
{
	LdSymbol *symbol;

	symbol = g_object_new (LD_TYPE_SYMBOL, NULL);
	/* TODO: Use the filename, Luke. */

	symbol->priv->library = library;
	g_object_ref (library);
}

/**
 * ld_symbol_build_identifier:
 * @self: A symbol object.
 *
 * Build an identifier for the symbol.
 * The identifier is in the format "Category/Category/Symbol".
 */
char *
ld_symbol_build_identifier (LdSymbol *self)
{
	return NULL;
}

/**
 * ld_symbol_draw:
 * @self: A symbol object.
 * @surface: A cairo surface to be drawn on.
 * @param: Parameters for the symbol in a table.
 * @x: The X coordinate on the surface.
 * @y: The Y coordinate on the surface.
 * @zoom: Zoom ratio.
 *
 * Draw the symbol onto a Cairo surface.
 */
void
ld_symbol_draw (LdSymbol *self, cairo_t *surface,
	GHashTable *param, gint x, gint y, gdouble zoom)
{
	return;
}
