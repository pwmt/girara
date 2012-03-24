/* See LICENSE file for license and copyright information */

#include <string.h>
#include <stdlib.h>
#include <glib/gi18n-lib.h>

#include "commands.h"
#include "datastructures.h"
#include "session.h"
#include "internal.h"
#include "utils.h"
#include "settings.h"
#include "shortcuts.h"

#if GTK_MAJOR_VERSION == 2
#include "gtk2-compat.h"
#endif

/* default commands implementation */
bool
girara_cmd_map_unmap(girara_session_t* session, girara_list_t* argument_list,
    bool unmap)
{
  typedef struct gdk_keyboard_button_s
  {
    char* identifier;
    int keyval;
  } gdk_keyboard_button_t;

  static const gdk_keyboard_button_t gdk_keyboard_buttons[] = {
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
    {"ShiftTab",  GDK_KEY_ISO_Left_Tab},
    {"Up",        GDK_KEY_Up}
  };

  typedef struct gdk_mouse_button_s
  {
    char* identifier;
    int button;
  } gdk_mouse_button_t;

  static const gdk_mouse_button_t gdk_mouse_buttons[] = {
    {"Button1", GIRARA_MOUSE_BUTTON1},
    {"Button2", GIRARA_MOUSE_BUTTON2},
    {"Button3", GIRARA_MOUSE_BUTTON3},
    {"Button4", GIRARA_MOUSE_BUTTON4},
    {"Button5", GIRARA_MOUSE_BUTTON5},
    {"Button6", GIRARA_MOUSE_BUTTON6},
    {"Button7", GIRARA_MOUSE_BUTTON7},
    {"Button8", GIRARA_MOUSE_BUTTON8},
    {"Button9", GIRARA_MOUSE_BUTTON9}
  };

  typedef struct event_type_s
  {
    char* identifier;
    int event;
  } event_type_t;

  static const event_type_t event_types[] = {
    {"motion",       GIRARA_EVENT_MOTION_NOTIFY},
    {"scroll_up",    GIRARA_EVENT_SCROLL_UP},
    {"scroll_down",  GIRARA_EVENT_SCROLL_DOWN},
    {"scroll_left",  GIRARA_EVENT_SCROLL_LEFT},
    {"scroll_right", GIRARA_EVENT_SCROLL_RIGHT}
  };

  typedef struct mouse_event_s
  {
    char* identifier;
    int event;
  } mouse_event_t;

  static const mouse_event_t mouse_events[] = {
    {"button-pressed",   GIRARA_EVENT_BUTTON_PRESS},
    {"2-button-pressed", GIRARA_EVENT_2BUTTON_PRESS},
    {"3-button-pressed", GIRARA_EVENT_2BUTTON_PRESS},
    {"button-released",  GIRARA_EVENT_BUTTON_RELEASE}
  };

  size_t number_of_arguments = girara_list_size(argument_list);

  if (number_of_arguments < ((unmap == true) ? 1 : 2)) {
    if (unmap == true) {
      girara_notify(session, GIRARA_WARNING, _("Usage: unmap <binding>"));
    } else {
      girara_notify(session, GIRARA_WARNING, _("Usage: map <binding> <function>"));
    }
    return false;
  }

  int shortcut_mask                            = 0;
  int shortcut_key                             = 0;
  int shortcut_mouse_button                    = 0;
  girara_mode_t shortcut_mode                  = session->modes.normal;
  char* shortcut_argument_data                 = NULL;
  int shortcut_argument_n                      = 0;
  char* shortcut_buffer_command                = NULL;
  girara_event_type_t event_type               = GIRARA_EVENT_BUTTON_PRESS;
  girara_shortcut_function_t shortcut_function = NULL;
  bool mouse_event                             = false;

  size_t current_command = 0;
  char* tmp              = girara_list_nth(argument_list, current_command);
  size_t tmp_length      = strlen(tmp);

  /* Check first argument for mode */
  bool is_mode = false;
  if (tmp_length >= 3 && tmp[0] == '[' && tmp[tmp_length - 1] == ']') {
    char* tmp_inner = g_strndup(tmp + 1, tmp_length - 2);

    GIRARA_LIST_FOREACH(session->modes.identifiers, girara_mode_string_t*, iter, mode)
      if (!g_strcmp0(tmp_inner, mode->name)) {
        shortcut_mode = mode->index;
        is_mode       = true;
        break;
      }
    GIRARA_LIST_FOREACH_END(session->modes.identifiers, girara_mode_string_t*, iter, mode);

    if (is_mode == false) {
      girara_warning("Unregistered mode specified: %s", tmp_inner);
      girara_notify(session, GIRARA_ERROR, _("Unregistered mode specified: %s"), tmp_inner);
      g_free(tmp_inner);
      return false;
    }
    g_free(tmp_inner);
  }

  unsigned int limit = (unmap == true) ? 1 : 2;
  if (number_of_arguments < limit) {
    girara_warning("Invalid number of arguments passed: %zu instead of at least %u", number_of_arguments, limit);
    girara_notify(session, GIRARA_ERROR,
        _("Invalid number of arguments passed: %zu instead of at least %u"), number_of_arguments, limit);
    return false;
  }

  if (is_mode == true) {
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
          girara_notify(session, GIRARA_ERROR, _("Invalid modifier in %s"), tmp);
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
          if (g_strcmp0(tmp + 2, gdk_keyboard_buttons[i].identifier) == 0) {
            shortcut_key = gdk_keyboard_buttons[i].keyval;
            found = true;
            break;
          }
        }

        for (unsigned int i = 0; i < LENGTH(gdk_mouse_buttons); i++) {
          if (!g_strcmp0(tmp + 2, gdk_mouse_buttons[i].identifier)) {
            shortcut_mouse_button = gdk_mouse_buttons[i].button;
            mouse_event = true;
            found = true;
            break;
          }
        }

        for (unsigned int i = 0; i < LENGTH(event_types); i++) {
          if (!g_strcmp0(tmp + 2, event_types[i].identifier)) {
            event_type = event_types[i].event;
            mouse_event = true;
            found = true;
            break;
          }
        }

        if (found == false) {
          girara_warning("Invalid special key value or mode: %s", tmp);
          girara_notify(session, GIRARA_ERROR, _("Invalid special key value for %s"), tmp);
          g_free(tmp);
          return false;
        }
      }
    /* Possible special key */
    } else {
      bool found = false;
      for (unsigned int i = 0; i < LENGTH(gdk_keyboard_buttons); i++) {
        if (g_strcmp0(tmp, gdk_keyboard_buttons[i].identifier) == 0) {
          shortcut_key = gdk_keyboard_buttons[i].keyval;
          found = true;
          break;
        }
      }

      for (unsigned int i = 0; i < LENGTH(gdk_mouse_buttons); i++) {
        if (!g_strcmp0(tmp, gdk_mouse_buttons[i].identifier)) {
          shortcut_mouse_button = gdk_mouse_buttons[i].button;
          mouse_event = true;
          found = true;
          break;
        }
      }

      for (unsigned int i = 0; i < LENGTH(event_types); i++) {
        if (!g_strcmp0(tmp, event_types[i].identifier)) {
          event_type = event_types[i].event;
          mouse_event = true;
          found = true;
          break;
        }
      }

      if (found == false) {
        girara_warning("Invalid special key value or mode: %s", tmp);
        girara_notify(session, GIRARA_ERROR, _("Invalid special key value or mode %s"), tmp);
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

  /* check for mouse mode */
  bool mouse_mode = false;
  if (unmap == false) {
    if (++current_command < number_of_arguments) {
      tmp = girara_list_nth(argument_list, current_command);
      tmp_length = strlen(tmp);

      if (tmp_length >= 3 && tmp[0] == '[' && tmp[tmp_length - 1] == ']') {
        mouse_mode = true;
        if (mouse_event == false) {
          girara_warning("Mode passed on non-mouse event: %s", tmp);
          return false;
        }

        char* tmp_inner = g_strndup(tmp + 1, tmp_length - 2);

        bool found = false;
        for (unsigned int i = 0; i < LENGTH(mouse_events); i++) {
          if (!g_strcmp0(tmp_inner, mouse_events[i].identifier)) {
            event_type = mouse_events[i].event;
            found = true;
            break;
          }
        }

        if (found == false) {
          girara_warning("Invalid mouse event mode has been passed: %s", tmp_inner);
          g_free(tmp_inner);
          return false;
        }

        g_free(tmp_inner);
      }
    } else {
      girara_warning("Invalid number of arguments passed");
      return false;
    }
  }

  if (unmap == false) {
    limit = (mouse_mode == true) ? 3 : 2;
    if (number_of_arguments < limit) {
      girara_warning("Invalid number of arguments passed: %zu instead of at least %u", number_of_arguments, limit);
      girara_notify(session, GIRARA_ERROR,
          _("Invalid number of arguments passed: %zu instead of at least %u"), number_of_arguments, limit);
      return false;
    }

    if (mouse_mode == true) {
      tmp = girara_list_nth(argument_list, ++current_command);
    }
  }

  /* Check for passed shortcut command */
  if (unmap == false) {
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
      girara_notify(session, GIRARA_ERROR, _("Not a valid shortcut function: %s"), tmp);
      if (shortcut_buffer_command) {
        g_free(shortcut_buffer_command);
      }
      return false;
    }
  }

  /* Check for passed argument */
  if (unmap == false) {
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
  }

  if (mouse_event == false) {
    if (unmap == true) {
      girara_shortcut_remove(session, shortcut_mask, shortcut_key,
          shortcut_buffer_command, shortcut_mode);
    } else {
      girara_shortcut_add(session, shortcut_mask, shortcut_key, shortcut_buffer_command,
          shortcut_function, shortcut_mode, shortcut_argument_n, shortcut_argument_data);
    }
  } else {
    if (unmap == true) {
      girara_mouse_event_remove(session, shortcut_mask, shortcut_mouse_button,
          shortcut_mode);
    } else {
      girara_mouse_event_add(session, shortcut_mask, shortcut_mouse_button,
          shortcut_function, shortcut_mode, event_type, shortcut_argument_n, shortcut_argument_data);
    }
  }

  if (shortcut_buffer_command) {
    g_free(shortcut_buffer_command);
  }

  return true;
}

bool
girara_cmd_map(girara_session_t* session, girara_list_t* argument_list)
{
  return girara_cmd_map_unmap(session, argument_list, false);
}

bool
girara_cmd_unmap(girara_session_t* session, girara_list_t* argument_list)
{
  return girara_cmd_map_unmap(session, argument_list, true);
}


bool
girara_cmd_quit(girara_session_t* session, girara_list_t* UNUSED(argument_list))
{
  girara_argument_t arg = { GIRARA_HIDE, NULL };
  girara_isc_completion(session, &arg, NULL, 0);

  gtk_main_quit();

  return true;
}

bool
girara_cmd_set(girara_session_t* session, girara_list_t* argument_list)
{
  const size_t number_of_arguments = girara_list_size(argument_list);

  if (number_of_arguments == 0) {
    girara_warning("Not enough arguments for :set.");
    girara_notify(session, GIRARA_ERROR, _("Not enough arguments."));
    return false;
  }
  if (number_of_arguments > 2) {
    girara_warning("Too many arguments for :set.");
    girara_notify(session, GIRARA_ERROR, _("Too many arguments."));
    return false;
  }

  /* search for existing setting */
  char* name = (char*) girara_list_nth(argument_list, 0);
  if (name == NULL) {
    return false;
  }

  girara_setting_t* setting = girara_setting_find(session, name);
  if (setting == NULL) {
    girara_warning("Unknown option: %s", name);
    girara_notify(session, GIRARA_ERROR, _("Unknown option: %s"), name);
    return false;
  }

  if (number_of_arguments == 1) {
    /* display setting*/
    switch (girara_setting_get_type(setting)) {
      case BOOLEAN:
      {
        /* for compatibility reasons: toogle the setting */
        bool value = false;
        girara_setting_get_value(setting, &value);
        bool tmp = !value;
        girara_setting_set_value(session, setting, &tmp);
        girara_notify(session, GIRARA_INFO, "%s: %s", name, tmp ? _("true") : _("false"));
        break;
      }
      case FLOAT:
      {
        float value = 0;
        girara_setting_get_value(setting, &value);
        girara_notify(session, GIRARA_INFO, "%s: %f", name, value);
        break;
      }
      case INT:
      {
        int value = 0;
        girara_setting_get_value(setting, &value);
        girara_notify(session, GIRARA_INFO, "%s: %i", name, value);
        break;
      }
      case STRING:
      {
        char* str = NULL;
        girara_setting_get_value(setting, &str);
        girara_notify(session, GIRARA_INFO, "%s: %s", name, str ? str : "(NULL)");
        g_free(str);
        break;
      }
      default:
        return false;
    }
  } else {
    char* value = (char*) girara_list_nth(argument_list, 1);
    if (value == NULL) {
      girara_warning("No value defined for option: %s", name);
      girara_notify(session, GIRARA_ERROR, _("No value defined for option: %s"), name);
      return false;
    }

    /* update value */
    switch (girara_setting_get_type(setting)) {
      case BOOLEAN:
        if (g_strcmp0(value, "false") == 0 || g_strcmp0(value, "0") == 0) {
          bool b = false;
          girara_setting_set_value(session, setting, &b);
        } else if (g_strcmp0(value, "true") == 0 || g_strcmp0(value, "1") == 0) {
          bool b = true;
          girara_setting_set_value(session, setting, &b);
        } else {
          girara_warning("Unknown value for option: %s", name);
          girara_notify(session, GIRARA_ERROR, _("Unknown value for option: %s"), name);
        }
        break;
      case FLOAT:
      {
        float f = strtof(value, NULL);
        girara_setting_set_value(session, setting, &f);
        break;
      }
      case INT:
      {
        int i = atoi(value);
        girara_setting_set_value(session, setting, &i);
        break;
      }
      case STRING:
        girara_setting_set_value(session, setting, value);
        break;
      default:
        return false;
    }
  }

  return true;
}

bool
girara_inputbar_command_add(girara_session_t* session, const char* command,
    const char* abbreviation, girara_command_function_t function,
    girara_completion_function_t completion, const char* description)
{
  g_return_val_if_fail(session  != NULL, false);
  g_return_val_if_fail(command  != NULL, false);
  g_return_val_if_fail(function != NULL, false);

  /* search for existing binding */
  GIRARA_LIST_FOREACH(session->bindings.commands, girara_command_t*, iter, commands_it)
    if (g_strcmp0(commands_it->command, command) == 0) {
      g_free(commands_it->abbr);
      g_free(commands_it->description);

      commands_it->abbr        = abbreviation ? g_strdup(abbreviation) : NULL;
      commands_it->function    = function;
      commands_it->completion  = completion;
      commands_it->description = description ? g_strdup(description) : NULL;

      girara_list_iterator_free(iter);
      return true;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.commands, girara_command_t*, iter, commands_it);

  /* add new inputbar command */
  girara_command_t* new_command = g_slice_new(girara_command_t);

  new_command->command     = g_strdup(command);
  new_command->abbr        = abbreviation ? g_strdup(abbreviation) : NULL;
  new_command->function    = function;
  new_command->completion  = completion;
  new_command->description = description ? g_strdup(description) : NULL;
  girara_list_append(session->bindings.commands, new_command);

  return true;
}

bool
girara_special_command_add(girara_session_t* session, char identifier, girara_inputbar_special_function_t function, bool always, int argument_n, void* argument_data)
{
  g_return_val_if_fail(session  != NULL, false);
  g_return_val_if_fail(function != NULL, false);

  girara_argument_t argument = {argument_n, argument_data};

  /* search for existing special command */
  GIRARA_LIST_FOREACH(session->bindings.special_commands, girara_special_command_t*, iter, scommand_it)
    if (scommand_it->identifier == identifier) {
      scommand_it->function = function;
      scommand_it->always   = always;
      scommand_it->argument = argument;
      girara_list_iterator_free(iter);
      return true;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.special_commands, girara_special_command_t*, iter, scommand_it);

  /* create new special command */
  girara_special_command_t* special_command = g_slice_new(girara_special_command_t);

  special_command->identifier = identifier;
  special_command->function   = function;
  special_command->always     = always;
  special_command->argument   = argument;

  girara_list_append(session->bindings.special_commands, special_command);

  return true;
}

void
girara_special_command_free(girara_special_command_t* special_command)
{
  if (special_command == NULL) {
    return;
  }
  g_slice_free(girara_special_command_t, special_command);
}

void
girara_command_free(girara_command_t* command)
{
  if (command == NULL) {
    return;
  }

  g_free(command->command);
  g_free(command->abbr);
  g_free(command->description);
  g_slice_free(girara_command_t, command);
}

bool
girara_cmd_exec(girara_session_t* session, girara_list_t* argument_list)
{
  char* cmd = NULL;
  girara_setting_get(session, "exec-command", &cmd);
  if (cmd == NULL || strlen(cmd) == 0) {
    girara_warning("exec-command is invalid.");
    girara_notify(session, GIRARA_ERROR, _("exec-command is invalid."));
    g_free(cmd);
    return false;
  }

  GString* command = g_string_new(cmd);
  g_free(cmd);

  GIRARA_LIST_FOREACH(argument_list, char*, iter, value)
    g_string_append_c(command, ' ');
    char* tmp = g_shell_quote(value);
    g_string_append(command, tmp);
    g_free(tmp);
  GIRARA_LIST_FOREACH_END(argument_list, char*, iter, value);

  GError* error = NULL;
  gboolean ret = g_spawn_command_line_async(command->str, &error);
  if (error != NULL) {
    girara_warning("Failed to execute command: %s", error->message);
    girara_notify(session, GIRARA_ERROR, _("Failed to execute command: %s"), error->message);
    g_error_free(error);
  }

  g_string_free(command, TRUE);
  return ret;
}
