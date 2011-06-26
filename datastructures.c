/* See LICENSE file for license and copyright information */

#include "girara-datastructures.h"
#include <glib.h>

struct girara_tree_node_s
{
  girara_free_function_t free; /**> The free function */
  GNode* node; /* The node object */
};

typedef struct girara_tree_node_data_s
{
  girara_tree_node_t* node; /**> The node */
  void* data; /**> The data */
} girara_tree_node_data_t;

struct girara_list_s
{
  girara_free_function_t free; /**> The free function */
  GList* start; /**> List start */
};

struct girara_list_iterator_s
{
  girara_list_t* list; /**> The list */
  GList* element; /**> The list object */
};

girara_list_t* girara_list_new(void)
{
  girara_list_t* list = g_malloc0(sizeof(girara_list_t));

  if (!list) {
    return NULL;
  }

  return list;
}

void girara_list_set_free_function(girara_list_t* list, girara_free_function_t gfree)
{
  g_return_if_fail(list);
  list->free = gfree;
}

void girara_list_free(girara_list_t* list)
{
  if (!list) {
    return;
  }

  if (list->free) {
    GList* start = list->start;

    while (start) {
      (*list->free)(start->data);
      start = g_list_next(start);
    }
  }

  g_list_free(list->start);
  g_free(list);
}

void girara_list_append(girara_list_t* list, void* data)
{
  g_return_if_fail(list);

  list->start = g_list_append(list->start, data);
}

void girara_list_prepend(girara_list_t* list, void* data)
{
  g_return_if_fail(list);

  list->start = g_list_prepend(list->start, data);
}

void girara_list_remove(girara_list_t* list, void* data)
{
  g_return_if_fail(list);
  if (!list->start) {
    return;
  }

  GList* tmp = g_list_find(list->start, data);
  if (!tmp) {
    return;
  }

  if (list->free) {
    (list->free)(tmp->data);
  }
  list->start = g_list_delete_link(list->start, tmp);
}

void* girara_list_nth(girara_list_t* list, unsigned int n)
{
  g_return_val_if_fail(list, NULL);
  g_return_val_if_fail(!list->start || (n < g_list_length(list->start)), NULL);

  GList* tmp = g_list_nth(list->start, n);
  g_return_val_if_fail(tmp, NULL);

  return tmp->data;
}

bool girara_list_contains(girara_list_t* list, void* data)
{
  g_return_val_if_fail(list, false);
  if (!list->start) {
    return false;
  }

  GList* tmp = g_list_find(list->start, data);

  if (!tmp) {
    return false;
  }

  return true;
}

girara_list_iterator_t* girara_list_iterator(girara_list_t* list)
{
  g_return_val_if_fail(list, NULL);

  if (!list->start) {
    return NULL;
  }

  girara_list_iterator_t* iter = g_malloc0(sizeof(girara_list_iterator_t));
  g_return_val_if_fail(iter, NULL);
  iter->list = list;
  iter->element = list->start;

  return iter;
}

girara_list_iterator_t* girara_list_iterator_next(girara_list_iterator_t* iter)
{
  if (!iter || !iter->element) {
    return NULL;
  }

  iter->element = g_list_next(iter->element);

  if (!iter->element) {
    return NULL;
  }

  return iter;
}

void* girara_list_iterator_data(girara_list_iterator_t* iter)
{
  g_return_val_if_fail(iter && iter->element, NULL);

  return iter->element->data;
}

void girara_list_iterator_set(girara_list_iterator_t* iter, void *data)
{
  g_return_if_fail(iter && iter->element && iter->list);

  if (iter->list->free) {
    (*iter->list->free)(iter->element->data);
  }

  iter->element->data = data;
}

void girara_list_iterator_free(girara_list_iterator_t* iter)
{
  if (!iter) {
    return;
  }

  g_free(iter);
}

size_t girara_list_size(girara_list_t* list)
{
  g_return_val_if_fail(list, 0);

  if (!list->start) {
    return 0;
  }

  return g_list_length(list->start);
}

int
girara_list_position(girara_list_t* list, void* data)
{
  g_return_val_if_fail(list != NULL, -1);

  if (list->start == NULL) {
    return -1;
  }

  for (unsigned int i = 0; i < g_list_length(list->start); i++) {
    GList* tmp = g_list_nth(list->start, i);

    if (data == tmp->data) {
      return i;
    }
  }

  return -1;
}

girara_tree_node_t* girara_node_new(void* data)
{
  girara_tree_node_t* node = g_malloc0(sizeof(girara_tree_node_t));
  g_return_val_if_fail(node, NULL);

  girara_tree_node_data_t* nodedata = g_malloc0(sizeof(girara_tree_node_data_t));

  if (!nodedata) {
    g_free(node);
    return NULL;
  }

  nodedata->data = data;
  nodedata->node = node;
  node->node = g_node_new(nodedata);

  if (!node->node) {
    g_free(node);
    g_free(nodedata);
    return NULL;
  }

  return node;
}

void girara_node_set_free_function(girara_tree_node_t* node, girara_free_function_t gfree)
{
  g_return_if_fail(node);
  node->free = gfree;
}

void girara_node_free(girara_tree_node_t* node)
{
  if (!node) {
    return;
  }

  g_return_if_fail(node->node);
  girara_tree_node_data_t* nodedata = (girara_tree_node_data_t*) node->node->data;
  g_return_if_fail(nodedata);

  if (node->free) {
    (*node->free)(nodedata->data);
  }

  g_free(nodedata);

  GNode* childnode = node->node->children;
  while (childnode) {
    girara_tree_node_data_t* nodedata = (girara_tree_node_data_t*) childnode->data;
    girara_node_free(nodedata->node);
    childnode = childnode->next;
  }

  g_node_destroy(node->node);
  g_free(node);
}

void girara_node_append(girara_tree_node_t* parent, girara_tree_node_t* child)
{
  g_return_if_fail(parent && child);
  g_node_append(parent->node, child->node);
}

girara_tree_node_t* girara_node_append_data(girara_tree_node_t* parent, void* data)
{
  g_return_val_if_fail(parent, NULL);
  girara_tree_node_t* child = girara_node_new(data);
  g_return_val_if_fail(child, NULL);
  child->free = parent->free;
  girara_node_append(parent, child);

  return child;
}

girara_tree_node_t* girara_node_get_parent(girara_tree_node_t* node)
{
  g_return_val_if_fail(node && node->node, NULL);

  if (!node->node->parent) {
    return NULL;
  }

  girara_tree_node_data_t* nodedata = (girara_tree_node_data_t*) node->node->parent->data;
  g_return_val_if_fail(nodedata, NULL);

  return nodedata->node;
}

girara_tree_node_t* girara_node_get_root(girara_tree_node_t* node)
{
  g_return_val_if_fail(node && node->node, NULL);

  if (!node->node->parent) {
    return node;
  }

  GNode* root = g_node_get_root(node->node);
  g_return_val_if_fail(root, NULL);
  girara_tree_node_data_t* nodedata = (girara_tree_node_data_t*) root->data;
  g_return_val_if_fail(nodedata, NULL);

  return nodedata->node;
}

girara_list_t* girara_node_get_children(girara_tree_node_t* node)
{
  g_return_val_if_fail(node, NULL);
  girara_list_t* list = girara_list_new();
  g_return_val_if_fail(list, NULL);

  GNode* childnode = node->node->children;
  while (childnode) {
    girara_tree_node_data_t* nodedata = (girara_tree_node_data_t*) childnode->data;
    girara_list_append(list, nodedata->node);
    childnode = childnode->next;
  }

  return list;
}

size_t girara_node_get_num_children(girara_tree_node_t* node)
{
  g_return_val_if_fail(node && node->node, 0);

  return g_node_n_children(node->node);
}

void* girara_node_get_data(girara_tree_node_t* node)
{
  g_return_val_if_fail(node && node->node, NULL);
  girara_tree_node_data_t* nodedata = (girara_tree_node_data_t*) node->node->data;
  g_return_val_if_fail(nodedata, NULL);

  return nodedata->data;
}

void girara_node_set_data(girara_tree_node_t* node, void* data)
{
  g_return_if_fail(node && node->node);
  girara_tree_node_data_t* nodedata = (girara_tree_node_data_t*) node->node->data;
  g_return_if_fail(nodedata);

  if (node->free) {
    (*node->free)(nodedata->data);
  }

  nodedata->data = data;
}
