#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_PROCESS_LAYOUT (pm_process_layout_get_type())

G_DECLARE_FINAL_TYPE(PmProcessLayout, pm_process_layout, PM, PROCESS_LAYOUT,
                     GtkBox)

PmProcessLayout *pm_process_layout_new(GtkFilter *filter);

G_END_DECLS