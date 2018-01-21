monitor_sources = [
    'main.c',
    'monitor.c',
]

executable('ldm-monitor',
    sources: monitor_sources,
    dependencies: [
        link_libldm,
        dep_gio_unix,
        dep_glib2,
    ],
    install: true,
)
