# See LICENSE file for license and copyright information

include ../config.mk
include ../common.mk

SOURCE   = example_1.c example_tabs.c
OBJECTS  = ${SOURCE:.c=-gtk${GIRARA_GTK_VERSION}}

all: options ${OBJECTS}

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

%-gtk${GIRARA_GTK_VERSION}: %.c ../libgirara-gtk${GIRARA_GTK_VERSION}.a
	$(ECHO) CC $<
	@mkdir -p .depend
	$(QUIET)${CC} ${CPPFLAGS} ${CFLAGS} -o $@ $< ../libgirara-gtk${GIRARA_GTK_VERSION}.a  ${LIBS} -MMD -MF .depend/$@.dep

%-gtk${GIRARA_GTK_VERSION}-debug: %.c ../libgirara-gtk${GIRARA_GTK_VERSION}-debug.a
	$(ECHO) CC $<
	@mkdir -p .depend
	$(QUIET)${CC} ${CPPFLAGS} ${CFLAGS} ${DFLAGS} -o $@ $< ../libgirara-gtk${GIRARA_GTK_VERSION}-debug.a ${LIBS} -MMD -MF .depend/$@.dep

debug: options ${SOURCE:.c=-gtk${GIRARA_GTK_VERSION}-debug}

${OBJECTS}:  ../config.mk

clean:
	$(QUIET)rm -rf ${OBJECTS} example_1-gtk${GIRARA_GTK_VERSION} example_1-gtk${GIRARA_GTK_VERSION}-debug \
		example_tabs-gtk${GIRARA_GTK_VERSION} example_tabs-gtk${GIRARA_GTK_VERSION}-debug .depend

.PHONY: all options debug clean

-include $(wildcard .depend/*.dep)
