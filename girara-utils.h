/* See LICENSE file for license and copyright information */

#ifndef GIRARA_UTILS_H
#define GIRARA_UTILS_H

#include <gtk/gtk.h>

/**
 * Enum for directories specified in the XDG specification.
 */
typedef enum {
  XDG_CONFIG,
  XDG_DATA
} girara_xdg_path_t;

/**
 * Returns the home directory for the given user. $HOME is prefered over the
 * value from g_get_home_dir.
 *
 * @param user a username or NULL to get home directory of the current user.
 * @return a string containing the path to the user's home directory (needs to
 * be freed with g_free) or NULL if the user doesn't exist.
 */
gchar* girara_get_home_directory(const gchar* user);

/**
 * Returns a specific path specified in the XDG specification.
 * @param path which path to get
 * @return a string containing the requested patch (needs to be freed with
 * g_free) or NULL for invalid values.
 */
gchar* girara_get_xdg_path(girara_xdg_path_t path);

/**
 * Returns a "fixed" version of path. Which means, it will be an absolute path
 * and fully expanded. ~ and ~user will be replaced by the current user's home
 * directory (user's home directory respectively).
 * @param path the path to "fix".
 * @return the "fixed" path (needs to be freed with g_free).
 */
gchar* girara_fix_path(const gchar* path);

/**
 * Open a file in a safe way
 *
 * @param path Path of the file
 * @param mode Mode that the file should be opened
 * @return NULL if an error occured
 */
FILE* girara_file_open(const char* path, char* mode);

/**
 * Reads a line from the file. Returned string has to be freed.
 *
 * @param file The file stream
 * @return Read line or NULL if an error occured
 */
char* girara_file_read_line(FILE* file);

/**
 * Trims and cleans a line from multiple whitespaces
 *
 * @param line
 */
void girara_clean_line(char* line);

/**
 * Debug levels
 */
typedef enum girara_debug_level_e
{
  GIRARA_INFO, /**> Information debug output */
  GIRARA_WARNING, /**> Warning level */
  GIRARA_DEBUG, /**> Debug messages */
  GIRARA_ERROR /**> Error */
} girara_debug_level_t;

/**
 * Prints a debug message
 *
 * @param function Name of the function
 * @param line Line number
 * @param level Debug level
 * @param format Debug message
 * @param ... Additional parameters
 */
#define girara_debug(format, ...)   _girara_debug(__FUNCTION__, __LINE__, GIRARA_DEBUG,   format, __VA_ARGS__)
#define girara_info(format, ...)    _girara_debug(__FUNCTION__, __LINE__, GIRARA_INFO,    format, __VA_ARGS__)
#define girara_warning(format, ...) _girara_debug(__FUNCTION__, __LINE__, GIRARA_WARNING, format, __VA_ARGS__)
#define girara_error(format, ...)   _girara_debug(__FUNCTION__, __LINE__, GIRARA_ERROR,   format, __VA_ARGS__)

void _girara_debug(const char* function, int line, girara_debug_level_t level, const char* format, ...);

#endif
