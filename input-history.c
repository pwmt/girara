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
static void girara_input_history_finalize(GObject* object);

/**
 * Class init
 */
static void
girara_input_history_class_init(GiraraInputHistoryClass* class)
{
  /* add private members */
  g_type_class_add_private(class, sizeof(girara_input_history_private_t));

  /* overwrite methods */
  GObjectClass* object_class = G_OBJECT_CLASS(class);
  object_class->finalize     = girara_input_history_finalize;
}

/**
 * Object init
 */
static void
girara_input_history_init(GiraraInputHistory* history)
{
  girara_input_history_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(history);
  priv->history = girara_list_new2((girara_free_function_t) g_free);
  priv->reset = true;
}

/**
 * Object finalize
 */
static void
girara_input_history_finalize(GObject* object)
{
  girara_input_history_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(object);
  girara_list_free(priv->history);

  G_OBJECT_CLASS(girara_input_history_parent_class)->finalize(object);
}

GiraraInputHistory*
girara_input_history_new(void)
{
  return GIRARA_INPUT_HISTORY(g_object_new(GIRARA_TYPE_INPUT_HISTORY, 0));
}

void
girara_input_history_append(GiraraInputHistory* history, const char* input)
{
  g_return_if_fail(GIRARA_IS_INPUT_HISTORY(history) == true);
  girara_input_history_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(history);

  if (input == NULL) {
    return;
  }

  GIRARA_LIST_FOREACH(priv->history, char*, iter, data)
    if (g_strcmp0(input, data) == 0) {
        girara_list_remove(priv->history, data);
    }
  GIRARA_LIST_FOREACH_END(priv->history, char*, iter, data);

  girara_list_append(priv->history, g_strdup(input));

  /* begin from the last command when navigating through history */
  priv->reset = true;
}

static const char*
find_next(GiraraInputHistory* history, const char* current_input, bool next)
{
  girara_input_history_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(history);

  size_t length = girara_list_size(priv->history);
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

    command = girara_list_nth(priv->history, priv->current);
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

const char*
girara_input_history_next(GiraraInputHistory* history,
    const char* current_input)
{
  g_return_val_if_fail(GIRARA_IS_INPUT_HISTORY(history) == true, NULL);
  return find_next(history, current_input, true);
}

const char*
girara_input_history_previous(GiraraInputHistory* history,
    const char* current_input)
{
  g_return_val_if_fail(GIRARA_IS_INPUT_HISTORY(history) == true, NULL);
  return find_next(history, current_input, false);
}

void
girara_input_history_reset(GiraraInputHistory* history)
{
  g_return_if_fail(GIRARA_IS_INPUT_HISTORY(history) == true);
  girara_input_history_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(history);
  priv->reset = true;
}

