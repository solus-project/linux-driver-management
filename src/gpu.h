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

#pragma once

#include <stdbool.h>

#include "device.h"

/**
 * LdmGPU represents a usable GPU on the system
 */
typedef struct LdmGPU {
        LdmDevice device;      /**<Extend LdmDevice */
        LdmPCIAddress address; /**<Address of the GPU */
        uint16_t vendor_id;    /**<PCI vendor ID */
        uint16_t device_id;    /**<PCI device ID */
        bool boot_vga;         /**<Whether this is the boot device */
} LdmGPU;

/**
 * Free a previously LdmGPU
 */
void ldm_gpu_free(LdmGPU *gpu);

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
