# See LICENSE file for license and copyright information

PROJECT  = girara
SOURCE   = girara.c completion.c config.c settings.c utils.c datastructures.c
OBJECTS  = ${SOURCE:.c=.o}
DOBJECTS = ${SOURCE:.c=.do}

include config.mk

all: options ${PROJECT}
	@${MAKE} -C examples

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

%.o: %.c
	@echo CC $<
	@mkdir -p .depend
	@${CC} -c ${CFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

%.do: %.c
	@echo CC $<
	@mkdir -p .depend
	@${CC} -c ${CFLAGS} ${DFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

${OBJECTS}:  girara.c config.mk
${DOBJECTS}: girara.c config.mk

${PROJECT}: lib${PROJECT}.a lib${PROJECT}.so.${SOVERSION}

lib${PROJECT}.a: ${OBJECTS}
	@echo AR rcs $@
	@ar rcs $@ ${OBJECTS}

lib${PROJECT}.so.${SOVERSION}: ${OBJECTS}
	@echo LD $@
	@${CC} -Wl,-soname,lib${PROJECT}.so.${SOMAJOR} -shared ${LDFLAGS} -o $@ ${OBJECTS}

clean:
	@rm -rf ${OBJECTS} ${PROJECT}-${VERSION}.tar.gz \
		${DOBJECTS} lib${PROJECT}.a lib${PROJECT}-debug.a ${PROJECT}.pc \
		lib$(PROJECT).so.${SOVERSION} lib${PROJECT}-debug.so.${SOVERSION} .depend
	@${MAKE} -C examples clean
	@${MAKE} -C tests clean

${PROJECT}-debug: lib${PROJECT}-debug.a lib${PROJECT}-debug.so.${SOVERSION}

lib${PROJECT}-debug.a: ${DOBJECTS}
	@echo AR rcs $@
	@ar rc $@ ${DOBJECTS}

lib${PROJECT}-debug.so.${SOVERSION}: ${DOBJECTS}
	@echo LD $@
	@${CC} -Wl,-soname,lib${PROJECT}.so.${SOMAJOR} -shared ${LDFLAGS} -o $@ ${DOBJECTS}

debug: options ${PROJECT}-debug
	@${MAKE} -C examples debug

test: debug
	@${MAKE} -C tests

dist: clean
	@mkdir -p ${PROJECT}-${VERSION}
	@cp -R LICENSE Makefile config.mk README ${PROJECT}.pc.in \
			girara.h girara-settings.h girara-types.h girara-datastructures.h \
			${SOURCE} examples/ ${PROJECT}-${VERSION}
	@tar -cf ${PROJECT}-${VERSION}.tar ${PROJECT}-${VERSION}
	@gzip ${PROJECT}-${VERSION}.tar
	@rm -rf ${PROJECT}-${VERSION}

${PROJECT}.pc: ${PROJECT}.pc.in config.mk
	@echo project=${PROJECT} > ${PROJECT}.pc
	@echo version=${VERSION} >> ${PROJECT}.pc
	@echo includedir=${PREFIX}/include >> ${PROJECT}.pc
	@echo libdir=${PREFIX}/lib >> ${PROJECT}.pc
	@cat ${PROJECT}.pc.in >> ${PROJECT}.pc

install: all ${PROJECT}.pc
	@echo installing library file
	@mkdir -p ${DESTDIR}${PREFIX}/lib
	@cp -f lib${PROJECT}.a ${DESTDIR}${PREFIX}/lib
	@install -m 644 lib${PROJECT}.so.${SOVERSION} ${DESTDIR}${PREFIX}/lib
	@echo installing header file
	@mkdir -p ${DESTDIR}${PREFIX}/include
	@cp -f girara.h ${DESTDIR}${PREFIX}/include
	@cp -f girara-utils.h ${DESTDIR}${PREFIX}/include
	@cp -f girara-datastructures.h ${DESTDIR}${PREFIX}/include
	@cp -f girara-types.h ${DESTDIR}${PREFIX}/include
	@echo installing pkgconfig file
	@mkdir -p ${DESTDIR}${PREFIX}/lib/pkgconfig
	@cp -f ${PROJECT}.pc ${DESTDIR}${PREFIX}/lib/pkgconfig

uninstall:
	@echo removing library file
	@rm -f ${PREFIX}/lib/lib${PROJECT}.a
	@rm -f ${PREFIX}/lib/lib${PROJECT}.so.${SOVERSION}
	@echo removing include file
	@rm -f ${PREFIX}/include/girara.h
	@rm -f ${PREFIX}/include/girara-utils.h
	@rm -f ${PREFIX}/include/girara-datastructures.h
	@rm -f ${PREFIX}/include/girara-types.h
	@echo removing pkgconfig file
	@rm -f ${PREFIX}/lib/pkgconfig/${PROJECT}.pc

.PHONY: all options clean debug test dist install uninstall ${PROJECT} ${PROJECT}-debug

-include $(wildcard .depend/*.dep)
