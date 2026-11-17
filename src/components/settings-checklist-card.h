#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_SETTINGS_CHECKLIST_CARD \
  (pm_settings_checklist_card_get_type())

G_DECLARE_FINAL_TYPE(PmSettingsChecklistCard, pm_settings_checklist_card, PM,
                     SETTINGS_CHECKLIST_CARD, GtkBox)

G_END_DECLS
