/*
 * ld-types.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010 - 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <math.h>

#include "liblogdiag.h"
#include "config.h"


/**
 * SECTION:ld-types
 * @short_description: Simple data types
 *
 * #LdPoint defines coordinates of a point.
 *
 * #LdPointArray defines an array of points.
 *
 * #LdRectangle defines the position and size of a rectangle.
 */

#define DEFINE_BOXED_TYPE(TypeName, type_name) \
GType \
type_name ## _get_type (void) \
{ \
	static GType our_type = 0; \
	if (our_type == 0) \
		our_type = g_boxed_type_register_static \
			(g_intern_static_string (#TypeName), \
			(GBoxedCopyFunc) type_name ## _copy, \
			(GBoxedFreeFunc) type_name ## _free); \
	return our_type; \
}

DEFINE_BOXED_TYPE (LdPoint, ld_point)
DEFINE_BOXED_TYPE (LdPointArray, ld_point_array)
DEFINE_BOXED_TYPE (LdRectangle, ld_rectangle)

#define DEFINE_BOXED_TRIVIAL_COPY(TypeName, type_name) \
TypeName * \
type_name ## _copy (const TypeName *self) \
{ \
	TypeName *new_copy; \
	g_return_val_if_fail (self != NULL, NULL); \
	new_copy = g_slice_new (TypeName); \
	*new_copy = *self; \
	return new_copy; \
}

#define DEFINE_BOXED_TRIVIAL_FREE(TypeName, type_name) \
void \
type_name ## _free (TypeName *self) \
{ \
	g_return_if_fail (self != NULL); \
	g_slice_free (TypeName, self); \
}

/**
 * ld_point_copy:
 * @self: an #LdPoint structure.
 *
 * Makes a copy of the structure.
 * The result must be freed by ld_point_free().
 *
 * Return value: a copy of @self.
 */
DEFINE_BOXED_TRIVIAL_COPY (LdPoint, ld_point)

/**
 * ld_point_free:
 * @self: an #LdPoint structure.
 *
 * Frees the structure created with ld_point_copy().
 */
DEFINE_BOXED_TRIVIAL_FREE (LdPoint, ld_point)

/**
 * ld_point_distance:
 * @self: an #LdPoint structure.
 * @x: the X coordinate of the second point.
 * @y: the Y coordinate of the second point.
 *
 * Compute the distance between two points.
 */
gdouble
ld_point_distance (LdPoint *self, gdouble x, gdouble y)
{
	gdouble dx, dy;

	g_return_val_if_fail (self != NULL, -1);

	dx = self->x - x;
	dy = self->y - y;
	return sqrt (dx * dx + dy * dy);
}

/**
 * ld_point_array_new:
 * @num_points: the number of points the array can store.
 *
 * Create a new array of points and initialize.
 *
 * Return value: an #LdPointArray structure.
 */
LdPointArray *
ld_point_array_new (gint num_points)
{
	LdPointArray *new_array;

	g_return_val_if_fail (num_points >= 1, NULL);

	new_array = g_slice_new (LdPointArray);
	new_array->num_points = num_points;
	new_array->points = g_malloc0 (num_points * sizeof (LdPoint));
	return new_array;
}

/**
 * ld_point_array_copy:
 * @self: an #LdPointArray structure.
 *
 * Makes a copy of the structure.
 * The result must be freed by ld_point_array_free().
 *
 * Return value: a copy of @self.
 */
LdPointArray *
ld_point_array_copy (const LdPointArray *self)
{
	LdPointArray *new_array;

	g_return_val_if_fail (self != NULL, NULL);

	new_array = g_slice_new (LdPointArray);
	new_array->num_points = self->num_points;
	new_array->points = g_memdup (self->points,
		self->num_points * sizeof (LdPoint));
	return new_array;
}

/**
 * ld_point_array_free:
 * @self: an #LdPointArray structure.
 *
 * Frees the structure created with ld_point_array_copy().
 */
void
ld_point_array_free (LdPointArray *self)
{
	g_return_if_fail (self != NULL);

	g_free (self->points);
	g_slice_free (LdPointArray, self);
}

/**
 * ld_rectangle_copy:
 * @self: an #LdRectangle structure.
 *
 * Makes a copy of the structure.
 * The result must be freed by ld_rectangle_free().
 *
 * Return value: a copy of @self.
 */
DEFINE_BOXED_TRIVIAL_COPY (LdRectangle, ld_rectangle)

/**
 * ld_rectangle_free:
 * @self: an #LdRectangle structure.
 *
 * Frees the structure created with ld_rectangle_copy().
 */
DEFINE_BOXED_TRIVIAL_FREE (LdRectangle, ld_rectangle)

/**
 * ld_rectangle_contains:
 * @self: an #LdRectangle structure.
 * @x: the X coordinate of the point to be checked.
 * @y: the Y coordinate of the point to be checked.
 *
 * Return value: %TRUE if the rectangle contains the specified point.
 */
gboolean
ld_rectangle_contains (LdRectangle *self, gdouble x, gdouble y)
{
	g_return_val_if_fail (self != NULL, FALSE);
	return (x >= self->x && x <= self->x + self->width
	     && y >= self->y && y <= self->y + self->height);
}

/**
 * ld_rectangle_intersects:
 * @self: an #LdRectangle structure.
 * @rect: an #LdRectangle to be checked for intersection.
 *
 * Return value: %TRUE if the two rectangles intersect.
 */
gboolean
ld_rectangle_intersects (LdRectangle *self, LdRectangle *rect)
{
	g_return_val_if_fail (self != NULL, FALSE);
	g_return_val_if_fail (rect != NULL, FALSE);

	return !(self->x > rect->x + rect->width
	      || self->y > rect->y + rect->height
	      || self->x + self->width  < rect->x
	      || self->y + self->height < rect->y);
}

/**
 * ld_rectangle_extend:
 * @self: an #LdRectangle structure.
 * @border: the border by which the rectangle should be extended.
 *
 * Extend a rectangle on all sides.
 */
void
ld_rectangle_extend (LdRectangle *self, gdouble border)
{
	g_return_if_fail (self != NULL);

	self->x -= border;
	self->y -= border;
	self->width  += 2 * border;
	self->height += 2 * border;
}
