/* See LICENSE file for license and copyright information */

#include "settings-manager.h"
#include "datastructures.h"

G_DEFINE_TYPE(GiraraSettingsManager, girara_settings_manager, G_TYPE_OBJECT)

/**
 * Structure of a settings entry
 */
struct girara_setting_s
{
  char* name; /**< Name of the setting */
  union
  {
    bool b; /**< Boolean */
    int i; /**< Integer */
    float f; /**< Floating number */
    char *s; /**< String */
  } value; /**< Value of the setting */
  girara_setting_type_t type; /**< Type identifier */
  bool init_only; /**< Option can be set only before girara gets initialized */
  char* description; /**< Description of this setting */
};

/* Helper functions */

/* find a setting in the list of settings */
static void
girara_setting_free(girara_setting_t* setting);
static girara_setting_t*
girara_settings_manager_find(girara_list_t* list, const char* name);
static void
girara_settings_manager_set_value(girara_setting_t* setting, void* value);
static bool
girara_settings_manager_get_value(girara_setting_t* setting, void* dest);
static int
cb_sort_settings(girara_setting_t* lhs, girara_setting_t* rhs);

/**
 * Private data of the settings manager
 */
typedef struct girara_settings_manager_private_s {
  girara_list_t* settings; /**< List of stored settings */
} girara_settings_manager_private_t;

#define GIRARA_SETTINGS_MANAGER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GIRARA_TYPE_SETTINGS_MANAGER, \
                                girara_settings_manager_private_t))

/* Signals */
enum {
  VALUE_CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* Methods */
static void girara_settings_manager_finalize(GObject* object);

/**
 * Class init
 */
static void
girara_settings_manager_class_init(GiraraSettingsManagerClass* class)
{
  /* add private members */
  g_type_class_add_private(class, sizeof(girara_settings_manager_private_t));

  /* overwrite methods */
  GObjectClass* object_class = G_OBJECT_CLASS(class);
  object_class->finalize     = girara_settings_manager_finalize;

  /* initialize signals */
  signals[VALUE_CHANGED] = g_signal_new("value-changed",
      GIRARA_TYPE_SETTINGS_MANAGER,
      G_SIGNAL_RUN_LAST,
      0,
      NULL,
      NULL,
      g_cclosure_marshal_generic,
      G_TYPE_NONE,
      2,
      G_TYPE_STRING,
      G_TYPE_POINTER);
}

/**
 * Object init
 */
static void
girara_settings_manager_init(GiraraSettingsManager* manager)
{
  girara_settings_manager_private_t* priv = GIRARA_SETTINGS_MANAGER_GET_PRIVATE(manager);
  priv->settings = girara_sorted_list_new2(
      (girara_compare_function_t) cb_sort_settings,
      (girara_free_function_t) girara_setting_free);
}

/**
 * Object finalize
 */
static void
girara_settings_manager_finalize(GObject* object)
{
  girara_settings_manager_private_t* priv = GIRARA_SETTINGS_MANAGER_GET_PRIVATE(object);
  girara_list_free(priv->settings);


  G_OBJECT_CLASS(girara_settings_manager_parent_class)->finalize(object);
}

GObject*
girara_settings_manager_new(void)
{
  return g_object_new(GIRARA_TYPE_SETTINGS_MANAGER, 0);
}

bool
girara_settings_manager_add(GiraraSettingsManager* manager, const char* name,
    void* value, girara_setting_type_t type, bool init_only,
    const char* description)
{
  g_return_val_if_fail(manager != NULL, false);
  g_return_val_if_fail(GIRARA_IS_SETTINGS_MANAGER(manager) == TRUE, false);
  girara_settings_manager_private_t* priv = GIRARA_SETTINGS_MANAGER_GET_PRIVATE(manager);

  g_return_val_if_fail(name != NULL, false);
  g_return_val_if_fail(type != UNKNOWN, false);
  if (type != STRING && value == NULL) {
    return false;
  }

  /* search for existing setting */
  if (girara_settings_manager_find(priv->settings, name) != NULL) {
    return false;
  }

  /* add new setting */
  girara_setting_t* setting = g_slice_new0(girara_setting_t);
  setting->name        = g_strdup(name);
  setting->type        = type;
  setting->init_only   = init_only;
  setting->description = description ? g_strdup(description) : NULL;
  girara_settings_manager_set_value(setting, value);
  girara_list_append(priv->settings, setting);

  return true;
}

bool
girara_settings_manager_set(GiraraSettingsManager* manager, const char* name,
    void* value)
{
  g_return_val_if_fail(manager != NULL, false);
  g_return_val_if_fail(GIRARA_IS_SETTINGS_MANAGER(manager) == TRUE, false);
  girara_settings_manager_private_t* priv = GIRARA_SETTINGS_MANAGER_GET_PRIVATE(manager);
  g_return_val_if_fail(name != NULL, false);

  /* search for existing setting */
  girara_setting_t* setting = girara_settings_manager_find(priv->settings, name);
  if (setting == NULL) {
    return false;
  }

  girara_settings_manager_set_value(setting, value);
  g_signal_emit(manager, signals[VALUE_CHANGED], 0, name, value);
  return true;
}

bool
girara_settings_manager_get(GiraraSettingsManager* manager, const char* name,
    void* dest)
{
  g_return_val_if_fail(manager != NULL, false);
  g_return_val_if_fail(GIRARA_IS_SETTINGS_MANAGER(manager) == TRUE, false);
  girara_settings_manager_private_t* priv = GIRARA_SETTINGS_MANAGER_GET_PRIVATE(manager);
  g_return_val_if_fail(name != NULL, false);

  /* search for existing setting */
  girara_setting_t* setting = girara_settings_manager_find(priv->settings, name);
  if (setting == NULL) {
    return false;
  }

  return girara_settings_manager_get_value(setting, dest);
}

girara_list_t*
girara_settings_manager_get_names(GiraraSettingsManager* manager, bool with_init_only)
{
  g_return_val_if_fail(manager != NULL, NULL);
  g_return_val_if_fail(GIRARA_IS_SETTINGS_MANAGER(manager) == TRUE, NULL);
  girara_settings_manager_private_t* priv = GIRARA_SETTINGS_MANAGER_GET_PRIVATE(manager);

  girara_list_t* result = girara_list_new2(g_free);
  if (result == NULL) {
    return NULL;
  }

  GIRARA_LIST_FOREACH(priv->settings, girara_setting_t*, iter, setting)
    if (with_init_only == false || setting->init_only == false) {
      girara_list_append(result, g_strdup(setting->name));
    }
  GIRARA_LIST_FOREACH_END(priv->settings, girara_setting_t*, iter, setting);

  return result;
}

const char*
girara_settings_manager_get_description(GiraraSettingsManager* manager,
    const char* name)
{
  g_return_val_if_fail(manager != NULL, NULL);
  g_return_val_if_fail(GIRARA_IS_SETTINGS_MANAGER(manager) == TRUE, NULL);
  girara_settings_manager_private_t* priv = GIRARA_SETTINGS_MANAGER_GET_PRIVATE(manager);
  g_return_val_if_fail(name != NULL, NULL);

  /* search for existing setting */
  girara_setting_t* setting = girara_settings_manager_find(priv->settings, name);
  if (setting == NULL) {
    return NULL;
  }
  return setting->description;
}

girara_setting_type_t
girara_settings_manager_type(GiraraSettingsManager* manager, const char* name)
{
  g_return_val_if_fail(manager != NULL, UNKNOWN);
  g_return_val_if_fail(GIRARA_IS_SETTINGS_MANAGER(manager) == TRUE, UNKNOWN);
  girara_settings_manager_private_t* priv = GIRARA_SETTINGS_MANAGER_GET_PRIVATE(manager);
  g_return_val_if_fail(name != NULL, UNKNOWN);

  /* search for existing setting */
  girara_setting_t* setting = girara_settings_manager_find(priv->settings, name);
  if (setting == NULL) {
    return UNKNOWN;
  }
  return setting->type;
}

bool
girara_settings_manager_exists(GiraraSettingsManager* manager,
    const char* name)
{
  g_return_val_if_fail(manager != NULL, false);
  g_return_val_if_fail(GIRARA_IS_SETTINGS_MANAGER(manager) == TRUE, false);
  girara_settings_manager_private_t* priv = GIRARA_SETTINGS_MANAGER_GET_PRIVATE(manager);
  g_return_val_if_fail(name != NULL, false);

  return girara_settings_manager_find(priv->settings, name) != NULL;
}


/* Helper function implementations */

static void
girara_setting_free(girara_setting_t* setting)
{
  if (setting == NULL) {
    return;
  }

  g_free(setting->name);
  g_free(setting->description);
  if (setting->type == STRING) {
    g_free(setting->value.s);
  }
  g_slice_free(girara_setting_t, setting);
}

static girara_setting_t*
girara_settings_manager_find(girara_list_t* list, const char* name)
{
  girara_setting_t* result = NULL;
  GIRARA_LIST_FOREACH(list, girara_setting_t*, iter, setting)
    if (g_strcmp0(setting->name, name) == 0) {
      result = setting;
      break;
    }
  GIRARA_LIST_FOREACH_END(list, girara_setting_t*, iter, setting);

  return result;
}

static int
cb_sort_settings(girara_setting_t* lhs, girara_setting_t* rhs)
{
  return g_strcmp0(lhs->name, rhs->name);
}

static void
girara_settings_manager_set_value(girara_setting_t* setting, void* value)
{
  g_return_if_fail(setting && (value || setting->type == STRING));

  switch (setting->type) {
    case BOOLEAN:
      setting->value.b = *((bool *) value);
      break;
    case FLOAT:
      setting->value.f = *((float *) value);
      break;
    case INT:
      setting->value.i = *((int *) value);
      break;
    case STRING:
      if (setting->value.s != NULL) {
        g_free(setting->value.s);
      }
      setting->value.s = value ? g_strdup(value) : NULL;
      break;
    default:
      g_assert(false);
  }
}

static bool
girara_settings_manager_get_value(girara_setting_t* setting, void* dest)
{
  g_return_val_if_fail(setting != NULL && dest != NULL, false);

  bool  *bvalue = (bool*) dest;
  float *fvalue = (float*) dest;
  int   *ivalue = (int*) dest;
  char **svalue = (char**) dest;

  switch(setting->type) {
    case BOOLEAN:
      *bvalue = setting->value.b;
      break;
    case FLOAT:
      *fvalue = setting->value.f;
      break;
    case INT:
      *ivalue = setting->value.i;
      break;
    case STRING:
      *svalue = setting->value.s ? g_strdup(setting->value.s) : NULL;
      break;
    default:
      g_assert(false);
  }

  return true;
}
