/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n-lib.h>

#include "config.h"
#include "commands.h"
#include "datastructures.h"
#include "internal.h"
#include "session.h"
#include "settings-manager.h"
#include "shortcuts.h"
#include "utils.h"

#define COMMENT_PREFIX "\"#"

static void
cb_window_icon(GiraraSettingsManager* manager, const char* name, void* value, void* data)
{
  if (g_strcmp0(name, "window-icon") != 0) {
    return;
  }

  girara_session_t* session = data;
  g_return_if_fail(session != NULL && manager != NULL && value != NULL);

  if (session->gtk.window != NULL) {
    GError* error = NULL;
    gtk_window_set_icon_from_file(GTK_WINDOW(session->gtk.window), (const char*) value, &error);
    if (error != NULL) {
      girara_error("failed to load window icon: %s", error->message);
      g_error_free(error);
    }
  }
}

static void
cb_font(GiraraSettingsManager* manager, const char* name, void* value, void* data)
{
  if (g_strcmp0(name, "font") != 0) {
    return;
  }

  girara_session_t* session = data;
  g_return_if_fail(session != NULL && manager != NULL && value != NULL);

  pango_font_description_free(session->style.font);

  /* parse font */
  PangoFontDescription* font = pango_font_description_from_string((const char*) value);
  session->style.font = font;

  /* inputbar */
  if (session->gtk.inputbar_entry != NULL) {
    gtk_widget_override_font(GTK_WIDGET(session->gtk.inputbar_entry), font);
  }

  if (session->gtk.inputbar_dialog != NULL) {
    gtk_widget_override_font(GTK_WIDGET(session->gtk.inputbar_dialog), font);
  }

  /* notification area */
  if (session->gtk.notification_text != NULL) {
    gtk_widget_override_font(GTK_WIDGET(session->gtk.notification_text), font);
  }

  GIRARA_LIST_FOREACH(session->elements.statusbar_items, girara_statusbar_item_t *, iter, item)
    if (item != NULL){
      gtk_widget_override_font(GTK_WIDGET(item->text), font);
    }
  GIRARA_LIST_FOREACH_END(session->elements.statusbar_items, girara_statusbar_item_t *, iter, item);
}

static void
cb_guioptions(GiraraSettingsManager* manager, const char* name, void* value, void* data)
{
  if (g_strcmp0(name, "guioptions") != 0) {
    return;
  }

  girara_session_t* session = data;
  g_return_if_fail(session != NULL && manager != NULL && value != NULL);

    /* set default values */
  bool show_commandline = false;
  bool show_statusbar   = false;

  /* evaluate input */
  const char* input = value;
  const size_t input_length = strlen(input);
  for (size_t i = 0; i < input_length; i++) {
    switch (input[i]) {
      /* command line */
      case 'c':
        show_commandline = true;
        break;
      /* statusbar */
      case 's':
        show_statusbar = true;
        break;
    }
  }

  /* apply settings */
  if (show_commandline == true) {
    session->global.autohide_inputbar = false;
    gtk_widget_show(session->gtk.inputbar);
  } else {
    session->global.autohide_inputbar = true;
    gtk_widget_hide(session->gtk.inputbar);
  }

  if (show_statusbar == true) {
    session->global.hide_statusbar = false;
    gtk_widget_show(session->gtk.statusbar);
  } else {
    session->global.hide_statusbar = true;
    gtk_widget_hide(session->gtk.statusbar);
  }
}

void
girara_config_load_default(girara_session_t* session)
{
  if (session == NULL) {
    return;
  }

  /* values */
  int window_width          = 800;
  int window_height         = 600;
  int n_completion_items    = 15;
  bool show_scrollbars      = false;
  girara_mode_t normal_mode = session->modes.normal;

  /* other values */
  session->global.autohide_inputbar = true;

  /* signals from settings manager */
  GiraraSettingsManager* manager = session->settings;
  g_signal_connect(manager, "value-changed", G_CALLBACK(cb_guioptions), session);
  g_signal_connect(manager, "value-changed", G_CALLBACK(cb_font), session);
  g_signal_connect(manager, "value-changed", G_CALLBACK(cb_window_icon), session);

  /* settings */
  girara_settings_manager_add(manager, "font",                     "monospace normal 9", STRING,  FALSE, _("Font"));
  girara_settings_manager_add(manager, "default-fg",               "#DDDDDD",            STRING,  TRUE,  _("Default foreground color"));
  girara_settings_manager_add(manager, "default-bg",               "#000000",            STRING,  TRUE,  _("Default background color"));
  girara_settings_manager_add(manager, "inputbar-fg",              "#9FBC00",            STRING,  TRUE,  _("Inputbar foreground color"));
  girara_settings_manager_add(manager, "inputbar-bg",              "#131313",            STRING,  TRUE,  _("Inputbar background color"));
  girara_settings_manager_add(manager, "statusbar-fg",             "#FFFFFF",            STRING,  TRUE,  _("Statusbar foreground color"));
  girara_settings_manager_add(manager, "statusbar-bg",             "#000000",            STRING,  TRUE,  _("Statsubar background color"));
  girara_settings_manager_add(manager, "completion-fg",            "#DDDDDD",            STRING,  TRUE,  _("Completion foreground color"));
  girara_settings_manager_add(manager, "completion-bg",            "#232323",            STRING,  TRUE,  _("Completion background color"));
  girara_settings_manager_add(manager, "completion-group-fg",      "#DEDEDE",            STRING,  TRUE,  _("Completion group foreground color"));
  girara_settings_manager_add(manager, "completion-group-bg",      "#000000",            STRING,  TRUE,  _("Completion group background color"));
  girara_settings_manager_add(manager, "completion-highlight-fg",  "#232323",            STRING,  TRUE,  _("Completion highlight foreground color"));
  girara_settings_manager_add(manager, "completion-highlight-bg",  "#9FBC00",            STRING,  TRUE,  _("Completion highlight background color"));
  girara_settings_manager_add(manager, "notification-error-fg",    "#FFFFFF",            STRING,  TRUE,  _("Error notification foreground color"));
  girara_settings_manager_add(manager, "notification-error-bg",    "#FF1212",            STRING,  TRUE,  _("Error notification background color"));
  girara_settings_manager_add(manager, "notification-warning-fg",  "#000000",            STRING,  TRUE,  _("Warning notification foreground color"));
  girara_settings_manager_add(manager, "notification-warning-bg",  "#F3F000",            STRING,  TRUE,  _("Warning notifaction background color"));
  girara_settings_manager_add(manager, "notification-fg",          "#000000",            STRING,  TRUE,  _("Notification foreground color"));
  girara_settings_manager_add(manager, "notification-bg",          "#FFFFFF",            STRING,  TRUE,  _("Notification background color"));
  girara_settings_manager_add(manager, "tabbar-fg",                "#939393",            STRING,  TRUE,  _("Tab bar foreground color"));
  girara_settings_manager_add(manager, "tabbar-bg",                "#000000",            STRING,  TRUE,  _("Tab bar background color"));
  girara_settings_manager_add(manager, "tabbar-focus-fg",          "#9FBC00",            STRING,  TRUE,  _("Tab bar foreground color (active)"));
  girara_settings_manager_add(manager, "tabbar-focus-bg",          "#000000",            STRING,  TRUE,  _("Tab bar background color (active)"));
  girara_settings_manager_add(manager, "word-separator",           " /.-=&#?",           STRING,  TRUE,  NULL);
  girara_settings_manager_add(manager, "window-width",             &window_width,        INT,     TRUE,  _("Initial window width"));
  girara_settings_manager_add(manager, "window-height",            &window_height,       INT,     TRUE,  _("Initial window height"));
  girara_settings_manager_add(manager, "n-completion-items",       &n_completion_items,  INT,     TRUE,  _("Number of completion items"));
  girara_settings_manager_add(manager, "show-scrollbars",          &show_scrollbars,     BOOLEAN, TRUE,  _("Show scrollbars"));
  girara_settings_manager_add(manager, "window-icon",              "",                   STRING,  FALSE, _("Window icon"));
  girara_settings_manager_add(manager, "exec-command",             "",                   STRING,  FALSE, _("Command to execute in :exec"));
  girara_settings_manager_add(manager, "guioptions",               "s",                  STRING,  FALSE, _("Show or hide certain GUI elements"));

  /* shortcuts */
  girara_shortcut_add(session, 0,                GDK_KEY_Escape, NULL, girara_sc_abort,           normal_mode, 0, NULL);
  girara_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_c,      NULL, girara_sc_abort,           normal_mode, 0, NULL);
  girara_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_q,      NULL, girara_sc_quit,            normal_mode, 0, NULL);
  girara_shortcut_add(session, 0,                GDK_KEY_colon,  NULL, girara_sc_focus_inputbar,  normal_mode, 0, ":");
  girara_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_w,      NULL, girara_sc_tab_close,       normal_mode, 0, NULL);
  girara_shortcut_add(session, 0,                0,              "gt", girara_sc_tab_navigate,    normal_mode, GIRARA_NEXT,     NULL);
  girara_shortcut_add(session, 0,                0,              "gT", girara_sc_tab_navigate,    normal_mode, GIRARA_PREVIOUS, NULL);

  /* inputbar shortcuts */
  girara_inputbar_shortcut_add(session, 0,                GDK_KEY_Escape,       girara_isc_abort,               0,                           NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_c,            girara_isc_abort,               0,                           NULL);
  girara_inputbar_shortcut_add(session, 0,                GDK_KEY_Tab,          girara_isc_completion,          GIRARA_NEXT,                 NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_Tab,          girara_isc_completion,          GIRARA_NEXT_GROUP,           NULL);
  girara_inputbar_shortcut_add(session, 0,                GDK_KEY_ISO_Left_Tab, girara_isc_completion,          GIRARA_PREVIOUS,             NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_ISO_Left_Tab, girara_isc_completion,          GIRARA_PREVIOUS_GROUP,       NULL);
  girara_inputbar_shortcut_add(session, 0,                GDK_KEY_BackSpace,    girara_isc_string_manipulation, GIRARA_DELETE_LAST_CHAR,     NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_h,            girara_isc_string_manipulation, GIRARA_DELETE_LAST_CHAR,     NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_u,            girara_isc_string_manipulation, GIRARA_DELETE_TO_LINE_START, NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_k,            girara_isc_string_manipulation, GIRARA_DELETE_TO_LINE_END,   NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_d,            girara_isc_string_manipulation, GIRARA_DELETE_CURR_CHAR,     NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_w,            girara_isc_string_manipulation, GIRARA_DELETE_LAST_WORD,     NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_f,            girara_isc_string_manipulation, GIRARA_NEXT_CHAR,            NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_b,            girara_isc_string_manipulation, GIRARA_PREVIOUS_CHAR,        NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_a,            girara_isc_string_manipulation, GIRARA_GOTO_START,           NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_e,            girara_isc_string_manipulation, GIRARA_GOTO_END,             NULL);
  girara_inputbar_shortcut_add(session, 0,                GDK_KEY_Up,           girara_isc_command_history,     GIRARA_PREVIOUS,             NULL);
  girara_inputbar_shortcut_add(session, 0,                GDK_KEY_Down,         girara_isc_command_history,     GIRARA_NEXT,                 NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_p,            girara_isc_command_history,     GIRARA_PREVIOUS,             NULL);
  girara_inputbar_shortcut_add(session, GDK_CONTROL_MASK, GDK_KEY_n,            girara_isc_command_history,     GIRARA_NEXT,                 NULL);

  /* commands */
  girara_inputbar_command_add(session, "exec",  NULL, girara_cmd_exec,  NULL,          _("Execute a command"));
  girara_inputbar_command_add(session, "map",   "m",  girara_cmd_map,   NULL,          _("Map a key sequence"));
  girara_inputbar_command_add(session, "quit",  "q",  girara_cmd_quit,  NULL,          _("Quit the program"));
  girara_inputbar_command_add(session, "set",   "s",  girara_cmd_set,   girara_cc_set, _("Set an option"));
  girara_inputbar_command_add(session, "unmap", NULL, girara_cmd_unmap, NULL,          _("Unmap a key sequence"));

  /* config handle */
  girara_config_handle_add(session, "map",   girara_cmd_map);
  girara_config_handle_add(session, "set",   girara_cmd_set);
  girara_config_handle_add(session, "unmap", girara_cmd_unmap);

  /* shortcut mappings */
  girara_shortcut_mapping_add(session, "focus_inputbar", girara_sc_focus_inputbar);
  girara_shortcut_mapping_add(session, "quit",           girara_sc_quit);
  girara_shortcut_mapping_add(session, "set",            girara_sc_set);
  girara_shortcut_mapping_add(session, "feedkeys",       girara_sc_feedkeys);
}

bool
girara_config_handle_add(girara_session_t* session, const char* identifier, girara_command_function_t handle)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(identifier != NULL, false);

  /* search for existing config handle */
  GIRARA_LIST_FOREACH(session->config.handles, girara_config_handle_t*, iter, data)
    if (strcmp(data->identifier, identifier) == 0) {
      data->handle = handle;
      girara_list_iterator_free(iter);
      return true;
    }
  GIRARA_LIST_FOREACH_END(session->config.handles, girara_config_handle_t*, iter, data);

  /* add new config handle */
  girara_config_handle_t* config_handle = g_slice_new(girara_config_handle_t);

  config_handle->identifier = g_strdup(identifier);
  config_handle->handle     = handle;
  girara_list_append(session->config.handles, config_handle);

  return true;
}

void
girara_config_handle_free(girara_config_handle_t* handle)
{
  if (handle == NULL) {
    return;
  }

  g_free(handle->identifier);
  g_slice_free(girara_config_handle_t, handle);
}

static bool
config_parse(girara_session_t* session, const char* path)
{
  /* open file */
  FILE* file = girara_file_open(path, "r");

  if (file == NULL) {
    return false;
  }

  /* read lines */
  char* line = NULL;
  unsigned int line_number = 1;
  while ((line = girara_file_read_line(file)) != NULL) {
    /* skip empty lines and comments */
    if (strlen(line) == 0 || strchr(COMMENT_PREFIX, line[0]) != NULL) {
      free(line);
      continue;
    }

    gchar** argv = NULL;
    gint    argc = 0;

    girara_list_t* argument_list = girara_list_new();
    if (argument_list == NULL) {
      free(line);
      fclose(file);
      return false;
    }

    girara_list_set_free_function(argument_list, g_free);
    if (g_shell_parse_argv(line, &argc, &argv, NULL) != FALSE) {
      for(int i = 1; i < argc; i++) {
        char* argument = g_strdup(argv[i]);
        girara_list_append(argument_list, (void*) argument);
      }
    } else {
      girara_list_free(argument_list);
      fclose(file);
      free(line);
      return false;
    }

    /* include gets a special treatment */
    if (strcmp(argv[0], "include") == 0) {
      if (argc != 2) {
        girara_warning("Could not process line %d in '%s': usage: include path.", line_number, path);
      } else {
        char* newpath = NULL;
        if (g_path_is_absolute(argv[1]) == TRUE) {
          newpath = g_strdup(argv[1]);
        } else {
          char* basename = g_path_get_dirname(path);
          char* tmp = g_build_filename(basename, argv[1], NULL);
          newpath = girara_fix_path(tmp);
          g_free(tmp);
          g_free(basename);
        }

        if (strcmp(newpath, path) == 0) {
          girara_warning("Could not process line %d in '%s': trying to include itself.", line_number, path);
        } else {
          girara_debug("Loading config file '%s'.", newpath);
          if (config_parse(session, newpath) == FALSE) {
            girara_warning("Could not process line %d in '%s': failed to load '%s'.", line_number, path, newpath);
          }
        }
        g_free(newpath);
      }
    } else {
      /* search for config handle */
      girara_config_handle_t* handle = NULL;
      GIRARA_LIST_FOREACH(session->config.handles, girara_config_handle_t*, iter, tmp)
        handle = tmp;
        if (strcmp(handle->identifier, argv[0]) == 0) {
          handle->handle(session, argument_list);
          break;
        } else {
          handle = NULL;
        }
      GIRARA_LIST_FOREACH_END(session->config.handles, girara_config_handle_t*, iter, tmp);

      if (handle == NULL) {
        girara_warning("Could not process line %d in '%s': Unknown handle '%s'", line_number, path, argv[0]);
      }
    }

    line_number++;
    girara_list_free(argument_list);
    g_strfreev(argv);
    free(line);
  }

  fclose(file);
  return true;
}

void
girara_config_parse(girara_session_t* session, const char* path)
{
  config_parse(session, path);
}

