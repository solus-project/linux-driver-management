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

#pragma once

#include <stdint.h>
#include <stdlib.h>

/**
 * Interesting vendors (to LDM)
 */
#define PCI_VENDOR_ID_INTEL 0x8086
#define PCI_VENDOR_ID_NVIDIA 0x10DE
#define PCI_VENDOR_ID_AMD 0x1002

/**
 * Forward declare for use in the dtor
 */
typedef struct LdmDevice LdmDevice;

/**
 * Deconstructor for an LdmDevice
 */
typedef void (*ldm_device_dtor)(LdmDevice *self);

/**
 * Encapsulate the PCI address for a device
 */
typedef struct LdmPCIAddress {
        uint16_t domain; /**<Host bridge */
        uint8_t bus;     /**<Bus on the bridge */
        uint8_t dev;     /**<Device */
        uint8_t func;    /**<Functon of the device */
} LdmPCIAddress;

/**
 * Basic type of a given device, i.e. gpu, printer, etc.
 */
typedef enum {
        LDM_DEVICE_MIN = 1 << 0,
        LDM_DEVICE_GPU = 1 << 1,     /**<This is a GPU device. */
        LDM_DEVICE_UNKNOWN = 1 << 2, /**<We don't know about this kind of device */
        LDM_DEVICE_MAX = 1 << 3,
} LdmDeviceType;

/**
 * LdmDevice is the fundamental device "type" within LDM
 */
struct LdmDevice {
        struct LdmDevice *next; /**<Simple device chaining */
        char *device_name;      /**<Allocated device name */
        char *driver;           /**<Name of the driver in used */
        ldm_device_dtor dtor;   /**<Deconstructor for this type */
        LdmDeviceType type;     /**<Type of this device */
};

/**
 * Free a previously allocated LdmDevice, and any chained siblings
 */
void ldm_device_free(LdmDevice *device);

/**
 * Find the first device in the list with the given vendor, and return
 * a pointer to it.
 */
LdmDevice *ldm_device_find_vendor(LdmDevice *device, uint16_t vendor);

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
