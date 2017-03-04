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

#include <unistd.h>

#include "../display-manager.h"
#include "../util.h"

/**
 * noop
 */
static bool gdm_set_xrandr_output(__ldm_unused__ const char *driver,
                                  __ldm_unused__ const char *output)
{
        return false;
}

/**
 * noop
 */
static bool gdm_remove_xrandr_output(void)
{
        return false;
}

/**
 * Determine if LightDM is in use
 */
static bool gdm_is_used(void)
{
        /* lightdm also sets this but we check lightdm *before* gdm */
        if (!getenv("GDMSESSION")) {
                return false;
        }
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
