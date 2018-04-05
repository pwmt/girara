/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <glib.h>

#include "datastructures.h"
#include "utils.h"

struct girara_tree_node_s
{
  GNode* node; /* The node object */
  girara_free_function_t free; /**> The free function */
};

typedef struct girara_tree_node_data_s
{
  girara_tree_node_t* node; /**> The node */
  void* data; /**> The data */
} girara_tree_node_data_t;

struct girara_list_s
{
  GList* start; /**> List start */
  girara_free_function_t free; /**> The free function */
  girara_compare_function_t cmp; /**> The sort function */
};

struct girara_list_iterator_s
{
  girara_list_t* list; /**> The list */
  GList* element; /**> The list object */
};

girara_list_t*
girara_list_new(void)
{
  return girara_list_new2(NULL);
}

girara_list_t*
girara_list_new2(girara_free_function_t gfree)
{
  girara_list_t* list = g_try_malloc0(sizeof(girara_list_t));
  if (list == NULL) {
    return NULL;
  }

  list->free = gfree;
  return list;
}

girara_list_t*
girara_sorted_list_new(girara_compare_function_t cmp)
{
  girara_list_t* list = girara_list_new();
  if (list == NULL) {
    return NULL;
  }

  list->cmp = cmp;
  return list;
}

girara_list_t*
girara_sorted_list_new2(girara_compare_function_t cmp, girara_free_function_t gfree)
{
  girara_list_t* list = girara_list_new2(gfree);
  if (list == NULL) {
    return NULL;
  }

  list->cmp = cmp;
  return list;
}

void
girara_list_set_free_function(girara_list_t* list, girara_free_function_t gfree)
{
  g_return_if_fail(list != NULL);
  list->free = gfree;
}

void
girara_list_clear(girara_list_t* list)
{
  if (list == NULL || list->start == NULL) {
    return;
  }

  if (list->free != NULL) {
    g_list_free_full(list->start, list->free);
  } else {
    g_list_free(list->start);
  }
  list->start = NULL;
}

void
girara_list_free(girara_list_t* list)
{
  if (list == NULL) {
    return;
  }

  girara_list_clear(list);
  g_free(list);
}

void
girara_list_append(girara_list_t* list, void* data)
{
  g_return_if_fail(list != NULL);

  if (list->cmp != NULL) {
    list->start = g_list_insert_sorted(list->start, data, list->cmp);
  } else {
    list->start = g_list_append(list->start, data);
  }
}

void
girara_list_prepend(girara_list_t* list, void* data)
{
  g_return_if_fail(list != NULL);

  if (list->cmp != NULL) {
    girara_list_append(list, data);
  } else {
    list->start = g_list_prepend(list->start, data);
  }
}

void
girara_list_remove(girara_list_t* list, void* data)
{
  g_return_if_fail(list != NULL);
  if (list->start == NULL) {
    return;
  }

  GList* tmp = g_list_find(list->start, data);
  if (tmp == NULL) {
    return;
  }

  if (list->free != NULL) {
    (list->free)(tmp->data);
  }
  list->start = g_list_delete_link(list->start, tmp);
}

void*
girara_list_nth(girara_list_t* list, size_t n)
{
  g_return_val_if_fail(list != NULL, NULL);
  g_return_val_if_fail(list->start != NULL && (n < g_list_length(list->start)), NULL);

  GList* tmp = g_list_nth(list->start, n);
  g_return_val_if_fail(tmp != NULL, NULL);

  return tmp->data;
}

bool
girara_list_contains(girara_list_t* list, void* data)
{
  g_return_val_if_fail(list != NULL, false);
  if (!list->start) {
    return false;
  }

  GList* tmp = g_list_find(list->start, data);
  if (tmp == NULL) {
    return false;
  }

  return true;
}

void*
girara_list_find(girara_list_t* list, girara_compare_function_t compare, const void* data)
{
  g_return_val_if_fail(list != NULL && compare != NULL, NULL);
  if (list->start == NULL) {
    return NULL;
  }

  GList* element = g_list_find_custom(list->start, data, compare);
  if (element == NULL) {
    return NULL;
  }

  return element->data;
}


girara_list_iterator_t*
girara_list_iterator(girara_list_t* list)
{
  g_return_val_if_fail(list != NULL, NULL);

  if (list->start == NULL) {
    return NULL;
  }

  girara_list_iterator_t* iter = g_try_malloc0(sizeof(girara_list_iterator_t));
  if (iter == NULL) {
    return NULL;
  }

  iter->list    = list;
  iter->element = list->start;

  return iter;
}

girara_list_iterator_t*
girara_list_iterator_copy(girara_list_iterator_t* iter)
{
  g_return_val_if_fail(iter != NULL, NULL);

  girara_list_iterator_t* iter2 = g_try_malloc0(sizeof(girara_list_iterator_t));
  if (iter2 == NULL) {
    return NULL;
  }

  iter2->list    = iter->list;
  iter2->element = iter->element;
  return iter2;
}

girara_list_iterator_t*
girara_list_iterator_next(girara_list_iterator_t* iter)
{
  if (girara_list_iterator_is_valid(iter) == false) {
    return NULL;
  }

  iter->element = g_list_next(iter->element);
  if (iter->element == NULL) {
    return NULL;
  }

  return iter;
}

bool
girara_list_iterator_has_next(girara_list_iterator_t* iter)
{
  if (girara_list_iterator_is_valid(iter) == false) {
    return false;
  }

  return g_list_next(iter->element);
}

girara_list_iterator_t*
girara_list_iterator_previous(girara_list_iterator_t* iter)
{
  if (girara_list_iterator_is_valid(iter) == false) {
    return NULL;
  }

  iter->element = g_list_previous(iter->element);
  if (iter->element == NULL) {
    return NULL;
  }

  return iter;
}

bool
girara_list_iterator_has_previous(girara_list_iterator_t* iter)
{
  if (girara_list_iterator_is_valid(iter) == false) {
    return false;
  }

  return g_list_previous(iter->element);
}

void
girara_list_iterator_remove(girara_list_iterator_t* iter) {
  if (girara_list_iterator_is_valid(iter) == false) {
    return;
  }

  GList* el = iter->element;
  if (iter->list != NULL && iter->list->free != NULL) {
    (iter->list->free)(iter->element->data);
  }

  iter->element     = el->next;
  iter->list->start = g_list_delete_link(iter->list->start, el);
}

bool
girara_list_iterator_is_valid(girara_list_iterator_t* iter)
{
  return iter != NULL && iter->element != NULL;
}

void*
girara_list_iterator_data(girara_list_iterator_t* iter)
{
  g_return_val_if_fail(girara_list_iterator_is_valid(iter), NULL);

  return iter->element->data;
}

void
girara_list_iterator_set(girara_list_iterator_t* iter, void *data)
{
  g_return_if_fail(girara_list_iterator_is_valid(iter));
  g_return_if_fail(iter->list->cmp == NULL);

  if (iter->list->free != NULL) {
    (*iter->list->free)(iter->element->data);
  }

  iter->element->data = data;
}

void
girara_list_iterator_free(girara_list_iterator_t* iter)
{
  if (iter == NULL) {
    return;
  }

  g_free(iter);
}

size_t
girara_list_size(girara_list_t* list)
{
  g_return_val_if_fail(list != NULL, 0);

  if (list->start == NULL) {
    return 0;
  }

  return g_list_length(list->start);
}

ssize_t
girara_list_position(girara_list_t* list, void* data)
{
  g_return_val_if_fail(list != NULL, -1);

  if (list->start == NULL) {
    return -1;
  }

  bool found = false;
  ssize_t pos = 0;
  GIRARA_LIST_FOREACH_BODY(list, void*, tmp,
    if (tmp == data) {
      found = true;
      break;
    }
    ++pos;
  );

  return found ? pos : -1;
}

void
girara_list_sort(girara_list_t* list, girara_compare_function_t compare)
{
  g_return_if_fail(list != NULL);
  if (list->start == NULL || compare == NULL) {
    return;
  }

  list->start = g_list_sort(list->start, compare);
}

void
girara_list_foreach(girara_list_t* list, girara_list_callback_t callback, void* data)
{
  g_return_if_fail(list != NULL && callback != NULL);
  if (list->start == NULL) {
    return;
  }

  g_list_foreach(list->start, callback, data);
}

static void
list_append(void* data, void* userdata)
{
  girara_list_t* list = userdata;
  girara_list_append(list, data);
}

girara_list_t*
girara_list_merge(girara_list_t* list, girara_list_t* other)
{
  if (list == NULL) {
    return other;
  }
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

girara_tree_node_t*
girara_node_new(void* data)
{
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
    girara_tree_node_data_t* childnodedata = childnode->data;
    girara_node_free(childnodedata->node);
    childnode = childnode->next;
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
