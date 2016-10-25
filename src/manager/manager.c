/*
 * This file is part of linux-driver-management.
 *
 * Copyright © 2016 Ikey Doherty
 *
 * linux-driver-management is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#include <pciaccess.h>
#include <stdbool.h>
#include <stdlib.h>

#include "atomics.h"
#include "ldm.h"

struct LdmManager {
        ldm_atomic_t atom;
};

static int _pci_status = 0;

static inline bool is_pci_available(void)
{
        return _pci_status == 0;
}

__attribute__((constructor)) static void _ldm_init(void)
{
        _pci_status = pci_system_init();
}

__attribute__((destructor)) static void _ldm_deinit(void)
{
        if (is_pci_available()) {
                pci_system_cleanup();
        }
}

static inline void ldm_manager_destroy(LdmManager *manager)
{
        if (!manager) {
                return;
        }
        free(manager);
}

__ldm_public__ LdmManager *ldm_manager_new(void)
{
        LdmManager *ret = NULL;

        ret = calloc(1, sizeof(LdmManager));
        if (!ret) {
                return NULL;
        }
        return ldm_atomic_init((ldm_atomic_t *)ret, (ldm_atomic_free)ldm_manager_destroy);
}

__ldm_public__ void ldm_manager_free(LdmManager *manager)
{
        ldm_atomic_unref(manager);
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
