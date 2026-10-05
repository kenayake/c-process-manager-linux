#include "core/process-model.h"
#include "config.h"
#include "core/process-item.h"
#include "core/process-raw.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include "process-raw.h"
#include "util/timespec-type.h"
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
    long *pid;
    pid = g_new0(long, 1);

    if (g_strcmp0(filename, "stat") == 0) {
      // TODO: Get system status
    }

    // get data in /proc/pid
    if (g_ascii_string_to_signed(filename, 10, G_MINLONG, G_MAXLONG, pid,
                                 error) == TRUE) {
      ProcessRaw *process_raw;
      ProcessRaw *existing_group_leader = NULL;

      // create raw process data container
      if ((process_raw = process_raw_scan(pid, error)) == NULL) {
        return FALSE;
      }

      // Check if process is group leader
      if (process_raw->pid == process_raw->pgrp) {

        // If exList contains group leader already,
        // update existing group leader with process data
        if (g_hash_table_lookup_extended(exList, pid, NULL,
                                         (gpointer *)&existing_group_leader)) {
          process_raw_copy(process_raw, existing_group_leader);
          process_raw_free(process_raw);

        } else {
          // Else, insert new process as process group leader
          // process_raw_set_group_leader(process_raw);
          g_hash_table_insert(exList, pid, process_raw);
        }

      } else {
        long *pgrp = NULL;

        // If process is not group leader, set as child
        process_raw_set_child(process_raw);

        pgrp = g_new(long, 1);
        *pgrp = process_raw->pgrp;

        // Check if group leader whose pid matches the child's pgid exists
        if (g_hash_table_lookup_extended(exList, pgrp, NULL,
                                         (gpointer *)&existing_group_leader)) {

          if (!existing_group_leader->has_children) {
            existing_group_leader->has_children = TRUE;
            existing_group_leader->children = g_hash_table_new_full(g_int64_hash, g_int64_equal, g_free,
                                            process_raw_free);
          }
                                          
          // Append child to existing group leader
          g_hash_table_insert(existing_group_leader->children, pid,
                              process_raw);
        } else {

          // If the child's group leader does not exist yet, create it first
          ProcessRaw *group_leader = process_raw_new(pgrp);
          process_raw_set_group_leader(group_leader);

          // Append child to the new group leader
          g_hash_table_insert(group_leader->children, pid, process_raw);
        }
      }
    }

    g_clear_error(error);
  }

  return TRUE;
}

static void second_pass(GListModel *loList, GHashTable *exList) {                        
  guint size = g_list_model_get_n_items(loList);

  if (size == 0) {
    return;
  }

  // Get local group item
  for (int i = size - 1; i >= 0; i--) {
    pid_t group_item_id;
    GListModel *group_item_children;
    ProcessRaw *process_raw;

    ProcessItem *item = (ProcessItem *)g_list_model_get_object(loList, i);
    g_object_get(item, "pid", &group_item_id, NULL);

    // if process group leader exists in external list
    if (g_hash_table_lookup_extended(exList, &group_item_id, NULL,
                                     (gpointer *)&process_raw)) {

      // Check if group leader has any children for both lists
      if ((group_item_children = process_item_get_children(item, NULL)) != NULL && process_raw->has_children == TRUE) {
        guint children_size = g_list_model_get_n_items(group_item_children);

        // loop local children
        for (int j = children_size - 1; j >= 0; j--) {
          pid_t child_item_id;
          ProcessRaw *child_process_raw;

          ProcessItem *child = (ProcessItem *)g_list_model_get_object(group_item_children, j);
          g_object_get(item, "pid", &child_item_id, NULL);
          
          // If child exists on external list
          if (g_hash_table_lookup_extended(process_raw->children, &child_item_id, NULL,
                                     (gpointer *)&child_process_raw)) {
            // Update local child data
            process_item_update_from_raw(child, child_process_raw);                          
          } else {
            // else, delete the local child
            g_list_store_remove(G_LIST_STORE(group_item_children), j);
          }
        }
      }

      // Update group leader
      process_item_update_from_raw(item, process_raw);
    } else {
      // Process group leader is redundant, delete it and its children
      g_list_store_remove(G_LIST_STORE(loList), i);
    }
  }
}

static void third_pass(gpointer key, gpointer value, gpointer user_data) {
  ProcessRaw *raw = (ProcessRaw *)value;
  GListModel *local_list = G_LIST_MODEL(user_data);
  ProcessItem *item = NULL;

  // Check if raw item exists in local list and external list
  if (process_item_model_contains(local_list, raw->pid, &item)) {
    // If both have children
    if (raw->has_children && process_item_has_children(item)) {
      GListModel *item_children;
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
      g_hash_table_foreach(raw->children, process_item_sync_children_with_raw, process_item_get_children(item, NULL));
    }
    g_list_store_append(G_LIST_STORE(local_list), item);
  }
}

static gboolean capture_process_data(GListStore *store, GError **error) {
  GHashTable *exList = g_hash_table_new_full(g_int64_hash, g_int64_equal,
                                             g_free, process_raw_free);

  // Perform the first pass: get data from proc and put it to external list
  if (!first_pass(exList, error))
    return FALSE;
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
  store = g_list_store_new(PROCESS_TYPE_ITEM);

  if (!(capture_process_data(store, &error))) {
    g_warning("Failed get processes: %s", error->message);

    g_error_free(error);
    return NULL;
  }

  return G_LIST_MODEL(store);
}

void process_model_refresh(GListStore *store) {
  GError *error = NULL;

  g_info("refreshing");

  if (!capture_process_data(store, &error)) {
    g_warning("Failed get processes: %s", error->message);

    g_error_free(error);
  }

}