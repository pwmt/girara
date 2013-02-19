/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <glib.h>
#include <string.h>

#include "settings.h"
#include "datastructures.h"
#include "completion.h"
#include "session.h"
#include "internal.h"

bool
girara_setting_add(girara_session_t* session, const char* name, void* value, girara_setting_type_t type, bool init_only, const char* description, girara_setting_callback_t callback, void* data)
{
  return girara_settings_manager_add(session->settings, name, value, type,
      init_only, description);
}

bool
girara_setting_set(girara_session_t* session, const char* name, void* value)
{
  return girara_settings_manager_set(session->settings, name, value);
}

bool
girara_setting_get(girara_session_t* session, const char* name, void* dest)
{
  return girara_settings_manager_get(session->settings, name, dest);
}

girara_completion_t*
girara_cc_set(girara_session_t* session, const char* input)
{
  if (input == NULL) {
    return NULL;
  }

  girara_completion_t* completion  = girara_completion_init();
  if (completion == NULL) {
    return NULL;
  }
  girara_completion_group_t* group = girara_completion_group_create(session, NULL);
  if (group == NULL) {
    girara_completion_free(completion);
    return NULL;
  }
  girara_completion_add_group(completion, group);

  unsigned int input_length = strlen(input);

  girara_list_t* settings = girara_settings_manager_get_names(session->settings, false);
  GIRARA_LIST_FOREACH(settings, const char*, iter, name)
    if ((input_length <= strlen(name)) &&
        !strncmp(input, name, input_length)) {
      girara_completion_group_add_element(group, name,
          girara_settings_manager_get_description(session->settings, name));
    }
  GIRARA_LIST_FOREACH_END(session->settings, girara_setting_t*, iter, setting);
  girara_list_free(settings);

  return completion;
}
