/* See LICENSE file for license and copyright information */

#ifndef GIRARA_DATASTRUCTURES_H
#define GIRARA_DATASTRUCTURES_H

#include <stddef.h>
#include <stdbool.h>

typedef struct girara_tree_node_s girara_tree_node_t;
typedef struct girara_list_s girara_list_t;
typedef struct girara_list_iterator_s girara_list_iterator_t;

typedef void (*girara_free_function_t)(void*);

/**
 * Create a new list.
 */
girara_list_t* girara_list_new(void);

/**
 * Set the function which should be called if the stored data should be freed.
 */
void girara_list_set_free_function(girara_list_t* list, girara_free_function_t gfree);

/**
 * Destroy list.
 */
void girara_list_free(girara_list_t* list);

/**
 * Append an element to the list.
 */
void girara_list_append(girara_list_t* list, void* data);

/**
 * Prepend an element to the list.
 */
void girara_list_prepend(girara_list_t* list, void* data);

/**
 * Remove an element of the list
 */
void girara_list_remove(girara_list_t* list, void* data);

/**
 * Returns nth entry
 */
void* girara_list_nth(girara_list_t* list, unsigned int n);

/**
 * Checks if the list contains the given element
 */
bool girara_list_contains(girara_list_t* list, void* data);

/**
 * Get size of the list.
 */
size_t girara_list_size(girara_list_t* list);

/**
 * Create an iterator pointing at the start of list.
 */
girara_list_iterator_t* girara_list_iterator(girara_list_t* list);

/**
 * Move iterator to next element.
 */
girara_list_iterator_t* girara_list_iterator_next(girara_list_iterator_t* iter);

/**
 * Get data from the element pointed to by the iterator.
 */
void* girara_list_iterator_data(girara_list_iterator_t* iter);

/**
 * Set data from the element pointed to by the iterator.
 */
void girara_list_iterator_set(girara_list_iterator_t* iter, void *data);

/**
 * Destroy the iterator.
 */
void girara_list_iterator_free(girara_list_iterator_t* iter);

/**
 * Create a new node.
 */
girara_tree_node_t* girara_node_new(void* data);

/**
 * Set the function which should be called if the stored data should be freed.
 */
void girara_node_set_free_function(girara_tree_node_t* node, girara_free_function_t gfree);

/**
 * Free a node. This will remove the node from its' parent and will destroy all
 * its' children.
 */
void girara_node_free(girara_tree_node_t* node);

/**
 * Append a node to another node.
 */
void girara_node_append(girara_tree_node_t* parent, girara_tree_node_t* child);

/**
 * Append data as new node to another node.
 */
girara_tree_node_t* girara_node_append_data(girara_tree_node_t* parent, void* data);

/**
 * Get parent node.
 */
girara_tree_node_t* girara_node_get_parent(girara_tree_node_t* node);

/**
 * Get root node.
 */
girara_tree_node_t* girara_node_get_root(girara_tree_node_t* node);

/**
 * Get list of children.
 */
girara_list_t* girara_node_get_children(girara_tree_node_t* node);

/**
 * Get number of children.
 */
size_t girara_node_get_num_children(girara_tree_node_t* node);

/**
 * Get data.
 */
void* girara_node_get_data(girara_tree_node_t* node);

/**
 * Set data.
 */
void girara_node_set_data(girara_tree_node_t* node, void* data);

#endif
