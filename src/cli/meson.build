cli_sources = [
    'main.c',
    'configure.c',
    'status.c',
    'version.c',
]

if with_glx_configuration == true
    cli_sources += 'configure.c'
endif

executable('linux-driver-management',
    sources: cli_sources,
    dependencies: [
        link_libldm,
        dep_glib2,
    ],
    install: true,
)
