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

static bool ldm_monitor_init(LdmMonitor *self);

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

        return true;
}

void ldm_monitor_free(LdmMonitor *self)
{
        g_clear_object(&self->udev_client);
        free(self);
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
