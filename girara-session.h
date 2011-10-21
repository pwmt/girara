/* See LICENSE file for license and copyright information */

#ifndef GIRARA_SESSION_H
#define GIRARA_SESSION_H

#include "girara-types.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#if (GTK_MAJOR_VERSION == 3)
#include <gtk/gtkx.h>
#endif


/**
 * Structure of a girara session
 */
struct girara_session_s
{
  struct
  {
    GtkWidget       *window; /**< The main window of the application */
    GtkBox          *box; /**< A box that contains all widgets */
    GtkWidget       *view; /**< The view area of the applications widgets */
    GtkWidget       *viewport; /**< The viewport of view */
    GtkWidget       *statusbar; /**< The statusbar */
    GtkBox          *statusbar_entries; /**< Statusbar entry box */
    GtkWidget       *notification_area; /**< The notification area */
    GtkWidget       *notification_text; /**< The notification entry */
    GtkWidget       *tabbar; /**< The tabbar */
    GtkEntry        *inputbar; /**< The inputbar */
    GtkNotebook     *tabs; /**< The tabs notebook */
    GtkBox          *results; /**< Completion results */

#if (GTK_MAJOR_VERSION == 3)
    Window embed; /**< Embedded window */
#else
    GdkNativeWindow embed; /**< Embedded window */
#endif // (GTK_MAJOR_VERSION == 3)
  } gtk;

  struct
  {
#if (GTK_MAJOR_VERSION == 3)
    GdkRGBA default_foreground; /**< The default foreground color */
    GdkRGBA default_background; /**< The default background color */
    GdkRGBA inputbar_foreground; /**< The foreground color of the inputbar */
    GdkRGBA inputbar_background; /**< The background color of the inputbar */
    GdkRGBA statusbar_foreground; /**< The foreground color of the statusbar */
    GdkRGBA statusbar_background; /**< The background color of the statusbar */
    GdkRGBA completion_foreground; /**< The foreground color of a completion item */
    GdkRGBA completion_background; /**< The background color of a completion item */
    GdkRGBA completion_group_foreground; /**< The foreground color of a completion group entry */
    GdkRGBA completion_group_background; /**< The background color of a completion group entry */
    GdkRGBA completion_highlight_foreground; /**< The foreground color of a highlighted completion item */
    GdkRGBA completion_highlight_background; /**< The background color of a highlighted completion item */
    GdkRGBA notification_error_foreground; /**< The foreground color of an error notification */
    GdkRGBA notification_error_background; /**< The background color of an error notification */
    GdkRGBA notification_warning_foreground; /**< The foreground color of a warning notification */
    GdkRGBA notification_warning_background; /**< The background color of a warning notification */
    GdkRGBA notification_default_foreground; /**< The foreground color of a default notification */
    GdkRGBA notification_default_background; /**< The background color of a default notification */
    GdkRGBA tabbar_foreground; /**< The foreground color for a tab */
    GdkRGBA tabbar_background; /**< The background color for a tab */
    GdkRGBA tabbar_focus_foreground; /**< The foreground color for a focused tab */
    GdkRGBA tabbar_focus_background; /**< The background color for a focused tab */
#else
    GdkColor default_foreground; /**< The default foreground color */
    GdkColor default_background; /**< The default background color */
    GdkColor inputbar_foreground; /**< The foreground color of the inputbar */
    GdkColor inputbar_background; /**< The background color of the inputbar */
    GdkColor statusbar_foreground; /**< The foreground color of the statusbar */
    GdkColor statusbar_background; /**< The background color of the statusbar */
    GdkColor completion_foreground; /**< The foreground color of a completion item */
    GdkColor completion_background; /**< The background color of a completion item */
    GdkColor completion_group_foreground; /**< The foreground color of a completion group entry */
    GdkColor completion_group_background; /**< The background color of a completion group entry */
    GdkColor completion_highlight_foreground; /**< The foreground color of a highlighted completion item */
    GdkColor completion_highlight_background; /**< The background color of a highlighted completion item */
    GdkColor notification_error_foreground; /**< The foreground color of an error notification */
    GdkColor notification_error_background; /**< The background color of an error notification */
    GdkColor notification_warning_foreground; /**< The foreground color of a warning notification */
    GdkColor notification_warning_background; /**< The background color of a warning notification */
    GdkColor notification_default_foreground; /**< The foreground color of a default notification */
    GdkColor notification_default_background; /**< The background color of a default notification */
    GdkColor tabbar_foreground; /**< The foreground color for a tab */
    GdkColor tabbar_background; /**< The background color for a tab */
    GdkColor tabbar_focus_foreground; /**< The foreground color for a focused tab */
    GdkColor tabbar_focus_background; /**< The background color for a focused tab */
#endif
    PangoFontDescription *font; /**< The used font */
  } style;

  struct
  {
    girara_mouse_event_t* mouse_events; /**< List of mouse events */
    girara_command_t* commands; /**< List of commands */
    girara_list_t* shortcuts; /**< List of shortcuts */
    girara_special_command_t* special_commands; /**< List of special commands */
    girara_list_t* inputbar_shortcuts; /**< List of inputbar shortcuts */
  } bindings;

  struct
  {
    girara_list_t* statusbar_items; /**< List of statusbar items */
  } elements;

  /**
   * List of settings
   */
  girara_list_t* settings;

  struct
  {
    int inputbar_activate; /**< Inputbar activation */
    int inputbar_key_pressed; /**< Pressed key in inputbar */
    int view_key_pressed; /**< Pressed key in view */
  } signals;

  struct
  {
    void (*buffer_changed)(girara_session_t* session);
  } events;

  struct
  {
    GString *buffer; /**< Buffer */
    void* data; /**< User data */
  } global;

  struct
  {
    girara_mode_t current_mode; /**< Current mode */
    girara_list_t *identifiers; /**< List of modes with its string identifiers */
    girara_mode_t normal; /**< The normal mode */
  } modes;

  struct
  {
    int n; /**< Numeric buffer */
    GString *command; /**< Command in buffer */
  } buffer;

  struct
  {
    girara_list_t* handles;
    girara_list_t* shortcut_mappings;
    girara_argument_mapping_t* argument_mappings;
  } config;
};

/**
 * Creates a girara session
 *
 * @return A valid session object
 * @return NULL when an error occured
 */
girara_session_t* girara_session_create();

/**
 * Initializes an girara session
 *
 * @param session The used girara session
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_session_init(girara_session_t* session);

/**
 * Destroys an girara session
 *
 * @param session The used girara session
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_session_destroy(girara_session_t* session);

#endif
