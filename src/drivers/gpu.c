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

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "device.h"
#include "ldm.h"
#include "pci.h"
#include "scanner.h"
#include "util.h"

/**
 * Map the enums to the real disk names
 */
static const char const *gl_driver_mapping[] = {
            [LDM_GL_NVIDIA] = "nvidia", [LDM_GL_AMD] = "amd", [LDM_GL_MESA] = "default",
};

LdmGLProvider ldm_pci_vendor_to_gl_provider(LdmPCIDevice *device)
{
        switch (device->vendor_id) {
        case PCI_VENDOR_ID_NVIDIA:
                return LDM_GL_NVIDIA;
        case PCI_VENDOR_ID_AMD:
                return LDM_GL_AMD;
        case PCI_VENDOR_ID_INTEL:
        default:
                return LDM_GL_MESA;
        }
}

static inline bool path_exists(const char *p)
{
        __attribute__((unused)) struct stat st = { 0 };
        return ((lstat(p, &st) == 0));
}

bool ldm_gl_provider_available(LdmGLProvider provider)
{
        return ldm_gl_provider_status(provider) != LDM_STATUS_UNINSTALLED;
}

/*
 * Basically we just iterate the mandatory paths and determine if we have a
 * full, partial, or missing installation.
 */
LdmInstallStatus ldm_gl_provider_status(LdmGLProvider provider_id)
{
        const char *provider = gl_driver_mapping[provider_id];
        autofree(char) *gl_dir = string_printf("%s/glx-provider/%s", LIBDIR, provider);
        if (!path_exists(gl_dir)) {
                return LDM_STATUS_UNINSTALLED;
        }
        LdmInstallStatus ret = LDM_STATUS_UNINSTALLED;

        /* libglx.so is not mandatory due to xorg / mesa separation */
        static const char *mandatory_links[] = {
                "libGL.so.1", "libEGL.so.1", "libGLESv1_CM.so.1", "libGLESv2.so.2",
        };

        for (size_t i = 0; i < ARRAY_SIZE(mandatory_links); i++) {
                const char *link = mandatory_links[i];
                autofree(char) *fullp =
                    string_printf("%s/glx-provider/%s/%s", LIBDIR, provider, link);
                if (!path_exists(fullp)) {
                        if (ret == LDM_STATUS_INSTALLED) {
                                return LDM_STATUS_CORRUPT;
                        }
                        ret = LDM_STATUS_UNINSTALLED;
                }
                ret = LDM_STATUS_INSTALLED;
        }
        return ret;
}

bool ldm_gl_provider_install(LdmGLProvider provider)
{
        fprintf(stderr, "Not yet implemented\n");
        return false;
}

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
        LdmGLProvider provider = ldm_pci_vendor_to_gl_provider(pci);

        /* Here we determine if we can use the proprietary driver or have to
         * fallback to mesa */
        if (ldm_gl_provider_available(provider)) {
                return ldm_gl_provider_install(provider);
        }

        /* Fallback to mesa, nothing we can do without the drivers */
        return ldm_gl_provider_install(LDM_GL_MESA);
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
static bool ldm_configure_gpu_amd_hybrid(__ldm_unused__ LdmDevice *igpu,
                                         __ldm_unused__ LdmDevice *dgpu)
{
        /* Currently we don't support AMD Hybrid graphics on Solus, as we don't
         * yet have the AMD proprietary driver installed. For now we'll just
         * ensure mesa is "OK"
         */
        return ldm_gl_provider_install(LDM_GL_MESA);
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
