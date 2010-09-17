/*
 * document.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __DOCUMENT_H__
#define __DOCUMENT_H__

G_BEGIN_DECLS


#define LOGDIAG_TYPE_DOCUMENT (logdiag_symbol_library_get_type ())
#define LOGDIAG_DOCUMENT(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LOGDIAG_TYPE_DOCUMENT, LogdiagDocument))
#define LOGDIAG_DOCUMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LOGDIAG_TYPE_DOCUMENT, LogdiagDocumentClass))
#define LOGDIAG_IS_DOCUMENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LOGDIAG_TYPE_DOCUMENT))
#define LOGDIAG_IS_DOCUMENT_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LOGDIAG_TYPE_DOCUMENT))
#define LOGDIAG_DOCUMENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LOGDIAG_DOCUMENT, LogdiagDocumentClass))

typedef struct _LogdiagDocument LogdiagDocument;
/*typedef struct _LogdiagDocumentPrivate LogdiagDocumentPrivate;*/
typedef struct _LogdiagDocumentClass LogdiagDocumentClass;


/**
 * LogdiagDocument:
 */
struct _LogdiagDocument
{
/*< private >*/
	GObject parent_instance;

/*< public >*/
};

struct _LogdiagDocumentClass
{
	GObjectClass parent_class;
};


GType logdiag_document_get_type (void) G_GNUC_CONST;

LogdiagDocument *logdiag_document_new (void);
gboolean logdiag_document_new_from_file (const char *file_name, GError *error);
gboolean logdiag_document_save_to_file (const char *file_name, GError *error);

#if 0
/*
 * LogdiagDocumentPrivate:
 * @objects: All the objects in the document.
 */
struct _LogdiagDocumentPrivate
{
	GSList *objects;
};

/** The contents of the document have changed. */
signal documentChanged (...);

/* TODO: A list of objects: */
LogdiagDocumentSymbol
LogdiagDocumentLabel

logdiag_document_add_symbol (LogdiagSymbol *symbol, x, y);

/* XXX: Separated lists of objects
 *      or a single list for all objects?
 */
/* TODO: Wires. */
logdiag_document_selection_...
logdiag_document_selection_get_json (LogdiagDocument *self);
logdiag_document_insert_json (LogdiagDocument *self);
/** Go back or forward in the history of changes. */
/* TODO: An interface that informs about the history. */
logdiag_document_history_go (LogdiagDocument *self);
#endif /* 0 */


G_END_DECLS

#endif /* ! __DOCUMENT_H__ */
