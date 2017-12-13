# Contains definitions for all of our tests
required_tests = [
    'modalias',
]

test_dependencies = [
    link_libldm_main,
    dep_check,
]

foreach test : required_tests
    t = executable(
        'test-@0@'.format(test),
        sources: [
            'check-@0@.c'.format(test),
        ],
        c_args: am_cflags,
        dependencies: test_dependencies,
        install: false,
    )
    test(test, t)
endforeach
