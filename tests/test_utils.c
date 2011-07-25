// See LICENSE file for license and copyright information

#define _BSD_SOURCE

#include <glib.h>
#include <tests.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include "girara-utils.h"
#include "girara-datastructures.h"
#include "helpers.h"

typedef struct
{
  gchar* name;
  gchar* dir;
} pwd_info_t;

static void
free_pwd_info(void* data)
{
  pwd_info_t* pwd = (pwd_info_t*) data;
  if (!pwd) {
    return;
  }

  g_free(pwd->name);
  g_free(pwd->dir);
  g_free(pwd);
}

static girara_list_t*
read_pwd_info(void)
{
  girara_list_t* list = girara_list_new();
  girara_list_set_free_function(list, &free_pwd_info);

  struct passwd* pw;
  errno = 0;
  while ((pw = getpwent()) != NULL) {
    pwd_info_t* pwdinfo = g_malloc0(sizeof(pwd_info_t));
    pwdinfo->name = g_strdup(pw->pw_name);
    pwdinfo->dir = g_strdup(pw->pw_dir);
    girara_list_append(list, pwdinfo);
  }
  g_assert_cmpint(errno, ==, 0);
  endpwent();

  return list;
}

void
test_utils_home_directory()
{
  const gchar* user = g_get_home_dir();
  gchar* oldenv = g_getenv("HOME") ? g_strdup(g_getenv("HOME")) : NULL;

  if (oldenv) {
    gchar* res = girara_get_home_directory(NULL);
    g_assert_cmpstr(res, ==, oldenv);
    g_free(res);
  }

  g_unsetenv("HOME");
  gchar* res = girara_get_home_directory(NULL);
  g_assert_cmpstr(res, ==, user);
  g_free(res);

  girara_list_t* list = read_pwd_info();
  girara_list_iterator_t* iter = girara_list_iterator(list);
  g_assert_cmpptr(iter, !=, NULL);
  while (girara_list_iterator_is_valid(iter))
  {
    pwd_info_t* pwdinfo = (pwd_info_t*) girara_list_iterator_data(iter);
    gchar* res = girara_get_home_directory(pwdinfo->name);
    g_assert_cmpstr(res, ==, pwdinfo->dir);
    g_free(res);
    girara_list_iterator_next(iter);
  }
  girara_list_iterator_free(iter);
  girara_list_free(list);

  g_setenv("HOME", "/home/test", TRUE);
  res = girara_get_home_directory(NULL);
  g_assert_cmpstr(res, ==, "/home/test");
  g_free(res);

  if (oldenv) {
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

  girara_list_t* list = read_pwd_info();
  girara_list_iterator_t* iter = girara_list_iterator(list);
  g_assert_cmpptr(iter, !=, NULL);
  while (girara_list_iterator_is_valid(iter))
  {
    pwd_info_t* pwdinfo = (pwd_info_t*) girara_list_iterator_data(iter);
    gchar* path = g_strdup_printf("~%s/test", pwdinfo->name);
    gchar* eres = g_build_filename(pwdinfo->dir, "test", NULL);

    gchar* res = girara_fix_path(path);
    g_assert_cmpstr(res, ==, eres);
    g_free(res);
    g_free(eres);
    g_free(path);
    girara_list_iterator_next(iter);
  }
  girara_list_iterator_free(iter);
  girara_list_free(list);
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

  if (oldenv) {
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
