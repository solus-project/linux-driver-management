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

#include "cli.h"
#include "ldm.h"
#include "util.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

static inline void print_usage(void)
{
        fputs("configure takes exactly one argument: gpu\n", stderr);
}

/**
 * Handle Optimus GPU configuration
 *
 * Effectively this means going the NVIDIA route with some custom hackery applied
 * in the X11 configuration to force switching to another output.
 */
static int ldm_gpu_configure_optimus(__ldm_unused__ LdmManager *manager,
                                     __ldm_unused__ LdmGPUConfig *config)
{
        fputs("Optimus configuration not yet supported\n", stderr);
        return EXIT_FAILURE;
}

/**
 * Perform "simple" GPU configuration.
 *
 * Basically find the appropriate driver configuration to use and ensure that
 * any past configuration attempts are undone, and that the libGL symlinks are
 * all pointing in the right place.
 */
static int ldm_gpu_configure_simple(__ldm_unused__ LdmManager *manager,
                                    __ldm_unused__ LdmGPUConfig *config)
{
        fputs("Simple configuration not yet supported\n", stderr);
        return EXIT_FAILURE;
}

/**
 * Perform configuration of the GPU. Right now this is required explicitly
 * for X11 systems, and those using NVIDIA proprietary drivers with the
 * libGL file conflicts.
 *
 * In future we'll support glvnd as and when Solus does, but for now we
 * need to know about both methods..
 */
static int ldm_cli_configure_gpu(void)
{
        g_autoptr(LdmManager) manager = NULL;
        g_autoptr(LdmGPUConfig) gpu_config = NULL;

        /* Need manager without hotplug capabilities */
        manager = ldm_manager_new(LDM_MANAGER_FLAGS_NO_MONITOR);
        if (!manager) {
                fputs("Failed to initialise LdmManager\n", stderr);
                return EXIT_FAILURE;
        }

        /* Obtain the GPU Configuration so we know what we're dealing with */
        gpu_config = ldm_gpu_config_new(manager);
        if (!gpu_config) {
                fputs("Failed to initialize LdmGPUConfig\n", stderr);
                return EXIT_FAILURE;
        }

        /* At this point just dispatch to the appropriate method. */
        if (ldm_gpu_config_has_type(gpu_config, LDM_GPU_TYPE_OPTIMUS)) {
                return ldm_gpu_configure_optimus(manager, gpu_config);
        }

        /* Just let them know that we know, but can't do anything with it right now. */
        if (ldm_gpu_config_has_type(gpu_config, LDM_GPU_TYPE_HYBRID)) {
                LdmDevice *detection = ldm_gpu_config_get_detection_device(gpu_config);
                fprintf(stdout,
                        "Currently treating hybrid detection as simple device: %s\n",
                        ldm_device_get_name(detection));
        }

        /* In all other cases we're just dealing with "simple" configs */
        return ldm_gpu_configure_simple(manager, gpu_config);
}

int ldm_cli_configure(int argc, char **argv)
{
        if (argc != 2) {
                print_usage();
                return EXIT_FAILURE;
        }

        if (g_str_equal(argv[1], "gpu")) {
                if (geteuid() != 0) {
                        fputs("You must be root to use this function\n", stderr);
                        return EXIT_FAILURE;
                }
                return ldm_cli_configure_gpu();
        }

        print_usage();
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
