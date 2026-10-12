#pragma once

#include "glib.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_SYSTEM_STATUS (pm_system_status_get_type())

G_DECLARE_FINAL_TYPE(PmSystemStatus, pm_system_status, PM, SYSTEM_STATUS, GObject)

PmSystemStatus *pm_system_status_new(void);

void pm_system_status_refresh(PmSystemStatus *sys_status);

void system_status_get_status(PmSystemStatus *status);

guint64 *system_status_get_cpu_delta(PmSystemStatus *stat);

G_END_DECLS