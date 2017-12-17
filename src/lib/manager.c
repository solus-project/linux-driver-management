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

#include "device.h"
#include "ldm-private.h"
#include "manager.h"
#include "util.h"

static void ldm_manager_init_udev(LdmManager *self);
static void ldm_manager_init_udev_monitor(LdmManager *self);
static void ldm_manager_push_sysfs(LdmManager *self, const char *sysfs_path);
static void ldm_manager_push_device(LdmManager *self, udev_device *device);
static gboolean ldm_manager_io_ready(GIOChannel *source, GIOCondition condition, gpointer v);

struct _LdmManagerClass {
        GObjectClass parent_class;
};

/**
 * SECTION:manager
 * @Short_description: Device Manager
 * @Title: LdmManager
 *
 * The manager object is used to introspect the system and discover various
 * devices. Using the #LdmManager APIs one can enumerate specific device
 * classes.
 *
 * LdmManager internally makes use of libudev to provide low level enumeration
 * and hotplug event capabilities, but will convert those raw devices and
 * interfaces into the more readily consumable #LdmDevice type.
 *
 * Using the manager is very simple, and in a few lines you can grab all
 * the devices from the system for introspection.
 *
 * C example:
 *
 * |[<!-- language="C" -->
 *      LdmManager *manager = ldm_manager_new();
 *      GList *devices = ldm_manager_get_devices(LDM_DEVICE_TYPE_ANY);
 *      LdmDevice *device = g_list_nth_data(devices, 0);
 *      g_list_free(devices);
 * ]|
 *
 * Vala example:
 *
 * |[<!-- language="Vala" -->
 *      var manager = new Ldm.Manager();
 *      var devices = manager.get_devices();
 * ]|
 */
struct _LdmManager {
        GObject parent;
        GHashTable *devices;

        /* Udev */
        udev_connection *udev;

        struct {
                udev_monitor *udev;  /* Connection to udev.. */
                GIOChannel *channel; /* Main channel for poll main loop */
                guint source;        /* GIO source */
        } monitor;
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

        /* Clear up our source */
        if (self->monitor.source > 0) {
                g_source_remove(self->monitor.source);
                self->monitor.source = 0;
        }

        /* Clear out the monitor */
        if (self->monitor.udev) {
                g_io_channel_shutdown(self->monitor.channel, FALSE, NULL);
                g_clear_pointer(&self->monitor.channel, g_io_channel_unref);
                g_clear_pointer(&self->monitor.udev, udev_monitor_unref);
        }

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

        ldm_manager_init_udev(self);
        ldm_manager_init_udev_monitor(self);
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

        /* Scan the devices. Due to umockdev we won't check this return. */
        udev_enumerate_scan_devices(ue);

        /* Grab head */
        list = udev_enumerate_get_list_entry(ue);

        /* Walk said list */
        udev_list_entry_foreach(entry, list)
        {
                ldm_manager_push_sysfs(self, udev_list_entry_get_name(entry));
        }
}

/**
 * ldm_manager_init_udev_monitor:
 *
 * Set up the udev monitor and attach the file descriptor to the main event
 * context to enable receiving the events on the idle loop.
 */
static void ldm_manager_init_udev_monitor(LdmManager *self)
{
        int fd = 0;

        self->monitor.udev = udev_monitor_new_from_netlink(self->udev, "udev");
        if (!self->monitor.udev) {
                g_warning("udev monitoring is unavailable");
                return;
        }

        /* We only want hotplug events for USB right now */
        if (udev_monitor_filter_add_match_subsystem_devtype(self->monitor.udev, "usb", NULL) != 0) {
                g_warning("Unable to install USB filter");
                g_clear_pointer(&self->monitor.udev, udev_monitor_unref);
                return;
        }

        if (udev_monitor_enable_receiving(self->monitor.udev) != 0) {
                g_warning("Failed to enable monitor receiving");
                g_clear_pointer(&self->monitor.udev, udev_monitor_unref);
                return;
        }

        /* Now let's hook up monitoring. */
        fd = udev_monitor_get_fd(self->monitor.udev);
        self->monitor.channel = g_io_channel_unix_new(fd);
        /* Don't do anything fancy with the channel */
        g_io_channel_set_encoding(self->monitor.channel, NULL, NULL);
        self->monitor.source =
            g_io_add_watch(self->monitor.channel, G_IO_IN, ldm_manager_io_ready, self);

        g_assert(self->monitor.udev != NULL);
}

/**
 * ldm_manager_io_ready:
 *
 * We have I/O on the udev channel, do something with it.
 */
static gboolean ldm_manager_io_ready(__ldm_unused__ GIOChannel *source, GIOCondition condition,
                                     gpointer v)
{
        LdmManager *self = v;
        autofree(udev_device) *device = NULL;
        const char *action = NULL;

        /* Only want G_IO_IN here. */
        if ((condition & G_IO_IN) != G_IO_IN) {
                return TRUE;
        }

        device = udev_monitor_receive_device(self->monitor.udev);
        if (!device) {
                /* Remove polling now, something is badly wrong. */
                g_warning("Failed to receive device!");
                return FALSE;
        }

        action = udev_device_get_action(device);
        if (!action) {
                return TRUE;
        }

        /* Interesting actions */
        if (g_str_equal(action, "add")) {
                g_message("Device added: %s", udev_device_get_syspath(device));
        } else if (g_str_equal(action, "remove")) {
                g_message("Device removed: %s", udev_device_get_syspath(device));
        }

        /* Keep the source around */
        return TRUE;
}

/**
 * ldm_manager_push_sysfs:
 * @sysfs_path: Path within the sysfs for the new device
 *
 * Potentially add a device from the sysfs name if it happens to have a modalias
 */
static void ldm_manager_push_sysfs(LdmManager *self, const char *sysfs_path)
{
        autofree(udev_device) *device = NULL;

        device = udev_device_new_from_syspath(self->udev, sysfs_path);

        ldm_manager_push_device(self, device);
}

/**
 * ldm_manager_get_device_parent:
 *
 * Get the associated LdmDevice that is the parent candidate for a newly
 * added device or interface.
 */
LdmDevice *ldm_manager_get_device_parent(LdmManager *self, const char *subsystem,
                                         udev_device *device)
{
        udev_device *udev_parent = NULL;
        LdmDevice *parent = NULL;
        const char *sysfs_path = NULL;
        const char *devtype = NULL;

        /* Must be a USB interface */
        if (!g_str_equal(subsystem, "usb")) {
                return NULL;
        }
        devtype = udev_device_get_devtype(device);
        if (!devtype || !g_str_equal(devtype, "usb_interface")) {
                return NULL;
        }

        udev_parent = udev_device_get_parent_with_subsystem_devtype(device, "usb", "usb_device");
        if (!udev_parent) {
                return NULL;
        }

        sysfs_path = udev_device_get_syspath(udev_parent);
        parent = g_hash_table_lookup(self->devices, sysfs_path);
        if (!parent) {
                return NULL;
        }

        return parent;
}

/**
 * ldm_manager_push_device:
 * @device: The udev device to add
 *
 * This will handle the real work of adding a new device to the manager
 */
static void ldm_manager_push_device(LdmManager *self, udev_device *device)
{
        LdmDevice *ldm_device = NULL;
        LdmDevice *parent = NULL;
        const char *sysfs_path = NULL;
        const char *subsystem = NULL;
        udev_list *properties = NULL;

        sysfs_path = udev_device_get_syspath(device);

        /* Don't dupe these guys. */
        if (g_hash_table_contains(self->devices, sysfs_path)) {
                return;
        }

        /* Get our basic information */
        subsystem = udev_device_get_subsystem(device);
        properties = udev_device_get_properties_list_entry(device);

        parent = ldm_manager_get_device_parent(self, subsystem, device);

        /* Build the actual device now */
        ldm_device = ldm_device_new_from_udev(parent, device, properties);
        if (parent) {
                ldm_device_add_child(parent, ldm_device);
                /* TODO: Emit signal on change! */
                return;
        }

        g_message("ldm_manager_push_device(%s): %s", subsystem, ldm_device_get_name(ldm_device));

        /* TODO: Emit signal for the new device. */
        g_hash_table_insert(self->devices, g_strdup(sysfs_path), ldm_device);
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

/**
 * ldm_manager_get_devices:
 * @class_mask: Bitwise mask of LdmDeviceType
 *
 * Return a subset of the devices known to this manager that happen to
 * match the given classmask. As an example you might call this function
 * with #LDM_DEVICE_TYPE_GPU|#LDM_DEVICE_TYPE_PCI to find all PCI GPUs on
 * the system.
 *
 * Returns: (element-type Ldm.Device) (transfer container): a list of all currently known devices
 */
GList *ldm_manager_get_devices(LdmManager *self, LdmDeviceType class_mask)
{
        GHashTableIter iter = { 0 };
        __ldm_unused__ gpointer key = NULL;
        LdmDevice *dev = NULL;
        GList *ret = NULL;

        g_return_val_if_fail(self != NULL, NULL);

        g_hash_table_iter_init(&iter, self->devices);
        while (g_hash_table_iter_next(&iter, &key, (void **)&dev)) {
                if (!ldm_device_has_type(dev, class_mask)) {
                        continue;
                }
                ret = g_list_append(ret, dev);
        }

        return ret;
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
