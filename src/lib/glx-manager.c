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

#include "config.h"

#include <errno.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <string.h>

#include "device.h"
#include "glx-manager.h"
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
 * An LdmGLXManager is used to provide control over the GL(X) system implementation.
 * Whilst most of what is controlled applies to "pure" GL implementations, the real
 * requirement is in dealing with explicit GLX implementations such as the NVIDIA
 * proprietary libGL.
 *
 * To make these "work", usually requires a very explicit X11 configuration as well
 * as the control of library availability. On non GLVND systems, the libGL files
 * from mesa (technically a GLX implementation in this instance) will conflict
 * on the filesystem with the ones from the proprietary driver.
 *
 * In the "new world" of GLVND these paths no longer conflict, however this does
 * not alter the fact that an X11 configuration needs writing explicitly for these
 * cases. Additionally the X11 `libglx.so` extension from X.Org is typically replaced
 * by the proprietary driver's own libglx.so implementation. This is usually handled
 * by either a filesystem mangling, or by using a patched X.Org server which understands
 * special directives to point to the real libglx.so.
 *
 * In short we have two worlds:
 *
 * - "Alternatives" links: Filesystem butchery is performed and `ld.so.conf` may also be used.
 * - GLVND: No filesystem butchery, and requiring patched X.Org for Extensions directory support.
 *
 * The LdmGLXManager requires privileges to be used, and will perform configurations as
 * the system dictates. This is only really intended for use by `linux-driver-management configure
 * gpu`
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

static gboolean ldm_xorg_config_has_driver(const gchar *path, const gchar *driver);
static gboolean ldm_xorg_config_write_simple(const gchar *path, LdmDevice *device);
static gboolean ldm_xorg_driver_present(LdmDevice *device);

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
        self->glx_xorg_config = g_build_filename(SYSCONFDIR,
                                                 "X11",
                                                 "xorg.conf.d,"
                                                 "00-ldm.conf",
                                                 NULL);
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
        g_autoptr(GError) error = NULL;
        g_autoptr(GFile) file = NULL;
        g_autoptr(GDataInputStream) dis = NULL;
        g_autoptr(GFileInputStream) fis = NULL;
        gsize length = 0;
        gboolean ret = FALSE;
        gchar *read = NULL;
        g_autofree gchar *driv_name = NULL;

        file = g_file_new_for_path(path);
        if (!file) {
                return FALSE;
        }

        if (!g_file_query_exists(file, NULL)) {
                return FALSE;
        }

        fis = g_file_read(file, NULL, &error);
        if (!fis || error) {
                goto emit_error;
        }

        dis = g_data_input_stream_new(G_INPUT_STREAM(fis));
        if (!dis) {
                goto emit_error;
        }

        driv_name = g_strdup_printf("\"%s\"", driver);

        while ((read = g_data_input_stream_read_line(dis, &length, NULL, &error)) != NULL) {
                char *tmp = g_strstrip(read);
                if (!g_str_has_prefix(tmp, "Driver")) {
                        goto next_line;
                }

                if (!g_str_has_suffix(tmp, driv_name)) {
                        goto next_line;
                }

                ret = TRUE;

        next_line:
                g_free(read);
                read = NULL;

                if (ret) {
                        break;
                }
        }

        if (read) {
                g_free(read);
        }

emit_error:
        if (error) {
                g_warning("Failed to parse X11 config %s: %s", path, error->message);
                return FALSE;
        }

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
 * @driver: The driver name to use
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

        test_path = g_build_filename(XORG_MODULE_DIRECTORY, drv_fragment, NULL);
        ;
        if (!test_path) {
                return FALSE;
        }

        return g_file_test(test_path, G_FILE_TEST_EXISTS);
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
