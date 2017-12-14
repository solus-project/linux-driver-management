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

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <umockdev.h>

#include "ldm-private.h"
#include "ldm.h"
#include "util.h"

DEF_AUTOFREE(UMockdevTestbed, g_object_unref)

#define YETI_UMOCKDEV_FILE TEST_DATA_ROOT "/blueYeti.umockdev"

/**
 * This test is to help us develop composite USB aggregation within the
 * library so that interfaces are merged into a USB device.
 */
START_TEST(test_manager_usb_simple)
{
        g_autoptr(LdmManager) manager = NULL;
        autofree(UMockdevTestbed) *bed = NULL;
        g_autoptr(GList) devices = NULL;

        bed = umockdev_testbed_new();
        fail_if(!umockdev_testbed_add_from_file(bed, YETI_UMOCKDEV_FILE, NULL),
                "Failed to create Blue Yeti device");
        manager = ldm_manager_new();
        fail_if(!manager, "Failed to get the LdmManager");

        /* 2 devices right now because of the root hub. */
        devices = ldm_manager_get_devices(manager, LDM_DEVICE_TYPE_USB | LDM_DEVICE_TYPE_AUDIO);
        fail_if(!devices, "Failed to obtain devices");
        fail_if(g_list_length(devices) != 1,
                "Expected 1 device, got %u devices",
                g_list_length(devices));
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
