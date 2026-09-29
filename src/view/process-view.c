#include "config.h"
#include "process-view.h"
#include "core/process-item.h"
#include "core/process-model.h"
#include "glib-object.h"
#include "glibconfig.h"
#include "gtk/gtkshortcut.h"
#include <gtk/gtk.h>

static void name_setup(GtkSignalListItemFactory *factory,
                       GtkListItem *list_item, gpointer user_data) {
  GtkWidget *label;

  label = gtk_label_new(NULL);

  gtk_list_item_set_child(list_item, label);
}

static void name_bind(GtkSignalListItemFactory *factory, GtkListItem *list_item,
                      gpointer user_data) {
  ProcessItem *process;
  GtkWidget *label;
  gchar *text;

  process = PROCESS_ITEM(gtk_list_item_get_item(list_item));
  label = gtk_list_item_get_child(list_item);

  if (process == NULL) {
    gtk_label_set_text(GTK_LABEL(label), "");
    return;
  }

  g_object_get(process, "name", &text, NULL);

  gtk_label_set_text(GTK_LABEL(label), text);
}

static void status_setup(GtkSignalListItemFactory *factory,
                         GtkListItem *list_item, gpointer user_data) {
  GtkWidget *label;

  label = gtk_label_new(NULL);

  gtk_list_item_set_child(list_item, label);
}

static void status_bind(GtkSignalListItemFactory *factory,
                        GtkListItem *list_item, gpointer user_data) {
  ProcessItem *process;
  GtkWidget *label;
  gchar *text;

  process = gtk_list_item_get_item(list_item);
  label = gtk_list_item_get_child(list_item);

  if (process == NULL) {
    gtk_label_set_text(GTK_LABEL(label), "");
    return;
  }

  g_object_get(process, "status", &text, NULL);

  gtk_label_set_text(GTK_LABEL(label), text);
}

static void cpu_setup(GtkSignalListItemFactory *factory, GtkListItem *list_item,
                      gpointer user_data) {
  GtkWidget *label;

  label = gtk_label_new(NULL);

  gtk_list_item_set_child(list_item, label);
}

static void cpu_bind(GtkSignalListItemFactory *factory, GtkListItem *list_item,
                     gpointer user_data) {
  ProcessItem *process;
  GtkWidget *label;
  gdouble value;
  gchar text[32];

  process = gtk_list_item_get_item(list_item);
  label = gtk_list_item_get_child(list_item);

  if (process == NULL) {
    gtk_label_set_text(GTK_LABEL(label), "");
    return;
  }

  g_object_get(process, "cpu", &value, NULL);

  g_snprintf(text, sizeof(text), "%.1f%%", value);

  gtk_label_set_text(GTK_LABEL(label), text);
}

static void memory_setup(GtkSignalListItemFactory *factory,
                         GtkListItem *list_item, gpointer user_data) {
  GtkWidget *label;

  label = gtk_label_new(NULL);

  gtk_list_item_set_child(list_item, label);
}

static void memory_bind(GtkSignalListItemFactory *factory,
                        GtkListItem *list_item, gpointer user_data) {
  ProcessItem *process;
  GtkWidget *label;
  gdouble value;
  gchar text[32];

  process = gtk_list_item_get_item(list_item);
  label = gtk_list_item_get_child(list_item);

  if (process == NULL) {
    gtk_label_set_text(GTK_LABEL(label), "");
    return;
  }

  g_object_get(process, "memory", &value, NULL);

  g_snprintf(text, sizeof(text), "%.1f MB", value);

  gtk_label_set_text(GTK_LABEL(label), text);
}

static void disk_setup(GtkSignalListItemFactory *factory,
                       GtkListItem *list_item, gpointer user_data) {
  GtkWidget *label;

  label = gtk_label_new(NULL);

  gtk_list_item_set_child(list_item, label);
}

static void disk_bind(GtkSignalListItemFactory *factory, GtkListItem *list_item,
                      gpointer user_data) {
  ProcessItem *process;
  GtkWidget *label;
  gdouble value;
  gchar text[32];

  process = gtk_list_item_get_item(list_item);
  label = gtk_list_item_get_child(list_item);

  if (process == NULL) {
    gtk_label_set_text(GTK_LABEL(label), "");
    return;
  }

  g_object_get(process, "disk", &value, NULL);

  g_snprintf(text, sizeof(text), "%.1f MB/s", value);

  gtk_label_set_text(GTK_LABEL(label), text);
}

static void user_setup(GtkSignalListItemFactory *factory,
                       GtkListItem *list_item, gpointer user_data) {
  GtkWidget *label;

  label = gtk_label_new(NULL);

  gtk_list_item_set_child(list_item, label);
}

static void user_bind(GtkSignalListItemFactory *factory, GtkListItem *list_item,
                      gpointer user_data) {
  ProcessItem *process;
  GtkWidget *label;
  gchar *text;

  process = gtk_list_item_get_item(list_item);
  label = gtk_list_item_get_child(list_item);

  if (process == NULL) {
    gtk_label_set_text(GTK_LABEL(label), "");
    return;
  }

  g_object_get(process, "user", &text, NULL);

  gtk_label_set_text(GTK_LABEL(label), text);
}

static void priority_setup(GtkSignalListItemFactory *factory,
                           GtkListItem *list_item, gpointer user_data) {
  GtkWidget *label;

  label = gtk_label_new(NULL);

  gtk_list_item_set_child(list_item, label);
}

static void priority_bind(GtkSignalListItemFactory *factory,
                          GtkListItem *list_item, gpointer user_data) {
  ProcessItem *process;
  GtkWidget *label;
  gchar *text;

  process = gtk_list_item_get_item(list_item);
  label = gtk_list_item_get_child(list_item);

  if (process == NULL) {
    gtk_label_set_text(GTK_LABEL(label), "");
    return;
  }

  g_object_get(process, "priority", &text, NULL);

  gtk_label_set_text(GTK_LABEL(label), text);
}

static GtkColumnViewColumn *create_name_column(void) {
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *expr;
  GtkSorter *string_sorter;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(name_setup), NULL);

  g_signal_connect(factory, "bind", G_CALLBACK(name_bind), NULL);

  column = gtk_column_view_column_new("Process Name",
                                      GTK_LIST_ITEM_FACTORY(factory));



  gtk_column_view_column_set_expand(column, TRUE);
  
  // Setup expression-based sorter
  expr = gtk_property_expression_new (PROCESS_TYPE_ITEM, NULL, "name");
  string_sorter = GTK_SORTER (gtk_string_sorter_new (expr));

  // Bind the sorter to the column
  gtk_column_view_column_set_sorter (column, string_sorter);
  g_object_unref (string_sorter);                     

  return column;
}

static GtkColumnViewColumn *create_status_column(void) {
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *expr;
  GtkSorter *string_sorter;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(status_setup), NULL);

  g_signal_connect(factory, "bind", G_CALLBACK(status_bind), NULL);

  column = gtk_column_view_column_new("Status", GTK_LIST_ITEM_FACTORY(factory));

  // Setup expression-based sorter
  expr = gtk_property_expression_new (PROCESS_TYPE_ITEM, NULL, "status");
  string_sorter = GTK_SORTER (gtk_string_sorter_new (expr));

  // Bind the sorter to the column
  gtk_column_view_column_set_sorter (column, string_sorter);
  g_object_unref (string_sorter);

  return column;
}

static GtkColumnViewColumn *create_cpu_column(void) {
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *expr;
  GtkSorter *numeric_sorter;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(cpu_setup), NULL);

  g_signal_connect(factory, "bind", G_CALLBACK(cpu_bind), NULL);

  column = gtk_column_view_column_new("CPU", GTK_LIST_ITEM_FACTORY(factory));

  // Setup expression-based sorter
  expr = gtk_property_expression_new (PROCESS_TYPE_ITEM, NULL, "cpu");
  numeric_sorter = GTK_SORTER (gtk_numeric_sorter_new (expr));

  // Bind the sorter to the column
  gtk_column_view_column_set_sorter (column, numeric_sorter);
  g_object_unref (numeric_sorter);

  return column;
}

static GtkColumnViewColumn *create_memory_column(void) {
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *expr;
  GtkSorter *numeric_sorter;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(memory_setup), NULL);

  g_signal_connect(factory, "bind", G_CALLBACK(memory_bind), NULL);

  column = gtk_column_view_column_new("Memory", GTK_LIST_ITEM_FACTORY(factory));

  // Setup expression-based sorter
  expr = gtk_property_expression_new (PROCESS_TYPE_ITEM, NULL, "memory");
  numeric_sorter = GTK_SORTER (gtk_numeric_sorter_new (expr));

  // Bind the sorter to the column
  gtk_column_view_column_set_sorter (column, numeric_sorter);
  g_object_unref (numeric_sorter);

  return column;
}

static GtkColumnViewColumn *create_disk_column(void) {
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *expr;
  GtkSorter *numeric_sorter;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(disk_setup), NULL);

  g_signal_connect(factory, "bind", G_CALLBACK(disk_bind), NULL);

  column = gtk_column_view_column_new("Disk", GTK_LIST_ITEM_FACTORY(factory));

  // Setup expression-based sorter
  expr = gtk_property_expression_new (PROCESS_TYPE_ITEM, NULL, "disk");
  numeric_sorter = GTK_SORTER (gtk_numeric_sorter_new (expr));

  // Bind the sorter to the column
  gtk_column_view_column_set_sorter (column, numeric_sorter);
  g_object_unref (numeric_sorter);

  return column;
}

static GtkColumnViewColumn *create_user_column(void) {
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *expr;
  GtkSorter *string_sorter;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(user_setup), NULL);

  g_signal_connect(factory, "bind", G_CALLBACK(user_bind), NULL);

  column = gtk_column_view_column_new("User", GTK_LIST_ITEM_FACTORY(factory));

  // Setup expression-based sorter
  expr = gtk_property_expression_new (PROCESS_TYPE_ITEM, NULL, "user");
  string_sorter = GTK_SORTER (gtk_string_sorter_new (expr));

  // Bind the sorter to the column
  gtk_column_view_column_set_sorter (column, string_sorter);
  g_object_unref (string_sorter);

  return column;
}

static GtkColumnViewColumn *create_priority_column(void) {
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *expr;
  GtkSorter *string_sorter;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(priority_setup), NULL);

  g_signal_connect(factory, "bind", G_CALLBACK(priority_bind), NULL);

  column =
      gtk_column_view_column_new("Priority", GTK_LIST_ITEM_FACTORY(factory));

  // Setup expression-based sorter
  expr = gtk_property_expression_new (PROCESS_TYPE_ITEM, NULL, "priority");
  string_sorter = GTK_SORTER (gtk_string_sorter_new (expr));

  // Bind the sorter to the column
  gtk_column_view_column_set_sorter (column, string_sorter);
  g_object_unref (string_sorter);

  return column;
}

GtkWidget *process_view_new(void) {
  GListModel *model;
  GtkSingleSelection *selection;
  GtkWidget *view;
  GtkSorter *view_sorter;
  GtkSortListModel *sort_model;

  // Get model
  model = process_model_new();
  if (model == NULL) {
    return gtk_label_new("No processes");
  }

  view = gtk_column_view_new(NULL);

  // Initialize sort model with column view sorter and process model
  view_sorter = g_object_ref (gtk_column_view_get_sorter (GTK_COLUMN_VIEW(view)));
  sort_model = gtk_sort_list_model_new (G_LIST_MODEL (model), view_sorter);

  selection = gtk_single_selection_new(G_LIST_MODEL (sort_model));
  
  // Apply sort model to column view sorter
  gtk_column_view_set_model (GTK_COLUMN_VIEW(view), GTK_SELECTION_MODEL (selection));
  
  // show column separators
  gtk_column_view_set_show_column_separators(GTK_COLUMN_VIEW(view), TRUE);

  // Append columns
  gtk_column_view_append_column(GTK_COLUMN_VIEW(view), create_name_column());
  gtk_column_view_append_column(GTK_COLUMN_VIEW(view), create_status_column());
  gtk_column_view_append_column(GTK_COLUMN_VIEW(view), create_cpu_column());
  gtk_column_view_append_column(GTK_COLUMN_VIEW(view), create_memory_column());
  gtk_column_view_append_column(GTK_COLUMN_VIEW(view), create_disk_column());
  gtk_column_view_append_column(GTK_COLUMN_VIEW(view), create_user_column());
  gtk_column_view_append_column(GTK_COLUMN_VIEW(view), create_priority_column());

  return view;
}