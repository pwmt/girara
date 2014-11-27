# See LICENSE file for license and copyright information

include ../config.mk
include ../common.mk
include ../colors.mk
include config.mk

DOXYGEN_SOURCES=$(wildcard ../*.h) Doxyfile

all: html
html: ${DOXYGEN_BUILDDIR}/html/index.html

clean:
	$(call colorecho,RM,doc/${DOXYGEN_BUILDDIR})
	$(QUIET)rm -rf ${DOXYGEN_BUILDDIR}

${DOXYGEN_BUILDDIR}/html/index.html:
	$(call colorecho,DOC,"Build HTML documentation")
	$(QUIET)${DOXYGEN_BIN} Doxyfile

.PHONY: clean html all
