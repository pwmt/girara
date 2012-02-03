/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <glib.h>
#include <string.h>

#include "settings.h"
#include "datastructures.h"
#include "completion.h"
#include "session.h"
#include "internal.h"

/**
 * Structure of a settings entry
 */
struct girara_setting_s
{
  char* name; /**< Name of the setting */
  union
  {
    bool b; /**< Boolean */
    int i; /**< Integer */
    float f; /**< Floating number */
    char *s; /**< String */
  } value; /**< Value of the setting */
  girara_setting_type_t type; /**< Type identifier */
  bool init_only; /**< Option can be set only before girara gets initialized */
  char* description; /**< Description of this setting */
  girara_setting_callback_t callback; /**< Callback that gets executed when the value of the setting changes */
  void* data; /**< Arbitary data that can be used by callbacks */
};

void
girara_setting_set_value(girara_session_t* session, girara_setting_t* setting, void* value)
{
  g_return_if_fail(setting && (value || setting->type == STRING));

  switch(setting->type) {
    case BOOLEAN:
      setting->value.b = *((bool *) value);
      break;
    case FLOAT:
      setting->value.f = *((float *) value);
      break;
    case INT:
      setting->value.i = *((int *) value);
      break;
    case STRING:
      if (setting->value.s != NULL) {
        g_free(setting->value.s);
      }
      setting->value.s = value ? g_strdup(value) : NULL;
      break;
    default:
      g_assert(false);
  }

  if (session && setting->callback != NULL) {
    setting->callback(session, setting->name, setting->type, value, setting->data);
  }
}

bool
girara_setting_add(girara_session_t* session, const char* name, void* value, girara_setting_type_t type, bool init_only, const char* description, girara_setting_callback_t callback, void* data)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(name != NULL, false);
  g_return_val_if_fail(type != UNKNOWN, false);
  if (type != STRING && value == NULL) {
    return false;
  }

  /* search for existing setting */
  if (girara_setting_find(session, name) != NULL) {
    return false;
  }

  /* add new setting */
  girara_setting_t* setting = g_slice_new(girara_setting_t);

  setting->name        = g_strdup(name);
  setting->type        = type;
  setting->init_only   = init_only;
  setting->description = description ? g_strdup(description) : NULL;
  setting->callback    = callback;
  setting->data        = data;
  girara_setting_set_value(NULL, setting, value);

  girara_list_append(session->settings, setting);

  return true;
}

bool
girara_setting_set(girara_session_t* session, const char* name, void* value)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(name != NULL, false);

  girara_setting_t* setting = girara_setting_find(session, name);
  if (setting == NULL) {
    return false;
  }

  girara_setting_set_value(session, setting, value);
  return true;
}

bool
girara_setting_get_value(girara_setting_t* setting, void* dest)
{
  g_return_val_if_fail(setting != NULL && dest != NULL, false);

  bool  *bvalue = (bool*) dest;
  float *fvalue = (float*) dest;
  int   *ivalue = (int*) dest;
  char **svalue = (char**) dest;

  switch(setting->type) {
    case BOOLEAN:
      *bvalue = setting->value.b;
      break;
    case FLOAT:
      *fvalue = setting->value.f;
      break;
    case INT:
      *ivalue = setting->value.i;
      break;
    case STRING:
      *svalue = setting->value.s ? g_strdup(setting->value.s) : NULL;
      break;
    default:
      g_assert(false);
  }

  return true;
}

bool
girara_setting_get(girara_session_t* session, const char* name, void* dest)
{
  g_return_val_if_fail(session != NULL && name != NULL && dest != NULL, false);

  girara_setting_t* setting = girara_setting_find(session, name);
  if (setting == NULL) {
    return false;
  }

  return girara_setting_get_value(setting, dest);
}

void
girara_setting_free(girara_setting_t* setting)
{
  if (!setting) {
    return;
  }

  g_free(setting->name);
  g_free(setting->description);
  if (setting->type == STRING) {
    g_free(setting->value.s);
  }
  g_slice_free(girara_setting_t, setting);
}

girara_setting_t*
girara_setting_find(girara_session_t* session, const char* name)
{
  g_return_val_if_fail(session != NULL, NULL);
  g_return_val_if_fail(name != NULL, NULL);

  girara_setting_t* result = NULL;
  GIRARA_LIST_FOREACH(session->settings, girara_setting_t*, iter, setting)
    if (g_strcmp0(setting->name, name) == 0) {
      result = setting;
      break;
    }
  GIRARA_LIST_FOREACH_END(session->settings, girara_setting_t*, iter, setting);

  return result;
}

const char*
girara_setting_get_name(girara_setting_t* setting) {
  g_return_val_if_fail(setting, NULL);
  return setting->name;
}

girara_setting_type_t
girara_setting_get_type(girara_setting_t* setting) {
  g_return_val_if_fail(setting, UNKNOWN);
  return setting->type;
}

girara_completion_t*
girara_cc_set(girara_session_t* session, const char* input)
{
  if (input == NULL) {
    return NULL;
  }

  girara_completion_t* completion  = girara_completion_init();
  if (completion == NULL) {
    return NULL;
  }
  girara_completion_group_t* group = girara_completion_group_create(session, NULL);
  if (group == NULL) {
    girara_completion_free(completion);
    return NULL;
  }
  girara_completion_add_group(completion, group);

  unsigned int input_length = strlen(input);

  GIRARA_LIST_FOREACH(session->settings, girara_setting_t*, iter, setting)
    if ((setting->init_only == false) && (input_length <= strlen(setting->name)) &&
        !strncmp(input, setting->name, input_length)) {
      girara_completion_group_add_element(group, setting->name, setting->description);
    }
  GIRARA_LIST_FOREACH_END(session->settings, girara_setting_t*, iter, setting);

  return completion;
}
