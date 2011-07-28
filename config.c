/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "girara.h"

#define COMMENT_PREFIX '#'

bool
girara_config_handle_add(girara_session_t* session, const char* identifier, girara_command_function_t handle)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(identifier != NULL, false);

  /* search for existing config handle */
  GIRARA_LIST_FOREACH(session->config.handles, girara_config_handle_t*, iter, data)
    if (strcmp(data->identifier, identifier) == 0) {
      data->handle = handle;
      girara_list_iterator_free(iter);
      return true;
    }
  GIRARA_LIST_FOREACH_END(session->config.handles, girara_config_handle_t*, iter, data)

  /* add new config handle */
  girara_config_handle_t* config_handle = g_slice_new(girara_config_handle_t);

  config_handle->identifier = g_strdup(identifier);
  config_handle->handle     = handle;
  girara_list_append(session->config.handles, config_handle);

  return true;
}

void
girara_config_handle_free(girara_config_handle_t* handle)
{
  if (handle == NULL) {
    return;
  }

  g_free(handle->identifier);
  g_slice_free(girara_config_handle_t, handle);
}

void
girara_config_parse(girara_session_t* session, const char* path)
{
  /* open file */
  FILE* file = girara_file_open(path, "r");

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
      free(line);
      continue;
    }

    gchar** argv = NULL;
    gint    argc = 0;

    girara_list_t* argument_list = girara_list_new();
    if (argument_list == NULL) {
      free(line);
      fclose(file);
      return;
    }

    girara_list_set_free_function(argument_list, g_free);
    if (g_shell_parse_argv(line, &argc, &argv, NULL) != FALSE) {
      for(int i = 1; i < argc; i++) {
        girara_list_append(argument_list, (void*) g_strdup(argv[i]));
      }
    } else {
      girara_list_free(argument_list);
      fclose(file);
      free(line);
      return;
    }

    /* search for config handle */
    girara_config_handle_t* handle = NULL;
    GIRARA_LIST_FOREACH(session->config.handles, girara_config_handle_t*, iter, tmp)
      handle = tmp;
      if (strcmp(handle->identifier, argv[0]) == 0) {
        handle->handle(session, argument_list);
        break;
      }
    GIRARA_LIST_FOREACH_END(session->config.handles, girara_config_handle_t*, iter, handle)

    if (handle == NULL) {
      girara_warning("Could not process line %d in '%s': Unknown handle '%s'", line_number, path, argv[0]);
    }

    line_number++;
    girara_list_free(argument_list);
    g_strfreev(argv);
    free(line);
  }

  fclose(file);
}
