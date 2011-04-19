/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "girara.h"

#define COMMENT_PREFIX '#'

bool
girara_config_handle_add(girara_session_t* session, const char* identifier, girara_command_function_t handle)
{
  g_return_val_if_fail(session  != NULL, false);

  /* search for existing config handle */
  girara_config_handle_t* config_handle_it   = session->config.handles;
  girara_config_handle_t* last_config_handle = config_handle_it;

  while (config_handle_it) {
    if (strcmp(config_handle_it->identifier, identifier) == 0) {
      config_handle_it->handle = handle;
      return true;
    }

    last_config_handle = config_handle_it;
    config_handle_it = config_handle_it->next;
  }

  /* add new config handle */
  girara_config_handle_t* config_handle = g_slice_new(girara_config_handle_t);

  config_handle->identifier = g_strdup(identifier);
  config_handle->handle     = handle;
  config_handle->next       = NULL;

  if (last_config_handle) {
    last_config_handle->next = config_handle;
  } else {
    session->config.handles = config_handle;
  }

  return true;
}

void
girara_config_parse(girara_session_t* session, const char* path)
{
  /* open file */
  FILE* file = girara_file_open(path, "r+");

  if (file == NULL) {
    girara_error("Could not open configuration file '%s'", path);
    return;
  }

  /* read lines */
  char* line = NULL;
  unsigned int line_number = 1;
  while ((line = girara_file_read_line(file)) != NULL) {
    /* skip empty lines and comments */
    if (strlen(line) == 0 || line[0] == COMMENT_PREFIX) {
      continue;
    }

    gchar** argv = NULL;
    gint    argc = 0;

    girara_list_t* argument_list = girara_list_new();
    if (argument_list == NULL) {
      return;
    }

    if (g_shell_parse_argv(line, &argc, &argv, NULL) != FALSE) {
      for(int i = 1; i < argc; i++) {
        girara_list_append(argument_list, (void*) g_strdup(argv[i]));
      }
    } else {
      girara_list_free(argument_list);
      return;
    }

    /* search for handle */
    girara_config_handle_t* handle = session->config.handles;
    while (handle) {
      if (strcmp(handle->identifier, argv[0]) == 0) {
        handle->handle(session, argument_list);
        break;
      }

      handle = handle->next;
    }

    if (handle == NULL) {
      girara_warning("Could not process line %d in '%s': Unknown handle '%s'", line_number, path, argv[0]);
    }

    line_number++;
    girara_list_free(argument_list);
    g_strfreev(argv);
    free(line);
  }
}
