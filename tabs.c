/* See LICENSE file for license and copyright information */

#include "tabs.h"
#include "session.h"
#include "internal.h"

#define UNTITLED_TAB_TITLE "untitled"

/**
 * Default implementation of the event that is executed if a tab is clicked
 *
 * @param widget The widget
 * @param event The event
 * @param data Additional data
 * @return true if an error occurred, otherwise false
 */
static bool girara_callback_tab_clicked(GtkWidget* widget, GdkEventButton* event, gpointer data);

void
girara_tabs_enable(girara_session_t* session)
{
  if (session == NULL || session->gtk.tabs == NULL) {
    return;
  }

  /* Display tab view */
  girara_set_view(session, GTK_WIDGET(session->gtk.tabs));

  /* Display tab bar */
  if (session->gtk.tabbar) {
    gtk_widget_show(session->gtk.tabbar);
  }
}

girara_tab_t*
girara_tab_new(girara_session_t* session, const char* title, GtkWidget* widget,
    bool next_to_current, void* data)
{
  if (session == NULL || widget == NULL) {
    return NULL;
  }

  girara_tab_t* tab = g_slice_new(girara_tab_t);

  tab->title   = title ? g_strdup(title) : g_strdup(UNTITLED_TAB_TITLE);
  tab->widget  = widget;
  tab->session = session;
  tab->data    = data;

  int position = (next_to_current) ?
    (gtk_notebook_get_current_page(session->gtk.tabs) + 1) : -1;

  /* insert tab into notebook */
  if (gtk_notebook_insert_page(session->gtk.tabs, tab->widget, NULL, position) == -1) {
    g_free(tab->title);
    g_slice_free(girara_tab_t, tab);
    return NULL;
  }

  /* create tab label */
  GtkWidget *tab_label = gtk_label_new(tab->title);
  GtkWidget *tab_event = gtk_event_box_new();

  g_object_set_data(G_OBJECT(tab->widget), "event", (gpointer) tab_event);
  g_object_set_data(G_OBJECT(tab->widget), "label", (gpointer) tab_label);
  g_object_set_data(G_OBJECT(tab->widget), "tab",   (gpointer) tab);

  g_signal_connect(G_OBJECT(tab_event), "button_press_event",
      G_CALLBACK(girara_callback_tab_clicked), tab);

  gtk_misc_set_alignment(GTK_MISC(tab_label), 0.0f, 0.0f);
  widget_add_class(tab_label, "tab");
  widget_add_class(tab_event, "tab");
  gtk_label_set_ellipsize(GTK_LABEL(tab_label), PANGO_ELLIPSIZE_MIDDLE);

  gtk_container_add(GTK_CONTAINER(tab_event), tab_label);
  gtk_box_pack_start(GTK_BOX(session->gtk.tabbar), tab_event, TRUE, TRUE, 0);
  gtk_box_reorder_child(GTK_BOX(session->gtk.tabbar), tab_event, position);

  gtk_widget_show_all(widget);
  gtk_widget_show_all(tab_event);

  gtk_notebook_set_current_page(session->gtk.tabs, position);

  girara_tab_update(session);

  return tab;
}

void
girara_tab_remove(girara_session_t* session, girara_tab_t* tab)
{
  if (session == NULL || tab == NULL || session->gtk.tabbar == NULL) {
    return;
  }

  /* Remove page from notebook */
  int tab_id = girara_tab_position_get(session, tab);

  /* Remove entry from tabbar */
  GtkWidget* tab_event = GTK_WIDGET(g_object_get_data(G_OBJECT(tab->widget), "event"));

  if (tab_event != NULL) {
    gtk_container_remove(GTK_CONTAINER(session->gtk.tabbar), tab_event);
  }

  if (tab_id != -1) {
    gtk_notebook_remove_page(session->gtk.tabs, tab_id);
  }

  g_free(tab->title);
  g_slice_free(girara_tab_t, tab);

  girara_tab_update(session);
}

girara_tab_t*
girara_tab_get(girara_session_t* session, unsigned int index)
{
  if (session == NULL || session->gtk.tabs == NULL) {
    return 0;
  }

  GtkWidget* widget = gtk_notebook_get_nth_page(session->gtk.tabs, index);

  return (girara_tab_t*) g_object_get_data(G_OBJECT(widget), "tab");
}

int
girara_get_number_of_tabs(girara_session_t* session)
{
  if (session == NULL || session->gtk.tabs == NULL) {
    return 0;
  }

  return gtk_notebook_get_n_pages(session->gtk.tabs);
}

void
girara_tab_update(girara_session_t* session)
{
  if (session == NULL || session->gtk.tabs == NULL) {
    return;
  }

  int number_of_tabs = girara_get_number_of_tabs(session);
  int current_tab    = girara_tab_position_get(session, girara_tab_current_get(session));

  for (int i = 0; i < number_of_tabs; i++) {
    GtkWidget* widget = gtk_notebook_get_nth_page(session->gtk.tabs, i);
    girara_tab_t* tab = (girara_tab_t*) g_object_get_data(G_OBJECT(widget), "tab");

    if (tab == NULL) {
      continue;
    }

    GtkWidget* tab_event = GTK_WIDGET(g_object_get_data(G_OBJECT(tab->widget), "event"));
    GtkWidget* tab_label = GTK_WIDGET(g_object_get_data(G_OBJECT(tab->widget), "label"));

    if (i == current_tab) {
      gtk_widget_set_state_flags(tab_event, GTK_STATE_FLAG_SELECTED, false);
      gtk_widget_set_state_flags(tab_label, GTK_STATE_FLAG_SELECTED, false);
    } else {
      gtk_widget_unset_state_flags(tab_event, GTK_STATE_FLAG_SELECTED);
      gtk_widget_unset_state_flags(tab_label, GTK_STATE_FLAG_SELECTED);
    }
  }
}

girara_tab_t*
girara_tab_current_get(girara_session_t* session)
{
  if (session == NULL || session->gtk.tabs == NULL) {
    return NULL;
  }

  int current = gtk_notebook_get_current_page(session->gtk.tabs);

  if (current != -1) {
    GtkWidget* widget = gtk_notebook_get_nth_page(session->gtk.tabs, current);
    return (girara_tab_t*) g_object_get_data(G_OBJECT(widget), "tab");
  } else {
    return NULL;
  }
}

void
girara_tab_current_set(girara_session_t* session, girara_tab_t* tab)
{
  if (session == NULL || session->gtk.tabs == NULL
      || tab == NULL || tab->widget == NULL) {
    return;
  }

  int index = gtk_notebook_page_num(session->gtk.tabs, tab->widget);

  if (index != -1) {
    gtk_notebook_set_current_page(session->gtk.tabs, index);
  }

  girara_tab_update(session);
}

void
girara_tab_title_set(girara_tab_t* tab, const char* title)
{
  if (tab == NULL) {
    return;
  }

  g_free(tab->title);
  tab->title = title ? g_strdup(title) : g_strdup(UNTITLED_TAB_TITLE);

  GtkWidget* tab_label = GTK_WIDGET(g_object_get_data(G_OBJECT(tab->widget), "label"));
  if (tab_label) {
    gtk_label_set_text(GTK_LABEL(tab_label), tab->title);
  }
}

const char*
girara_tab_title_get(girara_tab_t* tab)
{
  if (tab == NULL) {
    return NULL;
  }

  return tab->title;
}

int
girara_tab_position_get(girara_session_t* session, girara_tab_t* tab)
{
  if (session == NULL || session->gtk.tabs == NULL
      || tab == NULL || tab->widget == NULL) {
    return -1;
  }

  return gtk_notebook_page_num(session->gtk.tabs, tab->widget);
}

void
girara_tab_position_set(girara_session_t* session, girara_tab_t* tab, unsigned int position)
{
  if (session == NULL || session->gtk.tabs == NULL
      || tab == NULL || tab->widget == NULL) {
    return;
  }

  gtk_notebook_reorder_child(session->gtk.tabs, tab->widget, position);
}

static bool
girara_callback_tab_clicked(GtkWidget* UNUSED(widget), GdkEventButton* event, gpointer data)
{
  if (data == NULL) {
    return false;
  }

  girara_tab_t* tab         = (girara_tab_t*) data;
  girara_session_t* session = tab->session;

  switch (event->button) {
    case 1:
      girara_tab_current_set(session, tab);
      break;
    case 2:
      girara_tab_remove(session, tab);
      break;
  }

  return true;
}
