/* See LICENSE file for license and copyright information */

#include <math.h>
#include <string.h>

#include "girara.h"
#include "girara-internal.h"

/* header functions implementation */
GtkEventBox* girara_completion_row_create(girara_session_t*, char*, char*, bool);
void girara_completion_row_set_color(girara_session_t*, GtkEventBox*, int);

/* completion */
struct girara_internal_completion_entry_s
{
  bool group; /**< The entry is a group */
  char* value; /**< Name of the entry */
  GtkEventBox* widget; /**< Eventbox widget */
};

typedef struct girara_internal_completion_entry_s girara_internal_completion_entry_t;


girara_completion_t*
girara_completion_init()
{
  girara_completion_t *completion = g_slice_new(girara_completion_t);
  completion->groups = NULL;

  return completion;
}

girara_completion_group_t*
girara_completion_group_create(girara_session_t* UNUSED(session), char* name)
{
  girara_completion_group_t* group = g_slice_new(girara_completion_group_t);

  group->value    = name ? g_strdup(name) : NULL;
  group->elements = NULL;
  group->next     = NULL;

  return group;
}

void
girara_completion_add_group(girara_completion_t* completion, girara_completion_group_t* group)
{
  g_return_if_fail(completion != NULL);
  g_return_if_fail(group      != NULL);

  girara_completion_group_t* cg = completion->groups;

  while (cg && cg->next) {
    cg = cg->next;
  }

  if (cg) {
    cg->next = group;
  } else {
    completion->groups = group;
  }
}

void
girara_completion_free(girara_completion_t* completion)
{
  g_return_if_fail(completion != NULL);

  girara_completion_group_t* group = completion->groups;
  girara_completion_element_t *element;

  while (group) {
    element = group->elements;

    while (element) {
      girara_completion_element_t* ne = element->next;

      /* free element */
      g_free(element->value);
      if (element->description) {
        g_free(element->description);
      }
      g_slice_free(girara_completion_element_t,  element);

      element = ne;
    }

    /* free group */
    girara_completion_group_t *ng = group->next;
    if (group->value) {
      g_free(group->value);
    }
    g_slice_free(girara_completion_group_t, group);

    group = ng;
  }

  /* free completion */
  g_slice_free(girara_completion_t, completion);
}

void
girara_completion_group_add_element(girara_completion_group_t* group, char* name, char* description)
{
  g_return_if_fail(group   != NULL);
  g_return_if_fail(name    != NULL);

  girara_completion_element_t* el = group->elements;

  while (el && el->next) {
    el = el->next;
  }

  girara_completion_element_t* new_element = g_slice_new(girara_completion_element_t);

  new_element->value       = g_strdup(name);
  new_element->description = description ?  g_strdup(description) : NULL;
  new_element->next        = NULL;

  if (el) {
    el->next = new_element;
  } else {
    group->elements = new_element;
  }
}

bool
girara_isc_abort(girara_session_t* session, girara_argument_t* UNUSED(argument), unsigned int UNUSED(t))
{
  /* hide completion */
  girara_argument_t arg = { GIRARA_HIDE, NULL };
  girara_isc_completion(session, &arg, 0);

  /* clear inputbar */
  gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar), 0, -1);

  /* grab view */
  gtk_widget_grab_focus(GTK_WIDGET(session->gtk.view));

  return true;
}

bool
girara_isc_completion(girara_session_t* session, girara_argument_t* argument, unsigned int UNUSED(t))
{
  g_return_val_if_fail(session != NULL, false);

  /* get current text */
  gchar *input     = gtk_editable_get_chars(GTK_EDITABLE(session->gtk.inputbar), 0, -1);
  int input_length = strlen(input);

  if (input_length == 0 || input[0] != ':') {
    g_free(input);
    return false;
  }

  gchar **elements = g_strsplit(input + 1, " ", 2);
  int n_parameter  =  g_strv_length(elements);
  g_free(input);

  /* get current values */
  gchar *current_command   = (elements[0] != NULL && elements[0][0] != '\0') ? g_strdup(elements[0]) : NULL;
  gchar *current_parameter = (elements[0] != NULL && elements[1] != NULL)    ? g_strdup(elements[1]) : NULL;

  unsigned int current_command_length = current_command ? strlen(current_command) : 0;

  /* create result box */
  static GtkBox* results          = NULL;
  static GList* entries           = NULL;
  static GList* entries_current   = NULL;
  static char *previous_command   = NULL;
  static char *previous_parameter = NULL;
  static bool command_mode        = true;
  static int   previous_length    = 0;

  /* delete old list iff
   *   the completion should be hidden
   *   the current command differs from the previous one
   *   the current parameter differs from the previous one
   *   no current command is given
   */
  if ( (argument->n == GIRARA_HIDE) ||
      (current_parameter && previous_parameter && strcmp(current_parameter, previous_parameter)) ||
      (current_command && previous_command && strcmp(current_command, previous_command)) ||
      input_length != previous_length
    )
  {
    if (results) {
      /* destroy elements */
      for (GList* element = entries; element; element = g_list_next(element)) {
        girara_internal_completion_entry_t* entry = (girara_internal_completion_entry_t*) element->data;

        if (entry) {
          gtk_widget_destroy(GTK_WIDGET(entry->widget));
          g_free(entry->value);
          g_slice_free(girara_internal_completion_entry_t, entry);
        }
      }

      g_list_free(entries);
      entries         = NULL;
      entries_current = NULL;

      /* delete row box */
      gtk_widget_destroy(GTK_WIDGET(results));
      results = NULL;
    }

    command_mode = TRUE;

    if (argument->n == GIRARA_HIDE) {
      g_free(previous_command);
      previous_command = NULL;

      g_free(previous_parameter);
      previous_parameter = NULL;

      g_strfreev(elements);

      g_free(current_command);
      g_free(current_parameter);

      return false;
    }
  }

  /* create new list iff
   *  there is no current list
   */
  if (!results) {
    results = GTK_BOX(gtk_vbox_new(FALSE, 0));

    if (!results) {
      g_free(current_command);
      g_free(current_parameter);

      g_strfreev(elements);
      return false;
    }

    /* based on parameters */
    if (n_parameter > 1) {

      /* search matching command */
      girara_command_t* command = NULL;
      for (command = session->bindings.commands; command != NULL; command = command->next) {
        if ( (command->command && !strncmp(current_command, command->command, current_command_length)) ||
            (command->abbr    && !strncmp(current_command, command->abbr,    current_command_length))
          )
        {
          if (command->completion) {
            g_free(previous_command);
            previous_command = g_strdup(command->command);
            break;
          } else {
            g_free(current_command);
            g_free(current_parameter);

            g_strfreev(elements);
            return false;
          }
        }
      }

      if (!command) {
        g_free(current_command);
        g_free(current_parameter);

        g_strfreev(elements);
        return false;
      }

      /* generate completion result */
      girara_completion_t *result = command->completion(session, current_parameter);

      if (!result || !result->groups) {
        g_free(current_command);
        g_free(current_parameter);

        g_strfreev(elements);
        return false;
      }

      girara_completion_group_t* group     = result->groups;
      girara_completion_element_t *element = NULL;

      while (group) {
        element = group->elements;

        /* create group entry */
        if (group->value) {
          girara_internal_completion_entry_t* entry = g_slice_new(girara_internal_completion_entry_t);
          entry->group  = TRUE;
          entry->value  = g_strdup(group->value);
          entry->widget = girara_completion_row_create(session, group->value, NULL, TRUE);

          entries = g_list_append(entries, entry);

          gtk_box_pack_start(results, GTK_WIDGET(entry->widget), FALSE, FALSE, 0);
        }

        while (element) {
          girara_internal_completion_entry_t* entry = g_slice_new(girara_internal_completion_entry_t);
          entry->group  = FALSE;
          entry->value  = g_strdup(element->value);
          entry->widget = girara_completion_row_create(session, element->value, element->description, FALSE);

          entries = g_list_append(entries, entry);

          gtk_box_pack_start(results, GTK_WIDGET(entry->widget), FALSE, FALSE, 0);

          element = element->next;
        }

        group = group->next;
      }

      girara_completion_free(result);

      command_mode = FALSE;
    } else {
    /* based on commands */
      command_mode = TRUE;

      /* create command rows */
      for (girara_command_t* command = session->bindings.commands;
        command != NULL; command = command->next) {

        if (!current_command ||
            (command->command && !strncmp(current_command, command->command, current_command_length)) ||
            (command->abbr && !strncmp(current_command, command->abbr,    current_command_length))
          )
        {
          /* create entry */
          girara_internal_completion_entry_t* entry = g_slice_new(girara_internal_completion_entry_t);
          entry->group  = FALSE;
          entry->value  = g_strdup(command->command);
          entry->widget = girara_completion_row_create(session, command->command, NULL, FALSE);

          entries = g_list_append(entries, entry);

          /* show entry row */
          gtk_box_pack_start(results, GTK_WIDGET(entry->widget), FALSE, FALSE, 0);
        }
      }
    }

    if (entries) {
      entries_current = (argument->n == GIRARA_NEXT) ? g_list_last(entries) : entries;
      gtk_box_pack_start(session->gtk.box, GTK_WIDGET(results), FALSE, FALSE, 0);
      gtk_widget_show(GTK_WIDGET(results));
    }
  }

  /* update entries */
  if (entries && g_list_length(entries) > 1) {
    girara_completion_row_set_color(session, ((girara_internal_completion_entry_t *) entries_current->data)->widget, GIRARA_NORMAL);

    unsigned int n_elements = g_list_length(entries);
    bool next_group     = FALSE;

    for (unsigned int i = 0; i < n_elements; i++) {
      if (argument->n == GIRARA_NEXT || argument->n == GIRARA_NEXT_GROUP) {
        GList* entry = g_list_next(entries_current);
        if (!entry) {
          entry = g_list_first(entries);
        }

        entries_current = entry;
      } else if (argument->n == GIRARA_PREVIOUS || argument->n == GIRARA_PREVIOUS_GROUP) {
        GList* entry = g_list_previous(entries_current);
        if (!entry) {
          entry = g_list_last(entries);
        }

        entries_current = entry;
      }

      if (((girara_internal_completion_entry_t*) entries_current->data)->group) {
        if (!command_mode && (argument->n == GIRARA_NEXT_GROUP || argument->n == GIRARA_PREVIOUS_GROUP)) {
          next_group = TRUE;
        }
        continue;
      } else {
        if (!command_mode && (next_group == 0) && (argument->n == GIRARA_NEXT_GROUP || argument->n == GIRARA_PREVIOUS_GROUP)) {
          continue;
        }
        break;
      }
    }

    girara_completion_row_set_color(session, ((girara_internal_completion_entry_t *) entries_current->data)->widget, GIRARA_HIGHLIGHT);

    /* hide other items */
    int* tmp  = girara_setting_get(session, "n-completion-items");
    unsigned int n_completion_items = tmp ? *tmp : 15;
    unsigned int uh = ceil( n_completion_items / 2);
    unsigned int lh = floor(n_completion_items / 2);

    unsigned int current_item = g_list_position(entries, entries_current);

    for (unsigned int i = 0; i < n_elements; i++) {
      if (
          (i >= (current_item - lh) && (i <= current_item + uh)) ||
          (i < n_completion_items && current_item < lh) ||
          (i >= (n_elements - n_completion_items) && (current_item >= (n_elements - uh)))
        )
      {
        gtk_widget_show(GTK_WIDGET(((girara_internal_completion_entry_t*) (g_list_nth(entries, i))->data)->widget));
      } else {
        gtk_widget_hide(GTK_WIDGET(((girara_internal_completion_entry_t*) (g_list_nth(entries, i))->data)->widget));
      }
    }

    /* update text */
    char* temp;
    if (command_mode) {
      temp = g_strconcat(":", ((girara_internal_completion_entry_t *) entries_current->data)->value, NULL);
    } else {
      temp = g_strconcat(":", previous_command, " ", ((girara_internal_completion_entry_t *) entries_current->data)->value, NULL);
    }

    gtk_entry_set_text(session->gtk.inputbar, temp);
    gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar), -1);
    g_free(temp);

    /* update previous */
    g_free(previous_command);
    g_free(previous_parameter);
    previous_command   = g_strdup((command_mode) ? ((girara_internal_completion_entry_t*) entries_current->data)->value : current_command);
    previous_parameter = g_strdup((command_mode) ? current_parameter : ((girara_internal_completion_entry_t*) entries_current->data)->value);
    previous_length    = strlen(previous_command) + ((command_mode) ? (input_length - current_command_length) : (strlen(previous_parameter) + 2));
  }

  g_free(current_command);
  g_free(current_parameter);

  g_strfreev(elements);

  return false;
}

bool
girara_isc_string_manipulation(girara_session_t* session, girara_argument_t* argument, unsigned int UNUSED(t))
{
  gchar *input  = gtk_editable_get_chars(GTK_EDITABLE(session->gtk.inputbar), 0, -1);
  int    length = strlen(input);
  int pos       = gtk_editable_get_position(GTK_EDITABLE(session->gtk.inputbar));
  int i;

  switch (argument->n) {
    case GIRARA_DELETE_LAST_WORD:
      i = pos - 1;

      if (!pos) {
        return false;
      }

      /* remove trailing spaces */
      for (; i >= 0 && input[i] == ' '; i--);

      /* find the beginning of the word */
      while ((i == (pos - 1)) || ((i > 0) && (input[i] != ' ')
            && (input[i] != '/') && (input[i] != '.')
            && (input[i] != '-') && (input[i] != '=')
            && (input[i] != '&') && (input[i] != '#')
            && (input[i] != '?')
            )) {
        i--;
      }

      gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar),  i, pos);
      gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar), i);
      break;
    case GIRARA_DELETE_LAST_CHAR:
      if ((length - 1) <= 0) {
        girara_isc_abort(session, argument, 0);
      }
      gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar), pos - 1, pos);
      break;
    case GIRARA_DELETE_TO_LINE_START:
      gtk_editable_delete_text(GTK_EDITABLE(session->gtk.inputbar), 1, pos);
      break;
    case GIRARA_NEXT_CHAR:
      gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar), pos + 1);
      break;
    case GIRARA_PREVIOUS_CHAR:
      gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar), (pos == 0) ? 0 : pos - 1);
      break;
  }

  g_free(input);

  return false;
}

girara_completion_t*
girara_cc_set(girara_session_t* session, char* input)
{
  girara_completion_t* completion  = girara_completion_init();
  girara_completion_group_t* group = girara_completion_group_create(session, NULL);
  girara_completion_add_group(completion, group);

  unsigned int input_length = input ? strlen(input) : 0;

  girara_setting_t* setting = session->settings;
  while (setting && setting->next) {
    if ((setting->init_only == false) && (input_length <= strlen(setting->name)) &&
        !strncmp(input, setting->name, input_length)) {
      girara_completion_group_add_element(group, setting->name, setting->description);
    }

    setting = setting->next;
  }

  return completion;
}

GtkEventBox*
girara_completion_row_create(girara_session_t* session, char* command, char* description, bool group)
{
  GtkBox      *col = GTK_BOX(gtk_hbox_new(FALSE, 0));
  GtkEventBox *row = GTK_EVENT_BOX(gtk_event_box_new());

  GtkLabel *show_command     = GTK_LABEL(gtk_label_new(NULL));
  GtkLabel *show_description = GTK_LABEL(gtk_label_new(NULL));

  gtk_misc_set_alignment(GTK_MISC(show_command),     0.0, 0.0);
  gtk_misc_set_alignment(GTK_MISC(show_description), 0.0, 0.0);

  if (group) {
    gtk_misc_set_padding(GTK_MISC(show_command),     2.0, 4.0);
    gtk_misc_set_padding(GTK_MISC(show_description), 2.0, 4.0);
  } else {
    gtk_misc_set_padding(GTK_MISC(show_command),     1.0, 1.0);
    gtk_misc_set_padding(GTK_MISC(show_description), 1.0, 1.0);
  }

  gtk_label_set_use_markup(show_command,     TRUE);
  gtk_label_set_use_markup(show_description, TRUE);

  gchar* c = g_markup_printf_escaped(FORMAT_COMMAND,     command ? command : "");
  gchar* d = g_markup_printf_escaped(FORMAT_DESCRIPTION, description ? description : "");
  gtk_label_set_markup(show_command,     c);
  gtk_label_set_markup(show_description, d);
  g_free(c);
  g_free(d);

  if (group) {
    gtk_widget_modify_fg(GTK_WIDGET(show_command),     GTK_STATE_NORMAL, &(session->style.completion_group_foreground));
    gtk_widget_modify_fg(GTK_WIDGET(show_description), GTK_STATE_NORMAL, &(session->style.completion_group_foreground));
    gtk_widget_modify_bg(GTK_WIDGET(row),              GTK_STATE_NORMAL, &(session->style.completion_group_background));
  } else {
    gtk_widget_modify_fg(GTK_WIDGET(show_command),     GTK_STATE_NORMAL, &(session->style.completion_foreground));
    gtk_widget_modify_fg(GTK_WIDGET(show_description), GTK_STATE_NORMAL, &(session->style.completion_foreground));
    gtk_widget_modify_bg(GTK_WIDGET(row),              GTK_STATE_NORMAL, &(session->style.completion_background));
  }

  gtk_widget_modify_font(GTK_WIDGET(show_command),     session->style.font);
  gtk_widget_modify_font(GTK_WIDGET(show_description), session->style.font);

  gtk_box_pack_start(GTK_BOX(col), GTK_WIDGET(show_command),     TRUE,  TRUE,  2);
  gtk_box_pack_start(GTK_BOX(col), GTK_WIDGET(show_description), FALSE, FALSE, 2);

  gtk_container_add(GTK_CONTAINER(row), GTK_WIDGET(col));
  gtk_widget_show_all(GTK_WIDGET(row));

  return row;
}

void
girara_completion_row_set_color(girara_session_t* session, GtkEventBox* row, int mode)
{
  g_return_if_fail(session != NULL);
  g_return_if_fail(row     != NULL);

  GtkBox *col     = GTK_BOX(gtk_bin_get_child(GTK_BIN(row)));
  GList* items    = gtk_container_get_children(GTK_CONTAINER(col));
  GtkLabel *cmd   = GTK_LABEL(g_list_nth_data(items, 0));
  GtkLabel *desc  = GTK_LABEL(g_list_nth_data(items, 1));

  if (mode == GIRARA_HIGHLIGHT) {
    gtk_widget_modify_fg(GTK_WIDGET(cmd),   GTK_STATE_NORMAL, &(session->style.completion_highlight_foreground));
    gtk_widget_modify_fg(GTK_WIDGET(desc),  GTK_STATE_NORMAL, &(session->style.completion_highlight_foreground));
    gtk_widget_modify_bg(GTK_WIDGET(row),   GTK_STATE_NORMAL, &(session->style.completion_highlight_background));
  } else {
    gtk_widget_modify_fg(GTK_WIDGET(cmd),   GTK_STATE_NORMAL, &(session->style.completion_foreground));
    gtk_widget_modify_fg(GTK_WIDGET(desc),  GTK_STATE_NORMAL, &(session->style.completion_foreground));
    gtk_widget_modify_bg(GTK_WIDGET(row),   GTK_STATE_NORMAL, &(session->style.completion_background));
  }

  g_list_free(items);
}
