/* See LICENSE file for license and copyright information */

#include <stdlib.h>

#include "girara.h"
#include "girara-internal.h"

bool
girara_setting_add(girara_session_t* session, char* name, void* value, girara_setting_type_t type, bool init_only, char* description, girara_setting_callback_t callback, void* data)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);

  /* search for existing setting */
  GIRARA_LIST_FOREACH(session->settings, girara_setting_t*, iter, setting)
    if (!g_strcmp0(name, setting->name)) {
      girara_list_iterator_free(iter);
      return FALSE;
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

  switch (type) {
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
      setting->value.s = value ? g_strdup(value) : NULL;
      break;
  }

  girara_list_append(session->settings, setting);
  return TRUE;
}

bool
girara_setting_set(girara_session_t* session, char* name, void* value)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);

  GIRARA_LIST_FOREACH(session->settings, girara_setting_t*, iter, setting)
    if (g_strcmp0(setting->name, name) == 0) {
      girara_list_iterator_free(iter);
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

          return FALSE;
      }

      if (setting->callback != NULL) {
        setting->callback(session, setting);
      }

      return TRUE;
    }
  GIRARA_LIST_FOREACH_END(session->settings, girara_setting_t*, iter, setting);

  return FALSE;
}

void*
girara_setting_get(girara_session_t* session, char* name)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);

  GIRARA_LIST_FOREACH(session->settings, girara_setting_t*, iter, setting)
    if (g_strcmp0(setting->name, name) == 0) {
      girara_list_iterator_next(iter);

      bool *bvalue = NULL;
      float    *fvalue = NULL;
      int      *ivalue = NULL;

      switch(setting->type) {
        case BOOLEAN:
          bvalue = malloc(sizeof(bool));

          if (!bvalue) {
            return NULL;
          }

          *bvalue = setting->value.b;
          return bvalue;
        case FLOAT:
          fvalue = malloc(sizeof(float));

          if (!fvalue) {
            return NULL;
          }

          *fvalue = setting->value.f;
          return fvalue;
        case INT:
          ivalue = malloc(sizeof(int));

          if (!ivalue) {
            return NULL;
          }

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

