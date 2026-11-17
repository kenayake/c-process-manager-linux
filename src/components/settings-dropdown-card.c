#include "settings-dropdown-card.h"

struct _PmSettingsDropdownCard {
  GtkBox parent_instance;
  gchar *title;
  gchar *subtitle;
  gchar *icon_name;
  GtkWidget *child;
  GtkImage *icon;
  GtkLabel *title_label;
  GtkLabel *subtitle_label;
  GtkBox *content_box;
};

G_DEFINE_FINAL_TYPE(PmSettingsDropdownCard, pm_settings_dropdown_card,
                    GTK_TYPE_BOX)

enum {
  PROP_0,
  PROP_TITLE,
  PROP_SUBTITLE,
  PROP_ICON_NAME,
  PROP_CHILD,
  N_PROPERTIES,
};

static GParamSpec *properties[N_PROPERTIES];

static void pm_settings_dropdown_card_set_property(
    GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
  PmSettingsDropdownCard *self = PM_SETTINGS_DROPDOWN_CARD(object);

  switch (property_id) {
  case PROP_TITLE:
    g_free(self->title);
    self->title = g_value_dup_string(value);
    gtk_label_set_text(self->title_label,
                       self->title != NULL ? self->title : "");
    break;
  case PROP_SUBTITLE:
    g_free(self->subtitle);
    self->subtitle = g_value_dup_string(value);
    gtk_label_set_text(self->subtitle_label,
                       self->subtitle != NULL ? self->subtitle : "");
    break;
  case PROP_ICON_NAME:
    g_free(self->icon_name);
    self->icon_name = g_value_dup_string(value);
    gtk_image_set_from_icon_name(self->icon, self->icon_name);
    break;
  case PROP_CHILD: {
    GtkWidget *child = g_value_get_object(value);

    if (self->child == child)
      break;
    if (self->child != NULL) {
      GtkWidget *old_child = self->child;
      self->child = NULL;
      gtk_box_remove(self->content_box, old_child);
    }
    if (child != NULL) {
      self->child = child;
      gtk_box_append(self->content_box, child);
    }
    break;
  }
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pm_settings_dropdown_card_get_property(
    GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
  PmSettingsDropdownCard *self = PM_SETTINGS_DROPDOWN_CARD(object);

  switch (property_id) {
  case PROP_TITLE:
    g_value_set_string(value, self->title);
    break;
  case PROP_SUBTITLE:
    g_value_set_string(value, self->subtitle);
    break;
  case PROP_ICON_NAME:
    g_value_set_string(value, self->icon_name);
    break;
  case PROP_CHILD:
    g_value_set_object(value, self->child);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pm_settings_dropdown_card_finalize(GObject *object) {
  PmSettingsDropdownCard *self = PM_SETTINGS_DROPDOWN_CARD(object);

  g_free(self->title);
  g_free(self->subtitle);
  g_free(self->icon_name);
  G_OBJECT_CLASS(pm_settings_dropdown_card_parent_class)->finalize(object);
}

static void pm_settings_dropdown_card_class_init(
    PmSettingsDropdownCardClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  object_class->set_property = pm_settings_dropdown_card_set_property;
  object_class->get_property = pm_settings_dropdown_card_get_property;
  object_class->finalize = pm_settings_dropdown_card_finalize;

  properties[PROP_TITLE] =
      g_param_spec_string("title", "Title", "The settings card title", NULL,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                              G_PARAM_STATIC_STRINGS);
  properties[PROP_SUBTITLE] =
      g_param_spec_string("subtitle", "Subtitle",
                          "The settings card subtitle", NULL,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                              G_PARAM_STATIC_STRINGS);
  properties[PROP_ICON_NAME] =
      g_param_spec_string("icon-name", "Icon name",
                          "The settings card icon name", NULL,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                              G_PARAM_STATIC_STRINGS);
  properties[PROP_CHILD] =
      g_param_spec_object("child", "Child", "The dropdown control",
                          GTK_TYPE_WIDGET,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                              G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties(object_class, N_PROPERTIES, properties);

  gtk_widget_class_set_template_from_resource(
      widget_class,
      "/com/github/kenayake/ProcessManager/components/settings-dropdown-card.ui");
  gtk_widget_class_bind_template_child(widget_class, PmSettingsDropdownCard,
                                       icon);
  gtk_widget_class_bind_template_child(widget_class, PmSettingsDropdownCard,
                                       title_label);
  gtk_widget_class_bind_template_child(widget_class, PmSettingsDropdownCard,
                                       subtitle_label);
  gtk_widget_class_bind_template_child(widget_class, PmSettingsDropdownCard,
                                       content_box);
}

static void pm_settings_dropdown_card_init(PmSettingsDropdownCard *self) {
  gtk_widget_init_template(GTK_WIDGET(self));
}
