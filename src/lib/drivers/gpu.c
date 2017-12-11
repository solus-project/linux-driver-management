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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "device.h"
#include "dm/display-manager.h"
#include "ldm.h"
#include "pci.h"
#include "scanner.h"
#include "util.h"

/**
 * TODO: Consider an /etc/X11/xorg.conf.d/ path..
 */
#define XORG_CONFIG "/etc/X11/xorg.conf"

/**
 * Return the PCI ID on xorg's format
 */
static char *ldm_gpu_get_xorg_pci_id(LdmPCIAddress *addr)
{
        return string_printf("%d:%d:%d", addr->bus, addr->dev, addr->func);
}

/**
 * Map the enums to the real disk names
 */
static const char *gl_driver_mapping[] = {
        [LDM_GL_NVIDIA] = "nvidia",
        [LDM_GL_AMD] = "amd",
        [LDM_GL_MESA] = "default",
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
                "libGL.so.1",
                "libEGL.so.1",
                "libGLESv1_CM.so.1",
                "libGLESv2.so.2",
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

bool ldm_gl_provider_install(LdmGLProvider provider_id)
{
        const char *provider = gl_driver_mapping[provider_id];

        char *libdirs[] = {
                LIBDIR,
#ifdef WITH_EMUL32
                EMUL32LIBDIR,
#endif
        };

        /* Full paths */
        static const char *paths[] = { "libGL.so.1",
                                       "libEGL.so.1",
                                       "libGLESv1_CM.so.1",
                                       "libGLESv2.so.2",
                                       "libglx.so.1" };

        for (size_t j = 0; j < ARRAY_SIZE(libdirs); j++) {
                const char *dir = libdirs[j];
                if (!dir) {
                        continue;
                }

                for (size_t i = 0; i < ARRAY_SIZE(paths); i++) {
                        const char *current = paths[i];
                        autofree(char) *target_path = NULL;
                        autofree(char) *target_dir = NULL;
                        autofree(char) *source_path =
                            string_printf("%s/glx-provider/%s/%s", dir, provider, current);
                        autofree(char) *source_resolved = NULL;

                        /* Non-fatal due to a chicken-and-egg situation with xorg-server's libglx */
                        if (!path_exists(source_path)) {
                                continue;
                        }

                        /* Must still try to continue */
                        source_resolved = realpath(source_path, NULL);
                        if (!source_resolved) {
                                fprintf(stderr, "Cannot read link: %s\n", strerror(errno));
                                continue;
                        }

                        /* xorg module is handled differently */
                        if (streq(current, "libglx.so.1")) {
                                target_dir = string_printf("%s/xorg/modules/extensions", dir);
                                target_path = string_printf("%s/libglx.so", target_dir);
                        } else {
                                target_path = string_printf("%s/%s", dir, current);
                        }

                        /* Remove existing symlink first */
                        if (path_exists(target_path) && unlink(target_path) < 0) {
                                fprintf(stderr,
                                        "Unable to remove %s: %s\n",
                                        target_path,
                                        strerror(errno));
                                return false;
                        }

                        if (target_dir && !path_exists(target_dir) && !mkdir_p(target_dir, 00755)) {
                                fprintf(stderr,
                                        "Unable to create required directory %s: %s\n",
                                        target_dir,
                                        strerror(errno));
                                return false;
                        }

                        if (symlink(source_resolved, target_path) != 0) {
                                fprintf(stderr,
                                        "Unable to link %s -> %s: %s\n",
                                        source_resolved,
                                        target_path,
                                        strerror(errno));
                                return false;
                        }
                }
        }

        return true;
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
 * Perform a simple configure on NVIDIA
 */
static inline bool ldm_configure_gpu_xorg_nvidia(void)
{
        if (system("nvidia-xconfig") != 0) {
                fprintf(stderr, "Error invoking nvidia-xconfig: %s\n", strerror(errno));
                return false;
        }
        return true;
}

/**
 * Try to remove any previous xorg configs (non-fatal mostly.)
 */
static inline void ldm_configure_gpu_nuke_xorg(void)
{
        if (!path_exists(XORG_CONFIG)) {
                return;
        }
        if (unlink(XORG_CONFIG) < 0) {
                fprintf(stderr, "Error removing xorg config: %s\n", strerror(errno));
        }
}

/**
 * Simple GPU configuration.
 */
static bool ldm_configure_gpu_simple(LdmDevice *device)
{
        LdmPCIDevice *pci = NULL;
        LdmGLProvider provider = LDM_GL_MESA;

        /* No GPU device so just hook up stock mesa */
        if (!device) {
                return ldm_gl_provider_install(LDM_GL_MESA);
        }

        pci = (LdmPCIDevice *)device;
        provider = ldm_pci_vendor_to_gl_provider(pci);

        /* Here we determine if we can use the proprietary driver or have to
         * fallback to mesa */
        if (ldm_gl_provider_available(provider)) {
                if (!ldm_gl_provider_install(provider)) {
                        return false;
                }
                /* Now write the xorg config. */
                switch (provider) {
                case LDM_GL_NVIDIA:
                        return ldm_configure_gpu_xorg_nvidia();
                default:
                        break;
                }
        }

        /* Proprietary driver no longer available, make sure to remove xorg config */
        if (provider != LDM_GL_MESA) {
                ldm_configure_gpu_nuke_xorg();
        }

        /* Fallback to mesa, nothing we can do without the drivers */
        return ldm_gl_provider_install(LDM_GL_MESA);
}

/**
 * Encountered Optimus GPU configuration
 */
static bool ldm_configure_gpu_optimus(__ldm_unused__ LdmDevice *igpu, LdmDevice *dgpu)
{
        const LdmDisplayManager *dm = ldm_display_manager_get_default();
        autofree(FILE) *xorg_config = NULL;
        autofree(char) *pci_id = NULL;
        LdmPCIDevice *secondary = (LdmPCIDevice *)dgpu;

        /* No proprietary drivers available */
        if (!ldm_gl_provider_available(LDM_GL_NVIDIA)) {
                if (dm) {
                        dm->remove_xrandr_output();
                }
                ldm_configure_gpu_nuke_xorg();
                return ldm_gl_provider_install(LDM_GL_MESA);
        }

        /* Attempt to set up the links for nvidia */
        if (!ldm_gl_provider_install(LDM_GL_NVIDIA)) {
                ldm_configure_gpu_nuke_xorg();
                return ldm_gl_provider_install(LDM_GL_MESA);
        }

        /* Set up the diplay manager configuration files */
        if (dm) {
                if (!dm->set_xrandr_output("modesetting", "NVIDIA-0")) {
                        goto fail;
                }
        }

        /* Create an xorg config file */
        xorg_config = fopen(XORG_CONFIG, "w+");
        if (!xorg_config) {
                goto fail;
        }

        pci_id = ldm_gpu_get_xorg_pci_id(&(secondary->address));

        /* Set modesetting by default, and enable the nvidia driver only on the
         * nvidia PCI ID
         * Special thanks to the Arch Wiki:
         * https://wiki.archlinux.org/index.php/NVIDIA_Optimus#Using_nvidia
         */
        if (fprintf(xorg_config,
                    "Section \"Module\"\n"
                    "    Load \"modesetting\"\n"
                    "EndSection\n\n"
                    "Section \"Device\"\n"
                    "    Identifier \"nvidia\"\n"
                    "    Driver \"nvidia\"\n"
                    "    BusID \"%s\"\n"
                    "    Option \"AllowEmptyInitialConfiguration\"\n"
                    "EndSection\n",
                    pci_id) < 0) {
                goto fail;
        }

        return true;
fail:
        fprintf(stderr, "Failed to configure optimus: %s\n", strerror(errno));
        ldm_configure_gpu_nuke_xorg();
        return ldm_gl_provider_install(LDM_GL_MESA);
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
                /* We may be in a chroot or image build */
                fputs("Cannot find a usable GPU on this system\n", stderr);
                return ldm_configure_gpu_simple(NULL);
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
