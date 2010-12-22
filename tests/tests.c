// See LICENSE file for license and copyright information

#include <glib.h>
#include "tests.h"

int main(int argc, char** argv)
{
  g_test_init(&argc, &argv, NULL);
  g_test_add_func("/utils/home_directory", test_utils_home_directory);
  g_test_add_func("/utils/fix_path", test_utils_fix_path);
  g_test_add_func("/utils/xdg_path", test_utils_xdg_path);
  return g_test_run();
}
