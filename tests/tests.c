/* See LICENSE file for license and copyright information */

#include <check.h>
#include <gtk/gtk.h>

Suite* suite_utils();
Suite* suite_datastructures();
Suite* suite_settings();
Suite* suite_session();
Suite* suite_config();

int main(int argc, char *argv[])
{
  Suite* suite          = NULL;
  SRunner* suite_runner = NULL;

  /* test utils */
  suite        = suite_utils();
  suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_NORMAL);
  srunner_free(suite_runner);

  /* test datastructures */
  suite        = suite_datastructures();
  suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_NORMAL);
  srunner_free(suite_runner);

  /* test settings */
  suite        = suite_settings();
  suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_NORMAL);
  srunner_free(suite_runner);

  /* test config */
  suite        = suite_config();
  suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_NORMAL);
  srunner_free(suite_runner);


  /* test session */
  gtk_init(&argc, &argv);
  suite        = suite_session();
  suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_NORMAL);
  srunner_free(suite_runner);

  return 0;
}
