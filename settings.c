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
  girara_setting_t* settings_it = session->settings;
  if (settings_it) {
    if (!g_strcmp0(name, settings_it->name)) {
      return FALSE;
    }

    while (settings_it->next) {
      if (!g_strcmp0(name, settings_it->next->name)) {
        return FALSE;
      }

      settings_it = settings_it->next;
    }
  }

  /* add new setting */
  girara_setting_t* setting = g_slice_new(girara_setting_t);

  setting->name        = g_strdup(name);
  setting->type        = type;
  setting->init_only   = init_only;
  setting->description = description ? g_strdup(description) : NULL;
  setting->callback    = callback;
  setting->data        = data;
  setting->next        = NULL;

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

  if (settings_it) {
    settings_it->next = setting;
  } else {
    session->settings = setting;
  }

  return TRUE;
}

bool
girara_setting_set(girara_session_t* session, char* name, void* value)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);

  for (girara_setting_t* setting = session->settings; setting != NULL; setting = setting->next) {
    if (g_strcmp0(setting->name, name) != 0) {
      continue;
    }

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

        setting->value.s = g_strdup(value);
        break;
      default:
        return FALSE;
    }

    if (setting->callback != NULL) {
      setting->callback(session, setting);
    }

    return TRUE;
  }

  return FALSE;
}

void*
girara_setting_get(girara_session_t* session, char* name)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);

  for (girara_setting_t* setting = session->settings; setting != NULL; setting = setting->next) {
    if (g_strcmp0(setting->name, name) != 0) {
      continue;
    }

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

  return NULL;
}
