# See LICENSE file for license and copyright information

include ../config.mk
include ../common.mk
include ../colors.mk

include config.mk

PROJECT  = test
SOURCE   = tests.c $(wildcard test_*.c)
OBJECTS       = $(addprefix ${BUILDDIR_RELEASE}/,${SOURCE:.c=.o})
OBJECTS_DEBUG = $(addprefix ${BUILDDIR_DEBUG}/,${SOURCE:.c=.o})
OBJECTS_GCOV  = $(addprefix ${BUILDDIR_GCOV}/,${SOURCE:.c=.o})

GIRARA_OBJECTS = $(wildcard ${GIRARA_RELEASE}/*.o)
GIRARA_OBJECTS_DEBUG = $(wildcard ${GIRARA_DEBUG}/*.o)
GIRARA_OBJECTS_GCOV = $(wildcard ${GIRARA_GCOV}/*.o)

XDG_HELPER_SOURCE = xdg_test_helper.c
XDG_HELPER = $(addprefix ${BUILDDIR_RELEASE}/,${XDG_HELPER_SOURCE:.c=})

CPPFLAGS += -I../girara -DXDG_TEST_HELPER=\"${XDG_HELPER}\"

all: release ${XDG_HELPER}

../girara/version.h:
	$(MAKE) -C .. girara/version.h

# release

${OBJECTS}: config.mk ../config.mk ../girara/version.h

${BUILDDIR_RELEASE}/%.o: %.c
	$(call colorecho,CC,$<)
	@mkdir -p ${DEPENDDIR}/$(dir $(abspath $@))
	@mkdir -p $(dir $(abspath $@))
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} \
		-o $@ $< -MMD -MF ${DEPENDDIR}/$(abspath $@).dep

${BUILDDIR_RELEASE}/${BINDIR}/${PROJECT}: ${OBJECTS}
	$(QUIET)$(MAKE) -C .. release
	$(call colorecho,CC,$@)
	@mkdir -p ${BUILDDIR_RELEASE}/${BINDIR}
	$(QUIET)${CC} ${SFLAGS} ${LDFLAGS} \
		-o ${BUILDDIR_RELEASE}/${BINDIR}/${PROJECT} \
		${OBJECTS} ${GIRARA_OBJECTS} ${LIBS}

release: ${BUILDDIR_RELEASE}/${BINDIR}/${PROJECT}

run: release ${XDG_HELPER}
	$(QUIET)./${BUILDDIR_RELEASE}/${BINDIR}/${PROJECT}

# debug

${OBJECTS_DEBUG}: config.mk ../config.mk ../girara/version.h

${BUILDDIR_DEBUG}/%.o: %.c
	@mkdir -p ${DEPENDDIR}/$(dir $(abspath $@))
	@mkdir -p $(dir $(abspath $@))
	$(call colorecho,CC,$<)
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} ${DFLAGS} \
		-o $@ $< -MMD -MF ${DEPENDDIR}/$(abspath $@).dep

${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT}: ${OBJECTS_DEBUG}
	$(QUIET)$(MAKE) -C .. debug
	$(call colorecho,CC,$@)
	@mkdir -p ${BUILDDIR_DEBUG}/${BINDIR}
	$(QUIET)${CC} ${SFLAGS} ${LDFLAGS} \
		-o ${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT} \
		${OBJECTS} ${GIRARA_OBJECTS_DEBUG} ${LIBS}

debug: ${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT}

run-debug: debug ${XDG_HELPER}
	$(QUIET)./${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT}

# gcov

${OBJECTS_GCOV}: config.mk ../config.mk ../girara/version.h

${BUILDDIR_GCOV}/%.o: %.c
	@mkdir -p ${DEPENDDIR}/$(dir $(abspath $@))
	@mkdir -p $(dir $(abspath $@))
	$(call colorecho,CC,$<)
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} ${GCOV_CFLAGS} ${DFLAGS} ${GCOV_DFLAGS} \
		-o $@ $< -MMD -MF ${DEPENDDIR}/$(abspath $@).dep

${BUILDDIR_GCOV}/${BINDIR}/${PROJECT}: ${OBJECTS_GCOV}
	$(QUIET)$(MAKE) -C .. gcov
	$(call colorecho,CC,$@)
	@mkdir -p ${BUILDDIR_GCOV}/${BINDIR}
	$(QUIET)${CC} ${SFLAGS} ${LDFLAGS} ${GCOV_CFLAGS} ${GCOV_LDFLAGS} \
		-o ${BUILDDIR_GCOV}/${BINDIR}/${PROJECT} \
		${OBJECTS} ${GIRARA_OBJECTS_GCOV} ${LIBS}

gcov: ${BUILDDIR_GCOV}/${BINDIR}/${PROJECT}

run-gcov: gcov
	$(QUIET)./${BUILDDIR_GCOV}/${BINDIR}/${PROJECT}

# XDG test helper

${XDG_HELPER}: ${XDG_HELPER_SOURCE}
	$(MAKE) -C .. release
	@mkdir -p ${DEPENDDIR}/$(dir $(abspath $@))
	@mkdir -p $(dir $(abspath $@))
	$(call colorecho,CC,$@)
	$(QUIET)${CC} ${CPPFLAGS} ${CFLAGS} -o $@ $< ${GIRARA_OBJECTS} \
		${LDFLAGS} ${LIBS} -MMD -MF ${DEPENDDIR}/$(abspath $@).dep

clean:
	$(call colorecho,RM, "Clean test files")
	$(QUIET)rm -rf ${PROJECT}
	$(QUIET)rm -rf ${PROJECT}-debug
	$(QUIET)rm -rf ${PROJECT}-gcov
	$(QUIET)rm -rf ${BUILDDIR}
	$(QUIET)rm -rf ${DEPENDDIR}

.PHONY: all clean debug gciv run run-debug run-gcov

-include $(wildcard ${DEPENDDIR}/*.dep)
