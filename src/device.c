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

#include "device.h"
#include "pci.h"

void ldm_device_free(LdmDevice *device)
{
        if (!device) {
                return;
        }

        /* We'll free from last to first, in effect. */
        if (device->next) {
                ldm_device_free(device->next);
                device->next = NULL;
        }

        free(device->device_name);
        free(device->driver);

        /* Call any implementation specific destructor */
        if (device->dtor) {
                device->dtor(device);
        }

        free(device);
}

LdmDevice *ldm_device_find_vendor(LdmDevice *device, uint16_t vendor)
{
        for (LdmDevice *dev = device; dev; dev = dev->next) {
                if (dev->type != LDM_DEVICE_PCI) {
                        continue;
                }
                LdmPCIDevice *pci_dev = (LdmPCIDevice *)dev;
                if (pci_dev->vendor_id == vendor) {
                        return dev;
                }
        }
        return NULL;
}

/**
 * Determine the number of devices in the chain
 */
int ldm_device_n_devices(LdmDevice *device)
{
        int ret = 0;
        for (LdmDevice *d = device; d; d = d->next) {
                ++ret;
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
