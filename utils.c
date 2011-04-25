/* See LICENSE file for license and copyright information */

#define _BSD_SOURCE
#define _XOPEN_SOURCE 500

#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "girara-utils.h"

#define BLOCK_SIZE 64

gchar*
girara_fix_path(const gchar* path)
{
  g_return_val_if_fail(path != NULL || !strlen(path), NULL);

  gchar* rpath = NULL;
  if (path[0] == '~') {
    int len = strlen(path);
    gchar* user = NULL;
    int idx = 1;

    if (len > 1 && path[1] != '/') {
      while (path[idx] && path[idx] != '/') {
        ++idx;
      }

      user = g_strndup(path + 1, idx - 1);
    }

    gchar* home_path = girara_get_home_directory(user);
    g_free(user);

    if (!home_path) {
      return g_strdup(path);
    }

    rpath = g_build_filename(home_path, path + idx, NULL);
    g_free(home_path);
  } else {
    rpath = g_strdup(path);
  }

  return rpath;
}

gchar*
girara_get_home_directory(const gchar* user)
{
  if (!user || g_strcmp0(user, g_get_user_name()) == 0) {
    const gchar* homedir = g_getenv("HOME");
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

  gchar* buffer = g_malloc0(sizeof(gchar) * bufsize);
  if (!buffer) {
    return NULL;
  }

  getpwnam_r(user, &pwd, buffer, bufsize, &result);
  if (result == NULL) {
    g_free(buffer);
    return NULL;
  }

  gchar* dir = g_strdup(pwd.pw_dir);
  g_free(buffer);
  return dir;
}

gchar*
girara_get_xdg_path(girara_xdg_path_t path)
{
  switch (path) {
    case XDG_DATA:
      return g_strdup(g_get_user_data_dir());
    case XDG_CONFIG:
      return g_strdup(g_get_user_config_dir());
  }

  return NULL;
}

FILE*
girara_file_open(const char* path, char* mode)
{
  char* fixed_path = girara_fix_path(path);

  if (fixed_path == NULL) {
    return NULL;
  }

  FILE* fp;
  if ((fp = fopen(fixed_path, mode)) == NULL) {
    g_free(fixed_path);
    return NULL;
  }

  g_free(fixed_path);

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

char*
girara_file_read_line(FILE* file)
{
  unsigned int bc = BLOCK_SIZE;
  unsigned int i  = 0;
  char* buffer    = malloc(sizeof(char) * bc);

  if (!buffer) {
    goto error_ret;
  }

  char c;
  while ((c = fgetc(file)) != EOF && c != '\n') {
    buffer[i++] = c;

    if (i == bc) {
      bc += BLOCK_SIZE;
      char* tmp = realloc(buffer, sizeof(char) * bc);

      if (!tmp) {
        goto error_free;
      }

      buffer = tmp;
    }
  }

  if (i == 0 && c == EOF) {
    goto error_free;
  }

  char* tmp = realloc(buffer, sizeof(char) * (i + 1));
  if (!tmp) {
    goto error_free;
  }

  buffer = tmp;
  buffer[i] = '\0';

  return buffer;

error_free:

  free(buffer);

error_ret:

  return NULL;
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

void
_girara_debug(const char* function, int line, girara_debug_level_t level, const char* format, ...)
{
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
