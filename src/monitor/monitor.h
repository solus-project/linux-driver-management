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

typedef struct LdmMonitor LdmMonitor;

/**
 * Construct a new LdmMonitor instance.
 *
 * @returns A newly allocated LdmMonitor
 */
LdmMonitor *ldm_monitor_new(void);

/**
 * Free a previously allocated LdmMonitor.
 *
 * @param monitor Pointer to an allocated LdmMonitor instance
 */
void ldm_monitor_free(LdmMonitor *monitor);

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
