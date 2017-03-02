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

#include "../display-manager.h"

/**
 * noop
 */
static bool lightdm_set_xrandr_output(const char *driver, const char *output)
{
        return false;
}

/**
 * noop
 */
static bool lightdm_remove_xrandr_output(void)
{
        return false;
}

/**
 * noop
 */
static bool lightdm_is_used(void)
{
        return false;
}

/**
 * LightDM implementation of the display manager
 */
static LdmDisplayManager lightdm_display_manager = {
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
