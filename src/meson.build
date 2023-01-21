add_languages('c')

message('Checking dependencies')
kmod = dependency('libkmod')

project_sources = [
    'bbswitch.c',
    'logger.c',
    'main.c',
    'module.c',
    'pci.c',
    'server.c'
]

executable(meson.project_name(),
           project_sources,
           dependencies : kmod,
           install : true,
           install_dir : get_option('prefix') / get_option('sbindir'))
