/* See LICENSE file for license and copyright information */

#ifndef GIRARA_INTERNAL_H
#define GIRARA_INTERNAL_H

#define CLEAN(m) (m & ~(GDK_MOD2_MASK) & ~(GDK_BUTTON1_MASK) & ~(GDK_BUTTON2_MASK) & ~(GDK_BUTTON3_MASK) & ~(GDK_BUTTON4_MASK) & ~(GDK_BUTTON5_MASK) & ~(GDK_LEAVE_NOTIFY_MASK))
#define FORMAT_COMMAND "<b>%s</b>"
#define FORMAT_DESCRIPTION "<i>%s</i>"

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

void girara_toggle_widget_visibility(GtkWidget* widget);

#endif
