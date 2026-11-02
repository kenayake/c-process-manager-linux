#include <gtk/gtk.h>

void show_alert(GtkWindow *window, const gchar *title, GError *error) {
  g_autoptr(GtkAlertDialog) alert;

  alert = gtk_alert_dialog_new("%s", title);
  gtk_alert_dialog_set_detail(alert, error->message);
  gtk_alert_dialog_set_modal(alert, TRUE);
  gtk_alert_dialog_show(alert, window);
}