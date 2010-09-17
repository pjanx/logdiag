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


#define LD_TYPE_DOCUMENT (ld_symbol_library_get_type ())
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
/*typedef struct _LdDocumentPrivate LdDocumentPrivate;*/
typedef struct _LdDocumentClass LdDocumentClass;


/**
 * LdDocument:
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
/*
 * LdDocumentPrivate:
 * @objects: All the objects in the document.
 */
struct _LdDocumentPrivate
{
	GSList *objects;
};

/** The contents of the document have changed. */
signal documentChanged (...);

/* TODO: A list of objects: */
LdDocumentSymbol
LdDocumentLabel

ld_document_add_symbol (LdSymbol *symbol, x, y);

/* XXX: Separated lists of objects
 *      or a single list for all objects?
 */
/* TODO: Wires. */
ld_document_selection_...
ld_document_selection_get_json (LdDocument *self);
ld_document_insert_json (LdDocument *self);
/** Go back or forward in the history of changes. */
/* TODO: An interface that informs about the history. */
ld_document_history_go (LdDocument *self);
#endif /* 0 */


G_END_DECLS

#endif /* ! __LD_DOCUMENT_H__ */

