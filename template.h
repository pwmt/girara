/* See LICENSE file for license and copyright information */

#ifndef GIRARA_TEMPLATE_H
#define GIRARA_TEMPLATE_H

#include <glib-object.h>
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
GType girara_template_get_type(void);

/**
 * Create new template object.
 *
 * @param template a string that is used as template
 * @returns a templot object
 */
GiraraTemplate* girara_template_new(const char* base);

void girara_template_set_base(GiraraTemplate* object, const char* base);
const char* girara_template_get_base(GiraraTemplate* object);

girara_list_t* girara_template_referenced_variables(GiraraTemplate* object);
void girara_template_add_variable(GiraraTemplate* object, const char* name);
void girara_template_set_variable_value(GiraraTemplate* object, const char* name, const char* value);
char* girara_template_evaluate(GiraraTemplate* object);

#endif
