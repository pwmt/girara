/* SPDX-License-Identifier: Zlib */

#ifndef GIRARA_DATASTRUCTURES_H
#define GIRARA_DATASTRUCTURES_H

#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>
#include "macros.h"
#include "types.h"

/**
 * Create a new list.
 *
 * @return The girara list object or NULL if an error occurred
 */
girara_list_t* girara_list_new(void) GIRARA_VISIBLE;

/**
 * Create a new list.
 *
 * @param gfree Pointer to the free function
 * @return The girara list object or NULL if an error occurred.
 */
girara_list_t* girara_list_new2(girara_free_function_t gfree) GIRARA_VISIBLE;

/**
 * Create a new (sorted) list.
 *
 * @param cmp Pointer to the compare function.
 * @return The girara list object or NULL if an error occurred.
 */
girara_list_t* girara_sorted_list_new(girara_compare_function_t cmp) GIRARA_VISIBLE;

/**
 * Create a new (sorted) list.
 *
 * @param cmp Pointer to the compare function.
 * @param gfree Pointer to the free function
 * @return The girara list object or NULL if an error occurred.
 */

girara_list_t* girara_sorted_list_new2(girara_compare_function_t cmp,
    girara_free_function_t gfree) GIRARA_VISIBLE;

/**
 * Set the function which should be called if the stored data should be freed.
 *
 * @param list The girara list object
 * @param gfree Pointer to the free function
 */
void girara_list_set_free_function(girara_list_t* list,
    girara_free_function_t gfree) GIRARA_VISIBLE;

/**
 * Remove all elements from a list.
 *
 * @param list The girara list object
 */
void girara_list_clear(girara_list_t* list) GIRARA_VISIBLE;

/**
 * Destroy list.
 *
 * @param list The girara list object
 */
void girara_list_free(girara_list_t* list) GIRARA_VISIBLE;

/**
 * Append an element to the list.
 *
 * @param list The girara list object
 * @param data The element
 */
void girara_list_append(girara_list_t* list, void* data) GIRARA_VISIBLE;

/**
 * Prepend an element to the list.
 *
 * @param list The girara list object
 * @param data The element
 */
void girara_list_prepend(girara_list_t* list, void* data) GIRARA_VISIBLE;

/**
 * Remove an element of the list
 *
 * @param list The girara list object
 * @param data The element
 */
void girara_list_remove(girara_list_t* list, void* data) GIRARA_VISIBLE;

/**
 * Returns nth entry
 *
 * @param list The girara list object
 * @param n Index of the entry
 * @return The nth element or NULL if an error occurred
 */
void* girara_list_nth(girara_list_t* list, size_t n) GIRARA_VISIBLE;

/**
 * Checks if the list contains the given element
 *
 * @param list The girara list object
 * @param data The element
 * @return true if the list contains the element
 */
bool girara_list_contains(girara_list_t* list, void* data) GIRARA_VISIBLE;

/**
 * Get size of the list.
 *
 * @param list The girara list object
 * @return The size of the list
 */
size_t girara_list_size(girara_list_t* list) GIRARA_VISIBLE;

/**
 * Returns the position of the element in the list
 *
 * @param list The girara list object
 * @param data The element
 * @return The position or -1 if the data is not found
 */
ssize_t girara_list_position(girara_list_t* list, void* data) GIRARA_VISIBLE;

/**
 * Sort a list
 *
 * @param list The list to sort
 * @param compare compare function
 */
void girara_list_sort(girara_list_t* list, girara_compare_function_t compare) GIRARA_VISIBLE;

/**
 * Find an element
 *
 * @param list The list
 * @param compare compare function
 * @param data data passed as the second argument to the compare function
 * @return the element if found or NULL
 */
void* girara_list_find(girara_list_t* list, girara_compare_function_t compare,
    const void* data) GIRARA_VISIBLE;

/**
 * Create an iterator pointing at the start of list.
 *
 * @param list The girara list object
 * @return The list iterator or NULL if an error occurred
 */
girara_list_iterator_t* girara_list_iterator(girara_list_t* list) GIRARA_VISIBLE;

/**
 * Create an iterator pointing to the same element as iter.
 *
 * @param iter The girara list iterator to be copied
 * @return The list iterator or NULL if an error occurred
 */
girara_list_iterator_t* girara_list_iterator_copy(girara_list_iterator_t* iter) GIRARA_VISIBLE;

/**
 * Move iterator to next element.
 *
 * @param iter The list iterator
 * @return The moved iterator or NULL if an error occurred
 */
girara_list_iterator_t* girara_list_iterator_next(girara_list_iterator_t* iter) GIRARA_VISIBLE;

/**
 * Check if iterator has next element.
 *
 * @param iter The list iterator
 * @return true if iterator has a next element, false otherwise
 */
bool girara_list_iterator_has_next(girara_list_iterator_t* iter) GIRARA_VISIBLE;

/**
 * Move iterator to previous element.
 *
 * @param iter The list iterator
 * @return The moved iterator or NULL if an error occurred
 */
girara_list_iterator_t* girara_list_iterator_previous(girara_list_iterator_t* iter) GIRARA_VISIBLE;

/**
 * Check if iterator has previous element.
 *
 * @param iter The list iterator
 * @return true if iterator has a previous element, false otherwise
 */
bool girara_list_iterator_has_previous(girara_list_iterator_t* iter) GIRARA_VISIBLE;

/**
 * Remove element pointed by the iterator, and updates the iterator
 * to the next element
 *
 * @param iter The list iterator
 */
void girara_list_iterator_remove(girara_list_iterator_t* iter) GIRARA_VISIBLE;

/**
 * Check if iterator is valid
 *
 * @param iter The list iterator
 * @return true if iterator is valid, false otherwise
 */
bool girara_list_iterator_is_valid(girara_list_iterator_t* iter) GIRARA_VISIBLE;

/**
 * Get data from the element pointed to by the iterator.
 *
 * @param iter The list iterator
 * @return The data of the current element
 */
void* girara_list_iterator_data(girara_list_iterator_t* iter) GIRARA_VISIBLE;

/**
 * Set data from the element pointed to by the iterator.
 *
 * @param iter The list iterator
 * @param data Sets the list iterator to a specific element
 */
void girara_list_iterator_set(girara_list_iterator_t* iter, void *data) GIRARA_VISIBLE;

/**
 * Destroy the iterator.
 *
 * @param iter The list iterator
 */
void girara_list_iterator_free(girara_list_iterator_t* iter) GIRARA_VISIBLE;

/**
 * Call function for each element in the list.
 *
 * @param list The list
 * @param callback The function to call.
 * @param data Passed to the callback as second argument.
 */
void girara_list_foreach(girara_list_t* list, girara_list_callback_t callback,
    void* data) GIRARA_VISIBLE;

#define GIRARA_LIST_FOREACH(list, type, iter, data) \
  do { \
    girara_list_iterator_t* iter = girara_list_iterator(list); \
    while (girara_list_iterator_is_valid(iter)) { \
      type data = (type)girara_list_iterator_data(iter);

#define GIRARA_LIST_FOREACH_END(list, type, iter, data) \
      girara_list_iterator_next(iter); \
    } \
    girara_list_iterator_free(iter); \
  } while(0)

#define GIRARA_LIST_FOREACH_BODY_WITH_ITER(list, type, iter, data, ...) \
  GIRARA_LIST_FOREACH(list, type, iter, data) \
  __VA_ARGS__ \
  GIRARA_LIST_FOREACH_END(list, type, iter, data)

#define GIRARA_LIST_FOREACH_BODY(list, type, data, ...) \
  GIRARA_LIST_FOREACH(list, type, girara_list_foreach_iterator, data) \
  __VA_ARGS__ \
  GIRARA_LIST_FOREACH_END(list, type, girara_list_foreach_iterator, data)

/**
 * Merge a list into another one. Both lists need to have the same free
 * function. If other has a source free function set it will be set to NULL as
 * the elements then belong to list.
 * @param list the target list
 * @param other the source list
 * @returns list with the elements from other.
 */
girara_list_t* girara_list_merge(girara_list_t* list, girara_list_t* other) GIRARA_VISIBLE;

/**
 * Create a new node.
 *
 * @param data Data of the new node
 * @return A girara node object or NULL if an error occurred
 */
girara_tree_node_t* girara_node_new(void* data) GIRARA_VISIBLE;

/**
 * Set the function which should be called if the stored data should be freed.
 *
 * @param node The girara node object
 * @param gfree Pointer to the free function
 */
void girara_node_set_free_function(girara_tree_node_t* node,
    girara_free_function_t gfree) GIRARA_VISIBLE;

/**
 * Free a node. This will remove the node from its' parent and will destroy all
 * its' children.
 *
 * @param node The girara node object
 */
void girara_node_free(girara_tree_node_t* node) GIRARA_VISIBLE;

/**
 * Append a node to another node.
 *
 * @param parent The parent node
 * @param child The child node
 */
void girara_node_append(girara_tree_node_t* parent, girara_tree_node_t* child) GIRARA_VISIBLE;

/**
 * Append data as new node to another node.
 *
 * @param parent The parent node
 * @param data The data of the node
 * @return The node object or NULL if an error occurred
 */
girara_tree_node_t* girara_node_append_data(girara_tree_node_t* parent,
    void* data) GIRARA_VISIBLE;

/**
 * Get parent node.
 *
 * @param node The girara node object
 * @return The parent node or NULL if an error occurred or no parent exists
 */
girara_tree_node_t* girara_node_get_parent(girara_tree_node_t* node) GIRARA_VISIBLE;

/**
 * Get root node.
 *
 * @param node The girara node object
 * @return The root node or NULL if an error occurred
 */
girara_tree_node_t* girara_node_get_root(girara_tree_node_t* node) GIRARA_VISIBLE;

/**
 * Get list of children.
 *
 * @param node The girara node object
 * @return List object containing all child nodes or NULL if an error occurred
 */
girara_list_t* girara_node_get_children(girara_tree_node_t* node) GIRARA_VISIBLE;

/**
 * Get number of children.
 *
 * @param node The girara node object
 * @return The number of child nodes
 */
size_t girara_node_get_num_children(girara_tree_node_t* node) GIRARA_VISIBLE;

/**
 * Get data.
 *
 * @param node The girara node object
 * @return The data of the node
 */
void* girara_node_get_data(girara_tree_node_t* node) GIRARA_VISIBLE;

/**
 * Set data.
 *
 * @param node The girara node object
 * @param data The new data of the object
 */
void girara_node_set_data(girara_tree_node_t* node, void* data) GIRARA_VISIBLE;

#endif
