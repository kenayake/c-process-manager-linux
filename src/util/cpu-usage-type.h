#pragma once

#include "glib-object.h"

typedef struct {
    gint64 stime;
    gint64 utime;
} CpuUsage;

#define TYPE_CPU_USAGE (cpu_usage_get_type ())

GType
cpu_usage_get_type (void) G_GNUC_CONST;