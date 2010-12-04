/*
 * ld-document-symbol.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2010. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LD_DOCUMENT_SYMBOL_H__
#define __LD_DOCUMENT_SYMBOL_H__

G_BEGIN_DECLS


#define LD_TYPE_DOCUMENT_SYMBOL (ld_document_symbol_get_type ())
#define LD_DOCUMENT_SYMBOL(obj) (G_TYPE_CHECK_INSTANCE_CAST \
	((obj), LD_TYPE_DOCUMENT_SYMBOL, LdDocumentSymbol))
#define LD_DOCUMENT_SYMBOL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST \
	((klass), LD_TYPE_DOCUMENT_SYMBOL, LdDocumentSymbolClass))
#define LD_IS_DOCUMENT_SYMBOL(obj) (G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), LD_TYPE_DOCUMENT_SYMBOL))
#define LD_IS_DOCUMENT_SYMBOL_CLASS(klass) (G_TYPE_CHECK_INSTANCE_TYPE \
	((klass), LD_TYPE_DOCUMENT_SYMBOL))
#define LD_DOCUMENT_SYMBOL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS \
	((obj), LD_DOCUMENT_SYMBOL, LdDocumentSymbolClass))

typedef struct _LdDocumentSymbol LdDocumentSymbol;
typedef struct _LdDocumentSymbolPrivate LdDocumentSymbolPrivate;
typedef struct _LdDocumentSymbolClass LdDocumentSymbolClass;


/**
 * LdDocumentSymbol:
 */
struct _LdDocumentSymbol
{
/*< private >*/
	LdDocumentObject parent_instance;
	LdDocumentSymbolPrivate *priv;
};

/**
 * LdDocumentSymbolClass:
 */
struct _LdDocumentSymbolClass
{
/*< private >*/
	LdDocumentObjectClass parent_class;
};


GType ld_document_symbol_get_type (void) G_GNUC_CONST;

LdDocumentSymbol *ld_document_symbol_new (const gchar *klass);
const gchar *ld_document_symbol_get_class (LdDocumentSymbol *self);
void ld_document_symbol_set_class (LdDocumentSymbol *self, const gchar *klass);


G_END_DECLS

#endif /* ! __LD_DOCUMENT_SYMBOL_H__ */

