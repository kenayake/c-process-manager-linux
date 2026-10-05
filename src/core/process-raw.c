#include "core/process-raw.h"
#include "gio/gio.h"
#include "glib.h"
#include "glibconfig.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define PROCESS_RAW_PARSE_UID_ERROR process_raw_parse_uid_error_quark()
G_DEFINE_QUARK(process-raw-parse-uid-error-quark, process_raw_parse_uid_error)

enum {
  ProcessRawParseUIDError
} ErrorCodes;

static gchar *parse_comm(gchar *str) {
  g_autoptr(GRegex) regex;
  g_autofree GMatchInfo *match_info;
  gchar *match;

  regex = g_regex_new("\\((.*)\\)", 0, 0, NULL);

  if (g_regex_match(regex, str, 0, &match_info)) {
    match = g_match_info_fetch(match_info, 1);
  } else {
    match = str;
  }

  return match;
}

static gboolean parse_stat(long pid, ProcessRaw *data, GError **error) {
  g_auto(GStrv) stat_list = NULL;
  g_autofree gchar *contents = NULL;
  gsize length = 0;
  gint64 result;

  g_autofree gchar *filepath = g_strdup_printf("/proc/%ld/stat", pid);

  if (g_file_get_contents(filepath, &contents, &length, error)) {
    stat_list = g_strsplit_set(contents, " ", -1);

    data->pid = pid;
    data->comm = parse_comm(stat_list[1]);
    data->state = g_strdup(stat_list[2]);
    if (!g_ascii_string_to_signed(stat_list[4], 10, G_MINLONG, G_MAXLONG,
                                  &result, error)) {
      return FALSE;
    };
    data->pgrp = result;
    if (!g_ascii_string_to_signed(stat_list[17], 10, G_MINLONG, G_MAXLONG,
                                  &result, error)) {
      return FALSE;
    };
    data->priority = result;
    if (!g_ascii_string_to_signed(stat_list[18], 10, G_MINLONG, G_MAXLONG,
                                 &result, error)) {
      return FALSE;
    };
    data->nice = result;
    if (!g_ascii_string_to_signed(stat_list[18], 10, G_MINLONG, G_MAXLONG,
                                 &result, error)) {
      return FALSE;
    };
    if (!g_ascii_string_to_signed(stat_list[13], 10, G_MININT64, G_MAXINT64, &data->cpu_usage.utime, error)) {
      return FALSE;
    }
    if (!g_ascii_string_to_signed(stat_list[14], 10, G_MININT64, G_MAXINT64, &data->cpu_usage.stime, error)) {
      return FALSE;
    }

    // g_info("utime: %s, stime: %s", stat_list[13], stat_list[14]);

    return TRUE;
  }

  return FALSE;
}

static gboolean parse_uid(long pid, ProcessRaw *data, GError **error) {
  g_autofree gchar *filepath = g_strdup_printf("/proc/%ld/status", pid);

  GFile *file = g_file_new_for_path(filepath);
  GFileInputStream *in_stream = g_file_read(file, NULL, NULL);
  GDataInputStream *data_stream = g_data_input_stream_new(G_INPUT_STREAM(in_stream));

  char *line;
  gsize length;


  while ((line = g_data_input_stream_read_line(data_stream, &length, NULL, error)) != NULL) {

    if (g_ascii_strncasecmp(line, "Uid:", 4) == 0) {
      gint ruid, euid, suid, fsuid;
      // get effective uid
      sscanf(line + 4, "%d %d %d %d", &ruid, &euid, &suid, &fsuid);
      data->uid = ruid;

      return TRUE;
    }
  }

  g_set_error_literal(error, PROCESS_RAW_PARSE_UID_ERROR, ProcessRawParseUIDError, "Failed to find UID");

  return TRUE;
}

static gboolean parse_statm(long pid, ProcessRaw *data, GError **error) {
  int page_size = sysconf(_SC_PAGESIZE);
  g_autofree gchar *contents = NULL;
  gsize length = 0;
  g_auto(GStrv) statm_list = NULL;
  gint64 result;

  g_autofree gchar *filepath = g_strdup_printf("/proc/%ld/statm", pid);

  if (g_file_get_contents(filepath, &contents, &length, error)) {
    statm_list = g_strsplit_set(contents, " ", -1);

    g_ascii_string_to_signed(statm_list[1], 10, G_MINLONG, G_MAXLONG, &result,
                             error);
    data->rss = (double)(result * page_size);

    return TRUE;
  }

  return FALSE;
}

static gboolean parse_io(long pid, ProcessRaw *data) {
  GError *error = NULL;
  g_autofree gchar *contents = NULL;
  g_autofree gchar *filepath;
  gsize length = 0;

  // Get current time
  clock_gettime(CLOCK_MONOTONIC, &data->disk_usage.timestamp);

  filepath = g_strdup_printf("/proc/%ld/io", pid);

  // if proc/pid/io is readable
  if (g_file_get_contents(filepath, &contents, &length, &error)) {

    g_autoptr(GRegex) regex;
    GMatchInfo *match_info;
    gchar *match;

    // Parse read_bytes and write_bytes using regex
    regex = g_regex_new("(?<=read_bytes: )(\\d+)\nwrite_bytes: (\\d+)", 0, 0,
                        &error);
    if (g_regex_match(regex, contents, 0, &match_info)) {

      // Parse read_bytes
      match = g_match_info_fetch(match_info, 1);
      g_ascii_string_to_unsigned(match, 10, 0, G_MAXINT64,
                                 &data->disk_usage.read_bytes, &error);
      g_free(match);

      // Parse write_bytes
      match = g_match_info_fetch(match_info, 2);
      g_ascii_string_to_unsigned(match, 10, 0, G_MAXINT64,
                                 &data->disk_usage.write_bytes, &error);
      g_free(match);
    }

    return TRUE;
  }

  data->disk_usage.read_bytes = 0;
  data->disk_usage.write_bytes = 0;

  // g_warning("Failed get disk for process %ld: %s", pid, error->message);

  g_error_free(error);

  return FALSE;
};

void process_raw_free(gpointer data) {
  ProcessRaw *process = (ProcessRaw *)data;

  g_free(process->comm);
  g_free(process->state);

  if (process->has_children) {
    g_hash_table_unref(process->children);
  }

  g_free(process);
}

ProcessRaw *process_raw_scan(long *pid, GError **error) {
  ProcessRaw *process_raw = g_new0(ProcessRaw, 1);

  pid_t process_id = *pid;

  if (parse_stat(process_id, process_raw, error) == FALSE) {
    return NULL;
  }

  if (parse_statm(process_id, process_raw, error) == FALSE) {
    return NULL;
  }

  if (parse_uid(process_id, process_raw, error) == FALSE) {
    return NULL;
  }

  parse_io(process_id, process_raw);

  return process_raw;
}

ProcessRaw *process_raw_new(long *pid) {
  ProcessRaw *process_raw = g_new0(ProcessRaw, 1);

  process_raw->pid = *pid;

  return process_raw;
}

void process_raw_set_group_leader(ProcessRaw *process) {
  process->has_children = TRUE;
  process->children = g_hash_table_new_full(g_int64_hash, g_int64_equal, g_free,
                                            process_raw_free);
}

void process_raw_set_child(ProcessRaw *process) {
  process->has_children = FALSE;
  process->children = NULL;
}

void process_raw_copy(ProcessRaw *source_data, ProcessRaw *dest_data) {
  dest_data->pgrp = source_data->pgrp;
  dest_data->comm = source_data->comm;
  dest_data->disk_usage = source_data->disk_usage;
  dest_data->nice = source_data->nice;
  dest_data->priority = source_data->priority;
  dest_data->state = source_data->state;
  dest_data->rss = source_data->rss;
}

static void process_raw_print(gpointer key, gpointer data, gpointer user_data) {
  ProcessRaw *process = (ProcessRaw *)data;
  guint indent = GPOINTER_TO_UINT(user_data);

  g_autofree gchar *prefix = g_strnfill(indent * 2, ' ');

  g_info("%sPID: %d | Name: %s | status: %s | Children: %u",
         prefix, process->pid, process->comm ? process->comm : "(unknown)", process->state,
         process->has_children ? g_hash_table_size(process->children) : 0);

  if (process->has_children) {
    g_hash_table_foreach(process->children, process_raw_print, GUINT_TO_POINTER(indent + 2));
  }
}

void process_raw_print_hash_table(GHashTable *processes) {

  g_return_if_fail(processes != NULL);

  g_hash_table_foreach(processes, process_raw_print,  GUINT_TO_POINTER(0));
}