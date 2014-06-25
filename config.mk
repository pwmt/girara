# See LICENSE file for license and copyright information

GIRARA_VERSION_MAJOR = 0
GIRARA_VERSION_MINOR = 2
GIRARA_VERSION_REV   = 1
VERSION = ${GIRARA_VERSION_MAJOR}.${GIRARA_VERSION_MINOR}.${GIRARA_VERSION_REV}

# Rules for the SOMAJOR and SOMINOR.
# Before a release check perform the following checks against the last release:
# * If a function has been removed or the paramaters of a function have changed
#   bump SOMAJOR and set SOMINOR to 0.
# * If any of the exported datastructures have changed in a incompatible way
#   bump SOMAJOR and set SOMINOR to 0.
# * If a function has been added bump SOMINOR.
SOMAJOR = 1
SOMINOR = 1
SOVERSION = ${SOMAJOR}.${SOMINOR}

# libnotify
WITH_LIBNOTIFY ?= $(shell (pkg-config libnotify && echo 1) || echo 0)

# paths
PREFIX ?= /usr
LIBDIR ?= ${PREFIX}/lib
INCLUDEDIR ?= ${PREFIX}/include

# locale directory
LOCALEDIR ?= ${PREFIX}/share/locale

# version checks
# If you want to disable any of the checks, set *_VERSION_CHECK to 0.

# GTK+
GTK_VERSION_CHECK ?= 1
GTK_MIN_VERSION = 3.2
GTK_PKG_CONFIG_NAME = gtk+-3.0
# glib
GLIB_VERSION_CHECK ?= 1
GLIB_MIN_VERSION = 2.28
GLIB_PKG_CONFIG_NAME = glib-2.0

# libs
GTK_INC ?= $(shell pkg-config --cflags gtk+-3.0)
GTK_LIB ?= $(shell pkg-config --libs gtk+-3.0)

ifneq (${WITH_LIBNOTIFY},0)
LIBNOTIFY_INC ?= $(shell pkg-config --cflags libnotify)
LIBNOTIFY_LIB ?= $(shell pkg-config --libs libnotify)
endif

INCS = ${GTK_INC} ${LIBNOTIFY_INC}
LIBS = ${GTK_LIB} ${LIBNOTIFY_LIB} -lm

# flags
CFLAGS += -std=c99 -pedantic -Wall -Wextra -fPIC $(INCS)

# linker flags
LDFLAGS += -fPIC

# debug
DFLAGS = -O0 -g

# compiler
CC ?= gcc

# strip
SFLAGS ?= -s

# set to something != 0 if you want verbose build output
VERBOSE ?= 0

# gettext package name
GETTEXT_PACKAGE ?= lib${PROJECT}-${SOMAJOR}

# msgfmt
MSGFMT ?= msgfmt

# colors
COLOR ?= 1

# dist
TARFILE = ${PROJECTNV}-${VERSION}.tar.gz
TARDIR = ${PROJECTNV}-${VERSION}
