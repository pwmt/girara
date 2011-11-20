// See LICENSE file for license and copyright information

#include <gtk/gtk.h>
#include <glib.h>
#include "tests.h"

int main(int argc, char** argv)
{
  g_test_init(&argc, &argv, NULL);
  // utils tests
  g_test_add_func("/utils/path/home", test_utils_home_directory);
  g_test_add_func("/utils/path/fix", test_utils_fix_path);
  g_test_add_func("/utils/path/xdg", test_utils_xdg_path);
  g_test_add_func("/utils/file/invariants", test_utils_file_invariants);

  // datastructures tests
  g_test_add_func("/datastructures/list", test_datastructures_list);
  g_test_add_func("/datastructures/list/free", test_datastructures_list_free);
  g_test_add_func("/datastructures/list/sorted", test_datastructures_sorted_list);
  g_test_add_func("/datastructures/node", test_datastructures_node);

  // session tests
  // we need GTK+ from here onwards
  gtk_init(&argc, &argv);
  g_test_add_func("/session/basic", test_session_basic);
  return g_test_run();
}
