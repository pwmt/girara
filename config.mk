# See LICENSE file for license and copyright information

VERSION = 0.0.0
SOMAJOR = 0
SOMINOR = 0
SOVERSION = ${SOMAJOR}.${SOMINOR}

# paths
PREFIX ?= /usr

GTK_VERSION ?= 2

# libs
GTK_INC = $(shell pkg-config --cflags gtk+-${GTK_VERSION}.0)
GTK_LIB = $(shell pkg-config --libs gtk+-${GTK_VERSION}.0)

INCS = -I. -I/usr/include ${GTK_INC}
LIBS = -lc ${GTK_LIB}

# flags
CFLAGS += -std=c99 -pedantic -Wall -Wextra -fPIC $(INCS)

# linker flags
LDFLAGS += -fPIC

# debug
DFLAGS = -O0 -g

# compiler
CC ?= gcc

# strip
SFLAGS = -s
