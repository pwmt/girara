/* See LICENSE file for license and copyright information */

#include <check.h>
#include <glib.h>
#include <stdint.h>
#include <datastructures.h>

static unsigned int list_free_called = 0;
static unsigned int node_free_called = 0;

static void
list_free(void* data)
{
  fail_unless(list_free_called == 0u);
  fail_unless((intptr_t) data == 0xDEAD);

  ++list_free_called;
}

START_TEST(test_datastructures_list) {
  girara_list_t* list = girara_list_new();
  // size of empty list
  fail_unless(girara_list_size(list) == 0);

  // append
  for (intptr_t i = 0; i != 10; ++i) {
    girara_list_append(list, (void*)i);
  }

  // size of list
  fail_unless(girara_list_size(list) == 10);

  // iterator tests
  girara_list_iterator_t* iter = girara_list_iterator(list);
  fail_unless(iter != NULL);

  for (intptr_t i = 0; i != 10; ++i) {
    fail_unless((intptr_t) girara_list_iterator_data(iter) == i);
    if (i < 9) {
      fail_unless(girara_list_iterator_is_valid(iter));
      fail_unless(girara_list_iterator_has_next(iter));
      fail_unless(girara_list_iterator_next(iter) != NULL);
      fail_unless(girara_list_iterator_is_valid(iter));
    } else {
      fail_unless(girara_list_iterator_is_valid(iter));
      fail_unless(!girara_list_iterator_has_next(iter));
      fail_unless(girara_list_iterator_next(iter) == NULL);
      fail_unless(!girara_list_iterator_is_valid(iter));
    }
  }

  girara_list_iterator_free(iter);
  girara_list_free(list);

  // contains
  list = girara_list_new();
  for (intptr_t i = 0; i != 10; ++i) {
    fail_unless(girara_list_contains(list, (void*) i) == false);
    girara_list_append(list, (void*)i);
    fail_unless(girara_list_contains(list, (void*) i) == true);
  }

  // position
  for (intptr_t i = 0; i != 10; ++i) {
    fail_unless(girara_list_position(list, (void*) i) == i);
  }
  fail_unless(girara_list_position(list, (void*) 10) == -1);

  // remove
  for (intptr_t i = 9; i >= 0; --i) {
    fail_unless(girara_list_contains(list, (void*) i) == true);
    girara_list_remove(list, (void*)i);
    fail_unless(girara_list_contains(list, (void*)i) == false);
    girara_list_append(list, (void*)i);
  }

  iter = girara_list_iterator(list);
  fail_unless(iter != NULL);

  for (intptr_t i = 9; i >= 0; --i) {
    fail_unless((intptr_t)girara_list_iterator_data(iter) == i);
    if (i > 0) {
      fail_unless(girara_list_iterator_is_valid(iter));
      fail_unless(girara_list_iterator_has_next(iter));
      fail_unless(girara_list_iterator_next(iter) != NULL);
      fail_unless(girara_list_iterator_is_valid(iter));
    } else {
      fail_unless(girara_list_iterator_is_valid(iter));
      fail_unless(!girara_list_iterator_has_next(iter));
      fail_unless(girara_list_iterator_next(iter) == NULL);
      fail_unless(!girara_list_iterator_is_valid(iter));
    }
  }

  girara_list_iterator_free(iter);
  girara_list_free(list);
} END_TEST


START_TEST(test_datastructures_list_free) {
  // free function
  girara_list_t* list = girara_list_new();
  fail_unless(list != NULL);
  girara_list_set_free_function(list, list_free);
  girara_list_append(list, (void*) 0xDEAD);
  girara_list_free(list);
  fail_unless(list_free_called == 1);

  list = girara_list_new2(NULL);
  fail_unless(list != NULL);
  girara_list_free(list);

  // remove with free function
  list_free_called = 0;
  list = girara_list_new2(list_free);
  fail_unless(list != NULL);
  girara_list_append(list, (void*)0xDEAD);
  girara_list_remove(list, (void*)0xDEAD);
  fail_unless(girara_list_size(list) == 0);
  girara_list_free(list);
  fail_unless(list_free_called == 1);
} END_TEST

START_TEST(test_datastructures_sorted_list) {
  girara_list_t* list = girara_sorted_list_new(NULL);
  fail_unless(list != NULL);
  girara_list_free(list);

  list = girara_sorted_list_new2((girara_compare_function_t) g_strcmp0,
      (girara_free_function_t) g_free);
  fail_unless(list != NULL);
  girara_list_t* unsorted_list = girara_list_new2((girara_free_function_t) g_free);
  fail_unless(unsorted_list != NULL);

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
    girara_list_append(list, (void*)g_strdup(*p));
    girara_list_append(unsorted_list, (void*)g_strdup(*p));
  }

  fail_unless(girara_list_size(list) == sizeof(test_strings) / sizeof(char*) - 1);
  fail_unless(girara_list_size(unsorted_list) == sizeof(test_strings) / sizeof(char*) - 1);

  // check sorting
  const char** p = test_strings_sorted;
  GIRARA_LIST_FOREACH(list, const char*, iter, value)
    fail_unless(g_strcmp0(value, *p) == 0);
    ++p;
  GIRARA_LIST_FOREACH_END(list, const char*, iter, value);

  girara_list_sort(unsorted_list, (girara_compare_function_t) g_strcmp0);
  p = test_strings_sorted;
  GIRARA_LIST_FOREACH(unsorted_list, const char*, iter, value)
    fail_unless(g_strcmp0(value, *p) == 0);
    ++p;
  GIRARA_LIST_FOREACH_END(unsorted_list, const char*, iter, value);

  girara_list_free(list);
  girara_list_free(unsorted_list);
} END_TEST


static void
node_free(void* data)
{
  if (g_strcmp0((char*)data, "root") == 0) {
    fail_unless(node_free_called == 0);
  } else if (g_strcmp0((char*)data, "child") == 0) {
    fail_unless(node_free_called == 1);
  } else {
    fail("Should not be reached");
  }

  ++node_free_called;
}

START_TEST(test_datastructures_node) {
  girara_tree_node_t* root = girara_node_new("root");
  fail_unless(girara_node_get_num_children(root) == 0);
  fail_unless(girara_node_get_parent(root) == NULL);
  fail_unless(girara_node_get_root(root) == root);
  fail_unless(g_strcmp0((char*) girara_node_get_data(root), "root") == 0);
  girara_list_t* rchildren = girara_node_get_children(root);
  fail_unless(rchildren != NULL);
  fail_unless(girara_list_size(rchildren) == 0);
  girara_list_free(rchildren);
  girara_node_free(root);

  root = girara_node_new("root");
  girara_node_set_free_function(root, node_free);
  girara_node_append_data(root, "child");
  fail_unless(girara_node_get_num_children(root) == 1);
  fail_unless(node_free_called == 0);
  girara_node_free(root);
  fail_unless(node_free_called == 2);

  node_free_called = 0;
  root = girara_node_new("root");
  girara_node_set_free_function(root, node_free);
  girara_node_set_data(root, "child");
  fail_unless(node_free_called == 1);
  girara_node_free(root);
  fail_unless(node_free_called == 2);

  root = girara_node_new(g_strdup("root"));
  girara_node_set_free_function(root, g_free);
  for (unsigned int i = 0; i != 5; ++i) {
    girara_tree_node_t* child = girara_node_append_data(root, g_strdup_printf("child_%u", i));
    for (unsigned int j = 0; j != 10; ++j) {
      girara_node_append_data(child, g_strdup_printf("child_%u_%u", i, j));
    }
    fail_unless(girara_node_get_num_children(child) == 10);
  }
  fail_unless(girara_node_get_num_children(root) == 5);

  girara_list_t* children = girara_node_get_children(root);
  fail_unless(children != NULL);
  fail_unless(girara_list_size(children) == 5);
  unsigned int i = 0;
  girara_list_iterator_t* iter = girara_list_iterator(children);
  while (girara_list_iterator_is_valid(iter))
  {
    char* expected = g_strdup_printf("child_%u", i);
    girara_tree_node_t* child = (girara_tree_node_t*)girara_list_iterator_data(iter);
    fail_unless(g_strcmp0((char*)girara_node_get_data(child), expected) == 0);
    fail_unless(girara_node_get_parent(child) == root);
    fail_unless(girara_node_get_root(child) == root);
    g_free(expected);

    girara_list_t* grandchildren = girara_node_get_children(child);
    fail_unless(grandchildren != NULL);
    fail_unless(girara_list_size(grandchildren) == 10);
    unsigned int j = 0;
    girara_list_iterator_t* iter2 = girara_list_iterator(grandchildren);
    while (girara_list_iterator_is_valid(iter2))
    {
      char* expected = g_strdup_printf("child_%u_%u", i, j);
      girara_tree_node_t* gchild = (girara_tree_node_t*)girara_list_iterator_data(iter2);
      fail_unless(g_strcmp0((char*)girara_node_get_data(gchild), expected) == 0);
      fail_unless(girara_node_get_parent(gchild) == child);
      fail_unless(girara_node_get_root(gchild) == root);
      g_free(expected);
      ++j;
      girara_list_iterator_next(iter2);
    }
    fail_unless(j == 10);
    girara_list_iterator_free(iter2);
    girara_list_free(grandchildren);
    girara_list_iterator_next(iter);
    ++i;
  }
  fail_unless(i == 5);
  girara_list_iterator_free(iter);
  girara_list_free(children);

  girara_node_free(root);
} END_TEST

Suite* suite_datastructures()
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("Datastructures");

  /* list free */
  tcase = tcase_create("list_free_function");
  tcase_add_test(tcase, test_datastructures_list_free);
  suite_add_tcase(suite, tcase);

  /* list create */
  tcase = tcase_create("list_basics");
  tcase_add_test(tcase, test_datastructures_list);
  suite_add_tcase(suite, tcase);

  /* sorted list */
  tcase = tcase_create("list_sorted");
  tcase_add_test(tcase, test_datastructures_sorted_list);
  suite_add_tcase(suite, tcase);

  /* node free */
  tcase = tcase_create("node_free");
  tcase_add_test(tcase, test_datastructures_sorted_list);
  suite_add_tcase(suite, tcase);

  /* node basics */
  tcase = tcase_create("node_basics");
  tcase_add_test(tcase, test_datastructures_node);
  suite_add_tcase(suite, tcase);

  return suite;
}
