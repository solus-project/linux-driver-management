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
#include "ldm-enums.h"
#include "ldm-private.h"
#include "pci-device.h"
#include "usb-device.h"
#include "util.h"

#define HWDB_LOOKUP_PRODUCT_NAME "ID_MODEL_FROM_DATABASE"
#define HWDB_LOOKUP_PRODUCT_VENDOR "ID_VENDOR_FROM_DATABASE"

static void ldm_device_set_property(GObject *object, guint id, const GValue *value,
                                    GParamSpec *spec);
static void ldm_device_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec);

G_DEFINE_TYPE(LdmDevice, ldm_device, G_TYPE_OBJECT)

/* Property IDs */
enum { PROP_PARENT = 1,
       PROP_PATH,
       PROP_MODALIAS,
       PROP_NAME,
       PROP_VENDOR,
       PROP_DEV_TYPE,
       PROP_ATTRIBUTES,
       N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

/* Signal IDs */
enum { SIGNAL_CHILD_ADDED = 0, SIGNAL_CHILD_REMOVED, N_SIGNALS };

static guint obj_signals[N_SIGNALS] = { 0 };

/**
 * ldm_device_dispose:
 *
 * Clean up a LdmDevice instance
 */
static void ldm_device_dispose(GObject *obj)
{
        LdmDevice *self = LDM_DEVICE(obj);

        g_clear_pointer(&self->tree.kids, g_hash_table_unref);
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
        obj_class->set_property = ldm_device_set_property;

        /**
         * LdmDevice::child-added:
         * @device: The device owning the new child
         * @child: The newly available child
         *
         * Notify interested parties that a new child device was added
         */
        obj_signals[SIGNAL_CHILD_ADDED] = g_signal_new("child-added",
                                                       LDM_TYPE_DEVICE,
                                                       G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                                       G_STRUCT_OFFSET(LdmDeviceClass, child_added),
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       G_TYPE_NONE,
                                                       1,
                                                       LDM_TYPE_DEVICE);

        /**
         * LdmDevice::child-removed:
         * @device: The device that owned the child
         * @id: The child ID being removed.
         *
         * Notify interested parties that a child was removed
         */
        obj_signals[SIGNAL_CHILD_REMOVED] =
            g_signal_new("child-removed",
                         LDM_TYPE_DEVICE,
                         G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                         G_STRUCT_OFFSET(LdmDeviceClass, child_removed),
                         NULL,
                         NULL,
                         NULL,
                         G_TYPE_NONE,
                         1,
                         G_TYPE_STRING);

        /**
         * LdmDevice:parent: (type LdmDevice) (transfer none)
         *
         * Parent device for this device instance
         */
        obj_properties[PROP_PARENT] =
            g_param_spec_pointer("parent",
                                 "Parent device",
                                 "Parent device for this device",
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
        /**
         * LdmDevice:path
         *
         * The system path for this device. On Linux this is the sysfs path.
         */
        obj_properties[PROP_PATH] = g_param_spec_string("path",
                                                        "The device path",
                                                        "System path for this device",
                                                        NULL,
                                                        G_PARAM_READABLE);

        /**
         * LdmDevice:modalias
         *
         * The modalias reported by the kernel for this device.
         */
        obj_properties[PROP_MODALIAS] = g_param_spec_string("modalias",
                                                            "The device modalias",
                                                            "System modalias for this device",
                                                            NULL,
                                                            G_PARAM_READABLE);

        /**
         * LdmDevice:name
         *
         * The name used to display this device to users, i.e. the model.
         */
        obj_properties[PROP_NAME] = g_param_spec_string("name",
                                                        "The device name",
                                                        "Display name (model) for this device",
                                                        NULL,
                                                        G_PARAM_READABLE);

        /**
         * LdmDevice:vendor
         *
         * The vendor string to display the users, i.e. the manufacturer.
         */
        obj_properties[PROP_VENDOR] =
            g_param_spec_string("vendor",
                                "The device vendor",
                                "The vendor (manufacturer) for this device",
                                NULL,
                                G_PARAM_READABLE);

        /**
         * LdmDevice:device-type
         *
         * The composite type of this device, which is a bitwise combination
         * of multiple device types (such as PCI|GPU)
         */
        obj_properties[PROP_DEV_TYPE] = g_param_spec_flags("device-type",
                                                           "Device type",
                                                           "Composite type for this device",
                                                           LDM_TYPE_DEVICE_TYPE,
                                                           LDM_DEVICE_TYPE_ANY,
                                                           G_PARAM_READABLE);

        /**
         * LdmDevice:attributes
         *
         * The composite attributes of this device, which is a bitwise combination
         * of multiple attribute (such as BOOT_VGA)
         */
        obj_properties[PROP_ATTRIBUTES] = g_param_spec_flags("attributes",
                                                             "Device attributes",
                                                             "Composite attributes for this device",
                                                             LDM_TYPE_DEVICE_ATTRIBUTE,
                                                             LDM_DEVICE_ATTRIBUTE_ANY,
                                                             G_PARAM_READABLE);

        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

static void ldm_device_set_property(GObject *object, guint id, const GValue *value,
                                    GParamSpec *spec)
{
        LdmDevice *self = LDM_DEVICE(object);

        switch (id) {
        case PROP_PARENT:
                self->tree.parent = g_value_get_pointer(value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

static void ldm_device_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec)
{
        LdmDevice *self = LDM_DEVICE(object);

        switch (id) {
        case PROP_PARENT:
                g_value_set_pointer(value, self->tree.parent);
                break;
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
        case PROP_DEV_TYPE:
                g_value_set_flags(value, self->os.devtype);
                break;
        case PROP_ATTRIBUTES:
                g_value_set_flags(value, self->os.attributes);
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

        /* We have sysfs ID to child mapping and own the child */
        self->tree.kids = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
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
 * @parent: (nullable): Parent device, if any.
 * @device: Associated udev device
 * @hwinfo: If set, the hwdb entry for this device.
 *
 * Construct a new LdmDevice from the given udev device and hwdb information.
 * This is private API between the manager and the device.
 */
LdmDevice *ldm_device_new_from_udev(LdmDevice *parent, udev_device *device, udev_list *properties)
{
        udev_list *entry = NULL;
        LdmDevice *self = NULL;
        gchar *lookup = NULL;
        const char *subsystem = NULL;
        GType special_type = 0;

        /* Specialise the gtype here */
        subsystem = udev_device_get_subsystem(device);
        if (g_str_equal(subsystem, "usb")) {
                special_type = LDM_TYPE_USB_DEVICE;
        } else if (g_str_equal(subsystem, "pci")) {
                special_type = LDM_TYPE_PCI_DEVICE;
        } else {
                special_type = LDM_TYPE_DEVICE;
        }

        self = g_object_new(special_type, "parent", parent, NULL);

        /* Set the absolute basics */
        self->os.sysfs_path = g_strdup(udev_device_get_syspath(device));
        self->os.modalias = g_strdup(udev_device_get_sysattr_value(device, "modalias"));

        /* Shouldn't happen, but is definitely possible.. */
        if (!properties) {
                goto post_hwdb;
        }

        /* Duplicate the hardware data into a private table */
        udev_list_entry_foreach(entry, properties)
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

        if (special_type == LDM_TYPE_PCI_DEVICE) {
                ldm_pci_device_init_private(self, device);
        } else if (special_type == LDM_TYPE_USB_DEVICE) {
                ldm_usb_device_init_private(self, device);
        }

        return self;
}

/**
 * ldm_device_get_device_type:
 *
 * Return the device type (bitwise field)
 */
LdmDeviceType ldm_device_get_device_type(LdmDevice *self)
{
        g_return_val_if_fail(self != NULL, LDM_DEVICE_TYPE_ANY);
        return self->os.devtype;
}

/**
 * ldm_device_has_type:
 * @mask: Bitwise OR combination of #LdmDeviceType
 *
 * Test whether this device has the given type(s) by testing the mask against
 * our known types.
 */
gboolean ldm_device_has_type(LdmDevice *self, LdmDeviceType mask)
{
        g_return_val_if_fail(self != NULL, FALSE);

        if ((self->os.devtype & mask) == mask) {
                return TRUE;
        }

        return FALSE;
}

/**
 * ldm_device_get_attributes:
 *
 * Return the device type (bitwise field)
 */
LdmDeviceAttribute ldm_device_get_attributes(LdmDevice *self)
{
        g_return_val_if_fail(self != NULL, LDM_DEVICE_ATTRIBUTE_ANY);
        return self->os.attributes;
}

/**
 * ldm_device_has_attribute:
 * @mask: Bitwise OR combination of #LdmDeviceAttribute
 *
 * Test whether this device has the given attribute(s) by testing the mask against
 * our known attributes
 */
gboolean ldm_device_has_attribute(LdmDevice *self, LdmDeviceAttribute mask)
{
        g_return_val_if_fail(self != NULL, FALSE);

        if ((self->os.attributes & mask) == mask) {
                return TRUE;
        }

        return FALSE;
}

/**
 * ldm_device_get_parent:
 *
 * Get the parent device, if any
 *
 * Returns: (transfer none) (nullable): The parent device if it exists
 */
LdmDevice *ldm_device_get_parent(LdmDevice *self)
{
        g_return_val_if_fail(self != NULL, NULL);

        return self->tree.parent;
}

/**
 * ldm_device_get_children:
 *
 * Return any child devices, if any
 *
 * Returns: (element-type Ldm.Device) (transfer container): a list of all child devices
 */
GList *ldm_device_get_children(LdmDevice *self)
{
        g_return_val_if_fail(self != NULL, NULL);

        return g_hash_table_get_values(self->tree.kids);
}

/**
 * ldm_device_add_child:
 * @child: (transfer full): Child to add to this device
 *
 * Add a new child to this device, with this device now taking ownership of it.
 * Note: Children must be constructed with the parent explicitly being this
 * instance, it is not possible to reparent a child, instead they must
 * be destroyed.
 */
void ldm_device_add_child(LdmDevice *self, LdmDevice *child)
{
        const gchar *id = NULL;
        g_return_if_fail(self != NULL);

        id = ldm_device_get_path(child);
        if (!g_hash_table_replace(self->tree.kids, g_strdup(id), g_object_ref_sink(child))) {
                return;
        }
        g_signal_emit(self, obj_signals[SIGNAL_CHILD_ADDED], 0, child);
}

/**
 * ldm_device_remove_child:
 * @child: The child to remove from this device
 *
 * This is a wrapper around #ldm_device_remove_child_by_path
 */
void ldm_device_remove_child(LdmDevice *self, LdmDevice *child)
{
        const gchar *id = NULL;
        g_return_if_fail(self != NULL);

        id = ldm_device_get_path(child);
        ldm_device_remove_child_by_path(self, id);
}

/**
 * ldm_device_remove_child_by_path:
 * @path: Sysfs path for the child to be removed
 *
 * Remove a child from this device if we own it, and unset the parent on
 * that device.
 */
void ldm_device_remove_child_by_path(LdmDevice *self, const gchar *path)
{
        g_return_if_fail(self != NULL);

        if (!g_hash_table_remove(self->tree.kids, path)) {
                return;
        }
        g_signal_emit(self, obj_signals[SIGNAL_CHILD_REMOVED], 0, path);
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
