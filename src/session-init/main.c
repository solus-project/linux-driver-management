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

#define _GNU_SOURCE

#include "config.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"
#include <ldm.h>

/**
 * Perform static automatic configuration for Optimus.
 *
 * Long story short, we just invoke xrandr. Nowt fancy here.
 */
static int ldm_session_init_configure_optimus(void)
{
        g_autoptr(GError) error = NULL;
        gint ext = 0;

        /* Force the provider output source */
        if (!g_spawn_command_line_sync("xrandr --setprovideroutputsource modesetting NVIDIA-0",
                                       NULL,
                                       NULL,
                                       &ext,
                                       &error)) {
                g_warning("xrandr exited with status %d: %s", ext, error->message);
                return ext;
        }

        /* Force configuration */
        if (!g_spawn_command_line_sync("xrandr --auto", NULL, NULL, &ext, &error)) {
                g_warning("Flushing xrandr exited with status %d: %s", ext, error->message);
                return ext;
        }

        return EXIT_SUCCESS;
}

static int ldm_session_init_configure(void)
{
        g_autoptr(LdmManager) manager = NULL;
        g_autoptr(LdmGPUConfig) config = NULL;

        /* Grab manager now */
        manager = ldm_manager_new(LDM_MANAGER_FLAGS_NO_MONITOR | LDM_MANAGER_FLAGS_GPU_QUICK);
        if (!manager) {
                return EXIT_FAILURE;
        }

        /* Grab config */
        config = ldm_gpu_config_new(manager);
        if (!config) {
                return EXIT_FAILURE;
        }

        /* We only know Optimus right now.. */
        if (ldm_gpu_config_has_type(config, LDM_GPU_TYPE_OPTIMUS)) {
                return ldm_session_init_configure_optimus();
        }

        g_warning("ldm-session-init invoked with an unknown configuration!");
        return EXIT_FAILURE;
}

/**
 * Main entry into ldm-session-init
 *
 * This is a quick and easy init point for all sessions with LDM enabled distros.
 * If hybrid graphics are enabled, we execute the relevant xrandr setup and exit.
 * Otherwise, we just exit, real quick.
 *
 * The idea is to allow the package to provide the stateless configurations for
 * the various helpers (lightdm, gdm, etc) so it doesn't have to do lots of
 * filesystem mangling, and instead deal with a fixed point.
 */

int main(__ldm_unused__ int argc, __ldm_unused__ char **argv)
{
        /* If the hybrid file doesn't exist, immediately exit. */
        if (access(LDM_HYBRID_FILE, F_OK) != 0) {
                return EXIT_SUCCESS;
        }

        return ldm_session_init_configure();
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
