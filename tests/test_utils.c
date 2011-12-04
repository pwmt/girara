// See LICENSE file for license and copyright information

#define _BSD_SOURCE
#define _POSIX_SOURCE

#include <glib.h>
#include <glib/gstdio.h>
#include "tests.h"
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <utils.h>
#include <datastructures.h>
#include "helpers.h"
#include <stdlib.h>
#include <stdio.h>

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
test_utils_home_directory(void)
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
test_utils_fix_path(void)
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
  GIRARA_LIST_FOREACH(list, pwd_info_t*, iter, pwdinfo)
    gchar* path = g_strdup_printf("~%s/test", pwdinfo->name);
    gchar* eres = g_build_filename(pwdinfo->dir, "test", NULL);

    gchar* res = girara_fix_path(path);
    g_assert_cmpstr(res, ==, eres);
    g_free(res);
    g_free(eres);
    g_free(path);
  GIRARA_LIST_FOREACH_END(list, pwd_info_t*, iter, pwdinfo);
  girara_list_free(list);
}

static void
xdg_path_impl(girara_xdg_path_t path, const gchar* envvar,
    const gchar* expected)
{
  gchar* envp[] = { g_strdup_printf("%s=", envvar) , NULL };
  gchar* argv[] = { "./xdg_test_helper", g_strdup_printf("%d", path), NULL };

  gchar* output = NULL;
  bool res = g_spawn_sync(NULL, argv, envp, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &output, NULL, NULL, NULL);
  g_assert(res);
  g_assert(output);
  g_assert_cmpstr(output, ==, expected);
  g_free(output);

  g_free(envp[0]);
  envp[0] = g_strdup_printf("%s=~/xdg", envvar);

  res = g_spawn_sync(NULL, argv, envp, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &output, NULL, NULL, NULL);
  g_assert(res);
  g_assert(output);
  g_assert_cmpstr(output, ==, "~/xdg");

  g_free(envp[0]);
  envp[0] = g_strdup_printf("%s=/home/test/xdg", envvar);

  res = g_spawn_sync(NULL, argv, envp, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &output, NULL, NULL, NULL);
  g_assert(res);
  g_assert(output);
  g_assert_cmpstr(output, ==, "/home/test/xdg");

  g_free(argv[1]);
}

void
test_utils_xdg_path(void)
{
  xdg_path_impl(XDG_CONFIG, "XDG_CONFIG_HOME", g_get_user_config_dir());
  xdg_path_impl(XDG_DATA, "XDG_DATA_HOME", g_get_user_data_dir());
  xdg_path_impl(XDG_CONFIG_DIRS, "XDG_CONFIG_DIRS", "/etc/xdg");
  xdg_path_impl(XDG_DATA_DIRS, "XDG_DATA_DIRS", "/usr/local/share/:/usr/share");
}

void
test_utils_file_invariants(void)
{
  g_assert_cmpptr(girara_file_open(NULL, NULL), ==, NULL);
  g_assert_cmpptr(girara_file_open("somefile", NULL), ==, NULL);
  g_assert_cmpptr(girara_file_open(NULL, "r"), ==, NULL);

  g_assert_cmpptr(girara_file_read_line(NULL), ==, NULL);
  g_assert_cmpptr(girara_file_read(NULL), ==, NULL);
}

#include <unistd.h>

void
test_utils_file_read(void)
{
  static const char CONTENT[] = "test1\ntest2\ntest3";
  static const char* LINES[] = { "test1", "test2", "test3" };
  static size_t NUMLINES = 3;

  gchar* path = NULL;
  int fd = g_file_open_tmp("girara.test.XXXXXX", &path, NULL);
  g_assert_cmpint(fd, !=, -1);
  g_assert_cmpstr(path, !=, "");
  close(fd);

  g_assert(g_file_set_contents(path, CONTENT, -1, NULL));

  char* content = girara_file_read(path);
  g_assert_cmpstr(content, ==, CONTENT);
  free(content);

  FILE* file = girara_file_open(path, "r");
  g_assert_cmpptr(file, !=, NULL);
  for (size_t i = 0; i != NUMLINES; ++i) {
    char* line = girara_file_read_line(file);
    g_assert_cmpstr(line, ==, LINES[i]);
    free(line);
  }
  fclose(file);

  g_assert_cmpint(g_remove(path), ==, 0);
  g_free(path);
}

static gboolean
ignore_criticals(const gchar* GIRARA_UNUSED(log_domain),
    GLogLevelFlags log_level, const gchar* GIRARA_UNUSED(message),
    gpointer GIRARA_UNUSED(user_data))
{
  return (log_level & G_LOG_LEVEL_MASK) != G_LOG_LEVEL_CRITICAL;
}

void
test_utils_safe_realloc(void)
{
  g_test_log_set_fatal_handler(ignore_criticals, NULL);

  g_assert_cmpptr(girara_safe_realloc(NULL, 0u), ==, NULL);

  void* ptr = NULL;
  g_assert_cmpptr(girara_safe_realloc(&ptr, sizeof(int)), !=, NULL);
  g_assert_cmpptr(ptr, !=, NULL);
  g_assert_cmpptr(girara_safe_realloc(&ptr, 1024*sizeof(int)), !=, NULL);
  g_assert_cmpptr(ptr, !=, NULL);
  g_assert_cmpptr(girara_safe_realloc(&ptr, 0u), ==, NULL);
  g_assert_cmpptr(ptr, ==, NULL);
}
