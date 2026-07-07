/* SPDX-License-Identifier: Zlib */

#ifndef GIRARA_UTILS_H
#define GIRARA_UTILS_H

#include <stdbool.h>

#include "macros.h"
#include "types.h"

/**
 * Enum for directories specified in the XDG specification.
 */
typedef enum {
  XDG_CONFIG,      /**< XDG_CONFIG_HOME */
  XDG_DATA,        /**< XDG_DATA_HOME */
  XDG_CONFIG_DIRS, /**< XDG_CONFIG_DIRS */
  XDG_DATA_DIRS,   /**< XDG_DATA_DIRS */
  XDG_CACHE,       /**< XDG_CACHE_HOME */
} girara_xdg_path_t;

/**
 * Returns the home directory for the given user. $HOME is preferred over the
 * value from g_get_home_dir.
 *
 * @param user a username or NULL to get home directory of the current user.
 * @return a string containing the path to the user's home directory (needs to
 * be freed with g_free) or NULL if the user doesn't exist.
 */
char* girara_get_home_directory(const char* user) GIRARA_VISIBLE;

/**
 * Returns a specific path specified in the XDG specification. ~ in paths will
 * not be expanded.
 * @param path which path to get
 * @return a string containing the requested patch (needs to be freed with
 * g_free) or NULL for invalid values.
 */
char* girara_get_xdg_path(girara_xdg_path_t path) GIRARA_VISIBLE;

/**
 * Opens a URI with xdg-open. If xdg-open is not available, it falls back to the equivalent of gio
 * open.
 *
 * @param uri the URI to be opened.
 * @return true on success, false otherwise
 */
bool girara_xdg_open(const char* uri) GIRARA_VISIBLE;

/**
 * Opens a URI with xdg-open in a different working directory. If xdg-open is not available, it
 * falls back to the equivalent of gio open.
 *
 * @param uri the URI to be opened.
 * @param working_directory working directory
 * @return true on success, false otherwise
 */
bool girara_xdg_open_with_working_directory(const char* uri, const char* working_directory) GIRARA_VISIBLE;

/**
 * Returns a "fixed" version of path. Which means, it will be an absolute path
 * and fully expanded. ~ and ~user will be replaced by the current user's home
 * directory (user's home directory respectively).
 * @param path the path to "fix".
 * @return the "fixed" path (needs to be freed with g_free).
 */
char* girara_fix_path(const char* path) GIRARA_VISIBLE;

/**
 * Escape \\, \\t, ", ' and spaces in strings.
 * @param value The string to be escaped.
 * @returns The escaped string. Needs to be freed with g_free.
 */
char* girara_escape_string(const char* value) GIRARA_VISIBLE;

/**
 * Replaces all occurrences of old in string with new and returns
 * a new allocated string
 *
 * @param string The original string
 * @param old String to replace
 * @param new Replacement string
 *
 * @return new allocated string, needs to be freed with g_free
 */
char* girara_replace_substring(const char* string, const char* old, const char* new) GIRARA_VISIBLE;

/**
 * Return version of girara.
 *
 * @return version string
 */
const char* girara_version(void) GIRARA_VISIBLE;

#endif
