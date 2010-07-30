#ifndef GIRARA_H
#define GIRARA_H

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

typedef struct girara_session_s girara_session_t;

typedef struct girara_setting_s girara_setting_t;

struct girara_setting_s
{
  char* name;
  void* variable;
  char  type;
  char* description;
  void (*callback)(girara_session_t, girara_setting_t* setting);
  struct girara_setting_s *next;
};

typedef struct
{
  int   n;
  void *data;
} girara_argument_t;

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

struct girara_shortcut_s
{
  int mask;
  int key;
  char* buffered_command;
  void (*function)(girara_argument_t*);
  int mode;
  girara_argument_t argument;
  struct girara_shortcut_s *next;
};

typedef struct girara_shortcut_s girara_shortcut_t;

struct girara_inputbar_shortcut_s
{
  int mask;
  int key;
  void (*function)(girara_argument_t*);
  girara_argument_t argument;
  struct girara_inputbar_shortcut_s *next;
};

typedef struct girara_inputbar_shortcut_s girara_inputbar_shortcut_t;

struct girara_inputbar_special_command_s
{
  char identifier;
  gboolean (*function)(char*, girara_argument_t*);
  int always;
  girara_argument_t argument;
  struct girara_inputbar_special_command_s *next;
};

typedef struct girara_inputbar_special_command_s girara_inputbar_special_command_t;

struct girara_command_s
{
  char* command;
  char* abbr;
  gboolean (*function)(int, char**);
  girara_completion_t* (*completion)(char*);
  char* description;
  struct girara_command_s *next;
};

typedef struct girara_command_s girara_command_t;

struct girara_mouse_event_s
{
  int mask;
  int button;
  void (*function)(girara_argument_t*);
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
    GtkWidget       *view;
    GtkWidget       *statusbar;
    GtkEntry        *inputbar;
    GdkNativeWindow  embed;
    char            *winid;
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
};

#endif
