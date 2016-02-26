/* See LICENSE file for license and copyright information */

#include "utils.h"
#include <stdarg.h>
#include <stdio.h>

static girara_debug_level_t debug_level = GIRARA_DEBUG;

void
_girara_debug(const char* function, int line, girara_debug_level_t level, const char* format, ...)
{
  if (level < debug_level) {
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
      break;
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
