/* SPDX-License-Identifier: Zlib */

#ifndef GIRARA_TEMPLATE_H
#define GIRARA_TEMPLATE_H

#include <glib-object.h>
#include "macros.h"
#include "types.h"

struct girara_template_s {
  GObject parent;
};

struct girara_template_class_s {
  GObjectClass parent_class;

  void (*base_changed)(GiraraTemplate*);
  void (*variable_changed)(GiraraTemplate*, const char* name);
  void (*changed)(GiraraTemplate*);
};

#define GIRARA_TYPE_TEMPLATE \
  (girara_template_get_type())
#define GIRARA_TEMPLATE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GIRARA_TYPE_TEMPLATE, GiraraTemplate))
#define GIRARA_TEMPLATE_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_CAST((obj), GIRARA_TYPE_TEMPLATE, GiraraTemplateClass))
#define GIRARA_IS_TEMPLATE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GIRARA_TYPE_TEMPLATE))
#define GIRARA_IS_TEMPLATE_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((obj), GIRARA_TYPE_TEMPLATE))
#define GIRARA_TEMPLATE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj), GIRARA_TYPE_TEMPLATE, GiraraTemplateClass))

/**
 * Returns the type of the template.
 *
 * @return the type
 */
GType girara_template_get_type(void) G_GNUC_CONST GIRARA_VISIBLE;

/**
 * Create new template object.
 *
 * @param base a string that is used as template
 * @returns a templot object
 */
GiraraTemplate* girara_template_new(const char* base) GIRARA_VISIBLE;

/**
 * Set the base string of the template.
 *
 * @param object GiraraTemplate object
 * @param base a string that is used as template
 */
void girara_template_set_base(GiraraTemplate* object, const char* base) GIRARA_VISIBLE;

/**
 * Get the base string of the template.
 *
 * @param object GiraraTemplate object
 * @returns string that is used as template
 */
const char* girara_template_get_base(GiraraTemplate* object) GIRARA_VISIBLE;

/**
 * Get list of variable names referenced in the template.
 *
 * @param object GiraraTemplate object
 * @returns list of variables names referenced in the template
 */
girara_list_t* girara_template_referenced_variables(GiraraTemplate* object) GIRARA_VISIBLE;

/**
 * Register a variable.
 *
 * @param object GiraraTemplate object
 * @param name name of the variable
 * @returns true if the variable was added, false otherwise
 */
bool girara_template_add_variable(GiraraTemplate* object, const char* name) GIRARA_VISIBLE;

/**
 * Set value of a variable.
 *
 * @param object GiraraTemplate object
 * @param name name of the variable
 * @param value value of the variable
 */
void girara_template_set_variable_value(GiraraTemplate* object, const char* name, const char* value) GIRARA_VISIBLE;

/**
 * Replace all variables with their values in the template.
 *
 * @param object GiraraTemplate object
 * @returns evaluated template, needes to be deallocated with g_free
 */
char* girara_template_evaluate(GiraraTemplate* object) GIRARA_VISIBLE;

#endif
