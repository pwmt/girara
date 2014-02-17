/* See LICENSE file for license and copyright information */

#include "input-history.h"
#include "datastructures.h"

G_DEFINE_TYPE(GiraraInputHistory, girara_input_history, G_TYPE_OBJECT)

/**
 * Private data of the settings manager
 */
typedef struct ih_private_s {
  girara_list_t* history; /**< List of stored inputs */
  bool reset; /**< Show history starting from the most recent command */
  size_t current;
  size_t current_match;
  GiraraInputHistoryIO* io;
  char* command_line;
} ih_private_t;

#define GIRARA_INPUT_HISTORY_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GIRARA_TYPE_INPUT_HISTORY, \
                                ih_private_t))

/* Methods */
static void ih_dispose(GObject* object);
static void ih_finalize(GObject* object);
static void ih_set_property(GObject* object, guint prop_id,
    const GValue* value, GParamSpec* pspec);
static void ih_get_property(GObject* object, guint prop_id, GValue* value,
    GParamSpec* pspec);
static void ih_append(GiraraInputHistory* history, const char* input);
static girara_list_t* ih_list(GiraraInputHistory* history);
static const char* ih_next(GiraraInputHistory* history,
    const char* current_input);
static const char* ih_previous(GiraraInputHistory* history,
    const char* current_input);
static void ih_reset(GiraraInputHistory* history);

/* Properties */
enum {
  PROP_0,
  PROP_IO
};

/* Class init */
static void
girara_input_history_class_init(GiraraInputHistoryClass* class)
{
  /* add private members */
  g_type_class_add_private(class, sizeof(ih_private_t));

  /* overwrite methods */
  GObjectClass* object_class = G_OBJECT_CLASS(class);
  object_class->dispose      = ih_dispose;
  object_class->finalize     = ih_finalize;
  object_class->set_property = ih_set_property;
  object_class->get_property = ih_get_property;

  class->append = ih_append;
  class->list = ih_list;
  class->next = ih_next;
  class->previous = ih_previous;
  class->reset = ih_reset;

  /* properties */
  g_object_class_install_property(object_class, PROP_IO,
    g_param_spec_object("io", "history reader/writer",
      "GiraraInputHistoryIO object used to read and write history",
      girara_input_history_io_get_type(),
      G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));
}

/* Object init */
static void
girara_input_history_init(GiraraInputHistory* history)
{
  ih_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(history);
  priv->history = girara_list_new2((girara_free_function_t) g_free);
  priv->reset   = true;
  priv->io      = NULL;
}

/* GObject dispose */
static void
ih_dispose(GObject* object)
{
  ih_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(object);

  g_clear_object(&priv->io);

  G_OBJECT_CLASS(girara_input_history_parent_class)->dispose(object);
}

/* GObject finalize */
static void
ih_finalize(GObject* object)
{
  ih_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(object);
  girara_list_free(priv->history);
  g_free(priv->command_line);

  G_OBJECT_CLASS(girara_input_history_parent_class)->finalize(object);
}

/* GObject set_property */
static void
ih_set_property(GObject* object, guint prop_id, const GValue* value,
    GParamSpec* pspec)
{
  ih_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(object);

  switch (prop_id) {
    case PROP_IO: {
      if (priv->io != NULL) {
        g_object_unref(priv->io);
      }

      gpointer* tmp = g_value_dup_object(value);
      if (tmp != NULL) {
        priv->io = GIRARA_INPUT_HISTORY_IO(tmp);
      } else {
        priv->io = NULL;
      }
      girara_input_history_reset(GIRARA_INPUT_HISTORY(object));
      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}

/* GObject get_property */
static void
ih_get_property(GObject* object, guint prop_id, GValue* value,
    GParamSpec* pspec)
{
  ih_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(object);

  switch (prop_id) {
    case PROP_IO:
      g_value_set_object(value, priv->io);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}

/* Object new */
GiraraInputHistory*
girara_input_history_new(GiraraInputHistoryIO* io)
{
  return GIRARA_INPUT_HISTORY(g_object_new(GIRARA_TYPE_INPUT_HISTORY, "io",
        io, NULL));
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

  void* data = NULL;
  while ((data = girara_list_find(list, (girara_compare_function_t) g_strcmp0, data)) != NULL) {
    girara_list_remove(list, data);
  }

  girara_list_append(list, g_strdup(input));

  ih_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(history);
  if (priv->io != NULL) {
    girara_input_history_io_append(priv->io, input);
  }

  /* begin from the last command when navigating through history */
  girara_input_history_reset(history);
}

static girara_list_t*
ih_list(GiraraInputHistory* history)
{
  ih_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(history);
  return priv->history;
}

static const char*
find_next(GiraraInputHistory* history, const char* current_input, bool next)
{
  ih_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(history);

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

  /* Before moving into the history, save the current command-line. */
  if (priv->current_match == length) {
    g_free(priv->command_line);
    priv->command_line = g_strdup(current_input);
  }

  size_t i = 0;
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
      if (priv->current + 1 >= length) {
        /* At the bottom of the history, return what the command-line was. */
        priv->current_match = length;
        priv->current = priv->current_match;
        return priv->command_line;
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

    /* Only match history items starting with what was on the command-line. */
    if (g_str_has_prefix(command, priv->command_line)) {
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
  ih_private_t* priv = GIRARA_INPUT_HISTORY_GET_PRIVATE(history);
  priv->reset = true;

  if (priv->io != NULL) {
    girara_list_t* list = girara_input_history_list(history);
    if (list == NULL) {
      return;
    }
    girara_list_clear(list);

    girara_list_t* newlist = girara_input_history_io_read(priv->io);
    if (newlist != NULL) {
      GIRARA_LIST_FOREACH(newlist, const char*, iter, data)
        girara_list_append(list, g_strdup(data));
      GIRARA_LIST_FOREACH_END(newlist, const char*, iter, data);
      girara_list_free(newlist);
    }
  }
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
