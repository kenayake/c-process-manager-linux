#include "components/process-details.h"
#include "components/search-bar.h"
#include "config.h"
#include "process-view.h"
#include "components/expander-cell.h"
#include "components/numeric-cell.h"
#include "components/table-cell.h"
#include "core/process-item.h"
#include "core/process-model.h"
#include "core/system-status.h"
#include "gdk/gdk.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include "graphene.h"
#include "gtk/gtkshortcut.h"
#include "gtk/gtksingleselection.h"
#include "process-manager-window.h"
#include <gtk/gtk.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>

typedef struct {
  const gchar *name;
  const gchar *suffix;
  gdouble max;
  PmProcessView *process_view;
} UserData;

typedef struct {
  PmProcessView *process_view;
  GtkListItem *list_item;
} SelectionData;

struct _PmProcessView {
  GObject parent_instance;

  GListModel *model;
  PmSystemStatus *sys_stat;
  GtkWidget *column_view;
  guint timeout_id;
  GtkWidget *popover;
  GtkSingleSelection *selection;
  GtkFilterListModel *filter_model;
};

G_DEFINE_FINAL_TYPE(PmProcessView, pm_process_view, G_TYPE_OBJECT)

static void show_alert(PmProcessView *self, const gchar *title, GError *error) {
  g_autoptr(GtkAlertDialog) alert;

  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET(self->column_view));
  GtkWindow *parent_window = GTK_WINDOW (root);

  alert = gtk_alert_dialog_new("%s", title);
  gtk_alert_dialog_set_detail(alert, error->message);
  gtk_alert_dialog_set_modal(alert, TRUE);
  gtk_alert_dialog_show(alert, parent_window);
}

static void kill_process(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  g_autoptr(GError) error = NULL;
  PmProcessView *self = PM_PROCESS_VIEW(user_data);
  GtkTreeListRow *activeTreeRow = gtk_single_selection_get_selected_item(self->selection);
  PmProcessItem *selectedProcess = gtk_tree_list_row_get_item(activeTreeRow);

  if (!process_item_kill_process(selectedProcess, &error)) {
    show_alert(self, "Kill Process Failed", error);
  }
}

static void open_properties(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  PmProcessView *self = PM_PROCESS_VIEW(user_data);
  GtkTreeListRow *activeTreeRow = gtk_single_selection_get_selected_item(self->selection);
  PmProcessItem *selectedProcess = gtk_tree_list_row_get_item(activeTreeRow);
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET(self->column_view));
  GtkWindow *parent_window = GTK_WINDOW (root);

  pid_t pid; gchararray name;
  PmProcessDetails *details;

  g_object_get(selectedProcess, "pid", &pid, "name", &name, NULL);

  details = process_details_new(pid, name, parent_window);
  process_details_show(details);
}

static void open_dir(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  g_autoptr(GError) error = NULL;
  PmProcessView *self = PM_PROCESS_VIEW(user_data);
  GtkTreeListRow *activeTreeRow = gtk_single_selection_get_selected_item(self->selection);
  PmProcessItem *selectedProcess = gtk_tree_list_row_get_item(activeTreeRow);
  
  // const gchar *type_name = g_type_name(G_OBJECT_TYPE(child2));
  if (!process_item_open_folder(selectedProcess, &error)) {
    show_alert(self, "Open Directory Failed", error);
  }
}

static GActionGroup *
create_action_group (PmProcessView *self) {
  const GActionEntry entries[] = {
      { "kill", kill_process, NULL, NULL, NULL },
      { "properties", open_properties, NULL, NULL, NULL },
      { "open-dir", open_dir, NULL, NULL, NULL }
  };
  GSimpleActionGroup *group;

  group = g_simple_action_group_new ();
  g_action_map_add_action_entries (G_ACTION_MAP (group), entries, G_N_ELEMENTS (entries), self);

  return G_ACTION_GROUP (group);
}

static void create_popover(PmProcessView *self) {
  GtkBuilder *builder;
  GMenuModel *menu;
  GActionGroup *group;
  GtkColumnView *column_view;

  column_view = GTK_COLUMN_VIEW(self->column_view);
  builder = gtk_builder_new_from_resource("/com/github/kenayake/ProcessManager/view/process-view.ui");
  menu = G_MENU_MODEL(gtk_builder_get_object(builder, "ctx_menu"));

  group = create_action_group(self);
  gtk_widget_insert_action_group(GTK_WIDGET(column_view), "ctx", group);

  self->popover = gtk_popover_menu_new_from_model(menu);
  gtk_widget_set_parent(self->popover, GTK_WIDGET(column_view));
  gtk_popover_set_has_arrow(GTK_POPOVER(self->popover), FALSE);
}

gboolean bind_numeric_to_label(GBinding *binding, const GValue *from_value,
                               GValue *to_value, gpointer user_data) {
  UserData *data = user_data;

  gdouble numeric_value = g_value_get_double(from_value);

  const g_autofree gchar *label_text = g_strdup_printf(
      data->suffix != NULL ? "%.1f %s" : "%.1f", numeric_value, data->suffix);

  g_value_set_string(to_value, label_text);

  return TRUE;
}

static void on_right_click(GtkGestureClick *self, gint n_press, gdouble x, gdouble y, gpointer user_data) {
  PmProcessView *process_view = PM_PROCESS_VIEW(user_data);
  GtkListItem *list_item =
      g_object_get_data(G_OBJECT(self), "list-item");
  guint position = gtk_list_item_get_position(list_item);
  GtkWidget *source;
  graphene_point_t point;
  graphene_point_t point_in_view;
  GdkRectangle rect;
  GtkPopover *popover;

  if (position == GTK_INVALID_LIST_POSITION)
    return;

  gtk_single_selection_set_selected(process_view->selection, position);

  source = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
  point = GRAPHENE_POINT_INIT((float)x, (float)y);

  if (!gtk_widget_compute_point(source, process_view->column_view,
                                &point, &point_in_view)) {
    g_warning("Could not convert click coordinates to column-view coordinates");
    return;
  }

  rect = (GdkRectangle){ (int)point_in_view.x, (int)point_in_view.y, 1, 1 };
  popover = GTK_POPOVER(process_view->popover);

  gtk_popover_set_pointing_to(popover, &rect);
  gtk_popover_set_position(popover, GTK_POS_BOTTOM);
  gtk_popover_set_offset(popover, 8, 8);
  gtk_popover_popup(popover);
}

static void setup_expander_column(GtkSignalListItemFactory *factory,
                                  GtkListItem *list_item, gpointer user_data) {
  GtkWidget *expander;
  GtkGesture *click;

  expander = pm_expander_cell_new(NULL);
  gtk_list_item_set_child(list_item, expander);

  click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), GDK_BUTTON_SECONDARY);
  g_object_set_data(G_OBJECT(click), "list-item", list_item);
  g_signal_connect_object(click, "pressed",
                        G_CALLBACK(on_right_click),
                        user_data, 0);
  gtk_widget_add_controller(expander, GTK_EVENT_CONTROLLER(click));
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
  // label = gtk_tree_expander_get_child(GTK_TREE_EXPANDER(expander));

  pm_expander_cell_set_list_row(PM_EXPANDER_CELL(expander), row);

  binding = g_object_bind_property(process, data->name, expander, "label",
                                   G_BINDING_SYNC_CREATE);

  key = g_strdup_printf("%s-key", data->name);

  g_object_set_data(G_OBJECT(list_item), key, binding);

  // gchar *text;
  // g_object_get(expander, "label", &text, NULL);

  g_object_unref(process);
}

static void setup_generic_column(GtkSignalListItemFactory *factory,
                                 GtkListItem *list_item, gpointer user_data) {
  GtkWidget *label;
  GtkGesture *click;

  label = pm_table_cell_new(NULL);

  gtk_list_item_set_child(list_item, label);

  click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), GDK_BUTTON_SECONDARY);
  g_object_set_data(G_OBJECT(click), "list-item", list_item);
  g_signal_connect_object(click, "pressed",
                        G_CALLBACK(on_right_click),
                        user_data, 0);
  gtk_widget_add_controller(label, GTK_EVENT_CONTROLLER(click));
}

static void setup_numeric_column(GtkSignalListItemFactory *factory,
                                 GtkListItem *list_item, gpointer user_data) {
  GtkWidget *label;
  UserData *data = user_data;
  GtkGesture *click;

  label = pm_numeric_cell_new(0.0, data->max, data->suffix);

  gtk_list_item_set_child(list_item, label);

  click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), GDK_BUTTON_SECONDARY);
  g_object_set_data(G_OBJECT(click), "list-item", list_item);
  g_signal_connect_object(click, "pressed",
                        G_CALLBACK(on_right_click),
                        data->process_view, 0);
  gtk_widget_add_controller(label, GTK_EVENT_CONTROLLER(click));
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
    // gtk_label_set_text(GTK_LABEL(label), "");
    g_object_set(label, "label", "", NULL);
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
    g_object_set(label, "raw_value", 0.0, NULL);
    return;
  }

  binding = g_object_bind_property(process, data->name, label, "raw_value",
                                   G_BINDING_SYNC_CREATE);

  // binding = g_object_bind_property_full(
  //     process, data->name, label, "label", G_BINDING_SYNC_CREATE,
  //     bind_numeric_to_label, NULL, data, NULL);

  key = g_strdup_printf("%s-key", data->name);

  g_object_set_data(G_OBJECT(list_item), key, binding);

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

//! Note to self: dont turn it into builder factory, its not worth it
static GtkColumnViewColumn *create_first_column(const gchar *prop_name,
                                                const gchar *title,
                                                PmProcessView *process_view) {
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *expr;
  GtkSorter *string_sorter;
  UserData *user_data = g_new0(UserData, 1);
  user_data->name = prop_name;
  user_data->suffix = NULL;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(setup_expander_column), process_view);

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
                                                  const gchar *suffix,
                                                  const gdouble max,
                                                PmProcessView *self) {
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *expr;
  GtkSorter *numeric_sorter;
  UserData *user_data = g_new0(UserData, 1);
  user_data->name = prop_name;
  user_data->suffix = suffix;
  user_data->max = max;
  user_data->process_view = self;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(setup_numeric_column),
                   user_data);

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
                                                 const gchar *title,
                                                  PmProcessView *self) {
  GtkListItemFactory *factory;
  GtkColumnViewColumn *column;
  GtkExpression *expr;
  GtkSorter *string_sorter;
  UserData *user_data = g_new0(UserData, 1);
  user_data->name = prop_name;
  user_data->suffix = NULL;

  factory = gtk_signal_list_item_factory_new();

  g_signal_connect(factory, "setup", G_CALLBACK(setup_generic_column), self);

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
  GtkSorter *sort_model =
      process_view_get_sorter(GTK_COLUMN_VIEW(view->column_view));
  GListModel *model = view->model;
  guint64 *cpu_delta;
  // g_info("timeout started");

  if (model != NULL) {

    system_status_get_status(view->sys_stat);
    cpu_delta = system_status_get_cpu_delta(view->sys_stat);
    g_object_set_data_full(G_OBJECT(model), "cpu-delta", cpu_delta,
                           (GDestroyNotify)g_free);

    process_model_refresh(G_LIST_STORE(model));

    // Update sort
    gtk_sorter_changed(sort_model, GTK_SORTER_CHANGE_DIFFERENT);
  }

  return G_SOURCE_CONTINUE;
}

void on_view_map(GtkWidget *self, gpointer user_data) {
  PmProcessView *process_view = PM_PROCESS_VIEW(user_data);

  process_view->timeout_id = g_timeout_add(
      1000, on_timeout_tick, process_view); // replace with interval setting
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
  PmSystemStatus *sys_stat;
  gdouble disk_usage;
  guint64 mem_total;

  sys_stat = pm_system_status_new();
  system_status_get_status(sys_stat);
  g_object_get(sys_stat,  "mem_total", &mem_total,
               "disk_usage", &disk_usage, NULL);

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
  view_sorter =
      g_object_ref(gtk_column_view_get_sorter(GTK_COLUMN_VIEW(column_view)));
  tree_sorter =
      GTK_SORTER(gtk_tree_list_row_sorter_new(g_object_ref(view_sorter)));

  self->filter_model = gtk_filter_list_model_new(G_LIST_MODEL(tree_model), NULL);
  sort_model = gtk_sort_list_model_new(G_LIST_MODEL(self->filter_model), tree_sorter);

  self->selection = gtk_single_selection_new(G_LIST_MODEL(sort_model));

  // Apply sort model to column view sorter
  gtk_column_view_set_model(GTK_COLUMN_VIEW(column_view),
                            GTK_SELECTION_MODEL(self->selection));

  // show column separators
  gtk_column_view_set_show_column_separators(GTK_COLUMN_VIEW(column_view),
                                             TRUE);

  // Append columns
  gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view),
                                create_first_column("name", "Name", self));
  gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view),
                                create_string_column("status", "Status", self));
  gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view),
                                create_numeric_column("pid", "PID", NULL, INTMAX_MAX, self));
  gtk_column_view_append_column(
      GTK_COLUMN_VIEW(column_view),
      create_numeric_column("cpu", "CPU", "%", 100.0, self));
  gtk_column_view_append_column(
      GTK_COLUMN_VIEW(column_view),
      create_numeric_column("memory", "Memory", " MB", (double)mem_total/1e+6, self));
  gtk_column_view_append_column(
      GTK_COLUMN_VIEW(column_view),
      create_numeric_column("disk", "Disk", " MB/s", disk_usage, self));
  gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view),
                                create_string_column("user", "User", self));
  gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view),
                                create_string_column("priority", "Priority", self));

  g_signal_connect(column_view, "map", G_CALLBACK(on_view_map), self);
  g_signal_connect(column_view, "unmap", G_CALLBACK(on_view_unmap), self);

  self->column_view = column_view;
  self->model = G_LIST_MODEL(model);
  self->sys_stat = sys_stat;

  // Context Menu
  create_popover(self);
}

PmProcessView *process_view_new(GtkFilter *filter) {
  PmProcessView *process_view = g_object_new(PM_TYPE_PROCESS_VIEW, NULL);
  process_model_refresh(G_LIST_STORE(process_view->model));
  gtk_filter_list_model_set_filter(process_view->filter_model, filter);
  return process_view;
}

GtkWidget *process_view_get_view(PmProcessView *view) {
  return g_object_ref(view->column_view);
}