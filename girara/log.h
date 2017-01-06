/* See LICENSE file for license and copyright information */

#ifndef GIRARA_LOG_H
#define GIRARA_LOG_H

#include <glib.h>
#include <stdarg.h>

#include "types.h"
#include "macros.h"

/**
 * Prints a debug message. The arguments are passed to @ref _girara_log as
 * last argument.
 */
#define girara_debug(...)   girara_log(G_STRLOC, GIRARA_DEBUG,   __VA_ARGS__)
/**
 * Prints an info message. The arguments are passed to @ref _girara_log as
 * last argument.
 */
#define girara_info(...)    girara_log(G_STRLOC, GIRARA_INFO,    __VA_ARGS__)
/**
 * Prints a warning message. The arguments are passed to @ref _girara_log as
 * last argument.
 */
#define girara_warning(...) girara_log(G_STRLOC, GIRARA_WARNING, __VA_ARGS__)
/**
 * Prints an error message. The arguments are passed to @ref _girara_log as
 * last argument.
 */
#define girara_error(...)   girara_log(G_STRLOC, GIRARA_ERROR,   __VA_ARGS__)

/**
 * Print a message.
 *
 * @param location location of the call
 * @param level The log level of the message.
 * @param format printf like format string
 */
void girara_log(const char* location, girara_log_level_t level,
    const char* format, ...) GIRARA_PRINTF(3, 4);

/**
 * Print a message.
 *
 * @param location location of the call
 * @param level The log level of the message.
 * @param format printf like format string
 * @param ap varag list
 */
void girara_vlog(const char* location, girara_log_level_t level,
    const char* format, va_list ap);

/**
 * Get the log level.
 * @returns The log level.
 */
girara_log_level_t girara_get_log_level();

/**
 * Set the log level. Any message with a level lower than the log level will
 * be discarded.
 * @param level The new log level.
 */
void girara_set_log_level(girara_log_level_t level);

#endif
