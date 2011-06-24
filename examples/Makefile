# See LICENSE file for license and copyright information

include ../config.mk

SOURCE   = example_1.c example_tabs.c
OBJECTS  = ${SOURCE:.c=-gtk${GTK_VERSION}}

all: options ${OBJECTS}

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

%-gtk${GTK_VERSION}: %.c ../libgirara-gtk${GTK_VERSION}.a
	@echo CC $<
	@mkdir -p .depend
	@${CC} ${CFLAGS} -o $@ $< ../libgirara-gtk${GTK_VERSION}.a  ${LIBS} -MMD -MF .depend/$@.dep

%-gtk${GTK_VERSION}-debug: %.c ../libgirara-gtk${GTK_VERSION}-debug.a
	@echo CC $<
	@mkdir -p .depend
	@${CC} ${CFLAGS} ${DFLAGS} -o $@ $< ../libgirara-gtk${GTK_VERSION}-debug.a ${LIBS} -MMD -MF .depend/$@.dep

debug: options ${SOURCE:.c=-gtk${GTK_VERSION}-debug}

${OBJECTS}:  ../config.mk

clean:
	@rm -rf ${OBJECTS} example_1-gtk${GTK_VERSION} example_1-gtk${GTK_VERSION}-debug \
		example_tabs-gtk${GTK_VERSION} example_tabs-gtk${GTK_VERSION}-debug .depend

.PHONY: all options debug clean

-include $(wildcard .depend/*.dep)
