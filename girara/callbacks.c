/* SPDX-License-Identifier: Zlib */

#include "callbacks.h"

#include "datastructures.h"
#include "input-history.h"
#include "internal.h"
#include "session.h"
#include "shortcuts.h"
#include "utils.h"

#include <glib/gi18n-lib.h>
#include <string.h>

static const guint ALL_ACCELS_MASK = GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK;
static const guint MOUSE_MASK = GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK |
  GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK | GDK_BUTTON4_MASK | GDK_BUTTON5_MASK;

static bool
clean_mask(GtkWidget* widget, guint hardware_keycode, GdkModifierType state, gint group, guint* clean, guint* keyval)
{
  GdkDisplay* display      = gtk_widget_get_display(widget);
  GdkModifierType consumed = 0;

  if ((gdk_keymap_translate_keyboard_state(
        gdk_keymap_get_for_display(display),
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
gboolean
girara_callback_view_key_press_event(GtkWidget* widget,
    GdkEventKey* event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);

  guint clean  = 0;
  guint keyval = 0;

  if (clean_mask(widget, event->hardware_keycode, event->state, event->group, &clean, &keyval) == false) {
    return false;
  }

  girara_session_private_t* session_private = session->private_data;

  /* prepare event */
  GIRARA_LIST_FOREACH_BODY_WITH_ITER(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcut,
    if (session_private->buffer.command != NULL) {
      break;
    }

    if ( keyval == shortcut->key
      && (clean == shortcut->mask || (shortcut->key >= 0x21
      && shortcut->key <= 0x7E && clean == GDK_SHIFT_MASK))
      && (session->modes.current_mode == shortcut->mode || shortcut->mode == 0)
      && shortcut->function != NULL
      )
    {
      const int t = (session_private->buffer.n > 0) ? session_private->buffer.n : 1;
      for (int i = 0; i < t; i++) {
        if (shortcut->function(session, &(shortcut->argument), NULL, session_private->buffer.n) == false) {
          break;
        }
      }

      if (session->global.buffer != NULL) {
        g_string_free(session->global.buffer, TRUE);
        session->global.buffer = NULL;
      }

      session_private->buffer.n = 0;

      if (session->events.buffer_changed != NULL) {
        session->events.buffer_changed(session);
      }

      girara_list_iterator_free(iter);
      return TRUE;
    }
  );

  /* update buffer */
  if (keyval >= 0x21 && keyval <= 0x7E) {
    /* overall buffer */
    if (session->global.buffer == NULL) {
      session->global.buffer = g_string_new("");
    }

    session->global.buffer = g_string_append_c(session->global.buffer, keyval);

    if (session_private->buffer.command == NULL && keyval >= 0x30 && keyval <= 0x39) {
      if (((session_private->buffer.n * 10) + (keyval - '0')) < INT_MAX) {
        session_private->buffer.n = (session_private->buffer.n * 10) + (keyval - '0');
      }
    } else {
      if (session_private->buffer.command == NULL) {
        session_private->buffer.command = g_string_new("");
      }

      session_private->buffer.command = g_string_append_c(session_private->buffer.command, keyval);
    }

    if (session->events.buffer_changed != NULL) {
      session->events.buffer_changed(session);
    }
  }

  /* check for buffer command */
  if (session_private->buffer.command != NULL) {
    bool matching_command = FALSE;

    GIRARA_LIST_FOREACH_BODY_WITH_ITER(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcut,
      if (shortcut->buffered_command != NULL) {
        /* buffer could match a command */
        if (!strncmp(session_private->buffer.command->str, shortcut->buffered_command, session_private->buffer.command->len)) {
          /* command matches buffer exactly */
          if (!g_strcmp0(session_private->buffer.command->str, shortcut->buffered_command)
            && (session->modes.current_mode == shortcut->mode || shortcut->mode == 0)) {
            g_string_free(session_private->buffer.command, TRUE);
            g_string_free(session->global.buffer,  TRUE);
            session_private->buffer.command = NULL;
            session->global.buffer  = NULL;

            if (session->events.buffer_changed != NULL) {
              session->events.buffer_changed(session);
            }

            int t = (session_private->buffer.n > 0) ? session_private->buffer.n : 1;
            for (int i = 0; i < t; i++) {
              if (shortcut->function(session, &(shortcut->argument), NULL, session_private->buffer.n) == false) {
                break;
              }
            }

            session_private->buffer.n = 0;
            girara_list_iterator_free(iter);
            return TRUE;
          }

          matching_command = TRUE;
        }
      }
    );

    /* free buffer if buffer will never match a command */
    if (matching_command == false) {
      g_string_free(session_private->buffer.command, TRUE);
      g_string_free(session->global.buffer,  TRUE);
      session_private->buffer.command = NULL;
      session->global.buffer  = NULL;
      session_private->buffer.n       = 0;

      if (session->events.buffer_changed != NULL) {
        session->events.buffer_changed(session);
      }
    }
  }

  return FALSE;
}

gboolean
girara_callback_view_button_press_event(GtkWidget* UNUSED(widget),
    GdkEventButton* button, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(button  != NULL, false);

  /* prepare girara event */
  girara_event_t event = {
    .x = button->x,
    .y = button->y
  };

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

  const guint state = button->state & MOUSE_MASK;
  girara_session_private_t* session_private = session->private_data;

  bool handled = false;

  /* search registered mouse events */
  GIRARA_LIST_FOREACH_BODY(session->bindings.mouse_events, girara_mouse_event_t*, mouse_event,
    if (mouse_event->function != NULL
        && button->button == mouse_event->button
        && state  == mouse_event->mask
        && mouse_event->event_type == event.type
        && (session->modes.current_mode == mouse_event->mode || mouse_event->mode == 0)
       ) {
      mouse_event->function(session, &(mouse_event->argument), &event, session_private->buffer.n);
      handled = true;
      break;
    }
  );

  return handled;
}

gboolean
girara_callback_view_button_release_event(GtkWidget* UNUSED(widget), GdkEventButton* button, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(button  != NULL, false);

  /* prepare girara event */
  girara_event_t event = {
    .type = GIRARA_EVENT_BUTTON_RELEASE,
    .x    = button->x,
    .y    = button->y
  };

  const guint state = button->state & MOUSE_MASK;
  girara_session_private_t* session_private = session->private_data;
  bool handled = false;

  /* search registered mouse events */
  GIRARA_LIST_FOREACH_BODY(session->bindings.mouse_events, girara_mouse_event_t*, mouse_event,
    if (mouse_event->function != NULL
        && button->button == mouse_event->button
        && state  == mouse_event->mask
        && mouse_event->event_type == GIRARA_EVENT_BUTTON_RELEASE
        && (session->modes.current_mode == mouse_event->mode || mouse_event->mode == 0)
       ) {
      mouse_event->function(session, &(mouse_event->argument), &event, session_private->buffer.n);
      handled = true;
      break;
    }
  );

  return handled;
}

gboolean
girara_callback_view_button_motion_notify_event(GtkWidget* UNUSED(widget), GdkEventMotion* button, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(button  != NULL, false);

  /* prepare girara event */
  girara_event_t event = {
    .type = GIRARA_EVENT_MOTION_NOTIFY,
    .x    = button->x,
    .y    = button->y
  };

  const guint state = button->state & MOUSE_MASK;
  girara_session_private_t* session_private = session->private_data;
  bool handled = false;

  /* search registered mouse events */
  GIRARA_LIST_FOREACH_BODY(session->bindings.mouse_events, girara_mouse_event_t*, mouse_event,
    if (mouse_event->function != NULL
        && state  == mouse_event->mask
        && mouse_event->event_type == event.type
        && (session->modes.current_mode == mouse_event->mode || mouse_event->mode == 0)
       ) {
      mouse_event->function(session, &(mouse_event->argument), &event, session_private->buffer.n);
      handled = true;
      break;
    }
  );

  return handled;
}

gboolean
girara_callback_view_scroll_event(GtkWidget* UNUSED(widget), GdkEventScroll* scroll, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(scroll  != NULL, false);

  /* prepare girara event */
  girara_event_t event = {
    .x    = scroll->x,
    .y    = scroll->y
  };

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
    case GDK_SCROLL_SMOOTH:
      event.type = GIRARA_EVENT_SCROLL_BIDIRECTIONAL;
      /* We abuse x and y here. We really need more fields in girara_event_t. */
      gdk_event_get_scroll_deltas((GdkEvent*)scroll, &event.x, &event.y);
      break;
    default:
      return false;
  }

  const guint state = scroll->state & MOUSE_MASK;
  girara_session_private_t* session_private = session->private_data;
  bool handled = false;

  /* search registered mouse events */
  /* TODO: Filter correct event */
  GIRARA_LIST_FOREACH_BODY(session->bindings.mouse_events, girara_mouse_event_t*, mouse_event,
    if (mouse_event->function != NULL
        && state  == mouse_event->mask
        && mouse_event->event_type == event.type
        && (session->modes.current_mode == mouse_event->mode || mouse_event->mode == 0)
       ) {
      mouse_event->function(session, &(mouse_event->argument), &event, session_private->buffer.n);
      handled = true;
      break;
    }
  );

  return handled;
}

gboolean
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
  const char* command = gtk_entry_get_text(entry);
  girara_input_history_append(session->command_history, command);

  /* special commands */
  char *identifier_s = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, 1);
  if (identifier_s == NULL) {
    return false;
  }

  const char identifier = identifier_s[0];
  g_free(identifier_s);

  girara_debug("Processing special command with identifier '%c'.", identifier);
  GIRARA_LIST_FOREACH_BODY_WITH_ITER(session->bindings.special_commands, girara_special_command_t*, iter, special_command,
    if (special_command->identifier == identifier) {
      girara_debug("Found special command.");
      if (special_command->always != true) {
        special_command->function(session, input, &(special_command->argument));
      }

      girara_isc_abort(session, NULL, NULL, 0);

      girara_list_iterator_free(iter);
      return true;
    }
  );

  /* parse input */
  gchar** argv = NULL;
  gint    argc = 0;

  if (g_shell_parse_argv(input, &argc, &argv, NULL) == FALSE) {
    girara_debug("Failed to parse argument.");
    g_free(input);
    return false;
  }

  gchar *cmd = argv[0];

  /* search commands */
  GIRARA_LIST_FOREACH_BODY_WITH_ITER(session->bindings.commands, girara_command_t*, iter, binding_command,
    if ((g_strcmp0(cmd, binding_command->command) == 0) ||
        (g_strcmp0(cmd, binding_command->abbr)    == 0)) {
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

      binding_command->function(session, argument_list);

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
  );

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

gboolean
girara_callback_inputbar_key_press_event(GtkWidget* entry, GdkEventKey* event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, false);

  /* a custom handler has been installed (e.g. by girara_dialog) */
  gboolean custom_ret = false;
  if (session->signals.inputbar_custom_key_press_event != NULL) {
    girara_debug("Running custom key press event handler.");
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
  if (clean_mask(entry, event->hardware_keycode, event->state, event->group, &clean, &keyval) == false) {
    girara_debug("clean_mask returned false.");
    return false;
  }
  girara_debug("Proccessing key %u with mask %x.", keyval, clean);

  if (custom_ret == false) {
    GIRARA_LIST_FOREACH_BODY_WITH_ITER(session->bindings.inputbar_shortcuts, girara_inputbar_shortcut_t*, iter, inputbar_shortcut,
      if (inputbar_shortcut->key == keyval
       && inputbar_shortcut->mask == clean) {
        girara_debug("found shortcut for key %u and mask %x", keyval, clean);
        if (inputbar_shortcut->function != NULL) {
          inputbar_shortcut->function(session, &(inputbar_shortcut->argument), NULL, 0);
        }

        girara_list_iterator_free(iter);
        return true;
      }
    );
  }

  if ((session->gtk.results != NULL) &&
     (gtk_widget_get_visible(GTK_WIDGET(session->gtk.results)) == TRUE) &&
     (keyval == GDK_KEY_space))
  {
    gtk_widget_hide(GTK_WIDGET(session->gtk.results));
  }

  return custom_ret;
}

gboolean
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
  bool handled = false;

  GIRARA_LIST_FOREACH_BODY(session->bindings.special_commands, girara_special_command_t*, special_command,
    if ((special_command->identifier == identifier) &&
       (special_command->always == true)) {
      gchar *input  = gtk_editable_get_chars(GTK_EDITABLE(entry), 1, -1);
      special_command->function(session, input, &(special_command->argument));
      g_free(input);
      handled = true;
      break;
    }
  );

  return handled;
}
