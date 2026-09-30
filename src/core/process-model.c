#include "config.h"
#include "process-model.h"
#include "core/process-item.h"
#include "gio/gio.h"
#include "glib-object.h"
#include <gtk/gtk.h>

GListStore *store;

static gboolean capture_processes(GError **error) {
  // Dummy implementation
  // TODO: Implement actual process capture with /proc
  ProcessItem *process;
  ProcessItem *child;

  process = process_item_new_with_data("Overwatch.exe", "R", 40.0, 3000.0, 0.1,
                             "kenayake", "Real-Time", 23, 23, FALSE);
  g_list_store_append(store, process);
  g_object_unref(process);

  process = process_item_new_with_data("systemd", "R", 0.5, 10.0, 0.1, "root", "Normal", 1, 1, FALSE);
  g_list_store_append(store, process);
  g_object_unref(process);

  process = process_item_new_with_data("explorer.exe", "S", 1.0, 100.0, 0.0, "kenayake",
                             "Normal", 12, 12, FALSE);
  g_list_store_append(store, process);
  g_object_unref(process);

  process = process_item_new_with_data("chrome.exe", "S", 2.0, 1500.0, 1.0,
                             "kenayake", "Normal", 14, 14, TRUE);

  child = process_item_new_with_data("tab1", "S", 0, 300, 0, "kenayake", "Low", 15, 14, FALSE);
  process_item_append_child(process, child);
  g_object_unref(child);
  child = process_item_new_with_data("tab2", "S", 0, 600, 0.1, "kenayake", "Low", 16, 14, FALSE);
  process_item_append_child(process, child);
  g_object_unref(child);

  g_list_store_append(store, process);
  g_object_unref(process);

  return TRUE;
}

GListModel *process_model_new(void) {
  GError *error;
  store = g_list_store_new(PROCESS_TYPE_ITEM);

  if (!(capture_processes(&error))) {
    g_warning("Failed get processes: %s",
                  error->message);

        g_error_free(error);
        return NULL;
  }

  return G_LIST_MODEL(store);
}

void process_model_refresh(void) {
  GError *error = NULL;

  g_list_store_remove_all(store);

  if (!capture_processes(&error)) {
     g_warning("Failed get processes: %s",
                  error->message);

        g_error_free(error);
  }
}