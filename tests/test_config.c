/* SPDX-License-Identifier: Zlib */

#include <check.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "session.h"
#include "settings.h"
#include "config.h"
#include "tests.h"

START_TEST(test_config_parse) {
  girara_session_t* session = girara_session_create();
  ck_assert_msg(session != NULL, "Failed to create girara session.");

  int default_val = 1;
  ck_assert_msg(girara_setting_add(session, "test1", "default-string", STRING, false, NULL, NULL, NULL),
      "Failed to add setting 'test1'");
  ck_assert_msg(girara_setting_add(session, "test2", &default_val, INT, false, NULL, NULL, NULL),
      "Failed to add setting 'test2'");

  char* filename = NULL;
  int fd = g_file_open_tmp(NULL, &filename, NULL);
  ck_assert_msg(fd != -1 && filename != NULL, "Couldn't open temporary file.");
  GError* error = NULL;
  if (g_file_set_contents(filename,
        "set test1 config-string\n" \
        "set test2 2\n", -1, &error) == FALSE) {
    ck_abort_msg("Couldn't set content: %s", error->message);
    g_error_free(error);
  }
  girara_config_parse(session, filename);

  char* ptr = NULL;
  ck_assert_msg(girara_setting_get(session, "test1", &ptr), "Failed to get setting 'test1'.");
  ck_assert_msg(g_strcmp0(ptr, "config-string") == 0, "Value of 'test1' doesn't match (got: %s, expected: %s",
      ptr, "config-string");
  g_free(ptr);

  int real_val = 0;
  ck_assert_msg(girara_setting_get(session, "test2", &real_val), "Failed to get setting 'test1'.");
  ck_assert_msg(real_val == 2, "Value of 'test2' doesn't match (got: %d, expected: %d",
      real_val, 2);

  close(fd);
  ck_assert_msg(g_remove(filename) == 0, "Failed to remove temporary file.");
  g_free(filename);
  girara_session_destroy(session);
} END_TEST

static Suite* suite_config(void)
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("Config");

  /* parse */
  tcase = tcase_create("parse");
  tcase_add_checked_fixture(tcase, setup, NULL);
  tcase_add_test(tcase, test_config_parse);
  suite_add_tcase(suite, tcase);

  return suite;
}

int main()
{
  return run_suite(suite_config());
}
