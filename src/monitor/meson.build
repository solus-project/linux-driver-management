monitor_sources = [
    'main.c',
]

executable('ldm-monitor',
    sources: monitor_sources,
    dependencies: [
        link_libldm,
        dep_gio_unix,
        dep_glib2,
        dep_gudev,
    ],
    install: true,
)
