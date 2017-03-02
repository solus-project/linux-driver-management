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

#pragma once

#include <stdbool.h>
#include <stdlib.h>

/**
 * DisplayManager encapsulates display-manager types
 * so that they can be configured for use by the drivers
 */
typedef struct LdmDisplayManager {
        /**
         * Set a configuration item within the DM config
         */
        bool (*set_config)(const char *section, const char *key, char *value);

        /**
         * Get a configuration value from the DM config. May return
         * NULL or an allocated string
         */
        char *(*get_config)(const char *section, const char *key);

        /**
         * Determine if the DM is actually used
         */
        bool (*is_used)(void);
} LdmDisplayManager;

/**
 * Return the LdmDisplayManager for the current system
 */
const LdmDisplayManager const *ldm_display_manager_get_default(void);

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
