/* See LICENSE file for license and copyright information */

#ifndef GIRARA_UTILS_H
#define GIRARA_UTILS_H

#include <stdio.h>
#include "types.h"
#include "macros.h"

/**
 * Enum for directories specified in the XDG specification.
 */
typedef enum {
  XDG_CONFIG, /**< XDG_CONFIG_HOME */
  XDG_DATA, /**< XDG_DATA_HOME */
  XDG_CONFIG_DIRS, /**< XDG_CONFIG_DIRS */
  XDG_DATA_DIRS, /**< XDG_DATA_DIRS */
  XDG_CACHE, /**< XDG_CACHE_HOME */
} girara_xdg_path_t;

/**
 * Returns the home directory for the given user. $HOME is preferred over the
 * value from g_get_home_dir.
 *
 * @param user a username or NULL to get home directory of the current user.
 * @return a string containing the path to the user's home directory (needs to
 * be freed with g_free) or NULL if the user doesn't exist.
 */
char* girara_get_home_directory(const char* user);

/**
 * Returns a specific path specified in the XDG specification. ~ in paths will
 * not be expanded.
 * @param path which path to get
 * @return a string containing the requested patch (needs to be freed with
 * g_free) or NULL for invalid values.
 */
char* girara_get_xdg_path(girara_xdg_path_t path);

/**
 * Opens a URI with xdg-open.
 *
 * @param uri the URI to be opened.
 * @return true on success, false otherwise
 */
bool girara_xdg_open(const char* uri);

/**
 * Splits paths separated by : (as in $PATH) into a list.
 *
 * @param patharray String like $PATH to split
 * @return a list of paths and NULL on failure.
 */
girara_list_t* girara_split_path_array(const char* patharray);

/**
 * Returns a "fixed" version of path. Which means, it will be an absolute path
 * and fully expanded. ~ and ~user will be replaced by the current user's home
 * directory (user's home directory respectively).
 * @param path the path to "fix".
 * @return the "fixed" path (needs to be freed with g_free).
 */
char* girara_fix_path(const char* path);

/**
 * Open a file in a safe way
 *
 * @param path Path of the file
 * @param mode Mode that the file should be opened
 * @return NULL if an error occurred
 */
FILE* girara_file_open(const char* path, const char* mode);

/**
 * Reads a line from the file. The returned string has to be released with
 * g_free.
 *
 * @param file The file stream
 * @return Read line or NULL if an error occurred
 */
char* girara_file_read_line(FILE* file);

/**
 * Reads the whole content from a file. Returned string has to be freed.
 *
 * @param path Path to the file
 * @return Read file or NULL if an error occurred
 */
char* girara_file_read(const char* path);

/**
 * Reads the whole content from a file. Returned string has to be freed.
 *
 * @param file file to read
 * @return Read file or NULL if an error occurred
 */
char* girara_file_read2(FILE* file);

/**
 * Trims and cleans a line from multiple whitespaces
 *
 * @param line
 */
void girara_clean_line(char* line);

/**
 * Changes the size of the memory block by wrapping a realloc function call
 * In addition it frees the old memory block if realloc fails.
 *
 * @param ptr Memory space
 * @param size Number of bytes
 * @return Pointer to the allocated memory block or NULL
 */
void* girara_safe_realloc(void** ptr, size_t size) GIRARA_ALLOC_SIZE(2);

/**
 * Prints a debug message. The arguments are passed to @ref _girara_debug as
 * last argument.
 */
#define girara_debug(...)   _girara_debug(__func__, __LINE__, GIRARA_DEBUG,   __VA_ARGS__)
/**
 * Prints an info message. The arguments are passed to @ref _girara_debug as
 * last argument.
 */
#define girara_info(...)    _girara_debug(__func__, __LINE__, GIRARA_INFO,    __VA_ARGS__)
/**
 * Prints a warning message. The arguments are passed to @ref _girara_debug as
 * last argument.
 */
#define girara_warning(...) _girara_debug(__func__, __LINE__, GIRARA_WARNING, __VA_ARGS__)
/**
 * Prints an error message. The arguments are passed to @ref _girara_debug as
 * last argument.
 */
#define girara_error(...)   _girara_debug(__func__, __LINE__, GIRARA_ERROR,   __VA_ARGS__)

/**
 * Print a message.
 *
 * @param function The calling function
 * @param line The line of the call
 * @param level The debug level of the message.
 * @param format printf like format string
 */
void _girara_debug(const char* function, int line, girara_debug_level_t level,
    const char* format, ...) GIRARA_PRINTF(4, 5);

/**
 * Get the debug level.
 * @returns The debug level.
 */
girara_debug_level_t girara_get_debug_level();

/**
 * Set the debug level. Any message with a level lower than the debug level will
 * be discarded.
 * @param level The new debug level.
 */
void girara_set_debug_level(girara_debug_level_t level);

/**
 * Escape \\, \\t, ", ' and spaces in strings.
 * @param value The string to be escaped.
 * @returns The escaped string. Needs to be freed with g_free.
 */
char* girara_escape_string(const char* value);

/**
 * Replaces all occurrences of \ref old in \ref string with \ref new and returns
 * a new allocated string
 *
 * @param string The original string
 * @param old String to replace
 * @param new Replacement string
 *
 * @return new allocated string, needs to be freed with g_free
 */
char* girara_replace_substring(const char* string, const char* old, const char* new);

/**
 * Execute command from argument list
 *
 * @param session The used girara session
 * @param argument_list The argument list
 * @return true if no error occurred
 */
bool girara_exec_with_argument_list(girara_session_t* session, girara_list_t* argument_list);

#endif
