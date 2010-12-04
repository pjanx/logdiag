/*
 * ld-document-object.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <gtk/gtk.h>

#include "config.h"

#include "ld-document-object.h"


/**
 * SECTION:ld-document-object
 * @short_description: A document object.
 * @see_also: #LdDocument, #LdCanvas
 *
 * #LdDocumentObject represents an object in an #LdDocument.
 */

/*
 * LdDocumentObjectPrivate:
 * @x: The X coordinate of this object.
 * @y: The Y coordinate of this object.
 */
struct _LdDocumentObjectPrivate
{
	gdouble x;
	gdouble y;
};

G_DEFINE_ABSTRACT_TYPE (LdDocumentObject, ld_document_object, G_TYPE_OBJECT);

enum
{
	PROP_0,
	PROP_X,
	PROP_Y
};

static void ld_document_object_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec);
static void ld_document_object_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec);


static void
ld_document_object_class_init (LdDocumentObjectClass *klass)
{
	GObjectClass *object_class;
	GParamSpec *pspec;

	object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = ld_document_object_get_property;
	object_class->set_property = ld_document_object_set_property;

/**
 * LdDocumentObject:x:
 *
 * The X coordinate of the object.
 */
	pspec = g_param_spec_double ("x", "X",
		"The X coordinate of this object.",
		-G_MAXDOUBLE, G_MAXDOUBLE, 0, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_X, pspec);

/**
 * LdDocumentObject:y:
 *
 * The Y coordinate of the object.
 */
	pspec = g_param_spec_double ("y", "Y",
		"The Y coordinate of this object.",
		-G_MAXDOUBLE, G_MAXDOUBLE, 0, G_PARAM_READWRITE);
	g_object_class_install_property (object_class, PROP_Y, pspec);

	g_type_class_add_private (klass, sizeof (LdDocumentObjectPrivate));
}

static void
ld_document_object_init (LdDocumentObject *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE
		(self, LD_TYPE_DOCUMENT_OBJECT, LdDocumentObjectPrivate);
}

static void
ld_document_object_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	LdDocumentObject *self;

	self = LD_DOCUMENT_OBJECT (object);
	switch (property_id)
	{
	case PROP_X:
		g_value_set_double (value, ld_document_object_get_x (self));
		break;
	case PROP_Y:
		g_value_set_double (value, ld_document_object_get_y (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
ld_document_object_set_property (GObject *object, guint property_id,
	const GValue *value, GParamSpec *pspec)
{
	LdDocumentObject *self;

	self = LD_DOCUMENT_OBJECT (object);
	switch (property_id)
	{
	case PROP_X:
		ld_document_object_set_x (self, g_value_get_double (value));
		break;
	case PROP_Y:
		ld_document_object_set_y (self, g_value_get_double (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}


/**
 * ld_document_object_get_x:
 * @self: An #LdDocumentObject object.
 *
 * Return value: The X coordinate of the object.
 */
gdouble
ld_document_object_get_x (LdDocumentObject *self)
{
	g_return_val_if_fail (LD_IS_DOCUMENT_OBJECT (self), 0);
	return self->priv->x;
}

/**
 * ld_document_object_get_y:
 * @self: An #LdDocumentObject object.
 *
 * Return value: The Y coordinate of the object.
 */
gdouble
ld_document_object_get_y (LdDocumentObject *self)
{
	g_return_val_if_fail (LD_IS_DOCUMENT_OBJECT (self), 0);
	return self->priv->y;
}

/**
 * ld_document_object_get_x:
 * @self: An #LdDocumentObject object.
 *
 * Set the X coordinate of the object.
 */
void
ld_document_object_set_x (LdDocumentObject *self, gdouble x)
{
	g_return_if_fail (LD_IS_DOCUMENT_OBJECT (self));
	self->priv->x = x;
}

/**
 * ld_document_object_get_x:
 * @self: An #LdDocumentObject object.
 *
 * Set the Y coordinate of the object.
 */
void
ld_document_object_set_y (LdDocumentObject *self, gdouble y)
{
	g_return_if_fail (LD_IS_DOCUMENT_OBJECT (self));
	self->priv->y = y;
}
