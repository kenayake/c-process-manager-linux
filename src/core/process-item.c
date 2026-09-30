#include "process-item.h"
#include "config.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include <gtk/gtk.h>

G_DEFINE_FINAL_TYPE(ProcessItem, process_item, G_TYPE_OBJECT)

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

  object_class->finalize = process_item_finalize;
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

ProcessItem *process_item_new(pid_t pid, gboolean has_children) {

  ProcessItem *process_item = g_object_new(PROCESS_TYPE_ITEM, NULL, NULL);

  process_item->pid = pid;

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
                              const gchar *priority, pid_t pid, pid_t pgid, gboolean has_children) {

  ProcessItem *process_item = g_object_new(PROCESS_TYPE_ITEM, NULL, NULL);

  process_item->name = g_strdup(name);
  process_item->pid = pid;
  process_item->cpu = cpu;
  process_item->memory = memory;
  process_item->disk = disk;
  process_item->user = g_strdup(user);
  process_item->priority = g_strdup(priority);

  if (has_children) {
    process_item->children = g_list_store_new(PROCESS_TYPE_ITEM);
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