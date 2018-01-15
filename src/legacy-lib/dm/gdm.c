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

#include <errno.h>
#include <unistd.h>

#include "display-manager.h"
#include "util.h"

static const char *gdm_autostart_paths[] = { "/usr/share/gdm/greeter/autostart/optimus.desktop",
                                             "/etc/xdg/autostart/optimus.desktop" };

static const char *gdm_autostart_dirs[] = {
        "/usr/share/gdm/greeter/autostart",
        "/etc/xdg/autostart",
};

/**
 * Configure GDM to automatically initialise optimus via xrandr
 * Inspired/lifted directly from: https://wiki.archlinux.org/index.php/NVIDIA_Optimus#GDM
 * Many thanks to the Arch Wiki folks
 */
static bool gdm_set_xrandr_output(const char *driver, const char *output)
{
        /* Create required directories */
        for (size_t i = 0; i < ARRAY_SIZE(gdm_autostart_dirs); i++) {
                const char *dir = gdm_autostart_dirs[i];
                if (!mkdir_p(dir, 00755)) {
                        fprintf(stderr,
                                "Failed to create autostart dir %s: %s\n",
                                dir,
                                strerror(errno));
                        return false;
                }
        }
        /* Write the same autostart file twice. Once for GDM greeter, once for the
         * session initialisation itself */
        for (size_t i = 0; i < ARRAY_SIZE(gdm_autostart_paths); i++) {
                const char *st = gdm_autostart_paths[i];
                autofree(FILE) *gdm_conf = NULL;

                gdm_conf = fopen(st, "w+");
                if (!gdm_conf) {
                        fprintf(stderr, "Failed to create gdm randr file: %s\n", strerror(errno));
                        return false;
                }
                if (fprintf(gdm_conf,
                            "[Desktop Entry]\n"
                            "Type=Application\n"
                            "Name=Optimus\n"
                            "Exec=sh -c \"xrandr --setprovideroutputsource %s %s; xrandr --auto\"\n"
                            "NoDisplay=true\n"
                            "X-GNOME-Autostart-Phase=DisplayServer\n",
                            driver,
                            output) < 0) {
                        fprintf(stderr, "Failed to write gdm randr file: %s\n", strerror(errno));
                        return false;
                }
        }
        return true;
}

/**
 * Remove the autostart files for GDM
 */
static bool gdm_remove_xrandr_output(void)
{
        bool ret = true;
        for (size_t i = 0; i < ARRAY_SIZE(gdm_autostart_paths); i++) {
                const char *lfile = gdm_autostart_paths[i];

                if (access(lfile, F_OK) == 0 && unlink(lfile) < 0) {
                        ret = false;
                }
        }
        return ret;
}

/**
 * Determine if LightDM is in use
 */
static bool gdm_is_used(void)
{
        static const char *gdm_binaries[] = { "/usr/sbin/gdm", "/usr/bin/gdm" };
        /* Search for gdm and if its accessible, use it */
        for (size_t i = 0; i < ARRAY_SIZE(gdm_binaries[0]); i++) {
                if (access(gdm_binaries[i], X_OK) == 0) {
                        return true;
                }
        }
        /* Not gdm */
        return false;
}

/**
 * LightDM implementation of the display manager
 */
const LdmDisplayManager gdm_display_manager = {
        .set_xrandr_output = gdm_set_xrandr_output,
        .remove_xrandr_output = gdm_remove_xrandr_output,
        .is_used = gdm_is_used,
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
