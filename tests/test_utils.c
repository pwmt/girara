// See LICENSE file for license and copyright information

#define _BSD_SOURCE
#define _POSIX_SOURCE

#include <check.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "../utils.h"
#include "../datastructures.h"

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
    errno = 0;
  }
  fail_unless(errno == 0, "Non-zero errno :%d", errno, NULL);
  endpwent();

  return list;
}

START_TEST(test_home_directory) {
  const gchar* user = g_get_home_dir();
  gchar* oldenv = g_getenv("HOME") ? g_strdup(g_getenv("HOME")) : NULL;

  if (oldenv) {
    gchar* result = girara_get_home_directory(NULL);
    fail_unless(result != oldenv, "Home directory is not the same", NULL);
    g_free(result);
  }

  g_unsetenv("HOME");
  gchar* result = girara_get_home_directory(NULL);
  fail_unless(result != user, "Home directory is not the same", NULL);
  g_free(result);

  girara_list_t* list = read_pwd_info();
  girara_list_iterator_t* iter = girara_list_iterator(list);
  fail_unless(iter != NULL, "Could not create iterator", NULL);
  while (girara_list_iterator_is_valid(iter))
  {
    pwd_info_t* pwdinfo = (pwd_info_t*) girara_list_iterator_data(iter);
    gchar* result = girara_get_home_directory(pwdinfo->name);
    fail_unless(result != pwdinfo->dir, "Home directory is not the same", NULL);
    g_free(result);
    girara_list_iterator_next(iter);
  }
  girara_list_iterator_free(iter);
  girara_list_free(list);

  g_setenv("HOME", "/home/test", TRUE);
  result = girara_get_home_directory(NULL);
  fail_unless(g_strcmp0(result, "/home/test") == 0, "Home directory is not the same", NULL);
  g_free(result);

  if (oldenv) {
    g_setenv("HOME", oldenv, TRUE);
    g_free(oldenv);
  }
} END_TEST

START_TEST(test_fix_path_basic) {
  gchar* result = girara_fix_path("test");
  fail_unless(g_strcmp0(result, "test") == 0,
      "Fix path result does not match (got: %s, expected: %s)", result, "test", NULL);
  g_free(result);

  result = girara_fix_path("test/test");
  fail_unless(g_strcmp0(result, "test/test") == 0,
      "Fix path result does not match (got: %s, expected: %s)", result, "test/test", NULL);
  g_free(result);
} END_TEST

START_TEST(test_fix_path_extended) {
  girara_list_t* list = read_pwd_info();
  GIRARA_LIST_FOREACH(list, pwd_info_t*, iter, pwdinfo)
    gchar* path = g_strdup_printf("~%s/test", pwdinfo->name);
    gchar* eres = g_build_filename(pwdinfo->dir, "test", NULL);

    gchar* result = girara_fix_path(path);
    fail_unless(g_strcmp0(result, eres) == 0,
        "Fix path result does not match (got: %s, expected %s)", result, eres, NULL);
    g_free(result);
    g_free(eres);
    g_free(path);
  GIRARA_LIST_FOREACH_END(list, pwd_info_t*, iter, pwdinfo);
  girara_list_free(list);
} END_TEST

static void
xdg_path_impl(girara_xdg_path_t path, const gchar* envvar,
    const gchar* expected)
{
  gchar* envp[] = { g_strdup_printf("%s=", envvar) , NULL };
  gchar* argv[] = { "./xdg_test_helper", g_strdup_printf("%d", path), NULL };

  gchar* output = NULL;
  bool result = g_spawn_sync(NULL, argv, envp, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &output, NULL, NULL, NULL);
  g_assert(result);
  g_assert(output);
  fail_unless(g_strcmp0(output, expected) == 0, "Output is not the same (got: %s, expected: %s)",
      output, expected, NULL);
  g_free(output);

  g_free(envp[0]);
  envp[0] = g_strdup_printf("%s=~/xdg", envvar);

  result = g_spawn_sync(NULL, argv, envp, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &output, NULL, NULL, NULL);
  g_assert(result);
  g_assert(output);
  fail_unless(g_strcmp0(output, "~/xdg") == 0, "Output is not the same (got: %s, expected: %s)",
      output, "~/xdg", NULL);

  g_free(envp[0]);
  envp[0] = g_strdup_printf("%s=/home/test/xdg", envvar);

  result= g_spawn_sync(NULL, argv, envp, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &output, NULL, NULL, NULL);
  g_assert(result);
  g_assert(output);
  fail_unless(g_strcmp0(output, "/home/test/xdg") == 0, "Output is not the same (got: %s, expected: %s)",
      output, "/home/test/xdg", NULL);

  g_free(argv[1]);
}

START_TEST(test_xdg_path) {
  xdg_path_impl(XDG_CONFIG,      "XDG_CONFIG_HOME", g_get_user_config_dir());
  xdg_path_impl(XDG_DATA,        "XDG_DATA_HOME",   g_get_user_data_dir());
  xdg_path_impl(XDG_CONFIG_DIRS, "XDG_CONFIG_DIRS", "/etc/xdg");
  xdg_path_impl(XDG_DATA_DIRS,   "XDG_DATA_DIRS",   "/usr/local/share/:/usr/share");
} END_TEST

START_TEST(test_file_invariants) {
  fail_unless(girara_file_open(NULL, NULL) == NULL, NULL);
  fail_unless(girara_file_open("somefile", NULL) == NULL, NULL);
  fail_unless(girara_file_open(NULL, "r") == NULL, NULL);

  fail_unless(girara_file_read_line(NULL) == NULL, NULL);
  fail_unless(girara_file_read(NULL) == NULL, NULL);
} END_TEST

START_TEST(test_file_read) {
  static const char CONTENT[] = "test1\ntest2\ntest3";
  static const char* LINES[] = { "test1", "test2", "test3" };
  static size_t NUMLINES = 3;

  gchar* path = NULL;
  int fd = g_file_open_tmp("girara.test.XXXXXX", &path, NULL);
  fail_unless(fd != -1, "Failed to open temporary file.", NULL);
  fail_unless(g_strcmp0(path, "") != 0, "Failed to open temporary file.", NULL);

  GError* error = NULL;
  if (g_file_set_contents(path, CONTENT, -1, &error) == FALSE) {
    fail_unless(false, "Couldn't set content: %s", error->message, NULL);
    g_error_free(error);
  }

  char* content = girara_file_read(path);
  fail_unless(g_strcmp0(content, CONTENT) == 0, "Reading file failed", NULL);
  free(content);

  FILE* file = girara_file_open(path, "r");
  fail_unless(file != NULL, NULL);
  for (size_t i = 0; i != NUMLINES; ++i) {
    char* line = girara_file_read_line(file);
    fail_unless(g_strcmp0(line, LINES[i]) == 0, "Line doesn't match (got: %s, expected: %s)",
        line, LINES[i], NULL);
    free(line);
  }
  fclose(file);

  close(fd);
  fail_unless(g_remove(path) == 0, "Failed to remove temporary file.", NULL);
  g_free(path);
} END_TEST

START_TEST(test_safe_realloc) {
  fail_unless(girara_safe_realloc(NULL, 0u) == NULL, NULL);

  void* ptr = NULL;
  fail_unless(girara_safe_realloc(&ptr, sizeof(int)) != NULL, NULL);
  fail_unless(ptr != NULL, NULL);
  fail_unless(girara_safe_realloc(&ptr, 1024*sizeof(int)) != NULL, NULL);
  fail_unless(ptr != NULL, NULL);
  fail_unless(girara_safe_realloc(&ptr, 0u) == NULL, NULL);
  fail_unless(ptr == NULL, NULL);
} END_TEST

START_TEST(test_split_path) {
  fail_unless(girara_split_path_array(NULL) == NULL, NULL);
  fail_unless(girara_split_path_array("") == NULL, NULL);

  girara_list_t* res = girara_split_path_array("one/path");
  fail_unless(res != NULL, NULL);
  fail_unless(girara_list_size(res) == 1, NULL);
  fail_unless(g_strcmp0(girara_list_nth(res, 0), "one/path") == 0, NULL);
  girara_list_free(res);

  res = girara_split_path_array("first/path:second/path");
  fail_unless(res != NULL, NULL);
  fail_unless(girara_list_size(res) == 2, NULL);
  fail_unless(g_strcmp0(girara_list_nth(res, 0), "first/path") == 0, NULL);
  fail_unless(g_strcmp0(girara_list_nth(res, 1), "second/path") == 0, NULL);
  girara_list_free(res);
} END_TEST

Suite* suite_utils()
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("Utils");

  /* home directory */
  tcase = tcase_create("home_directory");
  tcase_add_test(tcase, test_home_directory);
  suite_add_tcase(suite, tcase);

  /* fix path */
  tcase = tcase_create("fix_path");
  tcase_add_test(tcase, test_fix_path_basic);
  tcase_add_test(tcase, test_fix_path_extended);
  suite_add_tcase(suite, tcase);

  /* xdg path */
  tcase = tcase_create("xdg_path");
  tcase_add_test(tcase, test_xdg_path);
  suite_add_tcase(suite, tcase);

  /* file invariants */
  tcase = tcase_create("file_invariants");
  tcase_add_test(tcase, test_file_invariants);
  suite_add_tcase(suite, tcase);

  /* read file */
  tcase = tcase_create("file_read");
  tcase_add_test(tcase, test_file_read);
  suite_add_tcase(suite, tcase);

  /* safe realloc */
  tcase = tcase_create("safe_realloc");
  tcase_add_test(tcase, test_safe_realloc);
  suite_add_tcase(suite, tcase);

  /* split path */
  tcase = tcase_create("split_path");
  tcase_add_test(tcase, test_split_path);
  suite_add_tcase(suite, tcase);

  return suite;
}
