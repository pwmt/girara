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
