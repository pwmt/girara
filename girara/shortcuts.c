/* SPDX-License-Identifier: Zlib */

#include "shortcuts.h"

#include "datastructures.h"
#include "input-history.h"
#include "internal.h"
#include "log.h"
#include "session.h"
#include "settings.h"

#include <gtk/gtk.h>
#include <string.h>

static void girara_toggle_widget_visibility(GtkWidget* widget);

bool
girara_shortcut_add(girara_session_t* session, guint modifier, guint key, const char* buffer, girara_shortcut_function_t function, girara_mode_t mode, int argument_n, void* argument_data)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(buffer || key || modifier, false);
  g_return_val_if_fail(function != NULL, false);

  girara_argument_t argument = {argument_n, (argument_data != NULL) ?
    g_strdup(argument_data) : NULL};

  /* search for existing binding */
  bool found_existing_shortcut = false;
  GIRARA_LIST_FOREACH_BODY_WITH_ITER(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcuts_it,
    if (((shortcuts_it->mask == modifier && shortcuts_it->key == key && (modifier != 0 || key != 0)) ||
       (buffer && shortcuts_it->buffered_command && !g_strcmp0(shortcuts_it->buffered_command, buffer)))
        && ((shortcuts_it->mode == mode) || (mode == 0)))
    {
      if (shortcuts_it->argument.data != NULL) {
        g_free(shortcuts_it->argument.data);
      }

      shortcuts_it->function  = function;
      shortcuts_it->argument  = argument;
      found_existing_shortcut = true;

      if (mode != 0) {
        girara_list_iterator_free(iter);
        return true;
      }
    }
  );

  if (found_existing_shortcut == true) {
    return true;
  }

  /* add new shortcut */
  girara_shortcut_t* shortcut = g_slice_new(girara_shortcut_t);

  shortcut->mask             = modifier;
  shortcut->key              = key;
  shortcut->buffered_command = g_strdup(buffer);
  shortcut->function         = function;
  shortcut->mode             = mode;
  shortcut->argument         = argument;
  girara_list_append(session->bindings.shortcuts, shortcut);

  return true;
}

bool
girara_shortcut_remove(girara_session_t* session, guint modifier, guint key, const char* buffer, girara_mode_t mode)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(buffer || key || modifier, false);

  bool handled = false;
  /* search for existing binding */
  GIRARA_LIST_FOREACH_BODY(session->bindings.shortcuts, girara_shortcut_t*, shortcuts_it,
    if (((shortcuts_it->mask == modifier && shortcuts_it->key == key && (modifier != 0 || key != 0)) ||
       (buffer && shortcuts_it->buffered_command && !g_strcmp0(shortcuts_it->buffered_command, buffer)))
        && shortcuts_it->mode == mode) {
      girara_list_remove(session->bindings.shortcuts, shortcuts_it);
      handled = true;
      break;
    }
  );

  return handled;
}

void
girara_shortcut_free(girara_shortcut_t* shortcut)
{
  g_return_if_fail(shortcut != NULL);
  g_free(shortcut->buffered_command);
  g_free(shortcut->argument.data);
  g_slice_free(girara_shortcut_t, shortcut);
}

bool
girara_inputbar_shortcut_add(girara_session_t* session, guint modifier, guint key, girara_shortcut_function_t function, int argument_n, void* argument_data)
{
  g_return_val_if_fail(session  != NULL, false);
  g_return_val_if_fail(function != NULL, false);

  girara_argument_t argument = {argument_n, argument_data};
  bool found = false;

  /* search for existing special command */
  GIRARA_LIST_FOREACH_BODY(session->bindings.inputbar_shortcuts, girara_inputbar_shortcut_t*, inp_sh_it,
    if (inp_sh_it->mask == modifier && inp_sh_it->key == key) {
      inp_sh_it->function = function;
      inp_sh_it->argument = argument;

      found = true;
      break;
    }
  );

  if (found == false) {
    /* create new inputbar shortcut */
    girara_inputbar_shortcut_t* inputbar_shortcut = g_slice_new(girara_inputbar_shortcut_t);

    inputbar_shortcut->mask     = modifier;
    inputbar_shortcut->key      = key;
    inputbar_shortcut->function = function;
    inputbar_shortcut->argument = argument;

    girara_list_append(session->bindings.inputbar_shortcuts, inputbar_shortcut);
  }

  return true;
}

bool
girara_inputbar_shortcut_remove(girara_session_t* session, guint modifier, guint key)
{
  g_return_val_if_fail(session  != NULL, false);

  /* search for existing special command */
  GIRARA_LIST_FOREACH_BODY(session->bindings.inputbar_shortcuts, girara_inputbar_shortcut_t*, inp_sh_it,
    if (inp_sh_it->mask == modifier && inp_sh_it->key == key) {
      girara_list_remove(session->bindings.inputbar_shortcuts, inp_sh_it);
      break;
    }
  );

  return true;
}

void
girara_inputbar_shortcut_free(girara_inputbar_shortcut_t* inputbar_shortcut)
{
  g_slice_free(girara_inputbar_shortcut_t, inputbar_shortcut);
}

bool
girara_isc_activate(girara_session_t* session, girara_argument_t* UNUSED(argument), girara_event_t* UNUSED(event), unsigned int UNUSED(t))
{
    girara_callback_inputbar_activate(session->gtk.inputbar_entry, session);
    return true;
}

bool
girara_isc_abort(girara_session_t* session, girara_argument_t* UNUSED(argument), girara_event_t* UNUSED(event), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  /* hide completion */
  girara_argument_t arg = { GIRARA_HIDE, NULL };
  girara_isc_completion(session, &arg, NULL, 0);

  /* clear inputbar */
  gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar_entry), 0, -1);

  /* grab view */
  gtk_widget_grab_focus(GTK_WIDGET(session->gtk.view));

  /* hide inputbar */
  gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar_dialog));
  if (session->global.autohide_inputbar == true) {
    gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar));
  }

  /* Begin from the last command when navigating through history */
  girara_input_history_reset(session->command_history);

  /* reset custom functions */
  session->signals.inputbar_custom_activate        = NULL;
  session->signals.inputbar_custom_key_press_event = NULL;
  gtk_entry_set_visibility(session->gtk.inputbar_entry, TRUE);

  return true;
}

bool
girara_isc_string_manipulation(girara_session_t* session, girara_argument_t* argument, girara_event_t* UNUSED(event), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  gchar *separator = NULL;
  girara_setting_get(session, "word-separator", &separator);
  gchar *input  = gtk_editable_get_chars(GTK_EDITABLE(session->gtk.inputbar_entry), 0, -1);
  int    length = strlen(input);
  int pos       = gtk_editable_get_position(GTK_EDITABLE(session->gtk.inputbar_entry));
  int i;

  switch (argument->n) {
    case GIRARA_DELETE_LAST_WORD:
      if (pos == 1 && (input[0] == ':' || input[0] == '/')) {
        break;
      }
      if (pos == 0) {
        break;
      }

      i = pos - 1;

      /* remove trailing spaces */
      for (; i >= 0 && input[i] == ' '; i--) {
      }

      /* find the beginning of the word */
      while ((i == (pos - 1)) || ((i > 0) && separator != NULL && !strchr(separator, input[i]))) {
        i--;
      }

      gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar_entry),  i + 1, pos);
      gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar_entry), i + 1);
      break;
    case GIRARA_DELETE_LAST_CHAR:
      if (pos == 1 && (input[0] == ':' || input[0] == '/')) {
        if (length != 1) break;
        else girara_isc_abort(session, argument, NULL, 0);
      }
      if (length == 0 && pos == 0) {
        girara_isc_abort(session, argument, NULL, 0);
      }
      gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar_entry), pos - 1, pos);
      break;
    case GIRARA_DELETE_TO_LINE_START:
      gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar_entry), 1, pos);
      break;
    case GIRARA_NEXT_CHAR:
      gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar_entry), pos + 1);
      break;
    case GIRARA_PREVIOUS_CHAR:
      gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar_entry), (pos == 1) ? 1 : pos - 1);
      break;
    case GIRARA_DELETE_CURR_CHAR:
      if (length != 1 && pos == 0 && (input[0] == ':' || input[0] == '/')){
        break;
      }
      if(length == 1 && pos == 0) {
        girara_isc_abort(session, argument, NULL, 0);
      }
      gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar_entry), pos, pos + 1);
      break;
    case GIRARA_DELETE_TO_LINE_END:
      gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar_entry), pos, length);
      break;
    case GIRARA_GOTO_START:
      gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar_entry), 1);
      break;
    case GIRARA_GOTO_END:
      gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar_entry), -1);
      break;
  }

  g_free(separator);
  g_free(input);

  return false;
}

bool
girara_isc_command_history(girara_session_t* session, girara_argument_t*
    argument, girara_event_t* UNUSED(event), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  char* temp = gtk_editable_get_chars(GTK_EDITABLE(session->gtk.inputbar_entry), 0, -1);
  const char* command = argument->n == GIRARA_NEXT ?
    girara_input_history_next(session->command_history, temp) :
    girara_input_history_previous(session->command_history, temp);
  g_free(temp);

  if (command != NULL) {
    gtk_entry_set_text(session->gtk.inputbar_entry, command);
    gtk_widget_grab_focus(GTK_WIDGET(session->gtk.inputbar_entry));
    gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar_entry), -1);
  }

  return true;
}

/* default shortcut implementation */
bool
girara_sc_focus_inputbar(girara_session_t* session, girara_argument_t* argument, girara_event_t* UNUSED(event), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->gtk.inputbar_entry != NULL, false);

  if (gtk_widget_get_visible(GTK_WIDGET(session->gtk.inputbar)) == false) {
    gtk_widget_show(GTK_WIDGET(session->gtk.inputbar));
  }

  if (gtk_widget_get_visible(GTK_WIDGET(session->gtk.notification_area)) == true) {
    gtk_widget_hide(GTK_WIDGET(session->gtk.notification_area));
  }

  gtk_widget_grab_focus(GTK_WIDGET(session->gtk.inputbar_entry));

  if (argument != NULL && argument->data != NULL) {
    gtk_entry_set_text(session->gtk.inputbar_entry, (char*) argument->data);

    /* we save the X clipboard that will be clear by "grab_focus" */
    gchar* x_clipboard_text = gtk_clipboard_wait_for_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY));

    gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar_entry), -1);

    if (x_clipboard_text != NULL) {
      /* we reset the X clipboard with saved text */
      gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY), x_clipboard_text, -1);
      g_free(x_clipboard_text);
    }
  }

  return true;
}

bool
girara_sc_abort(girara_session_t* session, girara_argument_t* UNUSED(argument), girara_event_t* UNUSED(event), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  girara_isc_abort(session, NULL, NULL, 0);

  gtk_widget_hide(GTK_WIDGET(session->gtk.notification_area));

  if (session->global.autohide_inputbar == false) {
    gtk_widget_show(GTK_WIDGET(session->gtk.inputbar));
  }

  return false;
}

bool
girara_sc_quit(girara_session_t* session, girara_argument_t* UNUSED(argument), girara_event_t* UNUSED(event), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  girara_argument_t arg = { GIRARA_HIDE, NULL };
  girara_isc_completion(session, &arg, NULL, 0);

  gtk_main_quit();

  return false;
}

static void
girara_toggle_widget_visibility(GtkWidget* widget)
{
  if (widget == NULL) {
    return;
  }

  if (gtk_widget_get_visible(widget) == TRUE) {
    gtk_widget_hide(widget);
  } else {
    gtk_widget_show(widget);
  }
}

bool
girara_sc_toggle_inputbar(girara_session_t* session, girara_argument_t* UNUSED(argument), girara_event_t* UNUSED(event), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  girara_toggle_widget_visibility(GTK_WIDGET(session->gtk.inputbar));

  return true;
}

bool
girara_sc_toggle_statusbar(girara_session_t* session, girara_argument_t* UNUSED(argument), girara_event_t* UNUSED(event), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  girara_toggle_widget_visibility(GTK_WIDGET(session->gtk.statusbar));

  return true;
}

static girara_list_t*
argument_to_argument_list(girara_argument_t* argument) {
  girara_list_t* argument_list = girara_list_new();
  if (argument_list == NULL) {
    return NULL;
  }

  gchar** argv = NULL;
  gint argc    = 0;

  girara_list_set_free_function(argument_list, g_free);
  if (g_shell_parse_argv((const gchar*) argument->data, &argc, &argv, NULL) != FALSE) {
    for (int i = 0; i < argc; i++) {
      char* arg = g_strdup(argv[i]);
      girara_list_append(argument_list, arg);
    }

    return argument_list;
  }

  girara_list_free(argument_list);
  return NULL;
}

bool
girara_sc_set(girara_session_t* session, girara_argument_t* argument, girara_event_t* UNUSED(event), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  if (argument == NULL || argument->data == NULL) {
    return false;
  }

  /* create argument list */
  girara_list_t* argument_list = argument_to_argument_list(argument);
  if (argument_list == NULL) {
    return false;
  }

  /* call set */
  girara_cmd_set(session, argument_list);

  /* cleanup */
  girara_list_free(argument_list);

  return false;
}

bool
girara_sc_exec(girara_session_t* session, girara_argument_t* argument, girara_event_t* UNUSED(event), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  if (argument == NULL || argument->data == NULL) {
    return false;
  }

  /* create argument list */
  girara_list_t* argument_list = argument_to_argument_list(argument);
  if (argument_list == NULL) {
    return false;
  }

  /* call exec */
  girara_cmd_exec(session, argument_list);

  /* cleanup */
  girara_list_free(argument_list);

  return false;
}

static bool
simulate_key_press(girara_session_t* session, int state, int key)
{
  if (session == NULL || session->gtk.box == NULL) {
    return false;
  }

  GdkEvent* event = gdk_event_new(GDK_KEY_PRESS);

  event->any.type       = GDK_KEY_PRESS;
  event->key.window     = g_object_ref(gtk_widget_get_parent_window(GTK_WIDGET(session->gtk.box)));
  event->key.send_event = false;
  event->key.time       = GDK_CURRENT_TIME;
  event->key.state      = state;
  event->key.keyval     = key;

  GdkDisplay* display = gtk_widget_get_display(GTK_WIDGET(session->gtk.box));
  GdkKeymapKey* keys  = NULL;
  gint number_of_keys = 0;

  if (gdk_keymap_get_entries_for_keyval(gdk_keymap_get_for_display(display),
        event->key.keyval, &keys, &number_of_keys) == FALSE) {
    gdk_event_free(event);
    return false;
  }

  event->key.hardware_keycode = keys[0].keycode;
  event->key.group            = keys[0].group;

  g_free(keys);

  gdk_event_put(event);
  gdk_event_free(event);

  gtk_main_iteration_do(FALSE);

  return true;
}

static int
update_state_by_keyval(int state, int keyval)
{
  /* The following is probably not true for some keyboard layouts. */
  if ((keyval >= '!' && keyval <= '/')
      || (keyval >= ':' && keyval <= '@')
      || (keyval >= '[' && keyval <= '`')
      || (keyval >= '{' && keyval <= '~')
      ) {
    state |= GDK_SHIFT_MASK;
  }

  return state;
}

bool
girara_sc_feedkeys(girara_session_t* session, girara_argument_t* argument,
    girara_event_t* UNUSED(event), unsigned int t)
{
  if (session == NULL || argument == NULL) {
    return false;
  }

  if (g_mutex_trylock(&session->private_data->feedkeys_mutex) == FALSE) {
    girara_error("Recursive use of feedkeys detected. Aborting evaluation.");
    return false;
  }

  typedef struct gdk_keyboard_button_s {
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
    {"Home",      GDK_KEY_Home},
    {"End",       GDK_KEY_End},
    {"Return",    GDK_KEY_Return},
    {"Right",     GDK_KEY_Right},
    {"Space",     GDK_KEY_space},
    {"Super",     GDK_KEY_Super_L},
    {"Tab",       GDK_KEY_Tab},
    {"ShiftTab",  GDK_KEY_ISO_Left_Tab},
    {"Up",        GDK_KEY_Up}
  };

  char* input               = (char*) argument->data;
  unsigned int input_length = strlen(input);

  t = MAX(1, t);
  for (unsigned int c = 0; c < t; c++) {
    for (unsigned i = 0; i < input_length; i++) {
      int state  = 0;
      int keyval = input[i];

      /* possible special button */
      if ((input_length - i) >= 3 && input[i] == '<') {
        char* end = strchr(input + i, '>');
        if (end == NULL) {
          goto single_key;
        }

        const int length = end - (input + i) - 1;
        char* tmp  = g_strndup(input + i + 1, length);
        bool found = false;

        /* Multi key shortcut */
        if (length > 2 && tmp[1] == '-') {
          switch (tmp[0]) {
            case 'S':
              state = GDK_SHIFT_MASK;
              break;
            case 'A':
              state = GDK_MOD1_MASK;
              break;
            case 'C':
              state = GDK_CONTROL_MASK;
              break;
            default:
              break;
          }

          if (length == 3) {
            keyval = tmp[2];
            found  = true;
          } else {
            for (unsigned int j = 0; j < LENGTH(gdk_keyboard_buttons); ++j) {
              if (g_strcmp0(tmp + 2, gdk_keyboard_buttons[j].identifier) == 0) {
                keyval = gdk_keyboard_buttons[j].keyval;
                found = true;
                break;
              }
            }
          }
        /* Possible special key */
        } else {
          for (unsigned int j = 0; j < LENGTH(gdk_keyboard_buttons); ++j) {
            if (g_strcmp0(tmp, gdk_keyboard_buttons[j].identifier) == 0) {
              keyval = gdk_keyboard_buttons[j].keyval;
              found = true;
              break;
            }
          }
        }

        g_free(tmp);

        /* parsed special key */
        if (found == true) {
          i += length + 1;
        }
      }

single_key:
      state = update_state_by_keyval(state, keyval);
      simulate_key_press(session, state, keyval);
    }
  }

  g_mutex_unlock(&session->private_data->feedkeys_mutex);
  return true;
}

bool
girara_shortcut_mapping_add(girara_session_t* session, const char* identifier, girara_shortcut_function_t function)
{
  g_return_val_if_fail(session  != NULL, false);

  if (function == NULL || identifier == NULL) {
    return false;
  }

  girara_session_private_t* session_private = session->private_data;
  bool found = false;

  GIRARA_LIST_FOREACH_BODY(session_private->config.shortcut_mappings, girara_shortcut_mapping_t*, data,
    if (g_strcmp0(data->identifier, identifier) == 0) {
      data->function = function;
      found = true;
      break;
    }
  );

  if (found == false) {
    /* add new config handle */
    girara_shortcut_mapping_t* mapping = g_slice_new(girara_shortcut_mapping_t);

    mapping->identifier = g_strdup(identifier);
    mapping->function   = function;
    girara_list_append(session_private->config.shortcut_mappings, mapping);
  }

  return true;
}

void
girara_shortcut_mapping_free(girara_shortcut_mapping_t* mapping)
{
  if (mapping == NULL) {
    return;
  }

  g_free(mapping->identifier);
  g_slice_free(girara_shortcut_mapping_t, mapping);
}

bool
girara_argument_mapping_add(girara_session_t* session, const char* identifier, int value)
{
  g_return_val_if_fail(session  != NULL, false);

  if (identifier == NULL) {
    return false;
  }

  girara_session_private_t* session_private = session->private_data;
  bool found = false;

  GIRARA_LIST_FOREACH_BODY(session_private->config.argument_mappings, girara_argument_mapping_t*, mapping,
    if (g_strcmp0(mapping->identifier, identifier) == 0) {
      mapping->value = value;
      found = true;
      break;
    }
  );

  if (found == false) {
    /* add new config handle */
    girara_argument_mapping_t* mapping = g_slice_new(girara_argument_mapping_t);

    mapping->identifier = g_strdup(identifier);
    mapping->value      = value;
    girara_list_append(session_private->config.argument_mappings, mapping);
  }

  return true;
}

void
girara_argument_mapping_free(girara_argument_mapping_t* argument_mapping)
{
  if (argument_mapping == NULL) {
    return;
  }

  g_free(argument_mapping->identifier);
  g_slice_free(girara_argument_mapping_t, argument_mapping);
}

bool
girara_mouse_event_add(girara_session_t* session, guint mask, guint button,
    girara_shortcut_function_t function, girara_mode_t mode, girara_event_type_t
    event_type, int argument_n, void* argument_data)
{
  g_return_val_if_fail(session  != NULL, false);
  g_return_val_if_fail(function != NULL, false);

  girara_argument_t argument = {argument_n, argument_data};
  bool found = false;

  /* search for existing binding */
  GIRARA_LIST_FOREACH_BODY(session->bindings.mouse_events, girara_mouse_event_t*, me_it,
    if (me_it->mask == mask && me_it->button == button &&
       me_it->mode == mode && me_it->event_type == event_type) {
      me_it->function = function;
      me_it->argument = argument;

      found = true;
      break;
    }
  );

  if (found == false) {
    /* add new mouse event */
    girara_mouse_event_t* mouse_event = g_slice_new(girara_mouse_event_t);

    mouse_event->mask       = mask;
    mouse_event->button     = button;
    mouse_event->function   = function;
    mouse_event->mode       = mode;
    mouse_event->event_type = event_type;
    mouse_event->argument   = argument;
    girara_list_append(session->bindings.mouse_events, mouse_event);
  }

  return true;
}

bool
girara_mouse_event_remove(girara_session_t* session, guint mask, guint button, girara_mode_t mode)
{
  g_return_val_if_fail(session != NULL, false);

  bool found = false;
  /* search for existing binding */
  GIRARA_LIST_FOREACH_BODY(session->bindings.mouse_events, girara_mouse_event_t*, me_it,
    if (me_it->mask == mask && me_it->button == button &&
       me_it->mode == mode) {
      girara_list_remove(session->bindings.mouse_events, me_it);
      found = true;
      break;
    }
  );

  return found;
}

void
girara_mouse_event_free(girara_mouse_event_t* mouse_event)
{
  if (mouse_event == NULL) {
    return;
  }
  g_slice_free(girara_mouse_event_t, mouse_event);
}
