// SPDX-License-Identifier: Zlib

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

static girara_list_t* read_pwd_info(void) {
  girara_list_t* list = girara_list_new();
  girara_list_set_free_function(list, g_free);

  struct passwd* pw;
  errno = 0;
  while ((pw = getpwent()) != NULL) {
    girara_list_append(list, g_strdup(pw->pw_name));
    errno = 0;
  }
  g_assert_cmpint(errno, ==, 0);
  endpwent();

  return list;
}

static void test_home_directory(void) {
  const gchar* user = g_get_home_dir();
  const gchar* home = g_getenv("HOME");

  girara_list_t* list = read_pwd_info();
  for (size_t idx = 0; idx != girara_list_size(list); ++idx) {
    const char* username = girara_list_nth(list, idx);
    gchar* result        = girara_get_home_directory(username);
    if (!home || g_strcmp0(user, username) != 0) {
      g_assert_nonnull(result);
      g_assert_cmpuint(strlen(result), !=, 0);
    }
    g_free(result);
  }
  girara_list_free(list);
}

static void test_home_directory_get_HOME(void) {
  const gchar* home = g_getenv("HOME");

  if (home) {
    gchar* result = girara_get_home_directory(NULL);
    g_assert_cmpstr(result, ==, home);
    g_free(result);
  }
}

static void test_fix_path_basic(void) {
  gchar* result = girara_fix_path("/test");
  g_assert_cmpstr(result, ==, "/test");
  g_free(result);

  result = girara_fix_path("/test/test");
  g_assert_cmpstr(result, ==, "/test/test");
  g_free(result);

  result = girara_fix_path("test");
  g_assert_true(g_str_has_suffix(result, "/test"));
  g_free(result);
}

static void test_fix_path_extended(void) {
  const gchar* user = g_get_home_dir();
  const gchar* home = g_getenv("HOME");

  girara_list_t* list = read_pwd_info();
  for (size_t idx = 0; idx != girara_list_size(list); ++idx) {
    const char* username = girara_list_nth(list, idx);
    gchar* path          = g_strdup_printf("~%s/test", username);
    gchar* result        = girara_fix_path(path);
    if (!home || g_strcmp0(user, username) != 0) {
      g_assert_nonnull(result);
      g_assert_cmpuint(strlen(result), !=, 0);
    }
    g_free(result);
    g_free(path);
  }
  girara_list_free(list);
}

static void xdg_path_impl(girara_xdg_path_t path, const gchar* envvar, const gchar* expected) {
  const char* xdg_test_helper_path = g_getenv("G_TEST_BUILDDIR");
  g_assert_nonnull(xdg_test_helper_path);

  gchar** envp = g_get_environ();

  envp          = g_environ_setenv(envp, envvar, "", TRUE);
  gchar* argv[] = {g_build_filename(xdg_test_helper_path, "xdg_test_helper", NULL), g_strdup_printf("%d", path), NULL};

  gchar* output = NULL;
  bool result   = g_spawn_sync(NULL, argv, envp, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &output, NULL, NULL, NULL);
  g_assert(result);
  g_assert(output);
  g_assert_cmpstr(output, ==, expected);
  g_free(output);

  envp = g_environ_setenv(envp, envvar, "~/xdg", TRUE);

  result = g_spawn_sync(NULL, argv, envp, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &output, NULL, NULL, NULL);
  g_assert(result);
  g_assert(output);
  g_assert_cmpstr(output, ==, "~/xdg");

  envp = g_environ_setenv(envp, envvar, "/home/test/xdg", TRUE);

  result = g_spawn_sync(NULL, argv, envp, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &output, NULL, NULL, NULL);
  g_assert(result);
  g_assert(output);
  g_assert_cmpstr(output, ==, "/home/test/xdg");

  g_free(argv[0]);
  g_free(argv[1]);
  g_strfreev(envp);
}

static void test_xdg_path(void) {
  xdg_path_impl(XDG_CONFIG, "XDG_CONFIG_HOME", g_get_user_config_dir());
  xdg_path_impl(XDG_DATA, "XDG_DATA_HOME", g_get_user_data_dir());
  xdg_path_impl(XDG_CONFIG_DIRS, "XDG_CONFIG_DIRS", "/etc/xdg");
  xdg_path_impl(XDG_DATA_DIRS, "XDG_DATA_DIRS", "/usr/local/share/:/usr/share");
  xdg_path_impl(XDG_CACHE, "XDG_CACHE_HOME", g_get_user_cache_dir());
}

static void test_file_invariants(void) {
  g_assert_null(girara_file_open(NULL, NULL));
  g_assert_null(girara_file_open("somefile", NULL));
  g_assert_null(girara_file_open(NULL, "r"));

  g_assert_null(girara_file_read_line(NULL));
}

static void test_file_read(void) {
  static const char CONTENT[] = "test1\ntest2\ntest3";
  static const char* LINES[]  = {"test1", "test2", "test3"};
  static size_t NUMLINES      = 3;

  gchar* path = NULL;
  int fd      = g_file_open_tmp("girara.test.XXXXXX", &path, NULL);
  g_assert_cmpint(fd, !=, -1);
  g_assert_cmpstr(path, !=, "");

  if (g_file_set_contents(path, CONTENT, -1, NULL) == FALSE) {
    g_assert_not_reached();
  }

  FILE* file = girara_file_open(path, "r");
  g_assert_nonnull(file);
  for (size_t i = 0; i != NUMLINES; ++i) {
    char* line = girara_file_read_line(file);
    g_assert_cmpstr(line, ==, LINES[i]);
    g_free(line);
  }
  fclose(file);

  close(fd);
  g_assert_cmpint(g_remove(path), ==, 0);
  g_free(path);
}

static void test_split_path(void) {
  g_assert_null(girara_split_path_array(NULL));
  g_assert_null(girara_split_path_array(""));

  girara_list_t* res = girara_split_path_array("one/path");
  g_assert_nonnull(res);
  g_assert_cmpuint(girara_list_size(res), ==, 1);
  g_assert_cmpstr(girara_list_nth(res, 0), ==, "one/path");
  girara_list_free(res);

  res = girara_split_path_array("first/path:second/path");
  g_assert_nonnull(res);
  g_assert_cmpuint(girara_list_size(res), ==, 2);
  g_assert_cmpstr(girara_list_nth(res, 0), ==, "first/path");
  g_assert_cmpstr(girara_list_nth(res, 1), ==, "second/path");
  girara_list_free(res);
}

static void test_strings_replace_substrings_invalid(void) {
  g_assert_null(girara_replace_substring(NULL, NULL, NULL));
  g_assert_null(girara_replace_substring("", NULL, NULL));
  g_assert_null(girara_replace_substring("", "", NULL));
}

static void test_strings_replace_substrings_nothing_to_replace(void) {
  char* result = girara_replace_substring("test", "n", "y");
  g_assert_nonnull(result);
  g_assert_cmpstr(result, ==, "test");
  g_free(result);
}

static void test_strings_replace_substrings_1(void) {
  char* result = girara_replace_substring("test", "e", "f");
  g_assert_nonnull(result);
  g_assert_cmpstr(result, ==, "tfst");
  g_free(result);
}

static void test_strings_replace_substrings_2(void) {
  char* result = girara_replace_substring("test", "es", "f");
  g_assert_nonnull(result);
  g_assert_cmpstr(result, ==, "tft");
  g_free(result);
}

static void test_strings_replace_substrings_3(void) {
  char* result = girara_replace_substring("test", "e", "fg");
  g_assert_nonnull(result);
  g_assert_cmpstr(result, ==, "tfgst");
  g_free(result);
}

int main(int argc, char* argv[]) {
  g_test_init(&argc, &argv, NULL);
  g_test_add_func("/file/invariants", test_file_invariants);
  g_test_add_func("/file/read", test_file_read);
  g_test_add_func("/home/base", test_home_directory);
  g_test_add_func("/home/get_home", test_home_directory_get_HOME);
  g_test_add_func("/path/fix_basic", test_fix_path_basic);
  g_test_add_func("/path/fix_extend", test_fix_path_extended);
  g_test_add_func("/path/split", test_split_path);
  g_test_add_func("/string/replace_1", test_strings_replace_substrings_1);
  g_test_add_func("/string/replace_2", test_strings_replace_substrings_2);
  g_test_add_func("/string/replace_3", test_strings_replace_substrings_3);
  g_test_add_func("/string/replace_invalid", test_strings_replace_substrings_invalid);
  g_test_add_func("/string/replase_nothing", test_strings_replace_substrings_nothing_to_replace);
  g_test_add_func("/xdg_path/basic", test_xdg_path);
  return g_test_run();
}
