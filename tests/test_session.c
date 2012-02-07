/* See LICENSE file for license and copyright information */

#include <check.h>

#include "../session.h"

START_TEST(test_basic) {
  girara_session_t* session = girara_session_create();
  fail_unless(session != NULL, "Could not create session");
  girara_session_destroy(session);

  session = girara_session_create();
  fail_unless(session != NULL, "Could not create session");
  bool res = girara_session_init(session, NULL);
  _assert_cmpuint(res, ==, true);
  girara_session_destroy(session);
} END_TEST

Suite* suite_session()
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("Session");

  /* basic */
  tcase = tcase_create("basic");
  tcase_add_test(tcase, test_basic);
  suite_add_tcase(suite, tcase);

  return suite;
}
