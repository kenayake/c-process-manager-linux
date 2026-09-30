#pragma once

#include "glib.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PROCESS_TYPE_ITEM (process_item_get_type())

G_DECLARE_FINAL_TYPE(ProcessItem, process_item, PROCESS, ITEM, GObject)

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

ProcessItem *process_item_new(pid_t pid, gboolean has_children);

ProcessItem *process_item_new_with_data(const gchar *name, const gchar *status,
                              const gdouble cpu, const gdouble memory,
                              const gdouble disk, const gchar *user,
                              const gchar *priority, pid_t pid, pid_t pgid, gboolean has_children);

GListModel *process_item_get_children(gpointer item, gpointer user_data);

void process_item_append_child(ProcessItem *item, ProcessItem *child);

G_END_DECLS