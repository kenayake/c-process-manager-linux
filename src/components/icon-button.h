#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_ICON_BUTTON (pm_icon_button_get_type())

G_DECLARE_FINAL_TYPE(PmIconButton, pm_icon_button, PM, ICON_BUTTON, GtkButton)

PmIconButton *pm_icon_button_new(const gchar *icon_name, const gchar *text);

void pm_icon_button_set_show_text(PmIconButton *self, gboolean show_text);

G_END_DECLS
