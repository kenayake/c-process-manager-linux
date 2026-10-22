#pragma once

#include "core/system-status.h"
#include "glib.h"
#include "process-raw.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_PROCESS_ITEM (pm_process_item_get_type())

G_DECLARE_FINAL_TYPE(PmProcessItem, pm_process_item, PM, PROCESS_ITEM, GObject)

PmProcessItem *process_item_new(pid_t pid, gboolean has_children);

PmProcessItem *process_item_new_with_data(const gchar *name, const gchar *status,
                                        const gdouble cpu, const gdouble memory,
                                        const gdouble disk, const gchar *user,
                                        const gchar *priority, pid_t pid,
                                        pid_t pgid, gboolean has_children);

GListModel *process_item_get_children(gpointer item, gpointer user_data);

void process_item_append_child(PmProcessItem *item, PmProcessItem *child);

void process_item_update_from_raw(PmProcessItem *item, ProcessRaw *raw, guint64 *cpu_usage_time);

gboolean process_item_model_contains(GListModel *list, pid_t pid, PmProcessItem **found_item);

PmProcessItem *process_item_new_from_raw(ProcessRaw *raw);

void process_item_sync_children_with_raw(gpointer key, gpointer value, gpointer user_data);

gboolean process_item_has_children(PmProcessItem *item);

void process_item_set_single(PmProcessItem *item);

void process_item_set_parent(PmProcessItem *item);

void process_item_print_model(GListModel *list, const gchar *prefix);

gboolean process_item_open_folder(PmProcessItem *self, GError **error);

gboolean process_item_kill_process(PmProcessItem *self, GError **error);

void process_item_force_stop(PmProcessItem *self);

G_END_DECLS