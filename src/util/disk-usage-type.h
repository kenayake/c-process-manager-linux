#pragma once

#include "glib-object.h"
#include "util/timespec-type.h"

typedef struct {
    guint64 read_bytes;
    guint64 write_bytes;
} DiskUsage;

#define TYPE_DISK_USAGE (disk_usage_get_type ())

GType
disk_usage_get_type (void) G_GNUC_CONST;