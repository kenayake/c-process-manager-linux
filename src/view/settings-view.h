#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_SETTINGS_VIEW (pm_settings_view_get_type())

G_DECLARE_FINAL_TYPE(PmSettingsView, pm_settings_view, PM, SETTINGS_VIEW,
                     GtkBox)

PmSettingsView *pm_settings_view_new(void);

G_END_DECLS