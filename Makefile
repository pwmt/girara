# See LICENSE file for license and copyright information

PROJECT  = girara
SOURCE   = girara.c completion.c settings.c utils.c datastructures.c
OBJECTS  = ${SOURCE:.c=.o}
DOBJECTS = ${SOURCE:.c=.do}

include config.mk

all: options ${PROJECT}
	@make -C examples

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

%.o: %.c
	@echo CC $<
	@${CC} -c ${CFLAGS} -o $@ $<

%.do: %.c
	@echo CC $<
	@${CC} -c ${CFLAGS} ${DFLAGS} -o $@ $<

${OBJECTS}:  girara.c config.mk
${DOBJECTS}: girara.c config.mk

${PROJECT}: ${OBJECTS}
	@echo AR rcs $@
	@ar rcs lib${PROJECT}.a $(OBJECTS)
	@echo LD $@
	@${CC} -shared ${LDFLAGS} -o lib${PROJECT}.so $(OBJECTS)

clean:
	@rm -rf ${OBJECTS} ${PROJECT}-${VERSION}.tar.gz \
		${DOBJECTS} lib${PROJECT}.a lib${PROJECT}-debug.a ${PROJECT}.pc \
		lib$(PROJECT).so lib${PROJECT}-debug.so
	@make -C examples clean
	@make -C tests clean

${PROJECT}-debug: ${DOBJECTS}
	@echo AR rcs $@
	@ar rc lib${PROJECT}-debug.a $(DOBJECTS)
	@echo LD $@
	@${CC} -shared ${LDFLAGS} -o lib${PROJECT}-debug.so $(DOBJECTS)

debug: options ${PROJECT}-debug
	@make -C examples debug

test: debug
	@make -C tests

dist: clean
	@mkdir -p ${PROJECT}-${VERSION}
	@cp -R LICENSE Makefile config.mk README ${PROJECT}.pc.in \
			girara.h ${SOURCE} examples/ ${PROJECT}-${VERSION}
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
	@cp -f lib${PROJECT}.so ${DESTDIR}${PREFIX}/lib
	@echo installing header file
	@mkdir -p ${DESTDIR}${PREFIX}/include
	@cp -f girara.h ${DESTDIR}${PREFIX}/include
	@echo installing pkgconfig file
	@mkdir -p ${DESTDIR}${PREFIX}/pkgconfig
	@cp -f ${PROJECT}.pc ${DESTDIR}${PREFIX}/pkgconfig

uninstall:
	@echo removing library file
	@rm -f ${PREFIX}/lib/lib${PROJECT}.a
	@rm -f ${PREFIX}/lib/lib${PROJECT}.so
	@echo removing include file
	@rm -f ${PREFIX}/include/girara.h
	@echo removing pkgconfig file
	@rm -f ${PREFIX}/pkgconfig/${PROJECT}.pc
