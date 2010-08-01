/* See LICENSE file for license and copyright information */

#include <stdlib.h>

#include "girara.h"

girara_session_t*
girara_session_create()
{
  girara_session_t* session = malloc(sizeof(girara_session_t));
  if(!session)
    return NULL;

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
  session->bindings.shortcuts          = NULL;
  session->bindings.inputbar_shortcuts = NULL;

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

  return session;
}

int
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

  /* box */
  gtk_box_set_spacing(session->gtk.box, 0);
  gtk_container_add(GTK_CONTAINER(session->gtk.window), GTK_WIDGET(session->gtk.box));

  /* statusbar */
  gtk_container_add(GTK_CONTAINER(session->gtk.statusbar), GTK_WIDGET(session->gtk.statusbar_entries));

  /* inputbar */
  gtk_entry_set_inner_border(session->gtk.inputbar, NULL);
  gtk_entry_set_has_frame(session->gtk.inputbar, FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(session->gtk.inputbar), TRUE);

  /* packing */
  gtk_box_pack_start(session->gtk.box, GTK_WIDGET(session->gtk.view),       TRUE,  TRUE, 0);
  gtk_box_pack_start(session->gtk.box, GTK_WIDGET(session->gtk.statusbar), FALSE, FALSE, 0);
  gtk_box_pack_end(  session->gtk.box, GTK_WIDGET(session->gtk.inputbar),  FALSE, FALSE, 0);

  /* set values */
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

  gtk_widget_show_all(GTK_WIDGET(session->gtk.window));

  return 0;
}

void
girara_session_destroy(girara_session_t* session)
{
  if(session)
    free(session);
}

int
girara_setting_add(girara_session_t* session, char* name, void* value, girara_setting_type_t type, gboolean init_only, char* description, girara_setting_callback_t callback)
{
  return 0;
}

int
girara_setting_set(girara_session_t* session, char* name, void* value)
{
  return 0;
}

int
girara_shortcut_add(girara_session_t* session, int modifier, int key, char* buffer, girara_shortcut_function_t function, girara_mode_t mode, girara_argument_t argument)
{
  return 0;
}

int
girara_inputbar_command_add(girara_session_t* session, char* command , char* abbreviation, girara_command_function_t function, girara_completion_function_t completion, char* description)
{
  return 0;
}

int
girara_inputbar_shortcut_add(girara_session_t* session, int modifier, int key, girara_shortcut_function_t function, girara_argument_t argument)
{
  return 0;
}

int 
girara_inputbar_special_command_add(girara_session_t* session, char identifier, girara_inputbar_special_function_t function, gboolean always, girara_argument_t argument)
{
  return 0;
}

int
girara_mouse_event_add(girara_session_t* session, int mask, int button, girara_shortcut_function_t function, girara_mode_t mode, girara_argument_t argument)
{
  return 0;
}

girara_statusbar_item_t*
girara_statusbar_add_item(girara_session_t* session, gboolean expand, gboolean fill, girara_statusbar_event_t callback)
{
  return NULL;
}

int
girara_statusbar_item_set_text(girara_session_t* session, girara_statusbar_item_t* item, char* text)
{
  return 0;
}

int
girara_set_view(girara_session_t* session, GtkWidget* widget)
{
  return 0;
}
