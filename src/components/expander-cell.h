#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_EXPANDER_CELL (pm_expander_cell_get_type())

G_DECLARE_FINAL_TYPE(PmExpanderCell, pm_expander_cell, PM, EXPANDER_CELL, GtkBox)

GtkWidget *pm_expander_cell_new(const gchar *label);

void pm_expander_cell_set_list_row(PmExpanderCell *self, GtkTreeListRow *list_row);

G_END_DECLS