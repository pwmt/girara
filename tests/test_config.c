/* See LICENSE file for license and copyright information */

#include <check.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "../session.h"
#include "../settings.h"
#include "../config.h"

START_TEST(test_config_parse) {
  girara_session_t* session = girara_session_create();
  fail_unless(session != NULL, "Failed to create girara session.", NULL);

  int default_val = 1;
  fail_unless(girara_setting_add(session, "test1", "default-string", STRING, false, NULL, NULL, NULL),
      "Failed to add setting 'test1'", NULL);
  fail_unless(girara_setting_add(session, "test2", &default_val, INT, false, NULL, NULL, NULL),
      "Failed to add setting 'test2'", NULL);

  char* filename = NULL;
  int fd = g_file_open_tmp(NULL, &filename, NULL);
  fail_unless(fd != -1 && filename != NULL, "Couldn't open temporary file.", NULL);
  GError* error = NULL;
  if (g_file_set_contents(filename,
        "set test1 config-string\n" \
        "set test2 2\n", -1, &error) == FALSE) {
    fail_unless(false, "Couldn't set content: %s", error->message, NULL);
    g_error_free(error);
  }
  girara_config_parse(session, filename);

  char* ptr = NULL;
  fail_unless(girara_setting_get(session, "test1", &ptr), "Failed to get setting 'test1'.", NULL);
  fail_unless(g_strcmp0(ptr, "config-string") == 0, "Value of 'test1' doesn't match (got: %s, expected: %s",
      ptr, "config-string", NULL);
  g_free(ptr);

  int real_val = 0;
  fail_unless(girara_setting_get(session, "test2", &real_val), "Failed to get setting 'test1'.", NULL);
  fail_unless(real_val == 2, "Value of 'test2' doesn't match (got: %d, expected: %d",
      real_val, 2, NULL);

  close(fd);
  fail_unless(g_remove(filename) == 0, "Failed to remove temporary file.", NULL);
  g_free(filename);
  girara_session_destroy(session);
} END_TEST

extern void setup(void);

Suite* suite_config()
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
