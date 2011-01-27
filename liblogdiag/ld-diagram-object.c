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

static const gchar **args_to_strv (const gchar *first_arg, va_list args);


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

/**
 * LdDiagramObject::data-changed:
 * @self: An #LdDiagramObject object.
 * @path: Path to the data.
 * @old_value: (allow-none): The old value of data.
 * @new_value: (allow-none): The new value of data.
 *
 * Some data have been changed in internal storage.
 */
	klass->data_changed_signal = g_signal_new
		("data-changed", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST, 0, NULL, NULL,
		ld_marshal_VOID__BOXED_BOXED_BOXED, G_TYPE_NONE, 3,
		G_TYPE_STRV, G_TYPE_VALUE, G_TYPE_VALUE);

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
	GValue tmp_value;

	self = LD_DIAGRAM_OBJECT (object);
	switch (property_id)
	{
	case PROP_STORAGE:
		g_value_set_boxed (value, ld_diagram_object_get_storage (self));
		break;
	case PROP_X:
	case PROP_Y:
		memset (&tmp_value, 0, sizeof (GValue));
		ld_diagram_object_get_data_for_param (self, &tmp_value, pspec);
		g_value_copy (&tmp_value, value);
		g_value_unset (&tmp_value);
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
 * ld_diagram_object_get_data:
 * @self: An #LdDiagramObject object.
 * @data: (out): An uninitialized storage for the data.
 * @type: Requested type of data. %G_TYPE_NONE for any.
 * @first_element: The first element of path to the data.
 * @...: Optional remaining elements, followed by %NULL.
 *
 * Retrieve data from internal storage.
 *
 * Return value: %TRUE if successful.
 */
gboolean
ld_diagram_object_get_data (LdDiagramObject *self,
	GValue *data, GType type, const gchar *first_element, ...)
{
	va_list args;
	gboolean result;

	va_start (args, first_element);
	result = ld_diagram_object_get_data_valist (self,
		data, type, first_element, args);
	va_end (args);
	return result;
}

/**
 * ld_diagram_object_set_data:
 * @self: An #LdDiagramObject object.
 * @data: (allow-none): The data. %NULL just removes the current data.
 * @first_element: The first element of path where the data will be stored.
 * @...: Optional remaining elements, followed by %NULL.
 *
 * Put data into internal storage.
 */
void
ld_diagram_object_set_data (LdDiagramObject *self,
	const GValue *data, const gchar *first_element, ...)
{
	va_list args;

	va_start (args, first_element);
	ld_diagram_object_set_data_valist (self, data, first_element, args);
	va_end (args);
}

/**
 * ld_diagram_object_get_data_valist:
 * @self: An #LdDiagramObject object.
 * @data: (out): An uninitialized storage for the data.
 * @type: Requested type of data. %G_TYPE_NONE for any.
 * @first_element: The first element of path to the data.
 * @var_args: Optional remaining elements, followed by %NULL.
 *
 * Retrieve data from internal storage.
 *
 * Return value: %TRUE if successful.
 */
gboolean
ld_diagram_object_get_data_valist (LdDiagramObject *self,
	GValue *data, GType type, const gchar *first_element, va_list var_args)
{
	const gchar **elements;
	gboolean result;

	elements = args_to_strv (first_element, var_args);
	result = ld_diagram_object_get_datav (self, data, type, elements);
	g_free (elements);
	return result;
}

/**
 * ld_diagram_object_set_data_valist:
 * @self: An #LdDiagramObject object.
 * @data: (allow-none): The data. %NULL just removes the current data.
 * @first_element: The first element of path where the data will be stored.
 * @var_args: Optional remaining elements, followed by %NULL.
 *
 * Put data into internal storage.
 */
void
ld_diagram_object_set_data_valist (LdDiagramObject *self,
	const GValue *data, const gchar *first_element, va_list var_args)
{
	const gchar **elements;

	elements = args_to_strv (first_element, var_args);
	ld_diagram_object_set_datav (self, data, elements);
	g_free (elements);
}

static const gchar **
args_to_strv (const gchar *first_arg, va_list args)
{
	const gchar **strv, *arg;
	size_t strv_len = 0, strv_size = 8;

	strv = g_malloc (strv_size * sizeof (gchar *));
	for (arg = first_arg; ; arg = va_arg (args, const gchar *))
	{
		if (strv_len == strv_size)
			strv = g_realloc (strv, (strv_size <<= 1) * sizeof (gchar *));
		strv[strv_len++] = arg;

		if (!arg)
			break;
	}
	return strv;
}

/**
 * ld_diagram_object_get_datav:
 * @self: An #LdDiagramObject object.
 * @data: (out): An uninitialized storage for the data.
 * @type: Requested type of data. %G_TYPE_NONE for any.
 * @elements: An array of elements of path to the data, terminated by %NULL.
 *
 * Retrieve data from internal storage.
 *
 * Return value: %TRUE if successful.
 */
gboolean
ld_diagram_object_get_datav (LdDiagramObject *self,
	GValue *data, GType type, const gchar **elements)
{
	JsonObject *object;
	JsonNode *node;
	guint i;

	g_return_val_if_fail (LD_IS_DIAGRAM_OBJECT (self), FALSE);
	g_return_val_if_fail (data != NULL, FALSE);
	g_return_val_if_fail (elements != NULL && *elements, FALSE);

	object = ld_diagram_object_get_storage (self);
	node = json_object_get_member (object, elements[0]);
	for (i = 1; elements[i]; i++)
	{
		if (!node)
			return FALSE;
		if (!JSON_NODE_HOLDS_OBJECT (node))
		{
			g_warning ("%s: unable to get a member of a non-object node",
				G_STRFUNC);
			return FALSE;
		}
		object = json_node_get_object (node);
		node = json_object_get_member (object, elements[i]);
	}
	if (!node)
		return FALSE;
	if (!JSON_NODE_HOLDS_VALUE (node))
	{
		g_warning ("%s: unable to read from a non-value node", G_STRFUNC);
		return FALSE;
	}

	if (type == G_TYPE_NONE)
	{
		json_node_get_value (node, data);
		return TRUE;
	}
	if (g_value_type_transformable (json_node_get_value_type (node), type))
	{
		GValue json_value;

		memset (&json_value, 0, sizeof (GValue));
		json_node_get_value (node, &json_value);
		g_value_init (data, type);
		g_value_transform (&json_value, data);
		g_value_unset (&json_value);
		return TRUE;
	}
	g_warning ("%s: unable to get value of type `%s' from node of type `%s'",
		G_STRFUNC, g_type_name (type), json_node_type_name (node));
	return FALSE;
}

/**
 * ld_diagram_object_set_datav:
 * @self: An #LdDiagramObject object.
 * @data: (allow-none): The data. %NULL just removes the current data.
 * @elements: An array of elements of path where the data will be stored,
 *            terminated by %NULL.
 *
 * Put data into internal storage.
 */
void ld_diagram_object_set_datav (LdDiagramObject *self,
	const GValue *data, const gchar **elements)
{
	GValue tmp_value, *old_value;
	JsonObject *object, *new_object;
	JsonNode *node, *new_node;
	const gchar *last_element;
	guint i;

	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (self));
	g_return_if_fail (!data || G_IS_VALUE (data));
	g_return_if_fail (elements != NULL && *elements);

	object = ld_diagram_object_get_storage (self);
	node = json_object_get_member (object, elements[0]);
	last_element = elements[0];
	for (i = 1; elements[i]; i++)
	{
		if (!node || JSON_NODE_HOLDS_NULL (node))
		{
			new_object = json_object_new ();
			json_object_set_object_member (object, last_element, new_object);
			object = new_object;
			node = NULL;
		}
		else if (!JSON_NODE_HOLDS_OBJECT (node))
		{
			g_warning ("%s: unable to get a member of a non-object node",
				G_STRFUNC);
			return;
		}
		else
		{
			object = json_node_get_object (node);
			node = json_object_get_member (object, elements[i]);
		}
		last_element = elements[i];
	}

	if (!node || JSON_NODE_HOLDS_NULL (node))
		old_value = NULL;
	else if (!JSON_NODE_HOLDS_VALUE (node))
	{
		g_warning ("%s: unable to replace a non-value node", G_STRFUNC);
		return;
	}
	else
	{
		memset (&tmp_value, 0, sizeof (GValue));
		json_node_get_value (node, &tmp_value);
		old_value = &tmp_value;
	}

	/* We have to remove it first due to a bug in json-glib. */
	json_object_remove_member (object, last_element);
	if (data)
	{
		new_node = json_node_new (JSON_NODE_VALUE);
		json_node_set_value (new_node, data);
		json_object_set_member (object, last_element, new_node);
	}

	if (old_value || data)
		g_signal_emit (self, LD_DIAGRAM_OBJECT_GET_CLASS (self)
			->data_changed_signal, 0, old_value, data);
	if (old_value)
		g_value_unset (old_value);
}

/**
 * ld_diagram_object_get_data_for_param:
 * @self: An #LdDiagramObject object.
 * @data: (out): An uninitialized storage for the data.
 * @pspec: The parameter to read data for. This must be a property of @self.
 *
 * Retrieve data for a parameter from internal storage. If there's no data
 * corresponding to this parameter, the value is set to the default.
 */
void
ld_diagram_object_get_data_for_param (LdDiagramObject *self,
	GValue *data, GParamSpec *pspec)
{
	const gchar *elements[2];

	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (self));
	g_return_if_fail (data != NULL);
	g_return_if_fail (G_IS_PARAM_SPEC (pspec));
	g_return_if_fail (g_type_is_a (pspec->owner_type, LD_TYPE_DIAGRAM_OBJECT));

	elements[0] = g_param_spec_get_name (pspec);
	elements[1] = NULL;
	if (!ld_diagram_object_get_datav (self, data, pspec->value_type, elements))
	{
		g_value_init (data, pspec->value_type);
		g_param_value_set_default (pspec, data);
		g_object_set_property (G_OBJECT (self), elements[0], data);
	}
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
	const gchar *elements[2];

	g_return_if_fail (LD_IS_DIAGRAM_OBJECT (self));
	g_return_if_fail (G_IS_VALUE (data));
	g_return_if_fail (G_IS_PARAM_SPEC (pspec));

	elements[0] = g_param_spec_get_name (pspec);
	elements[1] = NULL;
	ld_diagram_object_set_datav (self, data, elements);
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
