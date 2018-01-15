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

#pragma once

#include "device.h"

/**
 * Scan for PCI devices only, with the given classmask
 *
 * @return a chained list of devices
 */
LdmDevice *ldm_scan_pci_devices(unsigned int classmask);

/**
 * Scan for USB devices only, with the given classmask
 *
 * @return a chained list of devices
 */
LdmDevice *ldm_scan_usb_devices(unsigned int classmask);

/**
 * Determine the driver for a PCI device
 */
char *ldm_device_driver(LdmDevice *device);

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
