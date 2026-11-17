#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_SETTINGS_DROPDOWN_CARD \
  (pm_settings_dropdown_card_get_type())

G_DECLARE_FINAL_TYPE(PmSettingsDropdownCard, pm_settings_dropdown_card, PM,
                     SETTINGS_DROPDOWN_CARD, GtkBox)

G_END_DECLS
