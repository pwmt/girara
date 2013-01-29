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
  fail_unless(list_free_called == 0u, NULL);
  fail_unless((intptr_t) data == 0xDEAD, NULL);

  ++list_free_called;
}

START_TEST(test_datastructures_list) {
  girara_list_t* list = girara_list_new();
  // size of empty list
  fail_unless(girara_list_size(list) == 0, NULL);

  // append
  for (intptr_t i = 0; i != 10; ++i) {
    girara_list_append(list, (void*)i);
  }

  // size of list
  fail_unless(girara_list_size(list) == 10, NULL);

  // iterator tests
  girara_list_iterator_t* iter = girara_list_iterator(list);
  fail_unless(iter != NULL, NULL);

  for (intptr_t i = 0; i != 10; ++i) {
    fail_unless(((intptr_t) girara_list_iterator_data(iter) == i), NULL);
    if (i < 9) {
      fail_unless((girara_list_iterator_is_valid(iter)), NULL);
      fail_unless((girara_list_iterator_has_next(iter)), NULL);
      fail_unless((girara_list_iterator_next(iter) != NULL), NULL);
      fail_unless((girara_list_iterator_is_valid(iter)), NULL);
    } else {
      fail_unless((girara_list_iterator_is_valid(iter)), NULL);
      fail_unless((!girara_list_iterator_has_next(iter)), NULL);
      fail_unless((girara_list_iterator_next(iter) == NULL), NULL);
      fail_unless((!girara_list_iterator_is_valid(iter)), NULL);
    }
  }

  girara_list_iterator_free(iter);
  girara_list_free(list);

  // contains
  list = girara_list_new();
  for (intptr_t i = 0; i != 10; ++i) {
    fail_unless((girara_list_contains(list, (void*) i) == false), NULL);
    girara_list_append(list, (void*)i);
    fail_unless((girara_list_contains(list, (void*) i) == true), NULL);
  }

  // position
  for (intptr_t i = 0; i != 10; ++i) {
    fail_unless((girara_list_position(list, (void*) i) == i), NULL);
  }
  fail_unless((girara_list_position(list, (void*) 10) == -1), NULL);

  // remove
  for (intptr_t i = 9; i >= 0; --i) {
    fail_unless((girara_list_contains(list, (void*) i) == true), NULL);
    girara_list_remove(list, (void*)i);
    fail_unless((girara_list_contains(list, (void*)i) == false), NULL);
    girara_list_append(list, (void*)i);
  }

  iter = girara_list_iterator(list);
  fail_unless((iter != NULL), NULL);

  for (intptr_t i = 9; i >= 0; --i) {
    fail_unless(((intptr_t)girara_list_iterator_data(iter) == i), NULL);
    if (i > 0) {
      fail_unless((girara_list_iterator_is_valid(iter)), NULL);
      fail_unless((girara_list_iterator_has_next(iter)), NULL);
      fail_unless((girara_list_iterator_next(iter) != NULL), NULL);
      fail_unless((girara_list_iterator_is_valid(iter)), NULL);
    } else {
      fail_unless((girara_list_iterator_is_valid(iter)), NULL);
      fail_unless((!girara_list_iterator_has_next(iter)), NULL);
      fail_unless((girara_list_iterator_next(iter) == NULL), NULL);
      fail_unless((!girara_list_iterator_is_valid(iter)), NULL);
    }
  }

  girara_list_iterator_free(iter);
  girara_list_free(list);
} END_TEST

START_TEST(test_datastructures_list_merge) {
  girara_list_t* list1 = girara_list_new();
  girara_list_t* list2 = girara_list_new();
  fail_unless((list1 != NULL), NULL);
  fail_unless((list2 != NULL), NULL);

  fail_unless((girara_list_merge(NULL, NULL) == NULL), NULL);
  fail_unless((girara_list_merge(list1, NULL) == list1), NULL);
  fail_unless((girara_list_merge(NULL, list2) == list2), NULL);

  girara_list_append(list1, (void*)0);
  girara_list_append(list2, (void*)1);

  girara_list_t* list3 = girara_list_merge(list1, list2);
  fail_unless((list3 == list1), NULL);
  fail_unless((girara_list_nth(list3, 0) == (void*)0), NULL);
  fail_unless((girara_list_nth(list3, 1) == (void*)1), NULL);
  girara_list_free(list1);
  girara_list_free(list2);
} END_TEST

START_TEST(test_datastructures_list_free_empty) {
  // free empty list
  girara_list_t* list = girara_list_new();
  fail_unless((list != NULL), NULL);
  girara_list_free(list);

  list = girara_list_new2(NULL);
  fail_unless((list != NULL), NULL);
  girara_list_free(list);

  list = girara_list_new2(g_free);
  fail_unless((list != NULL), NULL);
  girara_list_free(list);
} END_TEST

START_TEST(test_datastructures_list_free_already_cleared) {
  // free cleared list
  girara_list_t* list = girara_list_new();
  fail_unless((list != NULL), NULL);
  girara_list_append(list, (void*) 0xDEAD);
  fail_unless((girara_list_size(list) == 1), NULL);
  girara_list_clear(list);
  fail_unless((girara_list_size(list) == 0), NULL);
  girara_list_free(list);
} END_TEST

START_TEST(test_datastructures_list_free_free_function) {
  // free function
  girara_list_t* list = girara_list_new();
  list_free_called = 0;
  fail_unless((list != NULL), NULL);
  girara_list_set_free_function(list, list_free);
  girara_list_append(list, (void*) 0xDEAD);
  girara_list_free(list);
  fail_unless((list_free_called == 1), NULL);
} END_TEST

START_TEST(test_datastructures_list_free_free_function_remove) {
  // remove with free function
  list_free_called = 0;
  girara_list_t* list = girara_list_new2(list_free);
  fail_unless((list != NULL), NULL);
  girara_list_append(list, (void*)0xDEAD);
  girara_list_remove(list, (void*)0xDEAD);
  fail_unless((girara_list_size(list) == 0), NULL);
  girara_list_free(list);
  fail_unless((list_free_called == 1), NULL);
} END_TEST

START_TEST(test_datastructures_sorted_list_basic) {
  girara_list_t* list = girara_sorted_list_new(NULL);
  fail_unless((list != NULL), NULL);
  girara_list_free(list);
} END_TEST

START_TEST(test_datastructures_sorted_list) {
  girara_list_t* list = girara_sorted_list_new2((girara_compare_function_t) g_strcmp0,
      (girara_free_function_t) g_free);
  fail_unless((list != NULL), NULL);
  girara_list_t* unsorted_list = girara_list_new2((girara_free_function_t) g_free);
  fail_unless((unsorted_list != NULL), NULL);

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

  fail_unless((girara_list_size(list) == sizeof(test_strings) / sizeof(char*) - 1), NULL);
  fail_unless((girara_list_size(unsorted_list) == sizeof(test_strings) / sizeof(char*) - 1), NULL);

  // check sorting
  const char** p = test_strings_sorted;
  GIRARA_LIST_FOREACH(list, const char*, iter, value)
    fail_unless((g_strcmp0(value, *p) == 0), NULL);
    ++p;
  GIRARA_LIST_FOREACH_END(list, const char*, iter, value);

  girara_list_sort(unsorted_list, (girara_compare_function_t) g_strcmp0);
  p = test_strings_sorted;
  GIRARA_LIST_FOREACH(unsorted_list, const char*, iter, value)
    fail_unless((g_strcmp0(value, *p) == 0), NULL);
    ++p;
  GIRARA_LIST_FOREACH_END(unsorted_list, const char*, iter, value);

  girara_list_free(list);
  girara_list_free(unsorted_list);
} END_TEST

static void
node_free(void* data)
{
  if (g_strcmp0((char*)data, "root") == 0) {
    fail_unless((node_free_called == 0), NULL);
  } else if (g_strcmp0((char*)data, "child") == 0) {
    fail_unless((node_free_called == 1), NULL);
  } else {
    fail("Should not be reached");
  }

  ++node_free_called;
}

START_TEST(test_datastructures_node) {
  girara_tree_node_t* root = girara_node_new("root");
  fail_unless((girara_node_get_num_children(root) == 0), NULL);
  fail_unless((girara_node_get_parent(root) == NULL), NULL);
  fail_unless((girara_node_get_root(root) == root), NULL);
  fail_unless((g_strcmp0((char*) girara_node_get_data(root), "root") == 0), NULL);
  girara_list_t* rchildren = girara_node_get_children(root);
  fail_unless((rchildren != NULL), NULL);
  fail_unless((girara_list_size(rchildren) == 0), NULL);
  girara_list_free(rchildren);
  girara_node_free(root);

  root = girara_node_new("root");
  girara_node_set_free_function(root, node_free);
  girara_node_append_data(root, "child");
  fail_unless((girara_node_get_num_children(root) == 1), NULL);
  fail_unless((node_free_called == 0), NULL);
  girara_node_free(root);
  fail_unless((node_free_called == 2), NULL);

  node_free_called = 0;
  root = girara_node_new("root");
  girara_node_set_free_function(root, node_free);
  girara_node_set_data(root, "child");
  fail_unless((node_free_called == 1), NULL);
  girara_node_free(root);
  fail_unless((node_free_called == 2), NULL);

  root = girara_node_new(g_strdup("root"));
  girara_node_set_free_function(root, g_free);
  for (unsigned int i = 0; i != 5; ++i) {
    girara_tree_node_t* child = girara_node_append_data(root, g_strdup_printf("child_%u", i));
    for (unsigned int j = 0; j != 10; ++j) {
      girara_node_append_data(child, g_strdup_printf("child_%u_%u", i, j));
    }
    fail_unless((girara_node_get_num_children(child) == 10), NULL);
  }
  fail_unless((girara_node_get_num_children(root) == 5), NULL);

  girara_list_t* children = girara_node_get_children(root);
  fail_unless((children != NULL), NULL);
  fail_unless((girara_list_size(children) == 5), NULL);
  unsigned int i = 0;
  girara_list_iterator_t* iter = girara_list_iterator(children);
  while (girara_list_iterator_is_valid(iter))
  {
    char* expected = g_strdup_printf("child_%u", i);
    girara_tree_node_t* child = (girara_tree_node_t*)girara_list_iterator_data(iter);
    fail_unless((g_strcmp0((char*)girara_node_get_data(child), expected) == 0), NULL);
    fail_unless((girara_node_get_parent(child) == root), NULL);
    fail_unless((girara_node_get_root(child) == root), NULL);
    g_free(expected);

    girara_list_t* grandchildren = girara_node_get_children(child);
    fail_unless((grandchildren != NULL), NULL);
    fail_unless((girara_list_size(grandchildren) == 10), NULL);
    unsigned int j = 0;
    girara_list_iterator_t* iter2 = girara_list_iterator(grandchildren);
    while (girara_list_iterator_is_valid(iter2))
    {
      char* expected = g_strdup_printf("child_%u_%u", i, j);
      girara_tree_node_t* gchild = (girara_tree_node_t*)girara_list_iterator_data(iter2);
      fail_unless((g_strcmp0((char*)girara_node_get_data(gchild), expected) == 0), NULL);
      fail_unless((girara_node_get_parent(gchild) == child), NULL);
      fail_unless((girara_node_get_root(gchild) == root), NULL);
      g_free(expected);
      ++j;
      girara_list_iterator_next(iter2);
    }
    fail_unless((j == 10), NULL);
    girara_list_iterator_free(iter2);
    girara_list_free(grandchildren);
    girara_list_iterator_next(iter);
    ++i;
  }
  fail_unless((i == 5), NULL);
  girara_list_iterator_free(iter);
  girara_list_free(children);

  girara_node_free(root);
} END_TEST

static int
find_compare(const void* item, const void* data)
{
  if (item == data) {
    return 1;
  } else {
    return 0;
  }
}

START_TEST(test_datastructures_list_find) {
  girara_list_t* list = girara_list_new();
  fail_unless((list != NULL), NULL);

  /* test parameters */
  fail_unless((girara_list_find(NULL, NULL, NULL) == NULL), NULL);
  fail_unless((girara_list_find(list, NULL, NULL) == NULL), NULL);
  fail_unless((girara_list_find(NULL, NULL, (void*) 0xDEAD) == NULL), NULL);
  fail_unless((girara_list_find(NULL, find_compare, NULL) == NULL), NULL);

  /* test functionality */
  girara_list_append(list, (void*) 0xDEAD);
  fail_unless((girara_list_find(list, find_compare, (void*) 0xDEAD) == NULL), NULL);
  fail_unless((girara_list_find(list, find_compare, (void*) 0xCAFE) != NULL), NULL);
  girara_list_free(list);
} END_TEST

START_TEST(test_datastructures_list_prepend) {
  girara_list_t* list = girara_list_new();
  fail_unless((list != NULL), NULL);

  /* test parameters */
  girara_list_prepend(list, NULL);
  fail_unless((girara_list_size(list) != 0), NULL);

  girara_list_free(list);
} END_TEST

Suite* suite_datastructures()
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("Datastructures");

  /* list free */
  tcase = tcase_create("list_free_function");
  tcase_add_test(tcase, test_datastructures_list_free_empty);
  tcase_add_test(tcase, test_datastructures_list_free_already_cleared);
  tcase_add_test(tcase, test_datastructures_list_free_free_function);
  tcase_add_test(tcase, test_datastructures_list_free_free_function_remove);
  suite_add_tcase(suite, tcase);

  /* list create */
  tcase = tcase_create("list_basics");
  tcase_add_test(tcase, test_datastructures_list);
  suite_add_tcase(suite, tcase);

  /* sorted list */
  tcase = tcase_create("list_sorted");
  tcase_add_test(tcase, test_datastructures_sorted_list_basic);
  tcase_add_test(tcase, test_datastructures_sorted_list);
  suite_add_tcase(suite, tcase);

  /* merge lists */
  tcase = tcase_create("list_merge");
  tcase_add_test(tcase, test_datastructures_list_merge);
  suite_add_tcase(suite, tcase);

  /* search lists */
  tcase = tcase_create("list_find");
  tcase_add_test(tcase, test_datastructures_list_find);
  suite_add_tcase(suite, tcase);

  /* prepend ists */
  tcase = tcase_create("list_prepend");
  tcase_add_test(tcase, test_datastructures_list_prepend);
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
