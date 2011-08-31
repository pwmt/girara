# See LICENSE file for license and copyright information

include config.mk
include common.mk

PROJECTNV = girara
PROJECT   = girara-gtk${GIRARA_GTK_VERSION}
SOURCE    = girara.c completion.c config.c settings.c utils.c datastructures.c
OBJECTS   = ${SOURCE:.c=-gtk${GIRARA_GTK_VERSION}.o}
DOBJECTS  = ${SOURCE:.c=-gtk${GIRARA_GTK_VERSION}.do}

all: options ${PROJECT}
	@${MAKE} -C examples

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

%-gtk${GIRARA_GTK_VERSION}.o: %.c
	@mkdir -p .depend
	$(ECHO) CC $<
	$(QUIET)${CC} -c ${CFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

%-gtk${GIRARA_GTK_VERSION}.do: %.c
	@mkdir -p .depend
	$(ECHO) CC $<
	$(QUIET)${CC} -c ${CFLAGS} ${DFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

${OBJECTS}:  config.mk
${DOBJECTS}: config.mk

${PROJECT}: lib${PROJECT}.a lib${PROJECT}.so.${SOVERSION}

lib${PROJECT}.a: ${OBJECTS}
	$(ECHO) AR rcs $@
	$(QUIET)ar rcs $@ ${OBJECTS}

lib${PROJECT}.so.${SOVERSION}: ${OBJECTS}
	$(ECHO) LD $@
	$(QUIET)${CC} -Wl,-soname,lib${PROJECT}.so.${SOMAJOR} -shared ${LDFLAGS} -o $@ ${OBJECTS} ${LIBS}

clean:
	$(QUIET)rm -rf ${OBJECTS} ${PROJECT}-${VERSION}.tar.gz \
		${DOBJECTS} lib${PROJECT}.a lib${PROJECT}-debug.a ${PROJECT}.pc \
		lib$(PROJECT).so.${SOVERSION} lib${PROJECT}-debug.so.${SOVERSION} .depend \
		${PROJECTNV}-${VERSION}.tar.gz
	$(QUIET)${MAKE} -C examples clean
	$(QUIET)${MAKE} -C tests clean

${PROJECT}-debug: lib${PROJECT}-debug.a lib${PROJECT}-debug.so.${SOVERSION}

lib${PROJECT}-debug.a: ${DOBJECTS}
	$(ECHO) AR rcs $@
	$(QUIET)ar rc $@ ${DOBJECTS}

lib${PROJECT}-debug.so.${SOVERSION}: ${DOBJECTS}
	$(ECHO) LD $@
	$(QUIET)${CC} -Wl,-soname,lib${PROJECT}.so.${SOMAJOR} -shared ${LDFLAGS} -o $@ ${DOBJECTS} ${LIBS}

debug: options ${PROJECT}-debug
	$(QUIET)${MAKE} -C examples debug

test: debug
	$(QUIET)${MAKE} -C tests

dist: clean
	$(QUIET)mkdir -p ${PROJECTNV}-${VERSION}
	$(QUIET)cp -R LICENSE Makefile config.mk README ${PROJECTNV}.pc.in \
		girara.h girara-utils.h girara-types.h girara-datastructures.h \
		${SOURCE} examples/ ${PROJECTNV}-${VERSION}
	$(QUIET)tar -cf ${PROJECTNV}-${VERSION}.tar ${PROJECTNV}-${VERSION}
	$(QUIET)gzip ${PROJECTNV}-${VERSION}.tar
	$(QUIET)rm -rf ${PROJECTNV}-${VERSION}

${PROJECT}.pc: ${PROJECTNV}.pc.in config.mk
	$(QUIET)echo project=${PROJECT} > ${PROJECT}.pc
	$(QUIET)echo version=${VERSION} >> ${PROJECT}.pc
	$(QUIET)echo includedir=${PREFIX}/include >> ${PROJECT}.pc
	$(QUIET)echo libdir=${PREFIX}/lib >> ${PROJECT}.pc
	$(QUIET)cat ${PROJECTNV}.pc.in >> ${PROJECT}.pc

install: all ${PROJECT}.pc
	$(ECHO) installing library file
	$(QUIET)mkdir -p ${DESTDIR}${PREFIX}/lib
	$(QUIET)install -m 644 lib${PROJECT}.a ${DESTDIR}${PREFIX}/lib
	$(QUIET)install -m 644 lib${PROJECT}.so.${SOVERSION} ${DESTDIR}${PREFIX}/lib
	$(QUIET)ln -s lib${PROJECT}.so.${SOVERSION} ${DESTDIR}${PREFIX}/lib/lib${PROJECT}.so.${SOMAJOR} || \
		echo "Failed to create lib${PROJECT}.so.${SOMAJOR}. Please check if it exists and points to the correct version of lib${PROJECT}.so."
	$(QUIET)ln -s lib${PROJECT}.so.${SOVERSION} ${DESTDIR}${PREFIX}/lib/lib${PROJECT}.so || \
		echo "Failed to create lib${PROJECT}.so. Please check if it exists and points to the correct version of lib${PROJECT}.so."
	$(ECHO) installing header file
	$(QUIET)mkdir -p ${DESTDIR}${PREFIX}/include
	$(QUIET)install -m 644 girara.h girara-utils.h girara-datastructures.h \
		girara-types.h ${DESTDIR}${PREFIX}/include
	$(ECHO) installing pkgconfig file
	$(QUIET)mkdir -p ${DESTDIR}${PREFIX}/lib/pkgconfig
	$(QUIET)install -m 644 ${PROJECT}.pc ${DESTDIR}${PREFIX}/lib/pkgconfig

uninstall:
	$(ECHO) removing library file
	$(QUIET)rm -f ${PREFIX}/lib/lib${PROJECT}.a ${PREFIX}/lib/lib${PROJECT}.so.${SOVERSION} \
		${PREFIX}/lib/lib${PROJECT}.so.${SOMAJOR} ${PREFIX}/lib/lib${PROJECT}.so
	$(ECHO) removing include file
	$(QUIET)rm -f ${PREFIX}/include/girara.h ${PREFIX}/include/girara-utils.h \
		${PREFIX}/include/girara-datastructures.h ${PREFIX}/include/girara-types.h
	$(ECHO) removing pkgconfig file
	$(QUIET)rm -f ${PREFIX}/lib/pkgconfig/${PROJECT}.pc

.PHONY: all options clean debug test dist install uninstall ${PROJECT} ${PROJECT}-debug

-include $(wildcard .depend/*.dep)
