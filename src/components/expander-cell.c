#include "components/expander-cell.h"
#include "glib-object.h"
#include "glib.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"

struct _PmExpanderCell {
  GtkBox parent_instance;
  gchar *label;
  GtkTreeExpander *expander;
  // GtkWidget *label_widget;
  // GBinding *label_binding;
};

enum {
  PROP_0,
  PROP_LABEL,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_FINAL_TYPE(PmExpanderCell, pm_expander_cell, GTK_TYPE_BOX)

static void pm_expander_cell_set_property(GObject *object, guint property_id,
                                      const GValue *value,
                                      GParamSpec *pspec) {
  PmExpanderCell *self = PM_EXPANDER_CELL(object);

  switch (property_id) {
  case PROP_LABEL:
    g_free(self->label);
    self->label = g_value_dup_string(value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pm_expander_cell_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {
  PmExpanderCell *self = PM_EXPANDER_CELL(object);

  switch (property_id) {
  case PROP_LABEL:
    g_value_set_string(value, self->label);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pm_expander_cell_finalize(GObject *object) {
  PmExpanderCell *self = PM_EXPANDER_CELL(object);

  // if (self->label_binding) {
  //   g_binding_unbind(self->label_binding);
  // }

  // g_object_unref(self->label_widget);
  g_free(self->label);

  G_OBJECT_CLASS(pm_expander_cell_parent_class)->finalize(object);
}

static void pm_expander_cell_class_init(PmExpanderCellClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = pm_expander_cell_set_property;
  object_class->get_property = pm_expander_cell_get_property;
  object_class->finalize = pm_expander_cell_finalize;

  properties[PROP_LABEL] =
      g_param_spec_string("label", "Label", "The expander cell label", NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  
  g_object_class_install_properties(object_class, N_PROPERTIES, properties);

  gtk_widget_class_set_template_from_resource(
      GTK_WIDGET_CLASS(object_class),
      "/com/github/kenayake/ProcessManager/components/expander-cell.ui");

  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(object_class), PmExpanderCell, expander);
}

static void pm_expander_cell_init(PmExpanderCell *self) {
  self->label = NULL;
  // self->label_widget = gtk_label_new(NULL);

  // self->label_binding = g_object_bind_property(self, "label", self->label_widget, "label", G_BINDING_DEFAULT);

  gtk_widget_init_template(GTK_WIDGET(self));
  gtk_widget_add_css_class (GTK_WIDGET(self), "expander-cell");
}

GtkWidget *pm_expander_cell_new(const gchar *label) {
  PmExpanderCell *self = g_object_new(PM_TYPE_EXPANDER_CELL, "label", label, NULL);
  // gtk_tree_expander_set_child(self->expander, self->label_widget);

  return GTK_WIDGET(self);
}

void pm_expander_cell_set_list_row(PmExpanderCell *self, GtkTreeListRow *list_row) {
  gtk_tree_expander_set_list_row(self->expander, list_row);
}
