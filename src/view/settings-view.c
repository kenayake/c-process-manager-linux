#include "view/settings-view.h"
#include "glib-object.h"
#include "glib.h"
#include "gtk/gtk.h"
#include "gtk/gtkdropdown.h"

struct _PmSettingsView {
  GtkBox parent_instance;
  GtkDropDown *theme_field;
  GtkDropDown *refresh_field;
};

G_DEFINE_FINAL_TYPE(PmSettingsView, pm_settings_view, GTK_TYPE_BOX)

static void on_item_changed(GtkDropDown *self, GParamSpec *pspec, gpointer user_data) {
  GtkSettings *settings;
  GtkStringObject *selected;
  const char *selected_string;
  int comp;

  settings = gtk_settings_get_default();
  selected = gtk_drop_down_get_selected_item(self);
  selected_string = gtk_string_object_get_string(selected);

  comp = g_strcmp0(selected_string, "Light");

  if (comp > 0) {
    // Use system setting
    g_object_set(settings, "gtk-interface-color-scheme", GTK_INTERFACE_COLOR_SCHEME_DEFAULT, NULL);
  } else if (comp < 0) {
    // Dark
    g_object_set(settings, "gtk-interface-color-scheme", GTK_INTERFACE_COLOR_SCHEME_DARK, NULL);
  } else {
    // Light
    g_object_set(settings, "gtk-interface-color-scheme", GTK_INTERFACE_COLOR_SCHEME_LIGHT, NULL);
  }
}

static void pm_settings_view_class_init(PmSettingsViewClass *klass) {
  gtk_widget_class_set_template_from_resource(
      GTK_WIDGET_CLASS(klass),
      "/com/github/kenayake/ProcessManager/view/settings-view.ui");
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), PmSettingsView, theme_field);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), PmSettingsView, refresh_field);
}

static void pm_settings_view_init(PmSettingsView *self) {
  gtk_widget_init_template(GTK_WIDGET(self));
  
  g_signal_connect(self->theme_field, "notify::selected-item", G_CALLBACK(on_item_changed), self);
}

PmSettingsView *pm_settings_view_new(void) {
  return g_object_new(PM_TYPE_SETTINGS_VIEW, NULL);
}