#include "layout/process-layout.h"
#include "components/icon-button.h"
#include "gtk/gtk.h"
#include "util/alert-dialog.h"
#include "components/run-window.h"
#include "core/process-item.h"
#include "glib.h"
#include "view/process-view.h"
#include <stdio.h>

struct _PmProcessLayout {
  GtkBox parent_instance;

  GtkScrolledWindow *scrolled_window;
  PmIconButton *add_process_button;
  PmIconButton *end_process_button;
  PmProcessView *process_view;
};

G_DEFINE_FINAL_TYPE(PmProcessLayout, pm_process_layout, GTK_TYPE_BOX)

enum {
  PROP_0,
  PROP_ADD_PROCESS_BUTTON,
  PROP_END_PROCESS_BUTTON,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

static void pm_process_layout_get_property(GObject *object, guint property_id,
                                           GValue *value, GParamSpec *pspec) {
  PmProcessLayout *self = PM_PROCESS_LAYOUT(object);

  switch (property_id) {
  case PROP_ADD_PROCESS_BUTTON:
    g_value_set_object(value, self->add_process_button);
    break;
  case PROP_END_PROCESS_BUTTON:
    g_value_set_object(value, self->end_process_button);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void add_process(GtkButton *button, PmProcessLayout *self) {
  GtkRoot *root;
  PmRunWindow *run_window;

  root = gtk_widget_get_root (GTK_WIDGET(self));

  run_window = pm_run_window_new(GTK_WINDOW (root));
  pm_run_window_show(run_window);
}

static void end_process(GtkButton *button, PmProcessLayout *self) {
  GtkTreeListRow *activeTreeRow;
  PmProcessItem *selectedProcess;
  g_autoptr(GtkSingleSelection) selection = NULL;
  g_autoptr(GError) error = NULL;

  g_object_get(self->process_view, "selection", &selection, NULL);
  
  if ((activeTreeRow = gtk_single_selection_get_selected_item(selection)) == NULL) {
    GtkRoot *root = gtk_widget_get_root (GTK_WIDGET(self));
    g_error_new_literal(0, 0, "No process Selected");
    show_alert(GTK_WINDOW (root), "Kill Process Failed", error);
  }

  selectedProcess = gtk_tree_list_row_get_item(activeTreeRow);

  if (!process_item_kill_process(selectedProcess, &error)) {
    GtkRoot *root = gtk_widget_get_root (GTK_WIDGET(self));
    show_alert(GTK_WINDOW (root), "Kill Process Failed", error);
  }
}

static void pm_process_layout_dispose(GObject *object) {
  PmProcessLayout *self = PM_PROCESS_LAYOUT(object);

  if (self->scrolled_window != NULL)
    gtk_scrolled_window_set_child(self->scrolled_window, NULL);

  g_clear_object(&self->process_view);

  G_OBJECT_CLASS(pm_process_layout_parent_class)->dispose(object);
}

static void pm_process_layout_class_init(PmProcessLayoutClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  g_type_ensure(PM_TYPE_ICON_BUTTON);

  object_class->get_property = pm_process_layout_get_property;
  object_class->dispose = pm_process_layout_dispose;

  properties[PROP_ADD_PROCESS_BUTTON] =
      g_param_spec_object("add-process-button", "Add process button",
                          "The button for starting a new process",
                          PM_TYPE_ICON_BUTTON,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  properties[PROP_END_PROCESS_BUTTON] =
      g_param_spec_object("end-process-button", "End process button",
                          "The button for ending a process", PM_TYPE_ICON_BUTTON,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties(object_class, N_PROPERTIES, properties);

  gtk_widget_class_set_template_from_resource(
      widget_class,
      "/com/github/kenayake/ProcessManager/layout/process-layout.ui");
  gtk_widget_class_bind_template_child(widget_class, PmProcessLayout,
                                       scrolled_window);
  gtk_widget_class_bind_template_child(widget_class, PmProcessLayout,
                                       add_process_button);
  gtk_widget_class_bind_template_child(widget_class, PmProcessLayout,
                                       end_process_button);
}

static void pm_process_layout_init(PmProcessLayout *self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  g_signal_connect(self->end_process_button, "clicked", G_CALLBACK(end_process), self);
  g_signal_connect(self->add_process_button, "clicked", G_CALLBACK(add_process), self);
}

PmProcessLayout *pm_process_layout_new(GtkFilter *filter) {
  PmProcessLayout *self;
  GtkWidget *column_view;

  g_return_val_if_fail(GTK_IS_FILTER(filter), NULL);

  self = g_object_new(PM_TYPE_PROCESS_LAYOUT, NULL);
  self->process_view = process_view_new(filter);

  column_view = process_view_get_view(self->process_view);
  gtk_scrolled_window_set_child(self->scrolled_window, column_view);
  g_object_unref(column_view);

  return self;
}