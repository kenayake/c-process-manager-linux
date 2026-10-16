#pragma once

#include "glib-object.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_TABLE_CELL (pm_table_cell_get_type())

G_DECLARE_DERIVABLE_TYPE(PmTableCell, pm_table_cell, PM, TABLE_CELL, GtkBox)

struct _PmTableCellClass {
  GtkBoxClass parent_class;

  // virtual functions
  gchararray (*label_binding_closure) (GtkWidget *this, gchararray label);

  gpointer padding[12];
};

GtkWidget *pm_table_cell_new(const gchar *label);

G_END_DECLS