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

#include "display-manager.h"

extern const LdmDisplayManager lightdm_display_manager;

/**
 * Valid DM implementations.
 */
static const LdmDisplayManager *_managers[] = {
        &lightdm_display_manager, /** LightDM must come before GDM */
};

/**
 * Return the LdmDisplayManager for the current system
 */
const LdmDisplayManager const *ldm_display_manager_get_default(void)
{
        for (size_t i = 0; i < sizeof(_managers) / sizeof(_managers[0]); i++) {
                const LdmDisplayManager *manager = _managers[i];
                if (manager->is_used()) {
                        return manager;
                }
        }
        return NULL;
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
