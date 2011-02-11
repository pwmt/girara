# See LICENSE file for license and copyright information

include ../config.mk

SOURCE   = tests.c test_utils.c test_datastructures.c test_session.c
OBJECTS  = ${SOURCE:.c=.o}

all: options girara_test
	@echo "running tests ..."
	@./girara_test

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

%.o: %.c
	@echo CC $<
	@${CC} -c -I.. ${CFLAGS} ${DFLAGS} -o $@ $<

girara_test: ${OBJECTS} ../libgirara-debug.a
	@echo LD $@
	@${CC} ${LDFLAGS} ${LIBS} -o $@ ${OBJECTS} ../libgirara-debug.a

${OBJECTS}: ../config.mk

clean:
	@rm -rf ${OBJECTS} girara_test
