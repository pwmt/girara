/* SPDX-License-Identifier: Zlib */

#ifndef GIRARA_RECORD_H
#define GIRARA_RECORD_H

#include <gdk/gdk.h>

#include "macros.h"
#include "types.h"

/**
 * Initialise macro recording
 *
 * @param session The girara session
 */
void girara_record_init(girara_session_t* session);

/**
 * Set keys to ignore during macro recording
 *
 * @param session The girara session
 * @param input   String of keys to ignore
 */
void girara_record_filter_keys(girara_session_t* session, char* input) GIRARA_VISIBLE;

/**
 * Record key event to macro
 *
 * @param session     The girara session
 * @param widget_name Name of the widget which recieved the keypress
 * @param event       Key event
 */
void girara_record_key_event(girara_session_t* session, const char* widget_name, GdkEventKey* event) GIRARA_VISIBLE;

/**
 * Record an assert to macro.
 * When replaying the macro, the name and value are passed 
 * to session->record.assert_cb.
 *
 * @param session The girara session
 * @param name    Name of the attribute
 * @param value   Value of the attribute
 */
void girara_record_assert(girara_session_t* session, char* name, char* value) GIRARA_VISIBLE;

/**
 * Record a breakpoint to macro.
 * During replay, execution is stopped at the breakpoint.
 * Calling `girara_record_run_macro` will resume execution.
 *
 * @param session The girara session
 */
void girara_record_breakpoint(girara_session_t* session) GIRARA_VISIBLE;

/**
 * Load a macro
 *
 * @param session The girara session
 * @param path    Path to macro file
 * @return false  An error occurred
 */
gboolean girara_record_load_macro(girara_session_t* session, char* path) GIRARA_VISIBLE;

/**
 * Run (or resume) a macro.
 *
 * @param session The girara session
 */
void girara_record_run_macro(girara_session_t* session) GIRARA_VISIBLE;

void girara_record_item_free(girara_record_item_t* item);

#endif // GIRARA_RECORD_H
