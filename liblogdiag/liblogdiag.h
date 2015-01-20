/*
 * liblogdiag.h
 *
 * This file is a part of logdiag.
 * Copyright Přemysl Janouch 2011, 2012. All rights reserved.
 *
 * See the file LICENSE for licensing information.
 *
 */

#ifndef __LIBLOGDIAG_H__
#define __LIBLOGDIAG_H__

/* Whatever, I don't care, someone should at least appreciate my effort of
 * porting it to GTK+ 3 in the first place.  You make me work for free!
 */
#define GDK_VERSION_MIN_REQUIRED GDK_VERSION_3_8

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "ld-marshal.h"
#include "ld-types.h"

#include "ld-symbol.h"
#include "ld-category.h"
#include "ld-library.h"

#include "ld-undo-action.h"
#include "ld-diagram-object.h"
#include "ld-diagram-symbol.h"
#include "ld-diagram-connection.h"
#include "ld-diagram.h"

#include "ld-diagram-view.h"
#include "ld-category-view.h"
#include "ld-category-symbol-view.h"
#include "ld-category-tree-view.h"

#include "ld-lua.h"
#include "ld-lua-symbol.h"

#endif /* ! __LIBLOGDIAG_H__ */

