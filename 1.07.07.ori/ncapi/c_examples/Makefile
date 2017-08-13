CFLAGS := -O2 -Wall
INSTALLDIR := ${DESTDIR}/usr/local

ifeq "$(wildcard $(INSTALLDIR) )" ""
	INSTALLDIR = ${DESTDIR}/usr
endif

all: ncs-check ncs-threadcheck ncs-fullcheck

.c.o:
	$(CC) -c $(CFLAGS) $<

ncs-check: ncs-check.o
	$(CC) $(LDFLAGS) -o $@ ncs-check.o -lmvnc

ncs-threadcheck: ncs-threadcheck.o
	$(CC) $(LDFLAGS) -o $@ ncs-threadcheck.o -lmvnc -pthread

ncs-fullcheck: ncs-fullcheck.o fp16.o
	$(CC) $(LDFLAGS) -o $@ ncs-fullcheck.o fp16.o -lmvnc -lm

clean:
	rm -f *.o ncs-check ncs-threadcheck ncs-fullcheck

install: ncs-check ncs-threadcheck ncs-fullcheck
	install -m 0755 -d ${INSTALLDIR}/bin
	install -m 0755 ncs-check ${INSTALLDIR}/bin/ncs-check
	install -m 0755 ncs-threadcheck ${INSTALLDIR}/bin/ncs-threadcheck
	install -m 0755 ncs-fullcheck ${INSTALLDIR}/bin/ncs-fullcheck

uninstall:
	rm -f ${INSTALLDIR}/bin/ncs-check
	rm -f ${INSTALLDIR}/bin/ncs-threadcheck
	rm -f ${INSTALLDIR}/bin/ncs-fullcheck
