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
#include <glib.h>

#include "utils.h"
#include "datastructures.h"

#define BLOCK_SIZE 64

char*
girara_fix_path(const char* path)
{
  g_return_val_if_fail(path != NULL, NULL);

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

char*
girara_get_home_directory(const char* user)
{
  if (!user || g_strcmp0(user, g_get_user_name()) == 0) {
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
girara_file_open(const char* path, char* mode)
{
  char* fixed_path = girara_fix_path(path);

  if (fixed_path == NULL) {
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

char*
girara_file_read_line(FILE* file)
{
  if (file == NULL) {
    return NULL;
  }

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
  /* i can be zero if c was '\n' because then you read empty line */
  if ( (i == 0) && (c != '\n') ) {
    goto error_free;
  }

  char* tmp = realloc(buffer, sizeof(char) * (i + 1));
  if (tmp == NULL) {
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

char*
girara_file_read_line_from_fd(int fd)
{
  unsigned int bc = BLOCK_SIZE;
  unsigned int i  = 0;
  char* buffer    = malloc(sizeof(char) * bc);

  if (buffer == NULL) {
    goto error_ret;
  }

  char c;
  while ((read(fd, &c, 1) == 1) && c != '\n') {
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

  if (i == 0 || c == EOF) {
    goto error_free;
  }

  char* tmp = realloc(buffer, sizeof(char) * (i + 1));
  if (tmp == NULL) {
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

  unsigned int bc = BLOCK_SIZE;
  unsigned int i  = 0;
  char* buffer    = malloc(sizeof(char) * bc);

  if (!buffer) {
    goto error_ret;
  }

  char c;
  while ((c = fgetc(file)) != EOF) {
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

  fclose(file);

  return buffer;

error_free:

  free(buffer);

error_ret:

  fclose(file);

  return NULL;
}

char*
girara_file_read_from_fd(int fd)
{
  unsigned int bc = BLOCK_SIZE;
  unsigned int i  = 0;
  char* buffer    = malloc(sizeof(char) * bc);

  if (buffer == NULL) {
    goto error_ret;
  }

  char c;
  while ((read(fd, &c, 1)) == 1) {
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
