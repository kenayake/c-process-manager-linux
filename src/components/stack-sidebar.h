#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PM_TYPE_STACK_SIDEBAR (pm_stack_sidebar_get_type())

G_DECLARE_FINAL_TYPE(PmStackSidebar, pm_stack_sidebar, PM, STACK_SIDEBAR,
                     GtkBox)

void pm_stack_sidebar_set_stack(PmStackSidebar *self, GtkStack *stack);
void pm_stack_sidebar_set_bottom_pages(PmStackSidebar *self,
                                       const gchar *bottom_pages);

G_END_DECLS
