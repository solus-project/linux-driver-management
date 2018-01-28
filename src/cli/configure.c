/*
 * This file is part of linux-driver-management.
 *
 * Copyright © 2016-2018 Linux Driver Management Developers, Solus Project
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
#include <unistd.h>

static inline void print_usage(void)
{
        fputs("configure takes exactly one argument: gpu\n", stderr);
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
        g_autoptr(LdmGLXManager) glx_manager = NULL;

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

        glx_manager = ldm_glx_manager_new();
        if (!ldm_glx_manager_apply_configuration(glx_manager, gpu_config)) {
                fputs("Failed to apply GLX configuration\n", stderr);
                return EXIT_FAILURE;
        }

        fputs("Successfully applied GLX configuration\n", stderr);
        return EXIT_SUCCESS;
}

int ldm_cli_configure(int argc, char **argv)
{
        if (argc != 2) {
                print_usage();
                return EXIT_FAILURE;
        }
        static const gchar *required_paths[] = {
                "/sys/bus/pci",
                "/proc/sys",
                "/sys/class",
        };

        for (guint i = 0; i < G_N_ELEMENTS(required_paths); i++) {
                const gchar *path = required_paths[i];
                if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
                        fprintf(stderr,
                                "Cowardly refusing to continue as path %s is not present\n",
                                path);
                        fprintf(stderr,
                                "This is deliberately avoided so that we don't break your "
                                "configuration\n");
                        return EXIT_SUCCESS;
                }
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
