/*
 * ld-library.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_LIBRARY_H__
#define __LD_LIBRARY_H__

G_BEGIN_DECLS


#define LD_TYPE_LIBRARY (ld_library_get_type ())
#define LD_LIBRARY(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_LIBRARY, LdLibrary))
#define LD_LIBRARY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_LIBRARY, LdLibraryClass))
#define LD_IS_LIBRARY(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_LIBRARY))
#define LD_IS_LIBRARY_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_LIBRARY))
#define LD_LIBRARY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_LIBRARY, LdLibraryClass))

typedef struct _LdLibrary LdLibrary;
typedef struct _LdLibraryPrivate LdLibraryPrivate;
typedef struct _LdLibraryClass LdLibraryClass;


/**
 * LdLibrary:
 * @categories: Lists all the categories (#LdSymbolCategory).
 *
 * Object structure.
 */
struct _LdLibrary
{
/*< private >*/
	GObject parent_instance;
	LdLibraryPrivate *priv;

/*< public >*/
	GHashTable *categories;
};

struct _LdLibraryClass
{
/*< private >*/
	GObjectClass parent_class;

	guint changed_signal;
};


GType ld_library_get_type (void) G_GNUC_CONST;

LdLibrary *ld_library_new (void);
gboolean ld_library_load (LdLibrary *self,
	const gchar *directory);
void ld_library_clear (LdLibrary *self);


G_END_DECLS

#endif /* ! __LD_LIBRARY_H__ */

