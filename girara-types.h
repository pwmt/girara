/* See LICENSE file for license and copyright information */

#ifndef GIRARA_TYPES_H
#define GIRARA_TYPES_H

typedef struct girara_tree_node_s girara_tree_node_t;
typedef struct girara_list_s girara_list_t;
typedef struct girara_list_iterator_s girara_list_iterator_t;

typedef void (*girara_free_function_t)(void*);
typedef void (*girara_list_callback_t)(void*, void*);

#endif
