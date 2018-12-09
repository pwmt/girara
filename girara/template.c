/* SPDX-License-Identifier: Zlib */

#include "template.h"
#include "utils.h"
#include "datastructures.h"
#include "internal.h"

#include <glib.h>

/**
 * Private data of the template
 */
typedef struct private_s {
  char* base;
  GRegex* variable_regex;
  GRegex* variable_check_regex;
  girara_list_t* variables_in_base;
  girara_list_t* variables;
  bool valid;
} GiraraTemplatePrivate;

G_DEFINE_TYPE_WITH_CODE(GiraraTemplate, girara_template, G_TYPE_OBJECT, G_ADD_PRIVATE(GiraraTemplate))

/**
 * Internal variables
 */
typedef struct variable_s {
  char*   name;
  char*   value;
} variable_t;

static variable_t*
new_variable(const char* name)
{
  if (name == NULL) {
    return NULL;
  }

  variable_t* variable = g_try_malloc0(sizeof(variable_t));
  if (variable == NULL) {
    return NULL;
  }

  variable->name  = g_strdup(name);
  variable->value = g_strdup("");

  return variable;
}

static void
free_variable(void* data)
{
  variable_t* variable = data;

  g_free(variable->name);
  g_free(variable->value);

  variable->name  = NULL;
  variable->value = NULL;

  g_free(variable);
}

static int
compare_variable_name(const void* data1, const void* data2)
{
  const variable_t* variable = data1;
  const char* name = data2;

  if (variable == NULL) {
    return -1;
  }

  return g_strcmp0(variable->name, name);
}

/* Methods */
static void dispose(GObject* object);
static void finalize(GObject* object);
static void set_property(GObject* object, guint prop_id,
    const GValue* value, GParamSpec* pspec);
static void get_property(GObject* object, guint prop_id, GValue* value,
    GParamSpec* pspec);
static void base_changed(GiraraTemplate* object);
static void variable_changed(GiraraTemplate* object, const char* name);
static void template_changed(GiraraTemplate* object);

/* Properties */
enum {
  PROP_0,
  PROP_BASE
};

/* Signals */
enum {
  BASE_CHANGED,
  VARIABLE_CHANGED,
  TEMPLATE_CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* Class init */
static void
girara_template_class_init(GiraraTemplateClass* class)
{
  /* overwrite methods */
  GObjectClass* object_class = G_OBJECT_CLASS(class);
  object_class->dispose      = dispose;
  object_class->finalize     = finalize;
  object_class->set_property = set_property;
  object_class->get_property = get_property;

  class->base_changed     = base_changed;
  class->variable_changed = variable_changed;
  class->changed          = template_changed;

  /* add properties */
  g_object_class_install_property(object_class, PROP_BASE,
    g_param_spec_object("base",
      "base template",
      "String used as base for the template.",
      girara_template_get_type(),
      G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  /* add signals */
  signals[BASE_CHANGED] = g_signal_new("base-changed",
      GIRARA_TYPE_TEMPLATE,
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET(GiraraTemplateClass, base_changed),
      NULL,
      NULL,
      NULL,
      G_TYPE_NONE,
      0);

  signals[VARIABLE_CHANGED] = g_signal_new("variable-changed",
      GIRARA_TYPE_TEMPLATE,
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET(GiraraTemplateClass, variable_changed),
      NULL,
      NULL,
      NULL,
      G_TYPE_NONE,
      1,
      G_TYPE_STRING);

  signals[TEMPLATE_CHANGED] = g_signal_new("changed",
      GIRARA_TYPE_TEMPLATE,
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET(GiraraTemplateClass, changed),
      NULL,
      NULL,
      NULL,
      G_TYPE_NONE,
      0);
}

/* GObject init */
static void
girara_template_init(GiraraTemplate* history)
{
  GError* error = NULL;
  GRegex* regex = g_regex_new("@([A-Za-z0-9][A-Za-z0-9_-]*)@",
                              G_REGEX_OPTIMIZE, 0, &error);
  if (regex == NULL) {
    girara_error("Failed to create regex: %s", error->message);
    g_error_free(error);
  }

  GRegex* check_regex = g_regex_new("^[A-Za-z0-9][A-Za-z0-9_-]*$",
                                    G_REGEX_OPTIMIZE, 0, &error);
  if (check_regex == NULL) {
    girara_error("Failed to create regex: %s", error->message);
    g_regex_unref(regex);
    g_error_free(error);
  }

  GiraraTemplatePrivate* priv            = girara_template_get_instance_private(history);
  priv->base                 = g_strdup("");
  priv->variable_regex       = regex;
  priv->variable_check_regex = check_regex;
  priv->variables_in_base    = girara_list_new2(g_free);
  priv->variables            = girara_list_new2(free_variable);
  priv->valid                = true;
}

/* GObject dispose */
static void
dispose(GObject* object)
{
  GiraraTemplate* obj = GIRARA_TEMPLATE(object);
  GiraraTemplatePrivate* priv = girara_template_get_instance_private(obj);

  g_regex_unref(priv->variable_regex);
  g_regex_unref(priv->variable_check_regex);

  priv->variable_regex = NULL;
  priv->variable_check_regex = NULL;

  G_OBJECT_CLASS(girara_template_parent_class)->dispose(object);
}

/* GObject finalize */
static void
finalize(GObject* object)
{
  GiraraTemplate* obj = GIRARA_TEMPLATE(object);
  GiraraTemplatePrivate* priv = girara_template_get_instance_private(obj);

  g_free(priv->base);
  girara_list_free(priv->variables_in_base);
  girara_list_free(priv->variables);

  priv->base = NULL;
  priv->variables_in_base = NULL;
  priv->variables = NULL;

  G_OBJECT_CLASS(girara_template_parent_class)->finalize(object);
}

/* GObject set_property */
static void
set_property(GObject* obj, guint prop_id, const GValue* value,
    GParamSpec* pspec)
{
  GiraraTemplate* object = GIRARA_TEMPLATE(obj);

  switch (prop_id) {
    case PROP_BASE: {
      girara_template_set_base(object, g_value_get_string(value));
      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
  }
}

/* GObject get_property */
static void
get_property(GObject* obj, guint prop_id, GValue* value,
    GParamSpec* pspec)
{
  GiraraTemplate* object = GIRARA_TEMPLATE(obj);

  switch (prop_id) {
    case PROP_BASE:
      g_value_set_string(value, girara_template_get_base(object));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
  }
}

/* Object new */
GiraraTemplate*
girara_template_new(const char* base)
{
  GObject* obj = g_object_new(GIRARA_TYPE_TEMPLATE, NULL);
  g_return_val_if_fail(obj, NULL);

  GiraraTemplate* object = GIRARA_TEMPLATE(obj);
  if (base != NULL) {
    girara_template_set_base(object, base);
  }
  return object;
}

void
girara_template_set_base(GiraraTemplate* object, const char* base)
{
  g_return_if_fail(GIRARA_IS_TEMPLATE(object));

  GiraraTemplatePrivate* priv = girara_template_get_instance_private(object);
  if (g_strcmp0(base, priv->base) != 0) {
    g_free(priv->base);
    priv->base = g_strdup(base != NULL ? base : "");

    g_signal_emit(object, signals[BASE_CHANGED], 0);
    g_signal_emit(object, signals[TEMPLATE_CHANGED], 0);
  }
}

const char*
girara_template_get_base(GiraraTemplate* object)
{
  g_return_val_if_fail(GIRARA_IS_TEMPLATE(object), NULL);

  GiraraTemplatePrivate* priv = girara_template_get_instance_private(object);
  return priv->base;
}

static void
base_changed(GiraraTemplate* object)
{
  GiraraTemplatePrivate* priv = girara_template_get_instance_private(object);
  girara_list_clear(priv->variables_in_base);
  priv->valid = true;

  GMatchInfo* match_info = NULL;
  if (g_regex_match(priv->variable_regex, priv->base, 0, &match_info) == true) {
    while (g_match_info_matches(match_info) == true) {
      char* variable = g_match_info_fetch(match_info, 1);
      char* found = girara_list_find(priv->variables_in_base,
          list_strcmp, variable);

      if (priv->valid == true) {
        if (girara_list_find(priv->variables, compare_variable_name,
                             variable) == NULL) {
          girara_debug("Variable '%s' not set.", variable);
          priv->valid = false;
        }
      }

      if (found == NULL) {
        girara_list_append(priv->variables_in_base, variable);
      } else {
        g_free(variable);
      }

      g_match_info_next(match_info, NULL);
    }
  }
  g_match_info_free(match_info);
}

static void
variable_changed(GiraraTemplate* object, const char* GIRARA_UNUSED(name))
{
  GiraraTemplatePrivate* priv = girara_template_get_instance_private(object);
  priv->valid = true;

  GIRARA_LIST_FOREACH_BODY(priv->variables_in_base, char*, variable,
    if (priv->valid == true &&
        girara_list_find(priv->variables, compare_variable_name,
                         variable) == NULL) {
      priv->valid = false;
    }
  );
}

static void
template_changed(GiraraTemplate* GIRARA_UNUSED(object))
{
}

girara_list_t*
girara_template_referenced_variables(GiraraTemplate* object)
{
  g_return_val_if_fail(GIRARA_IS_TEMPLATE(object), NULL);

  GiraraTemplatePrivate* priv = girara_template_get_instance_private(object);
  return priv->variables_in_base;
}

bool
girara_template_add_variable(GiraraTemplate* object, const char* name)
{
  g_return_val_if_fail(GIRARA_IS_TEMPLATE(object), false);
  g_return_val_if_fail(name != NULL, false);

  GiraraTemplatePrivate* priv = girara_template_get_instance_private(object);

  if (g_regex_match(priv->variable_check_regex, name, 0, NULL) == FALSE) {
    girara_debug("'%s' is not a valid variable name.", name);
    return false;
  }

  variable_t* variable = girara_list_find(priv->variables, compare_variable_name,
                                          name);
  if (variable != NULL) {
    girara_debug("Variable '%s' already exists.", name);
    return false;
  }

  variable = new_variable(name);
  if (variable == NULL) {
    girara_debug("Could not create new variable.");
    return false;
  }

  girara_list_append(priv->variables, variable);
  g_signal_emit(object, signals[VARIABLE_CHANGED], 0, name);
  g_signal_emit(object, signals[TEMPLATE_CHANGED], 0);

  return true;
}

void
girara_template_set_variable_value(GiraraTemplate* object, const char* name,
                                   const char* value)
{
  g_return_if_fail(GIRARA_IS_TEMPLATE(object));
  g_return_if_fail(name != NULL);
  g_return_if_fail(value != NULL);

  GiraraTemplatePrivate* priv = girara_template_get_instance_private(object);

  variable_t* variable = girara_list_find(priv->variables, compare_variable_name,
                                          name);
  if (variable == NULL) {
    girara_error("Variable '%s' does not exist.", name);
    return;
  }

  if (g_strcmp0(variable->value, value) != 0) {
    g_free(variable->value);
    variable->value = g_strdup(value);

    g_signal_emit(object, signals[VARIABLE_CHANGED], 0, name);
    g_signal_emit(object, signals[TEMPLATE_CHANGED], 0);
  }
}

static gboolean
eval_replace_cb(const GMatchInfo* info, GString* res, void* data)
{
  girara_list_t* variables = data;

  char* name = g_match_info_fetch(info, 1);
  variable_t* variable = girara_list_find(variables, compare_variable_name,
                                          name);
  g_return_val_if_fail(variable != NULL, TRUE);

  g_string_append(res, variable->value);
  g_free(name);

  return FALSE;
}

char*
girara_template_evaluate(GiraraTemplate* object)
{
  g_return_val_if_fail(GIRARA_IS_TEMPLATE(object), NULL);

  GiraraTemplatePrivate* priv = girara_template_get_instance_private(object);
  if (priv->valid == false) {
    girara_error("Base contains variables that do not have a value assigned.");
    return NULL;
  }

  return g_regex_replace_eval(priv->variable_regex, priv->base, -1, 0, 0,
                              eval_replace_cb, priv->variables, NULL);
}
