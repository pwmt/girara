/* SPDX-License-Identifier: Zlib */

#include "log.h"

#include <stdarg.h>
#include <stdio.h>

static girara_log_level_t log_level = GIRARA_ERROR;

static const char NAMES[][8] = {
    [GIRARA_DEBUG]   = "debug",
    [GIRARA_INFO]    = "info",
    [GIRARA_WARNING] = "warning",
    [GIRARA_ERROR]   = "error",
};

void girara_vlog(const char* location, const char* function, girara_log_level_t level, const char* format, va_list ap) {
  if (level < log_level || level < GIRARA_DEBUG || level > GIRARA_ERROR) {
    return;
  }

  fprintf(stderr, "%s: ", NAMES[level]);
  if (level == GIRARA_DEBUG) {
    if (location != NULL) {
      fprintf(stderr, "%s: ", location);
    }
    if (function != NULL) {
      fprintf(stderr, "%s(): ", function);
    }
  }
  vfprintf(stderr, format, ap);
  fprintf(stderr, "\n");
}

void girara_log(const char* location, const char* function, girara_log_level_t level, const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  girara_vlog(location, function, level, format, ap);
  va_end(ap);
}

girara_log_level_t girara_get_log_level(void) {
  return log_level;
}

void girara_set_log_level(girara_log_level_t level) {
  log_level = level;
}

void girara_set_log_level_from_string(const char* loglevel) {
  if (loglevel == NULL || g_strcmp0(loglevel, "info") == 0) {
    girara_set_log_level(GIRARA_INFO);
  } else if (g_strcmp0(loglevel, "warning") == 0) {
    girara_set_log_level(GIRARA_WARNING);
  } else if (g_strcmp0(loglevel, "error") == 0) {
    girara_set_log_level(GIRARA_ERROR);
  } else if (g_strcmp0(loglevel, "debug") == 0) {
    girara_set_log_level(GIRARA_DEBUG);
  } else {
    girara_error("Invalid log level: %s", loglevel);
  }
}
