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
#include <stdlib.h>
#include <unistd.h>

#include "cli.h"
#include "config.h"
#include "device.h"
#include "pci.h"
#include "scanner.h"
#include "util.h"

/**
 * Updated as and when we add more driver types
 */
static const char *known_driver_types = "gpu";

/**
 * Simple GPU configuration.
 */
int ldm_cli_configure_gpu_simple(__ldm_unused__ LdmDevice *device)
{
        fprintf(stderr, "Simple configure: %s\n", device->device_name);
        fputs("Not yet implemented\n", stderr);
        return EXIT_FAILURE;
}

/**
 * Encountered Optimus GPU configuration
 */
int ldm_cli_configure_gpu_optimus(LdmDevice *intel_dev, LdmDevice *nvidia_dev)
{
        fprintf(stderr, "Optimus: %s | %s\n", intel_dev->device_name, nvidia_dev->device_name);
        fputs("Not yet implemented\n", stderr);
        return EXIT_FAILURE;
}

/**
 * Configure the system GPU
 */
int ldm_cli_configure_gpu(void)
{
        autofree(LdmDevice) *devices = NULL;
        LdmDevice *intel = NULL;
        LdmDevice *nvidia = NULL;

        /* Find the usable GPUs first */
        devices = ldm_scan_devices(LDM_DEVICE_PCI, LDM_CLASS_GRAPHICS);
        if (!devices) {
                fputs("Cannot find a usable GPU on this system\n", stderr);
                return EXIT_FAILURE;
        }

        /* Trivial configuration */
        if (ldm_device_n_devices(devices) == 1) {
                return ldm_cli_configure_gpu_simple(devices);
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
                return ldm_cli_configure_gpu_optimus(intel, nvidia);
        }

        /* Has NVIDIA, but not optimus. Only configure the NVIDIA gpu */
        return ldm_cli_configure_gpu_simple(nvidia);

non_optimus:
        /* TODO: Check for AMDGPU hybrid */

        /* Multiple devices, potential ATX+iGPU+DGPU/SLI/Hybrid */
        fputs("Complex configure: Not yet implemented\n", stderr);
        return EXIT_FAILURE;
}

int ldm_cli_configure(int argc, char **argv)
{
        int (*arg_configure)(void) = NULL;

        if (argc != 1) {
                fputs("Incorrect usage. Expected keyword\n", stderr);
                goto emit_types;
        }

        /* Determine the helper to use */
        if (streq(argv[0], "gpu")) {
                arg_configure = ldm_cli_configure_gpu;
        }

        if (!arg_configure) {
                fprintf(stderr, "Unknown driver type: '%s'\n", argv[0]);
                goto emit_types;
        }

        /* Make sure they're root now. */
        if (geteuid() != 0) {
                fputs("You must have root privileges to use this command\n", stderr);
                return EXIT_FAILURE;
        }

        /* Execute using the helper */
        return arg_configure();

emit_types:
        fprintf(stderr, "Known types: %s\n", known_driver_types);
        return EXIT_FAILURE;
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
