/*
 * ld-diagram-connection.c
 *
 * This file is a part of logdiag.
 * Copyright 2011 Přemysl Eric Janouch
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <string.h>

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-diagram-connection
 * @short_description: A connection object
 * @see_also: #LdDiagramObject
 *
 * #LdDiagramConnection is an implementation of #LdDiagramObject.
 */

typedef struct _SetPointsActionData SetPointsActionData;

/*
 * SetPointsActionData:
 * @self: the object this action has happened on.
 * @old_node: the old node.
 * @new_node: the new node.
 */
struct _SetPointsActionData
{
	LdDiagramConnection *self;
	JsonNode *old_node;
	JsonNode *new_node;
};

enum
{
	PROP_0,
	PROP_POINTS
};

static void ld_diagram_connection_get_property (GObject *object,
	guint property_id, GValue *value, GParamSpec *pspec);
static void ld_diagram_connection_set_property (GObject *object,
	guint property_id, const GValue *value, GParamSpec *pspec);

static gboolean read_point_node (JsonNode *node, LdPoint *point);
static gboolean read_double_node (JsonNode *node, gdouble *value);

static void on_set_points_undo (gpointer user_data);
static void on_set_points_redo (gpointer user_data);
static void on_set_points_destroy (gpointer user_data);


G_DEFINE_TYPE (LdDiagramConnection, ld_diagram_connection,
	LD_TYPE_DIAGRAM_OBJECT)

static void
ld_diagram_connection_class_init (LdDiagramConnectionClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_diagram_connection_get_property;
	object_class->set_property = ld_diagram_connection_set_property;

/**
 * LdDiagramConnection:points:
 *
 * Points defining this connection.
 */
	pspec = g_param_spec_boxed ("points", "Points",
		"Points defining this connection.",
		LD_TYPE_POINT_ARRAY, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_POINTS, pspec);
}

static void
ld_diagram_connection_init (LdDiagramConnection *self)
{
}

static void
ld_diagram_connection_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdDiagramConnection *self;

	self = LD_DIAGRAM_CONNECTION (object);
	switch (property_id)
	{
		LdPointArray *points;

	case PROP_POINTS:
		points = ld_diagram_connection_get_points (self);
		g_value_take_boxed (value, points);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_diagram_connection_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdDiagramConnection *self;

	self = LD_DIAGRAM_CONNECTION (object);
	switch (property_id)
	{
	case PROP_POINTS:
		ld_diagram_connection_set_points (self, g_value_get_boxed (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}


/**
 * ld_diagram_connection_new:
 * @storage: a storage backend.
 *
 * Return value: a new #LdDiagramConnection object.
 */
LdDiagramConnection *
ld_diagram_connection_new (JsonObject *storage)
{
	LdDiagramConnection *self;

	self = g_object_new (LD_TYPE_DIAGRAM_CONNECTION, "storage", storage, NULL);
	return self;
}

#define WARN_NODE_TYPE(node, type) \
	G_STMT_START { \
		g_warning ("%s: unable to read a value of type `%s' from node" \
			" of type `%s'", G_STRLOC, g_type_name (type), \
			json_node_type_name (node)); \
	} G_STMT_END

/**
 * ld_diagram_connection_get_points:
 * @self: an #LdDiagramConnection object.
 *
 * Get points defining this connection. Coordinates of the points are relative
 * to the inherited #LdDiagramObject:x and #LdDiagramObject:y properties.
 *
 * Return value: (transfer full): a point array.
 */
LdPointArray *
ld_diagram_connection_get_points (LdDiagramConnection *self)
{
	LdPointArray *points;
	JsonObject *storage;
	JsonNode *node;
	JsonArray *array;
	GList *point_node_list, *iter;

	g_return_val_if_fail (LD_IS_DIAGRAM_CONNECTION (self), NULL);

	storage = ld_diagram_object_get_storage (LD_DIAGRAM_OBJECT (self));
	node = json_object_get_member (storage, "points");
	if (!node || json_node_is_null (node))
		return ld_point_array_new ();
	if (!JSON_NODE_HOLDS_ARRAY (node))
	{
		WARN_NODE_TYPE (node, LD_TYPE_POINT_ARRAY);
		return ld_point_array_new ();
	}

	array = json_node_get_array (node);
	point_node_list = json_array_get_elements (array);
	points = ld_point_array_sized_new (json_array_get_length (array));

	for (iter = point_node_list; iter; iter = g_list_next (iter))
	{
		g_assert (points->length < points->size);

		if (read_point_node (iter->data, &points->points[points->length]))
			points->length++;
	}
	g_list_free (point_node_list);
	return points;
}

static gboolean
read_point_node (JsonNode *node, LdPoint *point)
{
	JsonArray *array;
	JsonNode *x_node, *y_node;

	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (point != NULL, FALSE);

	if (!JSON_NODE_HOLDS_ARRAY (node))
	{
		WARN_NODE_TYPE (node, LD_TYPE_POINT);
		return FALSE;
	}
	array = json_node_get_array (node);
	if (json_array_get_length (array) < 2)
	{
		g_warning ("%s: too few values for a point", G_STRLOC);
		return FALSE;
	}

	x_node = json_array_get_element (array, 0);
	y_node = json_array_get_element (array, 1);

	return read_double_node (x_node, &point->x)
		&& read_double_node (y_node, &point->y);
}

static gboolean
read_double_node (JsonNode *node, gdouble *value)
{
	GValue double_value, json_value;

	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (value != NULL, FALSE);

	if (!JSON_NODE_HOLDS_VALUE (node) || !g_value_type_transformable
		(json_node_get_value_type (node), G_TYPE_DOUBLE))
	{
		WARN_NODE_TYPE (node, G_TYPE_DOUBLE);
		return FALSE;
	}

	memset (&json_value, 0, sizeof (GValue));
	memset (&double_value, 0, sizeof (GValue));

	json_node_get_value (node, &json_value);
	g_value_init (&double_value, G_TYPE_DOUBLE);
	g_value_transform (&json_value, &double_value);
	*value = g_value_get_double (&double_value);

	g_value_unset (&json_value);
	g_value_unset (&double_value);
	return TRUE;
}

/**
 * ld_diagram_connection_set_points:
 * @self: an #LdDiagramConnection object.
 * @points: a point array.
 *
 * Set the points defining this connection.
 */
void
ld_diagram_connection_set_points (LdDiagramConnection *self,
	const LdPointArray *points)
{
	LdUndoAction *action;
	SetPointsActionData *action_data;
	JsonNode *node;
	JsonObject *storage;
	JsonArray *array, *point_array;
	guint i;

	g_return_if_fail (LD_IS_DIAGRAM_CONNECTION (self));
	g_return_if_fail (points != NULL);

	storage = ld_diagram_object_get_storage (LD_DIAGRAM_OBJECT (self));
	array = json_array_new ();
	for (i = 0; i < points->length; i++)
	{
		point_array = json_array_new ();
		json_array_add_double_element (point_array, points->points[i].x);
		json_array_add_double_element (point_array, points->points[i].y);
		json_array_add_array_element (array, point_array);
	}

	action_data = g_slice_new (SetPointsActionData);
	action_data->self = g_object_ref (self);

	node = json_object_get_member (storage, "points");
	action_data->old_node = node ? json_node_copy (node) : NULL;

	node = json_node_new (JSON_NODE_ARRAY);
	json_node_take_array (node, array);
	action_data->new_node = json_node_copy (node);

	json_object_set_member (storage, "points", node);

	action = ld_undo_action_new (on_set_points_undo, on_set_points_redo,
		on_set_points_destroy, action_data);
	ld_diagram_object_changed (LD_DIAGRAM_OBJECT (self), action);
	g_object_unref (action);
}

static void
on_set_points_undo (gpointer user_data)
{
	SetPointsActionData *data;
	JsonObject *storage;

	data = user_data;
	storage = ld_diagram_object_get_storage (LD_DIAGRAM_OBJECT (data->self));

	json_object_set_member (storage, "points", json_node_copy (data->old_node));
}

static void
on_set_points_redo (gpointer user_data)
{
	SetPointsActionData *data;
	JsonObject *storage;

	data = user_data;
	storage = ld_diagram_object_get_storage (LD_DIAGRAM_OBJECT (data->self));

	json_object_set_member (storage, "points", json_node_copy (data->new_node));
}

static void
on_set_points_destroy (gpointer user_data)
{
	SetPointsActionData *data;

	data = user_data;
	g_object_unref (data->self);
	if (data->old_node)
		json_node_free (data->old_node);
	if (data->new_node)
		json_node_free (data->new_node);
	g_slice_free (SetPointsActionData, data);
}

