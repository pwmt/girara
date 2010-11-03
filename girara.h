#ifndef GIRARA_H
#define GIRARA_H

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

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
typedef gboolean (*girara_statusbar_event_t)(GtkWidget* widget, GdkEvent* event, girara_session_t* session);

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
    gboolean b; /**< Boolean */
    int i; /**< Integer */
    float f; /**< Floating number */
    char *s; /**< String */
  } value; /**< Value of the setting */
  int type; /**< Type identifier */
  gboolean init_only; /**< Option can be set only before girara gets initialized */
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
 */
typedef void (*girara_shortcut_function_t)(girara_session_t*, girara_argument_t*);

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
 * @session The current girara session
 * @input The current input
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
  int mode; /**< Mode identifier */
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
typedef gboolean (*girara_inputbar_special_function_t)(girara_session_t* session, char* input, girara_argument_t* argument);

/**
 * Structure of a special command
 */
typedef struct girara_special_command_s
{
  char identifier; /**< Identifier */
  girara_inputbar_special_function_t function; /**< Function */
  gboolean always; /**< Evalute on every change of the input */
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
typedef gboolean (*girara_command_function_t)(girara_session_t* session, int argc, char** argv);

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
  int mode; /**< Allowed modes */
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
    GtkWidget       *statusbar; /**< The statusbar */
    GtkBox          *statusbar_entries; /**< Statusbar entry box */
    GtkEntry        *inputbar; /**< The inputbar */
    GdkNativeWindow  embed; /**< Embedded window */
  } gtk;

  struct
  {
    GdkColor default_foreground;
    GdkColor default_background;
    GdkColor inputbar_foreground;
    GdkColor inputbar_background;
    GdkColor statusbar_foreground;
    GdkColor statusbar_background;
    GdkColor completion_foreground;
    GdkColor completion_background;
    GdkColor completion_group_background;
    GdkColor completion_group_foreground;
    GdkColor completion_highlight_foreground;
    GdkColor completion_highlight_background;
    GdkColor notification_error_foreground;
    GdkColor notification_error_background;
    GdkColor notification_warning_foreground;
    GdkColor notification_warning_background;
    PangoFontDescription *font;
  } style;

  struct
  {
    girara_mouse_event_t* mouse_events;
    girara_command_t* commands;
    girara_shortcut_t* shortcuts;
    girara_special_command_t* special_commands;
    girara_inputbar_shortcut_t* inputbar_shortcuts;
  } bindings;

  struct
  {
    girara_statusbar_item_t* statusbar_items;
  } elements;

  girara_setting_t* settings;

  struct
  {
    int inputbar_activate;
    int inputbar_key_pressed;
    int view_key_pressed;
  } signals;

  struct
  {
    GString *buffer;
    int current_mode;
    int number_of_commands;
  } global;

  struct
  {
    int n;
    GString *command;
  } buffer;
};

girara_session_t* girara_session_create();
gboolean girara_session_init(girara_session_t* session);
gboolean girara_session_destroy(girara_session_t* session);

gboolean girara_setting_add(girara_session_t* session, char* name, void* value, girara_setting_type_t type, gboolean init_only, char* description, girara_setting_callback_t callback);
gboolean girara_setting_set(girara_session_t* session, char* name, void* value);
void* girara_setting_get(girara_session_t* session, char* name);

gboolean girara_shortcut_add(girara_session_t* session, guint modifier, guint key, char* buffer, girara_shortcut_function_t function, girara_mode_t mode, int argument_n, void* argument_data);
gboolean girara_inputbar_command_add(girara_session_t* session, char* command , char* abbreviation, girara_command_function_t function, girara_completion_function_t completion, char* description);
gboolean girara_inputbar_shortcut_add(girara_session_t* session, guint modifier, guint key, girara_shortcut_function_t function, int argument_n, void* argument_data);
gboolean girara_special_command_add(girara_session_t* session, char identifier, girara_inputbar_special_function_t function, gboolean always, int argument_n, void* argument_data);
gboolean girara_mouse_event_add(girara_session_t* session, guint mask, guint button, girara_shortcut_function_t function, girara_mode_t mode, int argument_n, void* argument_data);

girara_statusbar_item_t* girara_statusbar_item_add(girara_session_t* session, gboolean expand, gboolean fill, gboolean left, girara_statusbar_event_t callback);
gboolean girara_statusbar_item_set_text(girara_session_t* session, girara_statusbar_item_t* item, char* text);
gboolean girara_statusbar_item_set_foreground(girara_session_t* session, girara_statusbar_item_t* item, char* color);
gboolean girara_statusbar_set_background(girara_session_t* session, char* color);

gboolean girara_set_view(girara_session_t* session, GtkWidget* widget);

girara_completion_t* girara_completion_init();
girara_completion_group_t* girara_completion_group_create(girara_session_t*, char*);
void girara_completion_add_group(girara_completion_t*, girara_completion_group_t*);
void girara_completion_free(girara_completion_t*);
void girara_completion_group_add_element(girara_session_t*, girara_completion_group_t*, char*, char*);
void girara_isc_completion(girara_session_t*, girara_argument_t*);

/* default shortcuts */
void girara_sc_focus_inputbar(girara_session_t*, girara_argument_t*);
void girara_sc_quit(girara_session_t*, girara_argument_t*);

/* default commands */
gboolean girara_cmd_map(girara_session_t*, int, char**);
gboolean girara_cmd_quit(girara_session_t*, int, char**);
gboolean girara_cmd_set(girara_session_t*, int, char**);

/* callback declarations */
gboolean girara_callback_view_key_press_event(GtkWidget*, GdkEventKey*, girara_session_t*);
gboolean girara_callback_inputbar_activate(GtkEntry*, girara_session_t*);
gboolean girara_callback_inputbar_key_press_event(GtkWidget*, GdkEventKey*, girara_session_t*);

/* inputbar shortcuts */
void girara_isc_abort(girara_session_t* session, girara_argument_t* argument);
void girara_isc_completion(girara_session_t* session, girara_argument_t* argument);
void girara_isc_string_manipulation(girara_session_t* session, girara_argument_t* argument);

/* completion functions */
girara_completion_t* girara_cc_set(girara_session_t* session, char* input);

#endif
