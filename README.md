# bbswitchd

Daemon for managing discrete NVIDIA GPU power state on Optimus laptops.

Useful for pre-Turing GPU generations without dynamic power management features,
allows to fully benefit from NVIDIA
[PRIME Render Offload](https://download.nvidia.com/XFree86/Linux-x86_64/495.46/README/primerenderoffload.html)
technology without need to keep graphics adapter turned on all the time.

For Turing generation cards with Intel Coffee Lake or above CPUs as well as some
Ryzen CPUs like the 5800H, it is possible to fully power down the GPU when not in use
automatically without user interaction:
[see NVIDIA documentation](https://us.download.nvidia.com/XFree86/Linux-x86_64/495.46/README/dynamicpowermanagement.html).

Uses [bbswitch](https://github.com/Bumblebee-Project/bbswitch) kernel module
to switch video adapter power state (`ON` or `OFF`).

## Installation

#### Requirements

First you have to install `kmod` library and `meson` build system.
If you need SELinux module, `selinux-policy` developent package has to be installed as well.

For Fedora:

```bash
$ sudo dnf install kmod-devel meson # selinux-policy-devel
```

For Ubuntu:

```bash
$ sudo apt-get install libkmod-dev meson # selinux-policy-dev
```

#### Building from source

Then build the daemon (in `build` subdirectory). If you need SELinux module
(which is usually required for RHEL-based distributions such as Fedora),
add `-Dselinux=enabled` option to `meson build` command:

```bash
$ meson build # -Dselinux=enabled
$ meson compile -C build
```

To install a daemon on your system:

```bash
$ meson install -C build
```

If you have built SELinux module, you need to load it after installation:

```bash
$ sudo semodule -n -i /usr/share/selinux/packages/bbswitchd.pp
$ sudo /usr/sbin/load_policy
$ sudo restorecon -R /usr/sbin/bbswitchd
```

## Usage

First create `bbswitchd` group and add yourself to it:

```bash
$ sudo groupadd -r bbswitchd
$ sudo gpasswd -a $(whoami) bbswitchd
```

Then enable and start the socket:

```bash
$ sudo systemctl enable bbswitchd.socket
$ sudo systemctl start bbswitchd.socket
```

After re-login you should be able to use `bbswitch` utility from your regular user:

```bash
$ bbswitch
bbswitch
Usage: bbswitch on | off | status
$ bbswitch on     # Turn discrete GPU on
$ bbswitch status # Request current status
ON
$ bbswitch off    # Turn discrete GPU off
```

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License

This software is distributed under [GNU GPL v3](https://www.gnu.org/licenses/gpl-3.0.en.html).
See `LICENSE` file for more information.