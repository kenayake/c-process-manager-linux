#include "config.h"
#include "process-model.h"
#include "core/process-item.h"
#include "gio/gio.h"
#include <gtk/gtk.h>

GListStore *store;

static gboolean capture_processes(GError **error) {
  // Dummy implementation
  // TODO: Implement actual process capture with /proc
  ProcessItem *process;

  process = process_item_new("Overwatch.exe", "R", 40.0, 3000.0, 0.1,
                             "kenayake", "Real-Time");
  g_list_store_append(store, process);
  g_object_unref(process);

  process = process_item_new("systemd", "R", 0.5, 10.0, 0.1, "root", "Normal");
  g_list_store_append(store, process);
  g_object_unref(process);

  process = process_item_new("vscode.exe", "S", 1.0, 100.0, 0.0, "kenayake",
                             "Normal");
  g_list_store_append(store, process);
  g_object_unref(process);

  process = process_item_new("Overwatch.exe", "R", 40.0, 3000.0, 0.1,
                             "kenayake", "Normal");
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