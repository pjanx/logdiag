/*
 * ld-diagram-connection.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2011
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_DIAGRAM_CONNECTION_H__
#define __LD_DIAGRAM_CONNECTION_H__

G_BEGIN_DECLS


#define LD_TYPE_DIAGRAM_CONNECTION (ld_diagram_connection_get_type ())
#define LD_DIAGRAM_CONNECTION(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_DIAGRAM_CONNECTION, LdDiagramConnection))
#define LD_DIAGRAM_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_DIAGRAM_CONNECTION, LdDiagramConnectionClass))
#define LD_IS_DIAGRAM_CONNECTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_DIAGRAM_CONNECTION))
#define LD_IS_DIAGRAM_CONNECTION_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_DIAGRAM_CONNECTION))
#define LD_DIAGRAM_CONNECTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_DIAGRAM_CONNECTION, LdDiagramConnectionClass))

typedef struct _LdDiagramConnection LdDiagramConnection;
typedef struct _LdDiagramConnectionPrivate LdDiagramConnectionPrivate;
typedef struct _LdDiagramConnectionClass LdDiagramConnectionClass;


/**
 * LdDiagramConnection:
 */
struct _LdDiagramConnection
{
/*< private >*/
	LdDiagramObject parent_instance;
};

/**
 * LdDiagramConnectionClass:
 */
struct _LdDiagramConnectionClass
{
/*< private >*/
	LdDiagramObjectClass parent_class;
};


GType ld_diagram_connection_get_type (void) G_GNUC_CONST;

LdDiagramConnection *ld_diagram_connection_new (JsonObject *storage);
LdPointArray *ld_diagram_connection_get_points (LdDiagramConnection *self);
void ld_diagram_connection_set_points (LdDiagramConnection *self,
	const LdPointArray *points);


G_END_DECLS

#endif /* ! __LD_DIAGRAM_CONNECTION_H__ */

