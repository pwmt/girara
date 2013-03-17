# See LICENSE file for license and copyright information

GIRARA_VERSION_MAJOR = 0
GIRARA_VERSION_MINOR = 1
GIRARA_VERSION_REV   = 5
VERSION = ${GIRARA_VERSION_MAJOR}.${GIRARA_VERSION_MINOR}.${GIRARA_VERSION_REV}
# Rules for the SOMAJOR and SOMINOR.
# Before a release check perform the following checks against the last release:
# * If a function has been removed or the paramaters of a function have changed
#   bump SOMAJOR and set SOMINOR to 0.
# * If any of the exported datastructures have changed in a incompatible way
# 	bump SOMAJOR and set SOMINOR to 0.
# * If a function has been added bump SOMINOR.
SOMAJOR = 1
SOMINOR = 1
SOVERSION = ${SOMAJOR}.${SOMINOR}

# paths
PREFIX ?= /usr
LIBDIR ?= ${PREFIX}/lib
INCLUDEDIR ?= ${PREFIX}/include
# locale directory
LOCALEDIR ?= ${PREFIX}/share/locale

GIRARA_GTK_VERSION ?= 2

# libs
GTK_INC ?= $(shell pkg-config --cflags gtk+-${GIRARA_GTK_VERSION}.0)
GTK_LIB ?= $(shell pkg-config --libs gtk+-${GIRARA_GTK_VERSION}.0)

INCS = ${GTK_INC}
LIBS = ${GTK_LIB} -lm

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
