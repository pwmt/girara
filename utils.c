/* See LICENSE file for license and copyright information */

#include "girara-utils.h"

gchar*
girara_fix_path(const gchar* path)
{
  if (!path)
    return NULL;

  if (path[0] == '~')
  {
    gchar* home_path = girara_get_home_directory(NULL);
    gchar* res = g_build_filename(home_path, path + 1, NULL);
    g_free(home_path);
    return res;
  }
  else
    return g_strdup(path);
}

gchar*
girara_get_home_directory(const gchar* user) {
  g_return_val_if_fail(user == NULL, NULL);

  const gchar* homedir = g_getenv("HOME");
  return g_strdup(homedir ? homedir : g_get_home_dir());
}

gchar*
girara_get_xdg_path(girara_xdg_path_t path)
{
  switch (path)
  {
    case XDG_DATA:
      return g_strdup(g_get_user_data_dir());
    case XDG_CONFIG:
      return g_strdup(g_get_user_config_dir());
  }

  return NULL;
}
