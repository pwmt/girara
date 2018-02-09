/* See LICENSE file for license and copyright information */

#include "log.h"

#include <stdarg.h>
#include <stdio.h>

static girara_log_level_t log_level = GIRARA_DEBUG;

static const char* NAMES[] = {
  [GIRARA_DEBUG] = "debug",
  [GIRARA_INFO] = "info",
  [GIRARA_WARNING] = "warning",
  [GIRARA_ERROR] = "error"
};

void
girara_vlog(const char* location, const char* function, girara_log_level_t level, const char* format, va_list ap)
{
  if (level < log_level || level < GIRARA_DEBUG || level > GIRARA_ERROR ) {
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

void
girara_log(const char* location, const char* function, girara_log_level_t level, const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  girara_vlog(location, function, level, format, ap);
  va_end(ap);
}

girara_log_level_t
girara_get_log_level(void)
{
  return log_level;
}

void
girara_set_log_level(girara_log_level_t level)
{
  log_level = level;
}
