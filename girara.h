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

#include "girara-utils.h"
#include "girara-datastructures.h"
#include "girara-settings.h"
#include "girara-completion.h"
#include "girara-tabs.h"

#endif
