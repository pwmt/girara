
#include <glib.h>
#include "girara-datastructures.h"

void test_datastructures_list()
{
  girara_list_t* list = girara_list_new();
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
}

void test_datastructures_node()
{}
