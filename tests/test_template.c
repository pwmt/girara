/* SPDX-License-Identifier: Zlib */

#include "template.h"
#include "datastructures.h"

static void test_new(void) {
  GiraraTemplate* obj = girara_template_new(NULL);
  g_assert_nonnull(obj);
  g_object_unref(obj);

  obj = girara_template_new("base");
  g_assert_nonnull(obj);
  g_object_unref(obj);
}

static void test_new_with_null(void) {
  GiraraTemplate* obj = girara_template_new(NULL);
  g_assert_nonnull(obj);

  const char* base = girara_template_get_base(obj);
  g_assert_cmpstr(base, ==, "");

  g_object_unref(obj);
}

static void test_new_with_base(void) {
  GiraraTemplate* obj = girara_template_new("base");
  g_assert_nonnull(obj);

  const char* base = girara_template_get_base(obj);
  g_assert_cmpstr(base, ==, "base");

  g_object_unref(obj);
}

static void test_base_variables_none(void) {
  GiraraTemplate* obj = girara_template_new("base");
  g_assert_nonnull(obj);

  girara_list_t* variables = girara_template_referenced_variables(obj);
  g_assert_cmpuint(girara_list_size(variables), ==, 0);

  g_object_unref(obj);
}

static void test_base_variables_one(void) {
  GiraraTemplate* obj = girara_template_new("@test@");
  g_assert_nonnull(obj);

  girara_list_t* variables = girara_template_referenced_variables(obj);
  g_assert_cmpuint(girara_list_size(variables), ==, 1);

  char* variable = girara_list_nth(variables, 0);
  g_assert_cmpstr(variable, ==, "test");

  g_object_unref(obj);
}

static void test_base_variables_one_twice(void) {
  GiraraTemplate* obj = girara_template_new("@test@ @test@");
  g_assert_nonnull(obj);

  girara_list_t* variables = girara_template_referenced_variables(obj);
  g_assert_cmpuint(girara_list_size(variables), ==, 1);

  g_object_unref(obj);
}

static void test_variable_add(void) {
  GiraraTemplate* obj = girara_template_new(NULL);
  g_assert_nonnull(obj);

  g_assert_true(girara_template_add_variable(obj, "name"));
  g_object_unref(obj);
}

static void test_variable_add_invalid(void) {
  GiraraTemplate* obj = girara_template_new(NULL);
  g_assert_nonnull(obj);

  g_assert_false(girara_template_add_variable(obj, "na|me"));
  g_object_unref(obj);
}

static void test_variable_set(void) {
  GiraraTemplate* obj = girara_template_new(NULL);
  g_assert_nonnull(obj);

  g_assert_true(girara_template_add_variable(obj, "name"));
  girara_template_set_variable_value(obj, "name", "value");
  g_object_unref(obj);
}

static void test_full_1(void) {
  GiraraTemplate* obj = girara_template_new("name = @name@");
  g_assert_nonnull(obj);

  g_assert_true(girara_template_add_variable(obj, "name"));
  girara_template_set_variable_value(obj, "name", "value");

  char* result = girara_template_evaluate(obj);
  g_assert_nonnull(result);
  g_assert_cmpstr(result, ==, "name = value");

  g_free(result);
  g_object_unref(obj);
}

static void test_full_2(void) {
  GiraraTemplate* obj = girara_template_new("name = @name@; test = @test@");
  g_assert_nonnull(obj);

  girara_template_add_variable(obj, "name");
  girara_template_set_variable_value(obj, "name", "value");

  char* result = girara_template_evaluate(obj);
  g_assert_null(result);

  g_object_unref(obj);
}

int main(int argc, char* argv[]) {
  g_test_init(&argc, &argv, NULL);
  g_test_add_func("/template/new", test_new);
  g_test_add_func("/template/new_with_null", test_new_with_null);
  g_test_add_func("/template/new_with_base", test_new_with_base);
  g_test_add_func("/template/base_variables_none", test_base_variables_none);
  g_test_add_func("/template/base_variables_one", test_base_variables_one);
  g_test_add_func("/template/base_variables_one_twice", test_base_variables_one_twice);
  g_test_add_func("/template/variable_add", test_variable_add);
  g_test_add_func("/template/variable_add_invalid", test_variable_add_invalid);
  g_test_add_func("/template/variable_set", test_variable_set);
  g_test_add_func("/template/full_1", test_full_1);
  g_test_add_func("/template/full_2", test_full_2);
  return g_test_run();
}
