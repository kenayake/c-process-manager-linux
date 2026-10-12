#include "core/process-model.h"
#include "config.h"
#include "core/process-item.h"
#include "core/process-raw.h"
#include "core/system-status.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include "process-raw.h"
#include <bits/time.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>

static gboolean first_pass(GHashTable *exList, GError **error) {
  const gchar *filename;

  GDir *dir = g_dir_open("/proc", 0, error);
  if (!dir) {
    return FALSE;
  }

  // Loop files in /proc
  while ((filename = g_dir_read_name(dir)) != NULL) {
    g_autofree gint64 *pid = g_new0(gint64, 1);

    // get data in /proc/pid
    if (g_ascii_string_to_signed(filename, 10, G_MININT64, G_MAXINT64, pid,
                                 error) == TRUE) {
      ProcessRaw *process_raw;
      ProcessRaw *existing_group_leader = NULL;

      // create raw process data container
      if ((process_raw = process_raw_scan(*pid, error)) == NULL) {
        g_dir_close(dir);
        return FALSE;
      }

      if (g_strcmp0(process_raw->comm, "init-systemd(Ub") == 0) {
        // process_raw_print_hash_table(exList);
      }

      // Check if process is group leader
      else if (process_raw->pid == process_raw->pgrp) {

        // If exList contains group leader already, skip it
        if (g_hash_table_lookup_extended(exList, pid, NULL,
                                         (gpointer *)&existing_group_leader) == TRUE) {
          process_raw_free(process_raw);

        } else {
          // Else, insert new process as process group leader
          // process_raw_set_group_leader(process_raw);
          g_hash_table_insert(exList, g_memdup2(pid, sizeof *pid), process_raw);
        }

      } else {
        g_autofree gint64 *pgrp = g_new(long, 1);;

        // If process is not group leader, set as child
        process_raw_set_child(process_raw);

        *pgrp = process_raw->pgrp;

        // Check if group leader whose pid matches the child's pgid exists
        if (g_hash_table_lookup_extended(exList, pgrp, NULL,
                                         (gpointer *)&existing_group_leader) == TRUE) {

          if (!existing_group_leader->has_children) {
            existing_group_leader->has_children = TRUE;
            existing_group_leader->children = g_hash_table_new_full(g_int64_hash, g_int64_equal, g_free,
                                            process_raw_free);
          }

          // Add child stats to parent
          process_raw_add_child_metrics_to_parent(process_raw, existing_group_leader);
                                          
          // Append child to existing group leader
          g_hash_table_insert(existing_group_leader->children, g_memdup2(pid, sizeof *pid),
                              process_raw);
        } else {

          // If the child's group leader does not exist yet, create it first
          ProcessRaw *group_leader;
          if ((group_leader = process_raw_scan(*pgrp, error)) != NULL) {
            process_raw_set_group_leader(group_leader);

            // Add child stats to parent
            process_raw_add_child_metrics_to_parent(process_raw, group_leader);

            // Append child to the new group leader and insert to exList
            g_hash_table_insert(group_leader->children, g_memdup2(pid, sizeof *pid), process_raw);
            g_hash_table_insert(exList, g_memdup2(pgrp, sizeof *pgrp), group_leader);
          } else {
            // if process with pgrp does not exist, set child as leader
            // g_info("Process with PID: %ld does not exist", *pid);

            g_hash_table_insert(exList, g_memdup2(pid, sizeof *pid), process_raw);
          }
        }
      }
    }

    g_clear_error(error);
  }

  g_dir_close(dir);
  return TRUE;
}

static void second_pass(GListModel *loList, GHashTable *exList) {                        
  guint size = g_list_model_get_n_items(loList);
  guint64 *cpu_delta = g_object_get_data(G_OBJECT(loList), "cpu-delta");

  if (size == 0) {
    return;
  }

  // Get local group item
  for (int i = size - 1; i >= 0; i--) {
    g_autofree gint64 *group_item_id = g_new(gint64, 1);
    g_autofree const gchar *item_name;
    GListModel *group_item_children;
    ProcessRaw *process_raw;

    g_autoptr(PmProcessItem) item = (PmProcessItem *)g_list_model_get_object(loList, i);
    g_object_get(item, "pid", group_item_id, "name", &item_name, NULL);
    
    if (g_strcmp0(item_name, "disk_write_test") == 0) {
      // process_item_print_model(loList, "");
      process_raw_print_hash_table(exList);
    }

    // if process group leader exists in external list
    if (g_hash_table_lookup_extended(exList, group_item_id, NULL,
                                     (gpointer *)&process_raw) == TRUE) {

      // Check if group leader has any children for both lists
      if ((group_item_children = process_item_get_children(item, NULL)) != NULL && process_raw->has_children == TRUE) {
        guint children_size = g_list_model_get_n_items(group_item_children);

        // loop local children
        for (int j = children_size - 1; j >= 0; j--) {
          g_autofree gint64 *child_item_id = g_new(gint64, 1);
          ProcessRaw *child_process_raw;

          g_autoptr(PmProcessItem) child = (PmProcessItem *)g_list_model_get_object(group_item_children, j);
          g_object_get(child, "pid", child_item_id, NULL);
          
          // If child exists on external list
          if (g_hash_table_lookup_extended(process_raw->children, child_item_id, NULL,
                                     (gpointer *)&child_process_raw) == TRUE) {
            // Update local child data
            process_item_update_from_raw(child, child_process_raw, cpu_delta);                          
          } else {
            // else, delete the local child
            g_list_store_remove(G_LIST_STORE(group_item_children), j);
          }
        }

        g_object_unref(group_item_children);
      }

      // Update group leader
      process_item_update_from_raw(item, process_raw, cpu_delta);
    } else {
      // Process group leader is redundant, delete it and its children
      g_list_store_remove(G_LIST_STORE(loList), i);
    }
  }
}

static void third_pass(gpointer key, gpointer value, gpointer user_data) {
  ProcessRaw *raw = (ProcessRaw *)value;
  GListModel *local_list = G_LIST_MODEL(user_data);
  g_autoptr(PmProcessItem) item = NULL;

  if (g_strcmp0(raw->comm, "disk_write_test") == 0) {
    process_item_print_model(local_list, "");
  }

  // Check if raw item exists in local list and external list
  if (process_item_model_contains(local_list, raw->pid, &item)) {
    // If both have children
    if (raw->has_children && process_item_has_children(item)) {
      g_autoptr(GListModel) item_children;
      GHashTable *raw_children;

      // Sync children
      item_children = process_item_get_children(item, NULL);
      raw_children = raw->children;

      g_hash_table_foreach(raw_children, process_item_sync_children_with_raw, item_children);
    } else if (raw->has_children) {
      // raw item has new children
      process_item_set_parent(item);
    } else if (process_item_has_children(item)) {
      // raw group leader no longer has any children
      process_item_set_single(item);
    }
    
  } else {
    // If local list do not have raw item
    // Create new local item and add all children
    item = process_item_new_from_raw(raw);
    if (raw->has_children) {
      g_autoptr(GListModel) children = process_item_get_children(item, NULL);
      g_hash_table_foreach(raw->children, process_item_sync_children_with_raw, children);
    }
    g_list_store_append(G_LIST_STORE(local_list), item);
  }
}

static gboolean capture_process_data(GListStore *store, GError **error) {
  g_autoptr(GHashTable) exList = g_hash_table_new_full(g_int64_hash, g_int64_equal,
                                             g_free, process_raw_free);

  // Perform the first pass: get data from proc and put it to external list
  if (!first_pass(exList, error)) {
    return FALSE;
  } 
  // process_raw_print_hash_table(exList);

  // Second pass: sync local list with external list for update and delete
  second_pass(G_LIST_MODEL(store), exList);

  // Third pass: add items not in local list
  g_hash_table_foreach(exList, third_pass, store);

  return TRUE;
}

GListModel *process_model_new(void) {
  GListStore *store;
  GError *error = NULL;
  store = g_list_store_new(PM_TYPE_PROCESS_ITEM);

  if (!(capture_process_data(store, &error))) {
    g_warning("Failed get processes: %s", error->message);

    g_error_free(error);
  }

  return G_LIST_MODEL(store);
}

void process_model_refresh(GListStore *store) {
  GError *error = NULL;

  // g_info("refreshing");

  if (!capture_process_data(store, &error)) {
    g_warning("Failed get processes: %s", error->message);

    g_error_free(error);
  }

}