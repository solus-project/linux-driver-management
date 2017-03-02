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
static bool lightdm_set_config(const char *section, const char *key, char *value)
{
        return false;
}

/**
 * noop
 */
static char *lightdm_get_config(const char *section, const char *key)
{
        return NULL;
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
        .set_config = lightdm_set_config,
        .get_config = lightdm_get_config,
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
