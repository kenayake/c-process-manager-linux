/* process-manager-window.c
 *
 * Copyright 2026 Unknown
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "config.h"
#include "glib-object.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include "components/search-bar.h"
#include "view/process-view.h"

#include "process-manager-window.h"

struct _ProcessManagerWindow {
  GtkApplicationWindow parent_instance;

  /* Template widgets */
  GtkWidget *scrolled_window;
  PmSearchBar *search_bar;
};

G_DEFINE_FINAL_TYPE(ProcessManagerWindow, process_manager_window,
                    GTK_TYPE_APPLICATION_WINDOW)

enum {
  PROP_0,
  PROP_SEARCH_BAR,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

static void process_manager_window_get_property(GObject *object,
                                                guint property_id,
                                                GValue *value,
                                                GParamSpec *pspec) {
  ProcessManagerWindow *self = PROCESS_MANAGER_WINDOW(object);

  switch (property_id) {
  case PROP_SEARCH_BAR:
    g_value_set_object(value, self->search_bar);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void
process_manager_window_class_init(ProcessManagerWindowClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  g_type_ensure(PM_TYPE_SEARCH_BAR);

  object_class->get_property = process_manager_window_get_property;

  properties[PROP_SEARCH_BAR] =
      g_param_spec_object("search-bar", "Search bar",
                          "The window's process search bar",
                          PM_TYPE_SEARCH_BAR,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties(object_class, N_PROPERTIES, properties);

  gtk_widget_class_set_template_from_resource(
      widget_class,
      "/com/github/kenayake/ProcessManager/process-manager-window.ui");
  gtk_widget_class_bind_template_child(widget_class, ProcessManagerWindow,
                                       scrolled_window);
  gtk_widget_class_bind_template_child(widget_class, ProcessManagerWindow,
                                       search_bar);
}

static void process_manager_window_init(ProcessManagerWindow *self) {
  PmProcessView *process_view;
  GtkCssProvider *provider;
  GtkWidget *cl;
  GtkFilter *filter;

  gtk_widget_init_template(GTK_WIDGET(self));

  provider = gtk_css_provider_new();
  gtk_css_provider_load_from_path(provider, "src/global.css");     

  gtk_style_context_add_provider_for_display(
    gdk_display_get_default(),
    GTK_STYLE_PROVIDER(provider),
    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
  );
  g_object_unref (provider);

  filter = pm_search_bar_get_filter(self->search_bar);

  process_view = process_view_new(filter);

  cl = process_view_get_view(process_view);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(self->scrolled_window),
                                cl);
}
