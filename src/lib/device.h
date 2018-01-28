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

G_BEGIN_DECLS

typedef struct _LdmDevice LdmDevice;
typedef struct _LdmDeviceClass LdmDeviceClass;

/**
 * LdmPCIVendorID
 * @LDM_PCI_VENDOR_ID_AMD: AMD PCI device ID
 * @LDM_PCI_VENDOR_ID_INTEL: Intel PCI device ID
 * @LDM_PCI_VENDOR_ID_NVIDIA: NVIDIA PCI device id
 *
 * Well known vendor IDs that are statically defined here to make future
 * lookup and comparisons simpler to users of libldm.
 */
typedef enum {
        LDM_PCI_VENDOR_ID_AMD = 0x1002,
        LDM_PCI_VENDOR_ID_INTEL = 0x8086,
        LDM_PCI_VENDOR_ID_NVIDIA = 0x10DE,
} LdmPCIVendorID;

/**
 * LdmDeviceType:
 * @LDM_DEVICE_TYPE_ANY: Device type is either unknown or unhandled
 * @LDM_DEVICE_TYPE_AUDIO: Audio device
 * @LDM_DEVICE_TYPE_BLUETOOTH: Bluetooth communication device
 * @LDM_DEVICE_TYPE_BOOT_VGA: Special attribute marking the GPU used to boot the system
 * @LDM_DEVICE_TYPE_GPU: A graphics device (onboard, NVIDIA, etc.)
 * @LDM_DEVICE_TYPE_HID: Human interface device (mouse, keyboard, etc.)
 * @LDM_DEVICE_TYPE_IMAGE: Imaging device such as a camera
 * @LDM_DEVICE_TYPE_PCI: The device is connected via PCI
 * @LDM_DEVICE_TYPE_PRINTER: Printer
 * @LDM_DEVICE_TYPE_STORAGE: Storage device
 * @LDM_DEVICE_TYPE_VIDEO: Video device, perhaps a webcam
 * @LDM_DEVICE_TYPE_USB: The device is connected via USB
 * @LDM_DEVICE_TYPE_WIRELESS: A wireless device, i.e. dongle or mouse
 *
 * Any device known to LDM may have one or more "types", defining the primary
 * use-case for these devices. Simple devices will tend to have a single distinct
 * type, such as "GPU". Composite devices, such as a USB camera, may have multiple
 * types, such s HID|IMAGE.
 *
 * As such - each #LdmDevice stores a composite type which may be queried by
 * the API.
 */
typedef enum {
        LDM_DEVICE_TYPE_ANY = 0,
        LDM_DEVICE_TYPE_AUDIO = 1 << 0,
        LDM_DEVICE_TYPE_BLUETOOTH = 1 << 1,
        LDM_DEVICE_TYPE_GPU = 1 << 2,
        LDM_DEVICE_TYPE_HID = 1 << 3,
        LDM_DEVICE_TYPE_IMAGE = 1 << 4,
        LDM_DEVICE_TYPE_PCI = 1 << 5,
        LDM_DEVICE_TYPE_PLATFORM = 1 << 6,
        LDM_DEVICE_TYPE_PRINTER = 1 << 7,
        LDM_DEVICE_TYPE_STORAGE = 1 << 8,
        LDM_DEVICE_TYPE_VIDEO = 1 << 9,
        LDM_DEVICE_TYPE_WIRELESS = 1 << 10,
        LDM_DEVICE_TYPE_USB = 1 << 11,
        LDM_DEVICE_TYPE_MAX = 1 << 12,
} LdmDeviceType;

/**
 * LdmDeviceAttribute
 * @LDM_DEVICE_ATTRIBUTE_ANY: No explicitly set attributes
 * @LDM_DEVICE_ATTRIBUTE_BOOT_VGA: This device is the GPU used to boot the system
 * @LDM_DEVICE_ATTRIBUTE_HOST: Enable differentiation between host and non host controllers
 * @LDM_DEVICE_ATTRIBUTE_INTERFACE: Pseudo-device (USB interface, etc.)
 *
 * A device have one or more special attributes that need to be queried beyond
 * the initial DeviceType search.
 */
typedef enum {
        LDM_DEVICE_ATTRIBUTE_ANY = 0,
        LDM_DEVICE_ATTRIBUTE_BOOT_VGA = 1 << 0,
        LDM_DEVICE_ATTRIBUTE_HOST = 1 << 1,
        LDM_DEVICE_ATTRIBUTE_INTERFACE = 1 << 2,
        LDM_DEVICE_ATTRIBUTE_MAX = 1 << 3,
} LdmDeviceAttribute;

#define LDM_TYPE_DEVICE ldm_device_get_type()
#define LDM_DEVICE(o) (G_TYPE_CHECK_INSTANCE_CAST((o), LDM_TYPE_DEVICE, LdmDevice))
#define LDM_IS_DEVICE(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), LDM_TYPE_DEVICE))
#define LDM_DEVICE_CLASS(o) (G_TYPE_CHECK_CLASS_CAST((o), LDM_TYPE_DEVICE, LdmDeviceClass))
#define LDM_IS_DEVICE_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), LDM_TYPE_DEVICE))
#define LDM_DEVICE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), LDM_TYPE_DEVICE, LdmDeviceClass))

GType ldm_device_get_type(void);

/* API */
const gchar *ldm_device_get_modalias(LdmDevice *device);
const gchar *ldm_device_get_name(LdmDevice *device);
const gchar *ldm_device_get_path(LdmDevice *device);
gint ldm_device_get_product_id(LdmDevice *device);
const gchar *ldm_device_get_vendor(LdmDevice *device);
gint ldm_device_get_vendor_id(LdmDevice *device);
LdmDeviceType ldm_device_get_device_type(LdmDevice *device);
LdmDeviceAttribute ldm_device_get_attributes(LdmDevice *device);

gboolean ldm_device_has_type(LdmDevice *device, LdmDeviceType mask);
gboolean ldm_device_has_attribute(LdmDevice *device, LdmDeviceAttribute mask);

LdmDevice *ldm_device_get_parent(LdmDevice *device);
GList *ldm_device_get_children(LdmDevice *device);

gint ldm_device_get_priority(LdmDevice *device);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(LdmDevice, g_object_unref)

G_END_DECLS

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
