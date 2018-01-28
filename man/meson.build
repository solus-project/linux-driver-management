manpages = [
    'linux-driver-management.1',
]

if enable_tools == true
    manpages += 'mkmodaliases.1'
endif

if with_glx_configuration == true
    manpages += 'ldm-session-init.1'
endif

install_data(
    manpages,
    install_dir: join_paths(path_mandir, 'man1')
)
