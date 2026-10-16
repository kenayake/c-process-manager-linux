#include "components/table-cell.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"

typedef struct _PmTableCellPrivate PmTableCellPrivate ;
struct _PmTableCellPrivate{
  gchar *label;
};

enum {
  PROP_0,
  PROP_LABEL,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE(PmTableCell, pm_table_cell, GTK_TYPE_BOX)

static void pm_table_cell_set_property(GObject *object, guint property_id,
                                      const GValue *value,
                                      GParamSpec *pspec) {
  PmTableCell *self = PM_TABLE_CELL(object);
  PmTableCellPrivate *self_private = pm_table_cell_get_instance_private(self);

  switch (property_id) {
  case PROP_LABEL:
    g_free(self_private->label);
    self_private->label = g_value_dup_string(value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pm_table_cell_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {
  PmTableCell *self = PM_TABLE_CELL(object);
  PmTableCellPrivate *self_private = pm_table_cell_get_instance_private(self);

  switch (property_id) {
  case PROP_LABEL:
    g_value_set_string(value, self_private->label);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pm_table_cell_finalize(GObject *object) {
  PmTableCell *self = PM_TABLE_CELL(object);
  PmTableCellPrivate *self_private = pm_table_cell_get_instance_private(self);

  g_free(self_private->label);

  G_OBJECT_CLASS(pm_table_cell_parent_class)->finalize(object);
}

static gchararray label_binding_closure_private(GtkWidget *self, gchararray label) {
  PmTableCellPrivate *self_private = pm_table_cell_get_instance_private(PM_TABLE_CELL(self));

  return g_strdup(self_private->label);
}

static gchararray label_binding_closure(GtkWidget *self, gchararray label) {
  PmTableCellClass *self_klass = PM_TABLE_CELL_GET_CLASS(self);

  if (self_klass->label_binding_closure != NULL) {
    return self_klass->label_binding_closure(self, label);
  }
  return "Parse Error";
}

static void pm_table_cell_class_init(PmTableCellClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  
  // gtk_widget_class_set_css_name (GTK_WIDGET_CLASS(object_class), "table-cell");
  
  object_class->set_property = pm_table_cell_set_property;
  object_class->get_property = pm_table_cell_get_property;
  object_class->finalize = pm_table_cell_finalize;
  PM_TABLE_CELL_CLASS(object_class)->label_binding_closure = label_binding_closure_private;
  
  properties[PROP_LABEL] =
  g_param_spec_string("label", "Label", "The table cell label", NULL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    
  g_object_class_install_properties(object_class, N_PROPERTIES, properties);
    
  gtk_widget_class_set_template_from_resource(
      GTK_WIDGET_CLASS(object_class),
      "/com/github/kenayake/ProcessManager/components/table-cell.ui");
  
  // gtk_widget_class_bind_template_callback(object_class, label_binding_closure_private);
}

static void pm_table_cell_init(PmTableCell *self) {
  PmTableCellPrivate *self_private = pm_table_cell_get_instance_private(self);
  self_private->label = NULL;
  gtk_widget_init_template(GTK_WIDGET(self));
  
  gtk_widget_add_css_class (GTK_WIDGET(self), "table-cell");
}

GtkWidget *pm_table_cell_new(const gchar *label) {
  PmTableCell *self = g_object_new(PM_TYPE_TABLE_CELL, "label", label, NULL);

  return GTK_WIDGET(self);
}