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

#define _GNU_SOURCE

#include <libudev.h>

#include "manager.h"
#include "util.h"

/*
 * Sanity helpers for dealing with udev.
 */
typedef struct udev udev_connection;
typedef struct udev_hwdb udev_hwdb;
typedef struct udev_device udev_device;
typedef struct udev_enumerate udev_enum;
typedef struct udev_list_entry udev_list;

DEF_AUTOFREE(udev_device, udev_device_unref)
DEF_AUTOFREE(udev_enum, udev_enumerate_unref)

static void ldm_manager_init_udev(LdmManager *self);
static void ldm_manager_push_sysfs(LdmManager *self, const char *sysfs_path);

struct _LdmManagerClass {
        GObjectClass parent_class;
};

/*
 * LdmManager
 *
 * The manager is the main library portion of libldm and is used to introspect
 * devices on the system.
 */
struct _LdmManager {
        GObject parent;
        GHashTable *devices;

        /* Udev */
        udev_connection *udev;
        udev_hwdb *hwdb;
};

G_DEFINE_TYPE(LdmManager, ldm_manager, G_TYPE_OBJECT)

/**
 * ldm_manager_dispose:
 *
 * Clean up a LdmManager instance
 */
static void ldm_manager_dispose(GObject *obj)
{
        LdmManager *self = LDM_MANAGER(obj);

        g_clear_pointer(&self->hwdb, udev_hwdb_unref);
        g_clear_pointer(&self->udev, udev_unref);

        /* clean ourselves up */
        g_clear_pointer(&self->devices, g_hash_table_unref);

        G_OBJECT_CLASS(ldm_manager_parent_class)->dispose(obj);
}

/**
 * ldm_manager_class_init:
 *
 * Handle class initialisation
 */
static void ldm_manager_class_init(LdmManagerClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_manager_dispose;
}

/**
 * ldm_manager_init:
 *
 * Handle construction of the LdmManager
 */
static void ldm_manager_init(LdmManager *self)
{
        /* Device table is a mapping of sysfs name to LdmDevice */
        self->devices = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);

        /* Get udev going */
        self->udev = udev_new();
        g_assert(self->udev != NULL);
        self->hwdb = udev_hwdb_new(self->udev);
        g_assert(self->hwdb != NULL);

        ldm_manager_init_udev(self);
}

/**
 * ldm_manager_init_udev:
 *
 * Set up udev and get some devices going.
 */
static void ldm_manager_init_udev(LdmManager *self)
{
        autofree(udev_enum) *ue = NULL;
        udev_list *list = NULL, *entry = NULL;
        static const char *subsystems[] = {
                "usb",
                "pci",
        };

        /* Set up the enumerator */
        ue = udev_enumerate_new(self->udev);
        g_assert(ue != NULL);

        for (size_t i = 0; i < G_N_ELEMENTS(subsystems); i++) {
                const char *sub = subsystems[i];
                if (udev_enumerate_add_match_subsystem(ue, sub) != 0) {
                        g_warning("Failed to add subsystem match: %s", sub);
                }
        }

        /* Request we can */
        if (udev_enumerate_scan_devices(ue) != 0) {
                g_warning("Failed to enumerate devices");
                return;
        }

        /* Grab head */
        list = udev_enumerate_get_list_entry(ue);

        /* Walk said list */
        udev_list_entry_foreach(entry, list)
        {
                ldm_manager_push_sysfs(self, udev_list_entry_get_name(entry));
        }
}

/**
 * ldm_manager_push_sysfs:
 * @sysfs_path: Path within the sysfs for the new device
 *
 * Attempt to add a new udev device to our state from a sysfs path
 */
static void ldm_manager_push_sysfs(LdmManager *self, const char *sysfs_path)
{
        const char *modalias = NULL;
        autofree(udev_device) *device = NULL;

        /* We only want stuff with a modalias. */
        device = udev_device_new_from_syspath(self->udev, sysfs_path);
        modalias = udev_device_get_sysattr_value(device, "modalias");
        if (!modalias) {
                return;
        }

        g_message("Device: %s", sysfs_path);
}

/**
 * ldm_manager_new:
 *
 * Construct a new LdmManager
 *
 * Returns: (transfer full): A newly created #LdmManager
 */
LdmManager *ldm_manager_new()
{
        return g_object_new(LDM_TYPE_MANAGER, NULL);
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
