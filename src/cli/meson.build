cli_sources = [
    'main.c',
]

executable('linux-driver-management',
    sources: cli_sources,
    dependencies: [
        link_libldm_main,
        dep_glib2,
    ],
    install: true,
)
