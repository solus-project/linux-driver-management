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

/* PCI display devices are between 0x03 and 0x0380 */
#define PCI_DISPLAY_MASK 0x300

struct _LdmPCIDeviceClass {
        LdmDeviceClass parent_class;
};

/*
 * LdmPCIDevice
 *
 * An LdmPCIDevice is a specialised implementation of the #LdmDevice which
 * is aware of PCI capabilities and GPU data.
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
 * ldm_pci_device_init_private:
 * @device: The udev device that we're being created from
 *
 * Handle PCI specific initialisation
 */
void ldm_pci_device_init_private(LdmDevice *self, udev_device *device)
{
        const char *sysattr = NULL;
        int pci_class = 0;

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
        if ((pci_class & PCI_DISPLAY_MASK) == PCI_DISPLAY_MASK) {
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
