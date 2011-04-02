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

    /* trim and clean line from multiple white spaces */
    girara_clean_line(line);

    /* split line into tokens */
    gchar **tokens = g_strsplit(line, " ", -1);
    int n_tokens   = g_strv_length(tokens);

    /* search for handle */
    girara_config_handle_t* handle = session->config.handles;
    while (handle) {
      if (strcmp(handle->identifier, tokens[0]) == 0) {
        handle->handle(session, n_tokens - 1, tokens + 1);
        break;
      }

      handle = handle->next;
    }

    if (handle == NULL) {
      girara_warning("Could not process line %d in '%s': Unknown handle '%s'", line, path, tokens[0]);
    }

    line_number++;

    g_strfreev(tokens);
    free(line);
  }
}
