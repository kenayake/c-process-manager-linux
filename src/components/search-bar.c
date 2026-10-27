#include "components/search-bar.h"
#include "core/process-item.h"
#include "glib-object.h"
#include "gtk/gtk.h"
#include "gtk/gtkdropdown.h"
#include "gtk/gtkexpression.h"
#include <stdlib.h>

struct _PmSearchBar {
  GtkBox parent_instance;

  GtkSearchEntry *search_entry;
  GtkCustomFilter *process_filter;
};

G_DEFINE_FINAL_TYPE(PmSearchBar, pm_search_bar, GTK_TYPE_BOX)

static void search_bar_changed(GtkSearchEntry *self, PmSearchBar *data) {
  const gchar *search_text = gtk_editable_get_text(GTK_EDITABLE(self));

  gtk_custom_filter_set_filter_func(data->process_filter, process_item_match_func, (gpointer) search_text, NULL);
  gtk_filter_changed(GTK_FILTER(data->process_filter), GTK_FILTER_CHANGE_DIFFERENT);
}

static void pm_search_bar_finalize(GObject *object) {
  PmSearchBar *self = PM_SEARCH_BAR(object);

  g_clear_object(&self->process_filter);

  G_OBJECT_CLASS(pm_search_bar_parent_class)->finalize(object);
}

static void pm_search_bar_class_init(PmSearchBarClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = pm_search_bar_finalize;

  gtk_widget_class_set_template_from_resource(
      GTK_WIDGET_CLASS(klass),
      "/com/github/kenayake/ProcessManager/components/search-bar.ui");
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), PmSearchBar,
                                       search_entry);
}

static void pm_search_bar_init(PmSearchBar *self) {
  self->process_filter = gtk_custom_filter_new(process_item_match_func, NULL, NULL);

  gtk_widget_init_template(GTK_WIDGET(self));
  g_signal_connect(self->search_entry, "search-changed", G_CALLBACK(search_bar_changed), self);
}

GtkWidget *pm_search_bar_new(void) {
  return GTK_WIDGET(g_object_new(PM_TYPE_SEARCH_BAR, NULL));
}

GtkFilter *pm_search_bar_get_filter(PmSearchBar *self) {
  g_return_val_if_fail(PM_IS_SEARCH_BAR(self), NULL);
  return GTK_FILTER(self->process_filter);
}