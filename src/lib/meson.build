libldm_main_sources = [
    'manager.c',
]

libldm_main_includes = [
    include_directories('.'),
    config_h_dir,
]

libldm_main = shared_library(
    'ldm-legacy',
    sources: libldm_main_sources,
    include_directories: libldm_main_includes,
    dependencies: [
        dep_glib2,
        dep_gio_unix,
    ],
    install: true,
)

link_libldm_main = declare_dependency(
    link_with: libldm_main,
    include_directories: libldm_main_includes,
)
