/* SPDX-License-Identifier: Zlib */

#include <check.h>

#include "session.h"
#include "settings.h"
#include "tests.h"

START_TEST(test_settings_basic) {
  girara_session_t* session = girara_session_create();
  ck_assert_msg(session != NULL, "Couldn't create session.");

  ck_assert(girara_setting_add(session, "test", NULL, STRING, false, NULL, NULL, NULL));
  char* ptr = NULL;
  ck_assert(girara_setting_get(session, "test", &ptr));
  ck_assert_ptr_null(ptr);

  ck_assert(girara_setting_set(session, "test", "value"));
  ck_assert(girara_setting_get(session, "test", &ptr));
  ck_assert_str_eq(ptr, "value");
  g_free(ptr);

  ptr = NULL;
  ck_assert(!girara_setting_get(session, "does-not-exist", &ptr));
  ck_assert_ptr_null(ptr);

  ck_assert(girara_setting_add(session, "test2", "value", STRING, false, NULL, NULL, NULL));
  ck_assert(girara_setting_get(session, "test2", &ptr));
  ck_assert_str_eq(ptr, "value");
  g_free(ptr);

  ptr = NULL;
  ck_assert(!girara_setting_add(session, "test3", NULL, INT, false, NULL, NULL, NULL));
  ck_assert(!girara_setting_get(session, "test3", &ptr));
  ck_assert_ptr_null(ptr);

  float val = 0.0, rval = 0.0;
  ck_assert(girara_setting_add(session, "test4", &val, FLOAT, false, NULL, NULL, NULL));
  ck_assert(girara_setting_get(session, "test4", &rval));
  ck_assert_float_eq(val, rval);

  girara_session_destroy(session);
} END_TEST

static int callback_called = 0;

static void
setting_callback(girara_session_t* session, const char* name, girara_setting_type_t type, const void* value, void* data)
{
  ck_assert_uint_eq(callback_called, 0);
  ck_assert_ptr_nonnull(session);
  ck_assert_str_eq(name, "test");
  ck_assert_uint_eq(type, STRING);
  ck_assert_str_eq(value, "value");
  ck_assert_str_eq(data, "data");
  callback_called++;
}

START_TEST(test_settings_callback) {
  girara_session_t* session = girara_session_create();
  ck_assert_ptr_nonnull(session);

  ck_assert(girara_setting_add(session, "test", "oldvalue", STRING, false, NULL, setting_callback, "data"));
  ck_assert(girara_setting_set(session, "test", "value"));
  ck_assert_uint_eq(callback_called, 1);

  girara_session_destroy(session);
} END_TEST

static Suite* suite_settings(void)
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
  return run_suite(suite_settings());
}
