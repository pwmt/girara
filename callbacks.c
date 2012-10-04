/* See LICENSE file for license and copyright information */

#include "callbacks.h"
#include "datastructures.h"
#include "session.h"
#include "shortcuts.h"
#include <string.h>
#include <glib/gi18n-lib.h>

#include "internal.h"
#if GTK_MAJOR_VERSION == 2
#include "gtk2-compat.h"
#endif

static const guint ALL_ACCELS_MASK = GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK;
static const guint MOUSE_MASK = GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK |
  GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK | GDK_BUTTON4_MASK | GDK_BUTTON5_MASK;

static bool
clean_mask(guint hardware_keycode, GdkModifierType state, gint group, guint* clean, guint* keyval)
{
  GdkModifierType consumed = 0;
  if ((gdk_keymap_translate_keyboard_state(
        gdk_keymap_get_default(),
        hardware_keycode,
        state, group,
        keyval,
        NULL,
        NULL,
        &consumed)
      ) == FALSE) {
    return false;
  }

  if (clean != NULL) {
    *clean = state & ~consumed & ALL_ACCELS_MASK;
  }

  /* numpad numbers */
  switch (*keyval) {
    case GDK_KEY_KP_0:
      *keyval = GDK_KEY_0;
      break;
    case GDK_KEY_KP_1:
      *keyval = GDK_KEY_1;
      break;
    case GDK_KEY_KP_2:
      *keyval = GDK_KEY_2;
      break;
    case GDK_KEY_KP_3:
      *keyval = GDK_KEY_3;
      break;
    case GDK_KEY_KP_4:
      *keyval = GDK_KEY_4;
      break;
    case GDK_KEY_KP_5:
      *keyval = GDK_KEY_5;
      break;
    case GDK_KEY_KP_6:
      *keyval = GDK_KEY_6;
      break;
    case GDK_KEY_KP_7:
      *keyval = GDK_KEY_7;
      break;
    case GDK_KEY_KP_8:
      *keyval = GDK_KEY_8;
      break;
    case GDK_KEY_KP_9:
      *keyval = GDK_KEY_9;
      break;
  }

  return true;
}

/* callback implementation */
bool
girara_callback_view_key_press_event(GtkWidget* UNUSED(widget),
    GdkEventKey* event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);

  guint clean  = 0;
  guint keyval = 0;

  if (clean_mask(event->hardware_keycode, event->state, event->group, &clean, &keyval) == false) {
    return false;
  }

  /* prepare event */
  GIRARA_LIST_FOREACH(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcut)
    if (session->buffer.command != NULL) {
      break;
    }

    if ( keyval == shortcut->key
      && (clean == shortcut->mask || (shortcut->key >= 0x21
      && shortcut->key <= 0x7E && clean == GDK_SHIFT_MASK))
      && (session->modes.current_mode == shortcut->mode || shortcut->mode == 0)
      && shortcut->function != NULL
      )
    {
      int t = (session->buffer.n > 0) ? session->buffer.n : 1;
      for (int i = 0; i < t; i++) {
        if (shortcut->function(session, &(shortcut->argument), NULL, session->buffer.n) == false) {
          break;
        }
      }

      if (session->global.buffer != NULL) {
        g_string_free(session->global.buffer, TRUE);
        session->global.buffer = NULL;
      }

      session->buffer.n = 0;

      if (session->events.buffer_changed != NULL) {
        session->events.buffer_changed(session);
      }

      girara_list_iterator_free(iter);
      return TRUE;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcut);

  /* update buffer */
  if (keyval >= 0x21 && keyval <= 0x7E) {
    /* overall buffer */
    if (session->global.buffer == NULL) {
      session->global.buffer = g_string_new("");
    }

    session->global.buffer = g_string_append_c(session->global.buffer, keyval);

    if (session->buffer.command == NULL && keyval >= 0x30 && keyval <= 0x39) {
      if (((session->buffer.n * 10) + (keyval - '0')) < INT_MAX) {
        session->buffer.n = (session->buffer.n * 10) + (keyval - '0');
      }
    } else {
      if (session->buffer.command == NULL) {
        session->buffer.command = g_string_new("");
      }

      session->buffer.command = g_string_append_c(session->buffer.command, keyval);
    }

    if (session->events.buffer_changed != NULL) {
      session->events.buffer_changed(session);
    }
  }

  /* check for buffer command */
  if (session->buffer.command != NULL) {
    bool matching_command = FALSE;

    GIRARA_LIST_FOREACH(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcut)
      if (shortcut->buffered_command != NULL) {
        /* buffer could match a command */
        if (!strncmp(session->buffer.command->str, shortcut->buffered_command, session->buffer.command->len)) {
          /* command matches buffer exactly */
          if (!strcmp(session->buffer.command->str, shortcut->buffered_command)
            && (session->modes.current_mode == shortcut->mode || shortcut->mode == 0)) {
            g_string_free(session->buffer.command, TRUE);
            g_string_free(session->global.buffer,  TRUE);
            session->buffer.command = NULL;
            session->global.buffer  = NULL;

            if (session->events.buffer_changed != NULL) {
              session->events.buffer_changed(session);
            }

            int t = (session->buffer.n > 0) ? session->buffer.n : 1;
            for (int i = 0; i < t; i++) {
              if (shortcut->function(session, &(shortcut->argument), NULL, session->buffer.n) == false) {
                break;
              }
            }

            session->buffer.n = 0;
            girara_list_iterator_free(iter);
            return TRUE;
          }

          matching_command = TRUE;
        }
      }
    GIRARA_LIST_FOREACH_END(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcut);

    /* free buffer if buffer will never match a command */
    if (matching_command == false) {
      g_string_free(session->buffer.command, TRUE);
      g_string_free(session->global.buffer,  TRUE);
      session->buffer.command = NULL;
      session->global.buffer  = NULL;
      session->buffer.n       = 0;

      if (session->events.buffer_changed != NULL) {
        session->events.buffer_changed(session);
      }
    }
  }

  return FALSE;
}

bool
girara_callback_view_button_press_event(GtkWidget* UNUSED(widget),
    GdkEventButton* button, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(button  != NULL, false);

  /* prepare girara event */
  girara_event_t event;

  switch (button->type) {
    case GDK_BUTTON_PRESS:
      event.type = GIRARA_EVENT_BUTTON_PRESS;
      break;
    case GDK_2BUTTON_PRESS:
      event.type = GIRARA_EVENT_2BUTTON_PRESS;
      break;
    case GDK_3BUTTON_PRESS:
      event.type = GIRARA_EVENT_3BUTTON_PRESS;
      break;
    default: /* do not handle unknown events */
      event.type = GIRARA_EVENT_OTHER;
      break;
  }

  event.x = button->x;
  event.y = button->y;

  const guint state = button->state & MOUSE_MASK;

  /* search registered mouse events */
  GIRARA_LIST_FOREACH(session->bindings.mouse_events, girara_mouse_event_t*, iter, mouse_event)
    if (mouse_event->function != NULL
        && button->button == mouse_event->button
        && state  == mouse_event->mask
        && mouse_event->event_type == event.type
        && (session->modes.current_mode == mouse_event->mode || mouse_event->mode == 0)
       ) {
        mouse_event->function(session, &(mouse_event->argument), &event, session->buffer.n);
        girara_list_iterator_free(iter);
      return true;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.mouse_events, girara_mouse_event_t*, iter, mouse_event);

  return false;
}

bool
girara_callback_view_button_release_event(GtkWidget* UNUSED(widget), GdkEventButton* button, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(button  != NULL, false);

  /* prepare girara event */
  girara_event_t event;
  event.type = GIRARA_EVENT_BUTTON_RELEASE;
  event.x    = button->x;
  event.y    = button->y;

  const guint state = button->state & MOUSE_MASK;

  /* search registered mouse events */
  GIRARA_LIST_FOREACH(session->bindings.mouse_events, girara_mouse_event_t*, iter, mouse_event)
    if (mouse_event->function != NULL
        && button->button == mouse_event->button
        && state  == mouse_event->mask
        && mouse_event->event_type == GIRARA_EVENT_BUTTON_RELEASE
        && (session->modes.current_mode == mouse_event->mode || mouse_event->mode == 0)
       ) {
        mouse_event->function(session, &(mouse_event->argument), &event, session->buffer.n);
        girara_list_iterator_free(iter);
      return true;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.mouse_events, girara_mouse_event_t*, iter, mouse_event);

  return false;
}

bool
girara_callback_view_button_motion_notify_event(GtkWidget* UNUSED(widget), GdkEventMotion* button, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(button  != NULL, false);

  /* prepare girara event */
  girara_event_t event;
  event.type = GIRARA_EVENT_MOTION_NOTIFY;
  event.x    = button->x;
  event.y    = button->y;

  const guint state = button->state & MOUSE_MASK;

  /* search registered mouse events */
  GIRARA_LIST_FOREACH(session->bindings.mouse_events, girara_mouse_event_t*, iter, mouse_event)
    if (mouse_event->function != NULL
        && state  == mouse_event->mask
        && mouse_event->event_type == event.type
        && (session->modes.current_mode == mouse_event->mode || mouse_event->mode == 0)
       ) {
        mouse_event->function(session, &(mouse_event->argument), &event, session->buffer.n);
        girara_list_iterator_free(iter);
      return true;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.mouse_events, girara_mouse_event_t*, iter, mouse_event);

  return false;
}

bool
girara_callback_view_scroll_event(GtkWidget* UNUSED(widget), GdkEventScroll* scroll, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(scroll  != NULL, false);

  /* prepare girara event */
  girara_event_t event;
  event.x    = scroll->x;
  event.y    = scroll->y;

  switch (scroll->direction) {
    case GDK_SCROLL_UP:
      event.type = GIRARA_EVENT_SCROLL_UP;
      break;
    case GDK_SCROLL_DOWN:
      event.type = GIRARA_EVENT_SCROLL_DOWN;
      break;
    case GDK_SCROLL_LEFT:
      event.type = GIRARA_EVENT_SCROLL_LEFT;
      break;
    case GDK_SCROLL_RIGHT:
      event.type = GIRARA_EVENT_SCROLL_RIGHT;
      break;
    default:
      return false;
  }

  const guint state = scroll->state & MOUSE_MASK;

  /* search registered mouse events */
  /* TODO: Filter correct event */
  GIRARA_LIST_FOREACH(session->bindings.mouse_events, girara_mouse_event_t*, iter, mouse_event)
    if (mouse_event->function != NULL
        && state  == mouse_event->mask
        && mouse_event->event_type == event.type
        && (session->modes.current_mode == mouse_event->mode || mouse_event->mode == 0)
       ) {
        mouse_event->function(session, &(mouse_event->argument), &event, session->buffer.n);
        girara_list_iterator_free(iter);
        return true;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.mouse_events, girara_mouse_event_t*, iter, mouse_event);

  return false;
}

bool
girara_callback_inputbar_activate(GtkEntry* entry, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);

  /* a custom handler has been installed (e.g. by girara_dialog) */
  if (session->signals.inputbar_custom_activate != NULL) {
    bool return_value = session->signals.inputbar_custom_activate(entry, session->signals.inputbar_custom_data);

    /* disconnect custom handler */
    session->signals.inputbar_custom_activate        = NULL;
    session->signals.inputbar_custom_key_press_event = NULL;
    session->signals.inputbar_custom_data            = NULL;

    if (session->gtk.inputbar_dialog != NULL && session->gtk.inputbar_entry != NULL) {
      gtk_label_set_markup(session->gtk.inputbar_dialog, "");
      gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar_dialog));
      if (session->global.autohide_inputbar == true) {
        gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar));
      }
      gtk_entry_set_visibility(session->gtk.inputbar_entry, TRUE);
      girara_isc_abort(session, NULL, NULL, 0);
      return true;
    }

    return return_value;
  }

  gchar *input = gtk_editable_get_chars(GTK_EDITABLE(entry), 1, -1);
  if (input == NULL) {
    girara_isc_abort(session, NULL, NULL, 0);
    return false;
  }

  if (strlen(input) == 0) {
    g_free(input);
    girara_isc_abort(session, NULL, NULL, 0);
    return false;
  }

  /* append to command history */
  if (session->global.command_history != NULL) {
    const char* command = gtk_entry_get_text(entry);
    girara_list_append(session->global.command_history, g_strdup(command));
  }

  /* parse input */
  gchar** argv = NULL;
  gint    argc = 0;

  if (g_shell_parse_argv(input, &argc, &argv, NULL) == FALSE) {
    g_free(input);
    return false;
  }

  gchar *cmd = argv[0];

  /* special commands */
  char *identifier_s = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, 1);
  if (identifier_s == NULL) {
    g_free(input);
    g_strfreev(argv);
    return false;
  }

  char identifier = identifier_s[0];
  g_free(identifier_s);

  GIRARA_LIST_FOREACH(session->bindings.special_commands, girara_special_command_t*, iter, special_command)
    if (special_command->identifier == identifier) {
      if (special_command->always != true) {
        special_command->function(session, input, &(special_command->argument));
      }

      g_free(input);
      g_strfreev(argv);

      girara_isc_abort(session, NULL, NULL, 0);

      girara_list_iterator_free(iter);
      return true;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.special_commands, girara_special_command_t*, iter, special_command);

  /* search commands */
  GIRARA_LIST_FOREACH(session->bindings.commands, girara_command_t*, iter, command)
    if ((g_strcmp0(cmd, command->command) == 0) ||
        (g_strcmp0(cmd, command->abbr)    == 0))
    {
      girara_list_t* argument_list = girara_list_new();
      if (argument_list == NULL) {
        g_free(input);
        g_strfreev(argv);
        girara_list_iterator_free(iter);
        return false;
      }

      girara_list_set_free_function(argument_list, g_free);

      for(int i = 1; i < argc; i++) {
        char* argument = g_strdup(argv[i]);
        girara_list_append(argument_list, (void*) argument);
      }

      command->function(session, argument_list);

      girara_list_free(argument_list);
      g_free(input);
      g_strfreev(argv);

      girara_isc_abort(session, NULL, NULL, 0);

      if (session->global.autohide_inputbar == true) {
        gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar));
      }
      gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar_dialog));
      girara_list_iterator_free(iter);
      return true;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.commands, girara_command_t*, iter, command);

  /* check for unknown command event handler */
  if (session->events.unknown_command != NULL) {
    if (session->events.unknown_command(session, input) == true) {
      g_strfreev(argv);
      g_free(input);
      girara_isc_abort(session, NULL, NULL, 0);

      if (session->global.autohide_inputbar == true) {
        gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar));
      }
      gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar_dialog));

      return true;
    }
  }

  /* unhandled command */
  girara_notify(session, GIRARA_ERROR, _("Not a valid command: %s"), cmd);
  g_strfreev(argv);
  girara_isc_abort(session, NULL, NULL, 0);

  return false;
}

bool
girara_callback_inputbar_key_press_event(GtkWidget* entry, GdkEventKey* event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, false);

  /* a custom handler has been installed (e.g. by girara_dialog) */
  bool custom_ret = false;
  if (session->signals.inputbar_custom_key_press_event != NULL) {
    custom_ret = session->signals.inputbar_custom_key_press_event(entry, event, session->signals.inputbar_custom_data);
    if (custom_ret == true) {
      girara_isc_abort(session, NULL, NULL, 0);

      if (session->global.autohide_inputbar == true) {
        gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar));
      }
      gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar_dialog));
    }
  }

  guint keyval = 0;
  guint clean  = 0;

  if (clean_mask(event->hardware_keycode, event->state, event->group, &clean, &keyval) == false) {
    return false;
  }

  if (custom_ret == false) {
    GIRARA_LIST_FOREACH(session->bindings.inputbar_shortcuts, girara_inputbar_shortcut_t*, iter, inputbar_shortcut)
      if (inputbar_shortcut->key == keyval
       && inputbar_shortcut->mask == clean)
      {
        if (inputbar_shortcut->function != NULL) {
          inputbar_shortcut->function(session, &(inputbar_shortcut->argument), NULL, 0);
        }

        girara_list_iterator_free(iter);
        return true;
      }
    GIRARA_LIST_FOREACH_END(session->bindings.inputbar_shortcuts, girara_inputbar_shortcut_t*, iter, inputbar_shortcut);
  }

  if ((session->gtk.results != NULL) &&
     (gtk_widget_get_visible(GTK_WIDGET(session->gtk.results)) == TRUE) &&
     (keyval == GDK_KEY_space))
  {
    gtk_widget_hide(GTK_WIDGET(session->gtk.results));
  }

  return custom_ret;
}

bool
girara_callback_inputbar_changed_event(GtkEditable* entry, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, false);

  /* special commands */
  char *identifier_s = gtk_editable_get_chars(entry, 0, 1);
  if (identifier_s == NULL) {
    return false;
  }

  char identifier    = identifier_s[0];
  g_free(identifier_s);

  GIRARA_LIST_FOREACH(session->bindings.special_commands, girara_special_command_t*, iter, special_command)
    if ((special_command->identifier == identifier) &&
       (special_command->always == true))
    {
      gchar *input  = gtk_editable_get_chars(GTK_EDITABLE(entry), 1, -1);
      special_command->function(session, input, &(special_command->argument));
      g_free(input);
      girara_list_iterator_free(iter);
      return false;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.special_commands, girara_special_command_t*, iter, special_command);

  return false;
}
