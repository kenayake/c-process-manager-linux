#include "components/numeric-cell.h"
#include "components/table-cell.h"
#include "glib-object.h"
#include "glib.h"
#include "gtk/gtk.h"

struct _PmNumericCell {
  PmTableCell parent_object;

  gdouble raw_value;
  gdouble max;
  gchar *suffix;
  GBinding *label_binding;
};

G_DEFINE_FINAL_TYPE(PmNumericCell, pm_numeric_cell, PM_TYPE_TABLE_CELL);

enum {
  PROP_0,
  PROP_RAW_VALUE,
  PROP_MAX,
  PROP_SUFFIX,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

static void pm_numeric_cell_set_property(GObject *object, guint property_id,
                                         const GValue *value,
                                         GParamSpec *pspec) {
  PmNumericCell *self = PM_NUMERIC_CELL(object);

  switch (property_id) {
  case PROP_RAW_VALUE:
    self->raw_value = g_value_get_double(value);
    break;
  case PROP_MAX:
    self->max = g_value_get_double(value);
    break;
  case PROP_SUFFIX:
    g_free(self->suffix);
    self->suffix = g_value_dup_string(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void pm_numeric_cell_get_property(GObject *object, guint property_id,
                                         GValue *value, GParamSpec *pspec) {
  PmNumericCell *self = PM_NUMERIC_CELL(object);

  switch (property_id) {
  case PROP_RAW_VALUE:
    g_value_set_double(value, self->raw_value);
    break;
  case PROP_MAX:
    g_value_set_double(value, self->max);
    break;
  case PROP_SUFFIX:
    g_value_set_string(value, self->suffix);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void pm_numeric_cell_finalize(GObject *object) {
  PmNumericCell *self = PM_NUMERIC_CELL(object);

  g_free(self->suffix);
  G_OBJECT_CLASS(pm_numeric_cell_parent_class)->finalize(object);
}

static gchararray numeric_label_binding_closure(GtkWidget *self) {
  PmNumericCell *obj = PM_NUMERIC_CELL(self);

  for (int i = 1; i < 5; i++) {
    g_autofree gchararray class = g_strdup_printf("accent%d", i);
    gtk_widget_remove_css_class(self, class);
  }

  if (obj->raw_value / obj->max < 0.01) {
    gtk_widget_add_css_class(self, "accent1");
  } else if (obj->raw_value / obj->max < 0.1) {
    gtk_widget_add_css_class(self, "accent2");
  } else if (obj->raw_value / obj->max < 0.3) {
    gtk_widget_add_css_class(self, "accent3");
  } else {
    gtk_widget_add_css_class(self, "accent4");
  }
  
  return g_strdup_printf(
      obj->suffix != NULL ? "%.1f%s" : "%.0f", obj->raw_value, obj->suffix);
}

static void pm_numeric_cell_class_init(PmNumericCellClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  PmTableCellClass *parent_class = PM_TABLE_CELL_CLASS(klass);

  object_class->set_property = pm_numeric_cell_set_property;
  object_class->get_property = pm_numeric_cell_get_property;
  object_class->finalize = pm_numeric_cell_finalize;

  properties[PROP_RAW_VALUE] =
      g_param_spec_double("raw-value", "Raw value", "The numeric value",
                          -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_MAX] =
      g_param_spec_double("max", "Maximum", "The maximum numeric value",
                          -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_SUFFIX] =
      g_param_spec_string("suffix", "Suffix", "The value suffix", NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties(object_class, N_PROPERTIES, properties);

  parent_class->label_binding_closure = numeric_label_binding_closure;
}

static void pm_numeric_cell_init(PmNumericCell *self) {
  self->max = 0.0;
  self->raw_value = 0.0;
  self->suffix = NULL;
  g_object_bind_property(self, "raw_value", self, "label", G_BINDING_SYNC_CREATE);
  gtk_widget_init_template(GTK_WIDGET(self));
}

GtkWidget *pm_numeric_cell_new(gdouble raw_value, gdouble max, const gchar *suffix) {
  PmNumericCell *obj = g_object_new(PM_TYPE_NUMERIC_CELL, "raw_value", raw_value, "max", max, "suffix", suffix, NULL);

  return GTK_WIDGET(obj);
}