/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#include "girara.h"

#define CLEAN(m) (m & ~(GDK_MOD2_MASK) & ~(GDK_BUTTON1_MASK) & ~(GDK_BUTTON2_MASK) & ~(GDK_BUTTON3_MASK) & ~(GDK_BUTTON4_MASK) & ~(GDK_BUTTON5_MASK) & ~(GDK_LEAVE_NOTIFY_MASK))
#define FORMAT_COMMAND "<b>%s</b>"
#define FORMAT_DESCRIPTION "<i>%s</i>"

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

/* header functions implementation */
GtkEventBox* girara_completion_row_create(girara_session_t*, char*, char*, bool);
void girara_completion_row_set_color(girara_session_t*, GtkEventBox*, int);

/* completion */
struct girara_internal_completion_entry_s
{
  bool group;
  char* value;
  GtkEventBox* widget;
};

typedef struct girara_internal_completion_entry_s girara_internal_completion_entry_t;

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

  session->settings = NULL;

  session->signals.view_key_pressed     = 0;
  session->signals.inputbar_key_pressed = 0;
  session->signals.inputbar_activate    = 0;

  session->buffer.n       = 0;
  session->buffer.command = NULL;

  session->global.current_mode       = 0;
  session->global.buffer             = NULL;

  /* default values */
  int window_width       = 800;
  int window_height      = 600;
  int n_completion_items = 15;

  /* add default settings */
  girara_setting_add(session, "font",                     "monospace normal 9", STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "default-fg",               "#DDDDDD",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "default-bg",               "#000000",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "inputbar-fg",              "#9FBC00",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "inputbar-bg",              "#131313",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "statusbar-fg",             "#FFFFFF",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "statusbar-bg",             "#000000",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "completion-fg",            "#DDDDDD",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "completion-bg",            "#232323",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "completion-group-fg",      "#DEDEDE",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "completion-group-bg",      "#000000",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "completion-highlight-fg",  "#232323",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "completion-highlight-bg",  "#9FBC00",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "notification-error-fg",    "#FF1212",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "notification-error-bg",    "#FFFFFF",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "notification-warning-fg",  "#FFF712",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "notification-warning-bg",  "#FFFFFF",            STRING, TRUE,  NULL, NULL);
  girara_setting_add(session, "window-width",             &window_width,        INT,    TRUE,  NULL, NULL);
  girara_setting_add(session, "window-height",            &window_height,       INT,    TRUE,  NULL, NULL);
  girara_setting_add(session, "n-completion-items",       &n_completion_items,  INT,    FALSE, NULL, NULL);

  /* default shortcuts */
  girara_shortcut_add(session, GDK_CONTROL_MASK, GDK_q,     NULL, girara_sc_quit,           0, 0, NULL);
  girara_shortcut_add(session, 0,                GDK_colon, NULL, girara_sc_focus_inputbar, 0, 0, ":");

  /* default inputbar shortcuts */
  girara_inputbar_shortcut_add(session, 0,                GDK_Escape,       girara_isc_abort,               0,                           NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_c,            girara_isc_abort,               0,                           NULL);
  girara_inputbar_shortcut_add(session, 0,                GDK_Tab,          girara_isc_completion,          GIRARA_NEXT,                 NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_Tab,          girara_isc_completion,          GIRARA_NEXT_GROUP,           NULL);
  girara_inputbar_shortcut_add(session, GDK_SHIFT_MASK,   GDK_ISO_Left_Tab, girara_isc_completion,          GIRARA_PREVIOUS,             NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_ISO_Left_Tab, girara_isc_completion,          GIRARA_PREVIOUS_GROUP,       NULL);
  girara_inputbar_shortcut_add(session, 0,                GDK_BackSpace,    girara_isc_string_manipulation, GIRARA_DELETE_LAST_CHAR,     NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_h,            girara_isc_string_manipulation, GIRARA_DELETE_LAST_CHAR,     NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_u,            girara_isc_string_manipulation, GIRARA_DELETE_TO_LINE_START, NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_w,            girara_isc_string_manipulation, GIRARA_DELETE_LAST_WORD,     NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_f,            girara_isc_string_manipulation, GIRARA_NEXT_CHAR,            NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_b,            girara_isc_string_manipulation, GIRARA_PREVIOUS_CHAR,        NULL);

  /* default commands */
  girara_inputbar_command_add(session, "map",  "m", girara_cmd_map,  NULL,          "Map a key sequence");
  girara_inputbar_command_add(session, "quit", "q", girara_cmd_quit, NULL,          "Quit the program");
  girara_inputbar_command_add(session, "set",  "s", girara_cmd_set,  girara_cc_set, "Set an option");

  return session;
}

bool
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
  GdkGeometry hints =
  {
    .base_height = 1,
    .base_width  = 1,
    .height_inc  = 0,
    .max_aspect  = 0,
    .max_height  = 0,
    .max_width   = 0,
    .min_aspect  = 0,
    .min_height  = 0,
    .min_width   = 0,
    .width_inc   = 0
  };

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
  char* tmp_value = NULL;

  tmp_value = girara_setting_get(session, "default-fg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.default_foreground));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "default-bg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.default_background));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "inputbar-fg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.inputbar_foreground));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "inputbar-bg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.inputbar_background));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "statusbar-bg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.statusbar_background));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "statusbar-fg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.statusbar_foreground));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "completion-fg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.completion_foreground));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "completion-bg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.completion_background));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "completion-group-fg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.completion_group_foreground));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "completion-group-bg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.completion_group_background));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "completion-highlight-fg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.completion_highlight_foreground));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "completion-highlight-bg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.completion_highlight_background));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "notification-error-fg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.notification_error_foreground));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "notification-error-bg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.notification_error_background));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "notification-warning-fg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.notification_warning_foreground));
    free(tmp_value);
    tmp_value = NULL;
  }

  tmp_value = girara_setting_get(session, "notification-warning-bg");
  if(tmp_value)
  {
    gdk_color_parse(tmp_value, &(session->style.notification_warning_background));
    free(tmp_value);
    tmp_value = NULL;
  }

  /* parse font */
  tmp_value = girara_setting_get(session, "font");
  if(tmp_value)
  {
    session->style.font = pango_font_description_from_string(tmp_value);
    free(tmp_value);
    tmp_value = NULL;
  }

  /* statusbar */
  gtk_widget_modify_bg(GTK_WIDGET(session->gtk.statusbar), GTK_STATE_NORMAL, &(session->style.statusbar_background));

  /* inputbar */
  gtk_widget_modify_base(GTK_WIDGET(session->gtk.inputbar), GTK_STATE_NORMAL, &(session->style.inputbar_background));
  gtk_widget_modify_text(GTK_WIDGET(session->gtk.inputbar), GTK_STATE_NORMAL, &(session->style.inputbar_foreground));
  gtk_widget_modify_font(GTK_WIDGET(session->gtk.inputbar),                     session->style.font);

  /* set window size */
  int* window_width  = girara_setting_get(session, "window-width");
  int* window_height = girara_setting_get(session, "window-height");

  if(window_width && window_height)
    gtk_window_set_default_size(GTK_WINDOW(session->gtk.window), *window_width, *window_height);

  if(window_width)
    free(window_width);
  if(window_height)
    free(window_height);

  gtk_widget_show_all(GTK_WIDGET(session->gtk.window));

  return TRUE;
}

bool
girara_session_destroy(girara_session_t* session)
{;
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
    if(command->command)
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
  girara_setting_t* setting = session->settings;
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

  /* clean up session */
  g_slice_free(girara_session_t, session);

  return TRUE;
}

bool
girara_setting_add(girara_session_t* session, char* name, void* value, girara_setting_type_t type, bool init_only, char* description, girara_setting_callback_t callback)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);

  /* search for existing setting */
  girara_setting_t* settings_it = session->settings;
  if(settings_it)
  {
    if(!g_strcmp0(name, settings_it->name))
        return FALSE;

    while(settings_it->next)
    {
      if(!g_strcmp0(name, settings_it->next->name))
        return FALSE;

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
  setting->next        = NULL;

  switch (type)
  {
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
      setting->value.s = g_strdup(value);
      break;
  }

  if(settings_it)
    settings_it->next = setting;
  else
    session->settings = setting;

  return TRUE;
}

bool
girara_setting_set(girara_session_t* session, char* name, void* value)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);

  for (girara_setting_t* setting = session->settings; setting != NULL; setting = setting->next)
  {
    if (g_strcmp0(setting->name, name) != 0)
      continue;

    switch (setting->type)
    {
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

void*
girara_setting_get(girara_session_t* session, char* name)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);

  for(girara_setting_t* setting = session->settings; setting != NULL; setting = setting->next)
  {
    if(g_strcmp0(setting->name, name) != 0)
      continue;

    bool *bvalue = NULL;
    float    *fvalue = NULL;
    int      *ivalue = NULL;

    switch(setting->type)
    {
      case BOOLEAN:
        bvalue = malloc(sizeof(bool));

        if(!bvalue)
          return NULL;

        *bvalue = setting->value.b;
        return bvalue;
      case FLOAT:
        fvalue = malloc(sizeof(float));

        if(!fvalue)
          return NULL;

        *fvalue = setting->value.f;
        return fvalue;
      case INT:
        ivalue = malloc(sizeof(int));

        if(!ivalue)
          return NULL;

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

bool
girara_shortcut_add(girara_session_t* session, guint modifier, guint key, char* buffer, girara_shortcut_function_t function, girara_mode_t mode, int argument_n, void* argument_data)
{
  g_return_val_if_fail(session != NULL, FALSE);
  g_return_val_if_fail(buffer || key || modifier, FALSE);

  girara_argument_t argument = {argument_n, argument_data};

  /* search for existing binding */
  girara_shortcut_t* shortcuts_it = session->bindings.shortcuts;
  girara_shortcut_t* last_shortcut = shortcuts_it;

  while(shortcuts_it)
  {
    if(((shortcuts_it->mask == modifier && shortcuts_it->key == key) ||
       (buffer && shortcuts_it->buffered_command && !strcmp(shortcuts_it->buffered_command, buffer)))
        && shortcuts_it->mode == mode)
    {
      shortcuts_it->function = function;
      shortcuts_it->argument = argument;
      return TRUE;
    }

    last_shortcut = shortcuts_it;
    shortcuts_it = shortcuts_it->next;
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

  if(last_shortcut)
    last_shortcut->next = shortcut;
  else
    session->bindings.shortcuts = shortcut;

  return TRUE;
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

  while(commands_it)
  {
    if(!g_strcmp0(commands_it->command, command))
    {
      if(commands_it->abbr)
        free(commands_it->abbr);
      if(commands_it->description)
        free(commands_it->description);

      commands_it->abbr        = abbreviation ? g_strdup(abbreviation) : NULL;
      commands_it->function    = function;
      commands_it->completion  = completion;
      commands_it->description = description ? g_strdup(description) : NULL;
      return TRUE;
    }

    last_command = commands_it;
    commands_it = commands_it->next;
  }

  /* add new inputbar command */
  girara_command_t* new_command = g_slice_new(girara_command_t);

  new_command->command     = g_strdup(command);
  new_command->abbr        = abbreviation ? g_strdup(abbreviation) : NULL;
  new_command->function    = function;
  new_command->completion  = completion;
  new_command->description = description ? g_strdup(description) : NULL;
  new_command->next        = NULL;

  if(last_command)
    last_command->next = new_command;
  else
    session->bindings.commands = new_command;

  return TRUE;
}

bool
girara_inputbar_shortcut_add(girara_session_t* session, guint modifier, guint key, girara_shortcut_function_t function, int argument_n, void* argument_data)
{
  g_return_val_if_fail(session  != NULL, FALSE);
  g_return_val_if_fail(function != NULL, FALSE);

  girara_argument_t argument = {argument_n, argument_data};

  /* search for existing special command */
  girara_inputbar_shortcut_t* inp_sh_it = session->bindings.inputbar_shortcuts;
  girara_inputbar_shortcut_t* last_inp_sh = inp_sh_it;

  while(inp_sh_it)
  {
    if(inp_sh_it->mask == modifier && inp_sh_it->key == key)
    {
      inp_sh_it->function = function;
      inp_sh_it->argument = argument;
      return TRUE;
    }

    last_inp_sh = inp_sh_it;
    inp_sh_it = inp_sh_it->next;
  }

  /* create new inputbar shortcut */
  girara_inputbar_shortcut_t* inputbar_shortcut = g_slice_new(girara_inputbar_shortcut_t);

  inputbar_shortcut->mask     = modifier;
  inputbar_shortcut->key      = key;
  inputbar_shortcut->function = function;
  inputbar_shortcut->argument = argument;
  inputbar_shortcut->next     = NULL;

  if(last_inp_sh)
    last_inp_sh->next = inputbar_shortcut;
  else
    session->bindings.inputbar_shortcuts = inputbar_shortcut;

  return TRUE;
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

  while(scommand_it)
  {
    if(scommand_it->identifier == identifier)
    {
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

  if(last_scommand)
    last_scommand->next = special_command;
  else
    session->bindings.special_commands = special_command;

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

  while(me_it)
  {
    if(me_it->mask == mask && me_it->button == button &&
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

  if(last_me)
    last_me->next = mouse_event;
  else
    session->bindings.mouse_events = mouse_event;

  return TRUE;
}

girara_statusbar_item_t*
girara_statusbar_item_add(girara_session_t* session, bool expand, bool fill, bool left, girara_statusbar_event_t callback)
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

  GdkColor gdk_color;
  gdk_color_parse(color, &gdk_color);
  gtk_widget_modify_bg(GTK_WIDGET(session->gtk.statusbar), GTK_STATE_NORMAL, &gdk_color);

  return TRUE;
}

bool
girara_set_view(girara_session_t* session, GtkWidget* UNUSED(widget))
{
  g_return_val_if_fail(session != NULL, FALSE);

  return TRUE;
}

/* default shortcut implementation */
bool
girara_sc_focus_inputbar(girara_session_t* session, girara_argument_t* argument)
{
  g_return_val_if_fail(session != NULL, false);

  if(!(GTK_WIDGET_VISIBLE(GTK_WIDGET(session->gtk.inputbar))))
    gtk_widget_show(GTK_WIDGET(session->gtk.inputbar));

  if(argument->data)
  {
    gtk_entry_set_text(session->gtk.inputbar, (char*) argument->data);
    gtk_widget_grab_focus(GTK_WIDGET(session->gtk.inputbar));
    gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar), -1);
  }

  return true;
}

bool
girara_sc_quit(girara_session_t* session, girara_argument_t* UNUSED(argument))
{
  girara_argument_t arg = { GIRARA_HIDE, NULL };
  girara_isc_completion(session, &arg);

  gtk_main_quit();

  return true;
}

/* default commands implementation */
bool
girara_cmd_map(girara_session_t* UNUSED(session), int UNUSED(argc), char** UNUSED(argv))
{
  return true;
}

bool
girara_cmd_quit(girara_session_t* session, int UNUSED(argc), char** UNUSED(argv))
{
  girara_argument_t arg = { GIRARA_HIDE, NULL };
  girara_isc_completion(session, &arg);

  gtk_main_quit();

  return true;
}

bool
girara_cmd_set(girara_session_t* UNUSED(session), int UNUSED(argc), char** UNUSED(argv))
{
  return true;
}

/* callback implementation */
bool
girara_callback_view_key_press_event(GtkWidget* UNUSED(widget), GdkEventKey* event, girara_session_t* session)
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
    bool matching_command = FALSE;

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

bool
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

      girara_isc_abort(session, NULL);

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

      girara_isc_abort(session, NULL);

      return TRUE;
    }

    command = command->next;
  }

  /* no known command */
  girara_isc_abort(session, NULL);

  return FALSE;
}

bool
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
girara_completion_group_create(girara_session_t* UNUSED(session), char* name)
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
      if(element->description) g_free(element->description);
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
girara_completion_group_add_element(girara_completion_group_t* group, char* name, char* description)
{
  g_return_if_fail(group   != NULL);
  g_return_if_fail(name    != NULL);

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

bool
girara_isc_abort(girara_session_t* session, girara_argument_t* UNUSED(argument))
{
  /* hide completion */
  girara_argument_t arg = { GIRARA_HIDE, NULL };
  girara_isc_completion(session, &arg);

  /* clear inputbar */
  gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar), 0, -1);

  /* grab view */
  gtk_widget_grab_focus(GTK_WIDGET(session->gtk.view));

  return true;
}

bool
girara_isc_completion(girara_session_t* session, girara_argument_t* argument)
{
  g_return_val_if_fail(session != NULL, false);

  /* get current text */
  gchar *input     = gtk_editable_get_chars(GTK_EDITABLE(session->gtk.inputbar), 0, -1);
  int input_length = strlen(input);

  if(input_length == 0 || input[0] != ':')
  {
    g_free(input);
    return false;
  }

  gchar **elements = g_strsplit(input + 1, " ", 2);
  int n_parameter  =  g_strv_length(elements);
  g_free(input);

  /* get current values */
  gchar *current_command   = (elements[0] != NULL && elements[0][0] != '\0') ? g_strdup(elements[0]) : NULL;
  gchar *current_parameter = (elements[0] != NULL && elements[1] != NULL)    ? g_strdup(elements[1]) : NULL;

  unsigned int current_command_length = current_command ? strlen(current_command) : 0;

  /* create result box */
  static GtkBox* results          = NULL;
  static GList* entries           = NULL;
  static GList* entries_current   = NULL;
  static char *previous_command   = NULL;
  static char *previous_parameter = NULL;
  static bool command_mode        = true;
  static int   previous_length    = 0;

  /* delete old list iff
   *   the completion should be hidden
   *   the current command differs from the previous one
   *   the current parameter differs from the previous one
   *   no current command is given
   */
  if( (argument->n == GIRARA_HIDE) ||
      (current_parameter && previous_parameter && strcmp(current_parameter, previous_parameter)) ||
      (current_command && previous_command && strcmp(current_command, previous_command)) ||
      input_length != previous_length
    )
  {
    if(results)
    {
      for(GList* element = entries; element; element = g_list_next(element))
      {
        girara_internal_completion_entry_t* entry = (girara_internal_completion_entry_t*) element->data;
        if(entry)
        {
          gtk_widget_destroy(GTK_WIDGET(entry->widget));
          g_free(entry->value);
          g_slice_free(girara_internal_completion_entry_t, entry);
        }
      }

      /* delete elements */
      g_list_free(entries);
      entries         = NULL;
      entries_current = NULL;

      /* delete row box */
      gtk_widget_destroy(GTK_WIDGET(results));
      results = NULL;
    }

    command_mode = TRUE;

    if(argument->n == GIRARA_HIDE)
    {
      g_free(previous_command);
      previous_command = NULL;

      g_free(previous_parameter);
      previous_parameter = NULL;

      g_strfreev(elements);

      g_free(current_command);
      g_free(current_parameter);

      return false;
    }
  }

  /* create new list iff
   *  there is no current list
   */
  if(!results)
  {
    results = GTK_BOX(gtk_vbox_new(FALSE, 0));
    if(!results)
    {
      g_free(current_command);
      g_free(current_parameter);

      g_strfreev(elements);
      return false;
    }

    /* based on parameters */
    if(n_parameter > 1)
    {
      /* search matching command */
      girara_command_t* command = NULL;
      for(command = session->bindings.commands; command != NULL; command = command->next)
      {
        if( !strncmp(current_command, command->command, current_command_length) ||
            !strncmp(current_command, command->abbr,    current_command_length) )
        {
          if(command->completion)
          {
            g_free(previous_command);
            previous_command = g_strdup(command->command);
            break;
          }
          else
          {
            g_free(current_command);
            g_free(current_parameter);

            g_strfreev(elements);
            return false;
          }
        }
      }

      if(!command)
      {
        g_free(current_command);
        g_free(current_parameter);

        g_strfreev(elements);
        return false;
      }

      /* generate completion result */
      girara_completion_t *result = command->completion(session, current_parameter);

      if(!result || !result->groups)
      {
        g_free(current_command);
        g_free(current_parameter);

        g_strfreev(elements);
        return false;
      }

      girara_completion_group_t* group     = result->groups;
      girara_completion_element_t *element = NULL;

      while(group)
      {
        element = group->elements;

        /* create group entry */
        if(group->value)
        {
          girara_internal_completion_entry_t* entry = g_slice_new(girara_internal_completion_entry_t);
          entry->group  = TRUE;
          entry->value  = g_strdup(group->value);
          entry->widget = girara_completion_row_create(session, group->value, NULL, TRUE);

          entries = g_list_append(entries, entry);

          gtk_box_pack_start(results, GTK_WIDGET(entry->widget), FALSE, FALSE, 0);
        }

        while(element)
        {
          girara_internal_completion_entry_t* entry = g_slice_new(girara_internal_completion_entry_t);
          entry->group  = FALSE;
          entry->value  = g_strdup(element->value);
          entry->widget = girara_completion_row_create(session, element->value, NULL, FALSE);

          entries = g_list_append(entries, entry);

          gtk_box_pack_start(results, GTK_WIDGET(entry->widget), FALSE, FALSE, 0);

          element = element->next;
        }

        group = group->next;
      }

      girara_completion_free(result);

      command_mode = FALSE;
    }
    /* based on commands */
    else
    {
      command_mode = TRUE;

      /* create command rows */
      for (girara_command_t* command = session->bindings.commands;
        command != NULL; command = command->next)
      {
        if(!current_command ||
            !strncmp(current_command, command->command, current_command_length) ||
            !strncmp(current_command, command->abbr,    current_command_length)
          )
        {
          /* create entry */
          girara_internal_completion_entry_t* entry = g_slice_new(girara_internal_completion_entry_t);
          entry->group  = FALSE;
          entry->value  = g_strdup(command->command);
          entry->widget = girara_completion_row_create(session, command->command, NULL, FALSE);

          entries = g_list_append(entries, entry);

          /* show entry row */
          gtk_box_pack_start(results, GTK_WIDGET(entry->widget), FALSE, FALSE, 0);
        }
      }
    }

    if(entries)
    {
      entries_current = (argument->n == GIRARA_NEXT) ? g_list_last(entries) : entries;
      gtk_box_pack_start(session->gtk.box, GTK_WIDGET(results), FALSE, FALSE, 0);
      gtk_widget_show(GTK_WIDGET(results));
    }
  }

  /* update entries */
  if(entries && g_list_length(entries) > 1)
  {
    girara_completion_row_set_color(session, ((girara_internal_completion_entry_t *) entries_current->data)->widget, GIRARA_NORMAL);

    unsigned int n_elements = g_list_length(entries);
    bool next_group     = FALSE;

    for(unsigned int i = 0; i < n_elements; i++)
    {
      if(argument->n == GIRARA_NEXT || argument->n == GIRARA_NEXT_GROUP)
      {
        GList* entry = g_list_next(entries_current);
        if(!entry)
          entry = g_list_first(entries);

        entries_current = entry;
      }
      else if(argument->n == GIRARA_PREVIOUS || argument->n == GIRARA_PREVIOUS_GROUP)
      {
        GList* entry = g_list_previous(entries_current);
        if(!entry)
          entry = g_list_last(entries);

        entries_current = entry;
      }

      if(((girara_internal_completion_entry_t*) entries_current->data)->group)
      {
        if(!command_mode && (argument->n == GIRARA_NEXT_GROUP || argument->n == GIRARA_PREVIOUS_GROUP))
          next_group = TRUE;
        continue;
      }
      else
      {
        if(!command_mode && (next_group == 0) && (argument->n == GIRARA_NEXT_GROUP || argument->n == GIRARA_PREVIOUS_GROUP))
          continue;
        break;
      }
    }

    girara_completion_row_set_color(session, ((girara_internal_completion_entry_t *) entries_current->data)->widget, GIRARA_HIGHLIGHT);

    /* hide other items */
    int* tmp  = girara_setting_get(session, "n-completion-items");
    unsigned int n_completion_items = tmp ? *tmp : 15;
    unsigned int uh = ceil( n_completion_items / 2);
    unsigned int lh = floor(n_completion_items / 2);

    unsigned int current_item = g_list_position(entries, entries_current);

    for(unsigned int i = 0; i < n_elements; i++)
    {
     if(
        (i >= (current_item - lh) && (i <= current_item + uh)) ||
        (i < n_completion_items && current_item < lh) ||
        (i >= (n_elements - n_completion_items) && (current_item >= (n_elements - uh)))
       )
        gtk_widget_show(GTK_WIDGET(((girara_internal_completion_entry_t*) (g_list_nth(entries, i))->data)->widget));
      else
        gtk_widget_hide(GTK_WIDGET(((girara_internal_completion_entry_t*) (g_list_nth(entries, i))->data)->widget));
    }

    /* update text */
    char* temp;
    if(command_mode)
      temp = g_strconcat(":", ((girara_internal_completion_entry_t *) entries_current->data)->value, NULL);
    else
      temp = g_strconcat(":", previous_command, " ", ((girara_internal_completion_entry_t *) entries_current->data)->value, NULL);

    gtk_entry_set_text(session->gtk.inputbar, temp);
    gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar), -1);
    g_free(temp);

    /* update previous */
    g_free(previous_command);
    g_free(previous_parameter);
    previous_command   = g_strdup((command_mode) ? ((girara_internal_completion_entry_t*) entries_current->data)->value : current_command);
    previous_parameter = g_strdup((command_mode) ? current_parameter : ((girara_internal_completion_entry_t*) entries_current->data)->value);
    previous_length    = strlen(previous_command) + ((command_mode) ? (input_length - current_command_length) : (strlen(previous_parameter) + 2));
  }

  g_free(current_command);
  g_free(current_parameter);

  g_strfreev(elements);

  return false;
}

bool
girara_isc_string_manipulation(girara_session_t* session, girara_argument_t* argument)
{
  gchar *input  = gtk_editable_get_chars(GTK_EDITABLE(session->gtk.inputbar), 0, -1);
  int    length = strlen(input);
  int pos       = gtk_editable_get_position(GTK_EDITABLE(session->gtk.inputbar));
  int i;

  switch (argument->n)
  {
    case GIRARA_DELETE_LAST_WORD:
      i = pos - 1;

      if(!pos)
        return false;

      /* remove trailing spaces */
      for(; i >= 0 && input[i] == ' '; i--);

      /* find the beginning of the word */
      while((i == (pos - 1)) || ((i > 0) && (input[i] != ' ')
            && (input[i] != '/') && (input[i] != '.')
            && (input[i] != '-') && (input[i] != '=')
            && (input[i] != '&') && (input[i] != '#')
            && (input[i] != '?')
            ))
        i--;

      gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar),  i, pos);
      gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar), i);
      break;
    case GIRARA_DELETE_LAST_CHAR:
      if((length - 1) <= 0)
        girara_isc_abort(session, argument);
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

  g_free(input);

  return false;
}

girara_completion_t*
girara_cc_set(girara_session_t* session, char* input)
{
  girara_completion_t* completion  = girara_completion_init();
  girara_completion_group_t* group = girara_completion_group_create(session, NULL);
  girara_completion_add_group(completion, group);

  unsigned int input_length = input ? strlen(input) : 0;

  girara_setting_t* setting = session->settings;
  while(setting && setting->next)
  {
    if((input_length <= strlen(setting->name)) && !strncmp(input, setting->name, input_length))
      girara_completion_group_add_element(group, setting->name, setting->description);

    setting = setting->next;
  }

  return completion;
}

GtkEventBox*
girara_completion_row_create(girara_session_t* session, char* command, char* description, bool group)
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
  gtk_widget_show_all(GTK_WIDGET(row));

  return row;
}

void
girara_completion_row_set_color(girara_session_t* session, GtkEventBox* row, int mode)
{
  g_return_if_fail(session != NULL);
  g_return_if_fail(row     != NULL);

  GtkBox *col     = GTK_BOX(gtk_bin_get_child(GTK_BIN(row)));
  GList* items    = gtk_container_get_children(GTK_CONTAINER(col));
  GtkLabel *cmd   = GTK_LABEL(g_list_nth_data(items, 0));
  GtkLabel *desc  = GTK_LABEL(g_list_nth_data(items, 1));

  if(mode == GIRARA_HIGHLIGHT)
  {
    gtk_widget_modify_fg(GTK_WIDGET(cmd),   GTK_STATE_NORMAL, &(session->style.completion_highlight_foreground));
    gtk_widget_modify_fg(GTK_WIDGET(desc),  GTK_STATE_NORMAL, &(session->style.completion_highlight_foreground));
    gtk_widget_modify_bg(GTK_WIDGET(row),   GTK_STATE_NORMAL, &(session->style.completion_highlight_background));
  }
  else
  {
    gtk_widget_modify_fg(GTK_WIDGET(cmd),   GTK_STATE_NORMAL, &(session->style.completion_foreground));
    gtk_widget_modify_fg(GTK_WIDGET(desc),  GTK_STATE_NORMAL, &(session->style.completion_foreground));
    gtk_widget_modify_bg(GTK_WIDGET(row),   GTK_STATE_NORMAL, &(session->style.completion_background));
  }

  g_list_free(items);
}

void
girara_mode_set(girara_session_t* session, girara_mode_t mode)
{
  g_return_if_fail(session != NULL);

  session->global.current_mode = mode;
}

girara_mode_t girara_mode_get(girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, 0);

  return session->global.current_mode;
}
