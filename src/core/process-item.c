#include "config.h"
#include "process-item.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include <gtk/gtk.h>

struct _ProcessItem {
  GObject parent_instance;

  gchar *name;
  pid_t pid;
  pid_t pgid;
  gchar *status;
  gdouble cpu;
  gdouble memory;
  gdouble disk;
  gchar *user;
  gchar *priority;
  gboolean has_children;

  GListStore *children;
};

G_DEFINE_FINAL_TYPE(ProcessItem, process_item, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_NAME,
  PROP_STATUS,
  PROP_CPU,
  PROP_MEMORY,
  PROP_DISK,
  PROP_USER,
  PROP_PRIORITY,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

static void process_item_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {
  ProcessItem *self = PROCESS_ITEM(object);

  switch (property_id) {
  case PROP_NAME:
    g_free(self->name);
    self->name = g_value_dup_string(value);
    break;

  case PROP_STATUS:
    g_free(self->status);
    self->status = g_value_dup_string(value);
    break;

  case PROP_CPU:
    self->cpu = g_value_get_double(value);
    break;

  case PROP_MEMORY:
    self->memory = g_value_get_double(value);
    break;

  case PROP_DISK:
    self->disk = g_value_get_double(value);
    break;

  case PROP_USER:
    g_free(self->user);
    self->user = g_value_dup_string(value);
    break;

  case PROP_PRIORITY:
    g_free(self->priority);
    self->priority = g_value_dup_string(value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void process_item_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {
  ProcessItem *self = PROCESS_ITEM(object);

  switch (property_id) {
  case PROP_NAME:
    g_value_set_string(value, self->name);
    break;

  case PROP_STATUS:
    g_value_set_string(value, self->status);
    break;

  case PROP_CPU:
    g_value_set_double(value, self->cpu);
    break;

  case PROP_MEMORY:
    g_value_set_double(value, self->memory);
    break;

  case PROP_DISK:
    g_value_set_double(value, self->disk);
    break;

  case PROP_USER:
    g_value_set_string(value, self->user);
    break;

  case PROP_PRIORITY:
    g_value_set_string(value, self->priority);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void process_item_finalize(GObject *object) {
  ProcessItem *self = PROCESS_ITEM(object);

  g_free(self->name);
  g_free(self->status);
  g_free(self->user);
  g_free(self->priority);
  g_clear_object (&self->children);
  G_OBJECT_CLASS (process_item_parent_class)->finalize (object);
}

static void process_item_class_init(ProcessItemClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = process_item_set_property;
  object_class->get_property = process_item_get_property;
  object_class->finalize = process_item_finalize;

  properties[PROP_NAME] = g_param_spec_string(
      "name", "Name", "The process name", NULL, G_PARAM_READWRITE);

  properties[PROP_STATUS] = g_param_spec_string(
      "status", "Status", "The process status", NULL, G_PARAM_READWRITE);

  // NOTE: Subject to change
  properties[PROP_CPU] =
      g_param_spec_double("cpu", "CPU", "The process CPU usage", 0.0,
                          G_MAXDOUBLE, 0.0, G_PARAM_READWRITE);
  properties[PROP_MEMORY] =
      g_param_spec_double("memory", "Memory", "The process memory usage", 0.0,
                          G_MAXDOUBLE, 0.0, G_PARAM_READWRITE);
  properties[PROP_DISK] =
      g_param_spec_double("disk", "Disk", "The process disk usage", 0.0,
                          G_MAXDOUBLE, 0.0, G_PARAM_READWRITE);

  properties[PROP_USER] = g_param_spec_string(
      "user", "User", "The process owner", NULL, G_PARAM_READWRITE);
  properties[PROP_PRIORITY] = g_param_spec_string(
      "priority", "Priority", "The process priority", NULL, G_PARAM_READWRITE);

  g_object_class_install_properties(object_class, N_PROPERTIES, properties);
}

static void process_item_init(ProcessItem *self) {
  self->name = NULL;
  self->status = NULL;
  self->cpu = 0;
  self->memory = 0;
  self->disk = 0;
  self->user = NULL;
  self->priority = NULL;
}

ProcessItem *process_item_new(const gchar *name, const gchar *status,
                              const gdouble cpu, const gdouble memory,
                              const gdouble disk, const gchar *user,
                              const gchar *priority, pid_t pid, pid_t pgid, gboolean has_children) {
  ProcessItem *process_item = g_object_new(
      PROCESS_TYPE_ITEM, "name", name, "status", status, "cpu", cpu, "memory",
      memory, "disk", disk, "user", user, "priority", priority, NULL);
  
  process_item->pid = pid;
  process_item->pgid = pgid;
  
  if (has_children) {
    process_item->children = g_list_store_new (PROCESS_TYPE_ITEM);
    process_item->has_children = TRUE;
  } else {
    process_item->has_children = FALSE;
  }

  return process_item;
}

GListModel *process_item_get_children(gpointer item, gpointer user_data) {
  ProcessItem *process_item = PROCESS_ITEM(item);

  if (process_item->has_children) {
    return G_LIST_MODEL(g_object_ref(process_item->children));
  }

  return NULL;
}

void process_item_append_child(ProcessItem *item, ProcessItem *child) {
  g_list_store_append(item->children, child);
}