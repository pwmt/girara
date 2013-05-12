/* See LICENSE file for license and copyright information */

#include <check.h>
#include <gtk/gtk.h>
#include <stdlib.h>

Suite* suite_utils();
Suite* suite_datastructures();
Suite* suite_settings();
Suite* suite_session();
Suite* suite_config();

void setup(void)
{
  gtk_init(NULL, NULL);
}

int main()
{
  Suite* suite          = NULL;
  SRunner* suite_runner = NULL;
  int number_failed     = 0;

  /* test utils */
  suite        = suite_utils();
  suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_NORMAL);
  number_failed += srunner_ntests_failed(suite_runner);
  srunner_free(suite_runner);

  /* test datastructures */
  suite        = suite_datastructures();
  suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_NORMAL);
  number_failed += srunner_ntests_failed(suite_runner);
  srunner_free(suite_runner);

  /* test settings */
  suite        = suite_settings();
  suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_NORMAL);
  number_failed += srunner_ntests_failed(suite_runner);
  srunner_free(suite_runner);

  /* test config */
  suite        = suite_config();
  suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_NORMAL);
  number_failed += srunner_ntests_failed(suite_runner);
  srunner_free(suite_runner);

  /* test session */
  suite        = suite_session();
  suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_NORMAL);
  number_failed += srunner_ntests_failed(suite_runner);
  srunner_free(suite_runner);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
