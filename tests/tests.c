// See LICENSE file for license and copyright information

#include <glib.h>
#include "tests.h"

int main(int argc, char** argv)
{
  g_test_init(&argc, &argv, NULL);
  // utils tests
  g_test_add_func("/utils/home_directory", test_utils_home_directory);
  g_test_add_func("/utils/fix_path", test_utils_fix_path);
  g_test_add_func("/utils/xdg_path", test_utils_xdg_path);
  // datastructures tests
  g_test_add_func("/datastructures/list", test_datastructures_list);
  g_test_add_func("/datastructures/node", test_datastructures_node);
  return g_test_run();
}
