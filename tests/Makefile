# See LICENSE file for license and copyright information

include ../config.mk

SOURCE   = tests.c test_utils.c test_datastructures.c test_session.c
OBJECTS  = ${SOURCE:.c=-gtk${GIRARA_GTK_VERSION}.o}

all: options girara_test-gtk${GIRARA_GTK_VERSION}
	@echo "running tests ..."
	@./girara_test-gtk${GIRARA_GTK_VERSION}

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

%-gtk${GIRARA_GTK_VERSION}.o: %.c
	@echo CC $<
	@mkdir -p .depend
	@${CC} -c -I.. ${CFLAGS} ${DFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

girara_test-gtk${GIRARA_GTK_VERSION}: ${OBJECTS} ../libgirara-gtk${GIRARA_GTK_VERSION}-debug.a
	@echo LD $@
	@${CC} ${LDFLAGS} -o $@ ${OBJECTS} ../libgirara-gtk${GIRARA_GTK_VERSION}-debug.a ${LIBS}

${OBJECTS}: ../config.mk

clean:
	@rm -rf ${OBJECTS} girara_test-gtk${GIRARA_GTK_VERSION} .depend

.PHONY: all options clean

-include $(wildcard .depend/*.dep)
