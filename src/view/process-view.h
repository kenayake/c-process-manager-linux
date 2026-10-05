#pragma once

#include "gtk/gtk.h"

GtkWidget *process_view_new(void);

GListModel *process_view_get_model(GtkColumnView *view);