/* See LICENSE file for license and copyright information */

#include <check.h>

#include "session.h"
#include "settings.h"
#include "tests.h"

START_TEST(test_settings_basic) {
  girara_session_t* session = girara_session_create();
  fail_unless(session != NULL, "Couldn't create session.", NULL);

  fail_unless(girara_setting_add(session, "test", NULL, STRING, false, NULL, NULL, NULL), NULL);
  char* ptr = NULL;
  fail_unless(girara_setting_get(session, "test", &ptr), NULL);
  fail_unless(ptr == NULL, NULL);

  fail_unless(girara_setting_set(session, "test", "value"), NULL);
  fail_unless(girara_setting_get(session, "test", &ptr), NULL);
  fail_unless(g_strcmp0(ptr, "value") == 0, NULL);
  g_free(ptr);

  ptr = NULL;
  fail_unless(!girara_setting_get(session, "does-not-exist", &ptr), NULL);
  fail_unless(ptr == NULL, NULL);

  fail_unless(girara_setting_add(session, "test2", "value", STRING, false, NULL, NULL, NULL), NULL);
  fail_unless(girara_setting_get(session, "test2", &ptr), NULL);
  fail_unless(g_strcmp0(ptr, "value") == 0, NULL);
  g_free(ptr);

  ptr = NULL;
  fail_unless(!girara_setting_add(session, "test3", NULL, INT, false, NULL, NULL, NULL), NULL);
  fail_unless(!girara_setting_get(session, "test3", &ptr), NULL);
  fail_unless(ptr == NULL, NULL);

  float val = 0.0, rval = 0.0;
  fail_unless(girara_setting_add(session, "test4", &val, FLOAT, false, NULL, NULL, NULL), NULL);
  fail_unless(girara_setting_get(session, "test4", &rval), NULL);
  fail_unless(val == rval, NULL);

  girara_session_destroy(session);
} END_TEST

static int callback_called = 0;

static void
setting_callback(girara_session_t* session, const char* name, girara_setting_type_t type, void* value, void* data)
{
  fail_unless(callback_called == 0, NULL);
  fail_unless(session != NULL, NULL);
  fail_unless(g_strcmp0(name, "test") == 0, NULL);
  fail_unless(type == STRING, NULL);
  fail_unless(g_strcmp0(value, "value") == 0, NULL);
  fail_unless(g_strcmp0(data, "data") == 0, NULL);
  callback_called++;
}

START_TEST(test_settings_callback) {
  girara_session_t* session = girara_session_create();
  fail_unless(session != NULL, NULL);

  fail_unless(girara_setting_add(session, "test", "oldvalue", STRING, false, NULL, setting_callback, "data"), NULL);
  fail_unless(girara_setting_set(session, "test", "value"), NULL);
  fail_unless(callback_called == 1, NULL);

  girara_session_destroy(session);
} END_TEST

Suite* suite_settings(void)
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("Settings");

  /* basic */
  tcase = tcase_create("basic");
  tcase_add_checked_fixture(tcase, setup, NULL);
  tcase_add_test(tcase, test_settings_basic);
  suite_add_tcase(suite, tcase);

  /* callback */
  tcase = tcase_create("callback");
  tcase_add_checked_fixture(tcase, setup, NULL);
  suite_add_tcase(suite, tcase);
  tcase_add_test(tcase, test_settings_callback);

  return suite;
}

int main()
{
  Suite* suite          = NULL;
  SRunner* suite_runner = NULL;
  int number_failed     = 0;

  /* test settings */
  suite        = suite_settings();
  suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_NORMAL);
  number_failed += srunner_ntests_failed(suite_runner);
  srunner_free(suite_runner);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
