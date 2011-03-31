# See LICENSE file for license and copyright information

include ../config.mk

SOURCE   = example_1.c
OBJECTS  = ${SOURCE:.c=}

all: options ${OBJECTS}

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

%: %.c
	@echo CC $<
	@${CC} ${CFLAGS} -o $@ $< ../libgirara.a ${LIBS}

%-debug: %.c
	@echo CC $<
	@${CC} ${CFLAGS} ${DFLAGS} -o $@ $< ../libgirara-debug.a ${LIBS}

debug: options ${SOURCE:.c=-debug}

${OBJECTS}:  ../config.mk

clean:
	@rm -rf ${OBJECTS} example_1 example_1-debug
