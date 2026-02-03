/* SPDX-License-Identifier: Zlib */

#include <ctype.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <limits.h>
#include <pwd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"
#include "datastructures.h"
#include "internal.h"
#include "log.h"

char* girara_fix_path(const char* path) {
  if (path == NULL) {
    return NULL;
  }

  char* rpath = NULL;
  if (path[0] == '~') {
    const size_t len      = strlen(path);
    g_autofree char* user = NULL;
    size_t idx            = 1;

    if (len > 1 && path[1] != '/') {
      while (path[idx] && path[idx] != '/') {
        ++idx;
      }

      user = g_strndup(path + 1, idx - 1);
    }

    g_autofree char* home_path = girara_get_home_directory(user);
    if (home_path == NULL) {
      return g_strdup(path);
    }

    rpath = g_build_filename(home_path, path + idx, NULL);
  } else if (g_path_is_absolute(path) == TRUE) {
    rpath = g_strdup(path);
  } else {
    g_autofree char* curdir = g_get_current_dir();
    rpath                   = g_build_filename(curdir, path, NULL);
  }

  return rpath;
}

bool girara_xdg_open_with_working_directory(const char* uri, const char* working_directory) {
  if (uri == NULL || strlen(uri) == 0) {
    return false;
  }

  /* g_spawn_async expects char** */
  static char xdg_open[] = "xdg-open";
  char* argv[]           = {xdg_open, g_strdup(uri), NULL};

  g_autoptr(GError) error = NULL;
  bool res                = g_spawn_async(working_directory, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error);
  if (error != NULL) {
    girara_warning("Failed to execute 'xdg-open %s': %s", uri, error->message);
    error = NULL;
  }

  if (res == false) {
    /* fall back to `gio open` */
    char* current_dir = working_directory != NULL ? g_get_current_dir() : NULL;
    if (working_directory != NULL) {
      g_chdir(working_directory);
    }

    res = g_app_info_launch_default_for_uri(uri, NULL, &error);
    if (error != NULL) {
      girara_warning("Failed to open '%s': %s", uri, error->message);
    }

    if (working_directory != NULL) {
      g_chdir(current_dir);
    }
  }

  g_free(argv[1]);

  return res;
}

bool girara_xdg_open(const char* uri) {
  return girara_xdg_open_with_working_directory(uri, NULL);
}

#if defined(HAVE_GETPWNAM_R)
static char* get_home_directory_getpwnam(const char* user) {
#ifdef _SC_GETPW_R_SIZE_MAX
  int bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (bufsize < 0) {
    bufsize = 4096;
  }
#else
  static const int bufsize = 4096;
#endif

  g_autofree char* buffer = g_try_malloc0(sizeof(char) * bufsize);
  if (buffer == NULL) {
    return NULL;
  }

  struct passwd pwd;
  struct passwd* result = NULL;
  char* dir             = NULL;
  if (getpwnam_r(user, &pwd, buffer, bufsize, &result) == 0 && result != NULL) {
    dir = g_strdup(pwd.pw_dir);
  }

  return dir;
}
#else
static char* get_home_directory_getpwnam(const char* user) {
  const struct passwd* pwd = getpwnam(user);
  if (pwd != NULL) {
    return g_strdup(pwd->pw_dir);
  }

  return NULL;
}
#endif

char* girara_get_home_directory(const char* user) {
  if (user == NULL || g_strcmp0(user, g_get_user_name()) == 0) {
    return g_strdup(g_get_home_dir());
  }

  return get_home_directory_getpwnam(user);
}

char* girara_get_xdg_path(girara_xdg_path_t path) {
  static const char VARS[][16] = {
      [XDG_CONFIG_DIRS] = "XDG_CONFIG_DIRS",
      [XDG_DATA_DIRS]   = "XDG_DATA_DIRS",
  };

  static const char DEFAULTS[][29] = {
      [XDG_CONFIG_DIRS] = "/etc/xdg",
      [XDG_DATA_DIRS]   = "/usr/local/share/:/usr/share",
  };

  switch (path) {
  case XDG_DATA:
    return g_strdup(g_get_user_data_dir());
  case XDG_CONFIG:
    return g_strdup(g_get_user_config_dir());
  case XDG_CONFIG_DIRS:
  case XDG_DATA_DIRS: {
    const char* tmp = g_getenv(VARS[path]);
    if (tmp == NULL || !g_strcmp0(tmp, "")) {
      return g_strdup(DEFAULTS[path]);
    }
    return g_strdup(tmp);
  }
  case XDG_CACHE:
    return g_strdup(g_get_user_cache_dir());
  }

  return NULL;
}

char* girara_escape_string(const char* value) {
  if (value == NULL) {
    return NULL;
  }

  GString* str = g_string_new("");
  while (*value != '\0') {
    const char c = *value++;
    if (strchr("\\ \t\"\'#", c) != NULL) {
      g_string_append_c(str, '\\');
    }
    g_string_append_c(str, c);
  }

  return g_string_free(str, FALSE);
}

char* girara_replace_substring(const char* string, const char* old, const char* new) {
  if (string == NULL || old == NULL || new == NULL) {
    return NULL;
  }

  if (*string == '\0' || *old == '\0' || strstr(string, old) == NULL) {
    return g_strdup(string);
  }

  gchar** split = g_strsplit(string, old, -1);
  char* ret     = g_strjoinv(new, split);
  g_strfreev(split);

  return ret;
}

const char* girara_version(void) {
  return GIRARA_VERSION;
}

int list_strcmp(const void* data1, const void* data2) {
  const char* str1 = data1;
  const char* str2 = data2;

  return g_strcmp0(str1, str2);
}
