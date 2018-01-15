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

#define _GNU_SOURCE

#include "monitor.h"
#include "config.h"
#include "ldm.h"
#include "util.h"

static void ldm_daemon_device_added(LdmDaemon *daemon, LdmDevice *device, gpointer v);
static void ldm_daemon_device_removed(LdmDaemon *daemon, const gchar *path, gpointer v);
static void ldm_daemon_discover_existing(LdmDaemon *daemon);
static void ldm_daemon_discover_gpu(LdmDaemon *daemon);
static void ldm_daemon_discover_drivers(LdmDaemon *daemon, LdmDevice *device);

struct _LdmDaemonClass {
        GObjectClass parent_class;
};

struct _LdmDaemon {
        GObject parent;

        LdmManager *manager;
};

G_DEFINE_TYPE(LdmDaemon, ldm_daemon, G_TYPE_OBJECT)

/**
 * ldm_daemon_dispose:
 *
 * Clean up a LdmDaemon instance
 */
static void ldm_daemon_dispose(GObject *obj)
{
        LdmDaemon *self = LDM_DAEMON(obj);

        g_clear_object(&self->manager);

        G_OBJECT_CLASS(ldm_daemon_parent_class)->dispose(obj);
}

/**
 * ldm_daemon_class_init:
 *
 * Handle class initialisation
 */
static void ldm_daemon_class_init(LdmDaemonClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_daemon_dispose;
}

/**
 * ldm_daemon_init:
 *
 * Handle construction of the LdmDaemon
 */
static void ldm_daemon_init(LdmDaemon *self)
{
        self->manager = ldm_manager_new(LDM_MANAGER_FLAGS_NONE);
        /* Ensure we have some plugins */
        ldm_manager_add_modalias_plugins_for_directory(self->manager, MODALIAS_DIR);

        /* Hook up hot plugging */
        g_signal_connect_swapped(self->manager,
                                 "device-added",
                                 G_CALLBACK(ldm_daemon_device_added),
                                 self);
        g_signal_connect_swapped(self->manager,
                                 "device-removed",
                                 G_CALLBACK(ldm_daemon_device_removed),
                                 self);

        ldm_daemon_discover_existing(self);
        ldm_daemon_discover_gpu(self);
}

/**
 * ldm_daemon_new:
 *
 * Returns: (transfer full): A new #LdmDaemon instance
 */
LdmDaemon *ldm_daemon_new()
{
        return g_object_new(LDM_TYPE_DAEMON, NULL);
}

static void ldm_daemon_device_added(__ldm_unused__ LdmDaemon *daemon, LdmDevice *device,
                                    __ldm_unused__ gpointer v)
{
        g_message("ldm_daemon_device_added(): %s %s",
                  ldm_device_get_vendor(device),
                  ldm_device_get_name(device));
        if (ldm_device_has_type(device, LDM_DEVICE_TYPE_HID)) {
                g_message("HID device!");
        }
        if (ldm_device_has_type(device, LDM_DEVICE_TYPE_STORAGE)) {
                g_message("Storage device!");
        }
}

static void ldm_daemon_device_removed(__ldm_unused__ LdmDaemon *daemon, const gchar *path,
                                      __ldm_unused__ gpointer v)
{
        g_message("ldm_daemon_device_removed: %s", path);
}

static void ldm_daemon_discover_existing(LdmDaemon *self)
{
        GList *existing_devices = NULL;

        /* Iterate all existing non GPU devices */
        existing_devices = ldm_manager_get_devices(self->manager, LDM_DEVICE_TYPE_ANY);
        for (GList *elem = existing_devices; elem; elem = elem->next) {
                LdmDevice *device = elem->data;
                if (ldm_device_has_type(device, LDM_DEVICE_TYPE_GPU)) {
                        continue;
                }
                /* Attempt to discover devices now */
                ldm_daemon_discover_drivers(self, device);
        }
        g_list_free(existing_devices);
}

/**
 * Discover explicit GPU configuration separately from the main device checks
 */
static void ldm_daemon_discover_gpu(LdmDaemon *self)
{
        g_autoptr(LdmGPUConfig) gpu_config = NULL;
        LdmDevice *device = NULL;

        gpu_config = ldm_gpu_config_new(self->manager);
        device = ldm_gpu_config_get_detection_device(gpu_config);
        g_message("Detection GPU discovered: %s %s",
                  ldm_device_get_vendor(device),
                  ldm_device_get_name(device));

        /* Just check for drivers for now */
        ldm_daemon_discover_drivers(self, device);

        /* Simple Optimus detection */
        if (ldm_gpu_config_has_type(gpu_config, LDM_GPU_TYPE_OPTIMUS)) {
                g_message("Optimus gpu");
                device = ldm_gpu_config_get_primary_device(gpu_config);
                g_message("Primary GPU in Optimus config: %s %s",
                          ldm_device_get_vendor(device),
                          ldm_device_get_name(device));
        }
}

/**
 * Attempt to discover all providers for the given device.
 */
static void ldm_daemon_discover_drivers(LdmDaemon *self, LdmDevice *device)
{
        g_autoptr(GPtrArray) providers = NULL;

        providers = ldm_manager_get_providers(self->manager, device);
        if (!providers || providers->len == 0) {
                return;
        }

        g_message("Found %d provider(s) for %s", providers->len, ldm_device_get_name(device));

        for (unsigned int i = 0; i < providers->len; i++) {
                LdmProvider *prov = providers->pdata[i];
                g_message("\tPackage: %s (%s)",
                          ldm_provider_get_package(prov),
                          ldm_provider_get_installed(prov) ? "installed" : "uninstalled");
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
