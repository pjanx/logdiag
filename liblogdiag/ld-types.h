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


#define LD_TYPE_POINT (ld_point_get_type ())
#define LD_TYPE_POINT_ARRAY (ld_point_array_get_type ())
#define LD_TYPE_RECTANGLE (ld_rectangle_get_type ())

typedef struct _LdPoint LdPoint;
typedef struct _LdPointArray LdPointArray;
typedef struct _LdRectangle LdRectangle;


/**
 * LdPoint:
 * @x: the X coordinate.
 * @y: the Y coordinate.
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
gdouble ld_point_distance (LdPoint *self, gdouble x, gdouble y);


/**
 * LdPointArray:
 * @points: an array of #LdPoint structures.
 * @num_points: count of points in @points.
 *
 * Defines an array of points.
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
 * @x: left-top X coordinate.
 * @y: left-top Y coordinate.
 * @width: width of the area, must be positive.
 * @height: height of the area, must be positive.
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

