/*
 * This file is part of linux-driver-management.
 *
 * Copyright © 2016-2018 Ikey Doherty
 *
 * linux-driver-management is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#define _GNU_SOURCE

#include <stdio.h>

#include "device.h"
#include "ldm-private.h"
#include "pci.h"
#include "scanner.h"
#include "usb.h"

LdmDevice *ldm_scan_devices(LdmDeviceType type, unsigned int classmask)
{
        switch (type) {
        case LDM_DEVICE_PCI:
                return ldm_scan_pci_devices(classmask);
        case LDM_DEVICE_USB:
                return ldm_scan_usb_devices(classmask);
        default:
                fputs("Unknown type of device\n", stderr);
                return NULL;
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
