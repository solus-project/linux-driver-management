/*
 * This file is part of linux-driver-management.
 *
 * Copyright © 2016-2018 Linux Driver Management Developers, Solus Project
 *
 * linux-driver-management is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#define _GNU_SOURCE

#include "config.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "device.h"
#include "glx-manager.h"
#include "pci-device.h"
#include "util.h"

struct _LdmGLXManagerClass {
        GObjectClass parent_class;
};

/**
 * SECTION:glx-manager
 * @Short_description: GL(X) configuration management
 * @see_also: #LdmGPUConfig
 * @Title: LdmGLXManager
 *
 * An LdmGLXManager is used to manage the X11 configuration for the OpenGL providers,
 * i.e. GLX implementations such as the NVIDIA proprietary driver. It is assumed that
 * the host system is using automatically managed drivers for the most part, i.e. glvnd
 * style, and not "alternatives" style.
 *
 * The older versions of LDM would manually manage the GL links, but this has been dropped
 * in favour of static packaging and GLVND. While GLVND isn't a strict requirement, it is
 * recommended.
 *
 * LdmGLXManager will remove existing X11 configurations if they reference a driver that
 * isn't considered valid, such as `/etc/X11/xorg.conf`, and manage a single snippet within
 * `/etc/X11/xorg.conf.d/00-ldm.conf`. This snippet will contain the bare minimum to "enable"
 * the drivers.
 *
 * Additionally a hybrid control file is written in the presence of enabled NVIDIA Proprietary
 * drivers for Optimus systems. This control file is used by `ldm-session-init(1)` to provide
 * xrandr bootstrap during the early initialisation of an X11 desktop session.
 *
 * This manager does not, and will not, control the specifics for Wayland. It is assumed that
 * Wayland compositors will set up offscreen surfaces with libGL_nvidia via glvnd and then
 * render the final result to the Intel device GL context (libGL_mesa). For non Optimus systems
 * they would simply use the primary GPU and GL implementation.
 */

/*
 * LdmGLXManager
 *
 * An LdmGLXManager is a utility mechanism to apply an #LdmGPUConfig to the system.
 * This is referred to as a GLX manager as the core problem is really the issue of
 * GLX libraries, and not necessarily pure GL libraries.
 */
struct _LdmGLXManager {
        GObject parent;

        gchar *stock_xorg_config;
        gchar *glx_xorg_config;
};

G_DEFINE_TYPE(LdmGLXManager, ldm_glx_manager, G_TYPE_OBJECT)

/* Helpers for xorg configurations */
static gboolean ldm_xorg_config_has_driver(const gchar *path, const gchar *driver);
static gboolean ldm_xorg_config_write_simple(const gchar *path, LdmDevice *device);
static gboolean ldm_xorg_config_write_optimus(const gchar *path, LdmDevice *device);
static gboolean ldm_xorg_driver_present(LdmDevice *device);

/* Private helpers for our class */
static gboolean ldm_glx_manager_configure_optimus(LdmGLXManager *self, LdmGPUConfig *config);
static gboolean ldm_glx_manager_configure_simple(LdmGLXManager *self, LdmGPUConfig *config);
static void ldm_glx_manager_nuke_legacy(void);

/**
 * ldm_glx_manager_dispose:
 *
 * Clean up a LdmGLXManager instance
 */
static void ldm_glx_manager_dispose(GObject *obj)
{
        LdmGLXManager *self = LDM_GLX_MANAGER(obj);

        g_clear_pointer(&self->stock_xorg_config, g_free);
        g_clear_pointer(&self->glx_xorg_config, g_free);

        G_OBJECT_CLASS(ldm_glx_manager_parent_class)->dispose(obj);
}

/**
 * ldm_glx_manager_class_init:
 *
 * Handle class initialisation
 */
static void ldm_glx_manager_class_init(LdmGLXManagerClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_glx_manager_dispose;
}

/**
 * ldm_glx_manager_init:
 *
 * Handle construction of the LdmGLXManager
 */
static void ldm_glx_manager_init(LdmGLXManager *self)
{
        /* Primary X.Org configuration */
        self->stock_xorg_config = g_build_filename(SYSCONFDIR, "X11", "xorg.conf", NULL);

        /* Where we'll make our config changes */
        self->glx_xorg_config =
            g_build_filename(SYSCONFDIR, "X11", "xorg.conf.d", "00-ldm.conf", NULL);
}

/**
 * ldm_glx_manager_new:
 *
 * Create a new LdmGLXManager instance
 *
 * Returns: (transfer full): An #LdmGLXManager instance.
 */
LdmGLXManager *ldm_glx_manager_new()
{
        return g_object_new(LDM_TYPE_GLX_MANAGER, NULL);
}

static gboolean ldm_xorg_config_has_driver(const gchar *path, const gchar *driver)
{
        FILE *fp = NULL;
        char *bfr = NULL;
        size_t n = 0;
        ssize_t read = 0;
        g_autofree gchar *driv_name = NULL;
        gboolean ret = FALSE;

        if (access(path, F_OK) != 0) {
                return FALSE;
        }

        fp = fopen(path, "r");
        if (!fp) {
                return FALSE;
        }

        driv_name = g_strdup_printf("\"%s\"", driver);

        while ((read = getline(&bfr, &n, fp)) > 0) {
                gchar *work = NULL;

                /* Strip the newline from it */
                if (bfr[read - 1] == '\n') {
                        bfr[read - 1] = '\0';
                        --read;
                }

                /* Empty lines are uninteresting. */
                if (read < 1) {
                        continue;
                }

                work = g_strstrip(bfr);
                if (g_str_has_prefix(work, "Driver") && g_str_has_suffix(work, driv_name)) {
                        ret = TRUE;
                        break;
                }

                free(bfr);
                bfr = NULL;
        }

        if (bfr) {
                free(bfr);
        }

        fclose(fp);

        return ret;
}

/**
 * ldm_xorg_config_id:
 * @device: Device to find a "pretty" ID for
 *
 * Construct a pretty ID for the LdmDevice based on the vendor
 */
static inline const gchar *ldm_xorg_config_id(LdmDevice *device)
{
        /* Construct fancy short string for xorg configuration */
        switch (ldm_device_get_vendor_id(device)) {
        case LDM_PCI_VENDOR_ID_AMD:
                return "AMD";
        case LDM_PCI_VENDOR_ID_INTEL:
                return "Intel";
        case LDM_PCI_VENDOR_ID_NVIDIA:
                return "NVIDIA";
        default:
                /* No fancy short string. */
                return "GPU";
        }
}

/**
 * ldm_xorg_config_driver:
 * @device: Device to translate X config for
 *
 * Translate the device vendor ID to a usable module name for X11 configuration
 */
static inline const gchar *ldm_xorg_config_driver(LdmDevice *device)
{
        /* Determine the xorg driver to use based on the vendor */
        switch (ldm_device_get_vendor_id(device)) {
        case LDM_PCI_VENDOR_ID_AMD:
                return "fglrx";
        case LDM_PCI_VENDOR_ID_NVIDIA:
                return "nvidia";
        default:
                return NULL;
        }
}

/**
 * ldm_xorg_config_write_simple:
 * @path: File path to alter
 * @device: Device to emit into the X.Org configuration
 */
static gboolean ldm_xorg_config_write_simple(const gchar *path, LdmDevice *device)
{
        g_autoptr(GError) error = NULL;
        g_autofree gchar *dirname = NULL;
        g_autofree gchar *contents = NULL;
        const gchar *device_id = NULL;
        const gchar *driver = NULL;

        dirname = g_path_get_dirname(path);
        if (!dirname) {
                return FALSE;
        }

        /* Make sure we have the leading directory first */
        if (!g_file_test(dirname, G_FILE_TEST_IS_DIR) &&
            g_mkdir_with_parents(dirname, 00755) != 0) {
                g_warning("Failed to construct leading directory %s: %s", dirname, strerror(errno));
                return FALSE;
        }

        /* Construct prettified simple x.org configuration */
        device_id = ldm_xorg_config_id(device);
        driver = ldm_xorg_config_driver(device);
        if (!driver) {
                g_warning("SHOULD NOT HAPPEN: Missing driver translation on %s",
                          ldm_device_get_path(device));
                return FALSE;
        }

        contents = g_strdup_printf(
            "Section \"Device\"\n"
            "        Identifier \"%s Card\"\n"
            "        Driver \"%s\"\n"
            "        VendorName \"%s\"\n"
            "        BoardName \"%s\"\n"
            "EndSection\n",
            device_id,
            driver,
            ldm_device_get_vendor(device),
            ldm_device_get_name(device));

        /* Write the file */
        if (!g_file_set_contents(path, contents, (gssize)strlen(contents), &error)) {
                g_warning("Failed to set X.Org config %s: %s", path, error->message);
                return FALSE;
        }

        return TRUE;
}

/**
 * ldm_xorg_config_write_optimus:
 * @path: File path to alter
 * @device: Confguration for the Optimus setup
 */
static gboolean ldm_xorg_config_write_optimus(const gchar *path, LdmDevice *device)
{
        g_autoptr(GError) error = NULL;
        g_autofree gchar *dirname = NULL;
        g_autofree gchar *contents = NULL;
        const gchar *device_id = NULL;
        const gchar *driver = NULL;
        guint bus = 0, dev = 0;
        gint func = 0;

        dirname = g_path_get_dirname(path);
        if (!dirname) {
                return FALSE;
        }

        /* Make sure we have the leading directory first */
        if (!g_file_test(dirname, G_FILE_TEST_IS_DIR) &&
            g_mkdir_with_parents(dirname, 00755) != 0) {
                g_warning("Failed to construct leading directory %s: %s", dirname, strerror(errno));
                return FALSE;
        }

        /* Bit of sanity if you please. */
        if (ldm_device_get_vendor_id(device) != LDM_PCI_VENDOR_ID_NVIDIA) {
                g_message("Something is insane with configuration: %s is not an NVIDIA device!",
                          ldm_device_get_name(device));
                return FALSE;
        }
        if (!ldm_device_has_type(device, LDM_DEVICE_TYPE_PCI)) {
                g_message("Something is insane with configuration: %s is not a PCI device!",
                          ldm_device_get_name(device));
                return FALSE;
        }

        /* Stash address for DRM style PCI ID */
        ldm_pci_device_get_address(LDM_PCI_DEVICE(device), &bus, &dev, &func);

        /* Construct prettified simple x.org configuration */
        device_id = ldm_xorg_config_id(device);
        driver = ldm_xorg_config_driver(device);
        if (!driver) {
                g_warning("SHOULD NOT HAPPEN: Missing driver translation on %s",
                          ldm_device_get_path(device));
                return FALSE;
        }

        contents = g_strdup_printf(
            "Section \"Module\"\n"
            "        Load \"modesetting\"\n"
            "EndSection\n\n"
            "Section \"Device\"\n"
            "        Identifier \"%s Card\"\n"
            "        Driver \"%s\"\n"
            "        BusID \"PCI:%u:%u:%d\"\n"
            "        Option \"AllowEmptyInitialConfiguration\"\n"
            "        VendorName \"%s\"\n"
            "        BoardName \"%s\"\n"
            "EndSection\n",
            device_id,
            driver,
            bus,
            dev,
            func,
            ldm_device_get_vendor(device),
            ldm_device_get_name(device));

        /* Write the file */
        if (!g_file_set_contents(path, contents, (gssize)strlen(contents), &error)) {
                g_warning("Failed to set X.Org config %s: %s", path, error->message);
                return FALSE;
        }

        return TRUE;
}

/**
 * ldm_xorg_driver_present:
 *
 * Work out if the X.Org driver for the given device is actually present.
 * For open source drivers, this is always going to be FALSE. We use this
 * to detect NVIDIA/AMD proprietary drivers, the presence of the xorg
 * module indicating that the proprietary driver is installed.
 *
 * Note that in a glvnd enabled world all of this stuff is X11 specific.
 * Wayland world is KMS driven and in NVIDIA requires eglplatform, all of
 * which is automatic and doesn't require any kind of configuration.
 */
static gboolean ldm_xorg_driver_present(LdmDevice *device)
{
        g_autofree gchar *test_path = NULL;
        const gchar *drv_fragment = NULL;

        switch (ldm_device_get_vendor_id(device)) {
        case LDM_PCI_VENDOR_ID_AMD:
                drv_fragment = "fglrx_drv.so";
                break;
        case LDM_PCI_VENDOR_ID_NVIDIA:
                drv_fragment = "nvidia_drv.so";
                break;
        default:
                return FALSE;
        }

        test_path = g_build_filename(XORG_MODULE_DIRECTORY, "drivers", drv_fragment, NULL);
        if (!test_path) {
                return FALSE;
        }

        return g_file_test(test_path, G_FILE_TEST_EXISTS);
}

/**
 * ldm_glx_manager_nuke_optimus:
 *
 * Nuke traces of the optimus configuration
 */
static void ldm_glx_manager_nuke_optimus(void)
{
        /* Remove any existing hybrid tracking file */
        if (g_file_test(LDM_HYBRID_FILE, G_FILE_TEST_EXISTS)) {
                if (unlink(LDM_HYBRID_FILE) != 0) {
                        g_warning("Failed to remove hybrid tracking file %s: %s",
                                  LDM_HYBRID_FILE,
                                  strerror(errno));
                }
        }
}

/**
 * ldm_glx_manager_nuke_user_configurations:
 *
 * Only nuke an existing /etc/X11/xorg.conf if it contains sections for proprietary
 * drivers.
 */
static gboolean ldm_glx_manager_nuke_user_configurations(LdmGLXManager *self)
{
        gboolean ret = TRUE;

        static const gchar *xorg_drivers[] = {
                "nvidia",
                "fglrx",
        };

        for (guint i = 0; i < G_N_ELEMENTS(xorg_drivers); i++) {
                if (!ldm_xorg_config_has_driver(self->stock_xorg_config, xorg_drivers[i])) {
                        continue;
                }
                fprintf(stderr,
                        "Removing %s as it references X11 driver '%s'\n",
                        self->stock_xorg_config,
                        xorg_drivers[i]);
                /* Need to remove traces of this stock config file */
                if (unlink(self->stock_xorg_config) != 0) {
                        g_warning("Failed to remove X11 config %s: %s",
                                  self->stock_xorg_config,
                                  strerror(errno));
                        ret = FALSE;
                }
        }

        return ret;
}

/**
 * ldm_glx_manager_nuke_configurations:
 *
 * Nuke any existing "bad" configurations we may have as we're unsetting
 * any potential proprietary driver enablings
 */
static void ldm_glx_manager_nuke_configurations(LdmGLXManager *self)
{
        ldm_glx_manager_nuke_user_configurations(self);
        ldm_glx_manager_nuke_optimus();

        if (g_file_test(self->glx_xorg_config, G_FILE_TEST_EXISTS)) {
                fprintf(stderr, "Removing now invalid X11 GLX config %s\n", self->glx_xorg_config);
                if (unlink(self->glx_xorg_config) != 0) {
                        g_warning("Failed to remove GLX config %s: %s",
                                  self->glx_xorg_config,
                                  strerror(errno));
                }
        }
}

/**
 * ldm_glx_manager_configure_optimus:
 *
 * Attempt configuration of an Optimus system with proprietary drivers
 */
static gboolean ldm_glx_manager_configure_optimus(LdmGLXManager *self, LdmGPUConfig *config)
{
        g_autoptr(GError) error = NULL;
        g_autofree gchar *dirname = NULL;

        /* For now we just write a 1 to touch the file and don't care about the contents.
           In future we'll use 0 or non-existent to disable, 1 for "always on", and 2 for dynamic.
        */
        static const gchar *contents = "1";

        ldm_glx_manager_nuke_user_configurations(self);

        /* Before we can write the hybrid bit, we have to be able to set the xorg config */
        if (!ldm_xorg_config_write_optimus(self->glx_xorg_config,
                                           ldm_gpu_config_get_secondary_device(config))) {
                return FALSE;
        }

        dirname = g_path_get_dirname(LDM_HYBRID_FILE);
        if (!dirname) {
                return FALSE;
        }

        /* Make sure we have the leading directory first */
        if (!g_file_test(dirname, G_FILE_TEST_IS_DIR) &&
            g_mkdir_with_parents(dirname, 00755) != 0) {
                g_warning("Failed to construct leading directory %s: %s", dirname, strerror(errno));
                return FALSE;
        }

        /* Write the hybrid file contents now */
        if (!g_file_set_contents(LDM_HYBRID_FILE, contents, (gssize)strlen(contents), &error)) {
                g_warning("Failed to set hybrid file contents %s: %s",
                          LDM_HYBRID_FILE,
                          error->message);
                return FALSE;
        }

        return TRUE;
}

/**
 * ldm_glx_manager_configure_simple:
 *
 * Attempt configuration of a simple proprietary driver
 */
static gboolean ldm_glx_manager_configure_simple(LdmGLXManager *self, LdmGPUConfig *config)
{
        /* Make sure we don't have Optimus! */
        ldm_glx_manager_nuke_optimus();

        /* Try to write new config first */
        if (!ldm_xorg_config_write_simple(self->glx_xorg_config,
                                          ldm_gpu_config_get_detection_device(config))) {
                return FALSE;
        }
        /* Now try to nuke any existing user config */
        return ldm_glx_manager_nuke_user_configurations(self);
}

/**
 * ldm_glx_manager_apply_configuration:
 * @config: Valid LdmGPUConfiguration
 *
 * Attempt to apply the primary portion of the configuration per the systems current configuration.
 * If proprietary drivers are installed and enabled, they will be configured. If an Optimus system
 * is encountered then it will also be configured in X11, and in the installed display manager
 * configurations.
 *
 * If it is not possible to "install" a configuration, then any changes we may have made will be
 * immediately unapplied and we'll go back to a "stock" configuration that intentionally removes any
 * enabling for the proprietary drivers we may have applied.
 *
 * This should only happen when the module isn't present for the primary detection device.
 */
gboolean ldm_glx_manager_apply_configuration(LdmGLXManager *self, LdmGPUConfig *config)
{
        LdmDevice *detection_device = NULL;

        g_return_val_if_fail(self != NULL, FALSE);

        /* Clean up before doing anything. */
        ldm_glx_manager_nuke_legacy();

        detection_device = ldm_gpu_config_get_detection_device(config);

        /* No primary device, this is fine, could be a chroot. */
        if (!detection_device) {
                ldm_glx_manager_nuke_configurations(self);
                return TRUE;
        }

        /* If there isn't a valid driver for this device, remove configurations for it */
        if (!ldm_xorg_driver_present(detection_device)) {
                ldm_glx_manager_nuke_configurations(self);
                return TRUE;
        }

        /* TODO: Support SLI/Crossfire + Hybrid etc. */
        if (ldm_gpu_config_has_type(config, LDM_GPU_TYPE_OPTIMUS)) {
                if (!ldm_glx_manager_configure_optimus(self, config)) {
                        goto failed;
                }
                return TRUE;
        }

        /* Assume we're just a simple device. */
        if (!ldm_glx_manager_configure_simple(self, config)) {
                goto failed;
        }

        return TRUE;

failed:

        g_warning("Encountered fatal issue in driver configuration, restoring defaults");
        ldm_glx_manager_nuke_configurations(self);
        return FALSE;
}

/**
 * ldm_glx_manager_nuke_legacy:
 *
 * Nuke previously constructed files from the old LDM implementation that are no longer
 * needed.
 */
static void ldm_glx_manager_nuke_legacy()
{
        /* Garbage paths left over from old LDM, make sure they die */
        static const gchar *bad_paths[] = {
                "/etc/lightdm/lightdm.conf.d/99-ldm-xrandr.conf",
                "/etc/lightdm-xrandr-init.sh",
                "/usr/share/gdm/greeter/autostart/optimus.desktop",
                "/etc/xdg/autostart/optimus.desktop",
        };

        for (guint i = 0; i < G_N_ELEMENTS(bad_paths); i++) {
                const gchar *path = bad_paths[i];
                if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
                        continue;
                }
                fprintf(stderr, "Removing legacy path %s\n", path);
                if (unlink(path) != 0) {
                        g_warning("Failed to remove legacy path %s: %s", path, strerror(errno));
                }
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
