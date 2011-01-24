
#ifndef __ld_marshal_MARSHAL_H__
#define __ld_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:OBJECT,OBJECT (ld-marshal.list:1) */
extern void ld_marshal_VOID__OBJECT_OBJECT (GClosure     *closure,
                                            GValue       *return_value,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint,
                                            gpointer      marshal_data);

/* VOID:OBJECT,STRING (ld-marshal.list:2) */
extern void ld_marshal_VOID__OBJECT_STRING (GClosure     *closure,
                                            GValue       *return_value,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint,
                                            gpointer      marshal_data);

G_END_DECLS

#endif /* __ld_marshal_MARSHAL_H__ */

