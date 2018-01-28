/*
 * This file is part of linux-driver-management.
 *
 * Copyright © 2016-2018 Linux Driver Management Developers, Solus Project
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
typedef struct udev_monitor udev_monitor;

struct _LdmDeviceClass {
        GInitiallyUnownedClass parent_class;
};

/*
 * LdmDevice
 *
 * An LdmDevice is an opaque representation of an available device on
 * the system, and provides introspection opportunities to discover
 * capabilities, drivers, etc.
 */
struct _LdmDevice {
        GInitiallyUnowned parent;

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
                gint product_id;
                gint vendor_id;
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

void ldm_dmi_device_init_private(LdmDevice *self, udev_device *device);
void ldm_pci_device_init_private(LdmDevice *self, udev_device *device);
void ldm_usb_device_init_private(LdmDevice *self, udev_device *device);
void ldm_bluetooth_device_init_private(LdmDevice *self, udev_device *device);

/* private child APIs */
void ldm_device_add_child(LdmDevice *device, LdmDevice *child);
void ldm_device_remove_child(LdmDevice *device, LdmDevice *child);
void ldm_device_remove_child_by_path(LdmDevice *device, const gchar *path);
LdmDevice *ldm_device_get_child_by_path(LdmDevice *device, const gchar *path);

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
