#include "system-status.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include <stdio.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>

#define SYSTEM_STATUS_SYSINFO_ERROR system_status_sysinfo_error_quark()
G_DEFINE_QUARK(system-status-sysinfo-error-quark, system_status_sysinfo_error)

enum {
  SystemStatusSysinfoError
} SystemStatusErrorCodes;

struct _PmSystemStatus {
  GObject parent_instance;

  guint64 mem_free;
  guint64 mem_total;
  gdouble cpu_usage;
  guint64 cpu_time;
  guint64 cpu_used;
  gdouble disk_usage;
  guint64 total_read_byte;
  guint64 total_write_byte;
  guint64 io_time;
  guint64 cpu_delta;
};

G_DEFINE_FINAL_TYPE(PmSystemStatus, pm_system_status, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_MEM_FREE,
  PROP_MEM_TOTAL,
  PROP_CPU_TIME,
  PROP_CPU_USED,
  PROP_TOTAL_READ_BYTE,
  PROP_TOTAL_WRITE_BYTE,
  PROP_IO_TIME,
  PROP_CPU_DELTA,
  PROP_CPU_USAGE,
  PROP_DISK_USAGE,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

static void pm_system_status_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {
  PmSystemStatus *self = PM_SYSTEM_STATUS(object);

  switch (property_id) {
  case PROP_MEM_FREE:
    self->mem_free = g_value_get_uint64(value);
    break;
  case PROP_MEM_TOTAL:
    self->mem_total = g_value_get_uint64(value);
    break;
  case PROP_CPU_TIME:
    self->cpu_time = g_value_get_uint64(value);
    break;
  case PROP_CPU_USED:
    self->cpu_used = g_value_get_uint64(value);
    break;
  case PROP_TOTAL_READ_BYTE:
    self->total_read_byte = g_value_get_uint64(value);
    break;
  case PROP_TOTAL_WRITE_BYTE:
    self->total_write_byte = g_value_get_uint64(value);
    break;
  case PROP_IO_TIME:
    self->io_time = g_value_get_uint64(value);
    break;
  case PROP_CPU_DELTA:
    self->io_time = g_value_get_uint64(value);
    break;
  case PROP_CPU_USAGE:
    self->cpu_usage = g_value_get_double(value);
    break;
  case PROP_DISK_USAGE:
    self->disk_usage = g_value_get_double(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pm_system_status_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {
  PmSystemStatus *self = PM_SYSTEM_STATUS(object);

  switch (property_id) {
  case PROP_MEM_FREE:
    g_value_set_uint64(value, self->mem_free);
    break;
  case PROP_MEM_TOTAL:
    g_value_set_uint64(value, self->mem_total);
    break;
  case PROP_CPU_TIME:
    g_value_set_uint64(value, self->cpu_time);
    break;
  case PROP_CPU_USED:
    g_value_set_uint64(value, self->cpu_used);
    break;
  case PROP_TOTAL_READ_BYTE:
    g_value_set_uint64(value, self->total_read_byte);
    break;
  case PROP_TOTAL_WRITE_BYTE:
    g_value_set_uint64(value, self->total_write_byte);
    break;
  case PROP_IO_TIME:
    g_value_set_uint64(value, self->io_time);
    break;
  case PROP_CPU_DELTA:
    g_value_set_uint64(value, self->io_time);
    break;
  case PROP_CPU_USAGE:
    g_value_set_double(value, self->cpu_usage);
    break;
  case PROP_DISK_USAGE:
    g_value_set_double(value, self->disk_usage);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pm_system_status_class_init(PmSystemStatusClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = pm_system_status_set_property;
  object_class->get_property = pm_system_status_get_property;

  properties[PROP_MEM_FREE] = g_param_spec_uint64(
      "mem-free", "Memory Free", "Free system memory", 0, G_MAXUINT64, 0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_MEM_TOTAL] = g_param_spec_uint64(
      "mem-total", "Memory Total", "Total system memory", 0, G_MAXUINT64, 0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_CPU_TIME] = g_param_spec_uint64(
      "cpu-time", "CPU Time", "Total CPU time", 0, G_MAXUINT64, 0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_CPU_USED] = g_param_spec_uint64(
      "cpu-used", "CPU Used", "CPU time in use", 0, G_MAXUINT64, 0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_TOTAL_READ_BYTE] = g_param_spec_uint64(
      "total-read-byte", "Total Read Bytes", "Total bytes read", 0, G_MAXUINT64,
      0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_TOTAL_WRITE_BYTE] = g_param_spec_uint64(
      "total-write-byte", "Total Write Bytes", "Total bytes written", 0,
      G_MAXUINT64, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_IO_TIME] = g_param_spec_uint64(
      "io-time", "I/O Time", "Total I/O time", 0, G_MAXUINT64, 0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_CPU_DELTA] = g_param_spec_uint64(
      "cpu-delta", "CPU Delta", "CPU Time Delta", 0, G_MAXUINT64, 0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_CPU_USAGE] = g_param_spec_double(
      "cpu-usage", "CPU Usage", "CPU Usage Percent", 0.0, G_MAXDOUBLE, 0.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_DISK_USAGE] = g_param_spec_double(
      "disk-usage", "Disk Usage", "Disk Usage Percent", 0.0, G_MAXDOUBLE, 0.0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPERTIES, properties);
}

static void pm_system_status_init(PmSystemStatus *self) {
  self->cpu_time = 0;
  self->io_time = 0;
  self->cpu_used = 0;
  self->mem_free = 0;
  self->mem_total = 0;
  self->total_read_byte = 0;
  self->total_write_byte = 0;
  self->cpu_usage = 0;
  self->disk_usage = 0;
  self->cpu_delta = 0;
}

static gboolean get_cpu_status(PmSystemStatus *system_status, GError **error) {
  g_autofree gchar *contents = NULL;
  gsize length = 0;
  g_auto(GStrv) stat_list;

  if (g_file_get_contents("/proc/stat", &contents, &length, error)) {
    guint64 user, nice, system, idle_time, iowait, irq, softirq, steal;

    stat_list = g_strsplit_set(contents, "\n", -1);

    sscanf(stat_list[0], "cpu  %ld %ld %ld %ld %ld %ld %ld %ld", &user, &nice,
           &system, &idle_time, &iowait, &irq, &softirq, &steal);

    guint64 idle = idle_time + iowait;
    guint64 total =
        user + nice + system + idle_time + iowait + irq + softirq + steal;
    guint64 in_use = total - idle;

    if (system_status->cpu_time != 0) {
      // 2 lazy to pretty up
      system_status->cpu_usage = (((double)in_use) - (system_status->cpu_used)) /
                               (total - system_status->cpu_time) * 100;
    }

    // g_info("%f", system_status->cpu_usage);

    system_status->cpu_delta = total - system_status->cpu_time;
    system_status->cpu_time = total;
    system_status->cpu_used = in_use;
    return TRUE;
  }

  return FALSE;
}

static gboolean get_memory_status(PmSystemStatus *system_status,
                                  GError **error) {
  struct sysinfo info;
  guint64 total, free;

  if (sysinfo(&info) != 0) {
    *error = g_error_new_literal(SYSTEM_STATUS_SYSINFO_ERROR, SystemStatusSysinfoError, "Failed to get sysinfo");
    return FALSE;
  }

  total = info.totalram * info.mem_unit;
  free = info.freeram * info.mem_unit;

  system_status->mem_free = free;
  system_status->mem_total = total;

  return TRUE;
}

static gboolean get_disk_status(PmSystemStatus *system_status, GError **error) {
  g_autofree gchar *contents = NULL;
  gsize length = 0;
  g_auto(GStrv) stat_list;

  guint64 total_read = 0, total_write = 0, total_io = 0;

  if (g_file_get_contents("/proc/stat", &contents, &length, error)) {
    stat_list = g_strsplit_set(contents, "\n", -1);

    for (int i = 0; i < g_strv_length(stat_list); i++) {
      unsigned int major, minor;
      gchar device_name[32];
      guint64 reads_completed, reads_merged, sectors_read, time_reading;
      guint64 writes_completed, writes_merged, sectors_written, time_writing;
      guint64 io_in_progress, io_time, io_time_weighted;

      // Parse fields according to the /proc/diskstats schema
      sscanf(stat_list[i],
             "%u %u %31s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", &major,
             &minor, device_name, &reads_completed, &reads_merged,
             &sectors_read, &time_reading, &writes_completed, &writes_merged,
             &sectors_written, &time_writing, &io_in_progress, &io_time,
             &io_time_weighted);

      total_read += reads_completed;
      total_write += writes_completed;
      total_io += io_time;
    }

    system_status->disk_usage = ((double)total_io - system_status->io_time) /
                                1000 * 100; // interval in ms

    system_status->io_time = total_io;
    system_status->total_read_byte = total_read;
    system_status->total_write_byte = total_write;

    return TRUE;
  }
  return FALSE;
}

PmSystemStatus *pm_system_status_new(void) {
  PmSystemStatus *system_status = NULL;

  system_status = g_object_new(PM_TYPE_SYSTEM_STATUS, NULL);

  return system_status;
}

void pm_system_status_refresh(PmSystemStatus *sys_status) {
  GError *error = NULL;

  if (!get_cpu_status(sys_status, &error)) {
    g_warning("Failed to refresh system status: %s", error->message);

    g_error_free(error);
  }
}

void system_status_get_status(PmSystemStatus *status) {
  GError *error = NULL;

  if (!get_cpu_status(status, &error)) {
    g_warning("Failed to refresh system status: %s", error->message);

    g_error_free(error);
  }

  if (!get_memory_status(status, &error)) {
    g_warning("Failed to refresh system status: %s", error->message);

    g_error_free(error);
  }

  if (!get_disk_status(status, &error)) {
    g_warning("Failed to refresh system status: %s", error->message);

    g_error_free(error);
  }
}

guint64 *system_status_get_cpu_delta(PmSystemStatus *stat) {
  guint64* cpu_delta = g_new0(guint64, 1);
  *cpu_delta = stat->cpu_delta;
  return cpu_delta;
}