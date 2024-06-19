/* SPDX-License-Identifier: Zlib */

#include <glib.h>
#include <stdint.h>
#include <datastructures.h>
#include <macros.h>
#include <log.h>

#include "tests.h"

static unsigned int list_free_called = 0;
static unsigned int node_free_called = 0;

static void list_free(void* data) {
  g_assert_cmpuint(list_free_called, ==, 0u);
  g_assert_cmpint((intptr_t)data, ==, 0xDEAD);

  ++list_free_called;
}

static void test_datastructures_list(void) {
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
  girara_list_iterator_t* iter      = girara_list_iterator(list);
  girara_list_iterator_t* prev_iter = girara_list_iterator(list);
  g_assert_nonnull(iter);
  g_assert_nonnull(prev_iter);

  for (intptr_t i = 0; i != 10; ++i) {
    g_assert_cmpint((intptr_t)girara_list_iterator_data(iter), ==, i);
    if (i < 9) {
      g_assert_true(girara_list_iterator_is_valid(iter));
      g_assert_true(girara_list_iterator_has_next(iter));
      g_assert_nonnull(girara_list_iterator_next(iter));
      g_assert_true(girara_list_iterator_is_valid(iter));
      g_assert_nonnull(girara_list_iterator_next(prev_iter));
    } else {
      g_assert_true(girara_list_iterator_is_valid(iter));
      g_assert_true(!girara_list_iterator_has_next(iter));
      g_assert_null(girara_list_iterator_next(iter));
      g_assert_true(!girara_list_iterator_is_valid(iter));
    }
  }

  for (intptr_t i = 0; i != 10; ++i) {
    g_assert_cmpint((intptr_t)girara_list_iterator_data(prev_iter), ==, 9 - i);
    if (i < 9) {
      g_assert_true(girara_list_iterator_is_valid(prev_iter));
      g_assert_true(girara_list_iterator_has_previous(prev_iter));
      g_assert_nonnull(girara_list_iterator_previous(prev_iter));
      g_assert_true(girara_list_iterator_is_valid(prev_iter));
    } else {
      g_assert_true(girara_list_iterator_is_valid(prev_iter));
      g_assert_true(!girara_list_iterator_has_previous(prev_iter));
      g_assert_null(girara_list_iterator_previous(prev_iter));
      g_assert_true(!girara_list_iterator_is_valid(prev_iter));
    }
  }

  girara_list_iterator_free(prev_iter);
  girara_list_iterator_free(iter);
  girara_list_free(list);

  // contains
  list = girara_list_new();
  for (intptr_t i = 0; i != 10; ++i) {
    g_assert_true(!girara_list_contains(list, (void*)i));
    girara_list_append(list, (void*)i);
    g_assert_true(girara_list_contains(list, (void*)i));
  }

  // position
  for (intptr_t i = 0; i != 10; ++i) {
    g_assert_cmpint(girara_list_position(list, (void*)i), ==, i);
  }
  g_assert_cmpint(girara_list_position(list, (void*)10), ==, -1);

  // remove
  for (intptr_t i = 9; i >= 0; --i) {
    g_assert_true(girara_list_contains(list, (void*)i));
    girara_list_remove(list, (void*)i);
    g_assert_true(!girara_list_contains(list, (void*)i));
    girara_list_append(list, (void*)i);
  }

  iter = girara_list_iterator(list);
  g_assert_nonnull(iter);

  for (intptr_t i = 9; i >= 0; --i) {
    g_assert_cmpint((intptr_t)girara_list_iterator_data(iter), ==, i);
    if (i > 0) {
      g_assert_true(girara_list_iterator_is_valid(iter));
      g_assert_true(girara_list_iterator_has_next(iter));
      g_assert_nonnull(girara_list_iterator_next(iter));
      g_assert_true(girara_list_iterator_is_valid(iter));
    } else {
      g_assert_true(girara_list_iterator_is_valid(iter));
      g_assert_true(!girara_list_iterator_has_next(iter));
      g_assert_null(girara_list_iterator_next(iter));
      g_assert_true(!girara_list_iterator_is_valid(iter));
    }
  }

  girara_list_iterator_free(iter);
  girara_list_free(list);
}

static void test_datastructures_list_merge(void) {
  setup_logger();

  girara_list_t* list1 = girara_list_new();
  girara_list_t* list2 = girara_list_new();
  g_assert_nonnull(list1);
  g_assert_nonnull(list2);

  g_assert_null(girara_list_merge(NULL, NULL));
  g_assert_true(girara_list_merge(list1, NULL) == list1);
  g_assert_null(girara_list_merge(NULL, list2));

  girara_list_append(list1, (void*)0);
  girara_list_append(list2, (void*)1);

  girara_list_t* list3 = girara_list_merge(list1, list2);
  g_assert_true(list3 == list1);
  g_assert_true(girara_list_nth(list3, 0) == (void*)0);
  g_assert_true(girara_list_nth(list3, 1) == (void*)1);
  girara_list_free(list1);
  girara_list_free(list2);
}

static void test_datastructures_list_free_empty(void) {
  // free empty list
  girara_list_t* list = girara_list_new();
  g_assert_nonnull(list);
  girara_list_free(list);

  list = girara_list_new_with_free(NULL);
  g_assert_nonnull(list);
  girara_list_free(list);

  list = girara_list_new_with_free(g_free);
  g_assert_nonnull(list);
  girara_list_free(list);
}

static void test_datastructures_list_free_already_cleared(void) {
  // free cleared list
  girara_list_t* list = girara_list_new();
  g_assert_nonnull(list);
  girara_list_append(list, (void*)0xDEAD);
  g_assert_cmpuint(girara_list_size(list), ==, 1);
  girara_list_clear(list);
  g_assert_cmpuint(girara_list_size(list), ==, 0);
  girara_list_free(list);
}

static void test_datastructures_list_free_free_function(void) {
  // free function
  girara_list_t* list = girara_list_new();
  list_free_called    = 0;
  g_assert_nonnull(list);
  girara_list_set_free_function(list, list_free);
  girara_list_append(list, (void*)0xDEAD);
  girara_list_free(list);
  g_assert_cmpuint(list_free_called, ==, 1);
}

static void test_datastructures_list_free_free_function_remove(void) {
  // remove with free function
  list_free_called    = 0;
  girara_list_t* list = girara_list_new_with_free(list_free);
  g_assert_nonnull(list);
  girara_list_append(list, (void*)0xDEAD);
  girara_list_remove(list, (void*)0xDEAD);
  g_assert_cmpuint(girara_list_size(list), ==, 0);
  girara_list_free(list);
  g_assert_cmpuint(list_free_called, ==, 1);
}

static void test_datastructures_sorted_list_basic(void) {
  girara_list_t* list = girara_sorted_list_new(NULL);
  g_assert_nonnull(list);
  girara_list_free(list);
}

static void test_datastructures_sorted_list(void) {
  girara_list_t* list =
      girara_sorted_list_new_with_free((girara_compare_function_t)g_strcmp0, (girara_free_function_t)g_free);
  g_assert_nonnull(list);
  girara_list_t* unsorted_list = girara_list_new_with_free((girara_free_function_t)g_free);
  g_assert_nonnull(unsorted_list);

  static const char* test_strings[]        = {"A", "C", "Baa", "Za", "Bba", "Bab", NULL};
  static const char* test_strings_sorted[] = {"A", "Baa", "Bab", "Bba", "C", "Za", NULL};

  // append
  for (const char** p = test_strings; *p != NULL; ++p) {
    girara_list_append(list, (void*)g_strdup(*p));
    girara_list_append(unsorted_list, (void*)g_strdup(*p));
  }

  g_assert_cmpuint(girara_list_size(list), ==, sizeof(test_strings) / sizeof(char*) - 1);
  g_assert_cmpuint(girara_list_size(unsorted_list), ==, sizeof(test_strings) / sizeof(char*) - 1);

  // check sorting
  const char** p = test_strings_sorted;
  for (size_t idx = 0; idx != girara_list_size(list); ++idx, ++p) {
    g_assert_cmpstr(girara_list_nth(list, idx), ==, *p);
  }

  girara_list_sort(unsorted_list, (girara_compare_function_t)g_strcmp0);
  p = test_strings_sorted;
  for (size_t idx = 0; idx != girara_list_size(unsorted_list); ++idx, ++p) {
    g_assert_cmpstr(girara_list_nth(unsorted_list, idx), ==, *p);
  }

  girara_list_free(list);
  girara_list_free(unsorted_list);
}

static void node_free(void* data) {
  if (g_strcmp0((char*)data, "root") == 0) {
    g_assert_cmpuint(node_free_called, ==, 0);
  } else if (g_strcmp0((char*)data, "child") == 0) {
    g_assert_cmpuint(node_free_called, ==, 1);
  } else {
    g_assert_not_reached();
  }

  ++node_free_called;
}

static void test_datastructures_node(void) {
  girara_tree_node_t* root = girara_node_new("root");
  g_assert_cmpuint(girara_node_get_num_children(root), ==, 0);
  g_assert_null(girara_node_get_parent(root));
  g_assert_true(girara_node_get_root(root) == root);
  g_assert_cmpstr((char*)girara_node_get_data(root), ==, "root");
  girara_list_t* rchildren = girara_node_get_children(root);
  g_assert_nonnull(rchildren);
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
  root             = girara_node_new("root");
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
  g_assert_nonnull(children);
  g_assert_cmpuint(girara_list_size(children), ==, 5);
  unsigned int i               = 0;
  girara_list_iterator_t* iter = girara_list_iterator(children);
  while (girara_list_iterator_is_valid(iter)) {
    char* expected            = g_strdup_printf("child_%u", i);
    girara_tree_node_t* child = (girara_tree_node_t*)girara_list_iterator_data(iter);
    g_assert_cmpstr((char*)girara_node_get_data(child), ==, expected);
    g_assert_true(girara_node_get_parent(child) == root);
    g_assert_true(girara_node_get_root(child) == root);
    g_free(expected);

    girara_list_t* grandchildren = girara_node_get_children(child);
    g_assert_nonnull(grandchildren);
    g_assert_cmpuint(girara_list_size(grandchildren), ==, 10);
    unsigned int j                = 0;
    girara_list_iterator_t* iter2 = girara_list_iterator(grandchildren);
    while (girara_list_iterator_is_valid(iter2)) {
      expected                   = g_strdup_printf("child_%u_%u", i, j);
      girara_tree_node_t* gchild = (girara_tree_node_t*)girara_list_iterator_data(iter2);
      g_assert_cmpstr((char*)girara_node_get_data(gchild), ==, expected);
      g_assert_true(girara_node_get_parent(gchild) == child);
      g_assert_true(girara_node_get_root(gchild) == root);
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

static int find_compare(const void* item, const void* data) {
  if (item == data) {
    return 1;
  } else {
    return 0;
  }
}

static void test_datastructures_list_find(void) {
  setup_logger();

  girara_list_t* list = girara_list_new();
  g_assert_nonnull(list);

  /* test parameters */
  g_assert_null(girara_list_find(NULL, NULL, NULL));
  g_assert_null(girara_list_find(list, NULL, NULL));
  g_assert_null(girara_list_find(NULL, NULL, (void*)0xDEAD));
  g_assert_null(girara_list_find(NULL, find_compare, NULL));

  /* test functionality */
  girara_list_append(list, (void*)0xDEAD);
  g_assert_null(girara_list_find(list, find_compare, (void*)0xDEAD));
  g_assert_nonnull(girara_list_find(list, find_compare, (void*)0xCAFE));
  girara_list_free(list);
}

static void test_datastructures_list_prepend(void) {
  girara_list_t* list = girara_list_new();
  g_assert_nonnull(list);

  /* test parameters */
  girara_list_prepend(list, NULL);
  g_assert_cmpuint(girara_list_size(list), !=, 0);

  girara_list_free(list);
}

int main(int argc, char* argv[]) {
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/list/free_empty", test_datastructures_list_free_empty);
  g_test_add_func("/list/free_cleared", test_datastructures_list_free_already_cleared);
  g_test_add_func("/list/free_function", test_datastructures_list_free_free_function);
  g_test_add_func("/list/free_function_remove", test_datastructures_list_free_free_function_remove);
  g_test_add_func("/list/basic", test_datastructures_list);
  g_test_add_func("/list/sorted_basic", test_datastructures_sorted_list_basic);
  g_test_add_func("/list/sorted", test_datastructures_sorted_list);
  g_test_add_func("/list/merge", test_datastructures_list_merge);
  g_test_add_func("/list/search", test_datastructures_list_find);
  g_test_add_func("/list/prepand", test_datastructures_list_prepend);
  g_test_add_func("/node/basic", test_datastructures_node);
  return g_test_run();
}
