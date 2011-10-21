/* See LICENSE file for license and copyright information */

#ifndef GIRARA_TABS_H
#define GIRARA_TABS_H

#include "girara-types.h"
#include <gtk/gtk.h>

/**
 * Structure of a tab
 */
struct girara_tab_s
{
  char* title; /**< The title of the tab */
  GtkWidget* widget; /**< The displayed widget of the tab */
  void* data; /**< Custom data */
  girara_session_t* session; /**< Girara session */
};

/**
 * Enables the tab view. If girara_set_view is used, the tab bar will
 * automatically vanish and girara_tabs_enable has to be called another time to
 * re-enable it again.
 *
 * @param session The girara session
 */
void girara_tabs_enable(girara_session_t* session);

/**
 * Creates and adds a new tab to the tab view
 *
 * @param session The girara session
 * @param title Title of the tab (optional)
 * @param widget Displayed widget
 * @param next_to_current Tab should be created right next to the current one
 * @param data Custom data
 * @return A new tab object or NULL if an error occured
 */
girara_tab_t* girara_tab_new(girara_session_t* session, const char* title,
    GtkWidget* widget, bool next_to_current, void* data);

/**
 * Removes and destroys a tab from the tab view
 *
 * @param session The girara session
 * @param tab Tab
 */
void girara_tab_remove(girara_session_t* session, girara_tab_t* tab);

/**
 * Returns the tab at the given index
 *
 * @param session The girara session
 * @param index Index of the tab
 * @return The tab object or NULL if an error occured
 */
girara_tab_t* girara_tab_get(girara_session_t* session, unsigned int index);

/**
 * Returns the number of tabs
 *
 * @param session The girara session
 * @return The number of tabs
 */
int girara_get_number_of_tabs(girara_session_t* session);

/**
 * Updates the color and states of all tabs
 *
 * @param session The girara session
 */
void girara_tab_update(girara_session_t* session);

/**
 * Returns the current tab
 *
 * @param session The girara session
 * @return The current tab or NULL if an error occured
 */
girara_tab_t* girara_tab_current_get(girara_session_t* session);

/**
 * Sets the current tab
 *
 * @param session The girara session
 * @param tab The new current tab
 */
void girara_tab_current_set(girara_session_t* session, girara_tab_t* tab);

/**
 * Sets the shown title of the tab
 *
 * @param tab The tab
 * @param title The new title
 */
void girara_tab_title_set(girara_tab_t* tab, const char* title);

/**
 * Returns the title of the tab
 *
 * @param tab The tab
 * @return The title of the tab or NULL if an error occured
 */
const char* girara_tab_title_get(girara_tab_t* tab);

/**
 * Returns the position of the tab
 *
 * @param session Girara session
 * @param tab The tab
 * @return The id of the tab or -1 if an error occured
 */
int girara_tab_position_get(girara_session_t* session, girara_tab_t* tab);

/**
 * Sets the new position of the tab
 *
 * @param session Girara session
 * @param tab The tab
 * @param position The new position
 */
void girara_tab_position_set(girara_session_t* session, girara_tab_t* tab,
    unsigned int position);


#endif
