# See LICENSE file for license and copyright information

include config.mk
include common.mk

PROJECTNV = girara
PROJECT   = girara-gtk${GIRARA_GTK_VERSION}
SOURCE    = $(wildcard *.c)
OBJECTS   = ${SOURCE:.c=-gtk${GIRARA_GTK_VERSION}.o}
DOBJECTS  = ${SOURCE:.c=-gtk${GIRARA_GTK_VERSION}.do}
HEADERS   = $(filter-out version.h,$(filter-out internal.h,$(wildcard *.h)))
HEADERS_INSTALL = ${HEADERS} version.h


ifeq (,$(findstring -DGETTEXT_PACKAGE,${CPPFLAGS}))
CPPFLAGS += -DGETTEXT_PACKAGE=\"${GETTEXT_PACKAGE}\"
endif
ifeq (,$(findstring -DLOCALEDIR,${CPPFLAGS}))
CPPFLAGS += -DLOCALEDIR=\"${LOCALEDIR}\"
endif


all: options ${PROJECT}

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

version.h: version.h.in config.mk
	$(QUIET)sed 's/GVMAJOR/${GIRARA_VERSION_MAJOR}/' < version.h.in | \
		sed 's/GVMINOR/${GIRARA_VERSION_MINOR}/' | \
		sed 's/GVREV/${GIRARA_VERSION_REV}/' > version.h

%-gtk${GIRARA_GTK_VERSION}.o: %.c
	@mkdir -p .depend
	$(ECHO) CC $<
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

%-gtk${GIRARA_GTK_VERSION}.do: %.c
	@mkdir -p .depend
	$(ECHO) CC $<
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} ${DFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

${OBJECTS}:  config.mk version.h
${DOBJECTS}: config.mk version.h

${PROJECT}: lib${PROJECT}.a lib${PROJECT}.so.${SOVERSION}

lib${PROJECT}.a: ${OBJECTS}
	$(ECHO) AR rcs $@
	$(QUIET)ar rcs $@ ${OBJECTS}

lib${PROJECT}.so.${SOVERSION}: ${OBJECTS}
	$(ECHO) LD $@
	$(QUIET)${CC} -Wl,-soname,lib${PROJECT}.so.${SOMAJOR} -shared ${LDFLAGS} -o $@ ${OBJECTS} ${LIBS}

clean:
	$(QUIET)rm -rf ${OBJECTS} ${PROJECT}-${VERSION}.tar.gz \
		${DOBJECTS} lib${PROJECT}.a lib${PROJECT}-debug.a ${PROJECT}.pc doc \
		lib$(PROJECT).so.${SOVERSION} lib${PROJECT}-debug.so.${SOVERSION} .depend \
		${PROJECTNV}-${VERSION}.tar.gz version.h *gcda *gcno $(PROJECT).info gcov
	$(QUIET)${MAKE} -C tests clean
	$(QUIET)${MAKE} -C po clean

${PROJECT}-debug: lib${PROJECT}-debug.a lib${PROJECT}-debug.so.${SOVERSION}

lib${PROJECT}-debug.a: ${DOBJECTS}
	$(ECHO) AR rcs $@
	$(QUIET)ar rc $@ ${DOBJECTS}

lib${PROJECT}-debug.so.${SOVERSION}: ${DOBJECTS}
	$(ECHO) LD $@
	$(QUIET)${CC} -Wl,-soname,lib${PROJECT}.so.${SOMAJOR} -shared ${LDFLAGS} -o $@ ${DOBJECTS} ${LIBS}

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
	$(QUIET)mkdir -p ${PROJECTNV}-${VERSION}
	$(QUIET)mkdir -p ${PROJECTNV}-${VERSION}/tests
	$(QUIET)mkdir -p ${PROJECTNV}-${VERSION}/po
	$(QUIET)cp LICENSE Makefile config.mk common.mk ${PROJECTNV}.pc.in \
		${HEADERS} internal.h version.h.in README AUTHORS Doxyfile \
		${SOURCE} ${PROJECTNV}-${VERSION}
	$(QUIET)cp tests/*.c tests/Makefile tests/config.mk ${PROJECTNV}-${VERSION}/tests
	$(QUIET)cp po/Makefile po/*.po ${PROJECTNV}-${VERSION}/po
	$(QUIET)tar -cf ${PROJECTNV}-${VERSION}.tar ${PROJECTNV}-${VERSION}
	$(QUIET)gzip ${PROJECTNV}-${VERSION}.tar
	$(QUIET)rm -rf ${PROJECTNV}-${VERSION}

${PROJECT}.pc: ${PROJECTNV}.pc.in config.mk
	$(QUIET)echo project=${PROJECT} > ${PROJECT}.pc
	$(QUIET)echo version=${VERSION} >> ${PROJECT}.pc
	$(QUIET)echo includedir=${INCLUDEDIR} >> ${PROJECT}.pc
	$(QUIET)echo libdir=${LIBDIR} >> ${PROJECT}.pc
	$(QUIET)echo GTK_VERSION=${GIRARA_GTK_VERSION} >> ${PROJECT}.pc
	$(QUIET)cat ${PROJECTNV}.pc.in >> ${PROJECT}.pc

po:
	$(QUIET)${MAKE} -C po

update-po:
	$(QUIET)${MAKE} -C po update-po

install: all install-headers po
	$(ECHO) installing library file
	$(QUIET)mkdir -p ${DESTDIR}${LIBDIR}
	$(QUIET)install -m 644 lib${PROJECT}.a ${DESTDIR}${LIBDIR}
	$(QUIET)install -m 644 lib${PROJECT}.so.${SOVERSION} ${DESTDIR}${LIBDIR}
	$(QUIET)ln -s lib${PROJECT}.so.${SOVERSION} ${DESTDIR}${LIBDIR}/lib${PROJECT}.so.${SOMAJOR} || \
		echo "Failed to create lib${PROJECT}.so.${SOMAJOR}. Please check if it exists and points to the correct version of lib${PROJECT}.so."
	$(QUIET)ln -s lib${PROJECT}.so.${SOVERSION} ${DESTDIR}${LIBDIR}/lib${PROJECT}.so || \
		echo "Failed to create lib${PROJECT}.so. Please check if it exists and points to the correct version of lib${PROJECT}.so."
	$(QUIET)${MAKE} -C po install

install-headers: version.h ${PROJECT}.pc
	$(ECHO) installing pkgconfig file
	$(QUIET)mkdir -p ${DESTDIR}${LIBDIR}/pkgconfig
	$(QUIET)install -m 644 ${PROJECT}.pc ${DESTDIR}${LIBDIR}/pkgconfig
	$(ECHO) installing header files
	$(QUIET)mkdir -p ${DESTDIR}${INCLUDEDIR}/girara
	$(QUIET)install -m 644 ${HEADERS_INSTALL} ${DESTDIR}${INCLUDEDIR}/girara

uninstall: uninstall-headers
	$(ECHO) removing library file
	$(QUIET)rm -f ${LIBDIR}/lib${PROJECT}.a ${LIBDIR}/lib${PROJECT}.so.${SOVERSION} \
		${LIBDIR}/lib${PROJECT}.so.${SOMAJOR} ${LIBDIR}/lib${PROJECT}.so
	$(QUIET)${MAKE} -C po uninstall

uninstall-headers:
	$(ECHO) removing header files
	$(QUIET)rm -rf ${INCLUDEDIR}/girara
	$(ECHO) removing pkgconfig file
	$(QUIET)rm -f ${LIBDIR}/pkgconfig/${PROJECT}.pc

.PHONY: all options clean debug doc test dist install install-headers uninstall \
	uninstall-headers ${PROJECT} ${PROJECT}-debug po update-po

TDEPENDS = ${OBJECTS:.o=.o.dep}
DEPENDS = ${TDEPENDS:^=.depend/}
-include ${DEPENDS}
