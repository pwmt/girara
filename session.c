/* See LICENSE file for license and copyright information */

#include <stdlib.h>

#include "girara-session.h"
#include "girara-settings.h"
#include "girara-datastructures.h"
#include "girara-internal.h"
#include "girara-commands.h"

#include "girara.h"
  
girara_session_t*
girara_session_create()
{
  girara_session_t* session = g_slice_new(girara_session_t);

  /* init values */
  session->gtk.window                  = NULL;
  session->gtk.box                     = NULL;
  session->gtk.view                    = NULL;
  session->gtk.viewport                = NULL;
  session->gtk.statusbar               = NULL;
  session->gtk.statusbar_entries       = NULL;
  session->gtk.notification_area       = NULL;
  session->gtk.notification_text       = NULL;
  session->gtk.tabbar                  = NULL;
  session->gtk.inputbar                = NULL;
  session->gtk.tabs                    = NULL;
  session->gtk.results                 = NULL;

  session->gtk.embed                   = 0;

  session->style.font                  = NULL;

  session->bindings.mouse_events       = girara_list_new2(
      (girara_free_function_t) girara_mouse_event_free);
  session->bindings.commands           = girara_list_new2(
      (girara_free_function_t) girara_command_free);
  session->bindings.special_commands   = girara_list_new2(
      (girara_free_function_t) girara_special_command_free);
  session->bindings.shortcuts          = girara_list_new2(
      (girara_free_function_t) girara_shortcut_free);
  session->bindings.inputbar_shortcuts = girara_list_new2(
      (girara_free_function_t) girara_inputbar_shortcut_free);
  session->elements.statusbar_items    = girara_list_new2(
      (girara_free_function_t) girara_statusbar_item_free);
  session->settings                    = girara_list_new2(
      (girara_free_function_t) girara_setting_free);

  session->signals.view_key_pressed     = 0;
  session->signals.inputbar_key_pressed = 0;
  session->signals.inputbar_activate    = 0;

  session->events.buffer_changed        = NULL;

  session->buffer.n       = 0;
  session->buffer.command = NULL;

  session->global.buffer  = NULL;
  session->global.data    = NULL;

  session->modes.identifiers  = girara_list_new2(
      (girara_free_function_t) girara_mode_string_free);
  girara_mode_t normal_mode   = girara_mode_add(session, "normal");
  session->modes.normal       = normal_mode;
  session->modes.current_mode = normal_mode;

  session->config.handles           = girara_list_new2(
      (girara_free_function_t) girara_config_handle_free);
  session->config.shortcut_mappings = girara_list_new2(
      (girara_free_function_t) girara_shortcut_mapping_free);
  session->config.argument_mappings = NULL;

  /* default values */
  int window_width       = 800;
  int window_height      = 600;
  int n_completion_items = 15;
  bool show_scrollbars   = false;

  /* add default settings */
  girara_setting_add(session, "font",                     "monospace normal 9", STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "default-fg",               "#DDDDDD",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "default-bg",               "#000000",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "inputbar-fg",              "#9FBC00",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "inputbar-bg",              "#131313",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "statusbar-fg",             "#FFFFFF",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "statusbar-bg",             "#000000",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "completion-fg",            "#DDDDDD",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "completion-bg",            "#232323",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "completion-group-fg",      "#DEDEDE",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "completion-group-bg",      "#000000",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "completion-highlight-fg",  "#232323",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "completion-highlight-bg",  "#9FBC00",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "notification-error-fg",    "#FFFFFF",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "notification-error-bg",    "#FF1212",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "notification-warning-fg",  "#000000",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "notification-warning-bg",  "#F3F000",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "notification-fg",          "#000000",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "notification-bg",          "#FFFFFF",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "tabbar-fg",                "#939393",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "tabbar-bg",                "#000000",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "tabbar-focus-fg",          "#9FBC00",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "tabbar-focus-bg",          "#000000",            STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "word-separator",           " /.-=&#?",           STRING,  TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "window-width",             &window_width,        INT,     TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "window-height",            &window_height,       INT,     TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "n-completion-items",       &n_completion_items,  INT,     TRUE,  NULL, NULL, NULL);
  girara_setting_add(session, "show-scrollbars",          &show_scrollbars,     BOOLEAN, TRUE,  NULL, NULL, NULL);

  /* default shortcuts */
#if (GTK_MAJOR_VERSION == 3)
  girara_shortcut_add(session, 0,                GDK_KEY_Escape, NULL, girara_sc_abort,           normal_mode, 0, NULL);
  girara_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_c,      NULL, girara_sc_abort,           normal_mode, 0, NULL);
  girara_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_q,      NULL, girara_sc_quit,            normal_mode, 0, NULL);
  girara_shortcut_add(session, 0,                GDK_KEY_colon,  NULL, girara_sc_focus_inputbar,  normal_mode, 0, ":");
  girara_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_w,      NULL, girara_sc_tab_close,       normal_mode, 0, NULL);
  girara_shortcut_add(session, 0,                0,              "gt", girara_sc_tab_navigate,    normal_mode, GIRARA_NEXT,     NULL);
  girara_shortcut_add(session, 0,                0,              "gT", girara_sc_tab_navigate,    normal_mode, GIRARA_PREVIOUS, NULL);
#else
  girara_shortcut_add(session, 0,                GDK_Escape, NULL, girara_sc_abort,          normal_mode, 0, NULL);
  girara_shortcut_add(session, GDK_CONTROL_MASK, GDK_c,      NULL, girara_sc_abort,          normal_mode, 0, NULL);
  girara_shortcut_add(session, GDK_CONTROL_MASK, GDK_q,      NULL, girara_sc_quit,           normal_mode, 0, NULL);
  girara_shortcut_add(session, 0,                GDK_colon,  NULL, girara_sc_focus_inputbar, normal_mode, 0, ":");
  girara_shortcut_add(session, GDK_CONTROL_MASK, GDK_w,      NULL, girara_sc_tab_close,      normal_mode, 0, NULL);
  girara_shortcut_add(session, 0,                0,          "gt", girara_sc_tab_navigate,   normal_mode, GIRARA_NEXT,     NULL);
  girara_shortcut_add(session, 0,                0,          "gT", girara_sc_tab_navigate,   normal_mode, GIRARA_PREVIOUS, NULL);
#endif

  /* default inputbar shortcuts */
#if (GTK_MAJOR_VERSION == 3)
  girara_inputbar_shortcut_add(session, 0,                GDK_KEY_Escape,       girara_isc_abort,               0,                           NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_c,            girara_isc_abort,               0,                           NULL);
  girara_inputbar_shortcut_add(session, 0,                GDK_KEY_Tab,          girara_isc_completion,          GIRARA_NEXT,                 NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_Tab,          girara_isc_completion,          GIRARA_NEXT_GROUP,           NULL);
  girara_inputbar_shortcut_add(session, GDK_SHIFT_MASK,   GDK_KEY_ISO_Left_Tab, girara_isc_completion,          GIRARA_PREVIOUS,             NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_ISO_Left_Tab, girara_isc_completion,          GIRARA_PREVIOUS_GROUP,       NULL);
  girara_inputbar_shortcut_add(session, 0,                GDK_KEY_BackSpace,    girara_isc_string_manipulation, GIRARA_DELETE_LAST_CHAR,     NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_h,            girara_isc_string_manipulation, GIRARA_DELETE_LAST_CHAR,     NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_u,            girara_isc_string_manipulation, GIRARA_DELETE_TO_LINE_START, NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_w,            girara_isc_string_manipulation, GIRARA_DELETE_LAST_WORD,     NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_f,            girara_isc_string_manipulation, GIRARA_NEXT_CHAR,            NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_b,            girara_isc_string_manipulation, GIRARA_PREVIOUS_CHAR,        NULL);
#else
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
#endif

  /* default commands */
  girara_inputbar_command_add(session, "map",  "m", girara_cmd_map,  NULL,          "Map a key sequence");
  girara_inputbar_command_add(session, "quit", "q", girara_cmd_quit, NULL,          "Quit the program");
  girara_inputbar_command_add(session, "set",  "s", girara_cmd_set,  girara_cc_set, "Set an option");

  /* default config handle */
  girara_config_handle_add(session, "map", girara_cmd_map);
  girara_config_handle_add(session, "set", girara_cmd_set);

  /* default shortcut mappings */
  girara_shortcut_mapping_add(session, "quit",           girara_sc_quit);
  girara_shortcut_mapping_add(session, "focus_inputbar", girara_sc_focus_inputbar);

  return session;
}

bool
girara_session_init(girara_session_t* session)
{
  if (session->gtk.embed){
    session->gtk.window = gtk_plug_new(session->gtk.embed);
  } else {
    session->gtk.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  }

  session->gtk.box               = GTK_BOX(gtk_vbox_new(FALSE, 0));
  session->gtk.view              = gtk_scrolled_window_new(NULL, NULL);
  session->gtk.viewport          = gtk_viewport_new(NULL, NULL);
  session->gtk.statusbar         = gtk_event_box_new();
  session->gtk.statusbar_entries = GTK_BOX(gtk_hbox_new(FALSE, 0));
  session->gtk.notification_area = gtk_event_box_new();
  session->gtk.notification_text = gtk_label_new(NULL);
  session->gtk.tabbar            = gtk_hbox_new(TRUE, 0);
  session->gtk.inputbar          = GTK_ENTRY(gtk_entry_new());
  session->gtk.tabs              = GTK_NOTEBOOK(gtk_notebook_new());

  /* window */
  GdkGeometry hints = {
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

#if (GTK_MAJOR_VERSION == 3)
  gtk_window_set_has_resize_grip(GTK_WINDOW(session->gtk.window), FALSE);
#endif

  /* view */
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(girara_callback_view_key_press_event), session);

  bool* tmp_bool_value = girara_setting_get(session, "show-scrollbars");
  if (tmp_bool_value) {
    if (*tmp_bool_value == true) {
      gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(session->gtk.view),
          GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    } else {
      gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(session->gtk.view),
          GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    }

    free(tmp_bool_value);
  }

  /* viewport */
  gtk_container_add(GTK_CONTAINER(session->gtk.view), session->gtk.viewport);
  gtk_viewport_set_shadow_type(GTK_VIEWPORT(session->gtk.viewport), GTK_SHADOW_NONE);

  /* box */
  gtk_box_set_spacing(session->gtk.box, 0);
  gtk_container_add(GTK_CONTAINER(session->gtk.window), GTK_WIDGET(session->gtk.box));

  /* statusbar */
  gtk_container_add(GTK_CONTAINER(session->gtk.statusbar), GTK_WIDGET(session->gtk.statusbar_entries));

  /* notification area */
  gtk_container_add(GTK_CONTAINER(session->gtk.notification_area), GTK_WIDGET(session->gtk.notification_text));
  gtk_misc_set_alignment(GTK_MISC(session->gtk.notification_text), 0.0, 0.0);
  gtk_misc_set_padding(GTK_MISC(session->gtk.notification_text), 2, 2);
  gtk_label_set_use_markup(GTK_LABEL(session->gtk.notification_text), TRUE);

  /* inputbar */
  gtk_entry_set_inner_border(session->gtk.inputbar, NULL);
  gtk_entry_set_has_frame(session->gtk.inputbar, FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(session->gtk.inputbar), TRUE);

  session->signals.inputbar_key_pressed = g_signal_connect(G_OBJECT(session->gtk.inputbar), "key-press-event", G_CALLBACK(girara_callback_inputbar_key_press_event), session);
  session->signals.inputbar_activate    = g_signal_connect(G_OBJECT(session->gtk.inputbar), "activate",        G_CALLBACK(girara_callback_inputbar_activate),        session);

  /* tabs */
  gtk_notebook_set_show_border(session->gtk.tabs, FALSE);
  gtk_notebook_set_show_tabs(session->gtk.tabs,   FALSE);

  /* packing */
  gtk_box_pack_start(session->gtk.box, GTK_WIDGET(session->gtk.tabbar),            FALSE, FALSE, 0);
  gtk_box_pack_start(session->gtk.box, GTK_WIDGET(session->gtk.view),              TRUE,  TRUE, 0);
  gtk_box_pack_start(session->gtk.box, GTK_WIDGET(session->gtk.statusbar),         FALSE, FALSE, 0);
  gtk_box_pack_start(session->gtk.box, GTK_WIDGET(session->gtk.notification_area), FALSE, FALSE, 0);
  gtk_box_pack_end(  session->gtk.box, GTK_WIDGET(session->gtk.inputbar),          FALSE, FALSE, 0);

  /* parse color values */
  char* tmp_value = NULL;

  typedef struct color_setting_mapping_s
  {
    char* identifier;
#if (GTK_MAJOR_VERSION == 3)
    GdkRGBA *color;
#else
    GdkColor *color;
#endif
  } color_setting_mapping_t;

  const color_setting_mapping_t color_setting_mappings[] = {
    {"default-fg",              &(session->style.default_foreground)},
    {"default-bg",              &(session->style.default_background)},
    {"inputbar-fg",             &(session->style.inputbar_foreground)},
    {"inputbar-bg",             &(session->style.inputbar_background)},
    {"statusbar-fg",            &(session->style.statusbar_foreground)},
    {"statusbar-bg",            &(session->style.statusbar_background)},
    {"completion-fg",           &(session->style.completion_foreground)},
    {"completion-bg",           &(session->style.completion_background)},
    {"completion-group-fg",     &(session->style.completion_group_foreground)},
    {"completion-group-bg",     &(session->style.completion_group_background)},
    {"completion-highlight-fg", &(session->style.completion_highlight_foreground)},
    {"completion-highlight-bg", &(session->style.completion_highlight_background)},
    {"notification-error-fg",   &(session->style.notification_error_foreground)},
    {"notification-error-bg",   &(session->style.notification_error_background)},
    {"notification-warning-fg", &(session->style.notification_warning_foreground)},
    {"notification-warning-bg", &(session->style.notification_warning_background)},
    {"notification-fg",         &(session->style.notification_default_foreground)},
    {"notification-bg",         &(session->style.notification_default_background)},
    {"tabbar-fg",               &(session->style.tabbar_foreground)},
    {"tabbar-bg",               &(session->style.tabbar_background)},
    {"tabbar-focus-fg",         &(session->style.tabbar_focus_foreground)},
    {"tabbar-focus-bg",         &(session->style.tabbar_focus_background)},
  };

  for (unsigned i = 0; i < LENGTH(color_setting_mappings); i++) {
    tmp_value = girara_setting_get(session, color_setting_mappings[i].identifier);
    if (tmp_value) {
#if (GTK_MAJOR_VERSION == 3)
      gdk_rgba_parse(color_setting_mappings[i].color, tmp_value);
#else
      gdk_color_parse(tmp_value, color_setting_mappings[i].color);
#endif
      free(tmp_value);
      tmp_value = NULL;
    }
  }

  /* parse font */
  tmp_value = girara_setting_get(session, "font");
  if (tmp_value) {
    session->style.font = pango_font_description_from_string(tmp_value);
    free(tmp_value);
    tmp_value = NULL;
  }

#if (GTK_MAJOR_VERSION == 3)
  /* view */
  gtk_widget_override_background_color(GTK_WIDGET(session->gtk.viewport),
      GTK_STATE_NORMAL, &(session->style.default_background));

  /* statusbar */
  gtk_widget_override_background_color(GTK_WIDGET(session->gtk.statusbar),
      GTK_STATE_NORMAL, &(session->style.statusbar_background));

  /* inputbar */
  gtk_widget_override_background_color(GTK_WIDGET(session->gtk.inputbar),
      GTK_STATE_NORMAL, &(session->style.inputbar_background));
  gtk_widget_override_color(GTK_WIDGET(session->gtk.inputbar),
      GTK_STATE_NORMAL, &(session->style.inputbar_foreground));
  gtk_widget_override_font(GTK_WIDGET(session->gtk.inputbar),
      session->style.font);

  /* notification area */
  gtk_widget_override_background_color(GTK_WIDGET(session->gtk.notification_area),
      GTK_STATE_NORMAL, &(session->style.notification_default_background));
  gtk_widget_override_color(GTK_WIDGET(session->gtk.notification_text),
      GTK_STATE_NORMAL, &(session->style.notification_default_foreground));
  gtk_widget_override_font(GTK_WIDGET(session->gtk.notification_text),
      session->style.font);
#else
  /* view */
  gtk_widget_modify_bg(GTK_WIDGET(session->gtk.viewport), GTK_STATE_NORMAL,
      &(session->style.default_background));

  /* statusbar */
  gtk_widget_modify_bg(GTK_WIDGET(session->gtk.statusbar), GTK_STATE_NORMAL,
      &(session->style.statusbar_background));

  /* inputbar */
  gtk_widget_modify_base(GTK_WIDGET(session->gtk.inputbar), GTK_STATE_NORMAL,
      &(session->style.inputbar_background));
  gtk_widget_modify_text(GTK_WIDGET(session->gtk.inputbar), GTK_STATE_NORMAL,
      &(session->style.inputbar_foreground));
  gtk_widget_modify_font(GTK_WIDGET(session->gtk.inputbar),
      session->style.font);

  /* notification area */
  gtk_widget_modify_bg(GTK_WIDGET(session->gtk.notification_area),
      GTK_STATE_NORMAL, &(session->style.notification_default_background));
  gtk_widget_modify_text(GTK_WIDGET(session->gtk.notification_text),
      GTK_STATE_NORMAL, &(session->style.notification_default_foreground));
  gtk_widget_modify_font(GTK_WIDGET(session->gtk.notification_text),
      session->style.font);
#endif

  /* set window size */
  int* window_width  = girara_setting_get(session, "window-width");
  int* window_height = girara_setting_get(session, "window-height");

  if (window_width && window_height) {
    gtk_window_set_default_size(GTK_WINDOW(session->gtk.window), *window_width, *window_height);
  }

  if (window_width) {
    free(window_width);
  }

  if (window_height) {
    free(window_height);
  }

  gtk_widget_show_all(GTK_WIDGET(session->gtk.window));
  gtk_widget_hide(GTK_WIDGET(session->gtk.notification_area));
  gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar));

  return TRUE;
}

bool
girara_session_destroy(girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);

  /* clean up style */
  pango_font_description_free(session->style.font);

  /* clean up shortcuts */
  girara_list_free(session->bindings.shortcuts);
  session->bindings.shortcuts = NULL;

  /* clean up inputbar shortcuts */
  girara_list_free(session->bindings.inputbar_shortcuts);
  session->bindings.inputbar_shortcuts = NULL;

  /* clean up commands */
  girara_list_free(session->bindings.commands);
  session->bindings.commands = NULL;

  /* clean up special commands */
  girara_list_free(session->bindings.special_commands);
  session->bindings.special_commands = NULL;

  /* clean up mouse events */
  girara_list_free(session->bindings.mouse_events);
  session->bindings.mouse_events = NULL;

  /* clean up settings */
  girara_list_free(session->settings);
  session->settings = NULL;

  /* clean up statusbar items */
  girara_list_free(session->elements.statusbar_items);
  session->elements.statusbar_items = NULL;

  /* clean up config handles */
  girara_list_free(session->config.handles);
  session->config.handles = NULL;

  /* clean up shortcut mappings */
  girara_list_free(session->config.shortcut_mappings);
  session->config.shortcut_mappings = NULL;

  /* clean up argument mappings */
  girara_list_free(session->config.argument_mappings);
  session->config.argument_mappings = NULL;

  /* clean up modes */
  girara_list_free(session->modes.identifiers);
  session->modes.identifiers = NULL;

  /* clean up buffer */
  if (session->buffer.command) {
    g_string_free(session->buffer.command, TRUE);
  }

  if (session->global.buffer) {
    g_string_free(session->global.buffer,  TRUE);
  }

  session->buffer.command = NULL;
  session->global.buffer  = NULL;

  /* clean up session */
  g_slice_free(girara_session_t, session);

  return TRUE;
}
