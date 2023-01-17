project('bbswitchd',
    version : '0.1.0',
    meson_version : '>= 0.53.0',
    default_options : ['warning_level=3', 'werror=true'],
)

# Required parts
subdir('src')
subdir('data')

# Build SELinux module if needed
if get_option('selinux').enabled()
    subdir('selinux')
endif

# Script for comminucating with daemon
install_data('bbswitch',
             install_mode : 'rwxr-xr-x',
             install_dir : get_option('prefix') / get_option('bindir'))
