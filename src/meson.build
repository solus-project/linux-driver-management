subdir('lib')

cli_sources = [
    'cli/configure.c',
    'cli/main.c',
    'cli/status.c',
    'cli/version.c',
]

executable('linux-driver-management',
    sources: cli_sources,
    dependencies: link_libldm,
    install: true,
)
