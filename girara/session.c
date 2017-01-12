/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <glib/gi18n-lib.h>

#ifdef WITH_LIBNOTIFY
#include <libnotify/notify.h>
#endif

#include "session.h"

#include "callbacks.h"
#include "commands.h"
#include "config.h"
#include "css-definitions.h"
#include "datastructures.h"
#include "entry.h"
#include "input-history.h"
#include "internal.h"
#include "settings.h"
#include "shortcuts.h"
#include "template.h"
#include "utils.h"

static int
cb_sort_settings(girara_setting_t* lhs, girara_setting_t* rhs)
{
  return g_strcmp0(girara_setting_get_name(lhs), girara_setting_get_name(rhs));
}

static void
ensure_gettext_initialized(void)
{
  static gsize initialized = 0;
  if (g_once_init_enter(&initialized) == TRUE) {
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    g_once_init_leave(&initialized, 1);
  }
}

static void
init_template_engine(GiraraTemplate* csstemplate)
{
  static const char* variable_names[] = {
    "session",
    "default-fg",
    "default-bg",
    "inputbar-fg",
    "inputbar-bg",
    "statusbar-fg",
    "statusbar-bg",
    "completion-fg",
    "completion-bg",
    "completion-group-fg",
    "completion-group-bg",
    "completion-highlight-fg",
    "completion-highlight-bg",
    "notification-error-fg",
    "notification-error-bg",
    "notification-warning-fg",
    "notification-warning-bg",
    "notification-fg",
    "notification-bg",
    "scrollbar-fg",
    "scrollbar-bg",
    "tabbar-fg",
    "tabbar-bg",
    "tabbar-focus-fg",
    "tabbar-focus-bg",
    "bottombox-padding1",
    "bottombox-padding2",
    "bottombox-padding3",
    "bottombox-padding4"
  };

  for (size_t idx = 0; idx < LENGTH(variable_names); ++idx) {
    girara_template_add_variable(csstemplate, variable_names[idx]);
  }

  if (gtk_check_version(3, 20, 0) == NULL) {
    girara_template_add_variable(csstemplate, "font-family");
    girara_template_add_variable(csstemplate, "font-size");
    girara_template_add_variable(csstemplate, "font-weight");
  } else {
    girara_template_add_variable(csstemplate, "font");
  }
}

void
css_template_fill_font(GiraraTemplate* csstemplate, const char* font)
{
  if (gtk_check_version(3, 20, 0) != NULL) {
    girara_template_set_variable_value(csstemplate, "font", font);
    return;
  }

  PangoFontDescription* descr = pango_font_description_from_string(font);
  if (descr == NULL) {
    return;
  }

  girara_template_set_variable_value(csstemplate, "font-family",
      pango_font_description_get_family(descr));

  char* size = g_strdup_printf("%d%s", pango_font_description_get_size(descr) / PANGO_SCALE,
      pango_font_description_get_size_is_absolute(descr) == FALSE ? "pt" : "");
  girara_template_set_variable_value(csstemplate, "font-size", size);
  g_free(size);

  switch (pango_font_description_get_weight(descr)) {
    case PANGO_WEIGHT_THIN:
      girara_template_set_variable_value(csstemplate, "font-weight", "thin");
      break;

    case PANGO_WEIGHT_ULTRALIGHT:
      girara_template_set_variable_value(csstemplate, "font-weight", "ultralight");
      break;

    case PANGO_WEIGHT_SEMILIGHT:
      girara_template_set_variable_value(csstemplate, "font-weight", "light");
      break;

    case PANGO_WEIGHT_LIGHT:
      girara_template_set_variable_value(csstemplate, "font-weight", "light");
      break;

    case PANGO_WEIGHT_BOOK:
      girara_template_set_variable_value(csstemplate, "font-weight", "book");
      break;

    case PANGO_WEIGHT_MEDIUM:
      girara_template_set_variable_value(csstemplate, "font-weight", "medium");
      break;

    case PANGO_WEIGHT_SEMIBOLD:
      girara_template_set_variable_value(csstemplate, "font-weight", "semibold");
      break;

    case PANGO_WEIGHT_BOLD:
      girara_template_set_variable_value(csstemplate, "font-weight", "bold");
      break;

    case PANGO_WEIGHT_ULTRABOLD:
      girara_template_set_variable_value(csstemplate, "font-weight", "ultrabold");
      break;

    case PANGO_WEIGHT_HEAVY:
      girara_template_set_variable_value(csstemplate, "font-weight", "heavy");
      break;

    case PANGO_WEIGHT_ULTRAHEAVY:
      girara_template_set_variable_value(csstemplate, "font-weight", "ultraheavy");
      break;

    default:
      girara_template_set_variable_value(csstemplate, "font-weight", "normal");
      break;
  }

  pango_font_description_free(descr);
}

static void
fill_template_with_values(girara_session_t* session)
{
  GiraraTemplate* csstemplate = session->private_data->csstemplate;

  girara_template_set_variable_value(csstemplate, "session",
      session->private_data->session_name);

  char* font = NULL;
  girara_setting_get(session, "font", &font);
  if (font != NULL) {
    css_template_fill_font(csstemplate, font);
    g_free(font);
  } else {
    if (gtk_check_version(3, 20, 0) == NULL) {
      girara_template_set_variable_value(csstemplate, "font-family", "monospace");
      girara_template_set_variable_value(csstemplate, "font-size", "9pt");
      girara_template_set_variable_value(csstemplate, "font-weight", "normal");
    } else {
      girara_template_set_variable_value(csstemplate, "font", "monospace normal 9");
    }
  };

  /* parse color values */
  const char* color_settings[] = {
    "default-fg",
    "default-bg",
    "inputbar-fg",
    "inputbar-bg",
    "statusbar-fg",
    "statusbar-bg",
    "completion-fg",
    "completion-bg",
    "completion-group-fg",
    "completion-group-bg",
    "completion-highlight-fg",
    "completion-highlight-bg",
    "notification-error-fg",
    "notification-error-bg",
    "notification-warning-fg",
    "notification-warning-bg",
    "notification-fg",
    "notification-bg",
    "scrollbar-fg",
    "scrollbar-bg",
    "tabbar-fg",
    "tabbar-bg",
    "tabbar-focus-fg",
    "tabbar-focus-bg",
  };

  for (size_t i = 0; i < LENGTH(color_settings); ++i) {
    char* tmp_value = NULL;
    girara_setting_get(session, color_settings[i], &tmp_value);

    GdkRGBA color = { 0, 0, 0, 0 };
    if (tmp_value != NULL) {
      gdk_rgba_parse(&color, tmp_value);
      g_free(tmp_value);
    }

    char* colorstr = gdk_rgba_to_string(&color);
    girara_template_set_variable_value(csstemplate, color_settings[i],
        colorstr);
    g_free(colorstr);
  }

  /* we want inputbar_entry the same height as notification_text and statusbar,
    so that when inputbar_entry is hidden, the size of the bottom_box remains
    the same. We need to get rid of the builtin padding in the GtkEntry
    widget. */

  int ypadding = 2;         /* total amount of padding (top + bottom) */
  int xpadding = 8;         /* total amount of padding (left + right) */
  girara_setting_get(session, "statusbar-h-padding", &xpadding);
  girara_setting_get(session, "statusbar-v-padding", &ypadding);

  typedef struct padding_mapping_s {
    const char* identifier;
    char* value;
  } padding_mapping_t;

  const padding_mapping_t padding_mapping[] = {
    {"bottombox-padding1", g_strdup_printf("%d", ypadding - ypadding/2)},
    {"bottombox-padding2", g_strdup_printf("%d", xpadding/2)},
    {"bottombox-padding3", g_strdup_printf("%d", ypadding/2)},
    {"bottombox-padding4", g_strdup_printf("%d", xpadding - xpadding/2)},
  };

  for (size_t i = 0; i < LENGTH(padding_mapping); ++i) {
    girara_template_set_variable_value(csstemplate,
        padding_mapping[i].identifier, padding_mapping[i].value);
    g_free(padding_mapping[i].value);
  }
}

static void
css_template_changed(GiraraTemplate* csstemplate, girara_session_t* session)
{
  GtkCssProvider* old = session->private_data->gtk.cssprovider;
  char* css_data      = girara_template_evaluate(csstemplate);
  if (css_data == NULL) {
    girara_error("Error while evaluating templates.");
    return;
  }

  GtkCssProvider* provider = gtk_css_provider_new();
  GError* error            = NULL;
  if (gtk_css_provider_load_from_data(provider, css_data, -1, &error) == FALSE) {
    girara_error("Unable to load CSS: %s", error->message);
    g_free(css_data);
    g_error_free(error);
    g_object_unref(provider);
    return;
  }
  g_free(css_data);

  /* add CSS style provider */
  GdkDisplay* display = gdk_display_get_default();
  GdkScreen* screen = gdk_display_get_default_screen(display);
  gtk_style_context_add_provider_for_screen(screen,
      GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  if (old != NULL) {
    gtk_style_context_remove_provider_for_screen(screen, GTK_STYLE_PROVIDER(old));
    g_object_unref(old);

    gtk_widget_queue_draw(GTK_WIDGET(session->gtk.window));
  }
  session->private_data->gtk.cssprovider = provider;
}

void
scrolled_window_set_scrollbar_visibility(GtkScrolledWindow* window,
                                         bool               show_horizontal,
                                         bool               show_vertical)
{
#if GTK_CHECK_VERSION(3, 16, 0)
  if (gtk_check_version(3, 16, 0) == NULL) {
    GtkPolicyType hpolicy = GTK_POLICY_AUTOMATIC;
    GtkPolicyType vpolicy = GTK_POLICY_AUTOMATIC;

    if (show_horizontal == false) {
      hpolicy = GTK_POLICY_EXTERNAL;
    }
    if (show_vertical == false) {
      vpolicy = GTK_POLICY_EXTERNAL;
    }

    gtk_scrolled_window_set_policy(window, hpolicy, vpolicy);
    return;
  }
#endif

  GtkWidget* vscrollbar = gtk_scrolled_window_get_vscrollbar(window);
  GtkWidget* hscrollbar = gtk_scrolled_window_get_hscrollbar(window);

  if (vscrollbar != NULL) {
    if (show_vertical == true) {
      gtk_widget_unset_state_flags(vscrollbar, GTK_STATE_FLAG_INSENSITIVE);
    } else {
      gtk_widget_set_state_flags(vscrollbar, GTK_STATE_FLAG_INSENSITIVE, false);
    }
  }
  if (hscrollbar != NULL) {
    if (show_horizontal == true) {
      gtk_widget_unset_state_flags(hscrollbar, GTK_STATE_FLAG_INSENSITIVE);
    } else {
      gtk_widget_set_state_flags(hscrollbar, GTK_STATE_FLAG_INSENSITIVE, false);
    }
  }
}


girara_session_t*
girara_session_create()
{
  ensure_gettext_initialized();

  girara_session_t* session = g_slice_alloc0(sizeof(girara_session_t));
  session->private_data     = g_slice_alloc0(sizeof(girara_session_private_t));

  /* init values */
  session->bindings.mouse_events       = girara_list_new2(
      (girara_free_function_t) girara_mouse_event_free);
  session->bindings.commands           = girara_list_new2(
      (girara_free_function_t) girara_command_free);
  session->bindings.special_commands   = girara_list_new2(
      (girara_free_function_t) girara_special_command_free);
  session->bindings.shortcuts          = girara_list_new2(
      (girara_free_function_t) girara_shortcut_free);
  session->bindings.inputbar_shortcuts = girara_list_new2(
      (girara_free_function_t) girara_inputbar_shortcut_free);

  session->elements.statusbar_items = girara_list_new2(
      (girara_free_function_t) girara_statusbar_item_free);

  /* settings */
  session->private_data->settings = girara_sorted_list_new2(
      (girara_compare_function_t) cb_sort_settings,
      (girara_free_function_t) girara_setting_free);

  /* CSS style provider */
  if (gtk_check_version(3, 20, 0) == NULL) {
    session->private_data->csstemplate     = girara_template_new(CSS_TEMPLATE_POST_3_20);
  } else {
    session->private_data->csstemplate     = girara_template_new(CSS_TEMPLATE_PRE_3_20);
  }
  session->private_data->gtk.cssprovider = NULL;
  init_template_engine(session->private_data->csstemplate);

  /* init modes */
  session->modes.identifiers  = girara_list_new2(
      (girara_free_function_t) girara_mode_string_free);
  girara_mode_t normal_mode   = girara_mode_add(session, "normal");
  girara_mode_t inputbar_mode = girara_mode_add(session, "inputbar");
  session->modes.normal       = normal_mode;
  session->modes.current_mode = normal_mode;
  session->modes.inputbar     = inputbar_mode;

  /* config handles */
  session->config.handles           = girara_list_new2(
      (girara_free_function_t) girara_config_handle_free);
  session->config.shortcut_mappings = girara_list_new2(
      (girara_free_function_t) girara_shortcut_mapping_free);
  session->config.argument_mappings = girara_list_new2(
      (girara_free_function_t) girara_argument_mapping_free);

  /* command history */
  session->command_history = girara_input_history_new(NULL);

  /* load default values */
  girara_config_load_default(session);

  /* create widgets */
  session->gtk.box                      = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  session->private_data->gtk.overlay    = gtk_overlay_new();
  session->private_data->gtk.bottom_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  session->gtk.statusbar_entries        = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  session->gtk.tabbar                   = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  session->gtk.inputbar_box             = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  gtk_box_set_homogeneous(GTK_BOX(session->gtk.tabbar), TRUE);
  gtk_box_set_homogeneous(session->gtk.inputbar_box, TRUE);
  session->gtk.view              = gtk_scrolled_window_new(NULL, NULL);
  session->gtk.viewport          = gtk_viewport_new(NULL, NULL);
  gtk_widget_add_events(session->gtk.viewport, GDK_SCROLL_MASK);
  session->gtk.statusbar         = gtk_event_box_new();
  session->gtk.notification_area = gtk_event_box_new();
  session->gtk.notification_text = gtk_label_new(NULL);
  session->gtk.inputbar_dialog   = GTK_LABEL(gtk_label_new(NULL));
  session->gtk.inputbar_entry    = GTK_ENTRY(girara_entry_new());
  session->gtk.inputbar          = gtk_event_box_new();
  session->gtk.tabs              = GTK_NOTEBOOK(gtk_notebook_new());

  return session;
}

bool
girara_session_init(girara_session_t* session, const char* sessionname)
{
  if (session == NULL) {
    return false;
  }

  bool smooth_scroll = false;
  girara_setting_get(session, "smooth-scroll", &smooth_scroll);
  if (smooth_scroll) {
    gtk_widget_add_events(session->gtk.viewport, GDK_SMOOTH_SCROLL_MASK);
  }

  session->private_data->session_name = g_strdup(
      (sessionname == NULL) ? "girara" : sessionname);

  /* load CSS style */
  fill_template_with_values(session);
  g_signal_connect(G_OBJECT(session->private_data->csstemplate), "changed",
      G_CALLBACK(css_template_changed), session);

  /* window */
#ifdef GDK_WINDOWING_X11
  if (session->gtk.embed != 0) {
    session->gtk.window = gtk_plug_new(session->gtk.embed);
  } else {
#endif
    session->gtk.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#ifdef GDK_WINDOWING_X11
  }
#endif

  gtk_widget_set_name(session->gtk.window, session->private_data->session_name);

  /* apply CSS style */
  css_template_changed(session->private_data->csstemplate, session);

  GdkGeometry hints = {
    .base_height = 1,
    .base_width  = 1,
    .height_inc  = 0,
    .max_aspect  = 0,
    .max_height  = 0,
    .max_width   = 0,
    .min_aspect  = 0,
    .min_height  = 0,
    .min_width   = 0,
    .width_inc   = 0
  };

  gtk_window_set_geometry_hints(GTK_WINDOW(session->gtk.window), NULL, &hints, GDK_HINT_MIN_SIZE);

  /* view */
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(girara_callback_view_key_press_event), session);

  session->signals.view_button_press_event = g_signal_connect(G_OBJECT(session->gtk.view), "button-press-event",
      G_CALLBACK(girara_callback_view_button_press_event), session);

  session->signals.view_button_release_event = g_signal_connect(G_OBJECT(session->gtk.view), "button-release-event",
      G_CALLBACK(girara_callback_view_button_release_event), session);

  session->signals.view_motion_notify_event = g_signal_connect(G_OBJECT(session->gtk.view), "motion-notify-event",
      G_CALLBACK(girara_callback_view_button_motion_notify_event), session);

  session->signals.view_scroll_event = g_signal_connect(G_OBJECT(session->gtk.view), "scroll-event",
      G_CALLBACK(girara_callback_view_scroll_event), session);

  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(session->gtk.view), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  /* invisible scrollbars */
  char* guioptions = NULL;
  girara_setting_get(session, "guioptions", &guioptions);

  const bool show_hscrollbar = strchr(guioptions, 'h') != NULL;
  const bool show_vscrollbar = strchr(guioptions, 'v') != NULL;

  scrolled_window_set_scrollbar_visibility(
    GTK_SCROLLED_WINDOW(session->gtk.view), show_hscrollbar, show_vscrollbar);
  g_free(guioptions);

  /* viewport */
  gtk_container_add(GTK_CONTAINER(session->gtk.view), session->gtk.viewport);
  gtk_viewport_set_shadow_type(GTK_VIEWPORT(session->gtk.viewport), GTK_SHADOW_NONE);

  /* statusbar */
  gtk_container_add(GTK_CONTAINER(session->gtk.statusbar), GTK_WIDGET(session->gtk.statusbar_entries));

  /* notification area */
  gtk_container_add(GTK_CONTAINER(session->gtk.notification_area), session->gtk.notification_text);
  gtk_widget_set_halign(session->gtk.notification_text, GTK_ALIGN_START);
  gtk_widget_set_valign(session->gtk.notification_text, GTK_ALIGN_CENTER);
  gtk_label_set_use_markup(GTK_LABEL(session->gtk.notification_text), TRUE);

  /* inputbar */
  gtk_entry_set_has_frame(session->gtk.inputbar_entry, FALSE);
  gtk_editable_set_editable(GTK_EDITABLE(session->gtk.inputbar_entry), TRUE);

  widget_add_class(GTK_WIDGET(session->gtk.inputbar_entry), "bottom_box");
  widget_add_class(session->gtk.notification_text, "bottom_box");

  session->signals.inputbar_key_pressed = g_signal_connect(
      G_OBJECT(session->gtk.inputbar_entry),
      "key-press-event",
      G_CALLBACK(girara_callback_inputbar_key_press_event),
      session
    );

  session->signals.inputbar_changed = g_signal_connect(
      G_OBJECT(session->gtk.inputbar_entry),
      "changed",
      G_CALLBACK(girara_callback_inputbar_changed_event),
      session
    );

  session->signals.inputbar_activate = g_signal_connect(
      G_OBJECT(session->gtk.inputbar_entry),
      "activate",
      G_CALLBACK(girara_callback_inputbar_activate),
      session
    );

  gtk_box_set_homogeneous(session->gtk.inputbar_box, FALSE);
  gtk_box_set_spacing(session->gtk.inputbar_box, 5);

  /* inputbar box */
  gtk_box_pack_start(GTK_BOX(session->gtk.inputbar_box),  GTK_WIDGET(session->gtk.inputbar_dialog), FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(session->gtk.inputbar_box),  GTK_WIDGET(session->gtk.inputbar_entry),  TRUE,  TRUE,  0);
  gtk_container_add(GTK_CONTAINER(session->gtk.inputbar), GTK_WIDGET(session->gtk.inputbar_box));

  /* bottom box */
  gtk_box_set_spacing(session->private_data->gtk.bottom_box, 0);

  gtk_box_pack_end(GTK_BOX(session->private_data->gtk.bottom_box), GTK_WIDGET(session->gtk.inputbar), TRUE, TRUE, 0);
  gtk_box_pack_end(GTK_BOX(session->private_data->gtk.bottom_box), GTK_WIDGET(session->gtk.notification_area), TRUE, TRUE, 0);
  gtk_box_pack_end(GTK_BOX(session->private_data->gtk.bottom_box), GTK_WIDGET(session->gtk.statusbar), TRUE, TRUE, 0);

  /* tabs */
  gtk_notebook_set_show_border(session->gtk.tabs, FALSE);
  gtk_notebook_set_show_tabs(session->gtk.tabs,   FALSE);

  /* packing */
  gtk_box_set_spacing(session->gtk.box, 0);
  gtk_box_pack_start(session->gtk.box, GTK_WIDGET(session->gtk.tabbar), FALSE, FALSE, 0);
  gtk_box_pack_start(session->gtk.box, GTK_WIDGET(session->gtk.view),   TRUE,  TRUE, 0);

  /* box */
  gtk_container_add(GTK_CONTAINER(session->private_data->gtk.overlay), GTK_WIDGET(session->gtk.box));
  /* overlay */
  g_object_set(session->private_data->gtk.bottom_box, "halign", GTK_ALIGN_FILL, NULL);
  g_object_set(session->private_data->gtk.bottom_box, "valign", GTK_ALIGN_END, NULL);

  gtk_overlay_add_overlay(GTK_OVERLAY(session->private_data->gtk.overlay), GTK_WIDGET(session->private_data->gtk.bottom_box));
  gtk_container_add(GTK_CONTAINER(session->gtk.window), GTK_WIDGET(session->private_data->gtk.overlay));

  /* statusbar */
  widget_add_class(GTK_WIDGET(session->gtk.statusbar), "statusbar");

  /* inputbar */
  widget_add_class(GTK_WIDGET(session->gtk.inputbar_box), "inputbar");
  widget_add_class(GTK_WIDGET(session->gtk.inputbar_entry), "inputbar");
  widget_add_class(GTK_WIDGET(session->gtk.inputbar), "inputbar");
  widget_add_class(GTK_WIDGET(session->gtk.inputbar_dialog), "inputbar");

  /* notification area */
  widget_add_class(session->gtk.notification_area, "notification");
  widget_add_class(session->gtk.notification_text, "notification");

  /* set window size */
  int window_width = 0;
  int window_height = 0;
  girara_setting_get(session, "window-width", &window_width);
  girara_setting_get(session, "window-height", &window_height);

  if (window_width > 0 && window_height > 0) {
    gtk_window_set_default_size(GTK_WINDOW(session->gtk.window), window_width, window_height);
  }

  gtk_widget_show_all(GTK_WIDGET(session->gtk.window));
  gtk_widget_hide(GTK_WIDGET(session->gtk.notification_area));
  gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar_dialog));

  if (session->global.autohide_inputbar == true) {
    gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar));
  }

  if (session->global.hide_statusbar == true) {
    gtk_widget_hide(GTK_WIDGET(session->gtk.statusbar));
  }

  char* window_icon = NULL;
  girara_setting_get(session, "window-icon", &window_icon);
  if (window_icon != NULL) {
    if (strlen(window_icon) != 0) {
      girara_setting_set(session, "window-icon", window_icon);
    }
    g_free(window_icon);
  }

  gtk_widget_grab_focus(GTK_WIDGET(session->gtk.view));

  return true;
}

static void
girara_session_private_free(girara_session_private_t* session)
{
  g_return_if_fail(session != NULL);

  if (session->session_name != NULL) {
    g_free(session->session_name);
  }

  /* clean up CSS style provider */
  if (session->gtk.cssprovider != NULL) {
    g_object_unref(session->gtk.cssprovider);
  }
  session->gtk.cssprovider = NULL;
  if (session->csstemplate != NULL) {
    g_object_unref(session->csstemplate);
  }
  session->csstemplate = NULL;

  /* clean up settings */
  girara_list_free(session->settings);
  session->settings = NULL;

  g_slice_free(girara_session_private_t, session);
}

bool
girara_session_destroy(girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);

  /* clean up shortcuts */
  girara_list_free(session->bindings.shortcuts);
  session->bindings.shortcuts = NULL;

  /* clean up inputbar shortcuts */
  girara_list_free(session->bindings.inputbar_shortcuts);
  session->bindings.inputbar_shortcuts = NULL;

  /* clean up commands */
  girara_list_free(session->bindings.commands);
  session->bindings.commands = NULL;

  /* clean up special commands */
  girara_list_free(session->bindings.special_commands);
  session->bindings.special_commands = NULL;

  /* clean up mouse events */
  girara_list_free(session->bindings.mouse_events);
  session->bindings.mouse_events = NULL;

  /* clean up input histry */
  g_object_unref(session->command_history);
  session->command_history = NULL;

  /* clean up statusbar items */
  girara_list_free(session->elements.statusbar_items);
  session->elements.statusbar_items = NULL;

  /* clean up config handles */
  girara_list_free(session->config.handles);
  session->config.handles = NULL;

  /* clean up shortcut mappings */
  girara_list_free(session->config.shortcut_mappings);
  session->config.shortcut_mappings = NULL;

  /* clean up argument mappings */
  girara_list_free(session->config.argument_mappings);
  session->config.argument_mappings = NULL;

  /* clean up modes */
  girara_list_free(session->modes.identifiers);
  session->modes.identifiers = NULL;

  /* clean up buffer */
  if (session->buffer.command) {
    g_string_free(session->buffer.command, TRUE);
  }

  if (session->global.buffer) {
    g_string_free(session->global.buffer,  TRUE);
  }

  session->buffer.command = NULL;
  session->global.buffer  = NULL;

  /* clean up private data */
  girara_session_private_free(session->private_data);
  session->private_data = NULL;

  /* clean up session */
  g_slice_free(girara_session_t, session);

  return TRUE;
}

char*
girara_buffer_get(girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, NULL);

  return (session->global.buffer) ? g_strdup(session->global.buffer->str) : NULL;
}

void
girara_libnotify(girara_session_t* session, const char *summary,
    const char *body)
{
  if (session == NULL
      || summary == NULL
      || body == NULL) {
    return;
  }

#ifdef WITH_LIBNOTIFY

  const bool was_initialized = notify_is_initted();

  if (was_initialized == false) {
    notify_init(session->private_data->session_name);
  }

  NotifyNotification* libnotify_notification = NULL;
  char* icon_name = NULL;

  /* We use the NotifyNotification constructor at many branches because
   * libnotify does not have a notify_notification_set_image_from_name()
   * function, and accessing private fields is frowned upon and subject to API
   * changes.
   */
  icon_name = g_strdup(gtk_window_get_icon_name(GTK_WINDOW(session->gtk.window)));
  if (icon_name != NULL) {
    /* Icon can be loaded from theme with adequate quality for notification */
    libnotify_notification = notify_notification_new(summary, body, icon_name);
    g_free(icon_name);
  } else {
    /* Or extracted from the current window */
    GdkPixbuf* icon_pix = gtk_window_get_icon(GTK_WINDOW(session->gtk.window));
    if (icon_pix != NULL) {
      libnotify_notification = notify_notification_new(summary, body, NULL);
      notify_notification_set_image_from_pixbuf(libnotify_notification, icon_pix);
      g_object_unref(G_OBJECT(icon_pix));
    } else {
      /* Or from a default image as a last resort */
      libnotify_notification = notify_notification_new(summary, body, "info");
    }
  }

  g_return_if_fail(libnotify_notification != NULL);
  notify_notification_show(libnotify_notification, NULL);
  g_object_unref(G_OBJECT(libnotify_notification));

  if (was_initialized == false) {
    notify_uninit();
  }

#else

  girara_notify(session, GIRARA_WARNING, "Girara was compiled without libnotify support.");

#endif
}

void
girara_notify(girara_session_t* session, int level, const char* format, ...)
{
  if (session == NULL
      || session->gtk.notification_text == NULL
      || session->gtk.notification_area == NULL
      || session->gtk.inputbar == NULL
      || session->gtk.view == NULL) {
    return;
  }

  if (level == GIRARA_ERROR) {
    widget_add_class(session->gtk.notification_area, "notification-error");
    widget_add_class(session->gtk.notification_text, "notification-error");
  } else {
    widget_remove_class(session->gtk.notification_area, "notification-error");
    widget_remove_class(session->gtk.notification_text, "notification-error");
  }
  if (level == GIRARA_WARNING) {
    widget_add_class(session->gtk.notification_area, "notification-warning");
    widget_add_class(session->gtk.notification_text, "notification-warning");
  } else {
    widget_remove_class(session->gtk.notification_area, "notification-warning");
    widget_remove_class(session->gtk.notification_text, "notification-warning");
  }

  /* prepare message */
  va_list ap;
  va_start(ap, format);
  char* message = g_strdup_vprintf(format, ap);
  va_end(ap);

  gtk_label_set_markup(GTK_LABEL(session->gtk.notification_text), message);
  g_free(message);

  /* update visibility */
  gtk_widget_show(GTK_WIDGET(session->gtk.notification_area));
  gtk_widget_hide(GTK_WIDGET(session->gtk.inputbar));

  gtk_widget_grab_focus(GTK_WIDGET(session->gtk.view));
}

void
girara_dialog(girara_session_t* session, const char* dialog, bool
    invisible, girara_callback_inputbar_key_press_event_t key_press_event,
    girara_callback_inputbar_activate_t activate_event, void* data)
{
  if (session == NULL || session->gtk.inputbar == NULL
      || session->gtk.inputbar_dialog == NULL
      || session->gtk.inputbar_entry == NULL) {
    return;
  }

  gtk_widget_show(GTK_WIDGET(session->gtk.inputbar_dialog));

  /* set dialog message */
  if (dialog != NULL) {
    gtk_label_set_markup(session->gtk.inputbar_dialog, dialog);
  }

  /* set input visibility */
  if (invisible == true) {
    gtk_entry_set_visibility(session->gtk.inputbar_entry, FALSE);
  } else {
    gtk_entry_set_visibility(session->gtk.inputbar_entry, TRUE);
  }

  /* set handler */
  session->signals.inputbar_custom_activate        = activate_event;
  session->signals.inputbar_custom_key_press_event = key_press_event;
  session->signals.inputbar_custom_data            = data;

  /* focus inputbar */
  girara_sc_focus_inputbar(session, NULL, NULL, 0);
}

bool
girara_set_view(girara_session_t* session, GtkWidget* widget)
{
  g_return_val_if_fail(session != NULL, false);

  GtkWidget* child = gtk_bin_get_child(GTK_BIN(session->gtk.viewport));

  if (child != NULL) {
    g_object_ref(child);
    gtk_container_remove(GTK_CONTAINER(session->gtk.viewport), child);
  }

  gtk_container_add(GTK_CONTAINER(session->gtk.viewport), widget);
  gtk_widget_show_all(widget);
  gtk_widget_grab_focus(session->gtk.view);

  return true;
}

void
girara_mode_set(girara_session_t* session, girara_mode_t mode)
{
  g_return_if_fail(session != NULL);

  session->modes.current_mode = mode;
}

girara_mode_t
girara_mode_add(girara_session_t* session, const char* name)
{
  g_return_val_if_fail(session  != NULL, FALSE);
  g_return_val_if_fail(name != NULL && name[0] != '\0', FALSE);

  girara_mode_t last_index = 0;
  GIRARA_LIST_FOREACH(session->modes.identifiers, girara_mode_string_t*, iter, mode)
    if (mode->index > last_index) {
      last_index = mode->index;
    }
  GIRARA_LIST_FOREACH_END(session->modes.identifiers, girara_mode_string_t*, iter, mode);

  /* create new mode identifier */
  girara_mode_string_t* mode = g_slice_new(girara_mode_string_t);
  mode->index = last_index + 1;
  mode->name = g_strdup(name);
  girara_list_append(session->modes.identifiers, mode);

  return mode->index;
}

void
girara_mode_string_free(girara_mode_string_t* mode)
{
  if (mode == NULL) {
    return;
  }

  g_free(mode->name);
  g_slice_free(girara_mode_string_t, mode);
}

girara_mode_t
girara_mode_get(girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, 0);

  return session->modes.current_mode;
}

bool
girara_set_window_title(girara_session_t* session, const char* name)
{
  if (session == NULL || session->gtk.window == NULL || name == NULL) {
    return false;
  }

  gtk_window_set_title(GTK_WINDOW(session->gtk.window), name);

  return true;
}

bool
girara_set_window_icon(girara_session_t* session, const char* name)
{
  if (session == NULL || session->gtk.window == NULL || name == NULL) {
    return false;
  }

  gtk_window_set_icon_name(GTK_WINDOW(session->gtk.window), name);

  return true;
}

girara_list_t*
girara_get_command_history(girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, NULL);
  return girara_input_history_list(session->command_history);
}

GiraraTemplate*
girara_session_get_template(girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, NULL);

  return session->private_data->csstemplate;
}

void
girara_session_set_template(girara_session_t* session, GiraraTemplate *template)
{
  g_return_val_if_fail(session != NULL, NULL);

  session->private_data->csstemplate = template;
  css_template_changed(template, session);
}

