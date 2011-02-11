// See LICENSE file for license and copyright information

#ifndef HELPERS_H
#define HELPERS_H

#include <glib.h>
#include <stdint.h>

#define g_assert_cmpptr(lhs, op, rhs) \
  g_assert_cmpuint((intptr_t)(lhs), op, (intptr_t)(rhs))

#endif
