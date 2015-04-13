# See LICENSE file for license and copyright information

CHECK_INC ?= $(shell pkg-config --cflags check)
CHECK_LIB ?= $(shell pkg-config --libs check)

LIBS += ${CHECK_LIB}

GIRARA_RELEASE=../${BUILDDIR_RELEASE}/girara
GIRARA_DEBUG=../${BUILDDIR_DEBUG}/girara
GIRARA_GCOV=../${BUILDDIR_GCOV}/girara
