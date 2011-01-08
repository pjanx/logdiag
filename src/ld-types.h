/*
 * ld-types.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010 - 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_TYPES_H__
#define __LD_TYPES_H__

G_BEGIN_DECLS


/**
 * SECTION:ld-types
 * @short_description: Simple data types.
 *
 * #LdPoint defines coordinates of a point.
 *
 * #LdRectangle defines the position and size of a rectangle.
 */

#define LD_TYPE_POINT (ld_point_get_type ())
#define LD_TYPE_POINT_ARRAY (ld_point_array_get_type ())
#define LD_TYPE_RECTANGLE (ld_rectangle_get_type ())

typedef struct _LdPoint LdPoint;
typedef struct _LdPointArray LdPointArray;
typedef struct _LdRectangle LdRectangle;


/**
 * LdPoint:
 * @x: The X coordinate.
 * @y: The Y coordinate.
 *
 * Defines a point.
 */
struct _LdPoint
{
	gdouble x, y;
};

GType ld_point_get_type (void) G_GNUC_CONST;

LdPoint *ld_point_copy (const LdPoint *self);
void ld_point_free (LdPoint *self);


/**
 * LdPointArray:
 * @points: An array of #LdPoint structures.
 * @num_points: Count of points in @points.
 *
 * Moves quickly.
 */
struct _LdPointArray
{
	LdPoint *points;
	gint num_points;
};

GType ld_point_array_get_type (void) G_GNUC_CONST;

LdPointArray *ld_point_array_new (gint num_points);
LdPointArray *ld_point_array_copy (const LdPointArray *self);
void ld_point_array_free (LdPointArray *self);


/**
 * LdRectangle:
 * @x: Left-top X coordinate.
 * @y: Left-top Y coordinate.
 * @width: Width of the area, must be positive.
 * @height: Height of the area, must be positive.
 *
 * Defines a rectangle.
 */
struct _LdRectangle
{
	gdouble x, y;
	gdouble width, height;
};

GType ld_rectangle_get_type (void) G_GNUC_CONST;

LdRectangle *ld_rectangle_copy (const LdRectangle *self);
void ld_rectangle_free (LdRectangle *self);
gboolean ld_rectangle_contains (LdRectangle *self, gdouble x, gdouble y);
gboolean ld_rectangle_intersects (LdRectangle *self, LdRectangle *rect);
void ld_rectangle_extend (LdRectangle *self, gdouble border);


G_END_DECLS

#endif /* ! __LD_TYPES_H__ */

