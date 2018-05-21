/* See LICENSE file for license and copyright information */

#include <check.h>

#include "template.h"
#include "datastructures.h"
#include "tests.h"

START_TEST(test_new) {
  GiraraTemplate* obj = girara_template_new(NULL);
  ck_assert_ptr_ne(obj, NULL);
  g_object_unref(obj);

  obj = girara_template_new("base");
  ck_assert_ptr_ne(obj, NULL);
  g_object_unref(obj);
} END_TEST

START_TEST(test_new_with_null) {
  GiraraTemplate* obj = girara_template_new(NULL);
  ck_assert_ptr_ne(obj, NULL);

  const char* base = girara_template_get_base(obj);
  ck_assert_str_eq((char*) base, "");

  g_object_unref(obj);
} END_TEST

START_TEST(test_new_with_base) {
  GiraraTemplate* obj = girara_template_new("base");
  ck_assert_ptr_ne(obj, NULL);

  const char* base = girara_template_get_base(obj);
  ck_assert_str_eq((char*) base, "base");

  g_object_unref(obj);
} END_TEST

START_TEST(test_base_variables_none) {
  GiraraTemplate* obj = girara_template_new("base");
  ck_assert_ptr_ne(obj, NULL);

  girara_list_t* variables = girara_template_referenced_variables(obj);
  ck_assert_uint_eq(girara_list_size(variables), 0);

  g_object_unref(obj);
} END_TEST

START_TEST(test_base_variables_one) {
  GiraraTemplate* obj = girara_template_new("@test@");
  ck_assert_ptr_ne(obj, NULL);

  girara_list_t* variables = girara_template_referenced_variables(obj);
  ck_assert_uint_eq(girara_list_size(variables), 1);

  char* variable = girara_list_nth(variables, 0);
  ck_assert_str_eq(variable, "test");

  g_object_unref(obj);
} END_TEST

START_TEST(test_base_variables_one_twice) {
  GiraraTemplate* obj = girara_template_new("@test@ @test@");
  ck_assert_ptr_ne(obj, NULL);

  girara_list_t* variables = girara_template_referenced_variables(obj);
  ck_assert_uint_eq(girara_list_size(variables), 1);

  g_object_unref(obj);
} END_TEST

START_TEST(test_variable_add) {
  GiraraTemplate* obj = girara_template_new(NULL);
  ck_assert_ptr_ne(obj, NULL);

  ck_assert(girara_template_add_variable(obj, "name"));
  g_object_unref(obj);
} END_TEST

START_TEST(test_variable_add_invalid) {
  GiraraTemplate* obj = girara_template_new(NULL);
  ck_assert_ptr_ne(obj, NULL);

  ck_assert(!girara_template_add_variable(obj, "na|me"));
  g_object_unref(obj);
} END_TEST

START_TEST(test_variable_set) {
  GiraraTemplate* obj = girara_template_new(NULL);
  ck_assert_ptr_ne(obj, NULL);

  ck_assert(girara_template_add_variable(obj, "name"));
  girara_template_set_variable_value(obj, "name", "value");
  g_object_unref(obj);
} END_TEST

START_TEST(test_full_1) {
  GiraraTemplate* obj = girara_template_new("name = @name@");
  ck_assert_ptr_ne(obj, NULL);

  ck_assert(girara_template_add_variable(obj, "name"));
  girara_template_set_variable_value(obj, "name", "value");

  char* result = girara_template_evaluate(obj);
  ck_assert_ptr_ne(result, NULL);
  ck_assert_str_eq(result, "name = value");

  g_free(result);
  g_object_unref(obj);
} END_TEST

START_TEST(test_full_2) {
  GiraraTemplate* obj = girara_template_new("name = @name@; test = @test@");
  ck_assert_ptr_ne(obj, NULL);

  girara_template_add_variable(obj, "name");
  girara_template_set_variable_value(obj, "name", "value");

  char* result = girara_template_evaluate(obj);
  ck_assert_ptr_eq(result, NULL);

  g_object_unref(obj);
} END_TEST

static Suite* suite_template(void)
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("Template");

  /* girara_template_new */
  tcase = tcase_create("object creation");
  tcase_add_test(tcase, test_new);
  tcase_add_test(tcase, test_new_with_null);
  tcase_add_test(tcase, test_new_with_base);
  suite_add_tcase(suite, tcase);

  /* base processing */
  tcase = tcase_create("base processing");
  tcase_add_test(tcase, test_base_variables_none);
  tcase_add_test(tcase, test_base_variables_one);
  tcase_add_test(tcase, test_base_variables_one_twice);
  suite_add_tcase(suite, tcase);

  /* basic variable operations */
  tcase = tcase_create("variables");
  tcase_add_test(tcase, test_variable_add);
  tcase_add_test(tcase, test_variable_add_invalid);
  tcase_add_test(tcase, test_variable_set);
  suite_add_tcase(suite, tcase);

  /* full processing */
  tcase = tcase_create("full");
  tcase_add_test(tcase, test_full_1);
  tcase_add_test(tcase, test_full_2);
  suite_add_tcase(suite, tcase);

  return suite;
}

int main()
{
  Suite* suite          = NULL;
  SRunner* suite_runner = NULL;
  int number_failed     = 0;

  /* test template */
  suite        = suite_template();
  suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_NORMAL);
  number_failed += srunner_ntests_failed(suite_runner);
  srunner_free(suite_runner);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
