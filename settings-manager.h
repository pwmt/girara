/* See LICENSE file for license and copyright information */

#ifndef GIRARA_SETTINGS_MANAGER_H
#define GIRARA_SETTINGS_MANAGER_H

#include <glib-object.h>
#include "types.h"

struct girara_settings_manager_s {
  GObject parent;
};

struct girara_settings_manager_class_s {
  GObjectClass parent_class;
};

#define GIRARA_TYPE_SETTINGS_MANAGER \
  (girara_settings_manager_get_type ())
#define GIRARA_SETTINGS_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIRARA_TYPE_SETTINGS_MANAGER, GiraraSettingsManager))
#define GIRARA_SETTINGS_MANAGER_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_CAST ((obj), GIRARA_TYPE_SETTINGS_MANAGER, GiraraSettingsManagerClass))
#define GIRARA_IS_SETTINGS_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIRARA_TYPE_SETTINGS_MANAGER))
#define GIRARA_IS_SETTINGS_MANAGER_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE ((obj), GIRARA_TYPE_SETTINGS_MANAGER))
#define GIRARA_SETTINGS_MANAGER_GET_CLASS \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIRARA_TYPE_SETTINGS_MANAGER, GiraraSettingsManagerClass))

/**
 * Returns the type of the settings manager.
 * @return the type
 */
GType girara_settings_manager_get_type(void);

/**
 * Create new session manager object.
 * @returns a session manager object
 */
GObject* girara_settings_manager_new(void);

/**
 * Adds an additional setting
 *
 * @param manager The settings manager
 * @param name The name of the setting
 * @param value The value of the setting
 * @param type The type of the setting
 * @param init_only Will only available on initialization
 * @param description Description of the setting
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_settings_manager_add(GiraraSettingsManager* manager,
    const char* name, void* value, girara_setting_type_t type,
    bool init_only, const char* description);

/**
 * Sets the value of a setting
 *
 * @param session The used girara session
 * @param name The name of the setting
 * @param value The new value of the setting
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_settings_manager_set(GiraraSettingsManager* manager,
    const char* name, void* value);

/**
 * Retreive the value of a setting. If the setting is a string, the value stored
 * in dest has to be deallocated with g_free.
 * @param session The used girara session
 * @param name The name of the setting
 * @param dest A pointer to the destination of the result.
 * @return true if the setting exists, false otherwise.
 */
bool girara_settings_manager_get(GiraraSettingsManager* manager, const char* name, void* dest);

/**
 * Get the names of all settings
 *
 * @param manager The setting manager
 * @param with_init_only Wheter to include init only settings or not
 * @return list of all names
 */
girara_list_t* girara_settings_manager_get_names(GiraraSettingsManager* manager,
    bool with_init_only);

/**
 * Get the setting's description.
 *
 * @param manager The setting manager
 * @return the setting's description
 */
const char* girara_settings_manager_get_description(GiraraSettingsManager* manager,
    const char* name);

/**
 * Get a setting's type.
 *
 * @param manager The setting manager
 * @return the type
 */
girara_setting_type_t girara_settings_manager_type(GiraraSettingsManager* manager,
    const char* name);

bool girara_settings_manager_exists(GiraraSettingsManager* manager,
    const char* name);

#endif
