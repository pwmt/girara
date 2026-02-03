girara - common components for zathura
======================================

girara was a library that implemented a user interface that focuses on simplicity
and minimalism. It consisted of three main components: The view is a widget that
represents the actual application (e.g.: a web site or a document). The input
bar is used to execute commands of the application, while the status bar
notifies the user with current information. It was designed to replace and the
enhance the user interface that is used by zathura.

The user interface library has been integrated into zathura. girara now contains common
datastructures and utilities for the zathura ecosystem.

Requirements
------------

The following dependencies are required:

* `glib` (>= 2.72)

For building girara, the following dependencies are also required:

* `meson` (>= 1.5)
* `pkgconf`

The following dependencies are optional build-time only dependencies:

* `doxygen`: HTML documentation

Installation
------------

Run the following command to build and install girara to your system using
meson's ninja backend:

    meson build
    cd build
    ninja
    ninja install

Note that the default backend for meson might vary based on the platform. Please
refer to the meson documentation for platform specific dependencies.

Bugs
----

Please report bugs at https://github.com/pwmt/girara.
