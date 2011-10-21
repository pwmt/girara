/* See LICENSE file for license and copyright information */

#ifndef GIRARA_H
#define GIRARA_H

#include "girara-types.h"
#include "girara-session.h"

/**
 * Function declaration for a statusbar event callback
 *
 * @param widget The statusbar item
 * @param event The occured event
 * @param session The current girara session
 * @return TRUE No error occured
 * @return FALSE Error occured (and forward event)
 */
typedef bool (*girara_statusbar_event_t)(GtkWidget* widget, GdkEvent* event, girara_session_t* session);


struct girara_mode_string_s
{
  girara_mode_t index; /**< Index */
  char* name; /**< Name of the mode object */
};

/**
 * Structure of a tab
 */
struct girara_tab_s
{
  char* title; /**< The title of the tab */
  GtkWidget* widget; /**< The displayed widget of the tab */
  void* data; /**< Custom data */
  girara_session_t* session; /**< Girara session */
};

/**
 * Structure of a statusbar item
 */
struct girara_statusbar_item_s
{
  GtkWidget* box; /**< Event box */
  GtkLabel *text; /**< Text label */
};

/**
 * Shortcut mapping
 */
struct girara_shortcut_mapping_s
{
  char* identifier; /**> Identifier string */
  girara_shortcut_function_t function; /** Shortcut function */
};

/**
 * Argument mapping
 */
struct girara_argument_mapping_s
{
  char* identifier; /**> Identifier string */
  int value; /**> Value */
  struct girara_argument_mapping_s* next; /**> Next entry */
};

/**
 * Structure of a shortcut
 */
struct girara_shortcut_s
{
  guint mask; /**< Mask */
  guint key; /**< Key */
  char* buffered_command; /**< Buffer command */
  girara_shortcut_function_t function; /**< The correspondending function */
  girara_mode_t mode; /**< Mode identifier */
  girara_argument_t argument; /**< Given argument */
};

/**
 * Structure of a inputbar shortcut
 */
struct girara_inputbar_shortcut_s
{
  guint mask; /**< Mask */
  guint key; /**< Key */
  girara_shortcut_function_t function; /**< Function */
  girara_argument_t argument; /**< Given argument */
};

/**
 * Structure of a special command
 */
struct girara_special_command_s
{
  char identifier; /**< Identifier */
  girara_inputbar_special_function_t function; /**< Function */
  bool always; /**< Evalute on every change of the input */
  girara_argument_t argument; /**< Argument */
  struct girara_special_command_s *next; /**< Next special command (linked list) */
};

/**
 * Structure of a mouse event
 */
struct girara_mouse_event_s
{
  guint mask; /**< Mask */
  guint button; /**< Button */
  girara_shortcut_function_t function; /**< Function */
  girara_mode_t mode; /**< Allowed modes */
  girara_argument_t argument; /**< Given argument */
  struct girara_mouse_event_s *next; /**< Next mouse event (linked list) */
};

/**
 * Config handle
 */
struct girara_config_handle_s
{
  char* identifier;
  girara_command_function_t handle;
};

/**
 * Adds an shortcut
 *
 * @param session The used girara session
 * @param modifier The modifier
 * @param key The key
 * @param buffer Buffer command
 * @param function Executed function
 * @param mode Available modes
 * @param argument_n Argument identifier
 * @param argument_data Argument data
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_shortcut_add(girara_session_t* session, guint modifier, guint key, char* buffer, girara_shortcut_function_t function, girara_mode_t mode, int argument_n, void* argument_data);

/**
 * Adds an inputbar command
 *
 * @param session The used girara session
 * @param command The name of the command
 * @param abbreviation The abbreviation of the command
 * @param function Executed function
 * @param completion Completion function
 * @param description Description of the command
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_inputbar_command_add(girara_session_t* session, char* command , char* abbreviation, girara_command_function_t function, girara_completion_function_t completion, char* description);

/**
 * Adds an inputbar shortcut
 *
 * @param session The used girara session
 * @param modifier The modifier
 * @param key The key
 * @param function Executed function
 * @param argument_n Argument identifier
 * @param argument_data Argument data
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_inputbar_shortcut_add(girara_session_t* session, guint modifier, guint key, girara_shortcut_function_t function, int argument_n, void* argument_data);

/**
 * Adds a special command
 *
 * @param session The used girara session
 * @param identifier Char identifier
 * @param function Executed function
 * @param always If the function should executed on every change of the input
 *        (e.g.: incremental search)
 * @param argument_n Argument identifier
 * @param argument_data Argument data
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_special_command_add(girara_session_t* session, char identifier, girara_inputbar_special_function_t function, bool always, int argument_n, void* argument_data);

/**
 * Adds a mouse event
 *
 * @param session The used girara session
 * @param mask The mask
 * @param button Pressed button
 * @param function Executed function
 * @param mode Available mode
 * @param argument_n Argument identifier
 * @param argument_data Argument data
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_mouse_event_add(girara_session_t* session, guint mask, guint button, girara_shortcut_function_t function, girara_mode_t mode, int argument_n, void* argument_data);

/**
 * Creates an statusbar item
 *
 * @param session The used girara session
 * @param expand Expand attribute
 * @param fill Fill attribute
 * @param left True if it should be aligned to the left
 * @param callback Function that gets executed when an event occurs
 * @return The created statusbar item
 * @return NULL An error occured
 */
girara_statusbar_item_t* girara_statusbar_item_add(girara_session_t* session, bool expand, bool fill, bool left, girara_statusbar_event_t callback);

/**
 * Sets the shown text of an statusbar item
 *
 * @param session The used girara session
 * @param item The statusbar item
 * @param text Text that should be displayed
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_statusbar_item_set_text(girara_session_t* session, girara_statusbar_item_t* item, char* text);

/**
 * Sets the foreground color of an statusbar item
 *
 * @param session The used girara session
 * @param item The statusbar item
 * @param color The color code
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_statusbar_item_set_foreground(girara_session_t* session, girara_statusbar_item_t* item, char* color);

/**
 * Sets the background color of the statusbar
 *
 * @param session The used girara session
 * @param color The color code
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_statusbar_set_background(girara_session_t* session, char* color);

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
 * Default shortcut function to focus the inputbar
 *
 * @param session The used girara session
 * @param argument The argument
 * @param t Number of executions
 * @return true No error occured
 * @return false An error occured (abort execution)
 */
bool girara_sc_focus_inputbar(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Default shortcut function to abort
 *
 * @param session The used girara session
 * @param argument The argument
 * @param t Number of executions
 * @return true No error occured
 * @return false An error occured (abort execution)
 */
bool girara_sc_abort(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Default shortcut function to quit the application
 *
 * @param session The used girara session
 * @param argument The argument
 * @param t Number of executions
 * @return true No error occured
 * @return false An error occured (abort execution)
 */
bool girara_sc_quit(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Closes the current tab
 *
 * @param session The used girara session
 * @param argument The argument
 * @param t Number of executions
 * @return true No error occured
 * @return false An error occured (abort execution)
 */
bool girara_sc_tab_close(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Default shortcut function to navigate through tabs
 *
 * @param session The used girara session
 * @param argument The argument
 * @param t Number of execution
 * @return true No error occured
 * @return false An error occured (abort execution)
 */
bool girara_sc_tab_navigate(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Toggles the visibility of the inputbar
 *
 * @param session The used girara session
 * @param argument The argument
 * @param t Numbr of execution
 * @return true No error occured
 * @return false An error occured (abort execution)
 */
bool girara_sc_toggle_inputbar(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Toggles the visibility of the statusbar
 *
 * @param session The used girara session
 * @param argument The argument
 * @param t Numbr of execution
 * @return true No error occured
 * @return false An error occured (abort execution)
 */
bool girara_sc_toggle_statusbar(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Toggles the visibility of the tabbar
 *
 * @param session The used girara session
 * @param argument The argument
 * @param t Numbr of execution
 * @return true No error occured
 * @return false An error occured (abort execution)
 */
bool girara_sc_toggle_tabbar(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Default command to map sortcuts
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_cmd_map(girara_session_t* session, girara_list_t* argument_list);

/**
 * Default command to quit the application
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_cmd_quit(girara_session_t* session, girara_list_t* argument_list);

/**
 * Default command to set the value of settings
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_cmd_set(girara_session_t* session, girara_list_t* argument_list);

/**
 * Default callback for key press events in the view area
 *
 * @param widget The used widget
 * @param event The occured event
 * @param session The used girara session
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_callback_view_key_press_event(GtkWidget* widget, GdkEventKey* event, girara_session_t* session);

/**
 * Default callback if the inputbar gets activated
 *
 * @param entry The inputbar entry
 * @param session The used girara session
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_callback_inputbar_activate(GtkEntry* entry, girara_session_t* session);

/**
 * Default callback if an key in the input bar gets pressed
 *
 * @param widget The used widget
 * @param event The occured event
 * @param session The used girara session
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_callback_inputbar_key_press_event(GtkWidget* widget, GdkEventKey* event, girara_session_t* session);

/**
 * Default inputbar shortcut to abort
 *
 * @param session The used girara session
 * @param argument The argument
 * @param t Number of executions
 * @return true No error occured
 * @return false An error occured (abort execution)
 */
bool girara_isc_abort(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Default inputbar shortcut that completes the given input
 * in the statusbar
 *
 * @param session The used girara session
 * @param argument The argument
 * @param t Number of executions
 * @return true No error occured
 * @return false An error occured (abort execution)
 */
bool girara_isc_completion(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Default inputbar shortcut to manipulate the inputbar string
 *
 * @param session The used girara session
 * @param argument The argument
 * @param t Number of executions
 * @return true No error occured
 * @return false An error occured (abort execution)
 */
bool girara_isc_string_manipulation(girara_session_t* session, girara_argument_t* argument, unsigned int t);

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

/**
 * Returns a copy of the buffer
 *
 * @param session The used girara session
 * @return Copy of the current buffer
 */
char* girara_buffer_get(girara_session_t* session);

/**
 * Parses and evaluates a configuration file
 *
 * @param session The used girara session
 * @param path Path to the configuration file
 */
void girara_config_parse(girara_session_t* session, const char* path);

/**
 * Adds an additional config handler
 *
 * @param session The girara session
 * @param identifier Identifier of the handle
 * @param handle Handle
 * @return true if no error occured, otherwise false
 */
bool girara_config_handle_add(girara_session_t* session, const char* identifier,
    girara_command_function_t handle);

/**
 * Creates a mapping between a shortcut function and an identifier and is used
 * to evaluate the mapping command
 *
 * @param session The girara session
 * @param identifier Optional identifier
 * @param function The function that should be mapped
 * @return true if no error occured
 */
bool girara_shortcut_mapping_add(girara_session_t* session, char* identifier,
    girara_shortcut_function_t function);

/**
 * Creates a mapping between a shortcut argument and an identifier and is used
 * to evalue the mapping command
 *
 * @param session The girara session
 * @param identifier The identifier
 * @param value The value that should be represented
 * @return true if no error occured
 */
bool girara_argument_mapping_add(girara_session_t* session, char* identifier,
    int value);

/**
 * Enables the tab view. If girara_set_view is used, the tab bar will
 * automatically vanish and girara_tabs_enable has to be called another time to
 * re-enable it again.
 *
 * @param session The girara session
 */
void girara_tabs_enable(girara_session_t* session);

/**
 * Creates and adds a new tab to the tab view
 *
 * @param session The girara session
 * @param title Title of the tab (optional)
 * @param widget Displayed widget
 * @param next_to_current Tab should be created right next to the current one
 * @param data Custom data
 * @return A new tab object or NULL if an error occured
 */
girara_tab_t* girara_tab_new(girara_session_t* session, const char* title,
    GtkWidget* widget, bool next_to_current, void* data);

/**
 * Removes and destroys a tab from the tab view
 *
 * @param session The girara session
 * @param tab Tab
 */
void girara_tab_remove(girara_session_t* session, girara_tab_t* tab);

/**
 * Returns the tab at the given index
 *
 * @param session The girara session
 * @param index Index of the tab
 * @return The tab object or NULL if an error occured
 */
girara_tab_t* girara_tab_get(girara_session_t* session, unsigned int index);

/**
 * Returns the number of tabs
 *
 * @param session The girara session
 * @return The number of tabs
 */
int girara_get_number_of_tabs(girara_session_t* session);

/**
 * Updates the color and states of all tabs
 *
 * @param session The girara session
 */
void girara_tab_update(girara_session_t* session);

/**
 * Returns the current tab
 *
 * @param session The girara session
 * @return The current tab or NULL if an error occured
 */
girara_tab_t* girara_tab_current_get(girara_session_t* session);

/**
 * Sets the current tab
 *
 * @param session The girara session
 * @param tab The new current tab
 */
void girara_tab_current_set(girara_session_t* session, girara_tab_t* tab);

/**
 * Sets the shown title of the tab
 *
 * @param tab The tab
 * @param title The new title
 */
void girara_tab_title_set(girara_tab_t* tab, const char* title);

/**
 * Returns the title of the tab
 *
 * @param tab The tab
 * @return The title of the tab or NULL if an error occured
 */
const char* girara_tab_title_get(girara_tab_t* tab);

/**
 * Returns the position of the tab
 *
 * @param session Girara session
 * @param tab The tab
 * @return The id of the tab or -1 if an error occured
 */
int girara_tab_position_get(girara_session_t* session, girara_tab_t* tab);

/**
 * Sets the new position of the tab
 *
 * @param session Girara session
 * @param tab The tab
 * @param position The new position
 */
void girara_tab_position_set(girara_session_t* session, girara_tab_t* tab,
    unsigned int position);

/**
 * Default implementation of the event that is executed if a tab is clicked
 *
 * @param widget The widget
 * @param event The event
 * @param data Additional data
 * @return true if an error occured, otherwise false
 */
bool girara_callback_tab_clicked(GtkWidget* widget, GdkEventButton* event, gpointer data);

/**
 * Displays a notification for the user. It is possible to pass GIRARA_INFO,
 * GIRARA_WARNING or GIRARA_ERROR as a notification level.
 *
 * @param session The girara session
 * @param level The level
 * @param format String format
 * @param ...
 */
void girara_notify(girara_session_t* session, int level, const char* format, ...);

#include "girara-utils.h"
#include "girara-datastructures.h"
#include "girara-settings.h"
#include "girara-completion.h"

#endif
