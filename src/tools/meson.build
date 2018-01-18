mkmodaliases_sources = [
    'mkmodaliases.c',
]

mkmodaliases = executable(
    'mkmodaliases',
    sources: mkmodaliases_sources,
    dependencies: [
        dep_glib2,
        dep_kmod,
    ],
    include_directories: [
        config_h_dir,
    ],
    install: true,
)
