/* See LICENSE file for license and copyright information */

#ifndef GIRARA_SETTINGS_H
#define GIRARA_SETTINGS_H

#include "types.h"
#include "macros.h"

/**
 * Adds an additional entry in the settings list
 *
 * @param session The used girara session
 * @param name The name of the setting
 * @param value The value of the setting
 * @param type The type of the setting
 * @param init_only Will only available on initialization
 * @param description Description of the setting
 * @param callback Function that is called when the setting changes
 * @param data Arbitary data that can be used by callbacks
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_setting_add(girara_session_t* session, const char* name,
    void* value, girara_setting_type_t type, bool init_only,
    const char* description, girara_setting_callback_t callback, void* data) GIRARA_DEPRECATED;

/**
 * Sets the value of a setting
 *
 * @param session The used girara session
 * @param name The name of the setting
 * @param value The new value of the setting
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_setting_set(girara_session_t* session, const char* name, void* value) GIRARA_DEPRECATED;

/**
 * Retreive the value of a setting. If the setting is a string, the value stored
 * in dest has to be deallocated with g_free.
 * @param session The used girara session
 * @param name The name of the setting
 * @param dest A pointer to the destination of the result.
 * @return true if the setting exists, false otherwise.
 */
bool girara_setting_get(girara_session_t* session, const char* name, void* dest) GIRARA_DEPRECATED;

#endif
