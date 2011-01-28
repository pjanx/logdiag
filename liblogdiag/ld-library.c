/*
 * ld-library.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010 - 2011. All rights reserved.
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
 * #LdLibrary is used for loading symbols from their files.
 */

/*
 * LdLibraryPrivate:
 * @lua: state of the scripting language.
 * @children: child objects of the library.
 */
struct _LdLibraryPrivate
{
	LdLua *lua;
	GSList *children;
};

static void ld_library_finalize (GObject *gobject);

static LdSymbolCategory *load_category (LdLibrary *self,
	const gchar *path, const gchar *name);
static gboolean load_category_cb (const gchar *base,
	const gchar *filename, gpointer userdata);
static void load_category_symbol_cb (LdSymbol *symbol, gpointer user_data);

static gchar *read_human_name_from_file (const gchar *filename);

static gboolean foreach_dir (const gchar *path,
	gboolean (*callback) (const gchar *, const gchar *, gpointer),
	gpointer userdata, GError **error);
static gboolean ld_library_load_cb
	(const gchar *base, const gchar *filename, gpointer userdata);


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
	self->priv->children = NULL;
}

static void
ld_library_finalize (GObject *gobject)
{
	LdLibrary *self;

	self = LD_LIBRARY (gobject);

	g_object_unref (self->priv->lua);

	g_slist_foreach (self->priv->children, (GFunc) g_object_unref, NULL);
	g_slist_free (self->priv->children);

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
	LdSymbolCategory *cat;
	gchar *icon_file, *category_file;
	gchar *human_name;
	LoadCategoryData data;

	g_return_val_if_fail (LD_IS_LIBRARY (self), NULL);
	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	if (!g_file_test (path, G_FILE_TEST_IS_DIR))
		goto load_category_fail_1;

	icon_file = g_build_filename (path, "icon.svg", NULL);
	if (!g_file_test (icon_file, G_FILE_TEST_IS_REGULAR))
	{
		g_warning ("the category in `%s' has no icon", path);
		goto load_category_fail_2;
	}

	category_file = g_build_filename (path, "category.json", NULL);
	human_name = read_human_name_from_file (category_file);
	if (!human_name)
		human_name = g_strdup (name);

	cat = ld_symbol_category_new (name, human_name);
	ld_symbol_category_set_image_path (cat, icon_file);

	data.self = self;
	data.cat = cat;
	foreach_dir (path, load_category_cb, &data, NULL);

	g_free (human_name);
	g_free (category_file);
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
		ld_lua_load_file (data->self->priv->lua, filename,
			load_category_symbol_cb, data->cat);
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
	const gchar *name;
	LdSymbolCategory *cat;
	const GSList *children, *iter;

	g_return_if_fail (LD_IS_SYMBOL (symbol));
	g_return_if_fail (LD_IS_SYMBOL_CATEGORY (user_data));

	cat = LD_SYMBOL_CATEGORY (user_data);
	name = ld_symbol_get_name (symbol);

	/* Check for name collisions with other symbols. */
	children = ld_symbol_category_get_children (cat);
	for (iter = children; iter; iter = iter->next)
	{
		if (!LD_IS_SYMBOL (iter->data))
			continue;
		if (!strcmp (name, ld_symbol_get_name (LD_SYMBOL (iter->data))))
		{
			g_warning ("attempted to insert multiple `%s' symbols into"
				" category `%s'", name, ld_symbol_category_get_name (cat));
			return;
		}
	}
	ld_symbol_category_insert_child (cat, G_OBJECT (symbol), -1);
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
 * @self: an #LdLibrary object.
 * @directory: a directory to be loaded.
 *
 * Load the contents of a directory into the library.
 */
gboolean
ld_library_load (LdLibrary *self, const gchar *directory)
{
	LibraryLoadData data;

	g_return_val_if_fail (LD_IS_LIBRARY (self), FALSE);
	g_return_val_if_fail (directory != NULL, FALSE);

	data.self = self;
	data.changed = FALSE;
	foreach_dir (directory, ld_library_load_cb, &data, NULL);

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
	LibraryLoadData *data;

	g_return_val_if_fail (base != NULL, FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);
	g_return_val_if_fail (userdata != NULL, FALSE);

	data = (LibraryLoadData *) userdata;

	cat = load_category (data->self, filename, base);
	if (cat)
		ld_library_insert_child (data->self, G_OBJECT (cat), -1);

	data->changed = TRUE;
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
/* XXX: With this level of indentation, this function is really ugly. */
LdSymbol *
ld_library_find_symbol (LdLibrary *self, const gchar *identifier)
{
	gchar **id_el_start, **id_el;
	const GSList *list, *list_el;

	g_return_val_if_fail (LD_IS_LIBRARY (self), NULL);
	g_return_val_if_fail (identifier != NULL, NULL);

	id_el_start = g_strsplit (identifier, LD_LIBRARY_IDENTIFIER_SEPARATOR, 0);
	if (!id_el_start)
		return NULL;

	list = ld_library_get_children (self);
	for (id_el = id_el_start; id_el[0]; id_el++)
	{
		LdSymbolCategory *cat;
		LdSymbol *symbol;
		gboolean found = FALSE;

		for (list_el = list; list_el; list_el = g_slist_next (list_el))
		{
			/* If the current identifier element is a category (not last)
			 * and this list element is a category.
			 */
			if (id_el[1] && LD_IS_SYMBOL_CATEGORY (list_el->data))
			{
				cat = LD_SYMBOL_CATEGORY (list_el->data);
				if (strcmp (id_el[0], ld_symbol_category_get_name (cat)))
					continue;

				list = ld_symbol_category_get_children (cat);
				found = TRUE;
				break;
			}
			/* If the current identifier element is a symbol (last)
			 * and this list element is a symbol.
			 */
			else if (!id_el[1] && LD_IS_SYMBOL (list_el->data))
			{
				symbol = LD_SYMBOL (list_el->data);
				if (strcmp (id_el[0], ld_symbol_get_name (symbol)))
					continue;

				g_strfreev (id_el_start);
				return symbol;
			}
		}

		if (!found)
			break;
	}
	g_strfreev (id_el_start);
	return NULL;
}

/**
 * ld_library_clear:
 * @self: an #LdLibrary object.
 *
 * Clear all the contents.
 */
void
ld_library_clear (LdLibrary *self)
{
	g_return_if_fail (LD_IS_LIBRARY (self));

	g_slist_foreach (self->priv->children, (GFunc) g_object_unref, NULL);
	g_slist_free (self->priv->children);
	self->priv->children = NULL;

	g_signal_emit (self,
		LD_LIBRARY_GET_CLASS (self)->changed_signal, 0);
}

/**
 * ld_library_insert_child:
 * @self: an #LdLibrary object.
 * @child: the child to be inserted.
 * @pos: the position at which the child will be inserted.
 *       Negative values will append to the end of list.
 *
 * Insert a child into the library.
 */
void
ld_library_insert_child (LdLibrary *self, GObject *child, gint pos)
{
	g_return_if_fail (LD_IS_LIBRARY (self));
	g_return_if_fail (G_IS_OBJECT (child));

	g_object_ref (child);
	self->priv->children = g_slist_insert (self->priv->children, child, pos);
}

/**
 * ld_library_remove_child:
 * @self: an #LdLibrary object.
 * @child: the child to be removed.
 *
 * Remove a child from the library.
 */
void
ld_library_remove_child (LdLibrary *self, GObject *child)
{
	g_return_if_fail (LD_IS_LIBRARY (self));
	g_return_if_fail (G_IS_OBJECT (child));

	if (g_slist_find (self->priv->children, child))
	{
		g_object_unref (child);
		self->priv->children = g_slist_remove (self->priv->children, child);
	}
}

/**
 * ld_library_get_children:
 * @self: an #LdLibrary object.
 *
 * Return value: the internal list of children. Do not modify.
 */
const GSList *
ld_library_get_children (LdLibrary *self)
{
	g_return_val_if_fail (LD_IS_LIBRARY (self), NULL);
	return self->priv->children;
}

