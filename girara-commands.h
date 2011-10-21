/* See LICENSE file for license and copyright information */

#ifndef GIRARA_COMMANDS_H
#define GIRARA_COMMANDS_H

#include "girara-types.h"
#include "girara-internal.h"

/**
 * Default command to map sortcuts
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
HIDDEN bool girara_cmd_map(girara_session_t* session, girara_list_t* argument_list);

/**
 * Default command to quit the application
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
HIDDEN bool girara_cmd_quit(girara_session_t* session, girara_list_t* argument_list);

/**
 * Default command to set the value of settings
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
HIDDEN bool girara_cmd_set(girara_session_t* session, girara_list_t* argument_list);

#endif
