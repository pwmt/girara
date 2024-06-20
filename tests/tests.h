/* SPDX-License-Identifier: Zlib */

#ifndef GIRARA_TESTS_H
#define GIRARA_TESTS_H

#include <girara/log.h>

static void critical_print(const gchar* GIRARA_UNUSED(log_domain), GLogLevelFlags GIRARA_UNUSED(log_level),
                           const gchar* message, gpointer GIRARA_UNUSED(user_data)) {
  girara_debug("expected glib critical: %s", message);
}

static gboolean ignore_all_log_errors(const gchar* GIRARA_UNUSED(log_domain), GLogLevelFlags GIRARA_UNUSED(log_level),
                                      const gchar* GIRARA_UNUSED(message), gpointer GIRARA_UNUSED(user_data)) {
  return FALSE;
}

static void setup_logger(void) {
  g_test_log_set_fatal_handler(ignore_all_log_errors, NULL);
  g_log_set_handler(NULL, G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL, critical_print, NULL);
}

#endif
