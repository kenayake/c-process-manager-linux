#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_PROCESS_DETAILS (pm_process_details_get_type())

G_DECLARE_FINAL_TYPE(PmProcessDetails, pm_process_details, PM, PROCESS_DETAILS, GtkWindow)

PmProcessDetails *process_details_new(pid_t id, const gchararray name, GtkWindow *parent_window);

void process_details_show(PmProcessDetails *self);

G_END_DECLS