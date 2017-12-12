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

#include "device.h"
#include "ldm-private.h"
#include "util.h"

#define HWDB_LOOKUP_PRODUCT_NAME "ID_MODEL_FROM_DATABASE"
#define HWDB_LOOKUP_PRODUCT_VENDOR "ID_VENDOR_FROM_DATABASE"

static void ldm_device_init_pci(LdmDevice *self, udev_device *device);

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

        /* OS Data */
        struct {
                gchar *sysfs_path;
                gchar *modalias;
                GHashTable *hwdb_info;
                guint devtype;
        } os;

        /* Identification */
        struct {
                gchar *name;
                gchar *vendor;
        } id;

        /* PCI data */
        struct {
                gboolean boot_vga;
        } pci;
};

G_DEFINE_TYPE(LdmDevice, ldm_device, G_TYPE_OBJECT)

enum { PROP_PATH = 1, PROP_MODALIAS, PROP_NAME, PROP_VENDOR, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

static void ldm_device_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec);

/**
 * ldm_device_dispose:
 *
 * Clean up a LdmDevice instance
 */
static void ldm_device_dispose(GObject *obj)
{
        LdmDevice *self = LDM_DEVICE(obj);

        g_clear_pointer(&self->os.hwdb_info, g_hash_table_unref);
        g_clear_pointer(&self->os.sysfs_path, g_free);
        g_clear_pointer(&self->os.modalias, g_free);
        g_clear_pointer(&self->id.name, g_free);
        g_clear_pointer(&self->id.vendor, g_free);

        G_OBJECT_CLASS(ldm_device_parent_class)->dispose(obj);
}

/**
 * ldm_device_class_init:
 *
 * Handle class initialisation
 */
static void ldm_device_class_init(LdmDeviceClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_device_dispose;
        obj_class->get_property = ldm_device_get_property;

        /**
         * LdmDevice::path
         *
         * The system path for this device. On Linux this is the sysfs path.
         */
        obj_properties[PROP_PATH] = g_param_spec_string("path",
                                                        "The device path",
                                                        "System path for this device",
                                                        NULL,
                                                        G_PARAM_READABLE);

        /**
         * LdmDevice::modalias
         *
         * The modalias reported by the kernel for this device.
         */
        obj_properties[PROP_MODALIAS] = g_param_spec_string("modalias",
                                                            "The device modalias",
                                                            "System modalias for this device",
                                                            NULL,
                                                            G_PARAM_READABLE);

        /**
         * LdmDevice::name
         *
         * The name used to display this device to users, i.e. the model.
         */
        obj_properties[PROP_NAME] = g_param_spec_string("name",
                                                        "The device name",
                                                        "Display name (model) for this device",
                                                        NULL,
                                                        G_PARAM_READABLE);

        /**
         * LdmDevice::vendor
         *
         * The vendor string to display the users, i.e. the manufacturer.
         */
        obj_properties[PROP_VENDOR] =
            g_param_spec_string("vendor",
                                "The device vendor",
                                "The vendor (manufacturer) for this device",
                                NULL,
                                G_PARAM_READABLE);

        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

static void ldm_device_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec)
{
        LdmDevice *self = LDM_DEVICE(object);

        switch (id) {
        case PROP_PATH:
                g_value_set_string(value, self->os.sysfs_path);
                break;
        case PROP_MODALIAS:
                g_value_set_string(value, self->os.modalias);
                break;
        case PROP_NAME:
                g_value_set_string(value, self->id.name);
                break;
        case PROP_VENDOR:
                g_value_set_string(value, self->id.vendor);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

/**
 * ldm_device_init:
 *
 * Handle construction of the LdmDevice
 */
static void ldm_device_init(LdmDevice *self)
{
        /* Just set up the table for our properties */
        self->os.hwdb_info = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
}

/**
 * ldm_device_get_modalias:
 *
 * The modalias is unique to the device and is used in identifying potential
 * driver candidates.
 *
 * Returns: (transfer none): The modalias of the device
 */
const gchar *ldm_device_get_modalias(LdmDevice *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return (const gchar *)self->os.modalias;
}

/**
 * ldm_device_get_name:
 *
 * This function will return the name (model) of this device, suitable
 * for presentation to a user.
 *
 * Returns: (transfer none): The name of the device
 */
const gchar *ldm_device_get_name(LdmDevice *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return (const gchar *)self->id.name;
}

/**
 * ldm_device_get_path:
 *
 * This function will return the system-specific path for this device.
 * On Linux this is the sysfs path.
 *
 * Returns: (transfer none): The path of the device
 */
const gchar *ldm_device_get_path(LdmDevice *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return (const gchar *)self->os.sysfs_path;
}

/**
 * ldm_device_get_vendor:
 *
 * This function will return the vendor (manufacturer) of this device,
 * suitable for presentation to a user.
 *
 * Returns: (transfer none): The vendor of the device
 */
const gchar *ldm_device_get_vendor(LdmDevice *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return (const gchar *)self->id.vendor;
}

/**
 * ldm_device_new_from_udev:
 * @device: Associated udev device
 * @hwinfo: If set, the hwdb entry for this device.
 *
 * Construct a new LdmDevice from the given udev device and hwdb information.
 * This is private API between the manager and the device.
 */
LdmDevice *ldm_device_new_from_udev(udev_device *device, udev_list *hwinfo)
{
        udev_list *entry = NULL;
        LdmDevice *self = NULL;
        gchar *lookup = NULL;
        const char *subsystem = NULL;

        self = g_object_new(LDM_TYPE_DEVICE, NULL);

        /* Set the absolute basics */
        self->os.sysfs_path = g_strdup(udev_device_get_syspath(device));
        self->os.modalias = g_strdup(udev_device_get_sysattr_value(device, "modalias"));

        /* Shouldn't happen, but is definitely possible.. */
        if (!hwinfo) {
                goto post_hwdb;
        }

        /* Duplicate the hardware data into a private table */
        udev_list_entry_foreach(entry, hwinfo)
        {
                const char *prop_id = NULL;
                const char *value = NULL;

                prop_id = udev_list_entry_get_name(entry);
                value = udev_list_entry_get_value(entry);

                g_hash_table_insert(self->os.hwdb_info, g_strdup(prop_id), g_strdup(value));
        }

        /* Set vendor from hwdb information */
        lookup = g_hash_table_lookup(self->os.hwdb_info, HWDB_LOOKUP_PRODUCT_VENDOR);
        if (lookup) {
                self->id.vendor = g_strdup(lookup);
                lookup = NULL;
        }

        /* Set name from hwdb information. TODO: Add fallback name! */
        lookup = g_hash_table_lookup(self->os.hwdb_info, HWDB_LOOKUP_PRODUCT_NAME);
        if (lookup) {
                self->id.name = g_strdup(lookup);
                lookup = NULL;
        }

post_hwdb:

        /* We might need to populate more information per device type */
        subsystem = udev_device_get_subsystem(device);
        if (g_str_equal(subsystem, "pci")) {
                self->os.devtype = LDM_DEVICE_TYPE_PCI;
                ldm_device_init_pci(self, device);
        } else if (g_str_equal(subsystem, "usb")) {
                self->os.devtype = LDM_DEVICE_TYPE_USB;
        }

        return self;
}

/**
 * ldm_device_init_pci:
 * @device: The udev device that we're being created from
 *
 * Handle PCI specific initialisation
 */
static void ldm_device_init_pci(LdmDevice *self, udev_device *device)
{
        const char *sysattr = NULL;

        /* Are we boot_vga ? */
        sysattr = udev_device_get_sysattr_value(device, "boot_vga");
        if (sysattr && g_str_equal(sysattr, "1")) {
                self->pci.boot_vga = TRUE;
        }
}

/**
 * ldm_device_has_type:
 * @mask: Bitwise OR combination of #LdmDeviceType
 *
 * Test whether this device has the given type(s) by testing the mask against
 * our known types.
 */
gboolean ldm_device_has_type(LdmDevice *self, guint mask)
{
        g_return_val_if_fail(self != NULL, FALSE);

        if ((self->os.devtype & mask) == mask) {
                return TRUE;
        }

        return FALSE;
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
