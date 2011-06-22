# See LICENSE file for license and copyright information

include ../config.mk

SOURCE   = example_1.c example_tabs.c
OBJECTS  = ${SOURCE:.c=}

all: options ${OBJECTS}

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

%: %.c ../libgirara.a
	@echo CC $<
	@mkdir -p .depend
	@${CC} ${CFLAGS} -o $@ $< ../libgirara.a  ${LIBS} -MMD -MF .depend/$@.dep

%-debug: %.c ../libgirara-debug.a
	@echo CC $<
	@mkdir -p .depend
	@${CC} ${CFLAGS} ${DFLAGS} -o $@ $< ../libgirara-debug.a ${LIBS} -MMD -MF .depend/$@.dep

debug: options ${SOURCE:.c=-debug}

${OBJECTS}:  ../config.mk

clean:
	@rm -rf ${OBJECTS} example_1 example_1-debug .depend

.PHONY: all options debug clean

-include $(wildcard .depend/*.dep)
