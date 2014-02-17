# See LICENSE file for license and copyright information

include ../config.mk
include ../common.mk
include config.mk

SOURCE   = tests.c $(wildcard test_*.c)
OBJECTS  = ${SOURCE:.c=.o}
DOBJECTS = ${SOURCE:.c=.do}
XDG_HELPER_SOURCE = xdg_test_helper.c
XDG_HELPER = ${XDG_HELPER_SOURCE:.c=}

all: options girara_test ${XDG_HELPER}

run: girara_test ${XDG_HELPER}
	$(ECHO) "running tests ..."
	$(QUIET)G_SLICE=debug-blocks ./girara_test

debug: options girara_test-debug ${XDG_HELPER}

run-debug: debug
	$(ECHO) "running tests ..."
	$(QUIET)G_SLICE=debug-blocks ./girara_test-debug

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

%.o: %.c
	@mkdir -p .depend
	$(ECHO) CC $<
	$(QUIET)${CC} -c -I.. ${CPPFLAGS} ${CFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

${XDG_HELPER}: ${XDG_HELPER_SOURCE} ../libgirara-gtk3.a
	@mkdir -p .depend
	$(ECHO) CC $<
	$(QUIET)${CC}  -I.. ${CPPFLAGS} ${CFLAGS} -o $@ $< ../libgirara-gtk3.a ${LDFLAGS} ${LIBS} -MMD -MF .depend/$@.dep

%.do: %.c
	@mkdir -p .depend
	$(ECHO) CC $<
	$(QUIET)${CC} -c -I.. ${CPPFLAGS} ${CFLAGS} ${DFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

girara_test: ${OBJECTS} ../libgirara-gtk3.a
	$(ECHO) LD $@
	$(QUIET)${CC} ${LDFLAGS} -o $@ ${OBJECTS} ../libgirara-gtk3.a ${LIBS}

girara_test-debug: ${DOBJECTS} ../libgirara-gtk3-debug.a
	$(ECHO) LD $@
	$(QUIET)${CC} ${LDFLAGS} -o $@ ${DOBJECTS} ../libgirara-gtk3-debug.a ${LIBS}

${DOBJECTS} ${OBJECTS}: ../config.mk

clean:
	$(QUIET)rm -rf ${OBJECTS} ${DOBJECTS} girara_test \
		girara_test-debug .depend ${XDG_HELPER} *gcda *gcno

.PHONY: all options clean debug run run-debug

-include $(wildcard .depend/*.dep)
