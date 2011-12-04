// See LICENSE file for license and copyright information

#ifndef TESTS_H
#define TESTS_H

// utils tests
void test_utils_home_directory(void);
void test_utils_fix_path(void);
void test_utils_xdg_path(void);
void test_utils_file_invariants(void);
void test_utils_file_read(void);
void test_utils_safe_realloc(void);

// datastructures tests
void test_datastructures_list(void);
void test_datastructures_list_free(void);
void test_datastructures_sorted_list(void);
void test_datastructures_node(void);

// session tests
void test_session_basic(void);

// settings tests
void test_settings_basic(void);
void test_settings_callback(void);

#endif
