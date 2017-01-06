/* See LICENSE file for license and copyright information */

#include "log.h"

#include <stdarg.h>
#include <stdio.h>

static girara_log_level_t log_level = GIRARA_DEBUG;

void
girara_vlog(const char* location, girara_log_level_t level, const char* format, va_list ap)
{
  if (level < log_level) {
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
      if (location != NULL) {
        fprintf(stderr, "debug: %s: ", location);
      } else {
        fprintf(stderr, "debug: ");
      }
      break;
    default:
      return;
  }

  vfprintf(stderr, format, ap);
  fprintf(stderr, "\n");
}

void
girara_log(const char* location, girara_log_level_t level, const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  girara_vlog(location, level, format, ap);
  va_end(ap);
}

girara_log_level_t
girara_get_log_level()
{
  return log_level;
}

void
girara_set_log_level(girara_log_level_t level)
{
  log_level = level;
}

/* old compat function, remove once we bump the SONAME */

void
_girara_log(const char* file, int line, girara_log_level_t level, const char* format, ...)
{
  char* tmp = g_strdup_printf("%s:%d", file, line);
  va_list ap;
  va_start(ap, format);
  girara_vlog(tmp, level, format, ap);
  va_end(ap);
}

girara_log_level_t
girara_get_debug_level()
{
  return girara_get_log_level();
}

void
girara_set_debug_level(girara_log_level_t level)
{
  girara_set_log_level(level);
}

