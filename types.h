/* See LICENSE file for license and copyright information */

#ifndef GIRARA_TYPES_H
#define GIRARA_TYPES_H

#include <stdbool.h>

typedef struct girara_tree_node_s girara_tree_node_t;
typedef struct girara_list_s girara_list_t;
typedef struct girara_list_iterator_s girara_list_iterator_t;
typedef struct girara_setting_s girara_setting_t;
typedef struct girara_session_s girara_session_t;
typedef struct girara_command_s girara_command_t;
typedef struct girara_mouse_event_s girara_mouse_event_t;
typedef struct girara_config_handle_s girara_config_handle_t;
typedef struct girara_mode_string_s girara_mode_string_t;
typedef struct girara_tab_s girara_tab_t;
typedef struct girara_statusbar_item_s girara_statusbar_item_t;
typedef struct girara_argument_s girara_argument_t;
typedef struct girara_shortcut_mapping_s girara_shortcut_mapping_t;
typedef struct girara_argument_mapping_s girara_argument_mapping_t;
typedef struct girara_completion_element_s girara_completion_element_t;
typedef struct girara_completion_s girara_completion_t;
typedef struct girara_completion_group_s girara_completion_group_t;
typedef struct girara_shortcut_s girara_shortcut_t;
typedef struct girara_inputbar_shortcut_s girara_inputbar_shortcut_t;
typedef struct girara_special_command_s girara_special_command_t;

/**
 * This structure defines the possible argument identifiers
 */
enum
{
  GIRARA_HIDE = 1, /**< Hide the completion list */
  GIRARA_NEXT, /**< Next entry */
  GIRARA_PREVIOUS, /**< Previous entry */
  GIRARA_NEXT_GROUP, /**< Next group in the completion list */
  GIRARA_PREVIOUS_GROUP, /**< Previous group in the completion list */
  GIRARA_HIGHLIGHT, /**< Highlight the entry */
  GIRARA_NORMAL, /**< Set to the normal state */
  GIRARA_DELETE_LAST_WORD, /**< Delete the last word */
  GIRARA_DELETE_LAST_CHAR, /**< Delete the last character */
  GIRARA_NEXT_CHAR, /**< Go to the next character */
  GIRARA_PREVIOUS_CHAR, /**< Go to the previous character */
  GIRARA_DELETE_TO_LINE_START, /**< Delete the line to the start */
  GIRARA_DELETE_TO_LINE_END, /**< Delete the line to the end */
  GIRARA_DELETE_CURR_CHAR, /**< Delete current char */
  GIRARA_GOTO_START, /**< Go to start of the line */
  GIRARA_GOTO_END /**< Go to end of the line */

};

/**
 * Debug levels
 */
typedef enum girara_debug_level_e
{
  GIRARA_INFO, /**> Information debug output */
  GIRARA_WARNING, /**> Warning level */
  GIRARA_DEBUG, /**> Debug messages */
  GIRARA_ERROR /**> Error */
} girara_debug_level_t;

/**
 * Mode identifier
 */
typedef int girara_mode_t;

/**
 * Function declaration of a function that generates a completion group
 *
 * @param session The current girara session
 * @param input The current input
 * @return The completion group
 */
typedef girara_completion_t* (*girara_completion_function_t)(girara_session_t* session, const char* input);

/**
 * Function declaration of a inputbar special function
 *
 * @param session The current girara session
 * @param input The current input
 * @param argument The given argument
 * @return TRUE No error occured
 * @return FALSE Error occured
 */
typedef bool (*girara_inputbar_special_function_t)(girara_session_t* session, const char* input, girara_argument_t* argument);

/**
 * Function declaration of a command function
 *
 * @param session The current girara session
 * @param argc Number of arguments
 * @param argv Arguments
 */
typedef bool (*girara_command_function_t)(girara_session_t* session, girara_list_t* argument_list);

/**
 * Function declaration of a shortcut function
 *
 * If a numeric value has been written into the buffer, this function gets as
 * often executed as the value defines or until the function returns false the
 * first time.
 */
typedef bool (*girara_shortcut_function_t)(girara_session_t*, girara_argument_t*, unsigned int);

/**
 * Function declaration of a function that frees something.
 *
 * @param data the data to be freed.
 */
typedef void (*girara_free_function_t)(void* data);

/** Function declaration of a function called as callback from girara_list_*
 * functions.
 *
 * @param data a list element.
 * @param userdata data passed as userdata to the calling function.
 */
typedef void (*girara_list_callback_t)(void* data, void* userdata);

/** Function declaration of a function which compares two elements.
 *
 * @param data1 the first element.
 * @param data2 the second element.
 * @return -1 if data1 < data2, 0 if data1 == data2 and 1 if data1 > data2
 */
typedef int (*girara_compare_function_t)(const void* data1, const void* data2);

/**
 * This structure defines the possible types that a setting value can have
 */
typedef enum girara_setting_type_e
{
  BOOLEAN, /**< Boolean type */
  FLOAT, /**< Floating number */
  INT, /**< Integer */
  STRING, /**< String */
  UNKNOWN = 0xFFFF /**< Unknown type */
} girara_setting_type_t;

/**
 * Function declaration for a settings callback
 *
 * @param session The current girara session
 * @param name The name of the affected settting
 * @param type The type of the affected setting
 * @param value Pointer to the new value
 * @param data User data
 */
typedef void (*girara_setting_callback_t)(girara_session_t* session, const char* name, girara_setting_type_t type, void* value, void* data);

/**
 * Definition of an argument of a shortcut or buffered command
 */
struct girara_argument_s
{
  int   n; /**< Identifier */
  void *data; /**< Data */
};

#endif
