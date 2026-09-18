/* process-manager-application.c
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
#include <glib/gi18n.h>

#include "process-manager-application.h"
#include "process-manager-window.h"

struct _ProcessManagerApplication
{
	GtkApplication parent_instance;
};

G_DEFINE_FINAL_TYPE (ProcessManagerApplication, process_manager_application, GTK_TYPE_APPLICATION)

ProcessManagerApplication *
process_manager_application_new (const char        *application_id,
                                 GApplicationFlags  flags)
{
	g_return_val_if_fail (application_id != NULL, NULL);

	return g_object_new (PROCESS_MANAGER_TYPE_APPLICATION,
	                     "application-id", application_id,
	                     "flags", flags,
	                     "resource-base-path", "/com/github/kenayake/ProcessManager",
	                     NULL);
}

static void
process_manager_application_activate (GApplication *app)
{
	GtkWindow *window;

	g_assert (PROCESS_MANAGER_IS_APPLICATION (app));

	window = gtk_application_get_active_window (GTK_APPLICATION (app));

	if (window == NULL)
		window = g_object_new (PROCESS_MANAGER_TYPE_WINDOW,
		                       "application", app,
		                       NULL);

	gtk_window_present (window);
}

static void
process_manager_application_class_init (ProcessManagerApplicationClass *klass)
{
	GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

	app_class->activate = process_manager_application_activate;
}

static void
process_manager_application_about_action (GSimpleAction *action,
                                          GVariant      *parameter,
                                          gpointer       user_data)
{
	static const char *authors[] = {"Unknown", NULL};
	ProcessManagerApplication *self = user_data;
	GtkWindow *window = NULL;

	g_assert (PROCESS_MANAGER_IS_APPLICATION (self));

	window = gtk_application_get_active_window (GTK_APPLICATION (self));

	gtk_show_about_dialog (window,
	                       "program-name", "Process-manager",
	                       "logo-icon-name", "com.github.kenayake.ProcessManager",
	                       "authors", authors,
	                       "translator-credits", _("translator-credits"),
	                       "version", "0.1.0",
	                       "copyright", "© 2026 Unknown",
	                       NULL);
}

static void
process_manager_application_quit_action (GSimpleAction *action,
                                         GVariant      *parameter,
                                         gpointer       user_data)
{
	ProcessManagerApplication *self = user_data;

	g_assert (PROCESS_MANAGER_IS_APPLICATION (self));

	g_application_quit (G_APPLICATION (self));
}

static const GActionEntry app_actions[] = {
	{ "quit", process_manager_application_quit_action },
	{ "about", process_manager_application_about_action },
};

static void
process_manager_application_init (ProcessManagerApplication *self)
{
	g_action_map_add_action_entries (G_ACTION_MAP (self),
	                                 app_actions,
	                                 G_N_ELEMENTS (app_actions),
	                                 self);
	gtk_application_set_accels_for_action (GTK_APPLICATION (self),
	                                       "app.quit",
	                                       (const char *[]) { "<control>q", NULL });
}
