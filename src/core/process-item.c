#include "process-item.h"
#include "config.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include "process-raw.h"
#include "util/cpu-usage-type.h"
#include "util/disk-usage-type.h"
#include <bits/time.h>
#include <gtk/gtk.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>

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

  CpuUsage cpu_usage;
  DiskUsage disk_usage;

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
  PROP_PID,
  PROP_PGID,
  PROP_CPU_USAGE,
  PROP_DISK_USAGE,
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

  case PROP_PID:
    self->pid = g_value_get_long(value);
    break;

  case PROP_PGID:
    self->pgid = g_value_get_long(value);
    break;

  case PROP_CPU_USAGE:
    self->cpu_usage = *(CpuUsage *)g_value_get_boxed(value);
    break;

  case PROP_DISK_USAGE:
    self->disk_usage = *(DiskUsage *)g_value_get_boxed(value);
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

  case PROP_PID:
    g_value_set_long(value, self->pid);
    break;

  case PROP_PGID:
    g_value_set_long(value, self->pgid);
    break;

  case PROP_CPU_USAGE:
    g_value_set_boxed(value, &self->cpu_usage);
    break;

  case PROP_DISK_USAGE:
    g_value_set_boxed(value, &self->disk_usage);
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
  g_clear_object(&self->children);
  G_OBJECT_CLASS(process_item_parent_class)->finalize(object);
}

static void process_item_class_init(ProcessItemClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = process_item_set_property;
  object_class->get_property = process_item_get_property;
  object_class->finalize = process_item_finalize;

  properties[PROP_NAME] =
      g_param_spec_string("name", "Name", "The process name", NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_STATUS] =
      g_param_spec_string("status", "Status", "The process status", NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  // NOTE: Subject to change
  properties[PROP_CPU] = g_param_spec_double(
      "cpu", "CPU", "The process CPU usage", 0.0, G_MAXDOUBLE, 0.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_MEMORY] = g_param_spec_double(
      "memory", "Memory", "The process memory usage", 0.0, G_MAXDOUBLE, 0.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_DISK] = g_param_spec_double(
      "disk", "Disk", "The process disk usage", 0.0, G_MAXDOUBLE, 0.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_USER] =
      g_param_spec_string("user", "User", "The process owner", NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_PRIORITY] =
      g_param_spec_string("priority", "Priority", "The process priority", NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_PID] =
      g_param_spec_long("pid", "PID", "The process ID", G_MINLONG, G_MAXLONG, 0,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_PGID] = g_param_spec_long(
      "pgid", "PGID", "The process group ID", G_MINLONG, G_MAXLONG, 0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_CPU_USAGE] = g_param_spec_boxed(
      "cpu-usage", "CPU Usage", "CPU usage information", TYPE_CPU_USAGE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  properties[PROP_DISK_USAGE] = g_param_spec_boxed(
      "disk-usage", "Disk Usage", "Disk usage information", TYPE_DISK_USAGE,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPERTIES, properties);
}

static void process_item_init(ProcessItem *self) {
  self->name = NULL;
  self->status = NULL;
  self->user = NULL;
  self->priority = NULL;
  self->cpu = 0.0;
  self->disk = 0.0;
  self->memory = 0.0;
}

ProcessItem *process_item_new(pid_t pid, gboolean has_children) {

  ProcessItem *process_item = g_object_new(PROCESS_TYPE_ITEM, "pid", pid, NULL);

  if (has_children) {
    process_item->children = g_list_store_new(PROCESS_TYPE_ITEM);
    process_item->has_children = TRUE;
  } else {
    process_item->has_children = FALSE;
  }

  return process_item;
}

ProcessItem *process_item_new_with_data(const gchar *name, const gchar *status,
                                        const gdouble cpu, const gdouble memory,
                                        const gdouble disk, const gchar *user,
                                        const gchar *priority, pid_t pid,
                                        pid_t pgid, gboolean has_children) {

  ProcessItem *process_item =
      g_object_new(PROCESS_TYPE_ITEM, "name", name, "status", status, "cpu",
                   cpu, "memory", memory, "disk", disk, "user", user,
                   "priority", priority, "pid", pid, "pgid", pgid, NULL);

  if (has_children) {
    process_item->children = g_list_store_new(PROCESS_TYPE_ITEM);
    process_item->has_children = TRUE;
  } else {
    process_item->has_children = FALSE;
  }

  return process_item;
}

ProcessItem *process_item_new_from_raw(ProcessRaw *raw) {

  struct passwd *user_entry = getpwuid(raw->uid);
  g_autofree gchar* user = g_strdup(user_entry->pw_name);

  ProcessItem *process_item = g_object_new(
    PROCESS_TYPE_ITEM, "name", raw->comm, "status", raw->state, "cpu", 0.0,
    "memory", raw->rss / (1024 * 1024), "disk", 0.0, "priority",
    (raw->priority + raw->nice) < 0 ? "Real-Time" : "Normal", "pid", raw->pid,
    "pgid", raw->pgrp, "user", user, NULL);

  process_item->has_children = raw->has_children;
  if (process_item->has_children) {
    process_item->children = g_list_store_new(PROCESS_TYPE_ITEM);
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

void process_item_update_from_raw(ProcessItem *item, ProcessRaw *raw) {
  double mem = raw->rss / (1024 * 1024);

  struct passwd *user_entry = getpwuid(raw->uid);

  int interval = 1; // replace with refresh_secs setting later

  // Calculate cpu usage percent
  long clock_tick = sysconf(_SC_CLK_TCK);
  long cores = sysconf(_SC_NPROCESSORS_ONLN);
  gint64 cpu_new = raw->cpu_usage.utime + raw->cpu_usage.stime;
  gint64 cpu_old = item->cpu_usage.utime + item->cpu_usage.stime;
  double cpu = (((double)cpu_new - cpu_old) / clock_tick * interval) *
              100; // divide again by refresh_secs setting later

  // Calculate disk usage
  guint64 disk_old = item->disk_usage.read_bytes + item->disk_usage.write_bytes;
  guint64 disk_new = raw->disk_usage.read_bytes + raw->disk_usage.write_bytes;
  double disk = ((double)disk_new - disk_old) / (interval * 1024 * 1024);

  g_object_set(item, "pgid", raw->pgrp, "name", raw->comm, "status", raw->state, "priority",
              (raw->priority + raw->nice) < 0 ? "Real-Time" : "Normal", "user",
              user_entry->pw_name, "memory", mem, "cpu", cpu, "disk", disk, NULL);

  // update member structs
  item->cpu_usage = raw->cpu_usage;
  item->disk_usage = raw->disk_usage;
}

static gboolean find_process(gconstpointer a, gconstpointer b,
                             gpointer user_data) {
  ProcessItem *item = (ProcessItem *)a;
  pid_t *pid = (pid_t *)user_data;

  if (item->pid == *pid) {
    return TRUE;
  }

  return FALSE;
}

gboolean process_item_model_contains(GListModel *list, pid_t pid,
                                     ProcessItem **found_item) {
  guint position;

  if (g_list_store_find_with_equal_func_full(G_LIST_STORE(list), NULL,
                                             find_process, &pid, &position)) {
    ProcessItem *item = PROCESS_ITEM(g_list_model_get_object(list, position));

    *found_item = item;

    return TRUE;
  };

  return FALSE;
}

void process_item_sync_children_with_raw(gpointer key, gpointer value,
                                         gpointer user_data) {
  ProcessRaw *child_raw = (ProcessRaw *)value;
  GListStore *store = G_LIST_STORE(user_data);
  g_autoptr(ProcessItem) item = NULL;

  if (process_item_model_contains(G_LIST_MODEL(store), child_raw->pid, &item)) {
    process_item_update_from_raw(item, child_raw);
  } else {
    item = process_item_new_from_raw(child_raw);
    g_list_store_append(store, item);
  }
}

gboolean process_item_has_children(ProcessItem *item) {
  return item->has_children;
}

void process_item_set_single(ProcessItem *item) {
  if (item->has_children) {
    GListStore *children = item->children;

    g_list_store_remove_all(children);
    g_object_unref(children);
    item->children = NULL;
    item->has_children = FALSE;
  }
}

void process_item_set_parent(ProcessItem *item) {
  if (!item->has_children) {
    item->children = g_list_store_new(PROCESS_TYPE_ITEM);
    item->has_children = TRUE;
  }
}

void process_item_print_model(GListModel *list, const gchar *prefix) {
  guint size = g_list_model_get_n_items(list);
  for (int i = 0; i < size; i++) {
    g_autoptr(ProcessItem) process =
        PROCESS_ITEM(g_list_model_get_item(list, i));

    g_info("%sPID: %d | Name: %s | status: %s | Mem: %0.3f | CPU: %0.3f | "
           "Disk: %0.3f | Children: %u",
           prefix, process->pid, process->name ? process->name : "(unknown)",
           process->status, process->memory, process->cpu, process->disk,
           process->has_children
               ? g_list_model_get_n_items(G_LIST_MODEL(process->children))
               : 0);

    if (process->has_children) {
      process_item_print_model(G_LIST_MODEL(process->children), "   ");
    }
  }
}