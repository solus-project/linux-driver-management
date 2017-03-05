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
#include "ldm.h"
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

static void print_device(LdmDevice *device, const char *label)
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
                " \u251C %02x:%02x.%x: %s (%s)\n",
                dev->address.bus,
                dev->address.dev,
                dev->address.func,
                device->device_name,
                label);
        fprintf(stderr, " \u251C Vendor ID     : %s\n", vendor);
        if (gpu) {
                fprintf(stderr, " \u251C Kernel driver : %s\n", device->driver);
                fprintf(stderr, " \u251C VGA Boot      : %s\n", boot_vga ? "yes" : "no");
                pci_id = get_xorg_pci_id(&(dev->address));
                fprintf(stderr, " \u2514 X.Org PCI ID  : %s\n", pci_id ? pci_id : "<unknown>");
        } else {
                fprintf(stderr, " \u2514 Kernel driver : %s\n", device->driver);
        }
}

void print_gpu_configuration(LdmGPUConfig *config)
{
        switch (config->type) {
        case LDM_GPU_AMD_HYBRID:
                fputs("AMD Hybrid\n", stderr);
                break;
        case LDM_GPU_OPTIMUS:
                fputs("NVIDIA Optimus\n", stderr);
                break;
        case LDM_GPU_CROSSFIRE:
                fputs("AMD Crossfire\n", stderr);
                break;
        case LDM_GPU_SLI:
                fputs("NVIDIA SLI\n", stderr);
        case LDM_GPU_SIMPLE:
        default:
                fputs("Simple GPU Configuration\n", stderr);
                break;
        }

        if (config->primary) {
                print_device(config->primary, "Primary");
        }
        if (config->secondary) {
                fputs(" \u251F Secondary GPU\n", stderr);
                print_device(config->secondary, "Secondary");
        }
}

int ldm_cli_status(__ldm_unused__ int argc, __ldm_unused__ char **argv)
{
        autofree(LdmDevice) *device = NULL;
        autofree(LdmGPUConfig) *gpu = NULL;

        /* TEMPORARY: Only show GPU */
        device = ldm_scan_devices(LDM_DEVICE_PCI, LDM_CLASS_GRAPHICS);
        if (!device) {
                fputs("Unable to locate graphical PCI devices\n", stderr);
                return EXIT_FAILURE;
        }

        gpu = ldm_gpu_config_new(device);
        print_gpu_configuration(gpu);

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
