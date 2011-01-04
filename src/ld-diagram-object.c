/*
 * ld-diagram-object.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>

#include "config.h"

#include "ld-diagram-object.h"


/**
 * SECTION:ld-diagram-object
 * @short_description: A diagram object.
 * @see_also: #LdDiagram, #LdCanvas
 *
 * #LdDiagramObject represents an object in an #LdDiagram.
 */

/*
 * LdDiagramObjectPrivate:
 * @x: The X coordinate of this object.
 * @y: The Y coordinate of this object.
 */
struct _LdDiagramObjectPrivate
{
	gdouble x;
	gdouble y;
};

enum
{
	PROP_0,
	PROP_X,
	PROP_Y
};

static void ld_diagram_object_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_diagram_object_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);


G_DEFINE_ABSTRACT_TYPE (LdDiagramObject, ld_diagram_object, G_TYPE_OBJECT);

static void
ld_diagram_object_class_init (LdDiagramObjectClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_diagram_object_get_property;
	object_class->set_property = ld_diagram_object_set_property;

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
	case PROP_X:
		g_value_set_double (value, ld_diagram_object_get_x (self));
		break;
	case PROP_Y:
		g_value_set_double (value, ld_diagram_object_get_y (self));
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
	case PROP_X:
		ld_diagram_object_set_x (self, g_value_get_double (value));
		break;
	case PROP_Y:
		ld_diagram_object_set_y (self, g_value_get_double (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
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
	g_return_val_if_fail (LD_IS_DIAGRAM_OBJECT (self), 0);
	return self->priv->x;
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
	g_return_val_if_fail (LD_IS_DIAGRAM_OBJECT (self), 0);
	return self->priv->y;
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
	self->priv->x = x;

	g_object_notify (G_OBJECT (self), "x");
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
	self->priv->y = y;

	g_object_notify (G_OBJECT (self), "y");
}
