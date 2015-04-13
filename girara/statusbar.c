/* See LICENSE file for license and copyright information */

#include "statusbar.h"
#include "session.h"
#include "datastructures.h"
#include "internal.h"
#include "settings.h"

girara_statusbar_item_t*
girara_statusbar_item_add(girara_session_t* session, bool expand, bool fill, bool left, girara_statusbar_event_t callback)
{
  g_return_val_if_fail(session != NULL && session->elements.statusbar_items, FALSE);

  girara_statusbar_item_t* item = g_slice_new(girara_statusbar_item_t);

  item->box  = gtk_event_box_new();
  item->text = GTK_LABEL(gtk_label_new(NULL));

  /* set style */
  widget_add_class(GTK_WIDGET(item->box), "statusbar");
  widget_add_class(GTK_WIDGET(item->text), "statusbar");

  /* set properties */
  gtk_widget_set_halign(GTK_WIDGET(item->text), left ? GTK_ALIGN_START : GTK_ALIGN_END);
  gtk_widget_set_valign(GTK_WIDGET(item->text), GTK_ALIGN_CENTER);
  gtk_label_set_use_markup(item->text, TRUE);

  /* add ellipsis if item is on the left side */
  if (left == true) {
    gtk_label_set_ellipsize(item->text, PANGO_ELLIPSIZE_END);
  }

  /* add name so it uses a custom style */
  gtk_widget_set_name(GTK_WIDGET(item->text), "bottom_box");

  if (callback != NULL) {
    g_signal_connect(G_OBJECT(item->box), "button-press-event", G_CALLBACK(callback), session);
  }

  /* add it to the list */
  gtk_container_add(GTK_CONTAINER(item->box), GTK_WIDGET(item->text));
  gtk_box_pack_start(session->gtk.statusbar_entries, GTK_WIDGET(item->box), expand, fill, 0);
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
