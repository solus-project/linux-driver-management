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

#include <assert.h>
#include <fcntl.h>
#include <pci/pci.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ldm-private.h"
#include "pci.h"

typedef struct pci_access pci_access;

DEF_AUTOFREE(pci_access, pci_cleanup)

/**
 * Construct the base PCI sysfs path
 */
static char *ldm_pci_device_sysfs_path(struct pci_dev *dev)
{
        return string_printf("/sys/bus/pci/devices/%04x:%02x:%02x.%x",
                             dev->domain,
                             dev->bus,
                             dev->dev,
                             dev->func);
}

bool ldm_pci_device_is_boot_vga(LdmDevice *device)
{
        char c;
        bool vga = false;
        int fd = -1;
        autofree(char) *p = string_printf("%s/boot_vga", device->sysfs_address);

        fd = open(p, O_RDONLY | O_NOCTTY | O_CLOEXEC);
        if (fd < 0) {
                return false;
        }

        if (read(fd, &c, sizeof(c)) != 1) {
                goto clean;
        }
        if (c == '1') {
                vga = true;
        }
clean:
        close(fd);
        return vga;
}

/**
 * Convert the PCI class into a usable LDM class, i.e. GPU
 */
static unsigned int ldm_pci_to_device_class(struct pci_dev *dev)
{
        if (dev->device_class >= PCI_CLASS_DISPLAY_VGA &&
            dev->device_class <= PCI_CLASS_DISPLAY_3D) {
                return LDM_CLASS_GRAPHICS;
        }
        return 0;
}

/**
 * Construct a new LdmDevice for a PCI device
 */
static LdmDevice *ldm_pci_device_new(struct pci_dev *dev, char *name)
{
        LdmDevice *ret = NULL;
        LdmPCIAddress addr = {
                .domain = (uint16_t)dev->domain,
                .bus = dev->bus,
                .dev = dev->dev,
                .func = dev->func,
        };

        /* Handle PCI specific device construction */
        LdmPCIDevice *pci_dev = NULL;

        pci_dev = calloc(1, sizeof(LdmPCIDevice));
        if (!pci_dev) {
                goto oom_fail;
        }
        *pci_dev = (LdmPCIDevice){
                .address = addr,
                .vendor_id = dev->vendor_id,
                .device_id = dev->device_id,
        };
        ret = (LdmDevice *)pci_dev;

        /* Finish off the structure */
        ret->type = LDM_DEVICE_PCI;
        ret->sysfs_address = ldm_pci_device_sysfs_path(dev);
        ret->driver = ldm_device_driver(ret);
        if (name) {
                ret->device_name = strdup(name);
        }

        return ret;

oom_fail:
        fputs("Out of memory", stderr);
        abort();
        return NULL;
}

LdmDevice *ldm_scan_pci_devices(unsigned int classmask)
{
        autofree(pci_access) *ac = NULL;
        char buf[1024];
        char *nom = NULL;
        LdmDevice *root = NULL;
        LdmDevice *last = NULL;

        /* Init PCI lookup */
        ac = pci_alloc();
        if (!ac) {
                abort();
        }
        pci_init(ac);
        pci_scan_bus(ac);

        /* Iterate devices looking for something interesting. */
        for (struct pci_dev *dev = ac->devices; dev != NULL; dev = dev->next) {
                pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);
                LdmDeviceClass class = ldm_pci_to_device_class(dev);
                LdmDevice *device = NULL;

                /* Skip unrequested types */
                if ((class & classmask) != classmask) {
                        continue;
                }

                /* Skip dodgy devices */
                if (dev->vendor_id == 0 || dev->device_id == 0) {
                        continue;
                }

                nom = pci_lookup_name(ac,
                                      buf,
                                      sizeof(buf),
                                      PCI_LOOKUP_DEVICE,
                                      dev->vendor_id,
                                      dev->device_id);

                device = ldm_pci_device_new(dev, nom);
                if (!device) {
                        goto cleanup;
                }
                device->class = class;
                if (!root) {
                        root = device;
                }
                if (last) {
                        last->next = device;
                }
                last = device;
        }

cleanup:
        return root;
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
