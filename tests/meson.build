# Contains definitions for all of our tests
required_tests = [
    'modalias',
    'manager',
    'usb',
]

test_dependencies = [
    link_libldm_main,
    dep_check,
    dep_umockdev,
]

test_data_root = join_paths(meson.current_source_dir(), 'data')

test_flags = [
    '-DTEST_DATA_ROOT="@0@"'.format(test_data_root),
]

foreach test : required_tests
    t = executable(
        'test-@0@'.format(test),
        sources: [
            'check-@0@.c'.format(test),
        ],
        c_args: am_cflags + test_flags,
        dependencies: test_dependencies,
        install: false,
    )
    test(test, run_umockdev, args: [t.full_path()])
endforeach
