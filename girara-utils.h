/* See LICENSE file for license and copyright information */

#ifndef GIRARA_UTILS_H
#define GIRARA_UTILS_H

#include <gtk/gtk.h>

typedef enum {
  XDG_CONFIG,
  XDG_DATA
} girara_xdg_path_t;

gchar* girara_get_home_directory(const gchar* user);
gchar* girara_get_xdg_path(girara_xdg_path_t path);
gchar* girara_fix_path(const gchar* path);

#endif
