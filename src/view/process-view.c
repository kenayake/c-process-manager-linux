#include "process-view.h"
#include "config.h"
#include "core/process-item.h"
#include "core/process-model.h"
#include "core/system-status.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include "gtk/gtkshortcut.h"
#include "gtk/gtksingleselection.h"
#include <gtk/gtk.h>
#include <stdio.h>

typedef struct {
  const gchar *name;
  const gchar *suffix;
} UserData;

struct _PmProcessView {
  GObject parent_instance;

  GListModel *model;
  PmSystemStatus *sys_stat;
  GtkWidget *column_view;
  guint timeout_id;
};

G_DEFINE_FINAL_TYPE(PmProcessView, pm_process_view, G_TYPE_OBJECT)

gboolean bind_numeric_to_label(GBinding *binding, const GValue *from_value,
                               GValue *to_value, gpointer user_data) {
  UserData *data = user_data;

  gdouble numeric_value = g_value_get_double(from_value);

  const g_autofree gchar *label_text = g_strdup_printf(
      data->suffix != NULL ? "%.1f %s" : "%.1f", numeric_value, data->suffix);

  g_value_set_string(to_value, label_text);

  return TRUE;
}

static void setup_expander_column(GtkSignalListItemFactory *factory,
                                  GtkListItem *list_item, gpointer user_data) {
  GtkWidget *expander;
  GtkWidget *label;

  expander = gtk_tree_expander_new();
  label = gtk_label_new(NULL);

  gtk_tree_expander_set_child(GTK_TREE_EXPANDER(expander), label);
  gtk_list_item_set_child(list_item, expander);
}

static void bind_expander_column(GtkSignalListItemFactory *factory,
                                 GtkListItem *list_item, gpointer user_data) {
  GtkTreeListRow *row;
  PmProcessItem *process;
  GtkWidget *expander;
  GtkWidget *label;
  g_autofree gchar *key;
  GBinding *binding;

  UserData *data = user_data;

  row = GTK_TREE_LIST_ROW(gtk_list_item_get_item(list_item));
  process = PM_PROCESS_ITEM(gtk_tree_list_row_get_item(row));
  expander = gtk_list_item_get_child(list_item);
  label = gtk_tree_expander_get_child(GTK_TREE_EXPANDER(expander));

  gtk_tree_expander_set_list_row(GTK_TREE_EXPANDER(expander), row);
  // g_object_get(process, data->name, &text, NULL);
  // gtk_label_set_text(GTK_LABEL(label), text);

  binding = g_object_bind_property(process, data->name, label, "label",
                                   G_BINDING_SYNC_CREATE);

  key = g_strdup_printf("%s-key", data->name);

  g_object_set_data(G_OBJECT(list_item), key, binding);

  g_object_unref(process);
}

static void setup_generic_column(GtkSignalListItemFactory *factory,
                                 GtkListItem *list_item, gpointer user_data) {
  GtkWidget *label;

  label = gtk_label_new(NULL);

  gtk_list_item_set_child(list_item, label);
}

static void bind_string_column(GtkSignalListItemFactory *factory,
                               GtkListItem *list_item, gpointer user_data) {
  GtkTreeListRow *row;
  PmProcessItem *process;
  GtkWidget *label;
  g_autofree gchar *key;
  GBinding *binding;

  UserData *data = user_data;

  row = GTK_TREE_LIST_ROW(gtk_list_item_get_item(list_item));
  process = gtk_tree_list_row_get_item(row);
  label = gtk_list_item_get_child(list_item);

  if (process == NULL) {
    gtk_label_set_text(GTK_LABEL(label), "");
    return;
  }

  binding = g_object_bind_property(process, data->name, label, "label",
                                   G_BINDING_SYNC_CREATE);

  // g_object_get(process, data->name, &text, NULL);
  key = g_strdup_printf("%s-key", data->name);

  g_object_set_data(G_OBJECT(list_item), key, binding);

  // gtk_label_set_text(GTK_LABEL(label), text);

  g_object_unref(process);
}

static void bind_numeric_column(GtkSignalListItemFactory *factory,
                                GtkListItem *list_item, gpointer user_data) {
  GtkTreeListRow *row;
  PmProcessItem *process;
  GtkWidget *label;
  // gdouble value;
  g_autofree gchar *key;
  GBinding *binding;

  UserData *data = user_data;

  row = GTK_TREE_LIST_ROW(gtk_list_item_get_item(list_item));
  process = gtk_tree_list_row_get_item(row);
  label = gtk_list_item_get_child(list_item);

  if (process == NULL) {
    gtk_label_set_text(GTK_LABEL(label), "");
    return;
  }

  binding = g_object_bind_property_full(
      process, data->name, label, "label", G_BINDING_SYNC_CREATE,
      bind_numeric_to_label, NULL, data, NULL);

  key = g_strdup_printf("%s-key", data->name);

  g_object_set_data(G_OBJECT(list_item), key, binding);

  // g_object_get(process, data->name, &value, NULL);

  // g_snprintf(text, sizeof(text), data->suffix != NULL ? "%.1f %s" : "%.1f",
  //            value, data->suffix);

  // gtk_label_set_text(GTK_LABEL(label), text);

  g_object_unref(process);
}

static void unbind_generic_column(GtkSignalListItemFactory *factory,
                                  GtkListItem *list_item, gpointer user_data) {
  UserData *data = user_data;
  g_autofree gchar *key = g_strdup_printf("%s-key", data->name);

  GBinding *binding = g_object_get_data(G_OBJECT(list_item), key);
  if (binding) {
    g_binding_unbind(binding);
    g_object_set_data(G_OBJECT(list_item), key, NULL);
  }
}

static GtkColumnViewColumn *create_first_column(const gchar *prop_name,
                                                const gchar *title) {
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *expr;
  GtkSorter *string_sorter;
  UserData *user_data = g_new0(UserData, 1);
  user_data->name = prop_name;
  user_data->suffix = NULL;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(setup_expander_column), NULL);

  g_signal_connect(factory, "bind", G_CALLBACK(bind_expander_column),
                   user_data);

  g_signal_connect(factory, "unbind", G_CALLBACK(unbind_generic_column),
                   user_data);

  column = gtk_column_view_column_new(title, GTK_LIST_ITEM_FACTORY(factory));

  // set default column properties
  gtk_column_view_column_set_resizable(column, true);
  gtk_column_view_column_set_expand(column, TRUE);

  // Setup expression-based sorter
  expr = gtk_property_expression_new(PM_TYPE_PROCESS_ITEM, NULL, prop_name);
  string_sorter = GTK_SORTER(gtk_string_sorter_new(expr));

  // Bind the sorter to the column
  gtk_column_view_column_set_sorter(column, string_sorter);
  g_object_unref(string_sorter);

  return column;
}

static GtkColumnViewColumn *create_numeric_column(const gchar *prop_name,
                                                  const gchar *title,
                                                  const gchar *suffix) {
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *expr;
  GtkSorter *numeric_sorter;
  UserData *user_data = g_new0(UserData, 1);
  user_data->name = prop_name;
  user_data->suffix = suffix;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(setup_generic_column), NULL);

  g_signal_connect(factory, "bind", G_CALLBACK(bind_numeric_column), user_data);

  g_signal_connect(factory, "unbind", G_CALLBACK(unbind_generic_column),
                   user_data);

  column = gtk_column_view_column_new(title, GTK_LIST_ITEM_FACTORY(factory));

  // set default column properties
  gtk_column_view_column_set_resizable(column, true);
  gtk_column_view_column_set_expand(column, TRUE);

  // Setup expression-based sorter
  expr = gtk_property_expression_new(PM_TYPE_PROCESS_ITEM, NULL, prop_name);
  numeric_sorter = GTK_SORTER(gtk_numeric_sorter_new(expr));

  // Bind the sorter to the column
  gtk_column_view_column_set_sorter(column, numeric_sorter);
  g_object_unref(numeric_sorter);

  return column;
}

static GtkColumnViewColumn *create_string_column(const gchar *prop_name,
                                                 const gchar *title) {
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *expr;
  GtkSorter *string_sorter;
  UserData *user_data = g_new0(UserData, 1);
  user_data->name = prop_name;
  user_data->suffix = NULL;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(setup_generic_column), NULL);

  g_signal_connect(factory, "bind", G_CALLBACK(bind_string_column), user_data);

  g_signal_connect(factory, "unbind", G_CALLBACK(unbind_generic_column),
                   user_data);

  column = gtk_column_view_column_new(title, GTK_LIST_ITEM_FACTORY(factory));

  // set default column properties
  gtk_column_view_column_set_resizable(column, true);
  gtk_column_view_column_set_expand(column, TRUE);

  // Setup expression-based sorter
  expr = gtk_property_expression_new(PM_TYPE_PROCESS_ITEM, NULL, prop_name);
  string_sorter = GTK_SORTER(gtk_string_sorter_new(expr));

  // Bind the sorter to the column
  gtk_column_view_column_set_sorter(column, string_sorter);
  g_object_unref(string_sorter);

  return column;
}

static GtkSorter *process_view_get_sorter(GtkColumnView *view) {
  GListModel *sort_model;
  GtkSelectionModel *selection_model;
  GtkSorter *tree_sorter;

  selection_model = gtk_column_view_get_model(view);
  sort_model =
      gtk_single_selection_get_model(GTK_SINGLE_SELECTION(selection_model));
  tree_sorter = gtk_sort_list_model_get_sorter(GTK_SORT_LIST_MODEL(sort_model));

  return tree_sorter;
}

static gboolean on_timeout_tick(gpointer user_data) {
  PmProcessView *view = user_data;
  GtkSorter *sort_model = process_view_get_sorter(GTK_COLUMN_VIEW(view->column_view));
  GListModel *model = view->model;
  guint64 *cpu_delta;
  // g_info("timeout started");

  if (model != NULL) {

    system_status_get_status(view->sys_stat);
    cpu_delta = system_status_get_cpu_delta(view->sys_stat);
    g_object_set_data_full(G_OBJECT(model), "cpu-delta", cpu_delta, (GDestroyNotify)g_free);

    process_model_refresh(G_LIST_STORE(model));
    
    // Update sort
    gtk_sorter_changed(sort_model, GTK_SORTER_CHANGE_DIFFERENT);
  }

  return G_SOURCE_CONTINUE;
}

void on_view_map(GtkWidget *self, gpointer user_data) {
  PmProcessView *process_view = PM_PROCESS_VIEW(user_data);

  process_view->timeout_id = g_timeout_add(1000, on_timeout_tick, process_view); // replace with interval setting
}

void on_view_unmap(GtkWidget *self, gpointer user_data) {
  PmProcessView *process_view = PM_PROCESS_VIEW(user_data);

  g_source_remove(process_view->timeout_id);
}

static void pm_process_view_class_init(PmProcessViewClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
}

void pm_process_view_init(PmProcessView *self) {
  GListStore *model;
  GtkWidget *column_view;
  GtkTreeListModel *tree_model;
  GtkSortListModel *sort_model;
  GtkSorter *view_sorter;
  GtkSorter *tree_sorter;
  GtkSingleSelection *selection;
  PmSystemStatus *sys_stat;

  sys_stat = pm_system_status_new();
  system_status_get_status(sys_stat);

  // Get model
  model = g_list_store_new(PM_TYPE_PROCESS_ITEM);
  g_object_set_data(G_OBJECT(model), "cpu-delta", NULL);

  // Wrap model inside tree list model
  tree_model = gtk_tree_list_model_new(G_LIST_MODEL(model), FALSE, FALSE,
                                       process_item_get_children, NULL, NULL);

  column_view = gtk_column_view_new(NULL);

  // Wrap column view sorter with tree sorter
  // then wrap tree model with sort model (yes)⠀
  // ⠀⠀⠀⠀⠀⠀⠀⠀
  //  ⠟⢦⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
  //  ⢷⡄⠈⡓⠢⢄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣠⠤⠂⢹
  //  ⠈⡷⡄⠈⠲⢤⣈⠻⠉⠛⠉⠉⠁⠒⠖⠉⠉⠉⠒⠶⢦⣤⠴⠒⢉⣡⠴⠀⢀⠏
  // ⠀  ⢸⡿⡂⠀⠀⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣠⣴⡞⠉⠀⢀⣠⡞⠀
  //  ⠀⠀⢙⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠀⠀⢠⡼⡟⠀⠀
  // ⠀  ⠀⡼⠋⠀⣤⣀⠀⠀⠀⠀⠀⠈⠐⣂⣄⠀⠀⠀⠀⠀⠀⠀⢀⠀⣰⡟⠁⠀⠀
  // ⠀  ⢠⡇⠀⠀⠘⠛⠃⠀⠀⠀⠀⠾⣿⠿⠟⠉⠀⠀⠀⠀⠀⠀⠀⠀⢻⠀⠀⠀⠀
  // ⠀  ⢸⡇⢺⡀⠀⢠⡒⠠⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⡀⠀⠀⠸⡇⠀⠀⠀
  // ⠀  ⢸⡇⣘⠑⡀⠀⠙⢏⣁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠂⠀⣔⣇⠀⠀⠀
  // ⠀  ⢸⡇⡁⠀⢳⣶⣾⣷⣦⣄⣀⡀⣀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⠀⠀⠀
  // ⠀  ⠸⡇⠁⠀⠀⢏⠉⠀⠀⠙⠛⠛⠛⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⢈⡏⠀⠀⠀
  // ⠀  ⠀⠯⣀⣈⣀⣈⣐⣲⣄⣄⣤⣴⣆⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣈⣛⡧⠀⠀⠀
  //
  view_sorter = g_object_ref(gtk_column_view_get_sorter(GTK_COLUMN_VIEW(column_view)));
  tree_sorter =
      GTK_SORTER(gtk_tree_list_row_sorter_new(g_object_ref(view_sorter)));
  sort_model = gtk_sort_list_model_new(G_LIST_MODEL(tree_model), tree_sorter);

  selection = gtk_single_selection_new(G_LIST_MODEL(sort_model));

  // Apply sort model to column view sorter
  gtk_column_view_set_model(GTK_COLUMN_VIEW(column_view),
                            GTK_SELECTION_MODEL(selection));

  // show column separators
  gtk_column_view_set_show_column_separators(GTK_COLUMN_VIEW(column_view), TRUE);

  // Append columns
  gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view),
                                create_first_column("name", "Process Name"));
  gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view),
                                create_string_column("status", "Status"));
  gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view),
                                create_numeric_column("cpu", "CPU", "%"));
  gtk_column_view_append_column(
      GTK_COLUMN_VIEW(column_view), create_numeric_column("memory", "Memory", "MB"));
  gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view),
                                create_numeric_column("disk", "Disk", "MB/s"));
  gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view),
                                create_string_column("user", "User"));
  gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view),
                                create_string_column("priority", "Priority"));

  g_signal_connect(column_view, "map", G_CALLBACK(on_view_map), self);
  g_signal_connect(column_view, "unmap", G_CALLBACK(on_view_unmap), self);

  self->column_view = column_view;
  self->model = G_LIST_MODEL(model);
  self->sys_stat = sys_stat;
}

PmProcessView *process_view_new(void) {
  PmProcessView *process_view = g_object_new(PM_TYPE_PROCESS_VIEW, NULL);
  process_model_refresh(G_LIST_STORE(process_view->model));
  return process_view;
}

GtkWidget *process_view_get_view(PmProcessView *view) {
  return g_object_ref(view->column_view);
}