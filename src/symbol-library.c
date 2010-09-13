/*
 * symbol-library.c
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
#include <stdlib.h>

#include "config.h"

#include "symbol-library.h"
#include "symbol-category.h"
#include "symbol.h"

/* ===== Symbol library ==================================================== */

/**
 * SECTION:symbol-library
 * @short_description: A symbol library.
 * @see_also: #LogdiagSymbol, #LogdiagSymbolCategory
 *
 * #LogdiagSymbolLibrary is used for loading symbols from their files.
 */

/*
 * LogdiagSymbolLibraryPrivate:
 * @lua_state: Lua state.
 */
struct _LogdiagSymbolLibraryPrivate
{
	lua_State *lua_state;
};

G_DEFINE_TYPE (LogdiagSymbolLibrary, logdiag_symbol_library, G_TYPE_OBJECT);

static void
logdiag_symbol_library_finalize (GObject *gobject);


static void
logdiag_symbol_library_class_init (LogdiagSymbolLibraryClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = logdiag_symbol_library_finalize;

/**
 * LogdiagSymbolLibrary::changed:
 * @library: The library object.
 *
 * Contents of the library have changed.
 */
	klass->changed_signal = g_signal_new
		("changed", G_TYPE_FROM_CLASS (object_class),
		G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
		0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	g_type_class_add_private (klass, sizeof (LogdiagSymbolLibraryPrivate));
}

static void
logdiag_symbol_library_init (LogdiagSymbolLibrary *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LOGDIAG_TYPE_SYMBOL_LIBRARY, LogdiagSymbolLibraryPrivate);

	/* TODO: lua */
	self->priv->lua_state = NULL;

	/* TODO: use _new_full and specify destroy functions. */
	self->categories = g_hash_table_new (g_str_hash, g_str_equal);
}

static void
logdiag_symbol_library_finalize (GObject *gobject)
{
	LogdiagSymbolLibrary *self;

	self = LOGDIAG_SYMBOL_LIBRARY (gobject);

	g_hash_table_destroy (self->categories);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (logdiag_symbol_library_parent_class)->finalize (gobject);
}

/**
 * logdiag_symbol_library_new:
 *
 * Create an instance.
 */
LogdiagSymbolLibrary *
logdiag_symbol_library_new (void)
{
	return g_object_new (LOGDIAG_TYPE_SYMBOL_LIBRARY, NULL);
}

/*
 * load_category:
 * @self: A symbol library object.
 * @path: The path to the category.
 * @name: The default name of the category.
 *
 * Loads a category into the library.
 */
LogdiagSymbolCategory *
load_category (LogdiagSymbolLibrary *self, const char *path, const char *name)
{
	LogdiagSymbolCategory *cat;
	gchar *icon_file;

	g_return_val_if_fail (LOGDIAG_IS_SYMBOL_LIBRARY (self), NULL);
	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	icon_file = g_build_filename (path, "icon.svg", NULL);
	if (!g_file_test (icon_file, G_FILE_TEST_IS_REGULAR))
	{
		g_warning ("The category in %s has no icon.", path);
		g_free (icon_file);
		return NULL;
	}

	/* TODO: Search for category.json and read the category name from it. */
	/* TODO: Search for xyz.lua and load the objects into the category. */

	cat = logdiag_symbol_category_new (self);
	cat->name = g_strdup (name);
	cat->image_path = icon_file;
	return cat;
}

/**
 * logdiag_symbol_library_load:
 * @self: A symbol library object.
 * @directory: A directory to be loaded.
 *
 * Load the contents of a directory into the library.
 */
gboolean
logdiag_symbol_library_load (LogdiagSymbolLibrary *self, const char *path)
{
	GDir *dir;
	const gchar *item;

	g_return_val_if_fail (LOGDIAG_IS_SYMBOL_LIBRARY (self), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);

	dir = g_dir_open (path, 0, NULL);
	if (!dir)
		return FALSE;

	while ((item = g_dir_read_name (dir)))
	{
		LogdiagSymbolCategory *cat;
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
 * logdiag_symbol_library_clear:
 *
 * Clears all the contents.
 */
void
logdiag_symbol_library_clear (LogdiagSymbolLibrary *self)
{
	g_return_if_fail (LOGDIAG_IS_SYMBOL_LIBRARY (self));

	g_hash_table_remove_all (self->categories);
	return;
}


/* ===== Symbol category =================================================== */

/**
 * SECTION:symbol-category
 * @short_description: A category of symbols.
 * @see_also: #LogdiagSymbol, #LogdiagSymbolLibrary
 *
 * #LogdiagSymbolCategory represents a category of #LogdiagSymbol objects.
 */

G_DEFINE_TYPE (LogdiagSymbolCategory, logdiag_symbol_category, G_TYPE_OBJECT);

static void
logdiag_symbol_category_finalize (GObject *gobject);


static void
logdiag_symbol_category_class_init (LogdiagSymbolCategoryClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = logdiag_symbol_category_finalize;
}

static void
logdiag_symbol_category_init (LogdiagSymbolCategory *self)
{
	/* TODO: use _new_full, correct equal and specify destroy functions. */
	/* XXX: How's the situation with subcategory names and symbol names
	 *      within the same hashtable?
	 */
	self->children = g_hash_table_new (g_str_hash, g_str_equal);
}

static void
logdiag_symbol_category_finalize (GObject *gobject)
{
	LogdiagSymbolCategory *self;

	self = LOGDIAG_SYMBOL_CATEGORY (gobject);

	if (self->name)
		g_free (self->name);
	if (self->image_path)
		g_free (self->image_path);

	g_object_unref (self->parent);
	g_hash_table_destroy (self->children);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (logdiag_symbol_category_parent_class)->finalize (gobject);
}

/**
 * logdiag_symbol_category_new:
 *
 * Create an instance.
 */
LogdiagSymbolCategory *
logdiag_symbol_category_new (LogdiagSymbolLibrary *parent)
{
	LogdiagSymbolCategory *cat;

	cat = g_object_new (LOGDIAG_TYPE_SYMBOL_CATEGORY, NULL);

	cat->parent = parent;
	g_object_ref (parent);

	return cat;
}


/* ===== Symbol ============================================================ */

/**
 * SECTION:symbol
 * @short_description: A symbol.
 * @see_also: #LogdiagDocument, #LogdiagCanvas
 *
 * #LogdiagSymbol represents a symbol in the #LogdiagDocument that is in turn
 * drawn onto the #LogdiagCanvas.
 */

/*
 * LogdiagSymbolPrivate:
 * @parent_library: The parent LogdiagSymbolLibrary.
 * The library contains the real function for rendering.
 */
struct _LogdiagSymbolPrivate
{
	LogdiagSymbolLibrary *parent_library;
};

/**
 * logdiag_symbol_build_identifier:
 *
 * Build an identifier for the symbol.
 * The identifier is in the format "Category/Category/Symbol".
 */
char *
logdiag_symbol_build_identifier (LogdiagSymbol *self)
{
	return NULL;
}

/**
 * logdiag_symbol_draw:
 *
 * Draw the symbol onto a Cairo surface.
 */
void
logdiag_symbol_draw (LogdiagSymbol *self, cairo_t *surface,
	GHashTable *param, gint x, gint y, gdouble zoom)
{
	return;
}
