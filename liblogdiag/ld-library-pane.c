/*
 * ld-library-pane.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-library-pane
 * @short_description: A library pane
 * @see_also: #LdLibrary
 *
 * #LdLibraryPane enables the user to drag symbols from an #LdLibrary
 * onto #LdDiagramView.
 */

/*
 * LdLibraryPanePrivate:
 * @library: a library object assigned as a model.
 */
struct _LdLibraryPanePrivate
{
	LdLibrary *library;
};

enum
{
	PROP_0,
	PROP_LIBRARY
};

static void ld_library_pane_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_library_pane_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);
static void ld_library_pane_dispose (GObject *gobject);

static void reload_library (LdLibraryPane *self);
static void load_category_cb (gpointer data, gpointer user_data);


G_DEFINE_TYPE (LdLibraryPane, ld_library_pane, GTK_TYPE_VBOX);

static void
ld_library_pane_class_init (LdLibraryPaneClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_library_pane_get_property;
	object_class->set_property = ld_library_pane_set_property;
	object_class->dispose = ld_library_pane_dispose;

/**
 * LdLibraryPane:library:
 *
 * The #LdLibrary that this toolbar retrieves symbols from.
 */
	pspec = g_param_spec_object ("library", "Library",
		"The library that this toolbar retrieves symbols from.",
		LD_TYPE_LIBRARY, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_LIBRARY, pspec);

	g_type_class_add_private (klass, sizeof (LdLibraryPanePrivate));
}

static void
ld_library_pane_init (LdLibraryPane *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_LIBRARY_PANE, LdLibraryPanePrivate);
}

static void
ld_library_pane_dispose (GObject *gobject)
{
	LdLibraryPane *self;

	self = LD_LIBRARY_PANE (gobject);

	ld_library_pane_set_library (self, NULL);

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_library_pane_parent_class)->dispose (gobject);
}

static void
ld_library_pane_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdLibraryPane *self;

	self = LD_LIBRARY_PANE (object);
	switch (property_id)
	{
	case PROP_LIBRARY:
		g_value_set_object (value, ld_library_pane_get_library (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_library_pane_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdLibraryPane *self;

	self = LD_LIBRARY_PANE (object);
	switch (property_id)
	{
	case PROP_LIBRARY:
		ld_library_pane_set_library (self,
			LD_LIBRARY (g_value_get_object (value)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}


/**
 * ld_library_pane_new:
 *
 * Create an instance.
 */
GtkWidget *
ld_library_pane_new (void)
{
	return g_object_new (LD_TYPE_LIBRARY_PANE, NULL);
}

/**
 * ld_library_pane_set_library:
 * @self: an #LdLibraryPane object.
 * @library: (allow-none): the library to be assigned to the pane.
 *
 * Assign an #LdLibrary object to the pane.
 */
void
ld_library_pane_set_library (LdLibraryPane *self, LdLibrary *library)
{
	g_return_if_fail (LD_IS_LIBRARY_PANE (self));
	g_return_if_fail (LD_IS_LIBRARY (library) || library == NULL);

	if (self->priv->library)
	{
		g_signal_handlers_disconnect_by_func (self->priv->library,
			reload_library, self);
		g_object_unref (self->priv->library);
	}

	self->priv->library = library;

	if (library)
	{
		g_signal_connect_data (library, "changed",
			G_CALLBACK (reload_library), self,
			NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);
		g_object_ref (library);
	}
	reload_library (self);
	g_object_notify (G_OBJECT (self), "library");
}

/**
 * ld_library_pane_get_library:
 * @self: an #LdLibraryPane object.
 *
 * Return value: (transfer none): the #LdLibrary object
 *               assigned to the pane.
 */
LdLibrary *
ld_library_pane_get_library (LdLibraryPane *self)
{
	g_return_val_if_fail (LD_IS_LIBRARY_PANE (self), NULL);
	return self->priv->library;
}

static void
reload_library (LdLibraryPane *self)
{
	g_return_if_fail (LD_IS_LIBRARY_PANE (self));

	/* Clear the toolbar first, if there was already something in it. */
	gtk_container_foreach (GTK_CONTAINER (self),
		(GtkCallback) gtk_widget_destroy, NULL);

	if (self->priv->library)
	{
		GSList *categories;

		categories = (GSList *) ld_category_get_children
			(ld_library_get_root (self->priv->library));
		g_slist_foreach (categories, load_category_cb, self);
	}
}

static void
load_category_cb (gpointer data, gpointer user_data)
{
	LdLibraryPane *self;
	LdCategory *cat;
	GtkExpander *expander;
	const gchar *human_name;

	g_return_if_fail (LD_IS_LIBRARY_PANE (user_data));
	g_return_if_fail (LD_IS_CATEGORY (data));

	self = user_data;
	cat = data;

	/* TODO: Set a child for the expander, recurse into category children. */
	human_name = ld_category_get_human_name (cat);
	expander = GTK_EXPANDER (gtk_expander_new (human_name));

	gtk_box_pack_start (GTK_BOX (self), GTK_WIDGET (expander), FALSE, FALSE, 0);
}

