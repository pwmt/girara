/* See LICENSE file for license and copyright information */

#include "statusbar.h"
#include "session.h"
#include "datastructures.h"

#if GTK_MAJOR_VERSION == 2
#include "gtk2-compat.h"
#endif

/**
 * Structure of a statusbar item
 */
struct girara_statusbar_item_s
{
  GtkWidget* box; /**< Event box */
  GtkLabel *text; /**< Text label */
};

girara_statusbar_item_t*
girara_statusbar_item_add(girara_session_t* session, bool expand, bool fill, bool left, girara_statusbar_event_t callback)
{
  g_return_val_if_fail(session != NULL && session->elements.statusbar_items, FALSE);

  girara_statusbar_item_t* item = g_slice_new(girara_statusbar_item_t);

  item->box  = gtk_event_box_new();
  item->text = GTK_LABEL(gtk_label_new(NULL));

  /* set style */
  gtk_widget_override_background_color(GTK_WIDGET(item->box),  GTK_STATE_NORMAL, &(session->style.statusbar_background));
  gtk_widget_override_color(GTK_WIDGET(item->text),            GTK_STATE_NORMAL, &(session->style.statusbar_foreground));
  gtk_widget_override_font(GTK_WIDGET(item->text),             session->style.font);

  /* set properties */
  gtk_misc_set_alignment(GTK_MISC(item->text),     left ? 0.0 : 1.0, 0.0);
  gtk_misc_set_padding(GTK_MISC(item->text),       2, 4);
  gtk_label_set_use_markup(item->text,             TRUE);

  if (callback) {
    g_signal_connect(G_OBJECT(item->box), "button-press-event", G_CALLBACK(callback), session);
  }

  /* add it to the list */
  gtk_container_add(GTK_CONTAINER(item->box), GTK_WIDGET(item->text));
  gtk_box_pack_start(session->gtk.statusbar_entries, GTK_WIDGET(item->box), expand, fill, 2);
  gtk_widget_show_all(GTK_WIDGET(item->box));

  girara_list_prepend(session->elements.statusbar_items, item);
  return item;
}

void
girara_statusbar_item_free(girara_statusbar_item_t* item)
{
  g_slice_free(girara_statusbar_item_t, item);
}

bool
girara_statusbar_item_set_text(girara_session_t* session, girara_statusbar_item_t* item, const char* text)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(item    != NULL, false);

  char* escaped_text = g_markup_escape_text(text, -1);
  gtk_label_set_markup((GtkLabel*) item->text, escaped_text);
  g_free(escaped_text);

  return true;
}

bool
girara_statusbar_item_set_foreground(girara_session_t* session, girara_statusbar_item_t* item, const char* color)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(item    != NULL, false);

  GdkColor gdk_color;
  gdk_color_parse(color, &gdk_color);
  gtk_widget_modify_fg(GTK_WIDGET(item->text), GTK_STATE_NORMAL, &gdk_color);

  return true;
}

bool
girara_statusbar_set_background(girara_session_t* session, const char* color)
{
  g_return_val_if_fail(session != NULL, false);

  GdkRGBA gdk_color;
  gdk_rgba_parse(&gdk_color, color);
  gtk_widget_override_background_color(GTK_WIDGET(session->gtk.statusbar), GTK_STATE_NORMAL, &gdk_color);

  return true;
}

