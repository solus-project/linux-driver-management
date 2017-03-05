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

#include <stdio.h>

#include "device.h"
#include "ldm.h"
#include "pci.h"
#include "scanner.h"
#include "util.h"

LdmGPUConfig *ldm_gpu_config_new(LdmDevice *devices)
{
        LdmDevice *non_boot_vga = NULL;
        LdmDevice *boot_vga = NULL;
        uint16_t boot_vendor_id;
        uint16_t non_boot_vendor_id;
        LdmGPUConfig *ret = NULL;

        if (!devices) {
                return NULL;
        }

        if (devices->type != LDM_DEVICE_PCI) {
                return NULL;
        }

        if ((devices->class & LDM_CLASS_GRAPHICS) != LDM_CLASS_GRAPHICS) {
                return NULL;
        }

        /* Must allocate. */
        ret = calloc(1, sizeof(LdmGPUConfig));
        if (!ret) {
                abort();
        }

        /* Assume initially we have a simple GPU config */
        ret->type = LDM_GPU_SIMPLE;
        ret->primary = devices;

        /* Trivial configuration */
        if (ldm_device_n_devices(devices) == 1) {
                return ret;
        }

        /* Find the boot_vga and the non_boot_vga */
        for (LdmDevice *dev = devices; dev; dev = dev->next) {
                if (dev->type != LDM_DEVICE_PCI) {
                        continue;
                }
                if ((dev->class & LDM_CLASS_GRAPHICS) != LDM_CLASS_GRAPHICS) {
                        continue;
                }
                if (ldm_pci_device_is_boot_vga((LdmPCIDevice *)dev)) {
                        boot_vga = dev;
                } else {
                        non_boot_vga = dev;
                }
        }

        /* If somehow we fail to get boot_vga (highly unlikely, make it the first one we encounter
         */
        if (!boot_vga) {
                boot_vga = devices;
        }

        /* Boot VGA is now primary in multiple GPU configuration */
        ret->primary = boot_vga;

        boot_vendor_id = ((LdmPCIDevice *)boot_vga)->vendor_id;

        /* SLI/Crossfire most likely. Configure the first boot_vga */
        if (!non_boot_vga) {
                switch (boot_vendor_id) {
                case PCI_VENDOR_ID_AMD:
                        ret->type = LDM_GPU_CROSSFIRE;
                        break;
                case PCI_VENDOR_ID_NVIDIA:
                        ret->type = LDM_GPU_SLI;
                        break;
                default:
                        break;
                }
                return ret;
        }

        non_boot_vendor_id = ((LdmPCIDevice *)non_boot_vga)->vendor_id;

        /* Detect hybrid GPU configurations */
        switch (non_boot_vendor_id) {
        case PCI_VENDOR_ID_AMD:
                if (boot_vendor_id == PCI_VENDOR_ID_INTEL || boot_vendor_id == PCI_VENDOR_ID_AMD) {
                        ret->type = LDM_GPU_AMD_HYBRID;
                        ret->secondary = non_boot_vga;
                        return ret;
                }
                break;
        case PCI_VENDOR_ID_NVIDIA:
                if (boot_vendor_id == PCI_VENDOR_ID_INTEL) {
                        ret->type = LDM_GPU_OPTIMUS;
                        ret->secondary = non_boot_vga;
                        return ret;
                }
                break;
        default:
                break;
        }

        /* Restort to simple configure on the boot_vga */
        ret->primary = boot_vga;
        return ret;
}

void ldm_gpu_config_free(LdmGPUConfig *self)
{
        if (!self) {
                return;
        }
        free(self);
}

/**
 * Simple GPU configuration.
 */
static bool ldm_configure_gpu_simple(LdmDevice *device)
{
        LdmPCIDevice *pci = (LdmPCIDevice *)device;
        fprintf(stderr, "Simple configure: %s\n", device->device_name);
        switch (pci->vendor_id) {
        case PCI_VENDOR_ID_AMD:
                fputs(" - AMD GPU\n", stderr);
                break;
        case PCI_VENDOR_ID_INTEL:
                fputs(" - Intel GPU\n", stderr);
                break;
        case PCI_VENDOR_ID_NVIDIA:
                fputs(" - NVIDIA GPU\n", stderr);
                break;
        default:
                fputs(" - Unknown GPU vendor\n", stderr);
                break;
        }
        fputs("Not yet implemented\n", stderr);
        return false;
}

/**
 * Encountered Optimus GPU configuration
 */
static bool ldm_configure_gpu_optimus(LdmDevice *igpu, LdmDevice *dgpu)
{
        fprintf(stderr, "Optimus: %s (IGPU) | %s (DGPU) \n", igpu->device_name, dgpu->device_name);
        fputs("Not yet implemented\n", stderr);
        return false;
}

/**
 * Configure AMD hybrid GPU situation
 */
static bool ldm_configure_gpu_amd_hybrid(LdmDevice *igpu, LdmDevice *dgpu)
{
        fprintf(stderr,
                "AMD Hybrid: %s (IGPU) | %s (DGPU)\n",
                igpu->device_name,
                dgpu->device_name);
        return false;
}

/**
 * Configure the system GPU
 */
bool ldm_configure_gpu(void)
{
        autofree(LdmDevice) *devices = NULL;
        autofree(LdmGPUConfig) *config = NULL;

        /* Find the usable GPUs first */
        devices = ldm_scan_devices(LDM_DEVICE_PCI, LDM_CLASS_GRAPHICS);
        if (!devices) {
                fputs("Cannot find a usable GPU on this system\n", stderr);
                return false;
        }

        config = ldm_gpu_config_new(devices);
        switch (config->type) {
        case LDM_GPU_AMD_HYBRID:
                return ldm_configure_gpu_amd_hybrid(config->primary, config->secondary);
        case LDM_GPU_OPTIMUS:
                return ldm_configure_gpu_optimus(config->primary, config->secondary);
        case LDM_GPU_SIMPLE:
        default:
                return ldm_configure_gpu_simple(config->primary);
        }
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
