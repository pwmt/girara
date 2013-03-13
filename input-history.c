/* See LICENSE file for license and copyright information */

#include "input-history.h"
#include "datastructures.h"

G_DEFINE_TYPE(GiraraInputHistory, girara_input_history, G_TYPE_OBJECT)

/**
 * Private data of the settings manager
 */
typedef struct girara_input_history_private_s {
  girara_list_t* history; /**< List of stored inputs */
  bool reset; /**< Show history starting from the most recent command */
  size_t current;
  size_t current_match;
} girara_input_history_private_t;

#define GIRARA_INPUT_HISTORY_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GIRARA_TYPE_INPUT_HISTORY, \
                                girara_input_history_private_t))

/* Methods */
static void ih_finalize(GObject* object);
static void ih_append(GiraraInputHistory* history, const char* input);
static girara_list_t* ih_list(GiraraInputHistory* history);
static const char* ih_next(GiraraInputHistory* history, const char* current_input);
static const char* ih_previous(GiraraInputHistory* history, const char* current_input);
static void ih_reset(GiraraInputHistory* history);

/* Class init */
static void
girara_input_history_class_init(GiraraInputHistoryClass* class)
{
  /* add private members */
  g_type_class_add_private(class, sizeof(girara_input_history_private_t));

  /* overwrite methods */
  GObjectClass* object_class = G_OBJECT_CLASS(class);
  object_class->finalize     = ih_finalize;

  class->append = ih_append;
  class->list = ih_list;
  class->next = ih_next;
  class->previous = ih_previous;
  class->reset = ih_reset;
}

/* Object init */
static void
girara_input_history_init(GiraraInputHistory* history)
{
  girara_input_history_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(history);
  priv->history = girara_list_new2((girara_free_function_t) g_free);
  priv->reset = true;
}

/* Object finalize */
static void
ih_finalize(GObject* object)
{
  girara_input_history_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(object);
  girara_list_free(priv->history);

  G_OBJECT_CLASS(girara_input_history_parent_class)->finalize(object);
}

/*Object new */
GiraraInputHistory*
girara_input_history_new(void)
{
  return GIRARA_INPUT_HISTORY(g_object_new(GIRARA_TYPE_INPUT_HISTORY, 0));
}

/* Method implementions */

static void
ih_append(GiraraInputHistory* history, const char* input)
{
  if (input == NULL) {
    return;
  }

  girara_list_t* list = girara_input_history_list(history);
  if (list == NULL) {
    return;
  }

  GIRARA_LIST_FOREACH(list, char*, iter, data)
    if (g_strcmp0(input, data) == 0) {
        girara_list_remove(list, data);
    }
  GIRARA_LIST_FOREACH_END(list, char*, iter, data);

  girara_list_append(list, g_strdup(input));

  /* begin from the last command when navigating through history */
  girara_input_history_reset(history);
}

static girara_list_t*
ih_list(GiraraInputHistory* history)
{
  girara_input_history_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(history);
  return priv->history;
}

static const char*
find_next(GiraraInputHistory* history, const char* current_input, bool next)
{
  girara_input_history_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(history);

  girara_list_t* list = girara_input_history_list(history);
  if (list == NULL) {
    return NULL;
  }

  size_t length = girara_list_size(list);
  if (length == 0) {
    return NULL;
  }

  if (priv->reset == true) {
    priv->current = length;
    priv->current_match = priv->current;
  }

  size_t i = 0;
  const char prefix = *current_input;
  const char* command = NULL;
  while (i < length) {
    if (priv->reset == true || next == false) {
      if (priv->current < 1) {
        priv->reset = false;
        priv->current = priv->current_match;
        return NULL;
      } else {
        --priv->current;
      }
    } else if (next == true) {
      if (priv->current + 1 == length) {
        priv->current = priv->current_match;
        return NULL;
      } else {
        ++priv->current;
      }
    } else {
      return NULL;
    }

    command = girara_list_nth(list, priv->current);
    if (command == NULL) {
      return NULL;
    }

    if (command[0] == prefix) {
      priv->reset = false;
      priv->current_match = priv->current;
      break;
    }

    ++i;
  }

  if (i == length) {
    return NULL;
  }

  return command;
}

static const char*
ih_next(GiraraInputHistory* history, const char* current_input)
{
  return find_next(history, current_input, true);
}

static const char*
ih_previous(GiraraInputHistory* history, const char* current_input)
{
  return find_next(history, current_input, false);
}

static void
ih_reset(GiraraInputHistory* history)
{
  girara_input_history_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(history);
  priv->reset = true;
}

/* Wrapper functions for the members */

void
girara_input_history_append(GiraraInputHistory* history, const char* input)
{
  g_return_if_fail(GIRARA_IS_INPUT_HISTORY(history) == true);
  GIRARA_INPUT_HISTORY_GET_CLASS(history)->append(history, input);
}

girara_list_t*
girara_input_history_list(GiraraInputHistory* history)
{
  g_return_val_if_fail(GIRARA_IS_INPUT_HISTORY(history) == true, NULL);
  return GIRARA_INPUT_HISTORY_GET_CLASS(history)->list(history);
}

const char*
girara_input_history_next(GiraraInputHistory* history, const char* current_input)
{
  g_return_val_if_fail(GIRARA_IS_INPUT_HISTORY(history) == true, NULL);
  return GIRARA_INPUT_HISTORY_GET_CLASS(history)->next(history, current_input);
}

const char*
girara_input_history_previous(GiraraInputHistory* history, const char* current_input)
{
  g_return_val_if_fail(GIRARA_IS_INPUT_HISTORY(history) == true, NULL);
  return GIRARA_INPUT_HISTORY_GET_CLASS(history)->previous(history, current_input);
}

void
girara_input_history_reset(GiraraInputHistory* history)
{
  g_return_if_fail(GIRARA_IS_INPUT_HISTORY(history) == true);
  GIRARA_INPUT_HISTORY_GET_CLASS(history)->reset(history);
}
