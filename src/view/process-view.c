#include "process-view.h"
#include "config.h"
#include "core/process-item.h"
#include "core/process-model.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include "gtk/gtkshortcut.h"
#include <gtk/gtk.h>

typedef struct {
  const gchar *name;
  const gchar *suffix;
} UserData;

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
  ProcessItem *item;
  GtkWidget *expander;
  GtkWidget *label;
  gchar *text;

  const UserData *data = user_data;

  row = GTK_TREE_LIST_ROW(gtk_list_item_get_item(list_item));
  item = PROCESS_ITEM(gtk_tree_list_row_get_item(row));
  expander = gtk_list_item_get_child(list_item);
  label = gtk_tree_expander_get_child(GTK_TREE_EXPANDER(expander));

  gtk_tree_expander_set_list_row(GTK_TREE_EXPANDER(expander), row);
  g_object_get(item, data->name, &text, NULL);
  gtk_label_set_text(GTK_LABEL(label), text);

  g_object_unref(item);
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
  ProcessItem *process;
  GtkWidget *label;
  gchar *text;

  const UserData *data = user_data;

  row = GTK_TREE_LIST_ROW (gtk_list_item_get_item (list_item));
  process = gtk_tree_list_row_get_item (row);
  label = gtk_list_item_get_child(list_item);

  if (process == NULL) {
    gtk_label_set_text(GTK_LABEL(label), "");
    return;
  }

  g_object_get(process, data->name, &text, NULL);

  gtk_label_set_text(GTK_LABEL(label), text);
}

static void bind_numeric_column(GtkSignalListItemFactory *factory,
                                GtkListItem *list_item, gpointer user_data) {
  GtkTreeListRow *row;
  ProcessItem *process;
  GtkWidget *label;
  gdouble value;
  gchar text[32];

  const UserData *data = user_data;

  row = GTK_TREE_LIST_ROW (gtk_list_item_get_item (list_item));
  process = gtk_tree_list_row_get_item (row);
  label = gtk_list_item_get_child(list_item);

  if (process == NULL) {
    gtk_label_set_text(GTK_LABEL(label), "");
    return;
  }

  g_object_get(process, data->name, &value, NULL);

  g_snprintf(text, sizeof(text), data->suffix != NULL ? "%.1f %s" : "%.1f",
             value, data->suffix);

  gtk_label_set_text(GTK_LABEL(label), text);
}

static GtkColumnViewColumn *create_first_column(const gchar *prop_name,
                                                const gchar *title) {
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *expr;
  GtkSorter *string_sorter;
  UserData *user_data = g_new(UserData, 1);
  user_data->name = prop_name;
  user_data->suffix = NULL;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(setup_expander_column), NULL);

  g_signal_connect(factory, "bind", G_CALLBACK(bind_expander_column),
                   user_data);

  column = gtk_column_view_column_new(title, GTK_LIST_ITEM_FACTORY(factory));

  // set default column properties
  gtk_column_view_column_set_resizable(column, true);
  gtk_column_view_column_set_expand(column, TRUE);

  // Setup expression-based sorter
  expr = gtk_property_expression_new(PROCESS_TYPE_ITEM, NULL, prop_name);
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
  UserData *user_data = g_new(UserData, 1);
  user_data->name = prop_name;
  user_data->suffix = suffix;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(setup_generic_column), NULL);

  g_signal_connect(factory, "bind", G_CALLBACK(bind_numeric_column), user_data);

  column = gtk_column_view_column_new(title, GTK_LIST_ITEM_FACTORY(factory));

  // set default column properties
  gtk_column_view_column_set_resizable(column, true);
  gtk_column_view_column_set_expand(column, TRUE);

  // Setup expression-based sorter
  expr = gtk_property_expression_new(PROCESS_TYPE_ITEM, NULL, prop_name);
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
  UserData *user_data = g_new(UserData, 1);
  user_data->name = prop_name;
  user_data->suffix = NULL;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(setup_generic_column), NULL);

  g_signal_connect(factory, "bind", G_CALLBACK(bind_string_column), user_data);

  column = gtk_column_view_column_new(title, GTK_LIST_ITEM_FACTORY(factory));

  // set default column properties
  gtk_column_view_column_set_resizable(column, true);
  gtk_column_view_column_set_expand(column, TRUE);

  // Setup expression-based sorter
  expr = gtk_property_expression_new(PROCESS_TYPE_ITEM, NULL, prop_name);
  string_sorter = GTK_SORTER(gtk_string_sorter_new(expr));

  // Bind the sorter to the column
  gtk_column_view_column_set_sorter(column, string_sorter);
  g_object_unref(string_sorter);

  return column;
}

GtkWidget *process_view_new(void) {
  GListModel          *model;
  GtkWidget           *view;
  GtkTreeListModel    *tree_model;
  GtkSortListModel    *sort_model;
  GtkSorter           *view_sorter;
  GtkSorter           *tree_sorter;
  GtkSingleSelection  *selection;

  // Get model
  model = process_model_new();
  if (model == NULL) {
    return gtk_label_new("No processes");
  }

  // Wrap model inside tree list model
  tree_model = gtk_tree_list_model_new(G_LIST_MODEL(model), FALSE, FALSE,
                                       process_item_get_children, NULL, NULL);

  view = gtk_column_view_new(NULL);

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
  view_sorter = g_object_ref(gtk_column_view_get_sorter(GTK_COLUMN_VIEW(view)));
  tree_sorter = GTK_SORTER (gtk_tree_list_row_sorter_new (g_object_ref (view_sorter)));
  sort_model = gtk_sort_list_model_new(G_LIST_MODEL(tree_model), tree_sorter);

  selection = gtk_single_selection_new(G_LIST_MODEL(sort_model));

  // Apply sort model to column view sorter
  gtk_column_view_set_model(GTK_COLUMN_VIEW(view),
                            GTK_SELECTION_MODEL(selection));

  // show column separators
  gtk_column_view_set_show_column_separators(GTK_COLUMN_VIEW(view), TRUE);

  // Append columns
  gtk_column_view_append_column(GTK_COLUMN_VIEW(view),
                                create_first_column("name", "Process Name"));
  gtk_column_view_append_column(GTK_COLUMN_VIEW(view),
                                create_string_column("status", "Status"));
  gtk_column_view_append_column(GTK_COLUMN_VIEW(view),
                                create_numeric_column("cpu", "CPU", "%"));
  gtk_column_view_append_column(
      GTK_COLUMN_VIEW(view), create_numeric_column("memory", "Memory", "MB"));
  gtk_column_view_append_column(GTK_COLUMN_VIEW(view),
                                create_numeric_column("disk", "Disk", "MB/s"));
  gtk_column_view_append_column(GTK_COLUMN_VIEW(view),
                                create_string_column("user", "User"));
  gtk_column_view_append_column(GTK_COLUMN_VIEW(view),
                                create_string_column("priority", "Priority"));

  return view;
}