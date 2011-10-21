# See LICENSE file for license and copyright information

include ../config.mk
include ../common.mk

SOURCE   = tests.c test_utils.c test_datastructures.c test_session.c
OBJECTS  = ${SOURCE:.c=-gtk${GIRARA_GTK_VERSION}.o}
DOBJECTS = ${SOURCE:.c=-gtk${GIRARA_GTK_VERSION}.do}

all: options girara_test-gtk${GIRARA_GTK_VERSION}
	$(ECHO) "running tests ..."
	$(QUIET)./girara_test-gtk${GIRARA_GTK_VERSION}

debug: options girara_test-gtk${GIRARA_GTK_VERSION}-debug
	$(ECHO) "running tests ..."
	$(QUIET)./girara_test-gtk${GIRARA_GTK_VERSION}-debug

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

%-gtk${GIRARA_GTK_VERSION}.o: %.c
	@mkdir -p .depend
	$(ECHO) CC $<
	$(QUIET)${CC} -c -I.. ${CFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

%-gtk${GIRARA_GTK_VERSION}.do: %.c
	@mkdir -p .depend
	$(ECHO) CC $<
	$(QUIET)${CC} -c -I.. ${CFLAGS} ${DFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

girara_test-gtk${GIRARA_GTK_VERSION}: ${OBJECTS} ../libgirara-gtk${GIRARA_GTK_VERSION}.a
	$(ECHO) LD $@
	$(QUIET)${CC} ${LDFLAGS} -o $@ ${OBJECTS} ../libgirara-gtk${GIRARA_GTK_VERSION}.a ${LIBS}

girara_test-gtk${GIRARA_GTK_VERSION}-debug: ${DOBJECTS} ../libgirara-gtk${GIRARA_GTK_VERSION}-debug.a
	$(ECHO) LD $@
	$(QUIET)${CC} ${LDFLAGS} -o $@ ${DOBJECTS} ../libgirara-gtk${GIRARA_GTK_VERSION}-debug.a ${LIBS}

${DOBJECTS} ${OBJECTS}: ../config.mk

clean:
	$(QUIET)rm -rf ${OBJECTS} ${DOBJECTS} girara_test-gtk${GIRARA_GTK_VERSION} \
		girara_test-gtk${GIRARA_GTK_VERSION}-debug .depend

.PHONY: all options clean debug

-include $(wildcard .depend/*.dep)
