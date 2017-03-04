project(
    'linux-driver-management',
    ['c'],
    version: '0.0.1',
    license: [
        'LGPL-2.1',
    ],
    default_options: [
        'c_std=c11',
    ],
)

am_cflags = [
    '-fstack-protector',
    '-pedantic',
    '-Wstrict-prototypes',
    '-Wundef',
    '-Werror-implicit-function-declaration',
    '-Wformat',
    '-Wformat-security',
    '-Werror=format-security',
    '-Wconversion',
    '-Wunused-variable',
    '-Wunreachable-code',
    '-Wall',
    '-W',
    '-I' + join_paths(meson.current_source_dir(), 'src'),
]

add_global_arguments(am_cflags, language: 'c')

dep_pci = dependency('libpci')

executable('linux-driver-management',
           [
                'src/device.c',
                'src/display-manager.c',
                'src/dm/gdm.c',
                'src/dm/lightdm.c',
                'src/main.c',
                'src/scanner.c',
                'src/scan/pci.c',
           ],
           dependencies: [dep_pci],
           c_args: am_cflags,
           install: true,
)
