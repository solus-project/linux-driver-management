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

#include <gio/gio.h>
#include <stdlib.h>

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
