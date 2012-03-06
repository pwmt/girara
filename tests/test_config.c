/* See LICENSE file for license and copyright information */

#include <check.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "../session.h"
#include "../settings.h"
#include "../config.h"

START_TEST(test_config_parse) {
  girara_session_t* session = girara_session_create();
  fail_unless(session != NULL);

  int default_val = 1;
  fail_unless(girara_setting_add(session, "test1", "default-string", STRING, false, NULL, NULL, NULL));
  fail_unless(girara_setting_add(session, "test2", &default_val, INT, false, NULL, NULL, NULL));
  
  char* filename = NULL;
  int fd = g_file_open_tmp(NULL, &filename, NULL);
  fail_unless(fd != -1 && filename != NULL);
  fail_unless(g_file_set_contents(filename,
        "set test1 config-string\n" \
        "set test2 2\n", -1, NULL));
  girara_config_parse(session, filename);

  char* ptr = NULL;
  fail_unless(girara_setting_get(session, "test1", &ptr));
  fail_unless(g_strcmp0(ptr, "config-string") == 0);
  g_free(ptr);

  int real_val = 0;
  fail_unless(girara_setting_get(session, "test2", &real_val));
  fail_unless(real_val == 2);

  girara_session_destroy(session);
} END_TEST

Suite* suite_config()
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("Config");

  /* parse */
  tcase = tcase_create("parse");
  tcase_add_test(tcase, test_config_parse);
  suite_add_tcase(suite, tcase);

  return suite;
}
