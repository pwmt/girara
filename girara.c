/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#include "girara.h"

#define CLEAN(m) (m & ~(GDK_MOD2_MASK) & ~(GDK_BUTTON1_MASK) & ~(GDK_BUTTON2_MASK) & ~(GDK_BUTTON3_MASK) & ~(GDK_BUTTON4_MASK) & ~(GDK_BUTTON5_MASK) & ~(GDK_LEAVE_NOTIFY_MASK))
#define FORMAT_COMMAND "<b>%s</b>"
#define FORMAT_DESCRIPTION "<i>%s</i>"

/* default shortcuts */
void girara_sc_focus_inputbar(girara_session_t*, girara_argument_t*);
void girara_sc_quit(girara_session_t*, girara_argument_t*);

/* default commands */
gboolean girara_cmd_map(girara_session_t*, int, char**);
gboolean girara_cmd_quit(girara_session_t*, int, char**);
gboolean girara_cmd_set(girara_session_t*, int, char**);

/* callback declarations */
gboolean girara_callback_view_key_press_event(GtkWidget*, GdkEventKey*, girara_session_t*);
gboolean girara_callback_inputbar_activate(GtkEntry*, girara_session_t*);
gboolean girara_callback_inputbar_key_press_event(GtkWidget*, GdkEventKey*, girara_session_t*);

/* header functions implementation */
GtkEventBox* girara_completion_row_create(girara_session_t*, GtkBox*, char*, char*, gboolean);
void girara_completion_row_set_color(girara_session_t*, GtkBox*, int, int);

girara_session_t*
girara_session_create()
{
  girara_session_t* session = g_slice_new(girara_session_t);

  /* init values */
  session->gtk.window            = NULL;
  session->gtk.box               = NULL;
  session->gtk.view              = NULL;
  session->gtk.statusbar         = NULL;
  session->gtk.statusbar_entries = NULL;
  session->gtk.inputbar          = NULL;
  session->gtk.embed             = 0;

  session->style.font            = NULL;

  session->bindings.mouse_events       = NULL;
  session->bindings.commands           = NULL;
  session->bindings.special_commands   = NULL;
  session->bindings.shortcuts          = NULL;
  session->bindings.inputbar_shortcuts = NULL;

  session->elements.statusbar_items    = NULL;

  session->settings.settings                        = NULL;
  session->settings.font                            = "monospace normal 9";
  session->settings.default_background              = "#000000";
  session->settings.default_foreground              = "#DDDDDD";
  session->settings.inputbar_background             = "#141414";
  session->settings.inputbar_foreground             = "#9FBC00";
  session->settings.statusbar_background            = "#000000";
  session->settings.statusbar_foreground            = "#FFFFFF";
  session->settings.completion_foreground           = "#DDDDDD";
  session->settings.completion_background           = "#232323";
  session->settings.completion_group_foreground     = "#DEDEDE";
  session->settings.completion_group_background     = "#000000";
  session->settings.completion_highlight_foreground = "#232323";
  session->settings.completion_highlight_background = "#9FBC00";
  session->settings.notification_error_background   = "#FF1212";
  session->settings.notification_error_foreground   = "#FFFFFF";
  session->settings.notification_warning_background = "#FFF712";
  session->settings.notification_warning_foreground = "#000000";

  session->settings.width              = 800;
  session->settings.height             = 600;
  session->settings.n_completion_items = 15;

  session->signals.view_key_pressed     = 0;
  session->signals.inputbar_key_pressed = 0;
  session->signals.inputbar_activate    = 0;

  session->buffer.n       = 0;
  session->buffer.command = NULL;

  session->global.current_mode       = 0;
  session->global.number_of_commands = 0;

  /* add default settings */
  girara_setting_add(session, "font",                     &(session->settings.font),                            STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "default-fg",               &(session->settings.default_foreground),              STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "default-bg",               &(session->settings.default_background),              STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "inputbar-fg",              &(session->settings.inputbar_foreground),             STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "inputbar-bg",              &(session->settings.inputbar_background),             STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "statusbar-fg",             &(session->settings.statusbar_foreground),            STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "statusbar-bg",             &(session->settings.statusbar_background),            STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "completion-fg",            &(session->settings.completion_foreground),           STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "completion-bg",            &(session->settings.completion_background),           STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "completion-group-fg",      &(session->settings.completion_group_foreground),     STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "completion-group-bg",      &(session->settings.completion_group_background),     STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "completion-highlight-fg",  &(session->settings.completion_highlight_foreground), STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "completion-highlight-bg",  &(session->settings.completion_highlight_background), STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "notification-error-fg",    &(session->settings.notification_error_foreground),   STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "notification-error-bg",    &(session->settings.notification_error_background),   STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "notification-warning-fg",  &(session->settings.notification_warning_foreground), STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "notification-warning-bg",  &(session->settings.notification_warning_background), STRING, TRUE, NULL, NULL);
  girara_setting_add(session, "width",                    &(session->settings.width),                           INT,    TRUE, NULL, NULL);
  girara_setting_add(session, "height",                   &(session->settings.height),                          INT,    TRUE, NULL, NULL);

  /* default shortcuts */
  girara_shortcut_add(session, GDK_CONTROL_MASK, GDK_q,     NULL, girara_sc_quit,           0, 0, NULL);
  girara_shortcut_add(session, 0,                GDK_colon, NULL, girara_sc_focus_inputbar, 0, 0, ":");

  /* default inputbar shortcuts */
  girara_inputbar_shortcut_add(session, 0, GDK_Tab, girara_isc_completion, GIRARA_NEXT, NULL);

  /* default commands */
  girara_inputbar_command_add(session, "map",  "m", girara_cmd_map,  NULL, "Map a key sequence");
  girara_inputbar_command_add(session, "quit", "q", girara_cmd_quit, NULL, "Quit the program");
  girara_inputbar_command_add(session, "set",  "s", girara_cmd_set,  NULL, "Set an option");

  return session;
}

gboolean
girara_session_init(girara_session_t* session)
{
  if(session->gtk.embed)
    session->gtk.window = gtk_plug_new(session->gtk.embed);
  else
    session->gtk.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  session->gtk.box               = GTK_BOX(gtk_vbox_new(FALSE, 0));
  session->gtk.view              = gtk_scrolled_window_new(NULL, NULL);
  session->gtk.statusbar         = gtk_event_box_new();
  session->gtk.statusbar_entries = GTK_BOX(gtk_hbox_new(FALSE, 0));
  session->gtk.inputbar          = GTK_ENTRY(gtk_entry_new());

  /* window */
  GdkGeometry hints = {1, 1};
  gtk_window_set_geometry_hints(GTK_WINDOW(session->gtk.window), NULL, &hints, GDK_HINT_MIN_SIZE);

  /* view */
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(girara_callback_view_key_press_event), session);

  /* box */
  gtk_box_set_spacing(session->gtk.box, 0);
  gtk_container_add(GTK_CONTAINER(session->gtk.window), GTK_WIDGET(session->gtk.box));

  /* statusbar */
  gtk_container_add(GTK_CONTAINER(session->gtk.statusbar), GTK_WIDGET(session->gtk.statusbar_entries));

  /* inputbar */
  gtk_entry_set_inner_border(session->gtk.inputbar, NULL);
  gtk_entry_set_has_frame(session->gtk.inputbar, FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(session->gtk.inputbar), TRUE);

  session->signals.inputbar_key_pressed = g_signal_connect(G_OBJECT(session->gtk.inputbar), "key-press-event", G_CALLBACK(girara_callback_inputbar_key_press_event), session);
  session->signals.inputbar_activate    = g_signal_connect(G_OBJECT(session->gtk.inputbar), "activate",        G_CALLBACK(girara_callback_inputbar_activate),        session);

  /* packing */
  gtk_box_pack_start(session->gtk.box, GTK_WIDGET(session->gtk.view),       TRUE,  TRUE, 0);
  gtk_box_pack_start(session->gtk.box, GTK_WIDGET(session->gtk.statusbar), FALSE, FALSE, 0);
  gtk_box_pack_end(  session->gtk.box, GTK_WIDGET(session->gtk.inputbar),  FALSE, FALSE, 0);

  /* parse color values */
  gdk_color_parse(session->settings.default_foreground,              &(session->style.default_foreground));
  gdk_color_parse(session->settings.default_background,              &(session->style.default_background));
  gdk_color_parse(session->settings.inputbar_foreground,             &(session->style.inputbar_foreground));
  gdk_color_parse(session->settings.inputbar_background,             &(session->style.inputbar_background));
  gdk_color_parse(session->settings.statusbar_foreground,            &(session->style.statusbar_foreground));
  gdk_color_parse(session->settings.statusbar_background,            &(session->style.statusbar_background));
  gdk_color_parse(session->settings.completion_foreground,           &(session->style.completion_foreground));
  gdk_color_parse(session->settings.completion_background,           &(session->style.completion_background));
  gdk_color_parse(session->settings.completion_group_foreground,     &(session->style.completion_group_foreground));
  gdk_color_parse(session->settings.completion_group_background,     &(session->style.completion_group_background));
  gdk_color_parse(session->settings.completion_highlight_foreground, &(session->style.completion_highlight_foreground));
  gdk_color_parse(session->settings.completion_highlight_background, &(session->style.completion_highlight_background));
  gdk_color_parse(session->settings.notification_error_foreground,   &(session->style.notification_error_foreground));
  gdk_color_parse(session->settings.notification_error_background,   &(session->style.notification_error_background));
  gdk_color_parse(session->settings.notification_warning_foreground, &(session->style.notification_warning_foreground));
  gdk_color_parse(session->settings.notification_warning_background, &(session->style.notification_warning_background));

  session->style.font = pango_font_description_from_string(session->settings.font);

  /* statusbar */
  gtk_widget_modify_bg(GTK_WIDGET(session->gtk.statusbar), GTK_STATE_NORMAL, &(session->style.statusbar_background));

  /* inputbar */
  gtk_widget_modify_base(GTK_WIDGET(session->gtk.inputbar), GTK_STATE_NORMAL, &(session->style.inputbar_background));
  gtk_widget_modify_text(GTK_WIDGET(session->gtk.inputbar), GTK_STATE_NORMAL, &(session->style.inputbar_foreground));
  gtk_widget_modify_font(GTK_WIDGET(session->gtk.inputbar),                     session->style.font);

  /* set window size */
  gtk_window_set_default_size(GTK_WINDOW(session->gtk.window), session->settings.width, session->settings.height);

  gtk_widget_show_all(GTK_WIDGET(session->gtk.window));

  return TRUE;
}

gboolean
girara_session_destroy(girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);

  /* clean up style */
  pango_font_description_free(session->style.font);

  /* clean up shortcuts */
  girara_shortcut_t* shortcut = session->bindings.shortcuts;
  while(shortcut)
  {
    girara_shortcut_t* tmp = shortcut->next;
    g_slice_free(girara_shortcut_t, shortcut);
    shortcut = tmp;
  }

  /* clean up inputbar shortcuts */
  girara_inputbar_shortcut_t* inputbar_shortcut = session->bindings.inputbar_shortcuts;
  while(inputbar_shortcut)
  {
    girara_inputbar_shortcut_t* tmp = inputbar_shortcut->next;
    g_slice_free(girara_inputbar_shortcut_t, inputbar_shortcut);
    inputbar_shortcut = tmp;
  }

  /* clean up commands */
  girara_command_t* command = session->bindings.commands;
  while(command)
  {
    girara_command_t* tmp = command->next;
    g_free(command->command);
    if(command->abbr)
      g_free(command->abbr);
    if(command->description)
      g_free(command->description);
    g_slice_free(girara_command_t, command);
    command = tmp;
  }

  /* clean up special commands */
  girara_special_command_t* special_command = session->bindings.special_commands;
  while(special_command)
  {
    girara_special_command_t* tmp = special_command->next;
    g_slice_free(girara_special_command_t, special_command);
    special_command = tmp;
  }

  /* clean up mouse events */
  girara_mouse_event_t* mouse_event = session->bindings.mouse_events;
  while(mouse_event)
  {
    girara_mouse_event_t* tmp = mouse_event->next;
    g_slice_free(girara_mouse_event_t, mouse_event);
    mouse_event = tmp;
  }

  /* clean up settings */
  girara_setting_t* setting = session->settings.settings;
  while(setting)
  {
    girara_setting_t* tmp = setting->next;

    g_free(setting->name);
    if(setting->description)
      g_free(setting->description);
    if(setting->type == STRING && setting->value.s != NULL)
      g_free(setting->value.s);
    g_slice_free(girara_setting_t, setting);

    setting = tmp;
  }

  /* clean up statusbar items */
  girara_statusbar_item_t* item = session->elements.statusbar_items;
  while(item)
  {
    girara_statusbar_item_t* tmp = item->next;
    g_slice_free(girara_statusbar_item_t, item);
    item = tmp;
  }

  /* clean up buffer */
  if(session->buffer.command) g_string_free(session->buffer.command, TRUE);
  if(session->global.buffer)  g_string_free(session->global.buffer,  TRUE);
  session->buffer.command = NULL;
  session->global.buffer  = NULL;

  g_slice_free(girara_session_t, session);
  return TRUE;
}

gboolean
girara_setting_add(girara_session_t* session, char* name, void* value, girara_setting_type_t type, gboolean init_only, char* description, girara_setting_callback_t callback)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);

  /* search for existing setting */
  girara_setting_t* tmp = session->settings.settings;
  while(tmp && tmp->next)
  {
    if(!g_strcmp0(name, tmp->name))
      return FALSE;

    tmp = tmp->next;
  }

  /* add new setting */
  girara_setting_t* setting = g_slice_new(girara_setting_t);

  setting->name        = g_strdup(name);
  setting->type        = type;
  setting->init_only   = init_only;
  setting->description = description ? g_strdup(description) : NULL;
  setting->callback    = callback;
  setting->next        = NULL;

  switch (type)
  {
    case BOOLEAN:
      setting->value.b = *((gboolean *) value);
      break;
    case FLOAT:
      setting->value.f = *((float *) value);
      break;
    case INT:
      setting->value.i = *((int *) value);
      break;
    case STRING:
      setting->value.s = g_strdup(value);
      break;
  }

  if(tmp)
    tmp->next = setting;
  if(!session->settings.settings)
    session->settings.settings = setting;

  return TRUE;
}

gboolean
girara_setting_set(girara_session_t* session, char* name, void* value)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);

  for (girara_setting_t* setting = session->settings.settings; setting != NULL; setting = setting->next)
  {
    if (g_strcmp0(setting->name, name) != 0)
      continue;

    switch (setting->type)
    {
      case BOOLEAN:
        setting->value.b = *((gboolean *) value);
        break;
      case FLOAT:
        setting->value.f = *((float *) value);
        break;
      case INT:
        setting->value.i = *((int *) value);
        break;
      case STRING:
        if (setting->value.s != NULL)
          g_free(setting->value.s);
        setting->value.s = g_strdup(value);
        break;
      default:
        return FALSE;
    }

    if (setting->callback != NULL)
      setting->callback(session, setting);

    return TRUE;
  }

  return FALSE;
}

gboolean
girara_shortcut_add(girara_session_t* session, int modifier, int key, char* buffer, girara_shortcut_function_t function, girara_mode_t mode, int argument_n, void* argument_data)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(buffer || key || modifier, FALSE);

  girara_argument_t argument = {argument_n, argument_data};

  /* search for existing binding */
  girara_shortcut_t* tmp = session->bindings.shortcuts;

  while(tmp && tmp->next)
  {
    if(((tmp->mask == modifier && tmp->key == key) ||
       (buffer && tmp->buffered_command && !strcmp(tmp->buffered_command, buffer)))
        && tmp->mode == mode)
    {
      tmp->function = function;
      tmp->argument = argument;
      return TRUE;
    }

    tmp = tmp->next;
  }

  /* add new shortcut */
  girara_shortcut_t* shortcut = g_slice_new(girara_shortcut_t);

  shortcut->mask             = modifier;
  shortcut->key              = key;
  shortcut->buffered_command = buffer;
  shortcut->function         = function;
  shortcut->mode             = mode;
  shortcut->argument         = argument;
  shortcut->next             = NULL;

  if(tmp)
    tmp->next = shortcut;
  if(!session->bindings.shortcuts)
    session->bindings.shortcuts = shortcut;

  return TRUE;
}

gboolean
girara_inputbar_command_add(girara_session_t* session, char* command , char* abbreviation, girara_command_function_t function, girara_completion_function_t completion, char* description)
{
  g_return_val_if_fail(session  != NULL, FALSE);
  g_return_val_if_fail(command  != NULL, FALSE);
  g_return_val_if_fail(function != NULL, FALSE);

  /* search for existing binding */
  girara_command_t* tmp = session->bindings.commands;

  while(tmp && tmp->next)
  {
    if(!g_strcmp0(tmp->command, command))
    {
      if(tmp->abbr)
        free(tmp->abbr);
      if(tmp->description)
        free(tmp->description);

      tmp->abbr        = abbreviation ? g_strdup(abbreviation) : NULL;
      tmp->function    = function;
      tmp->completion  = completion;
      tmp->description = description ? g_strdup(description) : NULL;
      return TRUE;
    }

    tmp = tmp->next;
  }

  /* add new inputbar command */
  girara_command_t* new_command = g_slice_new(girara_command_t);

  new_command->command     = g_strdup(command);
  new_command->abbr        = abbreviation ? g_strdup(abbreviation) : NULL;
  new_command->function    = function;
  new_command->completion  = completion;
  new_command->description = description ? g_strdup(description) : NULL;
  new_command->next        = NULL;

  if(tmp)
    tmp->next = new_command;
  if(!session->bindings.commands)
    session->bindings.commands = new_command;

  session->global.number_of_commands++;

  return TRUE;
}

gboolean
girara_inputbar_shortcut_add(girara_session_t* session, int modifier, int key, girara_shortcut_function_t function, int argument_n, void* argument_data)
{
  g_return_val_if_fail(session  != NULL, FALSE);
  g_return_val_if_fail(function != NULL, FALSE);

  girara_argument_t argument = {argument_n, argument_data};

  /* search for existing special command */
  girara_inputbar_shortcut_t* tmp = session->bindings.inputbar_shortcuts;

  while(tmp && tmp->next)
  {
    if(tmp->mask == modifier && tmp->key == key)
    {
      tmp->function = function;
      tmp->argument = argument;
      return TRUE;
    }

    tmp = tmp->next;
  }

  /* create new inputbar shortcut */
  girara_inputbar_shortcut_t* inputbar_shortcut = g_slice_new(girara_inputbar_shortcut_t);

  inputbar_shortcut->mask     = modifier;
  inputbar_shortcut->key      = key;
  inputbar_shortcut->function = function;
  inputbar_shortcut->argument = argument;

  if(tmp)
    tmp->next = inputbar_shortcut;
  if(!session->bindings.inputbar_shortcuts)
    session->bindings.inputbar_shortcuts = inputbar_shortcut;

  return TRUE;
}

gboolean
girara_special_command_add(girara_session_t* session, char identifier, girara_inputbar_special_function_t function, gboolean always, int argument_n, void* argument_data)
{
  g_return_val_if_fail(session  != NULL, FALSE);
  g_return_val_if_fail(function != NULL, FALSE);

  girara_argument_t argument = {argument_n, argument_data};

  /* search for existing special command */
  girara_special_command_t* tmp = session->bindings.special_commands;

  while(tmp && tmp->next)
  {
    if(tmp->identifier == identifier)
    {
      tmp->function = function;
      tmp->always   = always;
      tmp->argument = argument;
      return TRUE;
    }

    tmp = tmp->next;
  }

  /* create new special command */
  girara_special_command_t* special_command = g_slice_new(girara_special_command_t);

  special_command->identifier = identifier;
  special_command->function   = function;
  special_command->always     = always;
  special_command->argument   = argument;

  if(tmp)
    tmp->next = special_command;
  if(session->bindings.special_commands)
    session->bindings.special_commands = special_command;

  return TRUE;
}

gboolean
girara_mouse_event_add(girara_session_t* session, int mask, int button, girara_shortcut_function_t function, girara_mode_t mode, int argument_n, void* argument_data)
{
  g_return_val_if_fail(session  != NULL, FALSE);
  g_return_val_if_fail(function != NULL, FALSE);

  girara_argument_t argument = {argument_n, argument_data};

  /* search for existing binding */
  girara_mouse_event_t* tmp = session->bindings.mouse_events;

  while(tmp && tmp->next)
  {
    if(tmp->mask == mask && tmp->button == button &&
       tmp->mode == mode)
    {
      tmp->function = function;
      tmp->argument = argument;
      return TRUE;
    }

    tmp = tmp->next;
  }

  /* add new mouse event */
  girara_mouse_event_t* mouse_event = g_slice_new(girara_mouse_event_t);

  mouse_event->mask     = mask;
  mouse_event->button   = button;
  mouse_event->function = function;
  mouse_event->mode     = mode;
  mouse_event->argument = argument;
  mouse_event->next     = NULL;

  if(tmp)
    tmp->next = mouse_event;
  if(!session->bindings.mouse_events)
    session->bindings.mouse_events = mouse_event;

  return TRUE;
}

girara_statusbar_item_t*
girara_statusbar_item_add(girara_session_t* session, gboolean expand, gboolean fill, gboolean left, girara_statusbar_event_t callback)
{
  g_return_val_if_fail(session != NULL, FALSE);

  girara_statusbar_item_t* item = g_slice_new(girara_statusbar_item_t);

  item->text = GTK_LABEL(gtk_label_new(NULL));
  item->next = NULL;

  /* set style */
  gtk_widget_modify_fg(GTK_WIDGET(item->text),     GTK_STATE_NORMAL, &(session->style.statusbar_foreground));
  gtk_widget_modify_font(GTK_WIDGET(item->text),   session->style.font);

  /* set properties */
  gtk_misc_set_alignment(GTK_MISC(item->text),     left ? 0.0 : 1.0, 0.0);
  gtk_misc_set_padding(GTK_MISC(item->text),       2.0, 4.0);
  gtk_label_set_use_markup(item->text,             TRUE);

  if(callback)
    g_signal_connect(G_OBJECT(item->text), "event", G_CALLBACK(callback), NULL);

  /* add it to the list */
  gtk_box_pack_start(session->gtk.statusbar_entries, GTK_WIDGET(item->text), expand, fill, 2);
  gtk_widget_show_all(GTK_WIDGET(item->text));

  if(session->elements.statusbar_items)
    item->next = session->elements.statusbar_items;

  session->elements.statusbar_items = item;

  return item;
}

gboolean
girara_statusbar_item_set_text(girara_session_t* session, girara_statusbar_item_t* item, char* text)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(item    != NULL, FALSE);

  char* escaped_text = g_markup_escape_text(text, -1);
  gtk_label_set_markup((GtkLabel*) item->text, escaped_text);
  g_free(escaped_text);

  return TRUE;
}

gboolean
girara_statusbar_item_set_foreground(girara_session_t* session, girara_statusbar_item_t* item, char* color)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(item    != NULL, FALSE);

  GdkColor gdk_color;
  gdk_color_parse(color, &gdk_color);
  gtk_widget_modify_fg(GTK_WIDGET(item->text), GTK_STATE_NORMAL, &gdk_color);

  return TRUE;
}

gboolean
girara_statusbar_set_background(girara_session_t* session, char* color)
{
  g_return_val_if_fail(session != NULL, FALSE);

  GdkColor gdk_color;
  gdk_color_parse(color, &gdk_color);
  gtk_widget_modify_bg(GTK_WIDGET(session->gtk.statusbar), GTK_STATE_NORMAL, &gdk_color);

  return TRUE;
}

gboolean
girara_set_view(girara_session_t* session, GtkWidget* widget)
{
  g_return_val_if_fail(session != NULL, FALSE);

  return TRUE;
}

/* default shortcut implementation */
void
girara_sc_focus_inputbar(girara_session_t* session, girara_argument_t* argument)
{
  g_return_if_fail(session != NULL);

  if(!(GTK_WIDGET_VISIBLE(GTK_WIDGET(session->gtk.inputbar))))
    gtk_widget_show(GTK_WIDGET(session->gtk.inputbar));

  if(argument->data)
  {
    gtk_entry_set_text(session->gtk.inputbar, (char*) argument->data);
    gtk_widget_grab_focus(GTK_WIDGET(session->gtk.inputbar));
    gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar), -1);
  }
}

void
girara_sc_quit(girara_session_t* session, girara_argument_t* argument)
{
  gtk_main_quit();
}

/* default commands implementation */
gboolean
girara_cmd_map(girara_session_t* session, int argc, char** argv)
{
  return TRUE;
}

gboolean
girara_cmd_quit(girara_session_t* session, int argc, char** argv)
{
  gtk_main_quit();
  return TRUE;
}

gboolean
girara_cmd_set(girara_session_t* session, int argc, char** argv)
{
  return TRUE;
}

/* callback implementation */
gboolean
girara_callback_view_key_press_event(GtkWidget* widget, GdkEventKey* event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);

  /* check for existing shortcut */
  girara_shortcut_t* shortcut = session->bindings.shortcuts;
  while(!session->buffer.command && shortcut)
  {
    if(
       event->keyval == shortcut->key
       && (CLEAN(event->state) == shortcut->mask || (shortcut->key >= 0x21
       && shortcut->key <= 0x7E && CLEAN(event->state) == GDK_SHIFT_MASK))
       && (session->global.current_mode & shortcut->mode || shortcut->mode == 0)
       && shortcut->function
      )
    {
      int t = (session->buffer.n > 0) ? session->buffer.n : 1;
      for(int i = 0; i < t; i++)
        shortcut->function(session, &(shortcut->argument));
      return TRUE;
    }

    shortcut = shortcut->next;
  }

  /* update buffer */
  if(event->keyval >= 0x21 && event->keyval <= 0x7E)
  {
    /* overall buffer */
    if(!session->global.buffer)
      session->global.buffer = g_string_new("");

    session->global.buffer = g_string_append_c(session->global.buffer, event->keyval);

    if(!session->buffer.command && event->keyval >= 0x30 && event->keyval <= 0x39)
    {
      if(((session->buffer.n * 10) + (event->keyval - '0')) < INT_MAX)
        session->buffer.n = (session->buffer.n * 10) + (event->keyval - '0');
    }
    else
    {
      if(!session->buffer.command)
        session->buffer.command = g_string_new("");

      session->buffer.command = g_string_append_c(session->buffer.command, event->keyval);
    }
  }

  /* check for buffer command */
  if(session->buffer.command)
  {
    gboolean matching_command = FALSE;

    shortcut = session->bindings.shortcuts;
    while(shortcut)
    {
      if(shortcut->buffered_command)
      {
        /* buffer could match a command */
        if(!strncmp(session->buffer.command->str, shortcut->buffered_command, session->buffer.command->len))
        {
          /* command matches buffer exactly */
          if(!strcmp(session->buffer.command->str, shortcut->buffered_command))
          {
            g_string_free(session->buffer.command, TRUE);
            g_string_free(session->global.buffer,  TRUE);
            session->buffer.command = NULL;
            session->global.buffer  = NULL;

            shortcut->function(session, &(shortcut->argument));
            return TRUE;
          }

          matching_command = TRUE;
        }
      }

      shortcut = shortcut->next;
    }

    /* free buffer if buffer will never match a command */
    if(!matching_command)
    {
      g_string_free(session->buffer.command, TRUE);
      g_string_free(session->global.buffer,  TRUE);
      session->buffer.command = NULL;
      session->global.buffer  = NULL;
    }
  }

  return FALSE;
}

gboolean
girara_callback_inputbar_activate(GtkEntry* entry, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);

  gchar *input  = gtk_editable_get_chars(GTK_EDITABLE(entry), 1, -1);
  if(!input)
    return FALSE;

  gchar **tokens = g_strsplit(input, " ", -1);
  if(!tokens)
  {
    g_free(input);
    return FALSE;
  }

  gchar *cmd = tokens[0];
  int length = g_strv_length(tokens);

  if(length < 1)
  {
    g_free(input);
    g_strfreev(tokens);
    return FALSE;
  }

  /* special commands */
  char *identifier_s = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, 1);
  char identifier    = identifier_s[0];
  g_free(identifier_s);

  girara_special_command_t* special_command = session->bindings.special_commands;
  while(special_command)
  {
    if(special_command->identifier == identifier)
    {
      if(special_command->always == 1)
      {
        g_free(input);
        g_strfreev(tokens);
        return FALSE;
      }

      special_command->function(session, input, &(special_command->argument));

      g_free(input);
      g_strfreev(tokens);
      return TRUE;
    }

    special_command = special_command->next;
  }

  /* search commands */
  girara_command_t* command = session->bindings.commands;
  while(command)
  {
    if((g_strcmp0(cmd, command->command) == 0) ||
       (g_strcmp0(cmd, command->abbr)    == 0))
    {
      command->function(session, length - 1, tokens + 1);
      g_free(input);
      g_strfreev(tokens);
      return TRUE;
    }

    command = command->next;
  }

  return FALSE;
}

gboolean
girara_callback_inputbar_key_press_event(GtkWidget* entry, GdkEventKey* event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);

  girara_inputbar_shortcut_t* inputbar_shortcut = session->bindings.inputbar_shortcuts;
  while(inputbar_shortcut)
  {
    if(inputbar_shortcut->key == event->keyval
     && inputbar_shortcut->mask == CLEAN(event->state))
    {
      inputbar_shortcut->function(session, &(inputbar_shortcut->argument));
      return TRUE;
    }

    inputbar_shortcut = inputbar_shortcut->next;
  }

  /* special commands */
  char *identifier_s = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, 1);
  char identifier    = identifier_s[0];
  g_free(identifier_s);

  girara_special_command_t* special_command = session->bindings.special_commands;
  while(special_command)
  {
    if((special_command->identifier == identifier) &&
       (special_command->always == TRUE))
    {
      gchar *input  = gtk_editable_get_chars(GTK_EDITABLE(entry), 1, -1);
      special_command->function(session, input, &(special_command->argument));
      g_free(input);
      return TRUE;
    }

    special_command = special_command->next;
  }

  return FALSE;
}

girara_completion_t*
girara_completion_init()
{
  girara_completion_t *completion = g_slice_new(girara_completion_t);
  completion->groups = NULL;

  return completion;
}

girara_completion_group_t*
girara_completion_group_create(char* name)
{
  girara_completion_group_t* group = g_slice_new(girara_completion_group_t);

  group->value    = name ? g_strdup(name) : NULL;
  group->elements = NULL;
  group->next     = NULL;

  return group;
}

void
girara_completion_add_group(girara_completion_t* completion, girara_completion_group_t* group)
{
  g_return_if_fail(completion != NULL);
  g_return_if_fail(group      != NULL);

  girara_completion_group_t* cg = completion->groups;

  while(cg && cg->next)
    cg = cg->next;

  if(cg)
    cg->next = group;
  else
    completion->groups = group;
}

void
girara_completion_free(girara_completion_t* completion)
{
  g_return_if_fail(completion != NULL);

  girara_completion_group_t* group = completion->groups;
  girara_completion_element_t *element;

  while(group)
  {
    element = group->elements;
    while(element)
    {
      girara_completion_element_t* ne = element->next;
      g_free(element->value);
      if(element->description)
        g_free(element->description);
      g_slice_free(girara_completion_element_t,  element);
      element = ne;
    }

    girara_completion_group_t *ng = group->next;
    if(group->value) g_free(group->value);
    g_slice_free(girara_completion_group_t, group);
    group = ng;
  }

  g_slice_free(girara_completion_t, completion);
}

void
completion_group_add_element(girara_completion_group_t* group, char* name, char* description)
{
  g_return_if_fail(group != NULL);
  g_return_if_fail(name != NULL);

  girara_completion_element_t* el = group->elements;

  while(el && el->next)
    el = el->next;

  girara_completion_element_t* new_element = g_slice_new(girara_completion_element_t);

  new_element->value       = g_strdup(name);
  new_element->description = description ?  g_strdup(description) : NULL;
  new_element->next        = NULL;

  if(el)
    el->next = new_element;
  else
    group->elements = new_element;
}

void
girara_isc_completion(girara_session_t* session, girara_argument_t* argument)
{
  g_return_if_fail(session != NULL);

  gchar *input      = gtk_editable_get_chars(GTK_EDITABLE(session->gtk.inputbar), 1, -1);
  gchar *tmp_string = gtk_editable_get_chars(GTK_EDITABLE(session->gtk.inputbar), 0,  1);
  gchar  identifier = tmp_string[0];
  int    length     = strlen(input);

  if(!input || !tmp_string)
  {
    if(input)
      g_free(input);
    if(tmp_string)
      g_free(tmp_string);
    return;
  }

  /* get current information*/
  char* first_space = strstr(input, " ");
  char* current_command;
  char* current_parameter;
  int   current_command_length;
  int   current_parameter_length;

  if(!first_space)
  {
    current_command          = g_strdup(input);
    current_command_length   = length;
    current_parameter        = NULL;
    current_parameter_length = 0;
  }
  else
  {
    int offset               = abs(input - first_space);
    current_command          = g_strndup(input, offset);
    current_command_length   = strlen(current_command);
    current_parameter        = input + offset + 1;
    current_parameter_length = strlen(current_parameter);
  }

  /* if the identifier does not match the command sign and
   * the completion should not be hidden, leave this function */
  if((identifier != ':') && (argument->n != GIRARA_HIDE))
  {
    if(current_command)
      g_free(current_command);
    if(input)
      g_free(input);
    if(tmp_string)
      g_free(tmp_string);
    return;
  }

  /* static elements */
  static GtkBox        *results         = NULL;
  static girara_completion_row_t *rows  = NULL;

  static int current_item = 0;
  static int n_items      = 0;

  static char *previous_command   = NULL;
  static char *previous_parameter = NULL;
  static int   previous_id        = 0;
  girara_command_t* previous_i    = NULL;
  static int   previous_length    = 0;

  static gboolean command_mode = TRUE;

  /* delete old list iff
   *   the completion should be hidden
   *   the current command differs from the previous one
   *   the current parameter differs from the previous one
   */
  if( (argument->n == GIRARA_HIDE) ||
      (current_parameter && previous_parameter && strcmp(current_parameter, previous_parameter)) ||
      (current_command && previous_command && strcmp(current_command, previous_command)) ||
      (previous_length != length)
    )
  {
    if(results)
      gtk_widget_destroy(GTK_WIDGET(results));

    results = NULL;

    if(rows)
    {
      for(int i = 0; i != n_items; ++i)
      {
        g_free(rows[i].command);
        g_free(rows[i].description);
      }
      free(rows);
    }

    rows         = NULL;
    current_item = 0;
    n_items      = 0;
    command_mode = TRUE;

    if(argument->n == GIRARA_HIDE)
    {
      if(current_command)
        g_free(current_command);
      if(input)
        g_free(input);
      if(tmp_string)
        g_free(tmp_string);
      return;
     }
  }

  /* create new list iff
   *  there is no current list
   *  the current command differs from the previous one
   *  the current parameter differs from the previous one
   */
  if( (!results) )
  {
    results = GTK_BOX(gtk_vbox_new(FALSE, 0));

    /* create list based on parameters iff
     *  there is a current parameter given
     *  there is an old list with commands
     *  the current command does not differ from the previous one
     *  the current command has an completion function
     */
    if(strchr(input, ' '))
    {
      gboolean search_matching_command = FALSE;

      int i;
      girara_command_t* command = session->bindings.commands;

      while(command)
      {
        int abbr_length = command->abbr    ? strlen(command->abbr)    : 0;
        int cmd_length  = command->command ? strlen(command->command) : 0;

        if( ((current_command_length <= cmd_length)  && !strncmp(current_command, command->command, current_command_length)) ||
            ((current_command_length <= abbr_length) && !strncmp(current_command, command->abbr,    current_command_length))
          )
        {
          if(command->completion)
          {
            previous_command = current_command;
            previous_id = i;
            previous_i = command;
            search_matching_command = TRUE;
          }
          else
          {
            if(current_command)
              g_free(current_command);
            if(input)
              g_free(input);
            if(tmp_string)
              g_free(tmp_string);
            return;
          }
        }

        i++;
        command = command->next;
      }

      if(!search_matching_command)
      {
        if(current_command)
          g_free(current_command);
        if(input)
          g_free(input);
        if(tmp_string)
          g_free(tmp_string);
        return;
      }

      girara_completion_t *result = previous_i->completion(session, current_parameter);

      if(!result || !result->groups)
      {
        if(current_command)
          g_free(current_command);
        if(input)
          g_free(input);
        if(tmp_string)
          g_free(tmp_string);
        return;
      }

      command_mode                         = FALSE;
      girara_completion_group_t* group     = NULL;
      girara_completion_element_t* element = NULL;

      rows = malloc(sizeof(girara_completion_row_t));
      // TODO: free 

      for(group = result->groups; group != NULL; group = group->next)
      {
        int group_elements = 0;

        for(element = group->elements; element != NULL; element = element->next)
        {
          if(element->value)
          {
            if(group->value && !group_elements)
            {
              rows = realloc(rows, (n_items + 1) * sizeof(girara_completion_row_t));
              rows[n_items].command     = g_strdup(group->value);
              rows[n_items].description = NULL;
              rows[n_items].command_id  = -1;
              rows[n_items].is_group    = TRUE;
              rows[n_items++].row       = GTK_WIDGET(girara_completion_row_create(session, results, group->value, NULL, TRUE));
            }

            rows = realloc(rows, (n_items + 1) * sizeof(girara_completion_row_t));
            rows[n_items].command     = g_strdup(element->value);
            rows[n_items].description = element->description ? g_strdup(element->description) : NULL;
            rows[n_items].command_id  = previous_id;
            rows[n_items].is_group    = FALSE;
            rows[n_items++].row       = GTK_WIDGET(girara_completion_row_create(session, results, element->value, element->description, FALSE));
            group_elements++;
          }
        }
      }

      /* clean up */
      girara_completion_free(result);
    }
    /* create list based on commands */
    else
    {
      int i = 0;
      command_mode = TRUE;

      rows = malloc(session->global.number_of_commands * sizeof(girara_completion_row_t));

      girara_command_t* command = session->bindings.commands;
      while(command)
      {
        int abbr_length = command->abbr    ? strlen(command->abbr) : 0;
        int cmd_length  = command->command ? strlen(command->command) : 0;

        /* add command to list iff
         *  the current command would match the command
         *  the current command would match the abbreviation
         */
        if( ((current_command_length <= cmd_length)  && !strncmp(current_command, command[i].command, current_command_length)) ||
            ((current_command_length <= abbr_length) && !strncmp(current_command, command[i].abbr,    current_command_length))
          )
        {
          rows[n_items].command     = g_strdup(command[i].command);
          rows[n_items].description = g_strdup(command[i].description);
          rows[n_items].command_id  = i;
          rows[n_items].is_group    = FALSE;
          rows[n_items++].row       = GTK_WIDGET(girara_completion_row_create(session, results, command[i].command, command[i].description, FALSE));
        }

        command = command->next;
      }

      rows = realloc(rows, n_items * sizeof(girara_completion_row_t));
    }

    gtk_box_pack_start(session->gtk.box, GTK_WIDGET(results), FALSE, FALSE, 0);
    gtk_widget_show_all(GTK_WIDGET(session->gtk.window));

    current_item = (argument->n == GIRARA_NEXT) ? -1 : 0;
  }

  /* update coloring iff
   *  there is a list with items
   */
  if( (results) && (n_items > 0) )
  {
    girara_completion_row_set_color(session, results, GIRARA_NORMAL, current_item);
    char* temp;
    int i = 0, next_group = 0;

    for(i = 0; i < n_items; i++)
    {
      if(argument->n == GIRARA_NEXT || argument->n == GIRARA_NEXT_GROUP)
        current_item = (current_item + n_items + 1) % n_items;
      else if(argument->n == GIRARA_PREVIOUS || argument->n == GIRARA_PREVIOUS_GROUP)
        current_item = (current_item + n_items - 1) % n_items;

      if(rows[current_item].is_group)
      {
        if(!command_mode && (argument->n == GIRARA_NEXT_GROUP || argument->n == GIRARA_PREVIOUS_GROUP))
          next_group = 1;
        continue;
      }
      else
      {
        if(!command_mode && (next_group == 0) && (argument->n == GIRARA_NEXT_GROUP || argument->n == GIRARA_PREVIOUS_GROUP))
          continue;
        break;
      }
    }

    girara_completion_row_set_color(session, results, GIRARA_HIGHLIGHT, current_item);

    /* hide other items */
    int uh = ceil(session->settings.n_completion_items / 2);
    int lh = floor(session->settings.n_completion_items / 2);

    for(i = 0; i < n_items; i++)
    {
     if((n_items > 1) && (
        (i >= (current_item - lh) && (i <= current_item + uh)) ||
        (i < session->settings.n_completion_items && current_item < lh) ||
        (i >= (n_items - session->settings.n_completion_items) && (current_item >= (n_items - uh))))
       )
        gtk_widget_show(rows[i].row);
      else
        gtk_widget_hide(rows[i].row);
    }

    if(command_mode)
      temp = g_strconcat(":", rows[current_item].command, (n_items == 1) ? " " : NULL, NULL);
    else
      temp = g_strconcat(":", previous_command, " ", rows[current_item].command, NULL);

    gtk_entry_set_text(session->gtk.inputbar, temp);
    gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar), -1);
    g_free(temp);

    previous_command   = g_strdup((command_mode) ? rows[current_item].command : current_command);
    previous_parameter = g_strdup((command_mode) ? current_parameter : rows[current_item].command);
    previous_length    = strlen(previous_command) + ((command_mode) ? (length - current_command_length) : (strlen(previous_parameter) + 1));
    previous_id        = rows[current_item].command_id;
  }

  if(current_command)
    g_free(current_command);
  if(input)
    g_free(input);
  if(tmp_string)
    g_free(tmp_string);
}

GtkEventBox*
girara_completion_row_create(girara_session_t* session, GtkBox* results, char* command, char* description, gboolean group)
{
  GtkBox      *col = GTK_BOX(gtk_hbox_new(FALSE, 0));
  GtkEventBox *row = GTK_EVENT_BOX(gtk_event_box_new());

  GtkLabel *show_command     = GTK_LABEL(gtk_label_new(NULL));
  GtkLabel *show_description = GTK_LABEL(gtk_label_new(NULL));

  gtk_misc_set_alignment(GTK_MISC(show_command),     0.0, 0.0);
  gtk_misc_set_alignment(GTK_MISC(show_description), 0.0, 0.0);

  if(group)
  {
    gtk_misc_set_padding(GTK_MISC(show_command),     2.0, 4.0);
    gtk_misc_set_padding(GTK_MISC(show_description), 2.0, 4.0);
  }
  else
  {
    gtk_misc_set_padding(GTK_MISC(show_command),     1.0, 1.0);
    gtk_misc_set_padding(GTK_MISC(show_description), 1.0, 1.0);
  }

  gtk_label_set_use_markup(show_command,     TRUE);
  gtk_label_set_use_markup(show_description, TRUE);

  gchar* c = g_markup_printf_escaped(FORMAT_COMMAND,     command ? command : "");
  gchar* d = g_markup_printf_escaped(FORMAT_DESCRIPTION, description ? description : "");
  gtk_label_set_markup(show_command,     c);
  gtk_label_set_markup(show_description, d);
  g_free(c);
  g_free(d);

  if(group)
  {
    gtk_widget_modify_fg(GTK_WIDGET(show_command),     GTK_STATE_NORMAL, &(session->style.completion_group_foreground));
    gtk_widget_modify_fg(GTK_WIDGET(show_description), GTK_STATE_NORMAL, &(session->style.completion_group_foreground));
    gtk_widget_modify_bg(GTK_WIDGET(row),              GTK_STATE_NORMAL, &(session->style.completion_group_background));
  }
  else
  {
    gtk_widget_modify_fg(GTK_WIDGET(show_command),     GTK_STATE_NORMAL, &(session->style.completion_foreground));
    gtk_widget_modify_fg(GTK_WIDGET(show_description), GTK_STATE_NORMAL, &(session->style.completion_foreground));
    gtk_widget_modify_bg(GTK_WIDGET(row),              GTK_STATE_NORMAL, &(session->style.completion_background));
  }

  gtk_widget_modify_font(GTK_WIDGET(show_command),     session->style.font);
  gtk_widget_modify_font(GTK_WIDGET(show_description), session->style.font);

  gtk_box_pack_start(GTK_BOX(col), GTK_WIDGET(show_command),     TRUE,  TRUE,  2);
  gtk_box_pack_start(GTK_BOX(col), GTK_WIDGET(show_description), FALSE, FALSE, 2);

  gtk_container_add(GTK_CONTAINER(row), GTK_WIDGET(col));

  gtk_box_pack_start(results, GTK_WIDGET(row), FALSE, FALSE, 0);

  return row;
}

void
girara_completion_row_set_color(girara_session_t* session, GtkBox* results, int mode, int id)
{
  GtkEventBox *row   = (GtkEventBox*) g_list_nth_data(gtk_container_get_children(GTK_CONTAINER(results)), id);

  if(row)
  {
    GtkBox      *col   = (GtkBox*)      g_list_nth_data(gtk_container_get_children(GTK_CONTAINER(row)), 0);
    GtkLabel    *cmd   = (GtkLabel*)    g_list_nth_data(gtk_container_get_children(GTK_CONTAINER(col)), 0);
    GtkLabel    *cdesc = (GtkLabel*)    g_list_nth_data(gtk_container_get_children(GTK_CONTAINER(col)), 1);

    if(mode == GIRARA_NORMAL)
    {
      gtk_widget_modify_fg(GTK_WIDGET(cmd),   GTK_STATE_NORMAL, &(session->style.completion_foreground));
      gtk_widget_modify_fg(GTK_WIDGET(cdesc), GTK_STATE_NORMAL, &(session->style.completion_foreground));
      gtk_widget_modify_bg(GTK_WIDGET(row),   GTK_STATE_NORMAL, &(session->style.completion_background));
    }
    else
    {
      gtk_widget_modify_fg(GTK_WIDGET(cmd),   GTK_STATE_NORMAL, &(session->style.completion_highlight_foreground));
      gtk_widget_modify_fg(GTK_WIDGET(cdesc), GTK_STATE_NORMAL, &(session->style.completion_highlight_foreground));
      gtk_widget_modify_bg(GTK_WIDGET(row),   GTK_STATE_NORMAL, &(session->style.completion_highlight_background));
    }
  }
}
