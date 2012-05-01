/* See LICENSE file for license and copyright information */

#ifndef GIRARA_INTERNAL_H
#define GIRARA_INTERNAL_H

#include "types.h"
#include "macros.h"
#include <glib.h>

#define FORMAT_COMMAND "<b>%s</b>"
#define FORMAT_DESCRIPTION "<i>%s</i>"

#define UNUSED(x) GIRARA_UNUSED(x)
#define HIDDEN GIRARA_HIDDEN

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

/**
 * Free girara_setting_t struct
 *
 * @param setting The setting to free.
 */
HIDDEN void girara_setting_free(girara_setting_t* setting);

HIDDEN void girara_config_handle_free(girara_config_handle_t* handle);

HIDDEN void girara_shortcut_mapping_free(girara_shortcut_mapping_t* mapping);

HIDDEN void girara_shortcut_free(girara_shortcut_t* shortcut);

HIDDEN void girara_inputbar_shortcut_free(girara_inputbar_shortcut_t* shortcut);

HIDDEN void girara_mode_string_free(girara_mode_string_t* mode);

HIDDEN void girara_statusbar_item_free(girara_statusbar_item_t* statusbaritem);

HIDDEN void girara_argument_mapping_free(
    girara_argument_mapping_t* argument_mapping);

HIDDEN void girara_special_command_free(
    girara_special_command_t* special_command);

HIDDEN void girara_command_free(girara_command_t* command);

HIDDEN void girara_mouse_event_free(girara_mouse_event_t* mouse_event);

HIDDEN void girara_config_load_default(girara_session_t* session);

HIDDEN void update_state_by_keyval(int *state, int keyval);

/**
 * Default complection function for the settings
 *
 * @param session The used girara session
 * @param input The current input
 */
HIDDEN girara_completion_t* girara_cc_set(girara_session_t* session,
    const char* input);

/**
 * Default command to map sortcuts
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
HIDDEN bool girara_cmd_map(girara_session_t* session,
    girara_list_t* argument_list);

/**
 * Default command to unmap sortcuts
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
HIDDEN bool girara_cmd_unmap(girara_session_t* session,
    girara_list_t* argument_list);

/**
 * Default command to quit the application
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
HIDDEN bool girara_cmd_quit(girara_session_t* session,
    girara_list_t* argument_list);

/**
 * Default command to set the value of settings
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
HIDDEN bool girara_cmd_set(girara_session_t* session,
    girara_list_t* argument_list);

/**
 * Execute an external command
 * * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
HIDDEN bool girara_cmd_exec(girara_session_t* session,
    girara_list_t* argument_list);

/**
 * Process argument as a sequence of keys that were typed by the user
 *
 * @param session The session
 * @param argument The argument
 * @param event Event type
 * @param t Number of times
 * @return true No error occured
 * @return false An error occured
 */
HIDDEN bool girara_sc_feedkeys(girara_session_t* session, girara_argument_t* argument,
    girara_event_t* event, unsigned int t);

/**
 * Structure of a command
 */
struct girara_command_s
{
  char* command; /**< Name of the command */
  char* abbr; /**< Abbreviation of the command */
  girara_command_function_t function; /**< Function */
  girara_completion_function_t completion; /**< Completion function */
  char* description; /**< Description of the command */
};

struct girara_mode_string_s
{
  girara_mode_t index; /**< Index */
  char* name; /**< Name of the mode object */
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
};

/**
 * Structure of a shortcut
 */
struct girara_shortcut_s
{
  guint mask; /**< Mask */
  guint key; /**< Key */
  const char* buffered_command; /**< Buffer command */
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
  girara_event_type_t event_type; /**< Event type */
  girara_argument_t argument; /**< Given argument */
};

/**
 * Config handle
 */
struct girara_config_handle_s
{
  char* identifier;
  girara_command_function_t handle;
};

#endif
