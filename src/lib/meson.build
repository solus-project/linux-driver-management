gnome = import('gnome')
pkgconfig = import('pkgconfig')

# Generate enums for gobject typing
libldm_enums = gnome.mkenums(
    'ldm-enums',
    sources: [
        'device.h',
    ],
    c_template: 'ldm-enums.c.template',
    h_template: 'ldm-enums.h.template',
    install_header: true,
    install_dir: join_paths(get_option('includedir'), meson.project_name()),
)

# Fight meson race conditions..
libenum = static_library(
    'enum',
    libldm_enums,
    dependencies: dep_glib2,
)

link_libenum = declare_dependency(
    sources: libldm_enums,
    include_directories: include_directories('.'),
)

libldm_main_sources = [
    'device.c',
    'device-pci.c',
    'driver.c',
    'manager.c',
    'modalias.c',
    'usb-device.c',
    'drivers/modalias-driver.c',
]

libldm_headers = [
    'device.h',
    'driver.h',
    'manager.h',
    'modalias.h',
    'ldm.h',
    'usb-device.h',
]

libldm_driver_headers = [
    'drivers/modalias-driver.h',
]

libldm_main_includes = [
    include_directories('.'),
    config_h_dir,
]

libldm_dependencies = [
    link_libenum,
    dep_glib2,
    dep_gio_unix,
    dep_udev,
]

# Manually maintained symbol list.
sym_map = join_paths(meson.current_source_dir(), 'sym.map')

libldm_main = shared_library(
    'ldm',
    version: abi_version,
    sources: libldm_main_sources,
    link_args: ['-Wl,--version-script=@0@'.format(sym_map)],
    include_directories: libldm_main_includes,
    dependencies: libldm_dependencies,
    install: true,
)

link_libldm_main = declare_dependency(
    link_with: libldm_main,
    dependencies: [
        link_libenum,
        dep_glib2,
        dep_gio_unix,
    ],
    include_directories: libldm_main_includes,
)

# Install our main headers
install_headers(
    libldm_headers,
    subdir: meson.project_name(),
)
# Install driver headers
install_headers(
    libldm_driver_headers,
    subdir: '@0@/drivers'.format(meson.project_name()),
)

# Generate .gir
libldm_gir = gnome.generate_gir(
    libldm_main,
    sources: libldm_main_sources + libldm_headers + libldm_driver_headers + libldm_enums,
    namespace: 'Ldm',
    nsversion: '1.0',
    dependencies: libldm_dependencies,
    includes: [
        'GObject-2.0',
        'Gio-2.0',
    ],
    symbol_prefix: 'ldm',
    identifier_prefix: 'Ldm',
    install: true,
    extra_args: [
        '--warn-all',
        '--c-include', 'ldm.h',
    ],
)

libldm_vapi = gnome.generate_vapi(
    'ldm-1.0',
    sources: [libldm_gir[0]],
    packages: [
        'glib-2.0',
        'gio-unix-2.0',
    ],
    metadata_dirs: meson.current_source_dir(),
    install: true,
)

# Install pkgconfig file to enable building against libldm
pkgconfig.generate(
    name: 'ldm-1.0',
    description: 'Linux Driver Management Library',
    version: meson.project_version(),
    filebase: 'ldm-1.0',
    subdirs: meson.project_name(),
    libraries: ['-L${libdir}', '-lldm'],
    requires: [
        'glib-2.0 @0@'.format(glib_min_version),
        'gio-unix-2.0 @0@'.format(glib_min_version),
    ],
)
