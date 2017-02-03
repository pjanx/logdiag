/*
 * diagram.c
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2011
 *
 * See the file LICENSE for licensing information.
 *
 */

#include <liblogdiag/liblogdiag.h>

typedef struct
{
	LdDiagram *diagram;
}
Diagram;

static void
diagram_setup (Diagram *fixture, gconstpointer test_data)
{
	fixture->diagram = ld_diagram_new ();
}

static void
diagram_teardown (Diagram *fixture, gconstpointer test_data)
{
	g_object_unref (fixture->diagram);
}

static void
diagram_test_history (Diagram *fixture, gconstpointer user_data)
{
	const gdouble start_x = 1;
	const guint move_length = 3;
	LdDiagramObject *object;
	guint i;
	gdouble x;

	object = ld_diagram_object_new (NULL);
	g_object_set (object, "x", start_x, NULL);

	/* Link an object with the diagram. */
	ld_diagram_insert_object (fixture->diagram, object, 0);
	g_assert (ld_diagram_can_undo (fixture->diagram) != FALSE);

	/* Create some object actions to undo. */
	for (i = 0; i++ < move_length;)
		g_object_set (object, "x", start_x + i, NULL);

	/* Undo them and check the state. */
	for (i = move_length; i--;)
	{
		g_assert (ld_diagram_can_undo (fixture->diagram) != FALSE);
		ld_diagram_undo (fixture->diagram);

		g_object_get (object, "x", &x, NULL);
		g_assert_cmpfloat (x, ==, start_x + i);
	}

	/* Redo them and check the state. */
	for (i = 0; i++ < move_length;)
	{
		g_assert (ld_diagram_can_redo (fixture->diagram) != FALSE);
		ld_diagram_redo (fixture->diagram);

		g_object_get (object, "x", &x, NULL);
		g_assert_cmpfloat (x, ==, start_x + i);
	}

	g_object_unref (object);
}

static void
diagram_test_history_grouping (Diagram *fixture, gconstpointer user_data)
{
	const LdPoint start_position = {1, 3};
	const guint move_length = 3;
	LdDiagramObject *object;
	guint i;
	gdouble x, y;

	object = ld_diagram_object_new (NULL);
	g_object_set (object,
		"x", start_position.x,
		"y", start_position.y,
		NULL);

	/* Create a single user action. */
	ld_diagram_begin_user_action (fixture->diagram);
	ld_diagram_insert_object (fixture->diagram, object, 0);
	for (i = 0; i++ < move_length;)
	{
		ld_diagram_begin_user_action (fixture->diagram);
		g_object_set (object,
			"x", start_position.x + i,
			"y", start_position.y + i,
			NULL);
		ld_diagram_end_user_action (fixture->diagram);
	}
	ld_diagram_end_user_action (fixture->diagram);

	/* Undo the action. */
	g_assert (ld_diagram_get_objects (fixture->diagram) != NULL);
	g_assert (ld_diagram_can_undo (fixture->diagram) != FALSE);
	ld_diagram_undo (fixture->diagram);

	/* Check that it has been undone correctly. */
	g_assert (ld_diagram_get_objects (fixture->diagram) == NULL);
	g_assert (ld_diagram_can_undo (fixture->diagram) == FALSE);

	g_object_get (object, "x", &x, "y", &y, NULL);
	g_assert_cmpfloat (x, ==, start_position.x);
	g_assert_cmpfloat (y, ==, start_position.y);

	/* Redo the action. */
	g_assert (ld_diagram_can_redo (fixture->diagram) != FALSE);
	ld_diagram_redo (fixture->diagram);

	/* Check that it has been redone correctly. */
	g_object_get (object, "x", &x, "y", &y, NULL);
	g_assert_cmpfloat (x, ==, start_position.x + move_length);
	g_assert_cmpfloat (y, ==, start_position.y + move_length);

	g_object_unref (object);
}

int
main (int argc, char *argv[])
{
	gtk_test_init (&argc, &argv, NULL);

	/* History. */
	g_test_add ("/diagram/history", Diagram, NULL,
		diagram_setup, diagram_test_history,
		diagram_teardown);
	g_test_add ("/diagram/history-grouping", Diagram, NULL,
		diagram_setup, diagram_test_history_grouping,
		diagram_teardown);

	return g_test_run ();
}

