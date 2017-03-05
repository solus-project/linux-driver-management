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

conf_data = configuration_data()
conf_data.set_quoted('PACKAGE_VERSION', meson.project_version())
conf_data.set_quoted('PACKAGE_NAME', meson.project_name())

configure_file(
    output: 'config.h',
    configuration: conf_data,
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
                'src/drivers/gpu.c',
                'src/dm/display-manager.c',
                'src/dm/gdm.c',
                'src/dm/lightdm.c',
                'src/scanner.c',
                'src/scan/pci.c',
                'src/util.c',
                'src/cli/configure.c',
                'src/cli/main.c',
                'src/cli/status.c',
                'src/cli/version.c',
           ],
           dependencies: [dep_pci],
           c_args: am_cflags,
           install: true,
)
