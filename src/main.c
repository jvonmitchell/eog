/* Eye Of Gnome - Main 
 *
 * Copyright (C) 2000-2006 The Free Software Foundation
 *
 * Author: Lucas Rocha <lucasr@gnome.org>
 *
 * Based on code by:
 * 	- Federico Mena-Quintero <federico@gnu.org>
 *	- Jens Finke <jens@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_DBUS
#include <dbus/dbus-glib-bindings.h>
#include <gdk/gdkx.h>
#endif

#include "eog-session.h"
#include "eog-debug.h"
#include "eog-thumbnail.h"
#include "eog-job-queue.h"
#include "eog-application.h"
#include "eog-util.h"

#include <string.h>
#include <stdlib.h>
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <libgnomeui/gnome-ui-init.h>
#include <libgnomeui/gnome-client.h>
#include <libgnomeui/gnome-app-helper.h>
#include <libgnomeui/gnome-authentication-manager.h>

static EogStartupFlags flags;

static gboolean fullscreen = FALSE;
static gboolean slide_show = FALSE;
static gboolean disable_collection = FALSE;
static gchar **startup_files = NULL;

static const GOptionEntry goption_options[] =
{
	{ "fullscreen", 'f', 0, G_OPTION_ARG_NONE, &fullscreen, N_("Open in fullscreen mode"), NULL  },
	{ "disable-image-collection", 'c', 0, G_OPTION_ARG_NONE, &disable_collection, N_("Disable image collection"), NULL  },
	{ "slide-show", 's', 0, G_OPTION_ARG_NONE, &slide_show, N_("Open in slide show mode"), NULL  },
	{ G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &startup_files, NULL, N_("[FILE...]") },
	{ NULL }
};

static void 
set_startup_flags ()
{
  if (fullscreen)
    flags |= EOG_STARTUP_FULLSCREEN;

  if (disable_collection)
    flags |= EOG_STARTUP_DISABLE_COLLECTION;

  if (slide_show)
    flags |= EOG_STARTUP_SLIDE_SHOW;
}

static void 
load_files ()
{
	GSList *files = NULL;

	files = eog_util_string_array_to_list ((const gchar **) startup_files, TRUE);

	eog_application_open_uri_list (EOG_APP, 
				       files,
				       GDK_CURRENT_TIME,
				       flags,
				       NULL);

	g_slist_foreach (files, (GFunc) g_free, NULL);	
	g_slist_free (files);
}

#ifdef HAVE_DBUS
static gboolean
load_files_remote ()
{
	GError *error = NULL;
	DBusGConnection *connection;
	DBusGProxy *remote_object;
	gboolean result = TRUE;
	GdkDisplay *display;
	guint32 timestamp;
	gchar **files;
	
	display = gdk_display_get_default ();

	timestamp = gdk_x11_display_get_user_time (display);
	connection = dbus_g_bus_get (DBUS_BUS_STARTER, &error);

	if (connection == NULL) {
		g_warning (error->message);
		g_error_free (error);	

 		return FALSE;
 	}
 
 	files = eog_util_string_array_make_absolute (startup_files);
 	
 	remote_object = dbus_g_proxy_new_for_name (connection,
 						   "org.gnome.eog.ApplicationService",
						   "/org/gnome/eog/Eog",
						   "org.gnome.eog.Application");
 
 	if (!files) {
 		if (!dbus_g_proxy_call (remote_object, "OpenWindow", &error,
 					G_TYPE_UINT, timestamp,
 					G_TYPE_UCHAR, flags,
 					G_TYPE_INVALID,
 					G_TYPE_INVALID)) {
 			g_warning (error->message);
 			g_clear_error (&error);
 
 			result = FALSE;
 		}
 	} else {
 		if (!dbus_g_proxy_call (remote_object, "OpenUris", &error,
 					G_TYPE_STRV, files,
 					G_TYPE_UINT, timestamp,
 					G_TYPE_UCHAR, flags,
 					G_TYPE_INVALID,
 					G_TYPE_INVALID)) {
 			g_warning (error->message);
 			g_clear_error (&error);
 			
			result = FALSE;
 		}
		
 		g_strfreev (files);
 	}
 
 	g_object_unref (remote_object);
 	dbus_g_connection_unref (connection);
 
 	gdk_notify_startup_complete ();
 
 	return result;
}
#endif /* HAVE_DBUS */

int
main (int argc, char **argv)
{
	GnomeProgram *program;
	GOptionContext *ctx;

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	textdomain (PACKAGE);

	ctx = g_option_context_new (NULL);
	g_option_context_add_main_entries (ctx, goption_options, PACKAGE);

	program = gnome_program_init ("eog", VERSION,
				      LIBGNOMEUI_MODULE, argc, argv,
				      GNOME_PARAM_GOPTION_CONTEXT, ctx,
				      GNOME_PARAM_HUMAN_READABLE_NAME, _("Eye of GNOME"),
				      GNOME_PARAM_APP_DATADIR, EOG_DATADIR,
				      NULL);
	
	set_startup_flags ();
	
#ifdef HAVE_DBUS
	if (!eog_application_register_service (EOG_APP)) {
		if (load_files_remote ()) {
			g_object_unref (program);
			return 0;
		}
	}
#endif /* HAVE_DBUS */
	
	gnome_authentication_manager_init ();

	eog_debug_init ();
	eog_job_queue_init ();
	gdk_threads_init ();
	eog_thumbnail_init ();

	gtk_window_set_default_icon_name ("eog");
	g_set_application_name (_("Eye of GNOME Image Viewer"));

	load_files ();

	gtk_main ();

	gnome_accelerators_sync ();
	
  	if (startup_files)
		g_strfreev (startup_files);

	g_object_unref (program);

	return 0;
}