/* See LICENSE file for license and copyright information */

#ifndef GIRARA_MACROS_H
#define GIRARA_MACROS_H

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
# if (defined(__GNUC__) && (__GNUC__ >= 4)) || defined(__clang__)
#  define GIRARA_HIDDEN __attribute__((visibility("hidden")))
# elif defined(__SUNPRO_C)
#  define GIRARA_HIDDEN __hidden
# else
#  define GIRARA_HIDDEN
# endif
#endif

#ifndef GIRARA_DEPRECATED
# if defined(__GNUC__)
#  define GIRARA_DEPRECATED(x) x __attribute__((deprecated))
# else
#  define GIRARA_DEPRECATED(x) x
# endif
#endif

#ifndef GIRARA_ALLOC_SIZE
# if defined(__GNUC__)
#  define GIRARA_ALLOC_SIZE(...) __attribute__((alloc_size(__VA_ARGS__)))
# else
#  define GIRARA_ALLOC_SIZE(x)
# endif
#endif

#endif
