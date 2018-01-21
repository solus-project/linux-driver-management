cli_sources = [
    'main.c',
    'configure.c',
    'status.c',
    'version.c',
]

executable('linux-driver-management',
    sources: cli_sources,
    dependencies: [
        link_libldm,
        dep_glib2,
    ],
    install: true,
)
