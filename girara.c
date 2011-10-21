/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#include "girara.h"
#include "girara-internal.h"

bool
girara_shortcut_add(girara_session_t* session, guint modifier, guint key, char* buffer, girara_shortcut_function_t function, girara_mode_t mode, int argument_n, void* argument_data)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(buffer || key || modifier, FALSE);

  girara_argument_t argument = {argument_n, argument_data};

  /* search for existing binding */
  GIRARA_LIST_FOREACH(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcuts_it)
    if (((shortcuts_it->mask == modifier && shortcuts_it->key == key && (modifier != 0 || key != 0)) ||
       (buffer && shortcuts_it->buffered_command && !strcmp(shortcuts_it->buffered_command, buffer)))
        && shortcuts_it->mode == mode)
    {
      shortcuts_it->function = function;
      shortcuts_it->argument = argument;
      girara_list_iterator_free(iter);
      return TRUE;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcuts_it);

  /* add new shortcut */
  girara_shortcut_t* shortcut = g_slice_new(girara_shortcut_t);

  shortcut->mask             = modifier;
  shortcut->key              = key;
  shortcut->buffered_command = buffer;
  shortcut->function         = function;
  shortcut->mode             = mode;
  shortcut->argument         = argument;
  girara_list_append(session->bindings.shortcuts, shortcut);

  return TRUE;
}

void
girara_shortcut_free(girara_shortcut_t* shortcut)
{
  g_slice_free(girara_shortcut_t, shortcut);
}

bool
girara_inputbar_command_add(girara_session_t* session, char* command , char* abbreviation, girara_command_function_t function, girara_completion_function_t completion, char* description)
{
  g_return_val_if_fail(session  != NULL, FALSE);
  g_return_val_if_fail(command  != NULL, FALSE);
  g_return_val_if_fail(function != NULL, FALSE);

  /* search for existing binding */
  girara_command_t* commands_it = session->bindings.commands;
  girara_command_t* last_command = commands_it;

  while (commands_it) {
    if (!g_strcmp0(commands_it->command, command)) {
      if (commands_it->abbr) {
        free(commands_it->abbr);
      }

      if (commands_it->description) {
        free(commands_it->description);
      }

      commands_it->abbr        = abbreviation ? g_strdup(abbreviation) : NULL;
      commands_it->function    = function;
      commands_it->completion  = completion;
      commands_it->description = description ? g_strdup(description) : NULL;

      return TRUE;
    }

    last_command = commands_it;
    commands_it  = commands_it->next;
  }

  /* add new inputbar command */
  girara_command_t* new_command = g_slice_new(girara_command_t);

  new_command->command     = g_strdup(command);
  new_command->abbr        = abbreviation ? g_strdup(abbreviation) : NULL;
  new_command->function    = function;
  new_command->completion  = completion;
  new_command->description = description ? g_strdup(description) : NULL;
  new_command->next        = NULL;

  if (last_command) {
    last_command->next = new_command;
  } else {
    session->bindings.commands = new_command;
  }

  return TRUE;
}

bool
girara_inputbar_shortcut_add(girara_session_t* session, guint modifier, guint key, girara_shortcut_function_t function, int argument_n, void* argument_data)
{
  g_return_val_if_fail(session  != NULL, FALSE);
  g_return_val_if_fail(function != NULL, FALSE);

  girara_argument_t argument = {argument_n, argument_data};

  /* search for existing special command */
  GIRARA_LIST_FOREACH(session->bindings.inputbar_shortcuts, girara_inputbar_shortcut_t*, iter, inp_sh_it)
    if (inp_sh_it->mask == modifier && inp_sh_it->key == key) {
      inp_sh_it->function = function;
      inp_sh_it->argument = argument;

      girara_list_iterator_free(iter);
      return TRUE;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.inputbar_shortcuts, girara_inputbar_shortcut_t*, iter, inp_sh_it);

  /* create new inputbar shortcut */
  girara_inputbar_shortcut_t* inputbar_shortcut = g_slice_new(girara_inputbar_shortcut_t);

  inputbar_shortcut->mask     = modifier;
  inputbar_shortcut->key      = key;
  inputbar_shortcut->function = function;
  inputbar_shortcut->argument = argument;

  girara_list_append(session->bindings.inputbar_shortcuts, inputbar_shortcut);
  return TRUE;
}

void
girara_inputbar_shortcut_free(girara_inputbar_shortcut_t* inputbar_shortcut)
{
  g_slice_free(girara_inputbar_shortcut_t, inputbar_shortcut);
}

bool
girara_special_command_add(girara_session_t* session, char identifier, girara_inputbar_special_function_t function, bool always, int argument_n, void* argument_data)
{
  g_return_val_if_fail(session  != NULL, FALSE);
  g_return_val_if_fail(function != NULL, FALSE);

  girara_argument_t argument = {argument_n, argument_data};

  /* search for existing special command */
  girara_special_command_t* scommand_it = session->bindings.special_commands;
  girara_special_command_t* last_scommand = scommand_it;

  while (scommand_it) {
    if (scommand_it->identifier == identifier) {
      scommand_it->function = function;
      scommand_it->always   = always;
      scommand_it->argument = argument;
      return TRUE;
    }

    last_scommand = scommand_it;
    scommand_it = scommand_it->next;
  }

  /* create new special command */
  girara_special_command_t* special_command = g_slice_new(girara_special_command_t);

  special_command->identifier = identifier;
  special_command->function   = function;
  special_command->always     = always;
  special_command->argument   = argument;
  special_command->next       = NULL;

  if (last_scommand) {
    last_scommand->next = special_command;
  } else {
    session->bindings.special_commands = special_command;
  }

  return TRUE;
}

bool
girara_mouse_event_add(girara_session_t* session, guint mask, guint button, girara_shortcut_function_t function, girara_mode_t mode, int argument_n, void* argument_data)
{
  g_return_val_if_fail(session  != NULL, FALSE);
  g_return_val_if_fail(function != NULL, FALSE);

  girara_argument_t argument = {argument_n, argument_data};

  /* search for existing binding */
  girara_mouse_event_t* me_it = session->bindings.mouse_events;
  girara_mouse_event_t* last_me = me_it;

  while (me_it) {
    if (me_it->mask == mask && me_it->button == button &&
       me_it->mode == mode)
    {
      me_it->function = function;
      me_it->argument = argument;
      return TRUE;
    }

    last_me = me_it;
    me_it = me_it->next;
  }

  /* add new mouse event */
  girara_mouse_event_t* mouse_event = g_slice_new(girara_mouse_event_t);

  mouse_event->mask     = mask;
  mouse_event->button   = button;
  mouse_event->function = function;
  mouse_event->mode     = mode;
  mouse_event->argument = argument;
  mouse_event->next     = NULL;

  if (last_me) {
    last_me->next = mouse_event;
  } else {
    session->bindings.mouse_events = mouse_event;
  }

  return TRUE;
}

girara_statusbar_item_t*
girara_statusbar_item_add(girara_session_t* session, bool expand, bool fill, bool left, girara_statusbar_event_t callback)
{
  g_return_val_if_fail(session != NULL && session->elements.statusbar_items, FALSE);

  girara_statusbar_item_t* item = g_slice_new(girara_statusbar_item_t);

  item->box  = gtk_event_box_new();
  item->text = GTK_LABEL(gtk_label_new(NULL));

  /* set style */
#if (GTK_MAJOR_VERSION == 3)
  gtk_widget_override_background_color(GTK_WIDGET(item->box),  GTK_STATE_NORMAL, &(session->style.statusbar_background));
  gtk_widget_override_color(GTK_WIDGET(item->text),            GTK_STATE_NORMAL, &(session->style.statusbar_foreground));
  gtk_widget_override_font(GTK_WIDGET(item->text),             session->style.font);
#else
  gtk_widget_modify_bg(GTK_WIDGET(item->box),      GTK_STATE_NORMAL, &(session->style.statusbar_background));
  gtk_widget_modify_fg(GTK_WIDGET(item->text),     GTK_STATE_NORMAL, &(session->style.statusbar_foreground));
  gtk_widget_modify_font(GTK_WIDGET(item->text),   session->style.font);
#endif

  /* set properties */
  gtk_misc_set_alignment(GTK_MISC(item->text),     left ? 0.0 : 1.0, 0.0);
  gtk_misc_set_padding(GTK_MISC(item->text),       2, 4);
  gtk_label_set_use_markup(item->text,             TRUE);

  if (callback) {
    g_signal_connect(G_OBJECT(item->box), "button-press-event", G_CALLBACK(callback), session);
  }

  /* add it to the list */
  gtk_container_add(GTK_CONTAINER(item->box), GTK_WIDGET(item->text));
  gtk_box_pack_start(session->gtk.statusbar_entries, GTK_WIDGET(item->box), expand, fill, 2);
  gtk_widget_show_all(GTK_WIDGET(item->box));

  girara_list_prepend(session->elements.statusbar_items, item);
  return item;
}

void
girara_statusbar_item_free(girara_statusbar_item_t* item)
{
  g_slice_free(girara_statusbar_item_t, item);
}

bool
girara_statusbar_item_set_text(girara_session_t* session, girara_statusbar_item_t* item, char* text)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(item    != NULL, FALSE);

  char* escaped_text = g_markup_escape_text(text, -1);
  gtk_label_set_markup((GtkLabel*) item->text, escaped_text);
  g_free(escaped_text);

  return TRUE;
}

bool
girara_statusbar_item_set_foreground(girara_session_t* session, girara_statusbar_item_t* item, char* color)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(item    != NULL, FALSE);

  GdkColor gdk_color;
  gdk_color_parse(color, &gdk_color);
  gtk_widget_modify_fg(GTK_WIDGET(item->text), GTK_STATE_NORMAL, &gdk_color);

  return TRUE;
}

bool
girara_statusbar_set_background(girara_session_t* session, char* color)
{
  g_return_val_if_fail(session != NULL, FALSE);

#if (GTK_MAJOR_VERSION == 3)
  GdkRGBA gdk_color;
  gdk_rgba_parse(&gdk_color, color);
  gtk_widget_override_background_color(GTK_WIDGET(session->gtk.statusbar), GTK_STATE_NORMAL, &gdk_color);
#else
  GdkColor gdk_color;
  gdk_color_parse(color, &gdk_color);
  gtk_widget_modify_bg(GTK_WIDGET(session->gtk.statusbar), GTK_STATE_NORMAL, &gdk_color);
#endif

  return TRUE;
}

bool
girara_set_view(girara_session_t* session, GtkWidget* widget)
{
  g_return_val_if_fail(session != NULL, FALSE);

  GtkWidget* child = gtk_bin_get_child(GTK_BIN(session->gtk.viewport));

  if (child) {
    g_object_ref(child);
    gtk_container_remove(GTK_CONTAINER(session->gtk.viewport), child);
  }

  gtk_container_add(GTK_CONTAINER(session->gtk.viewport), widget);
  gtk_widget_show_all(widget);

  return TRUE;
}

bool
girara_isc_abort(girara_session_t* session, girara_argument_t* UNUSED(argument), unsigned int UNUSED(t))
{
  /* hide completion */
  girara_argument_t arg = { GIRARA_HIDE, NULL };
  girara_isc_completion(session, &arg, 0);

  /* clear inputbar */
  gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar), 0, -1);

  /* grab view */
  gtk_widget_grab_focus(GTK_WIDGET(session->gtk.view));

  /* hide inputbar */
  gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar));

  return true;
}

bool
girara_isc_string_manipulation(girara_session_t* session, girara_argument_t* argument, unsigned int UNUSED(t))
{
  gchar *separator = girara_setting_get(session, "word-separator");
  gchar *input  = gtk_editable_get_chars(GTK_EDITABLE(session->gtk.inputbar), 0, -1);
  int    length = strlen(input);
  int pos       = gtk_editable_get_position(GTK_EDITABLE(session->gtk.inputbar));
  int i;

  switch (argument->n) {
    case GIRARA_DELETE_LAST_WORD:
      i = pos - 1;

      if (!pos) {
        return false;
      }

      /* remove trailing spaces */
      for (; i >= 0 && input[i] == ' '; i--);

      /* find the beginning of the word */
      while ((i == (pos - 1)) || ((i > 0) && !strchr(separator, input[i]))) {
        i--;
      }

      gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar),  i + 1, pos);
      gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar), i + 1);
      break;
    case GIRARA_DELETE_LAST_CHAR:
      if ((length - 1) <= 0) {
        girara_isc_abort(session, argument, 0);
      }
      gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar), pos - 1, pos);
      break;
    case GIRARA_DELETE_TO_LINE_START:
      gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar), 1, pos);
      break;
    case GIRARA_NEXT_CHAR:
      gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar), pos + 1);
      break;
    case GIRARA_PREVIOUS_CHAR:
      gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar), (pos == 0) ? 0 : pos - 1);
      break;
  }

  g_free(separator);
  g_free(input);

  return false;
}

/* default shortcut implementation */
bool
girara_sc_focus_inputbar(girara_session_t* session, girara_argument_t* argument, unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  if (gtk_widget_get_visible(GTK_WIDGET(session->gtk.inputbar)) == FALSE) {
    gtk_widget_show(GTK_WIDGET(session->gtk.inputbar));
  }

  if (gtk_widget_get_visible(GTK_WIDGET(session->gtk.notification_area)) == TRUE) {
    gtk_widget_hide(GTK_WIDGET(session->gtk.notification_area));
  }

  if (argument->data) {
    gtk_entry_set_text(session->gtk.inputbar, (char*) argument->data);

    /* we save the X clipboard that will be clear by "grab_focus" */
    gchar* x_clipboard_text = gtk_clipboard_wait_for_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY));

    gtk_widget_grab_focus(GTK_WIDGET(session->gtk.inputbar));
    gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar), -1);

    if (x_clipboard_text != NULL) {
      /* we reset the X clipboard with saved text */
      gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY), x_clipboard_text, -1);
      g_free(x_clipboard_text);
    }
  }

  return true;
}

bool
girara_sc_abort(girara_session_t* session, girara_argument_t* UNUSED(argument), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  girara_isc_abort(session, NULL, 0);
  gtk_widget_hide(GTK_WIDGET(session->gtk.notification_area));

  return false;
}

bool
girara_sc_quit(girara_session_t* session, girara_argument_t* UNUSED(argument), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  girara_argument_t arg = { GIRARA_HIDE, NULL };
  girara_isc_completion(session, &arg, 0);

  gtk_main_quit();

  return false;
}

bool
girara_sc_tab_close(girara_session_t* session, girara_argument_t* UNUSED(argument), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  girara_tab_t* tab = girara_tab_current_get(session);

  if (tab != NULL) {
    girara_tab_remove(session, tab);
  }

  return false;
}

bool
girara_sc_tab_navigate(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);

  unsigned int number_of_tabs = girara_get_number_of_tabs(session);
  unsigned int current_tab    = girara_tab_position_get(session, girara_tab_current_get(session));
  unsigned int step           = (argument->n == GIRARA_PREVIOUS) ? -1 : 1;
  unsigned int new_tab        = (current_tab + step) % number_of_tabs;

  if (t != 0 && t <= number_of_tabs) {
    new_tab = t - 1;
  }

  girara_tab_t* tab = girara_tab_get(session, new_tab);

  if (tab != NULL) {
    girara_tab_current_set(session, tab);
  }

  girara_tab_update(session);

  return false;
}

void
girara_toggle_widget_visibility(GtkWidget* widget)
{
  if (widget == NULL) {
    return;
  }

  if (gtk_widget_get_visible(widget)) {
    gtk_widget_hide(widget);
  } else {
    gtk_widget_show(widget);
  }
}

bool
girara_sc_toggle_inputbar(girara_session_t* session, girara_argument_t* UNUSED(argument), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  girara_toggle_widget_visibility(GTK_WIDGET(session->gtk.inputbar));

  return true;
}

bool
girara_sc_toggle_statusbar(girara_session_t* session, girara_argument_t* UNUSED(argument), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  girara_toggle_widget_visibility(GTK_WIDGET(session->gtk.statusbar));

  return true;
}

bool
girara_sc_toggle_tabbar(girara_session_t* session, girara_argument_t* UNUSED(argument), unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  girara_toggle_widget_visibility(GTK_WIDGET(session->gtk.tabbar));

  return true;
}

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

    girara_argument_mapping_t* mapping = session->config.argument_mappings;
    while (mapping) {
      if (!g_strcmp0(tmp, mapping->identifier)) {
        shortcut_argument_n = mapping->value;
        break;
      }

      mapping = mapping->next;
    }

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

/* callback implementation */
bool
girara_callback_view_key_press_event(GtkWidget* UNUSED(widget), GdkEventKey* event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);

  GIRARA_LIST_FOREACH(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcut)
    if (session->buffer.command) {
      break;
    }
    if (
       event->keyval == shortcut->key
       && (CLEAN(event->state) == shortcut->mask || (shortcut->key >= 0x21
       && shortcut->key <= 0x7E && CLEAN(event->state) == GDK_SHIFT_MASK))
       && (session->modes.current_mode & shortcut->mode || shortcut->mode == 0)
       && shortcut->function
      )
    {
      int t = (session->buffer.n > 0) ? session->buffer.n : 1;
      for (int i = 0; i < t; i++) {
        if (!shortcut->function(session, &(shortcut->argument), session->buffer.n)) {
          break;
        }
      }

      if (session->global.buffer) {
        g_string_free(session->global.buffer, TRUE);
        session->global.buffer = NULL;
      }

      session->buffer.n = 0;

      if (session->events.buffer_changed) {
        session->events.buffer_changed(session);
      }

      girara_list_iterator_free(iter);
      return TRUE;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcut);

  /* update buffer */
  if (event->keyval >= 0x21 && event->keyval <= 0x7E) {
    /* overall buffer */
    if (!session->global.buffer) {
      session->global.buffer = g_string_new("");
    }

    session->global.buffer = g_string_append_c(session->global.buffer, event->keyval);

    if (!session->buffer.command && event->keyval >= 0x30 && event->keyval <= 0x39) {
      if (((session->buffer.n * 10) + (event->keyval - '0')) < INT_MAX) {
        session->buffer.n = (session->buffer.n * 10) + (event->keyval - '0');
      }
    } else {
      if (!session->buffer.command) {
        session->buffer.command = g_string_new("");
      }

      session->buffer.command = g_string_append_c(session->buffer.command, event->keyval);
    }

    if (session->events.buffer_changed) {
      session->events.buffer_changed(session);
    }
  }

  /* check for buffer command */
  if (session->buffer.command) {
    bool matching_command = FALSE;

    GIRARA_LIST_FOREACH(session->bindings.shortcuts, girara_shortcut_t*, iter, shortcut)
      if (shortcut->buffered_command) {
        /* buffer could match a command */
        if (!strncmp(session->buffer.command->str, shortcut->buffered_command, session->buffer.command->len)) {
          /* command matches buffer exactly */
          if (!strcmp(session->buffer.command->str, shortcut->buffered_command)) {
            g_string_free(session->buffer.command, TRUE);
            g_string_free(session->global.buffer,  TRUE);
            session->buffer.command = NULL;
            session->global.buffer  = NULL;

            if (session->events.buffer_changed) {
              session->events.buffer_changed(session);
            }

            int t = (session->buffer.n > 0) ? session->buffer.n : 1;
            for (int i = 0; i < t; i++) {
              if (!shortcut->function(session, &(shortcut->argument), session->buffer.n)) {
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
    if (!matching_command) {
      g_string_free(session->buffer.command, TRUE);
      g_string_free(session->global.buffer,  TRUE);
      session->buffer.command = NULL;
      session->global.buffer  = NULL;
      session->buffer.n       = 0;

      if (session->events.buffer_changed) {
        session->events.buffer_changed(session);
      }
    }
  }

  return FALSE;
}

bool
girara_callback_inputbar_activate(GtkEntry* entry, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);

  gchar *input  = gtk_editable_get_chars(GTK_EDITABLE(entry), 1, -1);
  if (!input) {
    return FALSE;
  }

  if (strlen(input) == 0) {
    return FALSE;
  }

  gchar** argv = NULL;
  gint    argc = 0;

  if (g_shell_parse_argv(input, &argc, &argv, NULL) == FALSE) {
    g_free(input);
    return FALSE;
  }

  gchar *cmd = argv[0];

  /* special commands */
  char *identifier_s = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, 1);
  char identifier    = identifier_s[0];
  g_free(identifier_s);

  girara_special_command_t* special_command = session->bindings.special_commands;
  while (special_command) {
    if (special_command->identifier == identifier) {
      if (special_command->always != true) {
        special_command->function(session, input, &(special_command->argument));
      }

      g_free(input);
      g_strfreev(argv);

      girara_isc_abort(session, NULL, 0);

      return TRUE;
    }

    special_command = special_command->next;
  }

  /* search commands */
  girara_command_t* command = session->bindings.commands;
  while (command) {
    if ((g_strcmp0(cmd, command->command) == 0) ||
       (g_strcmp0(cmd, command->abbr)    == 0))
    {
      girara_list_t* argument_list = girara_list_new();
      if (argument_list == NULL) {
        g_free(input);
        g_strfreev(argv);
        return FALSE;
      }

      girara_list_set_free_function(argument_list, g_free);

      for(int i = 1; i < argc; i++) {
        char* argument = g_strdup(argv[i]);
        if (argument != NULL) {
          girara_list_append(argument_list, (void*) argument);
        } else {
          girara_list_free(argument_list);
          g_free(input);
          g_strfreev(argv);
          return FALSE;
        }
      }

      command->function(session, argument_list);

      girara_list_free(argument_list);
      g_free(input);
      g_strfreev(argv);

      girara_isc_abort(session, NULL, 0);

      gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar));
      return TRUE;
    }

    command = command->next;
  }

  /* no known command */
  girara_isc_abort(session, NULL, 0);

  return FALSE;
}

bool
girara_callback_inputbar_key_press_event(GtkWidget* entry, GdkEventKey* event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, false);

  GIRARA_LIST_FOREACH(session->bindings.inputbar_shortcuts, girara_inputbar_shortcut_t*, iter, inputbar_shortcut)
    if (inputbar_shortcut->key == event->keyval
     && inputbar_shortcut->mask == CLEAN(event->state))
    {
      if (inputbar_shortcut->function != NULL) {
        inputbar_shortcut->function(session, &(inputbar_shortcut->argument), 0);
      }

      girara_list_iterator_free(iter);
      return true;
    }
  GIRARA_LIST_FOREACH_END(session->bindings.inputbar_shortcuts, girara_inputbar_shortcut_t*, iter, inputbar_shortcut);

  /* special commands */
  char *identifier_s = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, 1);
  char identifier    = identifier_s[0];
  g_free(identifier_s);

  girara_special_command_t* special_command = session->bindings.special_commands;
  while (special_command) {
    if ((special_command->identifier == identifier) &&
       (special_command->always == true))
    {
      gchar *input  = gtk_editable_get_chars(GTK_EDITABLE(entry), 1, -1);
      special_command->function(session, input, &(special_command->argument));
      g_free(input);
      return false;
    }

    special_command = special_command->next;
  }

  if ((session->gtk.results != NULL) &&
     (gtk_widget_get_visible(GTK_WIDGET(session->gtk.results)) == TRUE) &&
#if (GTK_MAJOR_VERSION == 3)
     (event->keyval == GDK_KEY_space))
#else
     (event->keyval == GDK_space))
#endif
  {
    gtk_widget_hide(GTK_WIDGET(session->gtk.results));
  }

  return false;
}

void
girara_mode_set(girara_session_t* session, girara_mode_t mode)
{
  g_return_if_fail(session != NULL);

  session->modes.current_mode = mode;
}

girara_mode_t
girara_mode_add(girara_session_t* session, const char* name)
{
  g_return_val_if_fail(session  != NULL, FALSE);
  g_return_val_if_fail(name != NULL && name[0] != 0x0, FALSE);

  girara_mode_t last_index = 0;
  GIRARA_LIST_FOREACH(session->modes.identifiers, girara_mode_string_t*, iter, mode)
    if (mode->index > last_index) {
      last_index = mode->index;
    }
  GIRARA_LIST_FOREACH_END(session->modes.identifiers, girara_mode_string_t*, iter, mode);

  /* create new mode identifier */
  girara_mode_string_t* mode = g_slice_new(girara_mode_string_t);
  mode->index = last_index + 1;
  mode->name = g_strdup(name);
  girara_list_append(session->modes.identifiers, mode);

  return (1 << mode->index);
}

void
girara_mode_string_free(girara_mode_string_t* mode)
{
  if (!mode) {
    return;
  }

  g_free(mode->name);
  g_slice_free(girara_mode_string_t, mode);
}

girara_mode_t
girara_mode_get(girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, 0);

  return session->modes.current_mode;
}

bool girara_shortcut_mapping_add(girara_session_t* session, char* identifier, girara_shortcut_function_t function)
{
  g_return_val_if_fail(session  != NULL, FALSE);

  if (function == NULL || identifier == NULL) {
    return false;
  }

  GIRARA_LIST_FOREACH(session->config.shortcut_mappings, girara_shortcut_mapping_t*, iter, data)
    if (strcmp(data->identifier, identifier) == 0) {
      data->function = function;
      girara_list_iterator_free(iter);
      return true;
    }
  GIRARA_LIST_FOREACH_END(session->config.shortcut_mappings, girara_shortcut_mapping_t*, iter, data);

  /* add new config handle */
  girara_shortcut_mapping_t* mapping = g_slice_new(girara_shortcut_mapping_t);

  mapping->identifier = g_strdup(identifier);
  mapping->function   = function;
  girara_list_append(session->config.shortcut_mappings, mapping);

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

bool girara_argument_mapping_add(girara_session_t* session, char* identifier, int value)
{
  g_return_val_if_fail(session  != NULL, FALSE);

  if (identifier == NULL) {
    return false;
  }

  girara_argument_mapping_t* mapping_it   = session->config.argument_mappings;
  girara_argument_mapping_t* last_mapping = mapping_it;

  while (mapping_it) {
    if (strcmp(mapping_it->identifier, identifier) == 0) {
      mapping_it->value = value;
      return true;
    }

    last_mapping = mapping_it;
    mapping_it = mapping_it->next;
  }

  /* add new config handle */
  girara_argument_mapping_t* mapping = g_slice_new(girara_argument_mapping_t);

  mapping->identifier = g_strdup(identifier);
  mapping->value      = value;
  mapping->next       = NULL;

  if (last_mapping) {
    last_mapping->next = mapping;
  } else {
    session->config.argument_mappings = mapping;
  }

  return true;
}

char*
girara_buffer_get(girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, NULL);

  return (session->global.buffer) ? g_strdup(session->global.buffer->str) : NULL;
}

void
girara_tabs_enable(girara_session_t* session)
{
  if (session == NULL || session->gtk.tabs == NULL) {
    return;
  }

  /* Display tab view */
  girara_set_view(session, GTK_WIDGET(session->gtk.tabs));

  /* Display tab bar */
  if (session->gtk.tabbar) {
    gtk_widget_show(session->gtk.tabbar);
  }
}

girara_tab_t*
girara_tab_new(girara_session_t* session, const char* title, GtkWidget* widget,
    bool next_to_current, void* data)
{
  if (session == NULL || widget == NULL) {
    return NULL;
  }

  girara_tab_t* tab = g_slice_new(girara_tab_t);

  tab->title   = title ? g_strdup(title) : g_strdup(UNTITLED_TAB_TITLE);
  tab->widget  = widget;
  tab->session = session;
  tab->data    = data;

  int position = (next_to_current) ?
    (gtk_notebook_get_current_page(session->gtk.tabs) + 1) : -1;

  /* insert tab into notebook */
  if (gtk_notebook_insert_page(session->gtk.tabs, tab->widget, NULL, position) == -1) {
    g_free(tab->title);
    g_slice_free(girara_tab_t, tab);
    return NULL;
  }

  /* create tab label */
  GtkWidget *tab_label = gtk_label_new(tab->title);
  GtkWidget *tab_event = gtk_event_box_new();

  g_object_set_data(G_OBJECT(tab->widget), "event", (gpointer) tab_event);
  g_object_set_data(G_OBJECT(tab->widget), "label", (gpointer) tab_label);
  g_object_set_data(G_OBJECT(tab->widget), "tab",   (gpointer) tab);

  g_signal_connect(G_OBJECT(tab_event), "button_press_event",
      G_CALLBACK(girara_callback_tab_clicked), tab);

  gtk_misc_set_alignment(GTK_MISC(tab_label), 0.0f, 0.0f);
  gtk_misc_set_padding(GTK_MISC(tab_label),   4, 4);
  gtk_widget_modify_font(tab_label, session->style.font);

  gtk_container_add(GTK_CONTAINER(tab_event), tab_label);
  gtk_box_pack_start(GTK_BOX(session->gtk.tabbar), tab_event, TRUE, TRUE, 0);
  gtk_box_reorder_child(GTK_BOX(session->gtk.tabbar), tab_event, position);

  gtk_widget_show_all(widget);
  gtk_widget_show_all(tab_event);

  gtk_notebook_set_current_page(session->gtk.tabs, position);

  girara_tab_update(session);

  return tab;
}

void
girara_tab_remove(girara_session_t* session, girara_tab_t* tab)
{
  if (session == NULL || tab == NULL || session->gtk.tabbar == NULL) {
    return;
  }

  /* Remove page from notebook */
  int tab_id = girara_tab_position_get(session, tab);

  /* Remove entry from tabbar */
  GtkWidget* tab_event = GTK_WIDGET(g_object_get_data(G_OBJECT(tab->widget), "event"));

  if (tab_event != NULL) {
    gtk_container_remove(GTK_CONTAINER(session->gtk.tabbar), tab_event);
  }

  if (tab_id != -1) {
    gtk_notebook_remove_page(session->gtk.tabs, tab_id);
  }

  g_free(tab->title);
  g_slice_free(girara_tab_t, tab);

  girara_tab_update(session);
}

girara_tab_t*
girara_tab_get(girara_session_t* session, unsigned int index)
{
  if (session == NULL || session->gtk.tabs == NULL) {
    return 0;
  }

  GtkWidget* widget = gtk_notebook_get_nth_page(session->gtk.tabs, index);

  return (girara_tab_t*) g_object_get_data(G_OBJECT(widget), "tab");
}

int
girara_get_number_of_tabs(girara_session_t* session)
{
  if (session == NULL || session->gtk.tabs == NULL) {
    return 0;
  }

  return gtk_notebook_get_n_pages(session->gtk.tabs);
}

void
girara_tab_update(girara_session_t* session)
{
  if (session == NULL || session->gtk.tabs == NULL) {
    return;
  }

  int number_of_tabs = girara_get_number_of_tabs(session);
  int current_tab    = girara_tab_position_get(session, girara_tab_current_get(session));

  for (int i = 0; i < number_of_tabs; i++) {
    GtkWidget* widget = gtk_notebook_get_nth_page(session->gtk.tabs, i);
    girara_tab_t* tab = (girara_tab_t*) g_object_get_data(G_OBJECT(widget), "tab");

    if (tab == NULL) {
      continue;
    }

    GtkWidget* tab_event = GTK_WIDGET(g_object_get_data(G_OBJECT(tab->widget), "event"));
    GtkWidget* tab_label = GTK_WIDGET(g_object_get_data(G_OBJECT(tab->widget), "label"));

#if (GTK_MAJOR_VERSION == 3)
    if (i == current_tab) {
      gtk_widget_override_background_color(tab_event, GTK_STATE_NORMAL, &(session->style.tabbar_focus_background));
      gtk_widget_override_color(tab_label, GTK_STATE_NORMAL, &(session->style.tabbar_focus_foreground));
    } else {
      gtk_widget_override_background_color(tab_event, GTK_STATE_NORMAL, &(session->style.tabbar_background));
      gtk_widget_override_color(tab_label, GTK_STATE_NORMAL, &(session->style.tabbar_foreground));
    }
#else
    if (i == current_tab) {
      gtk_widget_modify_bg(tab_event, GTK_STATE_NORMAL, &(session->style.tabbar_focus_background));
      gtk_widget_modify_fg(tab_label, GTK_STATE_NORMAL, &(session->style.tabbar_focus_foreground));
    } else {
      gtk_widget_modify_bg(tab_event, GTK_STATE_NORMAL, &(session->style.tabbar_background));
      gtk_widget_modify_fg(tab_label, GTK_STATE_NORMAL, &(session->style.tabbar_foreground));
    }
#endif
  }
}

girara_tab_t*
girara_tab_current_get(girara_session_t* session)
{
  if (session == NULL || session->gtk.tabs == NULL) {
    return NULL;
  }

  int current = gtk_notebook_get_current_page(session->gtk.tabs);

  if (current != -1) {
    GtkWidget* widget = gtk_notebook_get_nth_page(session->gtk.tabs, current);
    return (girara_tab_t*) g_object_get_data(G_OBJECT(widget), "tab");
  } else {
    return NULL;
  }
}

void
girara_tab_current_set(girara_session_t* session, girara_tab_t* tab)
{
  if (session == NULL || session->gtk.tabs == NULL
      || tab == NULL || tab->widget == NULL) {
    return;
  }

  int index = gtk_notebook_page_num(session->gtk.tabs, tab->widget);

  if (index != -1) {
    gtk_notebook_set_current_page(session->gtk.tabs, index);
  }

  girara_tab_update(session);
}

void
girara_tab_title_set(girara_tab_t* tab, const char* title)
{
  if (tab == NULL) {
    return;
  }

  g_free(tab->title);
  tab->title = title ? g_strdup(title) : g_strdup(UNTITLED_TAB_TITLE);

  GtkWidget* tab_label = GTK_WIDGET(g_object_get_data(G_OBJECT(tab->widget), "label"));
  if (tab_label) {
    gtk_label_set_text(GTK_LABEL(tab_label), tab->title);
  }
}

const char*
girara_tab_title_get(girara_tab_t* tab)
{
  if (tab == NULL) {
    return NULL;
  }

  return tab->title;
}

int
girara_tab_position_get(girara_session_t* session, girara_tab_t* tab)
{
  if (session == NULL || session->gtk.tabs == NULL
      || tab == NULL || tab->widget == NULL) {
    return -1;
  }

  return gtk_notebook_page_num(session->gtk.tabs, tab->widget);
}

void
girara_tab_position_set(girara_session_t* session, girara_tab_t* tab, unsigned int position)
{
  if (session == NULL || session->gtk.tabs == NULL
      || tab == NULL || tab->widget == NULL) {
    return;
  }

  gtk_notebook_reorder_child(session->gtk.tabs, tab->widget, position);
}

bool
girara_callback_tab_clicked(GtkWidget* UNUSED(widget), GdkEventButton* event, gpointer data)
{
  if (data == NULL) {
    return false;
  }

  girara_tab_t* tab         = (girara_tab_t*) data;
  girara_session_t* session = tab->session;

  switch (event->button) {
    case 1:
      girara_tab_current_set(session, tab);
      break;
    case 2:
      girara_tab_remove(session, tab);
      break;
  }

  return true;
}

void
girara_notify(girara_session_t* session, int level, const char* format, ...)
{
  if (session == NULL || session->gtk.notification_text == NULL ||
      session->gtk.notification_area == NULL) {
    return;
  }

  switch (level) {
    case GIRARA_ERROR:
#if (GTK_MAJOR_VERSION == 3)
      gtk_widget_override_background_color(GTK_WIDGET(session->gtk.notification_area),
          GTK_STATE_NORMAL, &(session->style.notification_error_background));
      gtk_widget_override_color(GTK_WIDGET(session->gtk.notification_text),
          GTK_STATE_NORMAL, &(session->style.notification_error_foreground));
#else
      gtk_widget_modify_bg(GTK_WIDGET(session->gtk.notification_area),
          GTK_STATE_NORMAL, &(session->style.notification_error_background));
      gtk_widget_modify_text(GTK_WIDGET(session->gtk.notification_text),
          GTK_STATE_NORMAL, &(session->style.notification_error_foreground));
#endif
      break;
    case GIRARA_WARNING:
#if (GTK_MAJOR_VERSION == 3)
      gtk_widget_override_background_color(GTK_WIDGET(session->gtk.notification_area),
          GTK_STATE_NORMAL, &(session->style.notification_warning_background));
      gtk_widget_override_color(GTK_WIDGET(session->gtk.notification_text),
          GTK_STATE_NORMAL, &(session->style.notification_warning_foreground));
#else
      gtk_widget_modify_bg(GTK_WIDGET(session->gtk.notification_area),
          GTK_STATE_NORMAL, &(session->style.notification_warning_background));
      gtk_widget_modify_text(GTK_WIDGET(session->gtk.notification_text),
          GTK_STATE_NORMAL, &(session->style.notification_warning_foreground));
#endif
      break;
    case GIRARA_INFO:
#if (GTK_MAJOR_VERSION == 3)
      gtk_widget_override_background_color(GTK_WIDGET(session->gtk.notification_area),
          GTK_STATE_NORMAL, &(session->style.notification_default_background));
      gtk_widget_override_color(GTK_WIDGET(session->gtk.notification_text),
          GTK_STATE_NORMAL, &(session->style.notification_default_foreground));
#else
      gtk_widget_modify_bg(GTK_WIDGET(session->gtk.notification_area),
          GTK_STATE_NORMAL, &(session->style.notification_default_background));
      gtk_widget_modify_text(GTK_WIDGET(session->gtk.notification_text),
          GTK_STATE_NORMAL, &(session->style.notification_default_foreground));
#endif
      break;
    default:
      return;
  }

  /* prepare message */
  va_list ap;
  va_start(ap, format);
  char* message = g_strdup_vprintf(format, ap);
  va_end(ap);

  gtk_label_set_markup(GTK_LABEL(session->gtk.notification_text), message);
  g_free(message);

  /* update visibility */
  gtk_widget_show(GTK_WIDGET(session->gtk.notification_area));
  gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar));
}
