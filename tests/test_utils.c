// SPDX-License-Identifier: Zlib

#include <check.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"
#include "datastructures.h"
#include "tests.h"

static girara_list_t*
read_pwd_info(void)
{
  girara_list_t* list = girara_list_new();
  girara_list_set_free_function(list, g_free);

  struct passwd* pw;
  errno = 0;
  while ((pw = getpwent()) != NULL) {
    girara_list_append(list, g_strdup(pw->pw_name));
    errno = 0;
  }
  ck_assert_msg(errno == 0, "Non-zero errno :%d", errno);
  endpwent();

  return list;
}

START_TEST(test_home_directory) {
  const gchar* user = g_get_home_dir();
  const gchar* home = g_getenv("HOME");

  girara_list_t* list = read_pwd_info();
  for (size_t idx = 0; idx != girara_list_size(list); ++idx) {
    const char* username = girara_list_nth(list, idx);
    gchar* result        = girara_get_home_directory(username);
    if (!home || g_strcmp0(user, username) != 0) {
      ck_assert_msg(result != NULL && strlen(result) != 0, "Home directory is empty");
    }
    g_free(result);
  }
  girara_list_free(list);
} END_TEST

START_TEST(test_home_directory_get_HOME) {
  const gchar* home = g_getenv("HOME");

  if (home) {
    gchar* result = girara_get_home_directory(NULL);
    ck_assert_str_eq(result, home);
    g_free(result);
  }
} END_TEST

START_TEST(test_fix_path_basic) {
  gchar* result = girara_fix_path("/test");
  ck_assert_msg(g_strcmp0(result, "/test") == 0,
      "Fix path result does not match (got: %s, expected: %s)", result, "test");
  g_free(result);

  result = girara_fix_path("/test/test");
  ck_assert_msg(g_strcmp0(result, "/test/test") == 0,
      "Fix path result does not match (got: %s, expected: %s)", result, "test/test");
  g_free(result);

  result = girara_fix_path("test");
  ck_assert_msg(g_str_has_suffix(result, "/test") == TRUE,
      "Fix path result does not match (got: %s, expected: %s)", result, ".../test");
  g_free(result);
} END_TEST

START_TEST(test_fix_path_extended) {
  const gchar* user = g_get_home_dir();
  const gchar* home = g_getenv("HOME");

  girara_list_t* list = read_pwd_info();
  for (size_t idx = 0; idx != girara_list_size(list); ++idx) {
    const char* username = girara_list_nth(list, idx);
    gchar* path          = g_strdup_printf("~%s/test", username);
    gchar* result        = girara_fix_path(path);
    if (!home || g_strcmp0(user, username) != 0) {
      ck_assert_msg(result != NULL && strlen(result) != 0, "Fix path result is empty");
    }
    g_free(result);
    g_free(path);
  }
  girara_list_free(list);
} END_TEST

static void
xdg_path_impl(girara_xdg_path_t path, const gchar* envvar,
    const gchar* expected)
{
  const char* xdg_test_helper_path = g_getenv("XDG_TEST_HELPER_PATH");
  ck_assert_msg(xdg_test_helper_path != NULL, "XDG_TEST_HELPER_PATH is not set");

  gchar** envp = g_get_environ();

  envp = g_environ_setenv(envp, envvar, "", TRUE);
  gchar* argv[] = { g_build_filename(xdg_test_helper_path, "xdg_test_helper", NULL), g_strdup_printf("%d", path), NULL };

  gchar* output = NULL;
  bool result = g_spawn_sync(NULL, argv, envp, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &output, NULL, NULL, NULL);
  g_assert(result);
  g_assert(output);
  ck_assert_msg(g_strcmp0(output, expected) == 0, "Output is not the same (got: %s, expected: %s)",
      output, expected);
  g_free(output);

  envp = g_environ_setenv(envp, envvar, "~/xdg", TRUE);

  result = g_spawn_sync(NULL, argv, envp, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &output, NULL, NULL, NULL);
  g_assert(result);
  g_assert(output);
  ck_assert_msg(g_strcmp0(output, "~/xdg") == 0, "Output is not the same (got: %s, expected: %s)",
      output, "~/xdg");

  envp = g_environ_setenv(envp, envvar, "/home/test/xdg", TRUE);

  result= g_spawn_sync(NULL, argv, envp, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &output, NULL, NULL, NULL);
  g_assert(result);
  g_assert(output);
  ck_assert_msg(g_strcmp0(output, "/home/test/xdg") == 0, "Output is not the same (got: %s, expected: %s)",
      output, "/home/test/xdg");

  g_free(argv[0]);
  g_free(argv[1]);
  g_strfreev(envp);
}

START_TEST(test_xdg_path) {
  xdg_path_impl(XDG_CONFIG,      "XDG_CONFIG_HOME", g_get_user_config_dir());
  xdg_path_impl(XDG_DATA,        "XDG_DATA_HOME",   g_get_user_data_dir());
  xdg_path_impl(XDG_CONFIG_DIRS, "XDG_CONFIG_DIRS", "/etc/xdg");
  xdg_path_impl(XDG_DATA_DIRS,   "XDG_DATA_DIRS",   "/usr/local/share/:/usr/share");
  xdg_path_impl(XDG_CACHE,       "XDG_CACHE_HOME",  g_get_user_cache_dir());
} END_TEST

START_TEST(test_file_invariants) {
  ck_assert_ptr_null(girara_file_open(NULL, NULL));
  ck_assert_ptr_null(girara_file_open("somefile", NULL));
  ck_assert_ptr_null(girara_file_open(NULL, "r"));

  ck_assert_ptr_null(girara_file_read_line(NULL));
  ck_assert_ptr_null(girara_file_read(NULL));
} END_TEST

START_TEST(test_file_read) {
  static const char CONTENT[] = "test1\ntest2\ntest3";
  static const char* LINES[] = { "test1", "test2", "test3" };
  static size_t NUMLINES = 3;

  gchar* path = NULL;
  int fd = g_file_open_tmp("girara.test.XXXXXX", &path, NULL);
  ck_assert_msg(fd != -1, "Failed to open temporary file.");
  ck_assert_msg(g_strcmp0(path, "") != 0, "Failed to open temporary file.");

  GError* error = NULL;
  if (g_file_set_contents(path, CONTENT, -1, &error) == FALSE) {
    ck_abort_msg("Couldn't set content: %s", error->message);
    g_error_free(error);
  }

  char* content = girara_file_read(path);
  ck_assert_msg(g_strcmp0(content, CONTENT) == 0, "Reading file failed");
  free(content);

  FILE* file = girara_file_open(path, "r");
  ck_assert_ptr_nonnull(file);
  for (size_t i = 0; i != NUMLINES; ++i) {
    char* line = girara_file_read_line(file);
    ck_assert_msg(g_strcmp0(line, LINES[i]) == 0, "Line doesn't match (got: %s, expected: %s)",
        line, LINES[i]);
    g_free(line);
  }
  fclose(file);

  close(fd);
  ck_assert_msg(g_remove(path) == 0, "Failed to remove temporary file.");
  g_free(path);
} END_TEST

START_TEST(test_split_path) {
  ck_assert_ptr_null(girara_split_path_array(NULL));
  ck_assert_ptr_null(girara_split_path_array(""));

  girara_list_t* res = girara_split_path_array("one/path");
  ck_assert_ptr_nonnull(res);
  ck_assert_uint_eq(girara_list_size(res), 1);
  ck_assert_str_eq(girara_list_nth(res, 0), "one/path");
  girara_list_free(res);

  res = girara_split_path_array("first/path:second/path");
  ck_assert_ptr_nonnull(res);
  ck_assert_uint_eq(girara_list_size(res), 2);
  ck_assert_str_eq(girara_list_nth(res, 0), "first/path");
  ck_assert_str_eq(girara_list_nth(res, 1), "second/path");
  girara_list_free(res);
} END_TEST

START_TEST(test_strings_replace_substrings_invalid) {
  ck_assert_ptr_null(girara_replace_substring(NULL, NULL, NULL));
  ck_assert_ptr_null(girara_replace_substring("", NULL, NULL));
  ck_assert_ptr_null(girara_replace_substring("", "", NULL));
} END_TEST

START_TEST(test_strings_replace_substrings_nothing_to_replace) {
  char* result = girara_replace_substring("test", "n", "y");
  ck_assert_ptr_nonnull(result);
  ck_assert_int_eq(strncmp(result, "test", 5), 0);
  g_free(result);
} END_TEST

START_TEST(test_strings_replace_substrings_1) {
  char* result = girara_replace_substring("test", "e", "f");
  ck_assert_ptr_nonnull(result);
  ck_assert_int_eq(strncmp(result, "tfst", 5), 0);
  g_free(result);
} END_TEST

START_TEST(test_strings_replace_substrings_2) {
  char* result = girara_replace_substring("test", "es", "f");
  ck_assert_ptr_nonnull(result);
  ck_assert_int_eq(strncmp(result, "tft", 4), 0);
  g_free(result);
} END_TEST

START_TEST(test_strings_replace_substrings_3) {
  char* result = girara_replace_substring("test", "e", "fg");
  ck_assert_ptr_nonnull(result);
  ck_assert_int_eq(strncmp(result, "tfgst", 6), 0);
  g_free(result);
} END_TEST

static Suite* suite_utils(void)
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("Utils");

  /* home directory */
  tcase = tcase_create("home_directory");
  tcase_add_test(tcase, test_home_directory);
  tcase_add_test(tcase, test_home_directory_get_HOME);
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

  /* split path */
  tcase = tcase_create("split_path");
  tcase_add_test(tcase, test_split_path);
  suite_add_tcase(suite, tcase);

  /* strings */
  tcase = tcase_create("strings");
  tcase_add_test(tcase, test_strings_replace_substrings_invalid);
  tcase_add_test(tcase, test_strings_replace_substrings_nothing_to_replace);
  tcase_add_test(tcase, test_strings_replace_substrings_1);
  tcase_add_test(tcase, test_strings_replace_substrings_2);
  tcase_add_test(tcase, test_strings_replace_substrings_3);
  suite_add_tcase(suite, tcase);

  return suite;
}

int main()
{
  return run_suite(suite_utils());
}
