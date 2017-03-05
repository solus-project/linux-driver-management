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

#include <pci/pci.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "device.h"
#include "pci.h"
#include "scanner.h"
#include "util.h"

/**
 * Return PCI id in format appropriate to X.Org (decimal, prefixed)
 */
static char *get_xorg_pci_id(LdmPCIAddress *addr)
{
        return string_printf("PCI:%d:%d:%d", addr->bus, addr->dev, addr->func);
}

/**
 * Demo code follows
 */
static void print_device(LdmDevice *device)
{
        autofree(char) *pci_id = NULL;

        if (device->type != LDM_DEVICE_PCI) {
                fprintf(stderr, "Ignoring unknown device with name '%s'\n", device->device_name);
                return;
        }
        LdmPCIDevice *dev = (LdmPCIDevice *)device;
        const char *vendor = NULL;
        bool boot_vga = false;
        bool gpu = (device->class & LDM_CLASS_GRAPHICS) == LDM_CLASS_GRAPHICS;
        if (gpu) {
                boot_vga = ldm_pci_device_is_boot_vga(dev);
        }

        switch (dev->vendor_id) {
        case PCI_VENDOR_ID_INTEL:
                vendor = "Intel";
                break;
        case PCI_VENDOR_ID_NVIDIA:
                vendor = "NVIDIA";
                break;
        case PCI_VENDOR_ID_AMD:
                vendor = "AMD";
                break;
        default:
                vendor = "<unknown>";
                break;
        }
        fprintf(stderr,
                " %02x:%02x.%x: %s\n",
                dev->address.bus,
                dev->address.dev,
                dev->address.func,
                device->device_name);
        fprintf(stderr, " \u251C Vendor ID     : %s\n", vendor);
        if (gpu) {
                fprintf(stderr, " \u251C Kernel driver : %s\n", device->driver);
                fprintf(stderr, " \u251C VGA Boot      : %s\n", boot_vga ? "yes" : "no");
                pci_id = get_xorg_pci_id(&(dev->address));
                fprintf(stderr, " \u2514 X.Org PCI ID  : %s\n", pci_id ? pci_id : "<unknown>");
        } else {
                fprintf(stderr, " \u2514 Kernel driver : %s\n", device->driver);
        }
        fputs("\n", stderr);
}

int ldm_cli_status(__ldm_unused__ int argc, __ldm_unused__ char **argv)
{
        autofree(LdmDevice) *device = NULL;
        LdmDevice *intel = NULL;

        device = ldm_scan_devices(LDM_DEVICE_PCI, LDM_CLASS_ANY);
        if (!device) {
                fputs("Unable to locate graphical PCI devices\n", stderr);
                return EXIT_FAILURE;
        }

        /* Iterate all discovered devices */
        for (LdmDevice *dev = device; dev; dev = dev->next) {
                print_device(dev);
        }

        /* Determine Optimus support */
        if ((intel = ldm_device_find_vendor(device, PCI_VENDOR_ID_INTEL)) != NULL &&
            ldm_device_find_vendor(device, PCI_VENDOR_ID_NVIDIA) != NULL) {
                if (ldm_pci_device_is_boot_vga((LdmPCIDevice *)intel)) {
                        fprintf(stderr, "*Technically* found Optimus. Probably didn't\n");
                }
        }

        return EXIT_SUCCESS;
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
