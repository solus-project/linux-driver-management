mkmodalias_sources = [
    'mkmodalias.c',
]

mkmodalias = executable(
    'mkmodalias',
    sources: mkmodalias_sources,
    dependencies: [
        dep_glib2,
        dep_kmod,
    ],
    include_directories: [
        config_h_dir,
    ],
    install: true,
)
