/* See LICENSE file for license and copyright information */

#define _BSD_SOURCE
#define _XOPEN_SOURCE 700
#define _FILE_OFFSET_BITS 64

#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <glib.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>

#include "utils.h"
#include "datastructures.h"

#define BLOCK_SIZE 64

char*
girara_fix_path(const char* path)
{
  if (path == NULL) {
    return NULL;
  }

  char* rpath = NULL;
  if (path[0] == '~') {
    const size_t len = strlen(path);
    char* user = NULL;
    size_t idx = 1;

    if (len > 1 && path[1] != '/') {
      while (path[idx] && path[idx] != '/') {
        ++idx;
      }

      user = g_strndup(path + 1, idx - 1);
    }

    char* home_path = girara_get_home_directory(user);
    g_free(user);

    if (home_path == NULL) {
      return g_strdup(path);
    }

    rpath = g_build_filename(home_path, path + idx, NULL);
    g_free(home_path);
  } else {
    rpath = g_strdup(path);
  }

  return rpath;
}

bool
girara_xdg_open(const char* uri)
{
  if (uri == NULL || strlen(uri) == 0) {
    return false;
  }

  GString* command = g_string_new("xdg-open ");
  char* tmp        = g_shell_quote(uri);

  g_string_append(command, tmp);
  g_free(tmp);

  GError* error = NULL;
  bool res = g_spawn_command_line_async(command->str, &error);
  if (error != NULL) {
    girara_warning("Failed to execute command: %s", error->message);
    g_error_free(error);
  }

  g_string_free(command, TRUE);
  return res;
}

char*
girara_get_home_directory(const char* user)
{
  if (user == NULL || g_strcmp0(user, g_get_user_name()) == 0) {
    const char* homedir = g_getenv("HOME");
    return g_strdup(homedir ? homedir : g_get_home_dir());
  }

  // XXX: The following code is very unportable.
  struct passwd pwd;
  struct passwd* result;
#ifdef _SC_GETPW_R_SIZE_MAX
  int bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (bufsize < 0) {
    bufsize = 4096;
  }
#else
  int bufsize = 4096;
#endif

  char* buffer = g_malloc0(sizeof(char) * bufsize);

  getpwnam_r(user, &pwd, buffer, bufsize, &result);
  if (result == NULL) {
    g_free(buffer);
    return NULL;
  }

  char* dir = g_strdup(pwd.pw_dir);
  g_free(buffer);
  return dir;
}

char*
girara_get_xdg_path(girara_xdg_path_t path)
{
  static const char* VARS[] = {
    "XDG_CONFIG_HOME",
    "XDG_DATA_HOME",
    "XDG_CONFIG_DIRS",
    "XDG_DATA_DIRS"
  };

  static const char* DEFAULTS[] = {
    "NOTUSED",
    "NOTUSED",
    "/etc/xdg",
    "/usr/local/share/:/usr/share",
  };

  switch (path) {
    case XDG_DATA:
      return g_strdup(g_get_user_data_dir());
    case XDG_CONFIG:
      return g_strdup(g_get_user_config_dir());
    case XDG_CONFIG_DIRS:
    case XDG_DATA_DIRS:
    {
      const char* tmp = g_getenv(VARS[path]);
      if (tmp == NULL || !g_strcmp0(tmp, "")) {
        return g_strdup(DEFAULTS[path]);
      }
      return g_strdup(tmp);
    }
  }

  return NULL;
}

girara_list_t*
girara_split_path_array(const char* patharray)
{
  if (patharray == NULL || !g_strcmp0(patharray, "")) {
    return NULL;
  }

  girara_list_t* res = girara_list_new2(g_free);
  char** paths = g_strsplit(patharray, ":", 0);
  for (unsigned int i = 0; paths[i] != '\0'; ++i) {
    girara_list_append(res, g_strdup(paths[i]));
  }
  g_strfreev(paths);

  return res;
}

FILE*
girara_file_open(const char* path, const char* mode)
{
  char* fixed_path = girara_fix_path(path);

  if (fixed_path == NULL || mode == NULL) {
    return NULL;
  }

  FILE* fp = fopen(fixed_path, mode);
  g_free(fixed_path);
  if (fp  == NULL) {
        return NULL;
  }

  return fp;

  /* TODO */
  /*FILE* fp;*/
  /*struct stat lstat;*/
  /*struct stat fstat;*/
  /*int fd;*/
  /*char* mode = "rb+";*/

  /*if (lstat(path, &lstat) == -1) {*/
    /*if (errno != ENOENT) {*/
      /*return NULL;*/
    /*}*/

    /*if ((fd = open(path, O_CREAT | O_EXCL | O_RDWR, 0600)) == -1) {*/
      /*return NULL;*/
    /*}*/

    /*mode = "wb";*/
  /*} else {*/
    /*if ((fd = open(path, O_RDONLY)) == -1) {*/
      /*return NULL;*/
    /*}*/

    /*if (fstat(fd, &fstat) == -1) {*/
      /*if (lstat.st_mode != fstat.st_mode ||*/
          /*lstat.st_ino  != fstat.st_ino ||*/
          /*lstat.st_dev  != fstat.st_dev) {*/
        /*close(fd);*/
        /*return NULL;*/
      /*}*/
    /*}*/

    /*ftruncate(fd, 0);*/
  /*}*/

  /*if ((fp = fdopen(fd, mode)) == NULL) {*/
    /*close(fd);*/
    /*unlink(path);*/
    /*return NULL;*/
  /*}*/

  /*return fp;*/
}

#if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)
char*
girara_file_read_line(FILE* file)
{
  if (file == NULL) {
    return NULL;
  }

  size_t size = 0;
  char* line = fgetln(file, &size);
  if (line  == NULL) {
    return NULL;
  }

  char* copy = strndup(line, size);
  if (copy == NULL) {
	  return NULL;
  }

  /* remove the trailing line deliminator */
  g_strdelimit(copy, "\n\r", '\0');

  return copy;
}
#else
char*
girara_file_read_line(FILE* file)
{
  if (file == NULL) {
    return NULL;
  }

  size_t size = 0;
  char* line = NULL;
  if (getline(&line, &size, file) == -1) {
    if (line != NULL) {
      free(line);
    }
    return NULL;
  }

  /* remove the trailing line deliminator */
  g_strdelimit(line, "\n\r", '\0');
  return line;
}
#endif

char*
girara_file_read(const char* path)
{
  if (path == NULL) {
    return NULL;
  }

  FILE* file = girara_file_open(path, "r");
  if (file == NULL) {
    return NULL;
  }

  char* content = girara_file_read2(file);
  fclose(file);
  return content;
}

char*
girara_file_read2(FILE* file)
{
  if (file == NULL) {
    return NULL;
  }

  const off_t curpos = ftello(file);
  if (curpos == -1) {
    return NULL;
  }

  fseeko(file, 0, SEEK_END);
  const off_t size = ftello(file) - curpos;
  fseeko(file, curpos, SEEK_SET);

  if (size == 0) {
    char* content = malloc(1);
    content[0] = '\0';
    return content;
  }
  /* this can happen on 32 bit systems */
  if ((uintmax_t)size >= (uintmax_t)SIZE_MAX) {
    girara_error("file is too large");
    return NULL;
  }

  char* buffer    = malloc(size + 1);
  if (!buffer) {
    return NULL;
  }

  size_t read = fread(buffer, size, 1, file);
  if (read != 1) {
    free(buffer);
    return NULL;
  }

  buffer[size] = '\0';
  return buffer;
}

void
girara_clean_line(char* line)
{
  if (line == NULL) {
    return;
  }

  unsigned int i = 0;
  unsigned int j = 0;
  bool ws_mode   = true;

  for(i = 0; i < strlen(line); i++) {
    if (isspace(line[i]) != 0) {
      if (ws_mode) {
        continue;
      }

      line[j++] = ' ';
      ws_mode = true;
    } else {
      line[j++] = line[i];
      ws_mode = false;
    }
  }

  line[j] = '\0';
}

void*
girara_safe_realloc(void** ptr, size_t size)
{
  if(ptr == NULL) {
    return NULL;
  }

  if (size == 0) {
    goto error_free;
  }

  void* tmp = realloc(*ptr, size);
  if(tmp == NULL) {
    goto error_free;
  }

  *ptr = tmp;
  return *ptr;

error_free:

  free(*ptr);
  *ptr = NULL;

  return NULL;
}

static girara_debug_level_t debug_level = GIRARA_DEBUG;

void
_girara_debug(const char* function, int line, girara_debug_level_t level, const char* format, ...)
{
  /* This could be simplified if DEBUG, INFO, WARNING, ERROR were ordered. */
  if ((debug_level == GIRARA_ERROR && level != GIRARA_ERROR) ||
      (debug_level == GIRARA_WARNING && (level != GIRARA_ERROR && level != GIRARA_WARNING)) ||
      (debug_level == GIRARA_INFO && level == GIRARA_DEBUG)) {
    return;
  }

  switch (level)
  {
    case GIRARA_WARNING:
      fprintf(stderr, "warning: ");
      break;
    case GIRARA_ERROR:
      fprintf(stderr, "error: ");
      break;
    case GIRARA_INFO:
      fprintf(stderr, "info: ");
      break;
    case GIRARA_DEBUG:
      fprintf(stderr, "debug: (%s:%d) ", function, line);
    default:
      return;
  }

  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);

  fprintf(stderr, "\n");
}

girara_debug_level_t
girara_get_debug_level()
{
  return debug_level;
}

void
girara_set_debug_level(girara_debug_level_t level)
{
  debug_level = level;
}
