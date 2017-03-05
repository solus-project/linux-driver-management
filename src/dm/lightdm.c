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

#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "display-manager.h"
#include "util.h"

#define LIGHTDM_CONF_DIR "/etc/lightdm/lightdm.conf.d"
#define LIGHTDM_RANDR_FILE LIGHTDM_CONF_DIR "/99-ldm-xrandr.conf"
#define LIGHTDM_RANDR_EXEC "/etc/lightdm-xrandr-init.sh"

/**
 * Create the xrandr optimus configuration
 */
static bool lightdm_set_xrandr_output(const char *driver, const char *output)
{
        autofree(FILE) *randr_conf = NULL;
        autofree(FILE) *randr_exec = NULL;

        if (!mkdir_p(LIGHTDM_CONF_DIR, 00755)) {
                fprintf(stderr,
                        "Unable to create lightdm conf dir %s: %s\n",
                        LIGHTDM_CONF_DIR,
                        strerror(errno));
                return false;
        }

        /* Write the executable for LDM */
        randr_exec = fopen(LIGHTDM_RANDR_EXEC, "w+");
        if (!randr_exec) {
                fprintf(stderr, "Failed to create lightdm randr file: %s\n", strerror(errno));
                return false;
        }
        if (fprintf(randr_exec,
                    "#!/bin/bash\n"
                    "xrandr --setprovideroutputsource %s %s\n"
                    "xrandr --auto\n",
                    driver,
                    output) < 0) {
                fprintf(stderr, "Failed to write lightdm randr file: %s\n", strerror(errno));
                return false;
        }
        /* Get it chmodded now by close/flush */
        fclose(randr_exec);
        randr_exec = NULL;
        chmod(LIGHTDM_RANDR_EXEC, 00755);

        /* Write the lightdm config file */
        randr_conf = fopen(LIGHTDM_RANDR_FILE, "w+");
        if (!randr_conf) {
                fprintf(stderr, "Failed to create lightdm config file: %s\n", strerror(errno));
                return false;
        }
        if (fprintf(randr_conf,
                    "[Seat:*]\n"
                    "display-setup-script=%s\n",
                    LIGHTDM_RANDR_EXEC) < 0) {
                fprintf(stderr, "Failed to write lightdm config file: %s\n", strerror(errno));
                return false;
        }
        return true;
}

/**
 * Remove the configuration files for our xrandr output if they exist
 */
static bool lightdm_remove_xrandr_output(void)
{
        static const char *lightdm_xrandr_files[] = {
                LIGHTDM_RANDR_FILE, LIGHTDM_RANDR_EXEC,
        };
        bool ret = true;

        for (size_t i = 0; i < ARRAY_SIZE(lightdm_xrandr_files); i++) {
                const char *lfile = lightdm_xrandr_files[i];

                if (access(lfile, F_OK) == 0 && unlink(lfile) < 0) {
                        ret = false;
                }
        }
        return ret;
}

/**
 * Determine if LightDM is in use
 */
static bool lightdm_is_used(void)
{
        /* lightdm always sets XDG_SEAT_PATH whereas GDM does not */
        if (!getenv("XDG_SEAT_PATH")) {
                return false;
        }
        static const char *lightdm_binaries[] = { "/usr/sbin/lightdm", "/usr/bin/lightdm" };
        /* Search for lightdm and if its accessible, use it */
        for (size_t i = 0; i < ARRAY_SIZE(lightdm_binaries); i++) {
                if (access(lightdm_binaries[i], X_OK) == 0) {
                        return true;
                }
        }
        /* Not lightdm */
        return false;
}

/**
 * LightDM implementation of the display manager
 */
const LdmDisplayManager lightdm_display_manager = {
        .set_xrandr_output = lightdm_set_xrandr_output,
        .remove_xrandr_output = lightdm_remove_xrandr_output,
        .is_used = lightdm_is_used,
};

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
