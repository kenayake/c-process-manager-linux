#pragma once

#include "glib.h"
#include "util/cpu-usage-type.h"
#include "util/disk-usage-type.h"
#include <sys/types.h>
#include <time.h>

typedef struct {
  pid_t pid;
  gchar *comm;
  gchar *state;
  pid_t pgrp;
  gint priority;
  gint nice;
  gdouble rss;
  gboolean has_children;
  uid_t uid;

  DiskUsage disk_usage;
  CpuUsage cpu_usage;

  GHashTable *children;
} ProcessRaw;

ProcessRaw *process_raw_scan(long *pid, GError **error);

ProcessRaw *process_raw_new(long *pid);

void process_raw_free(gpointer data);

void process_raw_set_group_leader(ProcessRaw *process);

void process_raw_set_child(ProcessRaw *process);

void process_raw_copy(ProcessRaw *source_data, ProcessRaw *dest_data);

void process_raw_print_hash_table(GHashTable *processes);