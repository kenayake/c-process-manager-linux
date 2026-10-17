#pragma once

#include "components/table-cell.h"
#include "glib-object.h"
#include "glibconfig.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_NUMERIC_CELL (pm_numeric_cell_get_type())

G_DECLARE_FINAL_TYPE(PmNumericCell, pm_numeric_cell, PM, NUMERIC_CELL, PmTableCell)

GtkWidget *pm_numeric_cell_new(gdouble raw_value, gdouble max, const gchar *suffix);

G_END_DECLS