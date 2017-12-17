/*
 * This file is part of linux-driver-management.
 *
 * Copyright © 2016-2017 Ikey Doherty
 *
 * linux-driver-management is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#include <gio/gio.h>
#include <glib-unix.h>
#include <stdbool.h>

#include "monitor.h"
#include "util.h"

#define LDM_APP_ID "com.solus-project.linux-driver-management.Monitor"

static bool have_shutdown = false;
static LdmDaemon *monitor = NULL;

/**
 * We're called on the main context *after* a SIGINT so we don't need
 * a volatile var. Just ask the application to release so that we can
 * begin the shutdown process cleanly.
 */
static gboolean ldm_shutdown_trigger(GApplication *app)
{
        if (have_shutdown) {
                return FALSE;
        }
        have_shutdown = true;
        g_message("Initiating shutdown");
        g_application_release(app);
        return FALSE;
}

/**
 * Handle our initial application startup and hook up signals so that
 * we can safely shut down again using the main context.
 */
static void ldm_app_startup(GApplication *app, __ldm_unused__ gpointer v)
{
        g_message("LDM Monitor running");
        monitor = ldm_daemon_new();

        /* Safely add shutdown callback. */
        g_unix_signal_add(SIGINT, (GSourceFunc)ldm_shutdown_trigger, app);

        /* We actually want to run as a background service */
        g_application_hold(app);
}

/**
 * The application was shutdown after g_application_release,
 * so clean up any existing state.
 */
static void ldm_app_shutdown(__ldm_unused__ GApplication *app, __ldm_unused__ gpointer v)
{
        g_message("Shutting down LDM Monitor");
        g_clear_object(&monitor);
}

int main(int argc, char **argv)
{
        g_autoptr(GApplication) app = NULL;

        app = g_application_new(LDM_APP_ID, G_APPLICATION_IS_SERVICE);
        g_signal_connect(app, "startup", G_CALLBACK(ldm_app_startup), NULL);
        g_signal_connect(app, "shutdown", G_CALLBACK(ldm_app_shutdown), NULL);

        /* Run the app */
        return g_application_run(app, argc, argv);
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 expandtab:
 * :indentSize=8:tabSize=8:noTabs=true:
 */
