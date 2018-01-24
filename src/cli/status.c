/*
 * This file is part of linux-driver-management.
 *
 * Copyright © 2016-2018 Ikey Doherty
 *
 * linux-driver-management is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#include "cli.h"
#include "config.h"
#include "ldm.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>

static void print_drivers(LdmManager *manager, LdmDevice *device)
{
        g_autoptr(GPtrArray) providers = NULL;

        /* Look for provider options */
        providers = ldm_manager_get_providers(manager, device);
        if (providers->len < 1) {
                return;
        }

        fprintf(stdout,
                "\nLDM Providers for %s: %d\n",
                ldm_device_get_name(device),
                providers->len);

        for (guint i = 0; i < providers->len; i++) {
                LdmProvider *provider = providers->pdata[i];
                const gchar *name = NULL;

                name = ldm_provider_get_package(provider);
                fprintf(stdout, " -  %s\n", name);
        }
}
/**
 * Handle pretty printing of a single device to the display
 */
static void print_device(LdmDevice *device)
{
        gboolean gpu = FALSE;
        /* PCI specific */
        guint bus = 0, dev = 0;
        gint func = 0;

        gpu = ldm_device_has_type(device, LDM_DEVICE_TYPE_GPU);

        /* Pretty strings */
        fprintf(stdout, " \u255E Device Name   : %s\n", ldm_device_get_name(device));
        fprintf(stdout, " \u255E Manufacturer  : %s\n", ldm_device_get_vendor(device));

        /* Ids */
        fprintf(stdout, " \u255E Product ID    : 0x%04x\n", ldm_device_get_product_id(device));
        fprintf(stdout,
                " %s Vendor ID     : 0x%04x\n",
                gpu ? "\u255E" : "\u2558",
                ldm_device_get_vendor_id(device));

        if (!gpu) {
                return;
        }

        if (ldm_device_has_type(device, LDM_DEVICE_TYPE_PCI)) {
                LdmPCIDevice *pci = LDM_PCI_DEVICE(device);
                ldm_pci_device_get_address(pci, &bus, &dev, &func);
                /* X.Org Address is decimal, not hex */
                fprintf(stdout, " \u255E X.Org PCI ID  : PCI:%u:%u:%d\n", bus, dev, func);
        }

        /* GPU Specifics */
        fprintf(stdout,
                " \u2558 Boot VGA      : %s\n",
                ldm_device_has_attribute(device, LDM_DEVICE_ATTRIBUTE_BOOT_VGA) ? "yes" : "no");
}

/**
 * Handle pretty printing of the GPU configuration to the display
 */
static void print_gpu_config(LdmManager *manager, LdmGPUConfig *config)
{
        LdmDevice *primary = NULL, *secondary = NULL;

        primary = ldm_gpu_config_get_primary_device(config);
        secondary = ldm_gpu_config_get_secondary_device(config);

        if (ldm_gpu_config_has_type(config, LDM_GPU_TYPE_OPTIMUS)) {
                fputs("\nNVIDIA Optimus\n", stdout);
        } else if (ldm_gpu_config_has_type(config, LDM_GPU_TYPE_HYBRID)) {
                fputs("\nHybrid Graphics\n", stdout);
        } else if (ldm_gpu_config_has_type(config, LDM_GPU_TYPE_CROSSFIRE)) {
                fputs("\nAMD Crossfire\n", stdout);
        } else if (ldm_gpu_config_has_type(config, LDM_GPU_TYPE_SLI)) {
                fputs("\nNVIDIA SLI\n", stdout);
        } else if (ldm_gpu_config_has_type(config, LDM_GPU_TYPE_COMPOSITE)) {
                fputs("\nComposite GPU\n", stdout);
        } else {
                fputs("\nSimple GPU configuration\n", stdout);
        }

        fputs("\n", stdout);

        /* We're only concerned with primary vs secondary devices */
        fprintf(stdout,
                " \u2552 Primary GPU%s\n",
                ldm_gpu_config_has_type(config, LDM_GPU_TYPE_HYBRID) ? " (iGPU)" : "");
        print_device(primary);

        if (!secondary) {
                goto emit_gpu_drivers;
        }

        fprintf(stdout,
                "\n \u2552 Secondary GPU%s\n",
                ldm_gpu_config_has_type(config, LDM_GPU_TYPE_HYBRID) ? " (dGPU)" : "");

        print_device(secondary);

emit_gpu_drivers:

        /* Only emit the drivers for the primary detection device */
        print_drivers(manager, ldm_gpu_config_get_detection_device(config));
}

/**
 * Handle pretty printing of the core DMI platform device.
 */
static void print_platform_device(LdmDevice *device)
{
        fprintf(stdout, " \u2552 %s\n", "Hardware Platform");
        fprintf(stdout, " \u255E %s : %s\n", "Platform Vendor", ldm_device_get_vendor(device));
        fprintf(stdout, " \u2558 %s  : %s\n", "Platform Model", ldm_device_get_name(device));
        /* TODO: Add chassis */
}

/**
 * Handle pretty printing of the remaining devices.
 */
static void print_non_gpu(LdmManager *manager, LdmDevice *device)
{
        const gchar *device_title = NULL;
        g_autoptr(GPtrArray) providers = NULL;

        /* We've already handled GPU devices in a special fashion */
        if (ldm_device_has_type(device, LDM_DEVICE_TYPE_GPU)) {
                return;
        }

        if (ldm_device_has_type(device, LDM_DEVICE_TYPE_PLATFORM)) {
                print_platform_device(device);
                return;
        }

        /* Only emit actionable items here */
        providers = ldm_manager_get_providers(manager, device);
        if (providers->len < 1) {
                return;
        }

        /* Try to ascertain the primary role */
        if (ldm_device_has_type(device, LDM_DEVICE_TYPE_AUDIO)) {
                device_title = "Audio Device";
        } else if (ldm_device_has_type(device, LDM_DEVICE_TYPE_HID)) {
                device_title = "HID Device";
        } else if (ldm_device_has_type(device, LDM_DEVICE_TYPE_IMAGE)) {
                device_title = "Image Device";
        } else if (ldm_device_has_type(device, LDM_DEVICE_TYPE_PRINTER)) {
                device_title = "Printer";
        } else if (ldm_device_has_type(device, LDM_DEVICE_TYPE_STORAGE)) {
                device_title = "Storage Device";
        } else if (ldm_device_has_type(device, LDM_DEVICE_TYPE_VIDEO)) {
                device_title = "Video Device";
        } else if (ldm_device_has_type(device, LDM_DEVICE_TYPE_WIRELESS)) {
                device_title = "Wireless Device";
        } else if (ldm_device_has_type(device, LDM_DEVICE_TYPE_PCI)) {
                device_title = "PCI Device";
        } else if (ldm_device_has_type(device, LDM_DEVICE_TYPE_USB)) {
                device_title = "USB Device";
        } else {
                device_title = "Device";
        }

        fprintf(stdout, " \u2552 %s\n", device_title);
        print_device(device);

        for (guint i = 0; i < providers->len; i++) {
                LdmProvider *provider = providers->pdata[i];
                fprintf(stdout,
                        "  \u2558 Provider %02u   : %s\n",
                        i + 1,
                        ldm_provider_get_package(provider));
        }

        fputs("\n", stdout);
}

int ldm_cli_status(__ldm_unused__ int argc, __ldm_unused__ char **argv)
{
        g_autoptr(LdmManager) manager = NULL;
        g_autoptr(LdmGPUConfig) gpu_config = NULL;
        g_autoptr(GPtrArray) devices = NULL;

        /* No need for hot plug events */
        manager = ldm_manager_new(LDM_MANAGER_FLAGS_NO_MONITOR);
        if (!manager) {
                fprintf(stderr, "Failed to initialiase LdmManager\n");
                return EXIT_FAILURE;
        }

        /* Add system modalias plugins - not fatal really. */
        if (!ldm_manager_add_system_modalias_plugins(manager)) {
                fprintf(stderr, "Failed to find any system modalias plugins\n");
        }

        gpu_config = ldm_gpu_config_new(manager);
        if (!gpu_config) {
                fprintf(stderr, "Failed to obtain LdmGPUConfig\n");
                return EXIT_FAILURE;
        }

        /* Emit non GPU items here, platform first */
        devices = ldm_manager_get_devices(manager, LDM_DEVICE_TYPE_ANY);
        for (guint i = 0; i < devices->len; i++) {
                print_non_gpu(manager, devices->pdata[i]);
        }

        /* Emit GPU config last for consistency */
        print_gpu_config(manager, gpu_config);

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
