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

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <umockdev.h>

#include "ldm-private.h"
#include "ldm.h"
#include "util.h"

DEF_AUTOFREE(UMockdevTestbed, g_object_unref)

#define YETI_UMOCKDEV_FILE TEST_DATA_ROOT "/blueYeti.umockdev"
#define PRINTER_UMOCKDEV_FILE TEST_DATA_ROOT "/brotherPrinter.umockdev"
#define NV_MOCKDEV_FILE TEST_DATA_ROOT "/nvidia1060.umockdev"

/**
 * This test is to help us develop composite USB aggregation within the
 * library so that interfaces are merged into a USB device.
 */
START_TEST(test_manager_usb_simple)
{
        g_autoptr(LdmManager) manager = NULL;
        autofree(UMockdevTestbed) *bed = NULL;
        g_autoptr(GPtrArray) devices = NULL;

        bed = umockdev_testbed_new();
        fail_if(!umockdev_testbed_add_from_file(bed, YETI_UMOCKDEV_FILE, NULL),
                "Failed to create Blue Yeti device");
        manager = ldm_manager_new(0);
        fail_if(!manager, "Failed to get the LdmManager");

        /* 2 devices right now because of the root hub. */
        devices = ldm_manager_get_devices(manager, LDM_DEVICE_TYPE_USB | LDM_DEVICE_TYPE_AUDIO);
        fail_if(!devices, "Failed to obtain devices");
        fail_if(devices->len != 1, "Expected 1 device, got %u devices", devices->len);
}
END_TEST

/**
 * Identify the USB printer in a noisy environment
 */
START_TEST(test_manager_usb_noisy)
{
        g_autoptr(LdmManager) manager = NULL;
        autofree(UMockdevTestbed) *bed = NULL;
        g_autoptr(GPtrArray) devices = NULL;
        const char *vendor = NULL;
        LdmDevice *device = NULL;
        static const gchar *test_files[] = {
                YETI_UMOCKDEV_FILE,
                PRINTER_UMOCKDEV_FILE,
                NV_MOCKDEV_FILE,
        };

        bed = umockdev_testbed_new();

        for (size_t i = 0; i < G_N_ELEMENTS(test_files); i++) {
                const gchar *dev = test_files[i];
                fail_if(!umockdev_testbed_add_from_file(bed, dev, NULL),
                        "Failed to add device %s",
                        dev);
        }

        manager = ldm_manager_new(0);
        fail_if(!manager, "Failed to get the LdmManager");

        /* No PCI pls */
        devices = ldm_manager_get_devices(manager, LDM_DEVICE_TYPE_PCI | LDM_DEVICE_TYPE_PRINTER);
        fail_if(devices->len != 0, "Printer should only be USB!");
        g_ptr_array_unref(devices);
        devices = NULL;

        /* Has USB printer? */
        devices = ldm_manager_get_devices(manager, LDM_DEVICE_TYPE_USB | LDM_DEVICE_TYPE_PRINTER);
        fail_if(!devices, "Failed to find USB printers");
        fail_if(devices->len != 1, "Should only have one printer!");

        device = devices->pdata[0];
        vendor = ldm_device_get_vendor(device);
        fail_if(!vendor, "Missing vendor on printer!");
        fail_if(!g_str_equal(vendor, "Brother Industries, Ltd"),
                "Invalid printer vendor, expected '%s', got '%s'",
                "Brother Industries, Ltd",
                vendor);
}
END_TEST

/**
 * Standard helper for running a test suite
 */
static int ldm_test_run(Suite *suite)
{
        SRunner *runner = NULL;
        int n_failed = 0;

        runner = srunner_create(suite);
        srunner_run_all(runner, CK_VERBOSE);
        n_failed = srunner_ntests_failed(runner);
        srunner_free(runner);

        return n_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static Suite *test_create(void)
{
        Suite *s = NULL;
        TCase *tc = NULL;

        s = suite_create(__FILE__);
        tc = tcase_create(__FILE__);
        suite_add_tcase(s, tc);

        tcase_add_test(tc, test_manager_usb_simple);
        tcase_add_test(tc, test_manager_usb_noisy);

        return s;
}

int main(__ldm_unused__ int argc, __ldm_unused__ char **argv)
{
        return ldm_test_run(test_create());
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
