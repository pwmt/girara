/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <glib.h>
#include <string.h>

#include "settings.h"
#include "datastructures.h"
#include "completion.h"
#include "session.h"
#include "internal.h"

static void
set_value(girara_setting_t* setting, void* value)
{
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
}

bool
girara_setting_add(girara_session_t* session, const char* name, void* value, girara_setting_type_t type, bool init_only, const char* description, girara_setting_callback_t callback, void* data)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(name != NULL, false);

  /* search for existing setting */
  GIRARA_LIST_FOREACH(session->settings, girara_setting_t*, iter, setting)
    if (!g_strcmp0(name, setting->name)) {
      girara_list_iterator_free(iter);
      return false;
    }
  GIRARA_LIST_FOREACH_END(session->settings, girara_setting_t*, iter, setting);

  /* add new setting */
  girara_setting_t* setting = g_slice_new(girara_setting_t);

  setting->name        = g_strdup(name);
  setting->type        = type;
  setting->init_only   = init_only;
  setting->description = description ? g_strdup(description) : NULL;
  setting->callback    = callback;
  setting->data        = data;
  set_value(setting, value);

  girara_list_append(session->settings, setting);

  return true;
}

bool
girara_setting_set(girara_session_t* session, const char* name, void* value)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(name != NULL, false);

  GIRARA_LIST_FOREACH(session->settings, girara_setting_t*, iter, setting)
    if (g_strcmp0(setting->name, name) == 0) {
      girara_list_iterator_free(iter);
      set_value(setting, value);

      if (setting->callback != NULL) {
        setting->callback(session, setting->name, setting->type, value, setting->data);
      }

      return true;
    }
  GIRARA_LIST_FOREACH_END(session->settings, girara_setting_t*, iter, setting);

  return false;
}

void*
girara_setting_get(girara_session_t* session, const char* name)
{
  g_return_val_if_fail(session != NULL, NULL);
  g_return_val_if_fail(name != NULL, NULL);

  GIRARA_LIST_FOREACH(session->settings, girara_setting_t*, iter, setting)
    if (g_strcmp0(setting->name, name) == 0) {
      girara_list_iterator_free(iter);

      bool  *bvalue = NULL;
      float *fvalue = NULL;
      int   *ivalue = NULL;

      switch(setting->type) {
        case BOOLEAN:
          bvalue = g_malloc0(sizeof(bool));
          *bvalue = setting->value.b;
          return bvalue;
        case FLOAT:
          fvalue = g_malloc0(sizeof(float));
          *fvalue = setting->value.f;
          return fvalue;
        case INT:
          ivalue = g_malloc0(sizeof(int));
          *ivalue = setting->value.i;
          return ivalue;
        case STRING:
          return g_strdup(setting->value.s);
        default:
          return NULL;
      }
    }
  GIRARA_LIST_FOREACH_END(session->settings, girara_setting_t*, iter, setting);

  return NULL;
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
