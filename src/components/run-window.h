#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_RUN_WINDOW (pm_run_window_get_type())

G_DECLARE_FINAL_TYPE(PmRunWindow, pm_run_window, PM, RUN_WINDOW, GtkWindow)

PmRunWindow *pm_run_window_new(GtkWindow *parent_window);

void pm_run_window_show(PmRunWindow *self);

G_END_DECLS
