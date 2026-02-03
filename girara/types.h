/* SPDX-License-Identifier: Zlib */

#ifndef GIRARA_TYPES_H
#define GIRARA_TYPES_H

#include "girara-version.h"
#include <stdbool.h>

typedef struct girara_tree_node_s girara_tree_node_t;
typedef struct girara_list_s girara_list_t;
typedef struct girara_list_iterator_s girara_list_iterator_t;

/**
 * Function declaration of a function that frees something.
 *
 * @param data the data to be freed.
 */
typedef void (*girara_free_function_t)(void* data);

/** Function declaration of a function called as callback from girara_list_*
 * functions.
 *
 * @param data a list element.
 * @param userdata data passed as userdata to the calling function.
 */
typedef void (*girara_list_callback_t)(void* data, void* userdata);

/** Function declaration of a function which compares two elements.
 *
 * @param data1 the first element.
 * @param data2 the second element.
 * @return -1 if data1 < data2, 0 if data1 == data2 and 1 if data1 > data2
 */
typedef int (*girara_compare_function_t)(const void* data1, const void* data2);

typedef struct girara_template_s GiraraTemplate;
typedef struct girara_template_class_s GiraraTemplateClass;
typedef struct girara_input_history_io_s GiraraInputHistoryIO;
typedef struct girara_input_history_io_interface_s GiraraInputHistoryIOInterface;
typedef struct girara_input_history_s GiraraInputHistory;
typedef struct girara_input_history_class_s GiraraInputHistoryClass;

#endif
