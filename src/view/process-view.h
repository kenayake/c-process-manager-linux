#pragma once

#include "glib-object.h"
#include "gtk/gtk.h"

G_BEGIN_DECLS

#define PM_TYPE_PROCESS_VIEW (pm_process_view_get_type())

G_DECLARE_FINAL_TYPE(PmProcessView, pm_process_view, PM, PROCESS_VIEW, GObject)

PmProcessView *process_view_new(GtkFilter *filter);

GtkWidget *process_view_get_view(PmProcessView *view);

G_END_DECLS