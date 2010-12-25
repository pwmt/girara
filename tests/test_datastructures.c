
#include <glib.h>
#include "girara-datastructures.h"

static unsigned int list_free_called = 0;

static void
list_free(void* data)
{
  g_assert_cmpuint(list_free_called, ==, 0);
  g_assert_cmpuint((unsigned int) data, ==, 0xDEAD);
  ++list_free_called;
}

void
test_datastructures_list()
{
  girara_list_t* list = girara_list_new();
  g_assert_cmpuint(girara_list_size(list), ==, 0);
  for (int i = 0; i != 10; ++i)
    girara_list_append(list, (void*)i);
  g_assert_cmpuint(girara_list_size(list), ==, 10);

  girara_list_iterator_t* iter = girara_list_iterator(list);
  g_assert_cmpuint(iter, !=, NULL);
  for (int i = 0; i != 10; ++i)
  {
    g_assert_cmpint((int)girara_list_iterator_data(iter), ==, i);
    if (i < 9)
      g_assert_cmpuint(girara_list_iterator_next(iter), !=, NULL);
    else
      g_assert_cmpuint(girara_list_iterator_next(iter), ==, NULL);
  }
  girara_list_iterator_free(iter);
  girara_list_free(list);

  list = girara_list_new();
  girara_list_set_free_function(list, list_free);
  girara_list_append(list, (void*)0xDEAD);
  girara_list_free(list);
  g_assert_cmpuint(list_free_called, ==, 1);
}

void
test_datastructures_node()
{
  girara_tree_node_t* root = girara_node_new("root");
  g_assert_cmpuint(girara_node_get_num_children(root), ==, 0);
  g_assert_cmpstr((char*)girara_node_get_data(root), ==, "root");
  girara_node_free(root);
}
