/* See LICENSE file for license and copyright information */

#ifndef GIRARA_MACROS_H
#define GIRARA_MACROS_H

#if     __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define GIRARA_PRINTF(format_idx, arg_idx) \
  __attribute__((__format__ (__printf__, format_idx, arg_idx)))
#else
#define GIRARA_PRINTF(format_idx, arg_idx)
#endif

#endif
