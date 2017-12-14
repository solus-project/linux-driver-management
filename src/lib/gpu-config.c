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

#include "gpu-config.h"
#include "ldm-enums.h"
#include "util.h"

struct _LdmGPUConfigClass {
        GObjectClass parent_class;
};

/**
 * LdmGPUConfig
 *
 * This object is used to query an #LdmManager for the system GPU configuration
 * and topology. Using the #LdmGPUConfig allows end-users to query exactly
 * what kind of configuration is present, and determine the primary vs secondary
 * GPUs, presence of Optimus/Hybrid GPUs, etc.
 */
struct _LdmGPUConfig {
        GObject parent;

        LdmManager *manager;

        /* Hybrid GPU tracking basically. */
        LdmDevice *primary;   /* Primary GPU */
        LdmDevice *secondary; /* Secondary GPU */

        guint n_gpu;    /* How many GPUs we got? */
        guint gpu_type; /* Primary type */
};

static void ldm_gpu_config_set_property(GObject *object, guint id, const GValue *value,
                                        GParamSpec *spec);
static void ldm_gpu_config_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec);
static void ldm_gpu_config_constructed(GObject *obj);
static void ldm_gpu_config_analyze(LdmGPUConfig *self);

G_DEFINE_TYPE(LdmGPUConfig, ldm_gpu_config, G_TYPE_OBJECT)

/* Property IDs */
enum { PROP_MANAGER = 1, PROP_TYPE, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

/**
 * ldm_gpu_config_dispose:
 *
 * Clean up a LdmGPUConfig instance
 */
static void ldm_gpu_config_dispose(GObject *obj)
{
        G_OBJECT_CLASS(ldm_gpu_config_parent_class)->dispose(obj);
}

/**
 * ldm_gpu_config_class_init:
 *
 * Handle class initialisation
 */
static void ldm_gpu_config_class_init(LdmGPUConfigClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->constructed = ldm_gpu_config_constructed;
        obj_class->dispose = ldm_gpu_config_dispose;
        obj_class->get_property = ldm_gpu_config_get_property;
        obj_class->set_property = ldm_gpu_config_set_property;

        /**
         * LdmGPUConfig:manager: (type LdmManager) (transfer none)
         *
         * Parent device for this device instance
         */
        obj_properties[PROP_MANAGER] =
            g_param_spec_pointer("manager",
                                 "LdmManager",
                                 "Manager for our instance",
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

        /**
         * LdmGPUConfig:gpu-type
         *
         * The composite type for this GPU Configuration
         */
        obj_properties[PROP_TYPE] = g_param_spec_flags("gpu-type",
                                                       "GPU type",
                                                       "Composite type for this GPU Config",
                                                       LDM_TYPE_GPU_TYPE,
                                                       LDM_GPU_TYPE_SIMPLE,
                                                       G_PARAM_READABLE);

        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

static void ldm_gpu_config_set_property(GObject *object, guint id, const GValue *value,
                                        GParamSpec *spec)
{
        LdmGPUConfig *self = LDM_GPU_CONFIG(object);

        switch (id) {
        case PROP_MANAGER:
                self->manager = g_value_get_pointer(value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

static void ldm_gpu_config_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec)
{
        LdmGPUConfig *self = LDM_GPU_CONFIG(object);

        switch (id) {
        case PROP_MANAGER:
                g_value_set_pointer(value, self->manager);
                break;
        case PROP_TYPE:
                g_value_set_flags(value, self->gpu_type);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

/**
 * ldm_gpu_config_constructed:
 *
 * We're now up and running with a valid Manager instance, probe it and
 * find out exactly what our state looks like. :)
 */
static void ldm_gpu_config_constructed(GObject *obj)
{
        ldm_gpu_config_analyze(LDM_GPU_CONFIG(obj));
        G_OBJECT_CLASS(ldm_gpu_config_parent_class)->constructed(obj);
}

/**
 * ldm_gpu_config_init:
 *
 * Handle construction of the LdmGPUConfig
 */
static void ldm_gpu_config_init(LdmGPUConfig *self)
{
        self->n_gpu = 0;
        self->gpu_type = LDM_GPU_TYPE_SIMPLE;
}

/**
 * ldm_gpu_config_search_boot:
 * @vga_boot: If we want it bootable or not
 * @not_like: Item to not be.
 *
 * Utility method to find the boot_vga device, i.e. the GPU that was used
 * to boot the system.
 */
static LdmDevice *ldm_gpu_config_search_boot(GList *devices, gboolean vga_boot, LdmDevice *not_like)
{
        for (GList *elem = devices; elem; elem = elem->next) {
                LdmDevice *device = LDM_DEVICE(elem->data);

                if (device == not_like) {
                        continue;
                }

                if (ldm_device_has_attribute(device, LDM_DEVICE_ATTRIBUTE_BOOT_VGA) == vga_boot) {
                        return device;
                }
        }

        return NULL;
}

/**
 * ldm_gpu_config_do_optimus:
 *
 * Attempt to verify whether we have an Optimus system. This basically means that the
 * primary GPU (boot_vga) must be an Intel device, and the secondary device must be
 * NVIDIA, without boot_vga attribute.
 *
 * Returns: TRUE if we detected Optimus
 */
static gboolean ldm_gpu_config_do_optimus(LdmGPUConfig *self, LdmDevice *primary,
                                          LdmDevice *secondary)
{
        if (!ldm_device_has_attribute(primary, LDM_DEVICE_ATTRIBUTE_BOOT_VGA)) {
                return FALSE;
        }
        if (ldm_device_has_attribute(secondary, LDM_DEVICE_ATTRIBUTE_BOOT_VGA)) {
                return FALSE;
        }
        if (ldm_device_get_vendor_id(primary) != LDM_PCI_VENDOR_ID_INTEL) {
                return FALSE;
        }
        if (ldm_device_get_vendor_id(secondary) != LDM_PCI_VENDOR_ID_NVIDIA) {
                return FALSE;
        }

        self->gpu_type = LDM_GPU_TYPE_HYBRID | LDM_GPU_TYPE_OPTIMUS;
        self->primary = primary;
        self->secondary = secondary;
        return TRUE;
}

/**
 * ldm_gpu_config_amd_hybrid:
 *
 * Hybrid AMD gpu configurations require that the primary GPU is boot_vga and is either
 * and AMD APU or Intel iGPU, and the secondary GPU is non boot-vga and also AMD.
 *
 * Returns: TRUE if we detected AMD Hybrid graphics
 */
static gboolean ldm_gpu_config_do_amd_hybrid(LdmGPUConfig *self, LdmDevice *primary,
                                             LdmDevice *secondary)
{
        gint primary_vendor_id = 0;

        if (!ldm_device_has_attribute(primary, LDM_DEVICE_ATTRIBUTE_BOOT_VGA)) {
                return FALSE;
        }
        if (ldm_device_has_attribute(secondary, LDM_DEVICE_ATTRIBUTE_BOOT_VGA)) {
                return FALSE;
        }

        primary_vendor_id = ldm_device_get_vendor_id(primary);
        if (primary_vendor_id != LDM_PCI_VENDOR_ID_INTEL &&
            primary_vendor_id != LDM_PCI_VENDOR_ID_AMD) {
                return FALSE;
        }
        if (ldm_device_get_vendor_id(secondary) != LDM_PCI_VENDOR_ID_AMD) {
                return FALSE;
        }

        self->gpu_type = LDM_GPU_TYPE_HYBRID;
        self->primary = primary;
        self->secondary = secondary;
        return TRUE;
}

/**
 * ldm_gpu_config_analyze:
 *
 * Ask the manager what the story is.
 */
static void ldm_gpu_config_analyze(LdmGPUConfig *self)
{
        g_autoptr(GList) devices = NULL;
        LdmDevice *boot_vga = NULL;
        LdmDevice *non_boot_vga = NULL;
        gint vendor_id = 0;

        devices = ldm_manager_get_devices(self->manager, LDM_DEVICE_TYPE_PCI | LDM_DEVICE_TYPE_GPU);
        self->n_gpu = g_list_length(devices);
        if (self->n_gpu < 1) {
                g_message("failed to discover any GPUs");
                return;
        }

        /* Safety set */
        self->primary = g_list_nth_data(devices, 0);

        /* Trivial GPU configuration */
        if (self->n_gpu == 1) {
                self->gpu_type = LDM_GPU_TYPE_SIMPLE;
                return;
        }

        /* Ensure we have boot_vga, compensate if required */
        boot_vga = ldm_gpu_config_search_boot(devices, TRUE, NULL);
        if (!boot_vga) {
                boot_vga = g_list_nth_data(devices, 0);
        }

        /* Ensure primary is properly set now */
        self->primary = boot_vga;

        /* Find a non_boot_vga that isn't boot_vga */
        non_boot_vga = ldm_gpu_config_search_boot(devices, FALSE, boot_vga);

        /* Optimus? */
        if (ldm_gpu_config_do_optimus(self, boot_vga, non_boot_vga)) {
                return;
        }

        /* AMD hybrid? */
        if (ldm_gpu_config_do_amd_hybrid(self, boot_vga, non_boot_vga)) {
                return;
        }

        /* Do we have composite graphics, i.e. SLI? */
        vendor_id = ldm_device_get_vendor_id(boot_vga);
        if (vendor_id == ldm_device_get_vendor_id(non_boot_vga)) {
                switch (vendor_id) {
                case LDM_PCI_VENDOR_ID_AMD:
                        self->gpu_type = LDM_GPU_TYPE_COMPOSITE | LDM_GPU_TYPE_CROSSFIRE;
                        return;
                case LDM_PCI_VENDOR_ID_NVIDIA:
                        self->gpu_type = LDM_GPU_TYPE_COMPOSITE | LDM_GPU_TYPE_SLI;
                        return;
                default:
                        break;
                }
        }

        /* Fugit, back to being simple device */
        self->gpu_type = LDM_GPU_TYPE_SIMPLE;
}

/**
 * ldm_gpu_config_new:
 * @manager: (transfer none): Manager to query for a GPU config
 *
 * Construct a GPU configuration from the #LdmManager to determine the
 * exact GPU topology.
 */
LdmGPUConfig *ldm_gpu_config_new(LdmManager *manager)
{
        g_assert(manager != NULL);

        return g_object_new(LDM_TYPE_GPU_CONFIG, "manager", manager, NULL);
}

/**
 * ldm_gpu_config_get_manager:
 *
 * Returns: (transfer none): A reference to our manager
 */
LdmManager *ldm_gpu_config_get_manager(LdmGPUConfig *self)
{
        g_return_val_if_fail(self != NULL, NULL);

        return self->manager;
}

/**
 * ldm_gpu_config_count:
 *
 * Determine the number of GPUs present on the system
 *
 * Returns: Number of GPUs
 */
guint ldm_gpu_config_count(LdmGPUConfig *self)
{
        g_return_val_if_fail(self != NULL, 0);
        return self->n_gpu;
}

/**
 * ldm_gpu_config_get_gpu_type:
 *
 * Get the type for this GPU Configuration to determine exactly
 * what kind of device set we're dealing with.
 *
 * Returns: The known type of this configuration
 */
LdmGPUType ldm_gpu_config_get_gpu_type(LdmGPUConfig *self)
{
        g_return_val_if_fail(self != NULL, LDM_GPU_TYPE_SIMPLE);
        return self->gpu_type;
}

/**
 * ldm_gpu_config_has_type:
 * @mask: Bitwise OR combination of #LdmGPUType
 *
 * Test whether this GPU config has the given type(s) by testing the mask against
 * our known types.
 */
gboolean ldm_gpu_config_has_type(LdmGPUConfig *self, LdmGPUType mask)
{
        g_return_val_if_fail(self != NULL, FALSE);

        if ((self->gpu_type & mask) == mask) {
                return TRUE;
        }

        return FALSE;
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
