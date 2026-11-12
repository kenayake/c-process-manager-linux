#include "view/settings-view.h"
#include "gio/gio.h"
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

static void on_theme_changed(GtkDropDown *self, GParamSpec *pspec, gpointer user_data) {
  GtkSettings *gtk_settings;
  g_autoptr(GSettings) app_settings;
  GtkStringObject *selected;
  const char *selected_string;
  int comp;

  gtk_settings = gtk_settings_get_default();
  app_settings = g_settings_new("com.github.kenayake.ProcessManager");
  selected = gtk_drop_down_get_selected_item(self);
  selected_string = gtk_string_object_get_string(selected);

  comp = g_strcmp0(selected_string, "Light");

  if (comp > 0) {
    // Use system setting
    gtk_settings_reset_property(gtk_settings, "gtk-interface-color-scheme");
    g_settings_set_int(app_settings, "theme", 1);
  } else if (comp == 0) {
    // Light
    g_object_set(gtk_settings, "gtk-interface-color-scheme", GTK_INTERFACE_COLOR_SCHEME_LIGHT, NULL);
    g_settings_set_int(app_settings, "theme", 2);
  } else {
    // Dark
    g_object_set(gtk_settings, "gtk-interface-color-scheme", GTK_INTERFACE_COLOR_SCHEME_DARK, NULL);
    g_settings_set_int(app_settings, "theme", 3);
  }
}

static void on_refresh_changed(GtkDropDown *self, GParamSpec *pspec, gpointer user_data) {
  g_autoptr(GSettings) app_settings;
  GtkStringObject *selected;
  const char *selected_string;

  app_settings = g_settings_new("com.github.kenayake.ProcessManager");
  selected = gtk_drop_down_get_selected_item(self);
  selected_string = gtk_string_object_get_string(selected);

  if (g_strcmp0(selected_string, "1 second") == 0) {
    g_settings_set_int(app_settings, "refresh-secs", 1);
  } else if (g_strcmp0(selected_string, "2 seconds") == 0) {
    g_settings_set_int(app_settings, "refresh-secs", 2);
  } else if (g_strcmp0(selected_string, "3 seconds") == 0) {
    g_settings_set_int(app_settings, "refresh-secs", 3);
  } else {
    g_settings_set_int(app_settings, "refresh-secs", 5);
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
  g_autoptr(GSettings) app_settings;
  gint theme_pos, refresh_pos;

  gtk_widget_init_template(GTK_WIDGET(self));

  app_settings = g_settings_new("com.github.kenayake.ProcessManager");

  // default pos for app theme drop down
  theme_pos = g_settings_get_int(app_settings, "theme");
  gtk_drop_down_set_selected(self->theme_field, theme_pos - 1);

  // pos for refresh secs, more involved but oh well
  refresh_pos = g_settings_get_int(app_settings, "refresh-secs");
  if (refresh_pos == 5) {
    gtk_drop_down_set_selected(self->refresh_field, refresh_pos - 2);
  } else {
    gtk_drop_down_set_selected(self->refresh_field, refresh_pos - 1);
  }
  
  g_signal_connect(self->theme_field, "notify::selected-item", G_CALLBACK(on_theme_changed), NULL);
  g_signal_connect(self->refresh_field, "notify::selected-item", G_CALLBACK(on_refresh_changed), NULL);
}

PmSettingsView *pm_settings_view_new(void) {
  return g_object_new(PM_TYPE_SETTINGS_VIEW, NULL);
}