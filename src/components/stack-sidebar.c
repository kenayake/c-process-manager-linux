#include "components/stack-sidebar.h"
#include "glib-object.h"
#include "glibconfig.h"
#include "gtk/gtk.h"

struct _PmStackSidebar {
  GtkBox parent_instance;

  GtkToggleButton *show_text_toggle;
  GtkBox *top_items;
  GtkBox *bottom_items;
  GtkStack *stack;
  GtkSelectionModel *pages;
  gchar *bottom_pages;
  gint icon_size;
};

enum {
  PROP_0,
  PROP_BOTTOM_PAGES,
  PROP_ICON_SIZE,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_FINAL_TYPE(PmStackSidebar, pm_stack_sidebar, GTK_TYPE_BOX)

static void pm_stack_sidebar_update_page(GtkStackPage *page,
                                         GParamSpec *pspec,
                                         GtkToggleButton *button) {
  g_autofree gchar *title = NULL;
  g_autofree gchar *name = NULL;
  g_autofree gchar *icon_name = NULL;
  GtkWidget *content;
  GtkWidget *icon;
  GtkWidget *text;
  gboolean visible;

  g_object_get(page, "title", &title, "name", &name, "icon-name", &icon_name,
               "visible", &visible, NULL);
  content = gtk_button_get_child(GTK_BUTTON(button));
  icon = gtk_widget_get_first_child(content);
  text = gtk_widget_get_next_sibling(icon);
  gtk_image_set_from_icon_name(GTK_IMAGE(icon), icon_name);
  gtk_label_set_text(GTK_LABEL(text), title != NULL ? title : name);
  gtk_accessible_update_property(GTK_ACCESSIBLE(button),
                                 GTK_ACCESSIBLE_PROPERTY_LABEL,
                                 title != NULL ? title : name, -1);
  gtk_widget_set_visible(GTK_WIDGET(button), visible);
}

static void pm_stack_sidebar_update_buttons(PmStackSidebar *self) {
  GtkBox *groups[] = {self->top_items, self->bottom_items};
  const gboolean show_text =
      gtk_toggle_button_get_active(self->show_text_toggle);

  for (guint i = 0; i < G_N_ELEMENTS(groups); i++) {
    GtkWidget *button = gtk_widget_get_first_child(GTK_WIDGET(groups[i]));

    while (button != NULL) {
      GtkStackPage *page =
          g_object_get_data(G_OBJECT(button), "pm-stack-page");
      g_autoptr(GtkWidget) page_child = NULL;
      GtkWidget *content = gtk_button_get_child(GTK_BUTTON(button));
      GtkWidget *text = gtk_widget_get_next_sibling(
          gtk_widget_get_first_child(content));

      gtk_widget_set_visible(text, show_text);
      if (page != NULL && self->stack != NULL) {
        g_object_get(page, "child", &page_child, NULL);
        gtk_toggle_button_set_active(
            GTK_TOGGLE_BUTTON(button),
            page_child != NULL &&
                gtk_stack_get_visible_child(self->stack) == page_child);
      }

      button = gtk_widget_get_next_sibling(button);
    }
  }
}

static void pm_stack_sidebar_update_icon_size(PmStackSidebar *self) {
  GtkBox *groups[] = {self->top_items, self->bottom_items};

  for (guint i = 0; i < G_N_ELEMENTS(groups); i++) {
    GtkWidget *button;

    if (groups[i] == NULL)
      continue;

    for (button = gtk_widget_get_first_child(GTK_WIDGET(groups[i]));
         button != NULL; button = gtk_widget_get_next_sibling(button)) {
      GtkWidget *content = gtk_button_get_child(GTK_BUTTON(button));
      GtkWidget *icon = gtk_widget_get_first_child(content);

      gtk_image_set_pixel_size(GTK_IMAGE(icon), self->icon_size);
    }
  }
}

static void pm_stack_sidebar_page_button_clicked(GtkButton *button,
                                                PmStackSidebar *self) {
  GtkStackPage *page =
      g_object_get_data(G_OBJECT(button), "pm-stack-page");
  g_autoptr(GtkWidget) child = NULL;

  if (page == NULL)
    return;

  g_object_get(page, "child", &child, NULL);
  if (child != NULL)
    gtk_stack_set_visible_child(self->stack, GTK_WIDGET(child));
  pm_stack_sidebar_update_buttons(self);
}

static gboolean pm_stack_sidebar_page_is_bottom(PmStackSidebar *self,
                                                const gchar *page_name,
                                                gchar **bottom_names) {
  if (page_name == NULL)
    return FALSE;

  for (guint i = 0; bottom_names[i] != NULL; i++) {
    if (g_str_equal(g_strstrip(bottom_names[i]), page_name))
      return TRUE;
  }

  return FALSE;
}

static void pm_stack_sidebar_rebuild(PmStackSidebar *self);

static void pm_stack_sidebar_page_name_changed(GtkStackPage *page,
                                               GParamSpec *pspec,
                                               PmStackSidebar *self) {
  pm_stack_sidebar_rebuild(self);
}

static void pm_stack_sidebar_rebuild(PmStackSidebar *self) {
  GtkBox *groups[] = {self->top_items, self->bottom_items};
  GtkWidget *child;
  g_auto(GStrv) bottom_names = NULL;
  guint n_pages;

  if (self->top_items == NULL || self->bottom_items == NULL)
    return;

  for (guint i = 0; i < G_N_ELEMENTS(groups); i++) {
    while ((child = gtk_widget_get_first_child(GTK_WIDGET(groups[i]))) != NULL)
      gtk_box_remove(groups[i], child);
  }

  if (self->pages == NULL)
    return;

  bottom_names = g_strsplit(self->bottom_pages != NULL ? self->bottom_pages : "",
                            ",", -1);
  n_pages = g_list_model_get_n_items(G_LIST_MODEL(self->pages));
  for (guint i = 0; i < n_pages; i++) {
    g_autoptr(GtkStackPage) page =
        g_list_model_get_item(G_LIST_MODEL(self->pages), i);
    g_autofree gchar *page_name = NULL;
    GtkBox *group;
    GtkToggleButton *button;
    GtkWidget *content;
    GtkWidget *icon;
    GtkWidget *text;

    g_object_get(page, "name", &page_name, NULL);
    group = pm_stack_sidebar_page_is_bottom(self, page_name, bottom_names)
                ? self->bottom_items
                : self->top_items;
    button = GTK_TOGGLE_BUTTON(gtk_toggle_button_new());
    gtk_button_set_has_frame(GTK_BUTTON(button), FALSE);
    gtk_widget_set_hexpand(GTK_WIDGET(button), TRUE);
    gtk_widget_add_css_class(GTK_WIDGET(button), "page-toggle");
    content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    icon = gtk_image_new();
    gtk_image_set_pixel_size(GTK_IMAGE(icon), self->icon_size);
    text = gtk_label_new(NULL);
    gtk_box_append(GTK_BOX(content), icon);
    gtk_box_append(GTK_BOX(content), text);
    gtk_button_set_child(GTK_BUTTON(button), content);
    g_object_set_data_full(G_OBJECT(button), "pm-stack-page",
                           g_object_ref(page), g_object_unref);
    pm_stack_sidebar_update_page(page, NULL, button);
    g_signal_connect_object(page, "notify::title",
                            G_CALLBACK(pm_stack_sidebar_update_page), button, 0);
    g_signal_connect_object(page, "notify::icon-name",
                            G_CALLBACK(pm_stack_sidebar_update_page), button, 0);
    g_signal_connect_object(page, "notify::visible",
                            G_CALLBACK(pm_stack_sidebar_update_page), button, 0);
    g_signal_connect_object(page, "notify::name",
                            G_CALLBACK(pm_stack_sidebar_page_name_changed), self,
                            0);
    g_signal_connect_object(button, "clicked",
                            G_CALLBACK(pm_stack_sidebar_page_button_clicked),
                            self, 0);
    gtk_box_append(group, GTK_WIDGET(button));
  }

  pm_stack_sidebar_update_buttons(self);
}

static void pm_stack_sidebar_items_changed(GListModel *pages, guint position,
                                           guint removed, guint added,
                                           PmStackSidebar *self) {
  pm_stack_sidebar_rebuild(self);
}

static void pm_stack_sidebar_visible_child_changed(GtkStack *stack,
                                                   GParamSpec *pspec,
                                                   PmStackSidebar *self) {
  pm_stack_sidebar_update_buttons(self);
}

static void pm_stack_sidebar_show_text_toggled(GtkToggleButton *toggle,
                                              PmStackSidebar *self) {
  const gboolean show_text = gtk_toggle_button_get_active(toggle);

  gtk_widget_set_tooltip_text(GTK_WIDGET(toggle),
                              show_text ? "Hide page labels" : "Show page labels");
  pm_stack_sidebar_update_buttons(self);
}

static void pm_stack_sidebar_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {
  PmStackSidebar *self = PM_STACK_SIDEBAR(object);

  switch (property_id) {
  case PROP_BOTTOM_PAGES:
    g_free(self->bottom_pages);
    self->bottom_pages = g_value_dup_string(value);
    pm_stack_sidebar_rebuild(self);
    break;
  case PROP_ICON_SIZE:
    self->icon_size = g_value_get_int(value);
    pm_stack_sidebar_update_icon_size(self);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pm_stack_sidebar_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {
  PmStackSidebar *self = PM_STACK_SIDEBAR(object);

  switch (property_id) {
  case PROP_BOTTOM_PAGES:
    g_value_set_string(value, self->bottom_pages);
    break;
  case PROP_ICON_SIZE:
    g_value_set_int(value, self->icon_size);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pm_stack_sidebar_finalize(GObject *object) {
  PmStackSidebar *self = PM_STACK_SIDEBAR(object);

  g_free(self->bottom_pages);

  G_OBJECT_CLASS(pm_stack_sidebar_parent_class)->finalize(object);
}

static void pm_stack_sidebar_dispose(GObject *object) {
  PmStackSidebar *self = PM_STACK_SIDEBAR(object);

  if (self->pages != NULL) {
    g_signal_handlers_disconnect_by_data(self->pages, self);
    g_clear_object(&self->pages);
  }
  if (self->stack != NULL)
    g_signal_handlers_disconnect_by_data(self->stack, self);
  g_clear_object(&self->stack);

  G_OBJECT_CLASS(pm_stack_sidebar_parent_class)->dispose(object);
}

static void pm_stack_sidebar_class_init(PmStackSidebarClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  object_class->set_property = pm_stack_sidebar_set_property;
  object_class->get_property = pm_stack_sidebar_get_property;
  object_class->finalize = pm_stack_sidebar_finalize;
  object_class->dispose = pm_stack_sidebar_dispose;

  properties[PROP_BOTTOM_PAGES] =
      g_param_spec_string("bottom-pages", "Bottom pages",
                          "Comma-separated stack page names pinned to the bottom",
                          NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties[PROP_ICON_SIZE] =
      g_param_spec_int("icon-size", "Icon size", "Size of page icons", 0,
                       G_MAXINT, 16,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties(object_class, N_PROPERTIES, properties);

  gtk_widget_class_set_template_from_resource(
      widget_class,
      "/com/github/kenayake/ProcessManager/components/stack-sidebar.ui");
  gtk_widget_class_bind_template_child(widget_class, PmStackSidebar,
                                       show_text_toggle);
  gtk_widget_class_bind_template_child(widget_class, PmStackSidebar, top_items);
  gtk_widget_class_bind_template_child(widget_class, PmStackSidebar,
                                       bottom_items);
}

static void pm_stack_sidebar_init(PmStackSidebar *self) {
  self->icon_size = 24;

  gtk_widget_init_template(GTK_WIDGET(self));
  g_signal_connect(self->show_text_toggle, "toggled",
                   G_CALLBACK(pm_stack_sidebar_show_text_toggled), self);
}

void pm_stack_sidebar_set_stack(PmStackSidebar *self, GtkStack *stack) {
  g_return_if_fail(PM_IS_STACK_SIDEBAR(self));
  g_return_if_fail(stack == NULL || GTK_IS_STACK(stack));

  if (self->stack == stack)
    return;

  if (self->pages != NULL) {
    g_signal_handlers_disconnect_by_data(self->pages, self);
    g_clear_object(&self->pages);
  }
  if (self->stack != NULL)
    g_signal_handlers_disconnect_by_data(self->stack, self);
  g_set_object(&self->stack, stack);

  if (self->stack == NULL) {
    pm_stack_sidebar_rebuild(self);
    return;
  }

  self->pages = g_object_ref(gtk_stack_get_pages(self->stack));
  g_signal_connect(self->pages, "items-changed",
                   G_CALLBACK(pm_stack_sidebar_items_changed), self);
  g_signal_connect(self->stack, "notify::visible-child",
                   G_CALLBACK(pm_stack_sidebar_visible_child_changed), self);
  pm_stack_sidebar_rebuild(self);
}

void pm_stack_sidebar_set_bottom_pages(PmStackSidebar *self,
                                       const gchar *bottom_pages) {
  g_return_if_fail(PM_IS_STACK_SIDEBAR(self));

  g_object_set(self, "bottom-pages", bottom_pages, NULL);
}
