# See LICENSE file for license and copyright information

include ../config.mk

SOURCE   = 
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
	@${CC} ${CFLAGS} -o $@ $< ../${ITLIB}

${OBJECTS}:  ../config.mk

clean:
	@rm -rf ${OBJECTS}
