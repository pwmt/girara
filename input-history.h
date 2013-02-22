/* See LICENSE file for license and copyright information */

#ifndef GIRARA_INPUT_HISTORY_H
#define GIRARA_INPUT_HISTORY_H

#include <glib-object.h>
#include "types.h"

struct girara_input_history_s {
  GObject parent;
};

struct girara_input_history_class_s {
  GObjectClass parent_class;
};

#define GIRARA_TYPE_INPUT_HISTORY \
  (girara_input_history_get_type ())
#define GIRARA_INPUT_HISTORY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIRARA_TYPE_INPUT_HISTORY, GiraraInputHistory))
#define GIRARA_INPUT_HISTORY_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_CAST ((obj), GIRARA_TYPE_INPUT_HISTORY, GiraraInputHistoryClass))
#define GIRARA_IS_INPUT_HISTORY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIRARA_TYPE_INPUT_HISTORY))
#define GIRARA_IS_INPUT_HISTORY_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE ((obj), GIRARA_TYPE_INPUT_HISTORY))
#define GIRARA_INPUT_HISTORY_GET_CLASS \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIRARA_TYPE_INPUT_HISTORY, GiraraInputHistoryClass))

/**
 * Returns the type of the input history.
 *
 * @return the type
 */
GType girara_input_history_get_type(void);

/**
 * Create new input history object.
 *
 * @returns an input history object
 */
GiraraInputHistory* girara_input_history_new(void);

/**
 * Append a new line of input.
 *
 * @param history an input history instance
 * @param input the input
 */
void girara_input_history_append(GiraraInputHistory* history, const char* input);

/**
 * Get the "next" input from the history
 *
 * @param history an input history instance
 * @param current_input input used to find the "next" input
 * @returns "next" input
 */
const char* girara_input_history_next(GiraraInputHistory* history,
    const char* current_input);

/**
 * Get the "previous" input from the history
 *
 * @param history an input history instance
 * @param current_input input used to find the "next" input
 * @returns "previous" input
 */
const char* girara_input_history_previous(GiraraInputHistory* history,
    const char* current_input);

/**
 * Reset state of the input history
 *
 * @param history an input history instance
 */
void girara_input_history_reset(GiraraInputHistory* history);

#endif
