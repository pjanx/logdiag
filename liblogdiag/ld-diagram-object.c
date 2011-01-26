/*
 * ld-diagram-object.c
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
 * SECTION:ld-diagram-object
 * @short_description: A diagram object.
 * @see_also: #LdDiagram, #LdCanvas
 *
 * #LdDiagramObject represents an object in an #LdDiagram.
 */

/*
 * LdDiagramObjectPrivate:
 * @storage: Storage for object parameters.
 */
struct _LdDiagramObjectPrivate
{
	JsonObject *storage;
};

enum
{
	PROP_0,
	PROP_STORAGE,
	PROP_X,
	PROP_Y
};

static void ld_diagram_object_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_diagram_object_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);
static void ld_diagram_object_dispose (GObject *gobject);


G_DEFINE_TYPE (LdDiagramObject, ld_diagram_object, G_TYPE_OBJECT);

static void
ld_diagram_object_class_init (LdDiagramObjectClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_diagram_object_get_property;
	object_class->set_property = ld_diagram_object_set_property;
	object_class->dispose = ld_diagram_object_dispose;

/**
 * LdDiagramObject:storage:
 *
 * Storage for object parameters.
 */
	pspec = g_param_spec_boxed ("storage", "Storage",
		"Storage for object parameters.",
		JSON_TYPE_OBJECT, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_STORAGE, pspec);

/**
 * LdDiagramObject:x:
 *
 * The X coordinate of the object.
 */
	pspec = g_param_spec_double ("x", "X",
		"The X coordinate of this object.",
		-G_MAXDOUBLE, G_MAXDOUBLE, 0, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_X, pspec);

/**
 * LdDiagramObject:y:
 *
 * The Y coordinate of the object.
 */
	pspec = g_param_spec_double ("y", "Y",
		"The Y coordinate of this object.",
		-G_MAXDOUBLE, G_MAXDOUBLE, 0, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_Y, pspec);

	g_type_class_add_private (klass, sizeof (LdDiagramObjectPrivate));
}

static void
ld_diagram_object_init (LdDiagramObject *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_DIAGRAM_OBJECT, LdDiagramObjectPrivate);
}

static void
ld_diagram_object_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdDiagramObject *self;

	self = LD_DIAGRAM_OBJECT (object);
	switch (property_id)
	{
	case PROP_STORAGE:
		g_value_set_boxed (value, ld_diagram_object_get_storage (self));
		break;
	case PROP_X:
	case PROP_Y:
		ld_diagram_object_get_data_for_param (self, value, pspec);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_diagram_object_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdDiagramObject *self;

	self = LD_DIAGRAM_OBJECT (object);
	switch (property_id)
	{
	case PROP_STORAGE:
		ld_diagram_object_set_storage (self, g_value_get_boxed (value));
		break;
	case PROP_X:
	case PROP_Y:
		ld_diagram_object_set_data_for_param (self, value, pspec);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_diagram_object_dispose (GObject *gobject)
{
	LdDiagramObject *self;

	self = LD_DIAGRAM_OBJECT (gobject);
	if (self->priv->storage)
	{
		json_object_unref (self->priv->storage);
		self->priv->storage = NULL;
	}

	/* Chain up to the parent class. */
	G_OBJECT_CLASS (ld_diagram_object_parent_class)->dispose (gobject);
}


/**
 * ld_diagram_object_new:
 * @storage: A storage backend.
 *
 * Return value: A new #LdDiagramObject object.
 */
LdDiagramObject *
ld_diagram_object_new (JsonObject *storage)
{
	LdDiagramObject *self;

	self = g_object_new (LD_TYPE_DIAGRAM_OBJECT, "storage", storage, NULL);
	return self;
}

/**
 * ld_diagram_object_get_storage:
 * @self: An #LdDiagramObject object.
 *
 * Get the storage for object parameters.
 *
 * Return value: (transfer none): A #JsonObject boxed type.
 */
JsonObject *
ld_diagram_object_get_storage (LdDiagramObject *self)
{
	g_return_val_if_fail (LD_IS_DIAGRAM_OBJECT (self), NULL);
	if (!self->priv->storage)
		self->priv->storage = json_object_new ();
	return self->priv->storage;
}

/**
 * ld_diagram_object_set_storage:
 * @self: An #LdDiagramObject object.
 * @storage: (transfer none): A #JsonObject boxed type.
 *
 * Set the storage for object parameters.
 */
void
ld_diagram_object_set_storage (LdDiagramObject *self, JsonObject *storage)
{
	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (self));

	if (self->priv->storage)
		json_object_unref (self->priv->storage);

	if (storage)
		self->priv->storage = json_object_ref (storage);
	else
		self->priv->storage = NULL;
}

/**
 * ld_diagram_object_get_data_for_param:
 * @self: An #LdDiagramObject object.
 * @data: (out): Where the data will be stored.
 * @pspec: The parameter to read data for.
 *
 * Retrieve data for a parameter from internal storage.
 */
void
ld_diagram_object_get_data_for_param (LdDiagramObject *self,
	GValue *data, GParamSpec *pspec)
{
	JsonObject *storage;
	JsonNode *node;
	const gchar *name;
	GValue json_value;
	gboolean result;

	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (self));
	g_return_if_fail (G_IS_VALUE (data));
	g_return_if_fail (G_IS_PARAM_SPEC (pspec));

	storage = ld_diagram_object_get_storage (self);
	name = g_param_spec_get_name (pspec);
	node = json_object_get_member (storage, name);
	if (!node || json_node_is_null (node))
		goto ld_diagram_object_get_data_default;
	if (!JSON_NODE_HOLDS_VALUE (node))
		goto ld_diagram_object_get_data_warn;

	memset (&json_value, 0, sizeof (json_value));
	json_node_get_value (node, &json_value);
	result = g_param_value_convert (pspec, &json_value, data, FALSE);
	g_value_unset (&json_value);
	if (result)
		return;

ld_diagram_object_get_data_warn:
	g_warning ("%s: unable to get parameter `%s' of type `%s'"
		" from node of type `%s'; setting the parameter to it's default value",
		G_STRFUNC, name, G_PARAM_SPEC_TYPE_NAME (pspec),
		json_node_type_name (node));

ld_diagram_object_get_data_default:
	g_param_value_set_default (pspec, data);
	g_object_set_property (G_OBJECT (self), name, data);
}

/**
 * ld_diagram_object_set_data_for_param:
 * @self: An #LdDiagramObject object.
 * @data: The data.
 * @pspec: The parameter to put data for.
 *
 * Put data for a parameter into internal storage.
 */
void
ld_diagram_object_set_data_for_param (LdDiagramObject *self,
	const GValue *data, GParamSpec *pspec)
{
	JsonObject *storage;
	const gchar *name;
	JsonNode *node;

	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (self));
	g_return_if_fail (G_IS_VALUE (data));
	g_return_if_fail (G_IS_PARAM_SPEC (pspec));

	storage = ld_diagram_object_get_storage (self);
	name = g_param_spec_get_name (pspec);

	node = json_node_new (JSON_NODE_VALUE);
	json_node_set_value (node, data);
	/* We have to remove it first due to a bug in json-glib. */
	json_object_remove_member (storage, name);
	json_object_set_member (storage, name, node);
}

/**
 * ld_diagram_object_get_x:
 * @self: An #LdDiagramObject object.
 *
 * Return value: The X coordinate of the object.
 */
gdouble
ld_diagram_object_get_x (LdDiagramObject *self)
{
	gdouble x;

	g_return_val_if_fail (LD_IS_DIAGRAM_OBJECT (self), 0);
	g_object_get (self, "x", &x, NULL);
	return x;
}

/**
 * ld_diagram_object_get_y:
 * @self: An #LdDiagramObject object.
 *
 * Return value: The Y coordinate of the object.
 */
gdouble
ld_diagram_object_get_y (LdDiagramObject *self)
{
	gdouble y;

	g_return_val_if_fail (LD_IS_DIAGRAM_OBJECT (self), 0);
	g_object_get (self, "y", &y, NULL);
	return y;
}

/**
 * ld_diagram_object_set_x:
 * @self: An #LdDiagramObject object.
 * @x: The new X coordinate.
 *
 * Set the X coordinate of the object.
 */
void
ld_diagram_object_set_x (LdDiagramObject *self, gdouble x)
{
	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (self));
	g_object_set (self, "x", x, NULL);
}

/**
 * ld_diagram_object_set_y:
 * @self: An #LdDiagramObject object.
 * @y: The new Y coordinate.
 *
 * Set the Y coordinate of the object.
 */
void
ld_diagram_object_set_y (LdDiagramObject *self, gdouble y)
{
	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (self));
	g_object_set (self, "y", y, NULL);
}
