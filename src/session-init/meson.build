session_init_sources = [
    'main.c'
]

session_init_includes = [
    include_directories('.'),
    config_h_dir,
]

# Main session init binary used to conditionally apply xrandr control
# on X11 sessions.
session_init = executable(
    'ldm-session-init',
    sources: session_init_sources,
    include_directories: session_init_includes,
    dependencies: link_libldm,
    install: true,
)
