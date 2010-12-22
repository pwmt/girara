/* See LICENSE file for license and copyright information */

#define _BSD_SOURCE
#define _XOPEN_SOURCE 500

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include "girara-utils.h"

gchar*
girara_fix_path(const gchar* path)
{
  g_return_val_if_fail(path != NULL || !strlen(path), NULL);

  gchar* rpath = NULL;
  if (path[0] == '~')
  {
    int len = strlen(path);
    gchar* user = NULL;
    int idx = 1;
    if (len > 1 && path[1] != '/')
    {
      while (path[idx] && path[idx] != '/')
        ++idx;
      user = g_strndup(path + 1, idx - 2);
    }

    gchar* home_path = girara_get_home_directory(user);
    g_free(user);
    if (!home_path)
      return g_strdup(path);

    rpath = g_build_filename(home_path, path + idx, NULL);
    g_free(home_path);
  }
  else
    rpath = g_strdup(path);

/*
  size_t pm
#ifdef PATH_MAX
    = PATH_MAX;
#else
    = pathconf(path,_PC_PATH_MAX);
  if(pm <= 0)
    pm = 4096;
#endif

  gchar* file = g_malloc0(sizeof(gchar) * pm);
  if (!file)
  {
    g_free(rpath);
    return NULL;
  }

  gchar* res = realpath(rpath, file);
  g_free(rpath);
  if (!res)
  {
    g_free(file);
    return NULL;
  }

  return file;
  */
  return rpath;
}

gchar*
girara_get_home_directory(const gchar* user)
{
  if (!user || g_strcmp0(user, g_get_user_name()) == 0)
  {
    const gchar* homedir = g_getenv("HOME");
    return g_strdup(homedir ? homedir : g_get_home_dir());
  }

  struct passwd pwd;
  struct passwd* result;
  int bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (bufsize < 0)
    bufsize = 4096;

  gchar* buffer = g_malloc0(sizeof(gchar) * bufsize);
  if (!buffer)
    return NULL;
  
  getpwnam_r(user, &pwd, buffer, bufsize, &result);
  if (result == NULL)
  {
    g_free(buffer);
    return NULL;
  }

  gchar* dir = g_strdup(pwd.pw_dir);
  g_free(buffer);
  return dir;
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
