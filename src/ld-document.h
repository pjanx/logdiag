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


#define LD_TYPE_DOCUMENT (ld_library_get_type ())
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


/**
 * LdDocument:
 *
 * A document object.
 */
struct _LdDocument
{
/*< private >*/
	GObject parent_instance;

/*< public >*/
};

struct _LdDocumentClass
{
	GObjectClass parent_class;
};


GType ld_document_get_type (void) G_GNUC_CONST;

LdDocument *ld_document_new (void);
gboolean ld_document_new_from_file (const char *file_name, GError *error);
gboolean ld_document_save_to_file (const char *file_name, GError *error);

#if 0
/* ===== Data proposal ===================================================== */
typedef struct _LdDocumentPrivate LdDocumentPrivate;

/*
 * LdDocumentPrivate:
 * @objects: All the objects in the document.
 * @selection: All currently selected objects.
 */
struct _LdDocumentPrivate
{
	GSList *objects;
	GSList *selection;
};

/* ===== Interface proposal ================================================ */
/* The contents of the document have changed. */
signal document-changed (...)

/* Add a symbol to the document at specified coordinates. */
/* TODO: Should the coordinates be double or int? */
void
ld_document_add_symbol (LdSymbol *symbol, x, y);

/* Parse a document in JSON and insert it into the document. */
gboolean
ld_document_insert_json (LdDocument *self, GError *error);

/* TODO: Create an interface for a list of this object: */
/* NOTE: In the future, labels will be also supported. */
LdDocumentSymbol

/* TODO: Create an interface for wires between pins of various symbols. */

/* TODO: Create an interface for object selection. */
ld_document_selection_...

gchar *
ld_document_selection_get_json (LdDocument *self);

#endif /* 0 */


G_END_DECLS

#endif /* ! __LD_DOCUMENT_H__ */

