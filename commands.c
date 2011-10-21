/* See LICENSE file for license and copyright information */

#include <string.h>
#include <stdlib.h>

#include "girara-commands.h"
#include "girara-datastructures.h"
#include "girara-session.h"
#include "girara-internal.h"

#include "girara.h"

/* default commands implementation */
bool
girara_cmd_map(girara_session_t* session, girara_list_t* argument_list)
{
  typedef struct gdk_keyboard_button_s
  {
    char* identifier;
    int keyval;
  } gdk_keyboard_button_t;

  static const gdk_keyboard_button_t gdk_keyboard_buttons[] = {
#if (GTK_MAJOR_VERSION == 3)
    {"BackSpace", GDK_KEY_BackSpace},
    {"CapsLock",  GDK_KEY_Caps_Lock},
    {"Down",      GDK_KEY_Down},
    {"Esc",       GDK_KEY_Escape},
    {"F10",       GDK_KEY_F10},
    {"F11",       GDK_KEY_F11},
    {"F12",       GDK_KEY_F12},
    {"F1",        GDK_KEY_F1},
    {"F2",        GDK_KEY_F2},
    {"F3",        GDK_KEY_F3},
    {"F4",        GDK_KEY_F4},
    {"F5",        GDK_KEY_F5},
    {"F6",        GDK_KEY_F6},
    {"F7",        GDK_KEY_F7},
    {"F8",        GDK_KEY_F8},
    {"F9",        GDK_KEY_F9},
    {"Left",      GDK_KEY_Left},
    {"PageDown",  GDK_KEY_Page_Down},
    {"PageUp",    GDK_KEY_Page_Up},
    {"Return",    GDK_KEY_Return},
    {"Right",     GDK_KEY_Right},
    {"Space",     GDK_KEY_space},
    {"Super",     GDK_KEY_Super_L},
    {"Tab",       GDK_KEY_Tab},
    {"Up",        GDK_KEY_Up},
#else
    {"BackSpace", GDK_BackSpace},
    {"CapsLock",  GDK_Caps_Lock},
    {"Down",      GDK_Down},
    {"Esc",       GDK_Escape},
    {"F10",       GDK_F10},
    {"F11",       GDK_F11},
    {"F12",       GDK_F12},
    {"F1",        GDK_F1},
    {"F2",        GDK_F2},
    {"F3",        GDK_F3},
    {"F4",        GDK_F4},
    {"F5",        GDK_F5},
    {"F6",        GDK_F6},
    {"F7",        GDK_F7},
    {"F8",        GDK_F8},
    {"F9",        GDK_F9},
    {"Left",      GDK_Left},
    {"PageDown",  GDK_Page_Down},
    {"PageUp",    GDK_Page_Up},
    {"Return",    GDK_Return},
    {"Right",     GDK_Right},
    {"Space",     GDK_space},
    {"Super",     GDK_Super_L},
    {"Tab",       GDK_Tab},
    {"Up",        GDK_Up},
#endif
  };

  typedef struct gdk_mouse_button_s
  {
    char* identifier;
    int button;
  } gdk_mouse_button_t;

  static const gdk_mouse_button_t gdk_mouse_buttons[] = {
    {"Button1", 1},
    {"Button2", 2},
    {"Button3", 3},
    {"Button4", 4},
    {"Button5", 5},
  };

  int number_of_arguments = girara_list_size(argument_list);

  if (number_of_arguments < 2) {
    girara_notify(session, GIRARA_WARNING, "Usage: map <binding> <function>");
    return false;
  }

  int shortcut_mask                            = 0;
  int shortcut_key                             = 0;
  int shortcut_mouse_button                    = 0;
  girara_mode_t shortcut_mode                  = session->modes.normal;
  char* shortcut_argument_data                 = NULL;
  int shortcut_argument_n                      = 0;
  char* shortcut_buffer_command                = NULL;
  girara_shortcut_function_t shortcut_function = NULL;

  int current_command = 0;
  char* tmp      = girara_list_nth(argument_list, current_command);
  int tmp_length = strlen(tmp);

  /* Check first argument for mode */
  bool is_mode = false;
  if (tmp_length >= 3 && tmp[0] == '[' && tmp[tmp_length - 1] == ']') {
    char* tmp_inner            = g_strndup(tmp + 1, tmp_length - 2);

    GIRARA_LIST_FOREACH(session->modes.identifiers, girara_mode_string_t*, iter, mode)
      if (!g_strcmp0(tmp_inner, mode->name)) {
        shortcut_mode = mode->index;
        is_mode       = true;
        break;
      }
    GIRARA_LIST_FOREACH_END(session->modes.identifiers, girara_mode_string_t*, iter, mode);

    if (is_mode == false) {
      girara_warning("Unregistered mode specified: %s", tmp_inner);
      girara_notify(session, GIRARA_ERROR, "Unregistered mode specified: %s", tmp_inner);
      g_free(tmp_inner);
      return false;
    }
    g_free(tmp_inner);
  }

  if (is_mode == true) {
    if (number_of_arguments < 3) {
      girara_warning("Invalid number of arguments passed: %d instead of at least 3", number_of_arguments);
      girara_notify(session, GIRARA_ERROR, "Invalid number of arguments passed: \
          %d instead of at least 3", number_of_arguments);
      return false;
    }
    tmp = girara_list_nth(argument_list, ++current_command);
    tmp_length = strlen(tmp);
  }

  /* Check for multi key shortcut */
  if (tmp_length >= 3 && tmp[0] == '<' && tmp[tmp_length - 1] == '>') {
    tmp        = g_strndup(tmp + 1, tmp_length - 2);
    tmp_length = strlen(tmp);

    /* Multi key shortcut */
    if (strchr(tmp, '-') != NULL && tmp_length > 2) {
      switch (tmp[0]) {
        case 'S':
          shortcut_mask = GDK_SHIFT_MASK;
          break;
        case 'A':
          shortcut_mask = GDK_MOD1_MASK;
          break;
        case 'C':
          shortcut_mask = GDK_CONTROL_MASK;
          break;
        default:
          girara_warning("Invalid modifier in %s", tmp);
          girara_notify(session, GIRARA_ERROR, "Invalid modifier in %s", tmp);
          g_free(tmp);
          return false;
      }

      /* Single key */
      if (tmp_length == 3) {
        shortcut_key = tmp[2];
      /* Possible special key */
      } else {
        bool found = false;
        for (unsigned int i = 0; i < LENGTH(gdk_keyboard_buttons); i++) {
          if (!g_strcmp0(tmp + 2, gdk_keyboard_buttons[i].identifier)) {
            shortcut_key = gdk_keyboard_buttons[i].keyval;
            found = true;
            break;
          }
        }

        for (unsigned int i = 0; i < LENGTH(gdk_mouse_buttons); i++) {
          if (!g_strcmp0(tmp + 2, gdk_mouse_buttons[i].identifier)) {
            shortcut_mouse_button = gdk_mouse_buttons[i].button;
            found = true;
            break;
          }
        }

        if (found == false) {
          girara_warning("Invalid special key value or mode: %s", tmp);
          girara_notify(session, GIRARA_ERROR, "Invalid special key value for %s", tmp);
          g_free(tmp);
          return false;
        }
      }
    /* Possible special key */
    } else {
      bool found = false;
      for (unsigned int i = 0; i < LENGTH(gdk_keyboard_buttons); i++) {
        if (!g_strcmp0(tmp, gdk_keyboard_buttons[i].identifier)) {
          shortcut_key = gdk_keyboard_buttons[i].keyval;
          found = true;
          break;
        }
      }

      for (unsigned int i = 0; i < LENGTH(gdk_mouse_buttons); i++) {
        if (!g_strcmp0(tmp, gdk_mouse_buttons[i].identifier)) {
          shortcut_mouse_button = gdk_mouse_buttons[i].button;
          found = true;
          break;
        }
      }

      if (found == false) {
        girara_warning("Invalid special key value or mode: %s", tmp);
        girara_notify(session, GIRARA_ERROR, "Invalid special key value or mode %s", tmp);
        g_free(tmp);
        return false;
      }
    }

    g_free(tmp);
  /* Single key shortcut */
  } else if (tmp_length == 1) {
    shortcut_key = tmp[0];
  /* Buffer command */
  } else {
    shortcut_buffer_command = g_strdup(tmp);
  }

  /* Check for passed shortcut command */
  if (++current_command < number_of_arguments) {
    tmp = girara_list_nth(argument_list, current_command);

    bool found_mapping = false;
    GIRARA_LIST_FOREACH(session->config.shortcut_mappings, girara_shortcut_mapping_t*, iter, mapping)
      if (!g_strcmp0(tmp, mapping->identifier)) {
        shortcut_function = mapping->function;
        found_mapping = true;
        break;
      }
    GIRARA_LIST_FOREACH_END(session->config.shortcut_mappings, girara_shortcut_mapping_t*, iter, mapping);

    if (found_mapping == false) {
      girara_warning("Not a valid shortcut function: %s", tmp);
      girara_notify(session, GIRARA_ERROR, "Not a valid shortcut function:  %s", tmp);
      if (shortcut_buffer_command) {
        g_free(shortcut_buffer_command);
      }
      return false;
    }
  } else {
    g_free(shortcut_buffer_command);
    return false;
  }

  /* Check for passed argument */
  if (++current_command < number_of_arguments) {
    tmp = (char*) girara_list_nth(argument_list, current_command);

    GIRARA_LIST_FOREACH(session->config.argument_mappings, girara_argument_mapping_t*, iter, mapping)
      if (!g_strcmp0(tmp, mapping->identifier)) {
        shortcut_argument_n = mapping->value;
        break;
      }
    GIRARA_LIST_FOREACH_END(session->config.argument_mappings, girara_argument_mapping_t*, iter, mapping);

    /* If no known argument is passed we save it in the data field */
    if (shortcut_argument_n == 0) {
      shortcut_argument_data = tmp;
    /* If a known argument is passed and there are still more arguments,
     * we save the next one in the data field */
    } else if (++current_command < number_of_arguments) {
      tmp = (char*) girara_list_nth(argument_list, current_command);
      shortcut_argument_data = tmp;
    }
  }

  if (shortcut_mouse_button == 0) {
    girara_shortcut_add(session, shortcut_mask, shortcut_key, shortcut_buffer_command,
        shortcut_function, shortcut_mode, shortcut_argument_n, shortcut_argument_data);
  } else {
    girara_mouse_event_add(session, shortcut_mask, shortcut_mouse_button,
        shortcut_function, shortcut_mode, shortcut_argument_n, shortcut_argument_data);
  }

  if (shortcut_buffer_command) {
    g_free(shortcut_buffer_command);
  }

  return true;
}

bool
girara_cmd_quit(girara_session_t* session, girara_list_t* UNUSED(argument_list))
{
  girara_argument_t arg = { GIRARA_HIDE, NULL };
  girara_isc_completion(session, &arg, 0);

  gtk_main_quit();

  return true;
}

bool
girara_cmd_set(girara_session_t* session, girara_list_t* argument_list)
{
  int number_of_arguments = girara_list_size(argument_list);

  if (number_of_arguments <= 0) {
    return false;
  }

  char* name  = (char*) girara_list_nth(argument_list, 0);
  char* value = (number_of_arguments >= 2) ? ((char*) girara_list_nth(argument_list, 1)) : NULL;

  /* search for existing setting */
  girara_setting_t*       setting = NULL;
  GIRARA_LIST_FOREACH(session->settings, girara_setting_t*, iter, tmp)
    if (!g_strcmp0(name, tmp->name)) {
      setting = tmp;
      break;
    }
  GIRARA_LIST_FOREACH_END(session->settings, girara_setting_t*, iter, tmp);

  if (setting == NULL) {
    girara_warning("Unknown option: %s", name);
    girara_notify(session, GIRARA_ERROR, "Unknown option: %s", name);
    return false;
  }

  /* update value */
  switch (setting->type) {
    case BOOLEAN:
      if (value) {
        if (!g_strcmp0(value, "false")) {
          setting->value.b = false;
        } else if (!g_strcmp0(value, "true")) {
          setting->value.b = true;
        } else {
          girara_warning("Unknown value for option: %s", name);
          girara_notify(session, GIRARA_ERROR, "Unknown value for option: %s", name);
        }
      } else {
        setting->value.b = !setting->value.b;
      }
      break;
    case FLOAT:
      if (value) {
        setting->value.f = strtof(value, NULL);
      } else {
        girara_warning("No value defined for option: %s", name);
        girara_notify(session, GIRARA_ERROR, "No value defined for option: %s", name);
      }
      break;
    case INT:
      if (value) {
        setting->value.i = atoi(value);
      } else {
        girara_warning("No value defined for option: %s", name);
        girara_notify(session, GIRARA_ERROR, "No value defined for option: %s", name);
      }
      break;
    case STRING:
      if (setting->value.s != NULL) {
        g_free(setting->value.s);
        setting->value.s = NULL;
      }
      if (value) {
        setting->value.s = g_strdup(value);
      } else {
        girara_warning("No value defined for option: %s", name);
        girara_notify(session, GIRARA_ERROR, "No value defined for option: %s", name);
      }
      break;
  }

  if (setting->callback != NULL) {
    setting->callback(session, setting);
  }

  return true;
}
