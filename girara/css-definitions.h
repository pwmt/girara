/* See LICENSE file for license and copyright information */

#ifndef GIRARA_CSS_DEFINITIONS_H
#define GIRARA_CSS_DEFINITIONS_H

#include <gtk/gtk.h>

#include "macros.h"

#if GTK_CHECK_VERSION(3,20,0)
#define DEFAULT_FONT "normal 9pt monospace"
#else
#define DEFAULT_FONT "monospace normal 9"
#endif

extern const char* CSS_TEMPLATE_PRE_3_20 GIRARA_HIDDEN;
extern const char* CSS_TEMPLATE_POST_3_20 GIRARA_HIDDEN;

#endif
