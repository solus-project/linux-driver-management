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

#include <stdio.h>

#include "device.h"
#include "pci.h"
#include "scanner.h"
#include "util.h"

/**
 * Simple GPU configuration.
 */
static bool ldm_configure_gpu_simple(LdmDevice *device)
{
        fprintf(stderr, "Simple configure: %s\n", device->device_name);
        fputs("Not yet implemented\n", stderr);
        return false;
}

/**
 * Encountered Optimus GPU configuration
 */
static bool ldm_configure_gpu_optimus(LdmDevice *intel_dev, LdmDevice *nvidia_dev)
{
        fprintf(stderr, "Optimus: %s | %s\n", intel_dev->device_name, nvidia_dev->device_name);
        fputs("Not yet implemented\n", stderr);
        return false;
}

/**
 * Configure the system GPU
 */
bool ldm_configure_gpu(void)
{
        autofree(LdmDevice) *devices = NULL;
        LdmDevice *intel = NULL;
        LdmDevice *nvidia = NULL;

        /* Find the usable GPUs first */
        devices = ldm_scan_devices(LDM_DEVICE_PCI, LDM_CLASS_GRAPHICS);
        if (!devices) {
                fputs("Cannot find a usable GPU on this system\n", stderr);
                return false;
        }

        /* Trivial configuration */
        if (ldm_device_n_devices(devices) == 1) {
                return ldm_configure_gpu_simple(devices);
        }

        /* Look for optimus GPU */
        intel = ldm_device_find_vendor(devices, PCI_VENDOR_ID_INTEL);
        nvidia = ldm_device_find_vendor(devices, PCI_VENDOR_ID_NVIDIA);

        if (!intel || !nvidia) {
                goto non_optimus;
        }

        /* At this point we have intel & nvidia. If intel is the GPU used
         * by the BIOS we have optimus. */
        if (ldm_pci_device_is_boot_vga((LdmPCIDevice *)intel)) {
                return ldm_configure_gpu_optimus(intel, nvidia);
        }

        /* Has NVIDIA, but not optimus. Only configure the NVIDIA gpu */
        return ldm_configure_gpu_simple(nvidia);

non_optimus:
        /* TODO: Check for AMDGPU hybrid */

        /* Multiple devices, potential ATX+iGPU+DGPU/SLI/Hybrid */
        fputs("Complex configure: Not yet implemented\n", stderr);
        return false;
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
