gnome = import('gnome')
pkgconfig = import('pkgconfig')

libldm_main_sources = [
    'manager.c',
]

libldm_headers = [
    'manager.h',
]

libldm_main_includes = [
    include_directories('.'),
    config_h_dir,
]

libldm_dependencies = [
    dep_glib2,
    dep_gio_unix,
]

libldm_main = shared_library(
    'ldm',
    version: abi_version,
    sources: libldm_main_sources,
    include_directories: libldm_main_includes,
    dependencies: libldm_dependencies,
    install: true,
)

link_libldm_main = declare_dependency(
    link_with: libldm_main,
    include_directories: libldm_main_includes,
)

# Install our main headers
install_headers(
    libldm_headers,
    subdir: meson.project_name(),
)

# Generate .gir
libldm_gir = gnome.generate_gir(
    libldm_main,
    sources: libldm_main_sources + libldm_headers,
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
    ],
)

libldm_vapi = gnome.generate_vapi(
    'ldm-1.0',
    sources: [libldm_gir[0]],
    packages: [
        'glib-2.0',
        'gio-unix-2.0',
    ],
    install: true,
)

# Install pkgconfig file to enable building against budgie-plugin
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
