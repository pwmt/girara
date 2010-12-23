// See LICENSE file for license and copyright information

#define _BSD_SOURCE

#include <glib.h>
#include <tests.h>
#include <sys/types.h>
#include <pwd.h>
#include "girara-utils.h"

void
test_utils_home_directory()
{
  const gchar* user = g_get_home_dir();
  gchar* oldenv = g_getenv("HOME") ? g_strdup(g_getenv("HOME")) : NULL;

  if (oldenv)
  {
    gchar* res = girara_get_home_directory(NULL);
    g_assert_cmpstr(res, ==, oldenv);
    g_free(res);
  }
  
  g_unsetenv("HOME");
  gchar* res = girara_get_home_directory(NULL);
  g_assert_cmpstr(res, ==, user);
  g_free(res);

  struct passwd* pw;
  while ((pw = getpwent()) != NULL)
  {
    gchar* res = girara_get_home_directory(pw->pw_name);
    g_assert_cmpstr(res, ==, pw->pw_dir);
    g_free(res);
  }
  endpwent();

  g_setenv("HOME", "/home/test", TRUE);
  res = girara_get_home_directory(NULL);
  g_assert_cmpstr(res, ==, "/home/test");
  g_free(res);

  if (oldenv)
  {
    g_setenv("HOME", oldenv, TRUE);
    g_free(oldenv);
  }
}

void
test_utils_fix_path()
{
  gchar* res = girara_fix_path("test");
  g_assert_cmpstr(res, ==, "test");
  g_free(res);

  res = girara_fix_path("test/test");
  g_assert_cmpstr(res, ==, "test/test");
  g_free(res);

  /*
  res = girara_fix_path("test/./test");
  g_assert_cmpstr(res, ==, "test/test");
  g_free(res);

  res = girara_fix_path("test/../test");
  g_assert_cmpstr(res, ==, "test");
  g_free(res);
  */

  struct passwd* pw;
  while ((pw = getpwent()) != NULL)
  {
    gchar* path = g_strdup_printf("~%s/test", pw->pw_name);
    gchar* eres = g_build_filename(pw->pw_dir, "test", NULL);

    gchar* res = girara_fix_path(path);
    g_assert_cmpstr(res, ==, eres);
    g_free(res);
    g_free(eres);
    g_free(path);
  }
  endpwent();
}

static void
xdg_path_impl(girara_xdg_path_t path, const gchar* envvar,
    const gchar* expected)
{
  gchar* oldenv = g_getenv(envvar) ? g_strdup(g_getenv(envvar)) : NULL;

  g_unsetenv(envvar);
  gchar* res = girara_get_xdg_path(path);
  g_assert_cmpstr(res, ==, expected);
  g_free(res);

  g_setenv(envvar, "~/xdg", TRUE);
  gchar* ex = g_build_filename(g_get_home_dir(), "xdg", NULL);
  res = girara_get_xdg_path(path);
  g_assert_cmpstr(res, ==, expected);
  g_free(res);
  g_free(ex);

  g_setenv(envvar, "/home/test/xdg", TRUE);
  res = girara_get_xdg_path(path);
  g_assert_cmpstr(res, ==, expected);
  g_free(res);

  if (oldenv)
  {
    g_setenv(envvar, oldenv, TRUE);
    g_free(oldenv);
  }
}

void
test_utils_xdg_path()
{
  xdg_path_impl(XDG_CONFIG, "XDG_CONFIG_HOME", g_get_user_config_dir());
  xdg_path_impl(XDG_DATA, "XDG_DATA_HOME", g_get_user_data_dir());
}
