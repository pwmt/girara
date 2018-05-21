/* See LICENSE file for license and copyright information */

#include <gtk/gtk.h>
#include <stdlib.h>

#include "tests.h"

int run_suite(Suite* suite)
{
  SRunner* suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_NORMAL);
  const int number_failed = srunner_ntests_failed(suite_runner);
  srunner_free(suite_runner);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

void setup(void)
{
  gtk_init(NULL, NULL);
}
