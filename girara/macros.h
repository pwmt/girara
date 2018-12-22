/* SPDX-License-Identifier: Zlib */

#ifndef GIRARA_MACROS_H
#define GIRARA_MACROS_H

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#ifndef GIRARA_PRINTF
# if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4) || defined(__clang__)
#  define GIRARA_PRINTF(format_idx, arg_idx) \
    __attribute__((__format__ (__printf__, format_idx, arg_idx)))
# else
#  define GIRARA_PRINTF(format_idx, arg_idx)
# endif
#endif

#ifndef GIRARA_UNUSED
# if defined(__GNUC__) || defined(__clang__)
#  define GIRARA_UNUSED(x) UNUSED_ ## x __attribute__((unused))
# elif defined(__LCLINT__)
#  define GIRARA_UNUSED(x) /*@unused@*/ x
# else
#  define GIRARA_UNUSED(x) x
# endif
#endif

#ifndef GIRARA_HIDDEN
# if (defined(__GNUC__) && (__GNUC__ >= 4)) || __has_attribute(visibility)
#  define GIRARA_HIDDEN __attribute__((visibility("hidden")))
# elif defined(__SUNPRO_C)
#  define GIRARA_HIDDEN __hidden
# else
#  define GIRARA_HIDDEN
# endif
#endif

#ifndef GIRARA_VISIBLE
# if (defined(__GNUC__) && (__GNUC__ >= 4)) || __has_attribute(visibility)
#  define GIRARA_VISIBLE __attribute__((visibility("default")))
# else
#  define GIRARA_VISIBLE
# endif
#endif

#ifndef GIRARA_DEPRECATED
# if defined(__GNUC__)
#  define GIRARA_DEPRECATED(x) x __attribute__((deprecated))
#  define GIRARA_DEPRECATED_ __attribute__((deprecated))
# else
#  define GIRARA_DEPRECATED(x) x
#  define GIRARA_DEPRECATED_
# endif
#endif

#ifndef GIRARA_ALLOC_SIZE
# if (!defined(__clang__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))) || \
      (defined(__clang__) && __has_attribute(__alloc_size__))
#  define GIRARA_ALLOC_SIZE(...) __attribute__((alloc_size(__VA_ARGS__)))
# else
#  define GIRARA_ALLOC_SIZE(x)
# endif
#endif

#ifndef GIRARA_DO_PRAGMA
# if defined(__GNUC__) || defined(__clang__)
#  define GIRARA_DO_PRAGMA(x) _Pragma(#x)
# else
#  define GIRARA_DO_PRAGMA(x)
# endif
#endif

#ifndef GIRARA_IGNORE_DEPRECATED
# define GIRARA_IGNORE_DEPRECATED \
    GIRARA_DO_PRAGMA(GCC diagnostic push) \
    GIRARA_DO_PRAGMA(GCC diagnostic ignored "-Wdeprecated-declarations")
#endif

#ifndef GIRARA_UNIGNORE
# define GIRARA_UNIGNORE \
    GIRARA_DO_PRAGMA(GCC diagnostic pop)
#endif

#endif
