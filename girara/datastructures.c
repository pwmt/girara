/* SPDX-License-Identifier: Zlib */

#include <stdlib.h>
#include <glib.h>

#include "datastructures.h"
#include "utils.h"

struct girara_tree_node_s {
  GNode* node;                 /**> The node object */
  girara_free_function_t free; /**> The free function */
};

typedef struct girara_tree_node_data_s {
  girara_tree_node_t* node; /**> The node */
  void* data;               /**> The data */
} girara_tree_node_data_t;

struct girara_list_s {
  void** start;                  /**> List start */
  size_t size;                   /**> The list size */
  girara_free_function_t free;   /**> The free function **/
  girara_compare_function_t cmp; /**> The sort function */
};

struct girara_list_iterator_s {
  girara_list_t* list; /**> The list */
  size_t index;        /**> The list index */
};

girara_list_t* girara_list_new(void) {
  return g_malloc0(sizeof(girara_list_t));
}

girara_list_t* girara_list_new_with_free(girara_free_function_t gfree) {
  girara_list_t* list = g_malloc0(sizeof(girara_list_t));
  if (list == NULL) {
    return NULL;
  }

  list->free = gfree;
  return list;
}

girara_list_t* girara_sorted_list_new(girara_compare_function_t cmp) {
  girara_list_t* list = g_malloc0(sizeof(girara_list_t));
  if (list == NULL) {
    return NULL;
  }

  list->cmp = cmp;
  return list;
}

girara_list_t* girara_sorted_list_new_with_free(girara_compare_function_t cmp, girara_free_function_t gfree) {
  girara_list_t* list = g_malloc0(sizeof(girara_list_t));
  if (list == NULL) {
    return NULL;
  }

  list->free = gfree;
  list->cmp  = cmp;
  return list;
}

void girara_list_set_free_function(girara_list_t* list, girara_free_function_t gfree) {
  g_return_if_fail(list != NULL);
  list->free = gfree;
}

void girara_list_clear(girara_list_t* list) {
  if (list == NULL) {
    return;
  }

  if (list->free) {
    for (size_t idx = 0; idx != list->size; ++idx) {
      list->free(list->start[idx]);
    }
  }
  g_free(list->start);
  list->start = NULL;
  list->size  = 0;
}

void girara_list_free(girara_list_t* list) {
  if (list == NULL) {
    return;
  }

  girara_list_clear(list);
  g_free(list);
}

void girara_list_append(girara_list_t* list, void* data) {
  g_return_if_fail(list != NULL);

  void** new_start = g_realloc_n(list->start, list->size + 1, sizeof(void*));
  g_return_if_fail(new_start != NULL);

  list->start               = new_start;
  list->start[list->size++] = data;
  if (list->cmp != NULL) {
    girara_list_sort(list, list->cmp);
  }
}

void girara_list_prepend(girara_list_t* list, void* data) {
  g_return_if_fail(list != NULL);

  if (list->cmp != NULL) {
    girara_list_append(list, data);
  } else {
    list->start = g_realloc_n(list->start, list->size + 1, sizeof(void*));
    memmove(list->start + 1, list->start, list->size * sizeof(void*));
    list->start[0] = data;
    ++list->size;
  }
}

void girara_list_remove(girara_list_t* list, void* data) {
  g_return_if_fail(list != NULL);

  ssize_t pos = girara_list_position(list, data);
  if (pos == -1) {
    return;
  }

  if (list->free) {
    list->free(list->start[pos]);
  }
  memmove(list->start + pos, list->start + pos + 1, (list->size - pos - 1) * sizeof(void*));
  --list->size;
}

void* girara_list_nth(girara_list_t* list, size_t n) {
  g_return_val_if_fail(list != NULL, NULL);
  g_return_val_if_fail(n < list->size, NULL);

  return list->start[n];
}

void girara_list_set_nth(girara_list_t* list, size_t n, void* data) {
  g_return_if_fail(list != NULL);
  g_return_if_fail(n < list->size);
  g_return_if_fail(list->cmp == NULL);

  if (list->free != NULL) {
    (*list->free)(list->start[n]);
  }

  list->start[n] = data;
}

bool girara_list_contains(girara_list_t* list, void* data) {
  g_return_val_if_fail(list != NULL, false);
  for (size_t idx = 0; idx != list->size; ++idx) {
    if (list->start[idx] == data) {
      return true;
    }
  }
  return false;
}

void* girara_list_find(const girara_list_t* list, girara_compare_function_t compare, const void* data) {
  g_return_val_if_fail(list != NULL && compare != NULL, NULL);

  for (size_t idx = 0; idx != list->size; ++idx) {
    if (compare(list->start[idx], data) == 0) {
      return list->start[idx];
    }
  }

  return NULL;
}

girara_list_iterator_t* girara_list_iterator(girara_list_t* list) {
  g_return_val_if_fail(list != NULL, NULL);

  if (list->size == 0) {
    return NULL;
  }

  girara_list_iterator_t* iter = g_malloc0(sizeof(girara_list_iterator_t));
  if (iter == NULL) {
    return NULL;
  }

  iter->list = list;
  return iter;
}

girara_list_iterator_t* girara_list_iterator_copy(girara_list_iterator_t* iter) {
  g_return_val_if_fail(iter != NULL, NULL);

  return g_memdup2(iter, sizeof(girara_list_iterator_t));
}

girara_list_iterator_t* girara_list_iterator_next(girara_list_iterator_t* iter) {
  if (girara_list_iterator_is_valid(iter) == false) {
    return NULL;
  }

  ++iter->index;
  if (iter->index < iter->list->size) {
    return iter;
  }
  return NULL;
}

bool girara_list_iterator_has_next(girara_list_iterator_t* iter) {
  if (girara_list_iterator_is_valid(iter) == false) {
    return false;
  }

  return iter->index + 1 < iter->list->size;
}

girara_list_iterator_t* girara_list_iterator_previous(girara_list_iterator_t* iter) {
  if (girara_list_iterator_is_valid(iter) == false) {
    return NULL;
  }

  if (iter->index == 0) {
    iter->index = iter->list->size;
    return NULL;
  }
  --iter->index;
  return iter;
}

bool girara_list_iterator_has_previous(girara_list_iterator_t* iter) {
  if (girara_list_iterator_is_valid(iter) == false || iter->index == 0) {
    return false;
  }

  return true;
}

void girara_list_iterator_remove(girara_list_iterator_t* iter) {
  if (girara_list_iterator_is_valid(iter) == false) {
    return;
  }

  if (iter->list->free) {
    iter->list->free(iter->list->start[iter->index]);
  }
  memmove(iter->list->start + iter->index, iter->list->start + iter->index + 1,
          (iter->list->size - iter->index - 1) * sizeof(void*));
  --iter->list->size;
}

bool girara_list_iterator_is_valid(girara_list_iterator_t* iter) {
  return iter != NULL && iter->list != NULL && iter->index < iter->list->size;
}

void* girara_list_iterator_data(girara_list_iterator_t* iter) {
  g_return_val_if_fail(girara_list_iterator_is_valid(iter), NULL);
  return girara_list_nth(iter->list, iter->index);
}

void girara_list_iterator_set(girara_list_iterator_t* iter, void* data) {
  g_return_if_fail(girara_list_iterator_is_valid(iter));
  g_return_if_fail(iter->list->cmp == NULL);

  girara_list_set_nth(iter->list, iter->index, data);
}

void girara_list_iterator_free(girara_list_iterator_t* iter) {
  g_free(iter);
}

size_t girara_list_size(girara_list_t* list) {
  g_return_val_if_fail(list != NULL, 0);
  return list->size;
}

ssize_t girara_list_position(girara_list_t* list, void* data) {
  g_return_val_if_fail(list != NULL, -1);

  for (size_t idx = 0; idx != list->size; ++idx) {
    if (list->start[idx] == data) {
      return idx;
    }
  }
  return -1;
}

typedef struct {
  girara_compare_function_t compare;
} comparer_t;

static int comparer(const void* p1, const void* p2, void* data) {
  comparer_t* compare = data;
  return compare->compare(*(const void**)p1, *(const void**)p2);
}

void girara_list_sort(girara_list_t* list, girara_compare_function_t compare) {
  g_return_if_fail(list != NULL);
  if (list->start == NULL || compare == NULL) {
    return;
  }

  comparer_t comp = {compare};
  g_qsort_with_data(list->start, list->size, sizeof(void*), comparer, &comp);
}

void girara_list_foreach(girara_list_t* list, girara_list_callback_t callback, void* data) {
  g_return_if_fail(list != NULL && callback != NULL);
  if (list->start == NULL) {
    return;
  }

  for (size_t idx = 0; idx != list->size; ++idx) {
    callback(list->start[idx], data);
  }
}

static void list_append(void* data, void* userdata) {
  girara_list_t* list = userdata;
  girara_list_append(list, data);
}

girara_list_t* girara_list_merge(girara_list_t* list, girara_list_t* other) {
  g_return_val_if_fail(list != NULL, NULL);
  if (other == NULL) {
    return list;
  }

  if (list->free != other->free) {
    girara_warning("girara_list_merge: merging lists with different free functions!");
  }
  other->free = NULL;

  girara_list_foreach(other, list_append, list);
  return list;
}

girara_tree_node_t* girara_node_new(void* data) {
  girara_tree_node_t* node = g_try_malloc0(sizeof(girara_tree_node_t));
  if (node == NULL) {
    return NULL;
  }

  girara_tree_node_data_t* nodedata = g_try_malloc0(sizeof(girara_tree_node_data_t));
  if (nodedata == NULL) {
    g_free(node);
    return NULL;
  }

  nodedata->data = data;
  nodedata->node = node;
  node->node     = g_node_new(nodedata);

  if (node->node == NULL) {
    g_free(node);
    g_free(nodedata);
    return NULL;
  }

  return node;
}

void
girara_node_set_free_function(girara_tree_node_t* node, girara_free_function_t gfree)
{
  g_return_if_fail(node);
  node->free = gfree;
}

void
girara_node_free(girara_tree_node_t* node)
{
  if (node == NULL) {
    return;
  }

  g_return_if_fail(node->node);
  girara_tree_node_data_t* nodedata = node->node->data;
  g_return_if_fail(nodedata);

  if (node->free != NULL) {
    (*node->free)(nodedata->data);
  }

  g_free(nodedata);

  GNode* childnode = node->node->children;
  while (childnode != NULL) {
    GNode* nextnode = childnode->next;
    girara_tree_node_data_t* childnodedata = childnode->data;
    girara_node_free(childnodedata->node);
    childnode = nextnode;
  }

  g_node_destroy(node->node);
  g_free(node);
}

void
girara_node_append(girara_tree_node_t* parent, girara_tree_node_t* child)
{
  g_return_if_fail(parent && child);
  g_node_append(parent->node, child->node);
}

girara_tree_node_t*
girara_node_append_data(girara_tree_node_t* parent, void* data)
{
  g_return_val_if_fail(parent, NULL);
  girara_tree_node_t* child = girara_node_new(data);
  g_return_val_if_fail(child, NULL);
  child->free = parent->free;
  girara_node_append(parent, child);

  return child;
}

girara_tree_node_t*
girara_node_get_parent(girara_tree_node_t* node)
{
  g_return_val_if_fail(node && node->node, NULL);

  if (node->node->parent == NULL) {
    return NULL;
  }

  girara_tree_node_data_t* nodedata = (girara_tree_node_data_t*) node->node->parent->data;
  g_return_val_if_fail(nodedata, NULL);

  return nodedata->node;
}

girara_tree_node_t*
girara_node_get_root(girara_tree_node_t* node)
{
  g_return_val_if_fail(node && node->node, NULL);

  if (node->node->parent == NULL) {
    return node;
  }

  GNode* root = g_node_get_root(node->node);
  g_return_val_if_fail(root, NULL);
  girara_tree_node_data_t* nodedata = (girara_tree_node_data_t*) root->data;
  g_return_val_if_fail(nodedata, NULL);

  return nodedata->node;
}

girara_list_t*
girara_node_get_children(girara_tree_node_t* node)
{
  g_return_val_if_fail(node, NULL);
  girara_list_t* list = girara_list_new();
  g_return_val_if_fail(list, NULL);

  GNode* childnode = node->node->children;
  while (childnode != NULL) {
    girara_tree_node_data_t* nodedata = (girara_tree_node_data_t*) childnode->data;
    girara_list_append(list, nodedata->node);
    childnode = childnode->next;
  }

  return list;
}

size_t
girara_node_get_num_children(girara_tree_node_t* node)
{
  g_return_val_if_fail(node && node->node, 0);

  return g_node_n_children(node->node);
}

void*
girara_node_get_data(girara_tree_node_t* node)
{
  g_return_val_if_fail(node && node->node, NULL);
  girara_tree_node_data_t* nodedata = (girara_tree_node_data_t*) node->node->data;
  g_return_val_if_fail(nodedata, NULL);

  return nodedata->data;
}

void
girara_node_set_data(girara_tree_node_t* node, void* data)
{
  g_return_if_fail(node && node->node);
  girara_tree_node_data_t* nodedata = (girara_tree_node_data_t*) node->node->data;
  g_return_if_fail(nodedata);

  if (node->free != NULL) {
    (*node->free)(nodedata->data);
  }

  nodedata->data = data;
}
