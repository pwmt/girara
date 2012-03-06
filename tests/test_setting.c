/* See LICENSE file for license and copyright information */

#include <check.h>

#include "../session.h"
#include "../settings.h"

START_TEST(test_settings_basic) {
  girara_session_t* session = girara_session_create();
  fail_unless(session != NULL);

  fail_unless(girara_setting_add(session, "test", NULL, STRING, false, NULL, NULL, NULL));
  char* ptr = NULL;
  fail_unless(girara_setting_get(session, "test", &ptr));
  fail_unless(ptr == NULL);

  fail_unless(girara_setting_set(session, "test", "value"));
  fail_unless(girara_setting_get(session, "test", &ptr));
  fail_unless(g_strcmp0(ptr, "value") == 0);
  g_free(ptr);

  ptr = NULL;
  fail_unless(!girara_setting_get(session, "does-not-exist", &ptr));
  fail_unless(ptr == NULL);

  fail_unless(girara_setting_add(session, "test2", "value", STRING, false, NULL, NULL, NULL));
  fail_unless(girara_setting_get(session, "test2", &ptr));
  fail_unless(g_strcmp0(ptr, "value") == 0);
  g_free(ptr);

  ptr = NULL;
  fail_unless(!girara_setting_add(session, "test3", NULL, INT, false, NULL, NULL, NULL));
  fail_unless(!girara_setting_get(session, "test3", &ptr));
  fail_unless(ptr == NULL);

  float val = 0.0, rval = 0.0;
  fail_unless(girara_setting_add(session, "test4", &val, FLOAT, false, NULL, NULL, NULL));
  fail_unless(girara_setting_get(session, "test4", &rval));
  fail_unless(val == rval);

  girara_session_destroy(session);
} END_TEST

static int callback_called = 0;

static void
setting_callback(girara_session_t* session, const char* name, girara_setting_type_t type, void* value, void* data)
{
  fail_unless(callback_called == 0);
  fail_unless(session != NULL);
  fail_unless(g_strcmp0(name, "test") == 0);
  fail_unless(type == STRING);
  fail_unless(g_strcmp0(value, "value") == 0);
  fail_unless(g_strcmp0(data, "data") == 0);
  callback_called++;
}

START_TEST(test_settings_callback) {
  girara_session_t* session = girara_session_create();
  fail_unless(session != NULL);

  fail_unless(girara_setting_add(session, "test", "oldvalue", STRING, false, NULL, setting_callback, "data"));
  fail_unless(girara_setting_set(session, "test", "value"));
  fail_unless(callback_called == 1);

  girara_session_destroy(session);
} END_TEST

Suite* suite_settings()
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("Settings");

  /* basic */
  tcase = tcase_create("basic");
  tcase_add_test(tcase, test_settings_basic);
  suite_add_tcase(suite, tcase);

  /* callback */
  tcase = tcase_create("callback");
  suite_add_tcase(suite, tcase);
  tcase_add_test(tcase, test_settings_callback);

  return suite;
}
