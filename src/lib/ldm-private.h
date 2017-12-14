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
#include <libudev.h>

#include "device.h"
#include "util.h"

/*
 * Sanity helpers for dealing with udev.
 */
typedef struct udev udev_connection;
typedef struct udev_device udev_device;
typedef struct udev_enumerate udev_enum;
typedef struct udev_list_entry udev_list;

struct _LdmDeviceClass {
        GObjectClass parent_class;
};

/*
 * LdmDevice
 *
 * An LdmDevice is an opaque representation of an available device on
 * the system, and provides introspection opportunities to discover
 * capabilities, drivers, etc.
 */
struct _LdmDevice {
        GObject parent;

        struct {
                LdmDevice *parent;
                GHashTable *kids;
        } tree;

        /* OS Data */
        struct {
                gchar *sysfs_path;
                gchar *modalias;
                GHashTable *hwdb_info;
                guint devtype;
                guint attributes;
        } os;

        /* Identification */
        struct {
                gchar *name;
                gchar *vendor;
        } id;
};

/*
 * Common autofree helpers.
 */
DEF_AUTOFREE(udev_device, udev_device_unref)
DEF_AUTOFREE(udev_enum, udev_enumerate_unref)
DEF_AUTOFREE(gchar, g_free)

/* Private device API */
LdmDevice *ldm_device_new_from_udev(LdmDevice *parent, udev_device *device, udev_list *properties);

void ldm_device_init_pci(LdmDevice *self, udev_device *device);
void ldm_usb_device_init_private(LdmDevice *self, udev_device *device);

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
