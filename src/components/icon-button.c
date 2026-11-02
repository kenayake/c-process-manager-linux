#include <gtk/gtk.h>
#include "components/icon-button.h"

struct _PmIconButton {
  GtkButton parent_instance;

  GtkImage *icon;
  GtkLabel *text;
  gchar *icon_name;
  gchar *text_value;
  gboolean show_text;
};

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_TEXT,
  PROP_SHOW_TEXT,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_FINAL_TYPE(PmIconButton, pm_icon_button, GTK_TYPE_BUTTON)

static void pm_icon_button_update_text_visibility(PmIconButton *self) {
  if (self->text != NULL)
    gtk_widget_set_visible(GTK_WIDGET(self->text), self->show_text);
}

static void pm_icon_button_set_property(GObject *object, guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {
  PmIconButton *self = PM_ICON_BUTTON(object);

  switch (property_id) {
  case PROP_ICON_NAME:
    g_free(self->icon_name);
    self->icon_name = g_value_dup_string(value);
    if (self->icon != NULL)
      gtk_image_set_from_icon_name(self->icon, self->icon_name);
    break;
  case PROP_TEXT:
    g_free(self->text_value);
    self->text_value = g_value_dup_string(value);
    if (self->text != NULL)
      gtk_label_set_text(self->text, self->text_value);
    gtk_accessible_update_property(GTK_ACCESSIBLE(self),
                                   GTK_ACCESSIBLE_PROPERTY_LABEL,
                                   self->text_value, -1);
    break;
  case PROP_SHOW_TEXT:
    self->show_text = g_value_get_boolean(value);
    pm_icon_button_update_text_visibility(self);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pm_icon_button_get_property(GObject *object, guint property_id,
                                        GValue *value, GParamSpec *pspec) {
  PmIconButton *self = PM_ICON_BUTTON(object);

  switch (property_id) {
  case PROP_ICON_NAME:
    g_value_set_string(value, self->icon_name);
    break;
  case PROP_TEXT:
    g_value_set_string(value, self->text_value);
    break;
  case PROP_SHOW_TEXT:
    g_value_set_boolean(value, self->show_text);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pm_icon_button_finalize(GObject *object) {
  PmIconButton *self = PM_ICON_BUTTON(object);

  g_free(self->icon_name);
  g_free(self->text_value);

  G_OBJECT_CLASS(pm_icon_button_parent_class)->finalize(object);
}

static void pm_icon_button_class_init(PmIconButtonClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  object_class->set_property = pm_icon_button_set_property;
  object_class->get_property = pm_icon_button_get_property;
  object_class->finalize = pm_icon_button_finalize;

  properties[PROP_ICON_NAME] =
      g_param_spec_string("icon-name", "Icon name", "The button icon name",
                          NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_TEXT] =
      g_param_spec_string("text", "Text", "The button text", NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_SHOW_TEXT] =
      g_param_spec_boolean("show-text", "Show text",
                           "Whether the button text is visible", TRUE,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties(object_class, N_PROPERTIES, properties);

  gtk_widget_class_set_template_from_resource(
      widget_class,
      "/com/github/kenayake/ProcessManager/components/icon-button.ui");
  gtk_widget_class_bind_template_child(widget_class, PmIconButton, icon);
  gtk_widget_class_bind_template_child(widget_class, PmIconButton, text);
}

static void pm_icon_button_init(PmIconButton *self) {
  self->show_text = TRUE;
  gtk_widget_init_template(GTK_WIDGET(self));
  pm_icon_button_update_text_visibility(self);
}

PmIconButton *pm_icon_button_new(const gchar *icon_name, const gchar *text) {
  return g_object_new(PM_TYPE_ICON_BUTTON,
                      "icon-name", icon_name,
                      "text", text,
                      NULL);
}

void pm_icon_button_set_show_text(PmIconButton *self, gboolean show_text) {
  g_return_if_fail(PM_IS_ICON_BUTTON(self));

  g_object_set(self, "show-text", show_text, NULL);
}
