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

#include "util.h"

/**
 * Forward declare for use in the dtor
 */
typedef struct LdmDevice LdmDevice;

/**
 * Deconstructor for an LdmDevice
 */
typedef void (*ldm_device_dtor)(LdmDevice *self);

/**
 * Basic type of device, i.e. how it connects to the system.
 */
typedef enum {
        LDM_DEVICE_MIN = 1 << 0,
        LDM_DEVICE_PCI = 1 << 1, /**<This is a PCI device. */
        LDM_DEVICE_USB = 1 << 2, /**<This is a USB device */
        LDM_DEVICE_MAX = 1 << 3,
} LdmDeviceType;

/**
 * Each device may have a given class, or capability.
 */
typedef enum {
        LDM_CLASS_ANY = 0,           /**<Special situation for any device match */
        LDM_CLASS_AUDIO = 1 << 0,    /**<Likely a USB headset */
        LDM_CLASS_GRAPHICS = 1 << 1, /**<Graphical device, i.e. GPU */
        LDM_CLASS_HID = 1 << 2,      /**<Input device, i.e. mouse */
        LDM_CLASS_PRINTER = 1 << 3,  /**<Attached printer */
        LDM_CLASS_VIDEO = 1 << 4,    /**<Video device such as a USB camera */
        LDM_CLASS_WIRELESS =
            1 << 5, /**<Some kind of wireless USB device (wireless mouse, dongle, etc) */
        LDM_CLASS_MAX = 1 << 6,
} LdmDeviceClass;

/**
 * LdmDevice is the fundamental device "type" within LDM
 */
struct LdmDevice {
        struct LdmDevice *next; /**<Simple device chaining */
        char *device_name;      /**<Allocated device name */
        char *driver;           /**<Name of the driver in used */
        char *sysfs_address;    /**</sys address on the host */
        ldm_device_dtor dtor;   /**<Deconstructor for this type */
        LdmDeviceType type;     /**<Type of this device */
        unsigned int class;     /**<Device class, may have multiple */
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

/**
 * Determine the number of devices
 */
int ldm_device_n_devices(LdmDevice *device);

DEF_AUTOFREE(LdmDevice, ldm_device_free)

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
