# See LICENSE file for license and copyright information

include ../config.mk
include ../common.mk

PROJECTNV = girara
PROJECT   = girara-gtk${GIRARA_GTK_VERSION}
CATALOGS  = $(wildcard *.po)
LINGUAS  ?= $(patsubst %.po, %, $(CATALOGS))
MOS = $(patsubst %, %/LC_MESSAGES/${GETTEXT_PACKAGE}.mo, $(LINGUAS))

all: ${MOS}

clean:
	$(QUIET)rm -rf POTFILES.in POTFILES.in.tmp $(patsubst %.po, %, $(CATALOGS)) ${PROJECT}.pot

POTFILES.in: $(wildcard ../*.c)
	$(QUIET) set -e && rm -f $@.tmp && touch $@.tmp && \
	for f in $(^F) ; do \
		echo $$f >> $@.tmp ; \
	done && \
	mv $@.tmp $@

${GETTEXT_PACKAGE}.pot: POTFILES.in
	$(ECHO) regenerate $@
	$(QUIET)intltool-update --pot --gettext-package=${GETTEXT_PACKAGE}

update-po: ${GETTEXT_PACKAGE}.pot
	$(ECHO) updating po files: ${CATALOGS}
	$(QUIET)set -e && for f in ${CATALOGS} ; do \
		intltool-update --dist --gettext-package=${GETTEXT_PACKAGE} `echo $$f | sed 's/\.po//'` ; \
	done

%/LC_MESSAGES/${GETTEXT_PACKAGE}.mo: %.po
	@mkdir -p $(@D)
	$(ECHO) msgfmt $(shell echo $< | sed 's/\.po//')
	$(QUIET)${MSGFMT} ${MSGFMTFLAGS} -c $< -o $@

install: ${MOS}
	$(ECHO) installing translations
	$(QUIET)set -e && for f in $^ ; do \
		mkdir -p -m 755 $(DESTDIR)$(LOCALEDIR)/`dirname $$f`; \
		install -m 644 $$f $(DESTDIR)$(LOCALEDIR)/`dirname $$f` ; \
	done

uninstall: ${MOS}
	$(ECHO) removing translations
	$(QUIET)set -e && for f in $^ ; do \
		rm -f $(LOCALEDIR)/$$f; \
	done

.PHONY: all clean install uninstall update-po
