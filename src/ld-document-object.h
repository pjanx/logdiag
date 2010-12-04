/*
 * ld-document-object.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_DOCUMENT_OBJECT_H__
#define __LD_DOCUMENT_OBJECT_H__

G_BEGIN_DECLS


#define LD_TYPE_DOCUMENT_OBJECT (ld_document_object_get_type ())
#define LD_DOCUMENT_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_DOCUMENT_OBJECT, LdDocumentObject))
#define LD_DOCUMENT_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_DOCUMENT_OBJECT, LdDocumentObjectClass))
#define LD_IS_DOCUMENT_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_DOCUMENT_OBJECT))
#define LD_IS_DOCUMENT_OBJECT_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_DOCUMENT_OBJECT))
#define LD_DOCUMENT_OBJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_DOCUMENT_OBJECT, LdDocumentObjectClass))

typedef struct _LdDocumentObject LdDocumentObject;
typedef struct _LdDocumentObjectPrivate LdDocumentObjectPrivate;
typedef struct _LdDocumentObjectClass LdDocumentObjectClass;


/**
 * LdDocumentObject:
 */
struct _LdDocumentObject
{
/*< private >*/
	GObject parent_instance;
	LdDocumentObjectPrivate *priv;
};

/**
 * LdDocumentObjectClass:
 */
struct _LdDocumentObjectClass
{
/*< private >*/
	GObjectClass parent_class;
};


GType ld_document_object_get_type (void) G_GNUC_CONST;

gdouble ld_document_object_get_x (LdDocumentObject *self);
gdouble ld_document_object_get_y (LdDocumentObject *self);
void ld_document_object_set_x (LdDocumentObject *self, gdouble x);
void ld_document_object_set_y (LdDocumentObject *self, gdouble y);


G_END_DECLS

#endif /* ! __LD_DOCUMENT_OBJECT_H__ */

