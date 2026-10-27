#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_SEARCH_BAR (pm_search_bar_get_type())

G_DECLARE_FINAL_TYPE(PmSearchBar, pm_search_bar, PM, SEARCH_BAR, GtkBox)

GtkWidget *pm_search_bar_new(void);

GtkFilter *pm_search_bar_get_filter(PmSearchBar *self);

G_END_DECLS
