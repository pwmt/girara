/* SPDX-License-Identifier: Zlib */

#include "record.h"

#include <gdk/gdk.h>
#include <glib/gstdio.h>

#include "datastructures.h"
#include "internal.h"
#include "log.h"
#include "session.h"
#include "settings.h"

const char keypress_ident[] = "keypress";
const char assert_ident[] = "assert";
const char breakpoint_ident[] = "breakpoint";
const unsigned int record_version = 1;

typedef struct gdk_keyboard_button_s {
  char* identifier;
  int keyval;
} gdk_keyboard_button_t;

static const gdk_keyboard_button_t gdk_keyboard_buttons[] = {
    {"BackSpace", GDK_KEY_BackSpace},
    {"CapsLock", GDK_KEY_Caps_Lock},
    {"Down", GDK_KEY_Down},
    {"Esc", GDK_KEY_Escape},
    {"F10", GDK_KEY_F10},
    {"F11", GDK_KEY_F11},
    {"F12", GDK_KEY_F12},
    {"F1", GDK_KEY_F1},
    {"F2", GDK_KEY_F2},
    {"F3", GDK_KEY_F3},
    {"F4", GDK_KEY_F4},
    {"F5", GDK_KEY_F5},
    {"F6", GDK_KEY_F6},
    {"F7", GDK_KEY_F7},
    {"F8", GDK_KEY_F8},
    {"F9", GDK_KEY_F9},
    {"Left", GDK_KEY_Left},
    {"PageDown", GDK_KEY_Page_Down},
    {"PageUp", GDK_KEY_Page_Up},
    {"Home", GDK_KEY_Home},
    {"End", GDK_KEY_End},
    {"Return", GDK_KEY_Return},
    {"Right", GDK_KEY_Right},
    {"Space", GDK_KEY_space},
    {"Super", GDK_KEY_Super_L},
    {"Tab", GDK_KEY_Tab},
    {"ShiftTab", GDK_KEY_ISO_Left_Tab},
    {"Up", GDK_KEY_Up},
    {"Print", GDK_KEY_Print},
    {"KPLeft", GDK_KEY_KP_Left},
    {"KPRight", GDK_KEY_KP_Right},
    {"KPUp", GDK_KEY_KP_Up},
    {"KPDown", GDK_KEY_KP_Down},
};

void girara_record_init(girara_session_t* session) {
  g_return_if_fail(session != NULL);

  girara_session_private_t* session_private = session->private_data;
  g_return_if_fail(session_private->record.output != NULL);

  GOutputStream* stream = G_OUTPUT_STREAM(session_private->record.output);
  g_output_stream_printf(stream, NULL, NULL, NULL, "[%d] # Girara Recording File\n\n", record_version);
}

static int update_state_by_keyval(int state, int keyval) {
  /* The following is probably not true for some keyboard layouts. */
  if ((keyval >= '!' && keyval <= '/') || (keyval >= ':' && keyval <= '@') || (keyval >= '[' && keyval <= '`') ||
      (keyval >= '{' && keyval <= '~')) {
    state |= GDK_SHIFT_MASK;
  }

  return state;
}

void girara_record_filter_keys(girara_session_t* session, char* input) {
  g_return_if_fail(session != NULL);
  girara_session_private_t* session_private = session->private_data;

  unsigned int input_length = strlen(input);

  for (unsigned i = 0; i < input_length; i++) {
    int state  = 0;
    int keyval = input[i];

    /* possible special button */
    if ((input_length - i) >= 3 && input[i] == '<') {
      char* end = strchr(input + i, '>');
      if (end == NULL) {
        goto single_key;
      }

      const int length     = end - (input + i) - 1;
      g_autofree char* tmp = g_strndup(input + i + 1, length);
      bool found           = false;

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
              found  = true;
              break;
            }
          }
        }
        /* Possible special key */
      } else {
        for (unsigned int j = 0; j < LENGTH(gdk_keyboard_buttons); ++j) {
          if (g_strcmp0(tmp, gdk_keyboard_buttons[j].identifier) == 0) {
            keyval = gdk_keyboard_buttons[j].keyval;
            found  = true;
            break;
          }
        }
      }

      /* parsed special key */
      if (found == true) {
        i += length + 1;
      }
    }

  single_key:
    state = update_state_by_keyval(state, keyval);
    GdkEvent* event = gdk_event_new(GDK_KEY_PRESS);

    event->any.type       = GDK_KEY_PRESS;
    event->key.state      = state;
    event->key.keyval     = keyval;

    girara_list_append(session_private->record.filter_keys, event);
  }
}

static char *key_event_to_string(GdkEventKey* event) {
  for (unsigned int j = 0; j < LENGTH(gdk_keyboard_buttons); ++j) {
    if (gdk_keyboard_buttons[j].keyval != (int)event->keyval) {
      continue;
    }

    return g_strdup(gdk_keyboard_buttons[j].identifier);
  }

  char key = gdk_keyval_to_unicode(event->keyval);
  char *str = g_strdup(""); 

  switch (event->state) {
  case 0:
    str = g_strdup_printf("%c", key);
    break;

  case GDK_SHIFT_MASK:
    str = g_strdup_printf("<S-%c>", key);
    break;

  case GDK_MOD1_MASK:
    str = g_strdup_printf("<A-%c>", key);
    break;

  case GDK_CONTROL_MASK:
    str = g_strdup_printf("<C-%c>", key);
    break;

  default:
    girara_warning("state %d not implemented", event->state);
    break;
  }

  return str;
}

void girara_record_key_event(girara_session_t* session, const char* widget_name, GdkEventKey* event) {
  g_return_if_fail(session != NULL);

  bool macro_record = false;
  girara_setting_get(session, "macro-record", &macro_record);

  if (!macro_record || event->type != GDK_KEY_PRESS) {
    return;
  }

  girara_session_private_t* session_private = session->private_data;
  g_return_if_fail(session_private->record.output != NULL);

  /* check if key is a filter key, and only filter view events */
  if (strncmp(widget_name, "view", 4) == 0) {
    for (size_t i = 0; i != girara_list_size(session_private->record.filter_keys); i++) {
      GdkEvent* filter_event = girara_list_nth(session_private->record.filter_keys, i);

      if (event->state == filter_event->key.state && event->keyval == filter_event->key.keyval) {
        return;
      }
    }
  }

  GOutputStream* stream = G_OUTPUT_STREAM(session_private->record.output);
  g_autofree char* str = key_event_to_string(event);
  g_output_stream_printf(stream, NULL, NULL, NULL, "%s %d %d # %s\n", 
                         keypress_ident, event->state, event->keyval, str);
}

void girara_record_assert(girara_session_t* session, char* name, char* value) {
  g_return_if_fail(session != NULL);

  bool macro_record = false;
  girara_setting_get(session, "macro-record", &macro_record);

  if (!macro_record) {
    return;
  }

  girara_session_private_t* session_private = session->private_data;
  g_return_if_fail(session_private->record.output != NULL);

  GOutputStream* stream = G_OUTPUT_STREAM(session_private->record.output);
  g_output_stream_printf(stream, NULL, NULL, NULL, "%s '%s' '%s'\n", assert_ident, name, value);
  girara_info("macro assert");
}

void girara_record_breakpoint(girara_session_t* session) {
  g_return_if_fail(session != NULL);

  girara_session_private_t* session_private = session->private_data;
  g_return_if_fail(session_private->record.output != NULL);

  GOutputStream* stream = G_OUTPUT_STREAM(session_private->record.output);
  g_output_stream_printf(stream, NULL, NULL, NULL, "%s\n", breakpoint_ident);
  girara_info("macro breakpoint");
}

void girara_record_item_free(girara_record_item_t* item) {
  if (item == NULL || item->type == RECORD_KEY) {
    return;
  }

  g_free(item->assert.name);
  g_free(item->assert.value);
  g_free(item);
}

static bool simulate_key_press(girara_session_t* session, int state, int key) {
  if (session == NULL || session->gtk.box == NULL) {
    return false;
  }

  g_autoptr(GdkEvent) event = gdk_event_new(GDK_KEY_PRESS);

  event->any.type       = GDK_KEY_PRESS;
  event->key.window     = g_object_ref(gtk_widget_get_parent_window(GTK_WIDGET(session->gtk.box)));
  event->key.send_event = false;
  event->key.time       = GDK_CURRENT_TIME;
  event->key.state      = state;
  event->key.keyval     = key;

  GdkDisplay* display           = gtk_widget_get_display(GTK_WIDGET(session->gtk.box));
  g_autofree GdkKeymapKey* keys = NULL;
  gint number_of_keys           = 0;

  if (gdk_keymap_get_entries_for_keyval(gdk_keymap_get_for_display(display), event->key.keyval, &keys,
                                        &number_of_keys) == FALSE) {
    return false;
  }

  event->key.hardware_keycode = keys[0].keycode;
  event->key.group            = keys[0].group;

  GdkSeat* seat       = gdk_display_get_default_seat(display);
  GdkDevice* keyboard = gdk_seat_get_keyboard(seat);
  gdk_event_set_device(event, keyboard);

  gdk_event_put(event);

  /* wait for all events to finish */
  do {
    g_main_context_iteration(NULL, FALSE);
  } while (g_main_context_pending(NULL));

  return true;
}

#define MATCH_TOKEN(cond)   \
    G_STMT_START { \
      g_scanner_get_next_token(scanner); \
      if (!(cond)) { \
        girara_notify(session, GIRARA_ERROR, \
          "recording file parse error: line %d, position %d", \
          scanner->line, scanner->position); \
        girara_debug("recording file parse error: line %d, position %d", \
          scanner->line, scanner->position); \
        goto scan_error; \
      } \
    } G_STMT_END

gboolean girara_record_load_macro(girara_session_t* session, char* path) {
  g_return_val_if_fail(session != NULL, true);
  girara_session_private_t* session_private = session->private_data;

  /* clear previous recording */
  session_private->record.index = 0;
  girara_list_clear(session_private->record.recording);

  /* setup scanner */
  FILE* file = g_fopen(path, "r");
  if (file == NULL) {
    girara_notify(session, GIRARA_ERROR, "Failed to open recording file '%s'", path);
    return false;
  }

  int fd = fileno(file);

  GScanner* scanner = g_scanner_new(NULL);
  g_scanner_input_file(scanner, fd);

  /* Header */
  MATCH_TOKEN(scanner->token == G_TOKEN_LEFT_BRACE);
  MATCH_TOKEN(scanner->token == G_TOKEN_INT);

  if (scanner->value.v_int != record_version) {
    girara_notify(session, GIRARA_ERROR, "load record file: incorrect version, expected %d got %ld", 
                  record_version, scanner->value.v_int);
    girara_debug("incorrect version number, expected %d got %ld", record_version, scanner->value.v_int);
    goto scan_error;
  }

  MATCH_TOKEN(scanner->token == G_TOKEN_RIGHT_BRACE);

  girara_debug("parsing record file '%s' version %d", path, record_version);

  while (true) {
    g_scanner_peek_next_token(scanner);
    if (scanner->next_token == G_TOKEN_EOF) {
      break;
    }

    MATCH_TOKEN(scanner->token == G_TOKEN_IDENTIFIER);
    GTokenValue value = scanner->value;
    girara_record_item_t* item = g_malloc0(sizeof( girara_record_item_t ));

    if (!g_strcmp0(value.v_identifier, keypress_ident)) {
      /* keypress */
      item->type = RECORD_KEY;

      MATCH_TOKEN(scanner->token == G_TOKEN_INT);
      item->key.state = scanner->value.v_int;

      MATCH_TOKEN(scanner->token == G_TOKEN_INT);
      item->key.keyval = scanner->value.v_int;

      girara_debug("keypress: state %d, keypress %d", item->key.state, item->key.keyval);
    } else if (!g_strcmp0(value.v_identifier, assert_ident)) {
      /* assert */
      item->type = RECORD_ASSERT;

      MATCH_TOKEN(scanner->token == G_TOKEN_STRING);
      item->assert.name = g_strdup(scanner->value.v_string);

      MATCH_TOKEN(scanner->token == G_TOKEN_STRING);
      item->assert.value = g_strdup(scanner->value.v_string);

      girara_debug("assert: name %s, value %s", item->assert.name, item->assert.value);
    } else if (!g_strcmp0(value.v_identifier, breakpoint_ident)) {
      /* breakpoint */
      item->type = RECORD_BREAKPOINT;

      girara_debug("breakpoint");
    } else {
      girara_notify(session, GIRARA_ERROR, "recording file parse error: line %d, position %d", 
                    scanner->line, scanner->position);

      girara_debug("recording file parse error: line %d, position %d", scanner->line, scanner->position);
      girara_debug("expected %s, or %s, got %s", keypress_ident, assert_ident, value.v_string);
      goto scan_error;
    }

    girara_list_append(session_private->record.recording, item);
  }

  g_scanner_destroy(scanner);
  close(fd);
  return true;

scan_error:
  g_scanner_destroy(scanner);
  close(fd);

  return false;
}

void girara_record_run_macro(girara_session_t* session) {
  g_return_if_fail(session != NULL);
  girara_session_private_t* session_private = session->private_data;

  g_return_if_fail(session->record.assert_cb != NULL);

  if (g_mutex_trylock(&session_private->record.mutex) == FALSE) {
    girara_error("Recursive use of replay detected. Aborting evaluation.");
    return;
  }

  if (session_private->record.index == 0) {
    girara_info("macro started");
  }

  gboolean retval;
  unsigned int n = girara_list_size(session_private->record.recording);
  for (; session_private->record.index != n; session_private->record.index++) {
    girara_record_item_t* item = girara_list_nth(session_private->record.recording, 
                                                 session_private->record.index);

    switch (item->type) {
    case RECORD_KEY:
      simulate_key_press(session, item->key.state, item->key.keyval);
      break;

    case RECORD_ASSERT:
      gtk_main_iteration_do(FALSE);
      retval = session->record.assert_cb(session, item->assert.name, item->assert.value);
      girara_info("[%s] assert %s: %s", retval ? "Pass" : "Fail", item->assert.name, item->assert.value);

      if (!retval) {
        goto macro_end;
      }

      break;

    case RECORD_BREAKPOINT:
      girara_info("macro breakpoint");
      session_private->record.index++;
      goto macro_end;

    default:
      girara_debug("unknown key type %d", item->type);
      break;
    }
  }

  girara_info("macro finished");

macro_end:
  g_mutex_unlock(&session_private->record.mutex);
}
