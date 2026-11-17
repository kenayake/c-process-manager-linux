#include "layout/app-layout.h"
#include "components/stack-sidebar.h"
#include "glib-object.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include "layout/process-layout.h"
#include "view/settings-view.h"

struct _PmAppLayout {
  GtkBox parent_instance;

  PmStackSidebar *sidebar;
  GtkStack *stack;
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

static void create_dummy_page(GtkStack *stack, const gchar *name, const gchar *title, const gchar *icon_name) {
  GtkWidget *label;
  GtkStackPage *stackpage;

  label = gtk_label_new("Dummy Page");

  gtk_stack_add_titled(stack, label, name, title);
  stackpage = gtk_stack_get_page(stack, label);
  gtk_stack_page_set_icon_name(stackpage, icon_name);
}

PmAppLayout *pm_app_layout_new(GtkFilter *filter) {
  PmProcessLayout *layout;
  PmSettingsView *settings;
  GtkStackPage *stackpage;

  PmAppLayout *obj =  g_object_new(PM_TYPE_APP_LAYOUT, NULL);

  // Processes page
  layout = pm_process_layout_new(filter);
  gtk_stack_add_titled(obj->stack, GTK_WIDGET(layout), "processes", "Processes");
  stackpage = gtk_stack_get_page(obj->stack, GTK_WIDGET(layout));
  gtk_stack_page_set_icon_name(stackpage, "taskmgr-processes-symbolic");

  // Settings Page
  settings = pm_settings_view_new();
  gtk_stack_add_titled(obj->stack, GTK_WIDGET(settings), "settings", "Settings");
  stackpage = gtk_stack_get_page(obj->stack, GTK_WIDGET(settings));
  gtk_stack_page_set_icon_name(stackpage, "taskmgr-settings-symbolic");

  // Dummy pages
  create_dummy_page(obj->stack, "performance", "Performance", "taskmgr-performance-symbolic");
  create_dummy_page(obj->stack, "history", "App History", "taskmgr-app-history-symbolic");
  create_dummy_page(obj->stack, "startup", "Startup Apps", "taskmgr-startup-apps-symbolic");
  create_dummy_page(obj->stack, "users", "Users", "taskmgr-users-symbolic");
  create_dummy_page(obj->stack, "details", "Details", "taskmgr-details-symbolic");
  create_dummy_page(obj->stack, "services", "Services", "taskmgr-services-symbolic");

  return obj;
}