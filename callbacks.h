/* See LICENSE file for license and copyright information */

#ifndef GIRARA_CALLBACKS_H
#define GIRARA_CALLBACKS_H

#include "types.h"
#include <gtk/gtk.h>

/**
 * Callback definition for an inputbar key press event handler
 *
 * @param widget The widget
 * @param event Event
 * @param data Custom data
 * @return true if no error occured
 */
typedef bool (*girara_callback_inputbar_key_press_event_t)(GtkWidget* widget, GdkEventKey* event, void* data);

/**
 * Callback definition for an inputbar key press event handler
 *
 * @param entry The inputbar
 * @param data Custom data
 * @return true if no error occured
 */
typedef bool (*girara_callback_inputbar_activate_t)(GtkEntry* entry, void* data);

/**
 * Default callback for key press events in the view area
 *
 * @param widget The used widget
 * @param event The occured event
 * @param session The used girara session
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_callback_view_key_press_event(GtkWidget* widget, GdkEventKey* event, girara_session_t* session);

/**
 * Default callback if the inputbar gets activated
 *
 * @param entry The inputbar entry
 * @param session The used girara session
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_callback_inputbar_activate(GtkEntry* entry, girara_session_t* session);

/**
 * Default callback if an key in the input bar gets pressed
 *
 * @param widget The used widget
 * @param event The occured event
 * @param session The used girara session
 * @return TRUE No error occured
 * @return FALSE An error occured
 */
bool girara_callback_inputbar_key_press_event(GtkWidget* widget, GdkEventKey* event, girara_session_t* session);

#endif
