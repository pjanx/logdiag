/*
 * ld-library-pane.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_LIBRARY_PANE_H__
#define __LD_LIBRARY_PANE_H__

G_BEGIN_DECLS


#define LD_TYPE_LIBRARY_PANE (ld_library_pane_get_type ())
#define LD_LIBRARY_PANE(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_LIBRARY_PANE, LdLibraryPane))
#define LD_LIBRARY_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_LIBRARY_PANE, LdLibraryPaneClass))
#define LD_IS_LIBRARY_PANE(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_LIBRARY_PANE))
#define LD_IS_LIBRARY_PANE_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_LIBRARY_PANE))
#define LD_LIBRARY_PANE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_LIBRARY_PANE, LdLibraryPaneClass))

typedef struct _LdLibraryPane LdLibraryPane;
typedef struct _LdLibraryPanePrivate LdLibraryPanePrivate;
typedef struct _LdLibraryPaneClass LdLibraryPaneClass;


/**
 * LdLibraryPane:
 */
struct _LdLibraryPane
{
/*< private >*/
	GtkVBox parent_instance;
	LdLibraryPanePrivate *priv;
};

struct _LdLibraryPaneClass
{
/*< private >*/
	GtkVBoxClass parent_class;
};


GType ld_library_pane_get_type (void) G_GNUC_CONST;

GtkWidget *ld_library_pane_new (void);

void ld_library_pane_set_library (LdLibraryPane *self, LdLibrary *library);
LdLibrary *ld_library_pane_get_library (LdLibraryPane *self);


G_END_DECLS

#endif /* ! __LD_LIBRARY_PANE_H__ */
