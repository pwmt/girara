# See LICENSE file for license and copyright information

include config.mk
include colors.mk
include common.mk

PROJECTNV     = girara
PROJECT       = girara-gtk3

SOURCE        = $(wildcard ${PROJECTNV}/*.c)
CSOURCE       = $(filter-out ${PROJECTNV}/css-definitions.c, ${SOURCE})

OBJECTS       = $(addprefix ${BUILDDIR_RELEASE}/,${CSOURCE:.c=.o}) \
	${BUILDDIR_RELEASE}/${PROJECTNV}/css-definitions.o
OBJECTS_DEBUG = $(addprefix ${BUILDDIR_DEBUG}/,${CSOURCE:.c=.o}) \
	${BUILDDIR_DEBUG}/${PROJECTNV}/css-definitions.o
OBJECTS_GCOV  = $(addprefix ${BUILDDIR_GCOV}/,${CSOURCE:.c=.o}) \
	${BUILDDIR_GCOV}/${PROJECTNV}/css-definitions.o

HEADERS       = $(filter-out ${PROJECTNV}/internal.h, $(wildcard ${PROJECTNV}/*.h))

ifneq (${WITH_LIBNOTIFY},0)
CPPFLAGS += -DWITH_LIBNOTIFY
LIBNOTIFY_PC_NAME = libnotify
else
LIBNOTIFY_PC_NAME =
endif

ifneq (${WITH_JSON},0)
CPPFLAGS += -DWITH_JSON
JSON_PC_NAME = json-c
else
JSON_PC_NAME =
endif

ifeq (,$(findstring -DGETTEXT_PACKAGE,${CPPFLAGS}))
CPPFLAGS += -DGETTEXT_PACKAGE=\"${GETTEXT_PACKAGE}\"
endif
ifeq (,$(findstring -DLOCALEDIR,${CPPFLAGS}))
CPPFLAGS += -DLOCALEDIR=\"${LOCALEDIR}\"
endif

UNAME := $(shell uname -s)
ifeq ($(UNAME), Darwin)
SONAME_FLAG = -install_name
SHARED_FLAG = -dynamiclib
SOFILE = lib${PROJECT}.dylib
SOVERSIONFILE = lib${PROJECT}.${SOVERSION}.dylib
SOMAJORFILE = lib${PROJECT}.${SOMAJOR}.dylib
BUILDSOMAJORFILE = ${LIBDIR}/${SOMAJORFILE}
PLATFORMFLAGS = -current_version ${SOVERSION} -compatibility_version ${SOMAJOR}
else
SOFILE = lib${PROJECT}.so
SOVERSIONFILE = ${SOFILE}.${SOVERSION}
SOMAJORFILE = ${SOFILE}.${SOMAJOR}
BUILDSOMAJORFILE = ${SOMAJORFILE}
PLATFORMFLAGS =
endif

all: ${PROJECTNV} ${BUILDDIR}/${PROJECT}.pc po

# pkg-config based version checks
.version-checks/%: config.mk
	$(QUIET)test $($(*)_VERSION_CHECK) -eq 0 || \
		${PKG_CONFIG} --atleast-version $($(*)_MIN_VERSION) $($(*)_PKG_CONFIG_NAME) || ( \
		echo "The minimum required version of $(*) is $($(*)_MIN_VERSION)" && \
		false \
	)
	@mkdir -p .version-checks
	$(QUIET)touch $@

options:
	@echo ${PROJECTNV} build options:
	@echo "CFLAGS             = ${CFLAGS}"
	@echo "LDFLAGS            = ${LDFLAGS}"
	@echo "DFLAGS             = ${DFLAGS}"
	@echo "CC                 = ${CC}"
	@echo "PLATFORMFLAGS      = ${PLATFORMFLAGS}"

# generated files

${PROJECTNV}/version.h: ${PROJECTNV}/version.h.in config.mk
	$(call colorecho,GEN,$@)
	$(QUIET)sed \
		-e 's,@GVMAJOR@,${GIRARA_VERSION_MAJOR},' \
		-e 's,@GVMINOR@,${GIRARA_VERSION_MINOR},' \
		-e 's,@GVREV@,${GIRARA_VERSION_REV},' \
		${PROJECTNV}/version.h.in > ${PROJECTNV}/version.h.tmp
	$(QUIET)mv ${PROJECTNV}/version.h.tmp ${PROJECTNV}/version.h

${PROJECTNV}/css-definitions.c: data/girara-pre-3.20.css_t
	$(call colorecho,GEN,$@)
	$(QUIET)echo '#include "css-definitions.h"' > $@.tmp
	$(QUIET)echo 'const char* CSS_TEMPLATE_PRE_3_20 =' >> $@.tmp
	$(QUIET)sed 's/^\(.*\)$$/"\1\\n"/' data/girara-pre-3.20.css_t >> $@.tmp
	$(QUIET)echo ';' >> $@.tmp
	$(QUIET)echo 'const char* CSS_TEMPLATE_POST_3_20 =' >> $@.tmp
	$(QUIET)sed 's/^\(.*\)$$/"\1\\n"/' data/girara-post-3.20.css_t >> $@.tmp
	$(QUIET)echo ';' >> $@.tmp

	$(QUIET)mv $@.tmp $@

${BUILDDIR}/${PROJECT}.pc: ${PROJECTNV}.pc.in config.mk
	$(call colorecho,GEN,$(shell basename $@))
	@mkdir -p ${BUILDDIR}
	$(QUIET)sed -e 's,@PROJECT@,${PROJECT},' \
		-e 's,@VERSION@,${VERSION},' \
		-e 's,@INCLUDEDIR@,${INCLUDEDIR},' \
		-e 's,@LIBDIR@,${LIBDIR},' \
		-e 's,@LIBNOTIFY_PC_NAME@,${LIBNOTIFY_PC_NAME},' \
		-e 's,@JSON_PC_NAME@,${JSON_PC_NAME},' \
		${PROJECTNV}.pc.in > $@.tmp
	$(QUIET)mv $@.tmp $@

# release build

${OBJECTS}: config.mk \
	${PROJECTNV}/version.h \
	.version-checks/GTK \
	.version-checks/GLIB

${BUILDDIR_RELEASE}/%.o: %.c
	$(call colorecho,CC,$<)
	@mkdir -p ${DEPENDDIR}/$(dir $@)
	@mkdir -p $(dir $(abspath $@))
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} -o $@ $< \
		-MMD -MF ${DEPENDDIR}/$@.dep

${BUILDDIR_RELEASE}/${BINDIR}/lib${PROJECT}.a: ${OBJECTS}
	$(call colorecho,AR,$@)
	@mkdir -p ${BUILDDIR_RELEASE}/${BINDIR}
	$(QUIET)${AR} rcs $@ ${OBJECTS}

${BUILDDIR_RELEASE}/${BINDIR}/${SOVERSIONFILE}: ${OBJECTS}
	$(call colorecho,LD,$@)
	@mkdir -p ${BUILDDIR_RELEASE}/${BINDIR}
	$(QUIET)${CC} -Wl,${SONAME_FLAG},${BUILDSOMAJORFILE} \
		${SHARED_FLAG} ${PLATFORMFLAGS} ${LDFLAGS} -o $@ ${OBJECTS} ${LIBS}

${PROJECT}: static shared
${PROJECTNV}: ${PROJECT}
static: ${BUILDDIR_RELEASE}/${BINDIR}/lib${PROJECT}.a
shared: ${BUILDDIR_RELEASE}/${BINDIR}/${SOVERSIONFILE}
release: ${PROJECT}

# debug build

${OBJECT_DEBUG}: config.mk \
	${PROJECTNV}/version.h \
	.version-checks/GTK \
	.version-checks/GLIB

${BUILDDIR_DEBUG}/%.o: %.c
	$(call colorecho,CC,$<)
	@mkdir -p ${DEPENDDIR}/$(dir $@)
	@mkdir -p $(dir $(abspath $@))
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} ${DFLAGS} -o $@ $< \
		-MMD -MF ${DEPENDDIR}/$@.dep

${BUILDDIR_DEBUG}/${BINDIR}/lib${PROJECT}.a: ${OBJECTS_DEBUG}
	$(call colorecho,AR,$@)
	@mkdir -p ${BUILDDIR_DEBUG}/${BINDIR}
	$(QUIET)${AR} rcs $@ ${OBJECTS_DEBUG}

${BUILDDIR_DEBUG}/${BINDIR}/${SOVERSIONFILE}: ${OBJECTS_DEBUG}
	$(call colorecho,LD,$@)
	@mkdir -p ${BUILDDIR_DEBUG}/${BINDIR}
	$(QUIET)${CC} -Wl,${SONAME_FLAG},${BUILDSOMAJORFILE} ${SHARED_FLAG} ${PLATFORMFLAGS} ${LDFLAGS} -o $@ ${OBJECTS_DEBUG} ${LIBS}

${PROJECT}-debug: \
	${BUILDDIR_DEBUG}/${BINDIR}/lib${PROJECT}.a \
	${BUILDDIR_DEBUG}/${BINDIR}/${SOVERSIONFILE}
debug: ${PROJECT}-debug

# gcov build

${OBJECTS_GCOV}: config.mk \
	${PROJECTNV}/version.h \
	.version-checks/GLIB \
	.version-checks/GTK

${BUILDDIR_GCOV}/%.o: %.c
	$(call colorecho,CC,$<)
	@mkdir -p ${DEPENDDIR}/$(dir $@)
	@mkdir -p $(dir $(abspath $@))
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} ${GCOV_CFLAGS} \
		-o $@ $< -MMD -MF ${DEPENDDIR}/$@.dep

${BUILDDIR_GCOV}/${BINDIR}/lib${PROJECT}.a: ${OBJECTS_GCOV}
	$(call colorecho,AR,$@)
	@mkdir -p ${BUILDDIR_GCOV}/${BINDIR}
	$(QUIET)${AR} rcs $@ ${OBJECTS_GCOV}

${BUILDDIR_GCOV}/${BINDIR}/${SOVERSIONFILE}: ${OBJECTS_GCOV}
	$(call colorecho,LD,$@)
	@mkdir -p ${BUILDDIR_GCOV}/${BINDIR}
	$(QUIET)${CC} -Wl,${SONAME_FLAG},${BUILDSOMAJORFILE} ${SHARED_FLAG} \
		${PLATFORMFLAGS} ${GCOV_LDFLAGS} -o $@ ${OBJECTS_GCOV} ${LIBS}

${PROJECT}-gcov: ${BUILDDIR_GCOV}/${BINDIR}/lib${PROJECT}.a
gcov: ${PROJECT}-gcov

run-gcov: options gcov
	$(QUIET)${MAKE} -C tests run-gcov
	$(call colorecho,LCOV,"Analyse data")
	$(QUIET)${LCOV_EXEC} ${LCOV_FLAGS}
	$(call colorecho,LCOV,"Generate report")
	$(QUIET)${GENHTML_EXEC} ${GENHTML_FLAGS}

# clean

clean:
	$(QUIET)rm -rf \
		${BUILDDIR} \
		${DEPENDDIR} \
		${TARFILE} \
		${GCDA} \
		${GCNO} \
		$(PROJECT).info \
		gcov \
		.version-checks \
		${PROJECTNV}/version.h \
		${PROJECTNV}/version.h.tmp \
		${PROJECTNV}/css-definitions.c \
		${PROJECTNV}/css-definitions.c.tmp
	$(QUIET)$(MAKE) -C tests clean
	$(QUIET)$(MAKE) -C po clean
	$(QUIET)$(MAKE) -C doc clean

# translations

po:
	$(QUIET)${MAKE} -C po

update-po:
	$(QUIET)${MAKE} -C po update-po

# documentation

doc:
	$(QUIET)$(MAKE) -C doc

# tests

test: ${PROJECT}
	$(QUIET)$(MAKE) -C tests run

test-debug: ${PROJECT}-debug
	$(QUIET)$(MAKE) -C tests run-debug

# create taball

dist:
	$(QUIET)tar -czf $(TARFILE) --exclude=.gitignore \
		--transform 's,^,$(PROJECTNV)-$(VERSION)/,' \
		`git ls-files`

# install

install-static: static
	$(call colorecho,INSTALL,"Install static library")
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${LIBDIR}
	$(QUIET)install -m 644 ${BUILDDIR_RELEASE}/${BINDIR}/lib${PROJECT}.a ${DESTDIR}${LIBDIR}

install-shared: shared
	$(call colorecho,INSTALL,"Install shared library")
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${LIBDIR}
	$(QUIET)install -m 644 ${BUILDDIR_RELEASE}/${BINDIR}/${SOVERSIONFILE} ${DESTDIR}${LIBDIR}
	$(QUIET)ln -sf ${SOVERSIONFILE} ${DESTDIR}${LIBDIR}/${SOMAJORFILE} || \
		echo "Failed to create ${SOMAJORFILE}. Please check if it exists and points to the correct version of ${SOFILE}."
	$(QUIET)ln -sf ${SOVERSIONFILE} ${DESTDIR}${LIBDIR}/${SOFILE} || \
		echo "Failed to create ${SOFILE}. Please check if it exists and points to the correct version of ${SOFILE}."

install-headers: ${PROJECTNV}/version.h ${BUILDDIR}/${PROJECT}.pc
	$(call colorecho,INSTALL,"Install pkg-config file")
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${LIBDIR}/pkgconfig
	$(QUIET)install -m 644 ${BUILDDIR}/${PROJECT}.pc ${DESTDIR}${LIBDIR}/pkgconfig
	$(call colorecho,INSTALL,"Install header files")
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${INCLUDEDIR}/girara
	$(QUIET)install -m 644 ${HEADERS} ${DESTDIR}${INCLUDEDIR}/girara

install-po:
	$(QUIET)${MAKE} -C po install

install: install-po install-static install-shared install-headers

# uninstall

uninstall: uninstall-headers
	$(call colorecho,UNINSTALL,"Remove library files")
	$(QUIET)rm -f ${LIBDIR}/lib${PROJECT}.a ${LIBDIR}/${SOVERSIONFILE} \
		${LIBDIR}/${SOMAJORFILE} ${LIBDIR}/${SOFILE}
	$(QUIET)${MAKE} -C po uninstall

uninstall-headers:
	$(call colorecho,UNINSTALL,"Remove header files")
	$(QUIET)rm -rf ${DESTDIR}${INCLUDEDIR}/girara
	$(call colorecho,UNINSTALL,"Remove pkg-config file")
	$(QUIET)rm -f ${DESTDIR}${LIBDIR}/pkgconfig/${PROJECT}.pc

# format and tidy

format:
	clang-tidy -fix -checks=readability-braces-around-statements \
		$(SOURCE) -- $(CPPFLAGS) $(CFLAGS)
	clang-format-3.8 -i $(SOURCE) $(HEADERS)

tidy:
	clang-tidy $(SOURCE) -- $(CPPFLAGS) $(CFLAGS)

DEPENDS = ${DEPENDDIRS:^=${DEPENDDIR}/}$(addprefix ${DEPENDDIR}/,${OBJECTS:.o=.o.dep})
-include ${DEPENDS}

.PHONY: all options clean debug doc test dist install install-headers uninstall \
	uninstall-headers ${PROJECT} ${PROJECT}-debug po update-po \
	static shared install-static install-shared
