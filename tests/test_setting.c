/* See LICENSE file for license and copyright information */

#include <session.h>
#include <settings.h>
#include "helpers.h"

void
test_settings_basic(void)
{
  girara_session_t* session = girara_session_create();
  g_assert_cmpptr(session, !=, NULL);
  
  g_assert(girara_setting_add(session, "test", NULL, STRING, false, NULL, NULL, NULL));
  char* ptr = girara_setting_get(session, "test");
  g_assert_cmpptr(ptr, ==, NULL);

  g_assert(girara_setting_set(session, "test", "value"));
  ptr = girara_setting_get(session, "test");
  g_assert_cmpstr(ptr, ==, "value");
  g_free(ptr);

  ptr = girara_setting_get(session, "does-not-exist");
  g_assert_cmpptr(ptr, ==, NULL);

  g_assert(girara_setting_add(session, "test2", "value", STRING, false, NULL, NULL, NULL));
  ptr = girara_setting_get(session, "test2");
  g_assert_cmpstr(ptr, ==, "value");

  girara_session_destroy(session);
}

static int callback_called = 0;

static void
setting_callback(girara_session_t* session, const char* name, girara_setting_type_t type, void* value, void* data)
{
  g_assert_cmpint(callback_called, ==, 0);
  g_assert_cmpptr(session, !=, NULL);
  g_assert_cmpstr(name, ==, "test");
  g_assert_cmpint(type, ==, STRING);
  g_assert_cmpstr(value, ==, "value");
  g_assert_cmpstr(data, ==, "data");
  callback_called++;
}

void
test_settings_callback(void)
{
  girara_session_t* session = girara_session_create();
  g_assert_cmpptr(session, !=, NULL);

  g_assert(girara_setting_add(session, "test", "oldvalue", STRING, false, NULL, setting_callback, "data"));
  g_assert(girara_setting_set(session, "test", "value"));
  g_assert_cmpint(callback_called, ==, 1);

  girara_session_destroy(session);
}
