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

#include <gudev/gudev.h>
#include <stdbool.h>
#include <stdlib.h>

#include "monitor.h"
#include "util.h"

static bool ldm_monitor_init(LdmMonitor *self);
static void ldm_monitor_uevent(LdmMonitor *self, const gchar *action, GUdevDevice *device,
                               GUdevClient *client);
static void ldm_monitor_udev_add(LdmMonitor *self, GUdevDevice *device);
static void ldm_monitor_udev_remove(LdmMonitor *self, GUdevDevice *device);

struct LdmMonitor {
        GUdevClient *udev_client; /**<Connection to udev */
};

LdmMonitor *ldm_monitor_new(void)
{
        LdmMonitor *ret = NULL;

        ret = calloc(1, sizeof(LdmMonitor));
        if (!ret) {
                return NULL;
        }

        if (!ldm_monitor_init(ret)) {
                ldm_monitor_free(ret);
                return NULL;
        }

        return ret;
}

/**
 * Attempt to initialise the udev client and systems
 */
static bool ldm_monitor_init(LdmMonitor *self)
{
        /* The subsystems we care about */
        static const gchar *subsystems[] = {
                "usb/usb_interface",
                NULL,
        };

        self->udev_client = g_udev_client_new(subsystems);
        if (!self->udev_client) {
                return false;
        }

        g_signal_connect_swapped(self->udev_client, "uevent", G_CALLBACK(ldm_monitor_uevent), self);

        return true;
}

void ldm_monitor_free(LdmMonitor *self)
{
        g_clear_object(&self->udev_client);
        free(self);
}

/**
 * Begin processing a uevent - udev has some hardware change for us.
 */
static void ldm_monitor_uevent(LdmMonitor *self, const gchar *action, GUdevDevice *device,
                               __ldm_unused__ GUdevClient *client)
{
        if (g_str_equal(action, "add")) {
                ldm_monitor_udev_add(self, device);
        } else if (g_str_equal(action, "remove")) {
                ldm_monitor_udev_remove(self, device);
        }
}

static void ldm_monitor_udev_add(LdmMonitor *self, GUdevDevice *device)
{
        g_message("uevent(add): %s", g_udev_device_get_sysfs_path(device));
}

static void ldm_monitor_udev_remove(LdmMonitor *self, GUdevDevice *device)
{
        g_message("uevent(remove): %s", g_udev_device_get_sysfs_path(device));
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
