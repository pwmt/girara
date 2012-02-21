# See LICENSE file for license and copyright information

include ../config.mk
include ../common.mk
include config.mk

SOURCE   = tests.c $(wildcard test_*.c)
OBJECTS  = ${SOURCE:.c=-gtk${GIRARA_GTK_VERSION}.o}
DOBJECTS = ${SOURCE:.c=-gtk${GIRARA_GTK_VERSION}.do}
XDG_HELPER_SOURCE = xdg_test_helper.c
XDG_HELPER = ${XDG_HELPER_SOURCE:.c=}

all: options girara_test-gtk${GIRARA_GTK_VERSION} ${XDG_HELPER}

tests: all
	$(ECHO) "running tests ..."
	$(QUIET)G_SLICE=debug-blocks ./girara_test-gtk${GIRARA_GTK_VERSION}

debug: options girara_test-gtk${GIRARA_GTK_VERSION}-debug ${XDG_HELPER}

tests-debug: debug
	$(ECHO) "running tests ..."
	$(QUIET)G_SLICE=debug-blocks ./girara_test-gtk${GIRARA_GTK_VERSION}-debug

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

%-gtk${GIRARA_GTK_VERSION}.o: %.c
	@mkdir -p .depend
	$(ECHO) CC $<
	$(QUIET)${CC} -c -I.. ${CPPFLAGS} ${CFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

${XDG_HELPER}: ${XDG_HELPER_SOURCE} ../libgirara-gtk${GIRARA_GTK_VERSION}.a
	@mkdir -p .depend
	$(ECHO) CC $<
	$(QUIET)${CC}  -I.. ${CPPFLAGS} ${CFLAGS} -o $@ $< ../libgirara-gtk${GIRARA_GTK_VERSION}.a ${LIBS} -MMD -MF .depend/$@.dep

%-gtk${GIRARA_GTK_VERSION}.do: %.c
	@mkdir -p .depend
	$(ECHO) CC $<
	$(QUIET)${CC} -c -I.. ${CPPFLAGS} ${CFLAGS} ${DFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

girara_test-gtk${GIRARA_GTK_VERSION}: ${OBJECTS} ../libgirara-gtk${GIRARA_GTK_VERSION}.a
	$(ECHO) LD $@
	$(QUIET)${CC} ${LDFLAGS} -o $@ ${OBJECTS} ../libgirara-gtk${GIRARA_GTK_VERSION}.a ${LIBS}

girara_test-gtk${GIRARA_GTK_VERSION}-debug: ${DOBJECTS} ../libgirara-gtk${GIRARA_GTK_VERSION}-debug.a
	$(ECHO) LD $@
	$(QUIET)${CC} ${LDFLAGS} -o $@ ${DOBJECTS} ../libgirara-gtk${GIRARA_GTK_VERSION}-debug.a ${LIBS}

${DOBJECTS} ${OBJECTS}: ../config.mk

clean:
	$(QUIET)rm -rf ${OBJECTS} ${DOBJECTS} girara_test-gtk${GIRARA_GTK_VERSION} \
		girara_test-gtk${GIRARA_GTK_VERSION}-debug .depend ${XDG_HELPER} *gcda *gcno

.PHONY: all options clean debug tests tests-debug

-include $(wildcard .depend/*.dep)
