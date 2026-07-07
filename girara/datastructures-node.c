/* SPDX-License-Identifier: Zlib */

#include "datastructures.h"

#include <stdlib.h>
#include <glib.h>

struct girara_tree_node_s {
  GNode* node;                 /**> The node object */
  girara_free_function_t free; /**> The free function */
  void* data;                  /**> The data */
};

girara_tree_node_t* girara_node_new(void* data) {
  girara_tree_node_t* node = g_try_malloc0(sizeof(girara_tree_node_t));
  if (node == NULL) {
    return NULL;
  }

  node->data = data;
  node->node = g_node_new(node);

  if (node->node == NULL) {
    g_free(node);
    return NULL;
  }

  return node;
}

void girara_node_set_free_function(girara_tree_node_t* node, girara_free_function_t gfree) {
  g_return_if_fail(node);
  node->free = gfree;
}

void girara_node_free(girara_tree_node_t* node) {
  if (node == NULL) {
    return;
  }

  if (node->free != NULL) {
    node->free(node->data);
  }

  GNode* childnode = node->node->children;
  while (childnode != NULL) {
    GNode* nextnode                   = childnode->next;
    girara_tree_node_t* childnodedata = childnode->data;
    girara_node_free(childnodedata);
    childnode = nextnode;
  }

  g_node_destroy(node->node);
  g_free(node);
}

void girara_node_append(girara_tree_node_t* parent, girara_tree_node_t* child) {
  g_return_if_fail(parent && child);
  g_node_append(parent->node, child->node);
}

girara_tree_node_t* girara_node_append_data(girara_tree_node_t* parent, void* data) {
  g_return_val_if_fail(parent, NULL);
  girara_tree_node_t* child = girara_node_new(data);
  g_return_val_if_fail(child, NULL);
  child->free = parent->free;
  girara_node_append(parent, child);

  return child;
}

girara_tree_node_t* girara_node_get_parent(girara_tree_node_t* node) {
  g_return_val_if_fail(node && node->node, NULL);

  if (node->node->parent == NULL) {
    return NULL;
  }

  girara_tree_node_t* parent = node->node->parent->data;
  g_return_val_if_fail(parent, NULL);

  return parent;
}

girara_tree_node_t* girara_node_get_root(girara_tree_node_t* node) {
  g_return_val_if_fail(node && node->node, NULL);

  if (node->node->parent == NULL) {
    return node;
  }

  GNode* root = g_node_get_root(node->node);
  g_return_val_if_fail(root, NULL);
  girara_tree_node_t* root_node = root->data;
  g_return_val_if_fail(root_node, NULL);

  return root_node;
}

girara_list_t* girara_node_get_children(girara_tree_node_t* node) {
  g_return_val_if_fail(node, NULL);
  girara_list_t* list = girara_list_new();
  g_return_val_if_fail(list, NULL);

  GNode* childnode = node->node->children;
  while (childnode != NULL) {
    girara_list_append(list, childnode->data);
    childnode = childnode->next;
  }

  return list;
}

size_t girara_node_get_num_children(girara_tree_node_t* node) {
  g_return_val_if_fail(node && node->node, 0);

  return g_node_n_children(node->node);
}

void* girara_node_get_data(girara_tree_node_t* node) {
  g_return_val_if_fail(node, NULL);

  return node->data;
}

void girara_node_set_data(girara_tree_node_t* node, void* data) {
  g_return_if_fail(node);

  if (node->free != NULL) {
    node->free(node->data);
  }

  node->data = data;
}
