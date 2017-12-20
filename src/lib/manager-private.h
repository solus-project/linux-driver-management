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

#pragma once

#include <glib-object.h>

#include "device.h"
#include "ldm-private.h"
#include "manager.h"

struct _LdmManagerClass {
        GObjectClass parent_class;

        /* Signals */
        void (*device_added)(LdmManager *self, LdmDevice *device);
        void (*device_removed)(LdmManager *self, const gchar *id);
};

struct _LdmManager {
        GObject parent;
        GPtrArray *devices;
        GHashTable *plugins;

        /* Udev */
        udev_connection *udev;

        LdmManagerFlags flags;

        struct {
                udev_monitor *udev;  /* Connection to udev.. */
                GIOChannel *channel; /* Main channel for poll main loop */
                guint source;        /* GIO source */
        } monitor;
};

/* Plugin API */
void ldm_manager_add_modalias_plugin_for_path(LdmManager *self, const gchar *path);

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
