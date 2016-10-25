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

#define _GNU_SOURCE

#include <pciaccess.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "atomics.h"
#include "ldm.h"

struct LdmManager {
        ldm_atomic_t atom;
};

#define PCI_VENDOR_ID_INTEL 0x8086
#define PCI_VENDOR_ID_NVIDIA 0x10DE
#define PCI_VENDOR_ID_AMD 0x1002

/** VGA device */
#define PCI_CLASS_DISPLAY_VGA 0x0300

/** 3D Controller */
#define PCI_CLASS_DISPLAY_3D 0x302

static int _pci_status = 0;

__ldm_inline__ static inline bool is_pci_available(void)
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

char *ldm_manager_get_xorg_pci_id(struct pci_device *device)
{
        char *p = NULL;
        int ret = asprintf(&p, "PCI:%d:%d:%d", device->domain, device->dev, device->func);
        if (ret < 0) {
                return NULL;
        }
        return p;
}

__ldm_public__ bool ldm_manager_scan(__ldm_unused__ LdmManager *manager)
{
        if (!is_pci_available()) {
                return false;
        }
        struct pci_device_iterator *devices = NULL;
        struct pci_device *device = NULL;
        struct pci_slot_match match = {.domain = PCI_MATCH_ANY,
                                       .bus = PCI_MATCH_ANY,
                                       .dev = PCI_MATCH_ANY,
                                       .func = PCI_MATCH_ANY };

        devices = pci_slot_match_iterator_create(&match);
        if (!devices) {
                return false;
        }

        while ((device = pci_device_next(devices)) != NULL) {
                if (device->device_class != PCI_CLASS_DISPLAY_VGA << 8 &&
                    device->device_class != PCI_CLASS_DISPLAY_3D << 8) {
                        continue;
                }
                fprintf(stderr, "Have VGA device: %#x %#x\n", device->vendor_id, device->device_id);
                switch (device->vendor_id) {
                case PCI_VENDOR_ID_INTEL:
                        fputs(" -> Intel device\n", stderr);
                        break;
                case PCI_VENDOR_ID_NVIDIA:
                        fputs(" -> NVIDIA device\n", stderr);
                        break;
                case PCI_VENDOR_ID_AMD:
                        fputs(" -> AMD device\n", stderr);
                        break;
                }
                autofree(char) *pci_id = ldm_manager_get_xorg_pci_id(device);
                if (pci_device_is_boot_vga(device)) {
                        fprintf(stderr, " -> Is boot VGA: %#x\n", device->device_class);
                }
                fprintf(stderr, "X.Org PCI ID: %s\n", pci_id ? pci_id : "<unknown>");
        }

        pci_iterator_destroy(devices);

        return true;
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
