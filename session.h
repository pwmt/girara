/* See LICENSE file for license and copyright information */

#ifndef GIRARA_SESSION_H
#define GIRARA_SESSION_H

#include "types.h"
#include "macros.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#if (GTK_MAJOR_VERSION == 3)
#include <gtk/gtkx.h>
#elif GTK_MAJOR_VERSION == 2
#include "gtk2-compat.h"
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
    GtkBox          *inputbar_box; /**< Inputbar box */
    GtkWidget       *inputbar; /**< Inputbar event box */
    GtkLabel        *inputbar_dialog; /**< Inputbar dialog */
    GtkEntry        *inputbar_entry; /**< Inputbar entry */
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
    PangoFontDescription *font; /**< The used font */
  } style;

  struct
  {
    girara_list_t* mouse_events; /**< List of mouse events */
    girara_list_t* commands; /**< List of commands */
    girara_list_t* shortcuts; /**< List of shortcuts */
    girara_list_t* special_commands; /**< List of special commands */
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
    girara_list_t* argument_mappings;
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
 * @param sessionname Name of the session (can be NULL)
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_session_init(girara_session_t* session, const char* appname);

/**
 * Destroys an girara session
 *
 * @param session The used girara session
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_session_destroy(girara_session_t* session);

/**
 * Sets the view widget of girara
 *
 * @param session The used girara session
 * @param widget The widget that should be displayed
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_set_view(girara_session_t* session, GtkWidget* widget);

/**
 * Returns a copy of the buffer
 *
 * @param session The used girara session
 * @return Copy of the current buffer
 */
char* girara_buffer_get(girara_session_t* session);

/**
 * Displays a notification for the user. It is possible to pass GIRARA_INFO,
 * GIRARA_WARNING or GIRARA_ERROR as a notification level.
 *
 * @param session The girara session
 * @param level The level
 * @param format String format
 * @param ...
 */
void girara_notify(girara_session_t* session, int level, const char* format, ...) GIRARA_PRINTF(3, 4);

/**
 * Adds a new mode by its string identifier
 *
 * @param session The used girara session
 * @param name The string identifier used in configs/inputbar etc to refer by
 * @return A newly defined girara_mode_t associated with name
 */
girara_mode_t girara_mode_add(girara_session_t* session, const char* name);

/**
 * Sets the current mode
 *
 * @param session The used girara session
 * @param mode The new mode
 */
void girara_mode_set(girara_session_t* session, girara_mode_t mode);

/**
 * Returns the current mode
 *
 * @param session The used girara session
 * @return The current mode
 */
girara_mode_t girara_mode_get(girara_session_t* session);

#endif
