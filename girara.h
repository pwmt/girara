#ifndef GIRARA_H
#define GIRARA_H

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

typedef enum girara_completion_arguments_e
{
  GIRARA_HIDE = 1,
  GIRARA_NEXT,
  GIRARA_PREVIOUS,
  GIRARA_NEXT_GROUP,
  GIRARA_PREVIOUS_GROUP,
  GIRARA_HIGHLIGHT,
  GIRARA_NORMAL,
  GIRARA_DELETE_LAST_WORD,
  GIRARA_DELETE_LAST_CHAR,
  GIRARA_NEXT_CHAR,
  GIRARA_PREVIOUS_CHAR,
  GIRARA_DELETE_TO_LINE_START
} girara_completion_argument_t;

typedef enum girara_setting_type_e
{
  BOOLEAN,
  FLOAT,
  INT,
  STRING
} girara_setting_type_t;

typedef int girara_mode_t;

typedef struct girara_session_s girara_session_t;

typedef struct girara_setting_s girara_setting_t;

typedef gboolean (*girara_statusbar_event_t)(GtkWidget* widget, GdkEvent* event, girara_session_t* session);

typedef int (*girara_setting_callback_t)(girara_session_t* session, girara_setting_t* setting);

struct girara_setting_s
{
  char* name;
  union
  {
    gboolean b;
    int i;
    float f;
    char *s;
  } value;
  int   type;
  gboolean init_only;
  char* description;
  girara_setting_callback_t callback;
  struct girara_setting_s *next;
};

typedef struct girara_statusbar_item_s
{
  GtkLabel *text;
  struct girara_statusbar_item_s *next;
} girara_statusbar_item_t;

typedef struct
{
  int   n;
  void *data;
} girara_argument_t;

typedef void (*girara_shortcut_function_t)(girara_session_t*, girara_argument_t*);

typedef struct girara_completion_element_s
{
  char *value;
  char *description;
  struct girara_completion_element_s *next;
} girara_completion_element_t;

typedef struct girara_completion_group_s
{
  char *value;
  girara_completion_element_t *elements;
  struct girara_completion_group_s *next;
} girara_completion_group_t;

typedef struct girara_completion_s
{
  girara_completion_group_t *groups;
} girara_completion_t;

typedef girara_completion_t* (*girara_completion_function_t)(girara_session_t*, char*);

typedef struct girara_shortcut_s
{
  int mask;
  int key;
  char* buffered_command;
  girara_shortcut_function_t function;
  int mode;
  girara_argument_t argument;
  struct girara_shortcut_s *next;
} girara_shortcut_t;

typedef struct girara_inputbar_shortcut_s
{
  int mask;
  int key;
  girara_shortcut_function_t function;
  girara_argument_t argument;
  struct girara_inputbar_shortcut_s *next;
} girara_inputbar_shortcut_t;

typedef gboolean (*girara_inputbar_special_function_t)(girara_session_t*, char*, girara_argument_t*);

typedef struct girara_special_command_s
{
  char identifier;
  girara_inputbar_special_function_t function;
  gboolean always;
  girara_argument_t argument;
  struct girara_special_command_s *next;
} girara_special_command_t;

typedef gboolean (*girara_command_function_t)(girara_session_t*, int, char**);

typedef struct girara_command_s
{
  char* command;
  char* abbr;
  girara_command_function_t function;
  girara_completion_function_t completion;
  char* description;
  struct girara_command_s *next;
} girara_command_t;

typedef struct girara_mouse_event_s
{
  int mask;
  int button;
  girara_shortcut_function_t function;
  int mode;
  girara_argument_t argument;
  struct girara_mouse_event_s *next;
} girara_mouse_event_t;

struct girara_session_s
{
  struct
  {
    GtkWidget       *window;
    GtkBox          *box;
    GtkWidget       *view;
    GtkWidget       *statusbar;
    GtkBox          *statusbar_entries;
    GtkEntry        *inputbar;
    GdkNativeWindow  embed;
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

  struct
  {
    girara_setting_t* settings;

    char* font;
    char* default_background;
    char* default_foreground;
    char* inputbar_background;
    char* inputbar_foreground;
    char* statusbar_background;
    char* statusbar_foreground;
    char* completion_foreground;
    char* completion_background;
    char* completion_group_foreground;
    char* completion_group_background;
    char* completion_highlight_foreground;
    char* completion_highlight_background;
    char* notification_error_background;
    char* notification_error_foreground;
    char* notification_warning_background;
    char* notification_warning_foreground;
    int   height;
    int   width;
    int   n_completion_items;
  } settings;

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

gboolean girara_shortcut_add(girara_session_t* session, int modifier, int key, char* buffer, girara_shortcut_function_t function, girara_mode_t mode, int argument_n, void* argument_data);
gboolean girara_inputbar_command_add(girara_session_t* session, char* command , char* abbreviation, girara_command_function_t function, girara_completion_function_t completion, char* description);
gboolean girara_inputbar_shortcut_add(girara_session_t* session, int modifier, int key, girara_shortcut_function_t function, int argument_n, void* argument_data);
gboolean girara_special_command_add(girara_session_t* session, char identifier, girara_inputbar_special_function_t function, gboolean always, int argument_n, void* argument_data);
gboolean girara_mouse_event_add(girara_session_t* session, int mask, int button, girara_shortcut_function_t function, girara_mode_t mode, int argument_n, void* argument_data);

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
