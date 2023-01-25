Name:           bbswitchd
Version:        0.1.1
Release:        1%{?dist}
Summary:        Daemon for toggling discrete NVIDIA GPU power on Optimus laptops

License:        GPLv3+
URL:            https://github.com/polter-rnd/bbswitchd
Source0:        https://github.com/polter-rnd/bbswitchd/archive/%{version}/bbswitchd-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  meson >= 0.53.0
BuildRequires:  systemd
BuildRequires:  pkgconfig(libkmod)

Requires:       bbswitch-kmod
Recommends:     %{name}-selinux = %{version}-%{release}

%{?systemd_requires}


# UNIX socket group
%define socket_group bbswitchd
# Users from these will be added to socket group
%define admin_groups wheel


%description
Internally uses bbswitch kernel module to switch video adapter power state.

Useful for pre-Turing GPU generations without dynamic power management features,
allows to fully benefit from NVIDIA PRIME Render Offload technology
without need to keep graphics adapter turned on all the time.

%package selinux
Summary:        SELinux policies for bbswitchd

BuildRequires:  selinux-policy-devel

Requires(post): libselinux-utils
Requires(post): policycoreutils
Requires(post): policycoreutils-python-utils
Requires(post): selinux-policy-base >= %{_selinux_policy_version}

BuildArch:      noarch

%description selinux
SELinux policy module for use with bbswitchd.


%prep
%autosetup -p1 -n bbswitchd-%{version}


%build
%meson -Dselinux=enabled
%meson_build


%install
%meson_install


%post
%systemd_post bbswitchd.service
%systemd_post bbswitchd.socket

# On upgrade restart if running, on install just start
systemctl daemon-reload
if [ "${1}" -gt 1 ]; then
    systemctl try-restart bbswitchd.service
else
    # only add a group and members if the configured group does match the
    # default group and if the group is missing
    if grep -qx SocketGroup=%{socket_group} %{_unitdir}/bbswitchd.socket &&
        ! getent group %{socket_group} > /dev/null; then
        groupadd --system %{socket_group}
        users=$(getent -s files group %{admin_groups} | cut -d: -f4 | tr , '\n' | sort -u)
        for user in $users; do
            gpasswd -a $user %{socket_group}
        done
    fi

    systemctl start bbswitchd.service
fi

# Regenerate initramfs on install/upgrade
if [ -x "$(which dracut)" ]; then
    dracut -f
fi

%preun
%systemd_preun bbswitchd.service
%systemd_preun bbswitchd.socket


%postun
%systemd_postun bbswitchd.service
%systemd_postun bbswitchd.socket

# Regenerate initramfs on uninstall
if [ "${1}" -eq 0 ] && [ -x "$(which dracut)" ]; then
    dracut -f
fi

# Remove group
groupdel %{socket_group} || true


%post selinux
%selinux_modules_install %{_datadir}/selinux/packages/bbswitchd.pp


%postun selinux
if [ "${1}" -eq 0 ]; then
    %selinux_modules_uninstall bbswitchd
fi


%files
%license LICENSE
%{_bindir}/bbswitch
%{_sbindir}/bbswitchd
%{_prefix}/lib/modprobe.d/bbswitchd-blacklist.conf
%{_unitdir}/bbswitchd.service
%{_unitdir}/bbswitchd.socket
%{_presetdir}/90-bbswitchd.preset
%{_udevrulesdir}/60-bbswitchd-nvidia-mutter.rules
%{_udevrulesdir}/90-bbswitchd-nvidia-dev.rules


%files selinux
%{_datadir}/selinux/packages/bbswitchd.pp
%{_datadir}/selinux/devel/include/contrib/bbswitchd.if


%changelog
* Thu Jan 26 2023 Pavel Artsishevsky <polter.rnd@gmail.com> - 0.1.1-1
- Various packaging updates

* Tue Jan 17 2023 Pavel Artsishevsky <polter.rnd@gmail.com> - 0.1.0-1
- Initial release
