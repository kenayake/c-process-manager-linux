#include "layout/app-layout.h"
#include "components/stack-sidebar.h"
#include "gtk/gtk.h"
#include "layout/process-layout.h"

struct _PmAppLayout {
  GtkBox parent_instance;

  PmStackSidebar *sidebar;
  GtkStack *stack;
  GtkFilter *filter;
};

G_DEFINE_FINAL_TYPE(PmAppLayout, pm_app_layout, GTK_TYPE_BOX)

static void pm_app_layout_class_init(PmAppLayoutClass *klass) {
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  g_type_ensure(PM_TYPE_STACK_SIDEBAR);
  gtk_widget_class_set_template_from_resource(
      widget_class,
      "/com/github/kenayake/ProcessManager/layout/app-layout.ui");

  gtk_widget_class_bind_template_child(widget_class, PmAppLayout, sidebar);
  gtk_widget_class_bind_template_child(widget_class, PmAppLayout, stack);
}

static void pm_app_layout_init(PmAppLayout *self) {
  gtk_widget_init_template(GTK_WIDGET(self));
  pm_stack_sidebar_set_stack(self->sidebar, self->stack);
}

PmAppLayout *pm_app_layout_new(GtkFilter *filter) {
  PmProcessLayout *layout;
  PmAppLayout *obj =  g_object_new(PM_TYPE_APP_LAYOUT, NULL);
  obj->filter = filter;

  // Processes page
  layout = pm_process_layout_new(filter);
  gtk_stack_add_titled(obj->stack, GTK_WIDGET(layout), "processes", "Processes");
  GtkStackPage *stackpage = gtk_stack_get_page(obj->stack, GTK_WIDGET(layout));
  gtk_stack_page_set_icon_name(stackpage, "view-grid-symbolic");

  // Settings Page (TODO)

  return obj;
}