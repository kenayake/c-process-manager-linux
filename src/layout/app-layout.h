#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_APP_LAYOUT (pm_app_layout_get_type())

G_DECLARE_FINAL_TYPE(PmAppLayout, pm_app_layout, PM, APP_LAYOUT, GtkBox)

PmAppLayout *pm_app_layout_new(GtkFilter *filter);

G_END_DECLS