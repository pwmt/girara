/* See LICENSE file for license and copyright information */

#ifndef GIRARA_H
#define GIRARA_H

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdbool.h>

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
  GIRARA_DELETE_TO_LINE_START /** Delete the line until the start */
};

/**
 * This structure defines the possible types that a setting value can have
 */
typedef enum girara_setting_type_e
{
  BOOLEAN, /**< Boolean type */
  FLOAT, /**< Floating number */
  INT, /**< Integer */
  STRING /**< String */
} girara_setting_type_t;

/**
 * Mode identifier
 */
typedef int girara_mode_t;

/**
 * Session typedef
 */
typedef struct girara_session_s girara_session_t;

/**
 * Setting typedef
 */
typedef struct girara_setting_s girara_setting_t;

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
 * Function declaration for a settings callback
 *
 * @param session The current girara session
 * @param setting The affected setting
 */
typedef void (*girara_setting_callback_t)(girara_session_t* session, girara_setting_t* setting);

/**
 * Structure of a settings entry
 */
struct girara_setting_s
{
  char* name; /**< Name of the setting */
  union
  {
    bool b; /**< Boolean */
    int i; /**< Integer */
    float f; /**< Floating number */
    char *s; /**< String */
  } value; /**< Value of the setting */
  int type; /**< Type identifier */
  bool init_only; /**< Option can be set only before girara gets initialized */
  char* description; /**< Description of this setting */
  girara_setting_callback_t callback; /**< Callback that gets executed when the value of the setting changes */
  struct girara_setting_s *next; /**< Next settings entry (linked list) */
};

/**
 * Structure of a statusbar item
 */
typedef struct girara_statusbar_item_s
{
  GtkLabel *text; /**< Text label */
  struct girara_statusbar_item_s *next; /**< Next statusbar item (linked list) */
} girara_statusbar_item_t;

/**
 * Definition of an argument of a shortcut or buffered command
 */
typedef struct
{
  int   n; /**< Identifier */
  void *data; /**< Data */
} girara_argument_t;

/**
 * Function declaration of a shortcut function
 *
 * If a numeric value has been written into the buffer, this function gets as
 * often executed as the value defines or until the function returns false the
 * first time.
 */
typedef bool (*girara_shortcut_function_t)(girara_session_t*, girara_argument_t*);

/**
 * Structure of a completion element
 */
typedef struct girara_completion_element_s
{
  char *value; /**> Name of the completion element */
  char *description; /**> Description of the completion element */
  struct girara_completion_element_s *next; /**> Next completion element (linked list) */
} girara_completion_element_t;

/**
 * Structure of a completion group
 */
typedef struct girara_completion_group_s
{
  char *value; /**> Name of the completion element */
  girara_completion_element_t *elements; /**> Elements of the completion group */
  struct girara_completion_group_s *next; /**> Next group (linked list) */
} girara_completion_group_t;

/**
 * Structure of a completion object
 */
typedef struct girara_completion_s
{
  girara_completion_group_t *groups; /**> Containing completion groups */
} girara_completion_t;

/**
 * Function declaration of a function that generates a completion group
 *
 * @param session The current girara session
 * @param input The current input
 * @return The completion group
 */
typedef girara_completion_t* (*girara_completion_function_t)(girara_session_t* session, char* input);

/**
 * Structure of a shortcut
 */
typedef struct girara_shortcut_s
{
  guint mask; /**< Mask */
  guint key; /**< Key */
  char* buffered_command; /**< Buffer command */
  girara_shortcut_function_t function; /**< The correspondending function */
  girara_mode_t mode; /**< Mode identifier */
  girara_argument_t argument; /**< Given argument */
  struct girara_shortcut_s *next; /**< Next shortcut (linked list) */
} girara_shortcut_t;

/**
 * Structure of a inputbar shortcut
 */
typedef struct girara_inputbar_shortcut_s
{
  guint mask; /**< Mask */
  guint key; /**< Key */
  girara_shortcut_function_t function; /**< Function */
  girara_argument_t argument; /**< Given argument */
  struct girara_inputbar_shortcut_s *next; /**< Next inputbar shortcut (linked list) */
} girara_inputbar_shortcut_t;

/**
 * Function declaration of a inputbar special function
 *
 * @param session The current girara session
 * @param input The current input
 * @param argument The given argument
 * @return TRUE No error occured
 * @return FALSE Error occured
 */
typedef bool (*girara_inputbar_special_function_t)(girara_session_t* session, char* input, girara_argument_t* argument);

/**
 * Structure of a special command
 */
typedef struct girara_special_command_s
{
  char identifier; /**< Identifier */
  girara_inputbar_special_function_t function; /**< Function */
  bool always; /**< Evalute on every change of the input */
  girara_argument_t argument; /**< Argument */
  struct girara_special_command_s *next; /**< Next special command (linked list) */
} girara_special_command_t;

/**
 * Function declaration of a command function
 *
 * @param session The current girara session
 * @param argc Number of arguments
 * @param argv Arguments
 */
typedef bool (*girara_command_function_t)(girara_session_t* session, int argc, char** argv);

/**
 * Structure of a command
 */
typedef struct girara_command_s
{
  char* command; /**< Name of the command */
  char* abbr; /**< Abbreviation of the command */
  girara_command_function_t function; /**< Function */
  girara_completion_function_t completion; /**< Completion function of the command */
  char* description; /**< Description of the command */
  struct girara_command_s *next; /**< Next command (linked list) */
} girara_command_t;

/**
 * Structure of a mouse event
 */
typedef struct girara_mouse_event_s
{
  guint mask; /**< Mask */
  guint button; /**< Button */
  girara_shortcut_function_t function; /**< Function */
  girara_mode_t mode; /**< Allowed modes */
  girara_argument_t argument; /**< Given argument */
  struct girara_mouse_event_s *next; /**< Next mouse event (linked list) */
} girara_mouse_event_t;

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
    GtkEntry        *inputbar; /**< The inputbar */
    GdkNativeWindow  embed; /**< Embedded window */
  } gtk;

  struct
  {
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
    PangoFontDescription *font; /**< The used font */
  } style;

  struct
  {
    girara_mouse_event_t* mouse_events; /**< List of mouse events */
    girara_command_t* commands; /**< List of commands */
    girara_shortcut_t* shortcuts; /**< List of shortcuts */
    girara_special_command_t* special_commands; /**< List of special commands */
    girara_inputbar_shortcut_t* inputbar_shortcuts; /**< List of inputbar shortcuts */
  } bindings;

  struct
  {
    girara_statusbar_item_t* statusbar_items; /**< List of statusbar items */
  } elements;

  /**
   * List of settings
   */
  girara_setting_t* settings;

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
    girara_mode_t current_mode; /**< Current mode */
    void* data; /**< User data */
  } global;

  struct
  {
    int n; /**< Numeric buffer */
    GString *command; /**< Command in buffer */
  } buffer;
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

/**
 * Adds an additional entry in the settings list
 *
 * @param session The used girara session
 * @param name The name of the setting
 * @param value The value of the setting
 * @param type The type of the setting
 * @param init_only Will only available on initialization
 * @param description Description of the setting
 * @param callback Function that is called when the setting changes
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_setting_add(girara_session_t* session, char* name, void* value, girara_setting_type_t type, bool init_only, char* description, girara_setting_callback_t callback);

/**
 * Sets the value of a setting
 *
 * @param session The used girara session
 * @param name The name of the setting
 * @param value The new value of the setting
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_setting_set(girara_session_t* session, char* name, void* value);

/**
 * Retreives the value of a setting
 *
 * @param session The used girara session
 * @param name The name of the setting
 * @return Value of the setting
 * @return NULL An error occured
 */
void* girara_setting_get(girara_session_t* session, char* name);

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
 * Creates an girara completion object
 *
 * @return Completion object
 * @return NULL An error occured
 */
girara_completion_t* girara_completion_init();

/**
 * Creates an girara completion group
 *
 * @return Completion object
 * @return NULL An error occured
 */
girara_completion_group_t* girara_completion_group_create(girara_session_t* session, char* name);

/**
 * Adds an group to a completion object
 *
 * @param completion The completion object
 * @param group The completion group
 */
void girara_completion_add_group(girara_completion_t* completion, girara_completion_group_t* group);

/**
 * Frees an completion and all of its groups and elements
 *
 * @param completion The completion
 */
void girara_completion_free(girara_completion_t* completion);

/**
 * Adds an element to a completion group
 *
 * @param group The completion group
 * @param value Value of the entry
 * @param description Description of the entry
 */
void girara_completion_group_add_element(girara_completion_group_t* group, char* value, char* description);

/**
 * Default shortcut function to focus the inputbar
 *
 * @param session The used girara session
 * @param argument The argument
 */
bool girara_sc_focus_inputbar(girara_session_t* session, girara_argument_t* argument);

/**
 * Default shortcut function to quit the application
 *
 * @param session The used girara session
 * @param argument The argument
 */
bool girara_sc_quit(girara_session_t* session, girara_argument_t* argument);

/**
 * Default command to map sortcuts
 *
 * @param session The used girara session
 * @param argc Number of arguments
 * @param argv Arguments
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_cmd_map(girara_session_t* session, int argc, char** argv);

/**
 * Default command to quit the application
 *
 * @param session The used girara session
 * @param argc Number of arguments
 * @param argv Arguments
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_cmd_quit(girara_session_t* session, int argc, char** argv);

/**
 * Default command to set the value of settings
 *
 * @param session The used girara session
 * @param argc Number of arguments
 * @param argv Arguments
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_cmd_set(girara_session_t* session, int argc, char** argv);

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
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_isc_abort(girara_session_t* session, girara_argument_t* argument);

/**
 * Default inputbar shortcut that completes the given input
 * in the statusbar
 *
 * @param session The used girara session
 * @param argument The argument
 */
bool girara_isc_completion(girara_session_t* session, girara_argument_t* argument);

/**
 * Default inputbar shortcut to manipulate the inputbar string
 *
 * @param session The used girara session
 * @param argument The argument
 */
bool girara_isc_string_manipulation(girara_session_t* session, girara_argument_t* argument);

/**
 * Default complection function for the settings
 *
 * @param session The used girara session
 * @param input The current input
 */
girara_completion_t* girara_cc_set(girara_session_t* session, char* input);

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

#include "girara-utils.h"
#include "girara-datastructures.h"

#endif
