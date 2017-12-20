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

#include <stdlib.h>

#include "ldm-private.h"
#include "pci-device.h"
#include "util.h"

#define PCI_CLASS_DISPLAY_VGA 0x0300
#define PCI_CLASS_DISPLAY_OTHER 0x0380

struct _LdmPCIDeviceClass {
        LdmDeviceClass parent_class;
};

/**
 * SECTION:pci-device
 * @Short_description: PCI Device abstraction
 * @see_also: #LdmDevice
 * @Title: LdmPCIDevice
 *
 * An LdmPCIDevice is a specialised implementation of the #LdmDevice which
 * is aware of PCI capabilities and GPU data. This class is never directly
 * created by the user, but is instead returned by the #LdmManager.
 *
 * This class extends the base #LdmDevice to add PCI specific data.
 * The primary use case within LDM is to detect GPUs, which will all
 * carry the #LdmDevice:device-type of #LDM_DEVICE_TYPE_PCI | #LDM_DEVICE_TYPE_GPU.
 *
 * Users can test if a device is a PCI device without having to cast, by
 * simply checking the #LdmDevice:device-type:
 *
 * |[<!-- language="C" -->
 *      if (ldm_device_has_type(device, LDM_DEVICE_TYPE_PCI)) {
 *              g_message("Found PCI device");
 *      }
 *
 *      // Alternatively..
 *      if (LDM_IS_PCI_DEVICE(device)) {
 *              g_message("Found PCI device through casting");
 *      }
 * ]|
 */
struct _LdmPCIDevice {
        LdmDevice parent;
};

G_DEFINE_TYPE(LdmPCIDevice, ldm_pci_device, LDM_TYPE_DEVICE)

/**
 * ldm_pci_device_dispose:
 *
 * Clean up a LdmPCIDevice instance
 */
static void ldm_pci_device_dispose(GObject *obj)
{
        G_OBJECT_CLASS(ldm_pci_device_parent_class)->dispose(obj);
}

/**
 * ldm_pci_device_class_init:
 *
 * Handle class initialisation
 */
static void ldm_pci_device_class_init(LdmPCIDeviceClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_pci_device_dispose;
}

/**
 * ldm_pci_device_init:
 *
 * Handle construction of the LdmPCIDevice
 */
static void ldm_pci_device_init(LdmPCIDevice *self)
{
        LdmDevice *ldm = LDM_DEVICE(self);
        ldm->os.devtype |= LDM_DEVICE_TYPE_PCI;
}

/**
 * ldm_pci_device_assign_pvid:
 *
 * Assign product/vendor ID to the device from the PCI sysfs attributes
 */
static void ldm_pci_device_assign_pvid(LdmDevice *self, udev_device *device)
{
        const char *sysattr = NULL;

        /* Grab the vendor */
        sysattr = udev_device_get_sysattr_value(device, "vendor");
        if (!sysattr) {
                goto product;
        }
        self->id.vendor_id = (gint)(strtoll(sysattr, NULL, 0));
        sysattr = NULL;

product:

        /* Grab the product */
        sysattr = udev_device_get_sysattr_value(device, "device");
        if (!sysattr) {
                return;
        }
        self->id.product_id = (gint)(strtoll(sysattr, NULL, 0));
}

/**
 * ldm_pci_device_init_private:
 * @device: The udev device that we're being created from
 *
 * Handle PCI specific initialisation
 */
void ldm_pci_device_init_private(LdmDevice *self, udev_device *device)
{
        const char *sysattr = NULL;
        int pci_class = 0;

        ldm_pci_device_assign_pvid(self, device);

        /* Are we boot_vga ? */
        sysattr = udev_device_get_sysattr_value(device, "boot_vga");
        if (sysattr && g_str_equal(sysattr, "1")) {
                self->os.attributes |= LDM_DEVICE_ATTRIBUTE_BOOT_VGA;
        }
        sysattr = NULL;

        /* Grab the device class */
        sysattr = udev_device_get_sysattr_value(device, "class");
        if (!sysattr) {
                return;
        }

        /* Does it look like a display device? */
        pci_class = (int)(strtoll(sysattr, NULL, 0) >> 8);
        if (pci_class >= PCI_CLASS_DISPLAY_VGA && pci_class <= PCI_CLASS_DISPLAY_OTHER) {
                self->os.devtype |= LDM_DEVICE_TYPE_GPU;
        }
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
