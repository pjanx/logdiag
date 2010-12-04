/*
 * ld-document.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_DOCUMENT_H__
#define __LD_DOCUMENT_H__

G_BEGIN_DECLS


#define LD_TYPE_DOCUMENT (ld_document_get_type ())
#define LD_DOCUMENT(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_DOCUMENT, LdDocument))
#define LD_DOCUMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_DOCUMENT, LdDocumentClass))
#define LD_IS_DOCUMENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_DOCUMENT))
#define LD_IS_DOCUMENT_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_DOCUMENT))
#define LD_DOCUMENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_DOCUMENT, LdDocumentClass))

typedef struct _LdDocument LdDocument;
typedef struct _LdDocumentClass LdDocumentClass;
typedef struct _LdDocumentPrivate LdDocumentPrivate;


/**
 * LdDocument:
 *
 * A document object.
 */
struct _LdDocument
{
/*< private >*/
	GObject parent_instance;
	LdDocumentPrivate *priv;
};

struct _LdDocumentClass
{
/*< private >*/
	GObjectClass parent_class;

	guint changed_signal;
};


GType ld_document_get_type (void) G_GNUC_CONST;

LdDocument *ld_document_new (void);
void ld_document_clear (LdDocument *self);
gboolean ld_document_load_from_file (LdDocument *self,
	const gchar *filename, GError *error);
gboolean ld_document_save_to_file (LdDocument *self,
	const gchar *filename, GError *error);

GSList *ld_document_get_objects (LdDocument *self);
void ld_document_insert_object (LdDocument *self,
	LdDocumentObject *object, gint pos);
void ld_document_remove_object (LdDocument *self,
	LdDocumentObject *object);

GSList *ld_document_get_selection (LdDocument *self);
void ld_document_selection_add (LdDocument *self,
	LdDocumentObject *object, gint pos);
void ld_document_selection_remove (LdDocument *self,
	LdDocumentObject *object);

/*
GSList *ld_document_get_connections (LdDocument *self);
void ld_document_connection_add (LdDocument *self,
	LdConnection *connection, gint pos);
void ld_document_connection_remove (LdDocument *self,
	LdConnection *connection);
*/


G_END_DECLS

#endif /* ! __LD_DOCUMENT_H__ */

