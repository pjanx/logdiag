/*
 * ld-library-toolbar.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2011. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_LIBRARY_TOOLBAR_H__
#define __LD_LIBRARY_TOOLBAR_H__

G_BEGIN_DECLS


#define LD_TYPE_LIBRARY_TOOLBAR (ld_library_toolbar_get_type ())
#define LD_LIBRARY_TOOLBAR(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_LIBRARY_TOOLBAR, LdLibraryToolbar))
#define LD_LIBRARY_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_LIBRARY_TOOLBAR, LdLibraryToolbarClass))
#define LD_IS_LIBRARY_TOOLBAR(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_LIBRARY_TOOLBAR))
#define LD_IS_LIBRARY_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_LIBRARY_TOOLBAR))
#define LD_LIBRARY_TOOLBAR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_LIBRARY_TOOLBAR, LdLibraryToolbarClass))

typedef struct _LdLibraryToolbar LdLibraryToolbar;
typedef struct _LdLibraryToolbarPrivate LdLibraryToolbarPrivate;
typedef struct _LdLibraryToolbarClass LdLibraryToolbarClass;


/**
 * LdLibraryToolbar:
 */
struct _LdLibraryToolbar
{
/*< private >*/
	GtkToolbar parent_instance;
	LdLibraryToolbarPrivate *priv;
};

struct _LdLibraryToolbarClass
{
/*< private >*/
	GtkToolbarClass parent_class;

	guint symbol_chosen_signal;
	guint symbol_selected_signal;
	guint symbol_deselected_signal;
};


GType ld_library_toolbar_get_type (void) G_GNUC_CONST;

GtkWidget *ld_library_toolbar_new (void);

void ld_library_toolbar_set_library (LdLibraryToolbar *self,
	LdLibrary *library);
LdLibrary *ld_library_toolbar_get_library (LdLibraryToolbar *self);
void ld_library_toolbar_set_canvas (LdLibraryToolbar *self,
	LdCanvas *canvas);
LdCanvas *ld_library_toolbar_get_canvas (LdLibraryToolbar *self);


G_END_DECLS

#endif /* ! __LD_LIBRARY_TOOLBAR_H__ */
