# See LICENSE file for license and copyright information

include config.mk
include common.mk
include colors.mk

PROJECTNV = girara
PROJECT   = girara-gtk3
SOURCE    = $(wildcard *.c)
CSOURCE   = $(filter-out css-definitions.c, $(SOURCE))
OBJECTS   = ${CSOURCE:.c=.o} css-definitions.o
DOBJECTS  = ${OBJECTS:.o=.do}
GCDA      = ${SOURCE:.c=.gcda}
GCNO      = ${SOURCE:.c=.gcno}
HEADERS   = $(filter-out version.h,$(filter-out internal.h,$(wildcard *.h)))
HEADERS_INSTALL = ${HEADERS} version.h

ifneq (${WITH_LIBNOTIFY},0)
INCS += $(LIBNOTIFY_INC)
LIBS += $(LIBNOTIFY_LIB)
CPPFLAGS += -DWITH_LIBNOTIFY
LIBNOTIFY_PC_NAME = libnotify
else
LIBNOTIFY_PC_NAME =
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
endif

all: ${PROJECT} po ${PROJECT}.pc

# pkg-config based version checks
.version-checks/%: config.mk
	$(QUIET)test $($(*)_VERSION_CHECK) -eq 0 || \
		pkg-config --atleast-version $($(*)_MIN_VERSION) $($(*)_PKG_CONFIG_NAME) || ( \
		echo "The minium required version of $(*) is $($(*)_MIN_VERSION)" && \
		false \
	)
	@mkdir -p .version-checks
	$(QUIET)touch $@

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

version.h: version.h.in config.mk
	$(QUIET)sed -e 's,@GVMAJOR@,${GIRARA_VERSION_MAJOR},' \
		-e 's,@GVMINOR@,${GIRARA_VERSION_MINOR},' \
		-e 's,@GVREV@,${GIRARA_VERSION_REV},' \
		version.h.in > version.h.tmp
	$(QUIET)mv version.h.tmp version.h

css-definitions.c: data/girara.css_t
	$(QUIET)echo '#include "css-definitions.h"' > $@.tmp
	$(QUIET)echo 'const char* CSS_TEMPLATE =' >> $@.tmp
	$(QUIET)sed 's/^\(.*\)$$/"\1\\n"/' $< >> $@.tmp
	$(QUIET)echo ';' >> $@.tmp
	$(QUIET)mv $@.tmp $@

%.o: %.c
	@mkdir -p .depend
	$(call colorecho,CC,$<)
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

%.do: %.c
	@mkdir -p .depend
	$(call colorecho,CC,$<)
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} ${DFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

${OBJECTS} ${DOBJECTS}: config.mk version.h \
	.version-checks/GTK .version-checks/GLIB

${PROJECT}: static shared
static: lib${PROJECT}.a
shared: lib${PROJECT}.so.${SOVERSION}

lib${PROJECT}.a: ${OBJECTS}
	$(call colorecho,AR,$@)
	$(QUIET)ar rcs $@ ${OBJECTS}

lib${PROJECT}.so.${SOVERSION}: ${OBJECTS}
	$(call colorecho,LD,$@)
	$(QUIET)${CC} -Wl,${SONAME_FLAG},lib${PROJECT}.so.${SOMAJOR} ${SHARED_FLAG} ${LDFLAGS} -o $@ ${OBJECTS} ${LIBS}

clean:
	$(QUIET)rm -rf \
		${OBJECTS} \
		${DOBJECTS} \
		${TARFILE} \
		${TARDIR} \
		lib${PROJECT}.a \
		lib${PROJECT}-debug.a \
		${PROJECT}.pc \
		doc \
		lib$(PROJECT).so.${SOVERSION} \
		lib${PROJECT}-debug.so.${SOVERSION} \
		.depend \
		version.h \
		${GCDA} \
		${GCNO} \
		$(PROJECT).info \
		gcov \
		.version-checks \
		version.h.tmp \
		${PROJECT}.pc.tmp \
		css-definitions.c \
		css-definitions.c.tmp
	$(QUIET)${MAKE} -C tests clean
	$(QUIET)${MAKE} -C po clean

${PROJECT}-debug: lib${PROJECT}-debug.a lib${PROJECT}-debug.so.${SOVERSION}

lib${PROJECT}-debug.a: ${DOBJECTS}
	$(call colorecho,AR,$@)
	$(QUIET)ar rc $@ ${DOBJECTS}

lib${PROJECT}-debug.so.${SOVERSION}: ${DOBJECTS}
	$(call colorecho,LD,$@)
	$(QUIET)${CC} -Wl,${SONAME_FLAG},lib${PROJECT}.so.${SOMAJOR} ${SHARED_FLAG} ${LDFLAGS} -o $@ ${DOBJECTS} ${LIBS}

debug: options ${PROJECT}-debug

doc:
	$(QUIET)doxygen Doxyfile

gcov: clean
	$(QUIET)CFLAGS="${CFLAGS} -fprofile-arcs -ftest-coverage" LDFLAGS="${LDFLAGS} -fprofile-arcs" ${MAKE} $(PROJECT)
	$(QUIET)CFLAGS="${CFLAGS} -fprofile-arcs -ftest-coverage" LDFLAGS="${LDFLAGS} -fprofile-arcs" ${MAKE} test
	$(QUIET)lcov --directory . --capture --output-file $(PROJECT).info
	$(QUIET)genhtml --output-directory gcov $(PROJECT).info

test: ${PROJECT}
	$(QUIET)${MAKE} -C tests run

test-debug: debug
	$(QUIET)${MAKE} -C tests run-debug

dist: clean
	$(QUIET)tar -czf $(TARFILE) --exclude=.gitignore \
		--transform 's,^,$(PROJECTNV)-$(VERSION)/,' \
		`git ls-files`

${PROJECT}.pc: ${PROJECTNV}.pc.in config.mk
	$(QUIET)sed -e 's,@PROJECT@,${PROJECT},' \
		-e 's,@VERSION@,${VERSION},' \
		-e 's,@INCLUDEDIR@,${INCLUDEDIR},' \
		-e 's,@LIBDIR@,${LIBDIR},' \
		-e 's,@LIBNOTIFY_PC_NAME@,${LIBNOTIFY_PC_NAME},' \
		${PROJECTNV}.pc.in > ${PROJECT}.pc.tmp
	$(QUIET)mv ${PROJECT}.pc.tmp ${PROJECT}.pc

po:
	$(QUIET)${MAKE} -C po

update-po:
	$(QUIET)${MAKE} -C po update-po

install-static: static
	$(call colorecho,INSTALL,"Install static library")
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${LIBDIR}
	$(QUIET)install -m 644 lib${PROJECT}.a ${DESTDIR}${LIBDIR}

install-shared: shared
	$(call colorecho,INSTALL,"Install shared library")
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${LIBDIR}
	$(QUIET)install -m 644 lib${PROJECT}.so.${SOVERSION} ${DESTDIR}${LIBDIR}
	$(QUIET)ln -sf lib${PROJECT}.so.${SOVERSION} ${DESTDIR}${LIBDIR}/lib${PROJECT}.so.${SOMAJOR} || \
		echo "Failed to create lib${PROJECT}.so.${SOMAJOR}. Please check if it exists and points to the correct version of lib${PROJECT}.so."
	$(QUIET)ln -sf lib${PROJECT}.so.${SOVERSION} ${DESTDIR}${LIBDIR}/lib${PROJECT}.so || \
		echo "Failed to create lib${PROJECT}.so. Please check if it exists and points to the correct version of lib${PROJECT}.so."

install: options po install-static install-shared install-headers
		$(QUIET)${MAKE} -C po install

install-headers: version.h ${PROJECT}.pc
	$(call colorecho,INSTALL,"Install pkg-config file")
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${LIBDIR}/pkgconfig
	$(QUIET)install -m 644 ${PROJECT}.pc ${DESTDIR}${LIBDIR}/pkgconfig
	$(call colorecho,INSTALL,"Install header files")
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${INCLUDEDIR}/girara
	$(QUIET)install -m 644 ${HEADERS_INSTALL} ${DESTDIR}${INCLUDEDIR}/girara

uninstall: uninstall-headers
	$(call colorecho,UNINSTALL,"Remove library files")
	$(QUIET)rm -f ${LIBDIR}/lib${PROJECT}.a ${LIBDIR}/lib${PROJECT}.so.${SOVERSION} \
		${LIBDIR}/lib${PROJECT}.so.${SOMAJOR} ${LIBDIR}/lib${PROJECT}.so
	$(QUIET)${MAKE} -C po uninstall

uninstall-headers:
	$(call colorecho,UNINSTALL,"Remove header files")
	$(QUIET)rm -rf ${INCLUDEDIR}/girara
	$(call colorecho,UNINSTALL,"Remove pkg-config file")
	$(QUIET)rm -f ${LIBDIR}/pkgconfig/${PROJECT}.pc

.PHONY: all options clean debug doc test dist install install-headers uninstall \
	uninstall-headers ${PROJECT} ${PROJECT}-debug po update-po \
	static shared install-static install-shared

TDEPENDS = ${OBJECTS:.o=.o.dep}
DEPENDS = ${TDEPENDS:^=.depend/}
-include ${DEPENDS}
