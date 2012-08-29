/*
 * ld-library.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010, 2011, 2012. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <string.h>

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-library
 * @short_description: A symbol library
 * @see_also: #LdSymbol, #LdSymbolCategory
 *
 * #LdLibrary is used for loading symbols from their files.  The library object
 * itself is a container for categories, which in turn contain other
 * subcategories and the actual symbols.
 */

/*
 * LdLibraryPrivate:
 * @lua: state of the scripting language.
 * @children: categories in the library.
 */
struct _LdLibraryPrivate
{
	LdLua *lua;
	LdSymbolCategory *root;
};

static void ld_library_finalize (GObject *gobject);

static LdSymbolCategory *load_category (LdLibrary *self,
	const gchar *path, const gchar *name);
static gboolean load_category_cb (const gchar *base,
	const gchar *path, gpointer userdata);
static void load_category_symbol_cb (LdSymbol *symbol, gpointer user_data);

static gchar *read_human_name_from_file (const gchar *filename);

static gboolean foreach_dir (const gchar *path,
	gboolean (*callback) (const gchar *, const gchar *, gpointer),
	gpointer userdata, GError **error);


G_DEFINE_TYPE (LdLibrary, ld_library, G_TYPE_OBJECT);

static void
ld_library_class_init (LdLibraryClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = ld_library_finalize;

/**
 * LdLibrary::changed:
 * @self: an #LdLibrary object.
 *
 * Contents of the library have changed.
 */
	klass->changed_signal = g_signal_new
		("changed", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST, 0, NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	g_type_class_add_private (klass, sizeof (LdLibraryPrivate));
}

static void
ld_library_init (LdLibrary *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_LIBRARY, LdLibraryPrivate);

	self->priv->lua = ld_lua_new ();
	self->priv->root = ld_symbol_category_new ("/", "/");
}

static void
ld_library_finalize (GObject *gobject)
{
	LdLibrary *self;

	self = LD_LIBRARY (gobject);

	g_object_unref (self->priv->lua);
	g_object_unref (self->priv->root);

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
	guint changed : 1;
	guint load_symbols : 1;
}
LoadCategoryData;

/*
 * load_category:
 * @self: an #LdLibrary object.
 * @path: the path to the category.
 * @name: the default name of the category.
 *
 * Loads a category into the library.
 */
static LdSymbolCategory *
load_category (LdLibrary *self, const gchar *path, const gchar *name)
{
	gchar *category_file, *human_name;
	LoadCategoryData data;

	g_return_val_if_fail (LD_IS_LIBRARY (self), NULL);
	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	if (!g_file_test (path, G_FILE_TEST_IS_DIR))
		return NULL;

	category_file = g_build_filename (path, "category.json", NULL);
	human_name = read_human_name_from_file (category_file);
	if (!human_name)
		human_name = g_strdup (name);

	data.self = self;
	data.cat = ld_symbol_category_new (name, human_name);
	data.load_symbols = TRUE;
	data.changed = FALSE;
	foreach_dir (path, load_category_cb, &data, NULL);

	g_free (human_name);
	g_free (category_file);
	return data.cat;
}

/*
 * load_category_cb:
 *
 * Load contents of a directory into a symbol category.
 */
static gboolean
load_category_cb (const gchar *base, const gchar *path, gpointer userdata)
{
	LoadCategoryData *data;

	g_return_val_if_fail (base != NULL, FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (userdata != NULL, FALSE);

	data = (LoadCategoryData *) userdata;

	if (g_file_test (path, G_FILE_TEST_IS_DIR))
	{
		LdSymbolCategory *cat;

		cat = load_category (data->self, path, base);
		if (cat)
		{
			ld_symbol_category_insert_subcategory (data->cat, cat, -1);
			g_object_unref (cat);
		}
	}
	else if (data->load_symbols
		&& ld_lua_check_file (data->self->priv->lua, path))
	{
		ld_lua_load_file (data->self->priv->lua, path,
			load_category_symbol_cb, data->cat);
	}

	data->changed = TRUE;
	return TRUE;
}

/*
 * load_category_symbol_cb:
 *
 * Insert newly registered symbols into the category.
 */
static void
load_category_symbol_cb (LdSymbol *symbol, gpointer user_data)
{
	LdSymbolCategory *cat;

	g_return_if_fail (LD_IS_SYMBOL (symbol));
	g_return_if_fail (LD_IS_SYMBOL_CATEGORY (user_data));

	cat = LD_SYMBOL_CATEGORY (user_data);
	ld_symbol_category_insert_symbol (cat, symbol, -1);
}

/*
 * read_human_name_from_file:
 * @filename: location of the JSON file.
 *
 * Read the human name of the processed category.
 */
static gchar *
read_human_name_from_file (const gchar *filename)
{
	const gchar *const *lang;
	JsonParser *parser;
	JsonNode *root;
	JsonObject *object;
	GError *error;

	g_return_val_if_fail (filename != NULL, NULL);

	parser = json_parser_new ();
	error = NULL;
	if (!json_parser_load_from_file (parser, filename, &error))
	{
		g_warning ("%s", error->message);
		g_error_free (error);
		goto read_human_name_from_file_end;
	}

	root = json_parser_get_root (parser);
	if (!JSON_NODE_HOLDS_OBJECT (root))
	{
		g_warning ("failed to parse `%s': %s", filename,
			"The root node is not an object.");
		goto read_human_name_from_file_end;
	}

	object = json_node_get_object (root);
	for (lang = g_get_language_names (); *lang; lang++)
	{
		const gchar *member;

		if (!json_object_has_member (object, *lang))
			continue;
		member = json_object_get_string_member (object, *lang);

		if (member != NULL)
		{
			gchar *result;

			result = g_strdup (member);
			g_object_unref (parser);
			return result;
		}
	}

read_human_name_from_file_end:
	g_object_unref (parser);
	return NULL;
}

/**
 * ld_library_load:
 * @self: an #LdLibrary object.
 * @directory: a directory to be loaded.
 *
 * Load the contents of a directory into the library.
 */
gboolean
ld_library_load (LdLibrary *self, const gchar *directory)
{
	LoadCategoryData data;

	g_return_val_if_fail (LD_IS_LIBRARY (self), FALSE);
	g_return_val_if_fail (directory != NULL, FALSE);

	/* Almost like load_category(). */
	data.self = self;
	data.cat = self->priv->root;
	data.load_symbols = FALSE;
	data.changed = FALSE;
	foreach_dir (directory, load_category_cb, &data, NULL);

	if (data.changed)
		g_signal_emit (self, LD_LIBRARY_GET_CLASS (self)->changed_signal, 0);

	return TRUE;
}

/**
 * ld_library_find_symbol:
 * @self: an #LdLibrary object.
 * @identifier: an identifier of the symbol to be searched for.
 *
 * Search for a symbol in the library.
 *
 * Return value: a symbol object if found, %NULL otherwise.
 */
LdSymbol *
ld_library_find_symbol (LdLibrary *self, const gchar *identifier)
{
	gchar **id_el_start, **id_el;
	const GSList *list, *list_el;
	LdSymbolCategory *cat;

	g_return_val_if_fail (LD_IS_LIBRARY (self), NULL);
	g_return_val_if_fail (identifier != NULL, NULL);

	id_el_start = g_strsplit (identifier, LD_LIBRARY_IDENTIFIER_SEPARATOR, 0);
	if (!id_el_start)
		return NULL;

	/* We need at least one category name plus the symbol name. */
	id_el = id_el_start;
	if (!id_el[0] || !id_el[1])
		goto ld_library_find_symbol_error;

	/* Find the category where the symbol is in. */
	cat = self->priv->root;
	while (1)
	{
		gboolean found = FALSE;

		list = ld_symbol_category_get_subcategories (cat);
		for (list_el = list; list_el; list_el = g_slist_next (list_el))
		{
			cat = LD_SYMBOL_CATEGORY (list_el->data);
			if (!strcmp (*id_el, ld_symbol_category_get_name (cat)))
			{
				found = TRUE;
				break;
			}
		}

		if (!found)
			goto ld_library_find_symbol_error;

		if (!(id_el++)[2])
			break;
	}

	/* And then the actual symbol. */
	list = ld_symbol_category_get_symbols (cat);
	for (list_el = list; list_el; list_el = g_slist_next (list_el))
	{
		LdSymbol *symbol;

		symbol = LD_SYMBOL (list_el->data);
		if (!strcmp (*id_el, ld_symbol_get_name (symbol)))
		{
			g_strfreev (id_el_start);
			return symbol;
		}
	}

ld_library_find_symbol_error:
	g_strfreev (id_el_start);
	return NULL;
}

/**
 * ld_library_get_root:
 * @self: an #LdLibrary object.
 *
 * Return value: (transfer none): the root category. Do not modify.
 */
LdSymbolCategory *
ld_library_get_root (LdLibrary *self)
{
	g_return_val_if_fail (LD_IS_LIBRARY (self), NULL);
	return self->priv->root;
}

