#include "settings-checklist-card.h"
#include "glib-object.h"
#include "glibconfig.h"
#include "gtk/gtk.h"

struct _PmSettingsChecklistCard {
  GtkBox parent_instance;
  gchar *title;
  gchar *subtitle;
  gchar *icon_name;
  GtkWidget *child;
  GtkImage *icon;
  GtkLabel *title_label;
  GtkLabel *subtitle_label;
  GtkBox *header_box;
  GtkExpander *expander;
  GtkImage *toggle_chevron;
};

G_DEFINE_FINAL_TYPE(PmSettingsChecklistCard, pm_settings_checklist_card,
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

static void pm_settings_checklist_card_set_property(
    GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
  PmSettingsChecklistCard *self = PM_SETTINGS_CHECKLIST_CARD(object);

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
    gtk_expander_set_child(self->expander, child);
    self->child = child;
    break;
  }
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pm_settings_checklist_card_get_property(
    GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
  PmSettingsChecklistCard *self = PM_SETTINGS_CHECKLIST_CARD(object);

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

static void on_toggle(GObject *object, GParamSpec *pspec, gpointer user_data) {
  GtkExpander *expander;
  PmSettingsChecklistCard *self;

  expander = GTK_EXPANDER(object);
  self = PM_SETTINGS_CHECKLIST_CARD(user_data);

  if (gtk_expander_get_expanded(expander)) {
    gtk_image_set_from_icon_name(self->toggle_chevron, "go-up-symbolic");
  } else {
    gtk_image_set_from_icon_name(self->toggle_chevron, "go-down-symbolic");
  }
}

static void pm_settings_checklist_card_finalize(GObject *object) {
  PmSettingsChecklistCard *self = PM_SETTINGS_CHECKLIST_CARD(object);

  g_free(self->title);
  g_free(self->subtitle);
  g_free(self->icon_name);
  G_OBJECT_CLASS(pm_settings_checklist_card_parent_class)->finalize(object);
}

static void pm_settings_checklist_card_class_init(
    PmSettingsChecklistCardClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  object_class->set_property = pm_settings_checklist_card_set_property;
  object_class->get_property = pm_settings_checklist_card_get_property;
  object_class->finalize = pm_settings_checklist_card_finalize;

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
      g_param_spec_object("child", "Child", "The checklist content",
                          GTK_TYPE_WIDGET,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                              G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties(object_class, N_PROPERTIES, properties);

  gtk_widget_class_set_template_from_resource(
      widget_class,
      "/com/github/kenayake/ProcessManager/components/settings-checklist-card.ui");
  gtk_widget_class_bind_template_child(widget_class, PmSettingsChecklistCard,
                                       icon);
  gtk_widget_class_bind_template_child(widget_class, PmSettingsChecklistCard,
                                       title_label);
  gtk_widget_class_bind_template_child(widget_class, PmSettingsChecklistCard,
                                       subtitle_label);
  gtk_widget_class_bind_template_child(widget_class, PmSettingsChecklistCard,
                                       header_box);
  gtk_widget_class_bind_template_child(widget_class, PmSettingsChecklistCard,
                                       expander);
  gtk_widget_class_bind_template_child(widget_class, PmSettingsChecklistCard,
                                       toggle_chevron);
}

static void pm_settings_checklist_card_init(PmSettingsChecklistCard *self) {
  gtk_widget_init_template(GTK_WIDGET(self));
  gtk_expander_set_expanded(self->expander, TRUE);
  g_signal_connect(self->expander, "notify::expanded", G_CALLBACK(on_toggle), self);
}
