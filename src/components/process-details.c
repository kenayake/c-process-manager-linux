#include "process-details.h"
#include "glib-object.h"
#include "glib.h"
#include <gtk/gtk.h>

struct _PmProcessDetails {
  GtkWindow parent_instance;
  gint64 pid;
  gchararray name;
  gchararray status;
  GtkTextView *text_view;
  GtkButton *button;
};

G_DEFINE_FINAL_TYPE(PmProcessDetails, pm_process_details, GTK_TYPE_WINDOW)

enum { PROP_0, PROP_PID, PROP_NAME, N_PROPERTIES };

static GParamSpec *properties[N_PROPERTIES];

static void pm_process_details_set_property(GObject *object, guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {
  PmProcessDetails *self = PM_PROCESS_DETAILS(object);

  switch (property_id) {
  case PROP_PID:
    self->pid = g_value_get_int64(value);
    break;
  case PROP_NAME:
    g_free(self->name);
    self->name = g_value_dup_string(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pm_process_details_get_property(GObject *object, guint property_id,
                                            GValue *value, GParamSpec *pspec) {
  PmProcessDetails *self = PM_PROCESS_DETAILS(object);

  switch (property_id) {
  case PROP_PID:
    g_value_set_int64(value, self->pid);
    break;
  case PROP_NAME:
    g_value_set_string(value, self->name);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static gchararray title_factory(PmProcessDetails *self, gchararray name) {
  return g_strdup_printf("Viewing %s Details", name);
}

static void button_click(GtkButton *self, PmProcessDetails *details) {
  gtk_window_close(GTK_WINDOW(details));
}

static void pm_process_details_finalize(GObject *object) {
  PmProcessDetails *self = PM_PROCESS_DETAILS(object);
  g_free(self->name);
  g_free(self->status);
  G_OBJECT_CLASS(pm_process_details_parent_class)->finalize(object);
}

static void pm_process_details_class_init(PmProcessDetailsClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = pm_process_details_set_property;
  object_class->get_property = pm_process_details_get_property;
  object_class->finalize = pm_process_details_finalize;

  properties[PROP_PID] =
      g_param_spec_int64("pid", "PID", "The process ID", G_MININT64, G_MAXINT64,
                         0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_NAME] =
      g_param_spec_string("name", "Name", "The process name", NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties(object_class, N_PROPERTIES, properties);

  gtk_widget_class_set_template_from_resource(
      GTK_WIDGET_CLASS(klass),
      "/com/github/kenayake/ProcessManager/components/process-details.ui");

  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass),
                                       PmProcessDetails, text_view);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass),
                                       PmProcessDetails, button);
  gtk_widget_class_bind_template_callback(GTK_WIDGET_CLASS(klass),
                                          title_factory);
}

static void pm_process_details_init(PmProcessDetails *self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  g_signal_connect(self->button, "clicked", G_CALLBACK(button_click), self);
}

static void on_file_loaded(GObject *source_object, GAsyncResult *res,
                           gpointer user_data) {
  GFile *file = G_FILE(source_object);
  gchar *contents = NULL;
  gsize length = 0;
  GError *error = NULL;
  PmProcessDetails *details = PM_PROCESS_DETAILS(user_data);
  GtkTextBuffer *buffer;

  if (g_file_load_contents_finish(file, res, &contents, &length, NULL,
                                  &error)) {

    buffer = gtk_text_view_get_buffer(details->text_view);

    gtk_text_buffer_set_text(buffer, contents, length);

    g_free(contents);
  } else {
    g_printerr("Error reading file: %s\n", error->message);
    g_error_free(error);
  }
}

static void process_details_get(PmProcessDetails *self) {
  g_autofree gchararray path = g_strdup_printf("/proc/%ld/status", self->pid);
  GFile *file = g_file_new_for_path(path);

  g_file_load_contents_async(file, NULL, on_file_loaded, self);
}

PmProcessDetails *process_details_new(pid_t id, const gchararray name,
                                      GtkWindow *parent_window) {
  PmProcessDetails *details = g_object_new(PM_TYPE_PROCESS_DETAILS, "pid",
                                           (gint64)id, "name", name, NULL);

  gtk_window_set_transient_for(GTK_WINDOW(details), parent_window);

  process_details_get(details);

  return details;
}

void process_details_show(PmProcessDetails *self) {
  gtk_window_present(GTK_WINDOW(self));
}