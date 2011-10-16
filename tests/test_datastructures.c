/* See LICENSE file for license and copyright information */

#include <glib.h>
#include <stdint.h>
#include "helpers.h"
#include "girara-datastructures.h"

static unsigned int list_free_called = 0;
static unsigned int node_free_called = 0;

static void
list_free(void* data)
{
  g_assert_cmpuint(list_free_called, ==, 0u);
  g_assert_cmpuint((intptr_t)data, ==, 0xDEAD);

  ++list_free_called;
}

void
test_datastructures_list()
{
  girara_list_t* list = girara_list_new();
  // size of empty list
  g_assert_cmpuint(girara_list_size(list), ==, 0);

  // append
  for (intptr_t i = 0; i != 10; ++i) {
    girara_list_append(list, (void*)i);
  }

  // size of list
  g_assert_cmpuint(girara_list_size(list), ==, 10);

  // iterator tests
  girara_list_iterator_t* iter = girara_list_iterator(list);
  g_assert_cmpptr(iter, !=, NULL);

  for (intptr_t i = 0; i != 10; ++i) {
    g_assert_cmpuint((intptr_t)girara_list_iterator_data(iter), ==, i);
    if (i < 9) {
      g_assert(girara_list_iterator_is_valid(iter));
      g_assert(girara_list_iterator_has_next(iter));
      g_assert(girara_list_iterator_next(iter));
      g_assert(girara_list_iterator_is_valid(iter));
    } else {
      g_assert(girara_list_iterator_is_valid(iter));
      g_assert(!girara_list_iterator_has_next(iter));
      g_assert(!girara_list_iterator_next(iter));
      g_assert(!girara_list_iterator_is_valid(iter));
    }
  }

  girara_list_iterator_free(iter);
  girara_list_free(list);

  // free function
  list = girara_list_new();
  girara_list_set_free_function(list, list_free);
  girara_list_append(list, (void*)0xDEAD);
  girara_list_free(list);
  g_assert_cmpuint(list_free_called, ==, 1);

  // contains
  list = girara_list_new();
  for (intptr_t i = 0; i != 10; ++i) {
    g_assert_cmpuint(girara_list_contains(list, (void*)i), ==, false);
    girara_list_append(list, (void*)i);
    g_assert_cmpuint(girara_list_contains(list, (void*)i), ==, true);
  }

  // remove
  for (intptr_t i = 9; i >= 0; --i) {
    g_assert_cmpuint(girara_list_contains(list, (void*)i), ==, true);
    girara_list_remove(list, (void*)i);
    g_assert_cmpuint(girara_list_contains(list, (void*)i), ==, false);
    girara_list_append(list, (void*)i);
  }

  iter = girara_list_iterator(list);
  g_assert_cmpptr(iter, !=, NULL);

  for (intptr_t i = 9; i >= 0; --i) {
    g_assert_cmpuint((intptr_t)girara_list_iterator_data(iter), ==, i);
    if (i > 0) {
      g_assert(girara_list_iterator_is_valid(iter));
      g_assert(girara_list_iterator_has_next(iter));
      g_assert(girara_list_iterator_next(iter));
      g_assert(girara_list_iterator_is_valid(iter));
    } else {
      g_assert(girara_list_iterator_is_valid(iter));
      g_assert(!girara_list_iterator_has_next(iter));
      g_assert(!girara_list_iterator_next(iter));
      g_assert(!girara_list_iterator_is_valid(iter));
    }
  }

  girara_list_iterator_free(iter);
  girara_list_free(list);

  // remove with free function
  list_free_called = 0;
  list = girara_list_new();
  girara_list_set_free_function(list, list_free);
  girara_list_append(list, (void*)0xDEAD);
  girara_list_remove(list, (void*)0xDEAD);
  g_assert_cmpuint(girara_list_size(list), ==, 0);
  girara_list_free(list);
  g_assert_cmpuint(list_free_called, ==, 1);
}

void
test_datastructures_sorted_list()
{
  girara_list_t* list = girara_sorted_list_new(NULL);
  g_assert_cmpptr(list, !=, NULL);
  girara_list_free(list);

  list = girara_sorted_list_new2((girara_compare_function_t) g_strcmp0,
      (girara_free_function_t) g_free);
  g_assert_cmpptr(list, !=, NULL);

  static const char* test_strings[] = {
    "A",
    "C",
    "Baa",
    "Za",
    "Bba",
    "Bab",
    NULL
  };
  static const char* test_strings_sorted[] = {
    "A",
    "Baa",
    "Bab",
    "Bba",
    "C",
    "Za",
    NULL
  };

  // append
  for (const char** p = test_strings; *p != NULL; ++p) {
    girara_list_append(list, (void*)*p);
  }

  g_assert_cmpuint(girara_list_size(list), ==, sizeof(test_strings) / sizeof(char*) - 1);

  // check sorting
  const char** p = test_strings_sorted;
  GIRARA_LIST_FOREACH(list, const char*, iter, value)
    g_assert_cmpstr(value, ==, *p);
    ++p;
  GIRARA_LIST_FOREACH_END(list, const char*, iter, value)
}


static void
node_free(void* data)
{
  if (g_strcmp0((char*)data, "root") == 0) {
    g_assert_cmpuint(node_free_called, ==, 0);
  } else if (g_strcmp0((char*)data, "child") == 0) {
    g_assert_cmpuint(node_free_called, ==, 1);
  } else {
    g_assert_not_reached();
  }

  ++node_free_called;
}

void
test_datastructures_node()
{
  girara_tree_node_t* root = girara_node_new("root");
  g_assert_cmpuint(girara_node_get_num_children(root), ==, 0);
  g_assert(girara_node_get_parent(root) == NULL);
  g_assert(girara_node_get_root(root) == root);
  g_assert_cmpstr((char*)girara_node_get_data(root), ==, "root");
  girara_list_t* rchildren = girara_node_get_children(root);
  g_assert(rchildren);
  g_assert_cmpuint(girara_list_size(rchildren), ==, 0);
  girara_list_free(rchildren);
  girara_node_free(root);

  root = girara_node_new("root");
  girara_node_set_free_function(root, node_free);
  girara_node_append_data(root, "child");
  g_assert_cmpuint(girara_node_get_num_children(root), ==, 1);
  g_assert_cmpuint(node_free_called, ==, 0);
  girara_node_free(root);
  g_assert_cmpuint(node_free_called, ==, 2);

  node_free_called = 0;
  root = girara_node_new("root");
  girara_node_set_free_function(root, node_free);
  girara_node_set_data(root, "child");
  g_assert_cmpuint(node_free_called, ==, 1);
  girara_node_free(root);
  g_assert_cmpuint(node_free_called, ==, 2);

  root = girara_node_new(g_strdup("root"));
  girara_node_set_free_function(root, g_free);
  for (unsigned int i = 0; i != 5; ++i) {
    girara_tree_node_t* child = girara_node_append_data(root, g_strdup_printf("child_%u", i));
    for (unsigned int j = 0; j != 10; ++j) {
      girara_node_append_data(child, g_strdup_printf("child_%u_%u", i, j));
    }
    g_assert_cmpuint(girara_node_get_num_children(child), ==, 10);
  }
  g_assert_cmpuint(girara_node_get_num_children(root), ==, 5);

  girara_list_t* children = girara_node_get_children(root);
  g_assert(children);
  g_assert_cmpint(girara_list_size(children), ==, 5);
  unsigned int i = 0;
  girara_list_iterator_t* iter = girara_list_iterator(children);
  while (girara_list_iterator_is_valid(iter))
  {
    char* expected = g_strdup_printf("child_%u", i);
    girara_tree_node_t* child = (girara_tree_node_t*)girara_list_iterator_data(iter);
    g_assert_cmpstr((char*)girara_node_get_data(child), ==, expected);
    g_assert(girara_node_get_parent(child) == root);
    g_assert(girara_node_get_root(child) == root);
    g_free(expected);

    girara_list_t* grandchildren = girara_node_get_children(child);
    g_assert(grandchildren);
    g_assert_cmpint(girara_list_size(grandchildren), ==, 10);
    unsigned int j = 0;
    girara_list_iterator_t* iter2 = girara_list_iterator(grandchildren);
    while (girara_list_iterator_is_valid(iter2))
    {
      char* expected = g_strdup_printf("child_%u_%u", i, j);
      girara_tree_node_t* gchild = (girara_tree_node_t*)girara_list_iterator_data(iter2);
      g_assert_cmpstr((char*)girara_node_get_data(gchild), ==, expected);
      g_assert(girara_node_get_parent(gchild) == child);
      g_assert(girara_node_get_root(gchild) == root);
      g_free(expected);
      ++j;
      girara_list_iterator_next(iter2);
    }
    g_assert_cmpuint(j, ==, 10);
    girara_list_iterator_free(iter2);
    girara_list_free(grandchildren);
    girara_list_iterator_next(iter);
    ++i;
  }
  g_assert_cmpuint(i, ==, 5);
  girara_list_iterator_free(iter);
  girara_list_free(children);

  girara_node_free(root);
}
