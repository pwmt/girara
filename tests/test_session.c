/* See LICENSE file for license and copyright information */

#include <session.h>
#include "helpers.h"

void
test_session_basic()
{
  // just create and destroy
  girara_session_t* session = girara_session_create();
  g_assert_cmpptr(session, !=, NULL);
  girara_session_destroy(session);

  session = girara_session_create();
  g_assert_cmpptr(session, !=, NULL);
  bool res = girara_session_init(session);
  g_assert_cmpuint(res, ==, true);
  girara_session_destroy(session);
}
