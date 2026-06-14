#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Include the udev device header for function declarations */
#include "udev-device.h"

START_TEST(test_buffer_overflow_devpath)
{
    /* Invariant: Buffer reads/writes never exceed declared length,
       even when oversized input is provided to device path functions */
    char oversized_2x[512];
    char oversized_10x[2560];
    char boundary[256];
    const char *valid = "/devices/pci0000:00/0000:00:1f.2";

    memset(oversized_2x, 'A', sizeof(oversized_2x) - 1);
    oversized_2x[sizeof(oversized_2x) - 1] = '\0';

    memset(oversized_10x, 'B', sizeof(oversized_10x) - 1);
    oversized_10x[sizeof(oversized_10x) - 1] = '\0';

    memset(boundary, 'C', sizeof(boundary) - 1);
    boundary[sizeof(boundary) - 1] = '\0';

    const char *payloads[] = {
        oversized_10x,   /* 10x expected buffer - exploit case */
        oversized_2x,    /* 2x expected buffer */
        boundary,        /* boundary value at typical PATH_MAX/256 */
        valid            /* valid input */
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    struct udev *udev = udev_new();
    ck_assert_ptr_nonnull(udev);

    for (int i = 0; i < num_payloads; i++) {
        struct udev_device *dev = udev_device_new_from_devpath(udev, payloads[i]);
        /* Function should either return NULL (rejecting bad input)
           or return a valid device without crashing */
        if (dev != NULL) {
            const char *result = udev_device_get_devpath(dev);
            if (result != NULL) {
                ck_assert_uint_le(strlen(result), strlen(payloads[i]));
            }
            udev_device_unref(dev);
        }
    }

    udev_unref(udev);
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_buffer_overflow_devpath);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}