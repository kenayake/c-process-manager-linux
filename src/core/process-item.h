#pragma once

#include "glib.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PROCESS_TYPE_ITEM (process_item_get_type())

G_DECLARE_FINAL_TYPE(ProcessItem, process_item, PROCESS, ITEM, GObject)

ProcessItem *process_item_new(const gchar *name, const gchar *status,
                                     const gdouble cpu, const gdouble memory,
                                     const gdouble disk, const gchar *user,
                                     const gchar *priority);

G_END_DECLS