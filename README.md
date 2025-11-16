girara - user interface library
===============================

girara is a library that implements a user interface that focuses on simplicity
and minimalism. It consists of three main components: The view is a widget that
represents the actual application (e.g.: a web site or a document). The input
bar is used to execute commands of the application, while the status bar
notifies the user with current information. It is designed to replace and the
enhance the user interface that is used by zathura.

Requirements
------------

The following dependencies are required:

* `gtk3` (>= 3.24)
* `glib` (>= 2.72)

The following dependencies are optional:

* `json-glib-1.0`: configuration dumping support

For building girara, the following dependencies are also required:

* `meson` (>= 1.5)
* `gettext`
* `pkgconf`

The following dependencies are optional build-time only dependencies:

* `doxygen`: HTML documentation

To disable the optional support for `json-glib-1.0`, configure the build system
with `-Djson=disabled`.

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
