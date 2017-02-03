/*
 * point-array.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2011
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <liblogdiag/liblogdiag.h>

#define POINT_ARRAY_LENGTH 5

typedef struct
{
	LdPointArray *points;
}
PointArray;

static void
point_array_setup (PointArray *fixture, gconstpointer test_data)
{
	guint i;

	fixture->points = ld_point_array_sized_new (POINT_ARRAY_LENGTH);
	fixture->points->length = POINT_ARRAY_LENGTH;
	for (i = 0; i < POINT_ARRAY_LENGTH; i++)
	{
		fixture->points->points[i].x = i;
		fixture->points->points[i].y = i;
	}
}

static void
point_array_teardown (PointArray *fixture, gconstpointer test_data)
{
	ld_point_array_free (fixture->points);
}

static void
point_array_test_new (void)
{
	LdPointArray *points;

	points = ld_point_array_new ();
	g_assert_cmpuint (points->length, ==, 0);
	ld_point_array_free (points);
}

static void
point_array_test_sized_new (void)
{
	LdPointArray *points;

	points = ld_point_array_sized_new (5);
	g_assert_cmpuint (points->length, ==, 0);
	g_assert_cmpuint (points->size, ==, 5);
	g_assert (points->points != NULL);
	ld_point_array_free (points);
}

static void
point_array_test_insert (PointArray *fixture, gconstpointer user_data)
{
	LdPoint points[] = {{3, -1}, {4, -1}, {5, -9}};
	const guint offset = 1;
	guint i, j;

	ld_point_array_insert (fixture->points,
		points, offset, G_N_ELEMENTS (points));
	g_assert_cmpuint (fixture->points->length,
		==, POINT_ARRAY_LENGTH + G_N_ELEMENTS (points));

	for (i = 0, j = 0; i < POINT_ARRAY_LENGTH + G_N_ELEMENTS (points); i++)
	{
		/* Check that our values have been really inserted. */
		if (i >= offset && i < offset + G_N_ELEMENTS (points))
		{
			g_assert_cmpfloat (fixture->points->points[i].x,
				==, points[i - offset].x);
			g_assert_cmpfloat (fixture->points->points[i].y,
				==, points[i - offset].y);
			continue;
		}

		/* And everything else is intact. */
		g_assert_cmpfloat (fixture->points->points[i].x, ==, j);
		g_assert_cmpfloat (fixture->points->points[i].y, ==, j);
		j++;
	}
}

static void
point_array_test_remove (PointArray *fixture, gconstpointer user_data)
{
	const guint offset = 1;
	const guint length = 3;
	guint i, j;

	ld_point_array_remove (fixture->points, offset, length);
	g_assert_cmpuint (fixture->points->length,
		==, POINT_ARRAY_LENGTH - length);

	for (i = 0, j = 0; i < POINT_ARRAY_LENGTH; i++)
	{
		/* Leave out the hole. */
		if (i >= offset && i < offset + length)
			continue;

		/* And test that everything else is intact. */
		g_assert_cmpfloat (fixture->points->points[j].x, ==, i);
		g_assert_cmpfloat (fixture->points->points[j].y, ==, i);
		j++;
	}
}

int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);

	/* Creation. */
	g_test_add_func ("/point-array/new", point_array_test_new);
	g_test_add_func ("/point-array/sized-new", point_array_test_sized_new);

	/* Modification. */
	g_test_add ("/point-array/insert", PointArray, NULL,
		point_array_setup, point_array_test_insert,
		point_array_teardown);
	g_test_add ("/point-array/remove", PointArray, NULL,
		point_array_setup, point_array_test_remove,
		point_array_teardown);

	return g_test_run ();
}

