#pragma once

#include "glib.h"
#include "process-raw.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PROCESS_TYPE_ITEM (process_item_get_type())

G_DECLARE_FINAL_TYPE(ProcessItem, process_item, PROCESS, ITEM, GObject)

ProcessItem *process_item_new(pid_t pid, gboolean has_children);

ProcessItem *process_item_new_with_data(const gchar *name, const gchar *status,
                                        const gdouble cpu, const gdouble memory,
                                        const gdouble disk, const gchar *user,
                                        const gchar *priority, pid_t pid,
                                        pid_t pgid, gboolean has_children);

GListModel *process_item_get_children(gpointer item, gpointer user_data);

void process_item_append_child(ProcessItem *item, ProcessItem *child);

void process_item_update_from_raw(ProcessItem *item, ProcessRaw *raw);

gboolean process_item_model_contains(GListModel *list, pid_t pid, ProcessItem **found_item);

ProcessItem *process_item_new_from_raw(ProcessRaw *raw);

void process_item_sync_children_with_raw(gpointer key, gpointer value, gpointer user_data);

gboolean process_item_has_children(ProcessItem *item);

void process_item_set_single(ProcessItem *item);

void process_item_set_parent(ProcessItem *item);

G_END_DECLS