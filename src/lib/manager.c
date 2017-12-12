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
        struct udev *udev;
        struct udev_hwdb *hwdb;
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
