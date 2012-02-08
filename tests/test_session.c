/* See LICENSE file for license and copyright information */

#include <check.h>

#include "../session.h"

START_TEST(test_create) {
  girara_session_t* session = girara_session_create();
  fail_unless(session != NULL, "Could not create session");
  girara_session_destroy(session);
} END_TEST

START_TEST(test_init) {
  girara_session_t* session = girara_session_create();
  fail_unless(session != NULL, "Could not create session");
  bool result = girara_session_init(session, NULL);
  fail_unless(result == true, "Could not init session");
  girara_session_destroy(session);
} END_TEST

Suite* suite_session()
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("Session");

  /* basic */
  tcase = tcase_create("basic");
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_init);
  suite_add_tcase(suite, tcase);

  return suite;
}
