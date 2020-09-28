/*
 * ld-types.c
 *
 * This file is a part of logdiag.
 * Copyright 2010, 2011 Přemysl Eric Janouch
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <math.h>
#include <string.h>

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
ld_point_distance (const LdPoint *self, gdouble x, gdouble y)
{
	gdouble dx, dy;

	g_return_val_if_fail (self != NULL, -1);

	dx = self->x - x;
	dy = self->y - y;
	return sqrt (dx * dx + dy * dy);
}

/**
 * ld_point_array_new:
 *
 * Create a new #LdPointArray.
 *
 * Return value: (transfer full): an #LdPointArray structure.
 */
LdPointArray *
ld_point_array_new (void)
{
	return ld_point_array_sized_new (0);
}

/**
 * ld_point_array_sized_new:
 * @preallocated: the number of points preallocated.
 *
 * Create a new #LdPointArray and preallocate storage for @preallocated items.
 *
 * Return value: (transfer full): an #LdPointArray structure.
 */
LdPointArray *
ld_point_array_sized_new (guint preallocated)
{
	LdPointArray *new_array;

	new_array = g_slice_new (LdPointArray);
	new_array->length = 0;
	new_array->size = preallocated;
	new_array->points = g_malloc0 (preallocated * sizeof (LdPoint));
	return new_array;
}

/**
 * ld_point_array_copy:
 * @self: an #LdPointArray structure.
 *
 * Makes a copy of the structure.
 * The result must be freed by ld_point_array_free().
 *
 * Return value: (transfer full): a copy of @self.
 */
LdPointArray *
ld_point_array_copy (const LdPointArray *self)
{
	LdPointArray *new_array;

	g_return_val_if_fail (self != NULL, NULL);

	new_array = g_slice_new (LdPointArray);
	new_array->length = self->length;
	new_array->size = self->size;
	new_array->points = g_memdup (self->points, self->size * sizeof (LdPoint));
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
 * ld_point_array_insert:
 * @self: an #LdPointArray structure.
 * @points: an array of points to be inserted.
 * @pos: the position at which the points should be inserted. This number
 *       must not be bigger than the number of points already present
 *       in the array. Negative values append to the array.
 * @length: count of points in @points.
 *
 * Insert points into the array.
 */
void
ld_point_array_insert (LdPointArray *self, LdPoint *points,
	gint pos, guint length)
{
	guint new_size;

	g_return_if_fail (self != NULL);
	g_return_if_fail (points != NULL);
	g_return_if_fail (pos <= (signed) self->length);

	new_size = self->size ? self->size : 1;
	while (self->length + length > new_size)
		new_size <<= 1;
	if (new_size != self->size)
		ld_point_array_set_size (self, new_size);

	if (pos < 0)
		pos = self->length;

	g_memmove (self->points + pos + length, self->points + pos,
		(self->length - pos) * sizeof (LdPoint));
	memcpy (self->points + pos, points, length * sizeof (LdPoint));
	self->length += length;
}

/**
 * ld_point_array_remove:
 * @self: an #LdPointArray structure.
 * @pos: the position at which the points should be removed.
 *       Negative values are relative to the end of the array.
 * @length: count of points to remove.
 *
 * Remove points from the array. The array may be resized as a result.
 */
void
ld_point_array_remove (LdPointArray *self, gint pos, guint length)
{
	guint new_size;

	g_return_if_fail (self != NULL);

	if (pos < 0)
	{
		pos += self->length;
		if (pos < 0)
		{
			length += pos;
			pos = 0;
		}
	}
	if ((unsigned) pos >= self->length)
		return;
	if (pos + length > self->length)
		length = self->length - pos;

	g_memmove (self->points + pos, self->points + pos + length,
		(self->length - pos) * sizeof (LdPoint));
	self->length -= length;

	new_size = self->size;
	while (new_size >> 2 > self->length)
		new_size >>= 1;
	if (new_size != self->size)
		ld_point_array_set_size (self, new_size);
}

/**
 * ld_point_array_set_size:
 * @self: an #LdPointArray structure.
 * @size: the new size.
 *
 * Change size of the array.
 */
void ld_point_array_set_size (LdPointArray *self, guint size)
{
	g_return_if_fail (self != NULL);

	if (self->size == size)
		return;

	self->points = g_realloc (self->points, size * sizeof (LdPoint));
	if (self->length > size)
		self->length = size;
	if (self->size < size)
		memset (self->points + self->length, 0, size - self->length);

	self->size = size;
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
 * ld_rectangle_intersects:
 * @self: an #LdRectangle structure.
 * @rect: an #LdRectangle to be checked for intersection.
 *
 * Return value: %TRUE if the two rectangles intersect.
 */
gboolean
ld_rectangle_intersects (const LdRectangle *self, const LdRectangle *rect)
{
	g_return_val_if_fail (self != NULL, FALSE);
	g_return_val_if_fail (rect != NULL, FALSE);

	return !(self->x > rect->x + rect->width
	      || self->y > rect->y + rect->height
	      || self->x + self->width  < rect->x
	      || self->y + self->height < rect->y);
}

/**
 * ld_rectangle_contains:
 * @self: an #LdRectangle structure.
 * @rect: an #LdRectangle to be checked for containment.
 *
 * Return value: %TRUE if @self fully contains @rect.
 */
gboolean
ld_rectangle_contains (const LdRectangle *self, const LdRectangle *rect)
{
	g_return_val_if_fail (self != NULL, FALSE);
	g_return_val_if_fail (rect != NULL, FALSE);

	return (self->x <= rect->x
	     && self->y <= rect->y
	     && self->x + self->width  >= rect->x + rect->width
	     && self->y + self->height >= rect->y + rect->height);
}

/**
 * ld_rectangle_contains_point:
 * @self: an #LdRectangle structure.
 * @point: the point to be checked.
 *
 * Return value: %TRUE if the rectangle contains the specified point.
 */
gboolean
ld_rectangle_contains_point (const LdRectangle *self, const LdPoint *point)
{
	g_return_val_if_fail (self != NULL, FALSE);
	g_return_val_if_fail (point != NULL, FALSE);

	return (point->x >= self->x && point->x <= self->x + self->width
	     && point->y >= self->y && point->y <= self->y + self->height);
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
