/* See LICENSE file for license and copyright information */

#include <check.h>

#include "session.h"
#include "tests.h"

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

static Suite* suite_session(void)
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("Session");

  /* basic */
  tcase = tcase_create("basic");
  tcase_add_checked_fixture(tcase, setup, NULL);
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_init);
  suite_add_tcase(suite, tcase);

  return suite;
}

int main()
{
  Suite* suite          = NULL;
  SRunner* suite_runner = NULL;
  int number_failed     = 0;

  /* test session */
  suite        = suite_session();
  suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_NORMAL);
  number_failed += srunner_ntests_failed(suite_runner);
  srunner_free(suite_runner);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
