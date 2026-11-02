#include "run-window.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "glibconfig.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include "util/alert-dialog.h"

struct _PmRunWindow {
  GtkWindow parent_instance;

  GtkEntry *entry;
};

enum {
  PROP_0,
  PROP_ENTRY,
  N_PROPERTIES,
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_FINAL_TYPE(PmRunWindow, pm_run_window, GTK_TYPE_WINDOW)

static void pm_run_window_get_property(GObject *object, guint prop_id,
                                       GValue *value, GParamSpec *pspec) {
  PmRunWindow *self = PM_RUN_WINDOW(object);

  switch (prop_id) {
    case PROP_ENTRY:
      g_value_set_object(value, self->entry);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void cancel_button_clicked(GtkButton *button, PmRunWindow *self) {
  gtk_window_destroy(GTK_WINDOW(self));
}

static void ok_button_clicked(GtkButton *button, PmRunWindow *self) {
  g_autoptr(GError) error = NULL;
  g_autoptr(GSubprocess) process;
  g_autoptr(GAppInfo) appinfo;
  const gchar *string;

  string = gtk_editable_get_text(GTK_EDITABLE(self->entry));

  appinfo = g_app_info_create_from_commandline(
    string, 
    NULL, 
    G_APP_INFO_CREATE_NONE, 
    &error);
  g_app_info_launch (appinfo, NULL, NULL, &error);
  
  if (error != NULL) {
    GtkWindow *window = gtk_window_get_transient_for (GTK_WINDOW(self));
    show_alert(GTK_WINDOW(window), "Failed to launch process", error);
  }

  gtk_window_destroy(GTK_WINDOW(self));
}

static void pm_run_window_class_init(PmRunWindowClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  object_class->get_property = pm_run_window_get_property;

  gtk_widget_class_bind_template_child(widget_class, PmRunWindow, entry);

  properties[PROP_ENTRY] = g_param_spec_object(
      "entry", "Entry", "The run window entry widget", GTK_TYPE_ENTRY,
      G_PARAM_READABLE);

  g_object_class_install_properties(object_class, N_PROPERTIES, properties);

  gtk_widget_class_set_template_from_resource(
      widget_class,
      "/com/github/kenayake/ProcessManager/components/run-window.ui");

  gtk_widget_class_bind_template_child(widget_class, PmRunWindow, entry);
  gtk_widget_class_bind_template_callback(widget_class, ok_button_clicked);
  gtk_widget_class_bind_template_callback(widget_class, cancel_button_clicked);
}

static void pm_run_window_init(PmRunWindow *self) {
  gtk_widget_init_template(GTK_WIDGET(self));
}

PmRunWindow *pm_run_window_new(GtkWindow *parent_window) {
  PmRunWindow *run = g_object_new(PM_TYPE_RUN_WINDOW, NULL);

  gtk_window_set_transient_for(GTK_WINDOW(run), parent_window);

  return run;
}

void pm_run_window_show(PmRunWindow *self) {
  gtk_window_present(GTK_WINDOW(self));
}