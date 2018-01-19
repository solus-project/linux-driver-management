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

#include "ldm.h"
#include "util.h"

#include "cli.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * Handle pretty printing of a single device to the display
 */
static void print_device(LdmDevice *device)
{
        gboolean gpu = FALSE;

        gpu = ldm_device_has_type(device, LDM_DEVICE_TYPE_GPU);

        /* Pretty strings */
        fprintf(stdout, " \u255E Device Name   : %s\n", ldm_device_get_name(device));
        fprintf(stdout, " \u255E Manufacturer  : %s\n", ldm_device_get_vendor(device));

        /* Ids */
        fprintf(stdout, " \u255E Product ID    : %x\n", ldm_device_get_product_id(device));
        fprintf(stdout,
                " %s Vendor ID     : %x\n",
                gpu ? "\u255E" : "\u2558",
                ldm_device_get_vendor_id(device));
        if (!gpu) {
                return;
        }

        /* GPU Specifics */
        fprintf(stdout,
                " \u2558 Boot VGA      : %s\n",
                ldm_device_has_attribute(device, LDM_DEVICE_ATTRIBUTE_BOOT_VGA) ? "yes" : "no");
}

/**
 * Handle pretty printing of the GPU configuration to the display
 */
static void print_gpu_config(LdmGPUConfig *config)
{
        LdmDevice *primary = NULL, *secondary = NULL;

        primary = ldm_gpu_config_get_primary_device(config);
        secondary = ldm_gpu_config_get_secondary_device(config);

        if (ldm_gpu_config_has_type(config, LDM_GPU_TYPE_OPTIMUS)) {
                fputs("NVIDIA Optimus\n", stdout);
        } else if (ldm_gpu_config_has_type(config, LDM_GPU_TYPE_HYBRID)) {
                fputs("Hybrid Graphics\n", stdout);
        } else if (ldm_gpu_config_has_type(config, LDM_GPU_TYPE_CROSSFIRE)) {
                fputs("AMD Crossfire\n", stdout);
        } else if (ldm_gpu_config_has_type(config, LDM_GPU_TYPE_SLI)) {
                fputs("NVIDIA SLI\n", stdout);
        } else if (ldm_gpu_config_has_type(config, LDM_GPU_TYPE_COMPOSITE)) {
                fputs("Composite GPU\n", stdout);
        } else {
                fputs("Simple GPU configuration\n", stdout);
        }

        fputs("\n", stdout);

        /* We're only concerned with primary vs secondary devices */
        fprintf(stdout,
                " \u2552 Primary GPU%s\n",
                ldm_gpu_config_has_type(config, LDM_GPU_TYPE_HYBRID) ? " (iGPU)" : "");
        ;
        print_device(primary);

        if (!secondary) {
                return;
        }

        fprintf(stdout,
                " \u2552 Primary GPU%s\n",
                ldm_gpu_config_has_type(config, LDM_GPU_TYPE_HYBRID) ? " (dGPU)" : "");
        ;
        print_device(secondary);
}

int ldm_cli_status(__ldm_unused__ int argc, __ldm_unused__ char **argv)
{
        g_autoptr(LdmManager) manager = NULL;
        g_autoptr(LdmGPUConfig) gpu_config = NULL;

        /* No need for hot plug events */
        manager = ldm_manager_new(LDM_MANAGER_FLAGS_NO_MONITOR);
        if (!manager) {
                fprintf(stderr, "Failed to initialiase LdmManager\n");
                return EXIT_FAILURE;
        }

        gpu_config = ldm_gpu_config_new(manager);
        if (!gpu_config) {
                fprintf(stderr, "Failed to obtain LdmGPUConfig\n");
                return EXIT_FAILURE;
        }

        print_gpu_config(gpu_config);

        /* TODO: Emit any other devices with potential matches */

        return EXIT_SUCCESS;
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
