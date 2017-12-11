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

#include <gudev/gudev.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ldm.h"
#include "monitor.h"
#include "scanner.h"
#include "util.h"

static bool ldm_monitor_init(LdmMonitor *self);
static void ldm_monitor_uevent(LdmMonitor *self, const gchar *action, GUdevDevice *device,
                               GUdevClient *client);
static void ldm_monitor_udev_add(LdmMonitor *self, GUdevDevice *device);
static void ldm_monitor_udev_remove(LdmMonitor *self, GUdevDevice *device);
static bool ldm_monitor_refresh_gpu(LdmMonitor *self);
static bool ldm_monitor_refresh_usb(LdmMonitor *self);

struct LdmMonitor {
        GUdevClient *udev_client; /**<Connection to udev */
        LdmDevice *gpu_list;      /**<Known GPUs */
        LdmDevice *usb_list;      /**<Known USB devices */
        LdmGPUConfig *gpu_config; /**<Condensed GPU configuration */
};

LdmMonitor *ldm_monitor_new(void)
{
        LdmMonitor *ret = NULL;

        ret = calloc(1, sizeof(LdmMonitor));
        if (!ret) {
                return NULL;
        }

        if (!ldm_monitor_init(ret)) {
                ldm_monitor_free(ret);
                return NULL;
        }

        return ret;
}

/**
 * Attempt to initialise the udev client and systems
 */
static bool ldm_monitor_init(LdmMonitor *self)
{
        /* The subsystems we care about */
        static const gchar *subsystems[] = {
                "usb/usb_interface",
                NULL,
        };

        self->udev_client = g_udev_client_new(subsystems);
        if (!self->udev_client) {
                return false;
        }

        g_signal_connect_swapped(self->udev_client, "uevent", G_CALLBACK(ldm_monitor_uevent), self);

        if (!ldm_monitor_refresh_gpu(self)) {
                g_warning("GPU configuration is unknown");
        }
        if (!ldm_monitor_refresh_usb(self)) {
                g_warning("USB configuration is unknown");
        }

        g_message("Early probe complete");

        return true;
}

void ldm_monitor_free(LdmMonitor *self)
{
        g_clear_object(&self->udev_client);
        g_clear_pointer(&self->gpu_config, ldm_gpu_config_free);
        g_clear_pointer(&self->gpu_list, ldm_device_free);
        g_clear_pointer(&self->usb_list, ldm_device_free);
        free(self);
}

/**
 * Begin processing a uevent - udev has some hardware change for us.
 */
static void ldm_monitor_uevent(LdmMonitor *self, const gchar *action, GUdevDevice *device,
                               __ldm_unused__ GUdevClient *client)
{
        if (g_str_equal(action, "add")) {
                ldm_monitor_udev_add(self, device);
        } else if (g_str_equal(action, "remove")) {
                ldm_monitor_udev_remove(self, device);
        }
}

static void ldm_monitor_udev_add(LdmMonitor *self, GUdevDevice *device)
{
        g_message("uevent(add): %s", g_udev_device_get_sysfs_path(device));
}

static void ldm_monitor_udev_remove(LdmMonitor *self, GUdevDevice *device)
{
        g_message("uevent(remove): %s", g_udev_device_get_sysfs_path(device));
}

/**
 * Attempt to learn some details about the GPU configuration.
 *
 * This allows us to learn some early PCI devices
 */
static bool ldm_monitor_refresh_gpu(LdmMonitor *self)
{
        g_clear_pointer(&self->gpu_config, ldm_gpu_config_free);
        g_clear_pointer(&self->gpu_list, ldm_device_free);

        self->gpu_list = ldm_scan_devices(LDM_DEVICE_PCI, LDM_CLASS_GRAPHICS);
        if (!self->gpu_list) {
                return false;
        }
        self->gpu_config = ldm_gpu_config_new(self->gpu_list);
        if (!self->gpu_config) {
                return false;
        }

        if (self->gpu_config->type == LDM_GPU_OPTIMUS) {
                g_message("debug: discovered Optimus system");
        } else {
                g_message("debug: discovered simple GPU config");
        }

        return true;
}
/**
 *
 * Attempt to learn some details about the USB configuration.
 */
static bool ldm_monitor_refresh_usb(LdmMonitor *self)
{
        g_clear_pointer(&self->usb_list, ldm_device_free);

        self->usb_list = ldm_scan_devices(LDM_DEVICE_USB, LDM_CLASS_ANY);
        if (!self->usb_list) {
                return false;
        }

        for (LdmDevice *device = self->usb_list; device; device = device->next) {
                fprintf(stderr, "USB Device: %s\n", device->device_name);
        }

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
