#ifndef GIRARA_H
#define GIRARA_H

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

enum girara_setting_type_e
{
  BOOLEAN,
  FLOAT,
  INT,
  STRING
};

typedef int girara_mode_t;

typedef enum girara_setting_type_e girara_setting_type_t;

typedef struct girara_session_s girara_session_t;

typedef struct girara_setting_s girara_setting_t;

typedef gboolean (*girara_statusbar_event_t)(GtkWidget* widget, GdkEvent* event, girara_session_t* session);

struct girara_setting_s
{
  char* name;
  void* variable;
  char  type;
  gboolean init_only;
  char* description;
  void (*callback)(girara_session_t*, girara_setting_t*);
  struct girara_setting_s *next;
};

typedef int (*girara_setting_callback_t)(girara_session_t* session, girara_setting_t* setting);

typedef struct
{
  GtkLabel *text;
} girara_statusbar_item_t;

typedef struct
{
  int   n;
  void *data;
} girara_argument_t;

typedef gboolean (*girara_shortcut_function_t)(girara_session_t*, girara_argument_t*);

struct girara_completion_element_s
{
  char *value;
  char *description;
  struct girara_completion_element_s *next;
};

typedef struct girara_completion_element_s girara_completion_element_t;

struct girara_completion_s
{
  char *value;
  girara_completion_element_t *elements;
  struct girara_completion_s *next;
};

typedef struct girara_completion_s girara_completion_t;

typedef int (*girara_completion_function_t)(girara_session_t*, char*);

struct girara_shortcut_s
{
  int mask;
  int key;
  char* buffered_command;
  girara_shortcut_function_t function;
  int mode;
  girara_argument_t argument;
  struct girara_shortcut_s *next;
};

typedef struct girara_shortcut_s girara_shortcut_t;

struct girara_inputbar_shortcut_s
{
  int mask;
  int key;
  girara_shortcut_function_t function;
  girara_argument_t argument;
  struct girara_inputbar_shortcut_s *next;
};

typedef struct girara_inputbar_shortcut_s girara_inputbar_shortcut_t;

typedef gboolean (*girara_inputbar_special_function_t)(girara_session_t*, char*, girara_argument_t*);

struct girara_inputbar_special_command_s
{
  char identifier;
  girara_inputbar_special_function_t function;
  gboolean always;
  girara_argument_t argument;
  struct girara_inputbar_special_command_s *next;
};

typedef struct girara_inputbar_special_command_s girara_inputbar_special_command_t;

typedef gboolean (*girara_command_function_t)(int, char**);

struct girara_command_s
{
  char* command;
  char* abbr;
  girara_command_function_t function;
  girara_completion_function_t completion;
  char* description;
  struct girara_command_s *next;
};

typedef struct girara_command_s girara_command_t;

struct girara_mouse_event_s
{
  int mask;
  int button;
  girara_shortcut_function_t function;
  int mode;
  girara_argument_t argument;
  struct girara_mouse_event_s *next;
};

typedef struct girara_mouse_event_s girara_mouse_event_t;

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
    girara_inputbar_shortcut_t* inputbar_shortcuts;
  } bindings;

  struct
  {
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
  } settings;
};

girara_session_t* girara_session_create();
int girara_session_init(girara_session_t* session);
void girara_session_destroy(girara_session_t* session);

int girara_setting_add(girara_session_t* session, char* name, void* value, girara_setting_type_t type, gboolean init_only, char* description, girara_setting_callback_t callback);
int girara_setting_set(girara_session_t* session, char* name, void* value);

int girara_shortcut_add(girara_session_t* session, int modifier, int key, char* buffer, girara_shortcut_function_t function, girara_mode_t mode, girara_argument_t argument);
int girara_inputbar_command_add(girara_session_t* session, char* command , char* abbreviation, girara_command_function_t function, girara_completion_function_t completion, char* description);
int girara_inputbar_shortcut_add(girara_session_t* session, int modifier, int key, girara_shortcut_function_t function, girara_argument_t argument);
int girara_inputbar_special_command_add(girara_session_t* session, char identifier, girara_inputbar_special_function_t function, gboolean always, girara_argument_t argument);
int girara_mouse_event_add(girara_session_t* session, int mask, int button, girara_shortcut_function_t function, girara_mode_t mode, girara_argument_t argument);

girara_statusbar_item_t* girara_statusbar_add_item(girara_session_t* session, gboolean expand, gboolean fill, girara_statusbar_event_t callback);
int girara_statusbar_item_set_text(girara_session_t* session, girara_statusbar_item_t* item, char* text);

int girara_set_view(girara_session_t* session, GtkWidget* widget);

#endif
