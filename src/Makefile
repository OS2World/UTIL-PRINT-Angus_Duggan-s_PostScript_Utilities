# Makefile for PS utilities
# updated AJCD 23/9/93
#
# This Makefile is rather revolting; unfortunately, there are so many
# incompatible makes around that putting all of the non-standard targets
# explicitly seems to be the only way of ensuring portability.

BINMODE=0755
MANMODE=0644
CHMOD=chmod
INSTALL=install -c -m $(BINMODE)
INSTALLMAN=install -c -m $(MANMODE)

BINDIR=/usr/local/bin
SCRIPTDIR=$(BINDIR)
MANDIR=/usr/local/share/man/man1
MANEXT=1
PAPER=a4
INCLUDEDIR=/usr/local/share/psutils
PERL=/usr/local/bin/perl

# CFLAGS variable controls compilation. Add following flags if necessary:
#	-DVMS	if compiling under VMS
#	-DMSDOS	if compiling under MSDOS
CFLAGS=-DPAPER=\"$(PAPER)\"

# epsffit fits an epsf file to a given bounding box
# psbook rearranges pages into signatures
# psselect selects page ranges
# pstops performs general page rearrangement and merging
# psnup puts multiple logical pages on one physical page
# psresize scales and moves pages to fit on different paper sizes

# Do NOT leave "obj" undefined in the definitions below, or "make clean" will
# remove ALL of the files.

# definitions for Unix:
exe =
obj = .o
BIN = psbook psselect pstops epsffit psnup psresize
SHELLSCRIPTS = getafm
PERLSCRIPTS = fixfmps fixmacps fixpsditps fixpspps fixtpps fixwfwps fixwpps \
              fixwwps fixdlsrps extractres includeres
INCLUDES = md68_0.ps md71_0.ps
MANPAGES = psbook.$(MANEXT) psselect.$(MANEXT) pstops.$(MANEXT) \
	   epsffit.$(MANEXT) psnup.$(MANEXT) psresize.$(MANEXT) \
	   getafm.$(MANEXT) fixdlsrps.$(MANEXT) fixfmps.$(MANEXT) \
	   fixmacps.$(MANEXT) fixpsditps.$(MANEXT) fixpspps.$(MANEXT) \
	   fixtpps.$(MANEXT) fixwfwps.$(MANEXT) fixwpps.$(MANEXT) \
	   fixwwps.$(MANEXT) extractres.$(MANEXT) includeres.$(MANEXT)
RM = rm -f

# Definitions for MSDOS
#exe = .exe
#obj = .obj
#BIN = psbook$(exe) psselect$(exe) pstops$(exe) epsffit$(exe) psnup$(exe) \
#      psresize$(exe)
#PERLSCRIPTS =
#INCLUDES =
#MANPAGES =
#RM = del

all: $(BIN) $(PERLSCRIPTS) $(MANPAGES)

psutil$(obj): psutil.h patchlev.h psutil.c

psspec$(obj): psutil.h patchlev.h psspec.h psspec.c

epsffit$(exe): epsffit.c
	$(CC) $(CFLAGS) -o epsffit$(exe) epsffit.c

epsffit.$(MANEXT): epsffit.man
	./maketext MAN="$(MANPAGES)" $? > $@

psnup$(exe): psnup$(obj) psutil$(obj) psspec$(obj)
	$(CC) $(CFLAGS) -o psnup$(exe) psutil$(obj) psspec$(obj) psnup$(obj)

psnup$(obj): psutil.h patchlev.h psspec.h psnup.c

psnup.$(MANEXT): psnup.man
	./maketext MAN="$(MANPAGES)" PAPER=$(PAPER) $? > $@

psresize$(exe): psresize$(obj) psutil$(obj) psspec$(obj)
	$(CC) $(CFLAGS) -o psresize$(exe) psutil$(obj) psspec$(obj) psresize$(obj)

psresize$(obj): psutil.h patchlev.h psspec.h psresize.c

psresize.$(MANEXT): psresize.man
	./maketext MAN="$(MANPAGES)" PAPER=$(PAPER) $? > $@

psbook$(exe): psbook$(obj) psutil$(obj)
	$(CC) $(CFLAGS) -o psbook$(exe) psutil$(obj) psbook$(obj)

psbook$(obj): psutil.h patchlev.h psbook.c

psbook.$(MANEXT): psbook.man
	./maketext "MAN=$(MANPAGES)" $? > $@

psselect$(exe): psselect$(obj) psutil$(obj)
	$(CC) $(CFLAGS) -o psselect$(exe) psutil$(obj) psselect$(obj)

psselect$(obj): psutil.h patchlev.h psselect.c

psselect.$(MANEXT): psselect.man
	./maketext "MAN=$(MANPAGES)" $? > $@

pstops$(exe): pstops$(obj) psutil$(obj) psspec$(obj)
	$(CC) $(CFLAGS) -o pstops$(exe) psutil$(obj) psspec$(obj) pstops$(obj)

pstops$(obj): psutil.h patchlev.h psspec.h pstops.c

pstops.$(MANEXT): pstops.man
	./maketext "MAN=$(MANPAGES)" PAPER=$(PAPER) $? > $@

fixfmps: fixfmps.pl
	./maketext PERL=$(PERL) $? >$@;
	$(CHMOD) $(BINMODE) $@

fixfmps.$(MANEXT): fixfmps.man
	./maketext "MAN=$(MANPAGES)" $? > $@

fixmacps: fixmacps.pl
	./maketext PERL=$(PERL) INCLUDE=$(INCLUDEDIR) $? >$@;
	$(CHMOD) $(BINMODE) $@

fixmacps.$(MANEXT): fixmacps.man
	./maketext "MAN=$(MANPAGES)" INCLUDE=$(INCLUDEDIR) $? > $@

fixpsditps: fixpsditps.pl
	./maketext PERL=$(PERL) $? >$@;
	$(CHMOD) $(BINMODE) $@

fixpsditps.$(MANEXT): fixpsditps.man
	./maketext "MAN=$(MANPAGES)" $? > $@

fixpspps: fixpspps.pl
	./maketext PERL=$(PERL) $? >$@;
	$(CHMOD) $(BINMODE) $@

fixpspps.$(MANEXT): fixpspps.man
	./maketext "MAN=$(MANPAGES)" $? > $@

fixtpps: fixtpps.pl
	./maketext PERL=$(PERL) $? >$@;
	$(CHMOD) $(BINMODE) $@

fixtpps.$(MANEXT): fixtpps.man
	./maketext "MAN=$(MANPAGES)" $? > $@

fixwfwps: fixwfwps.pl
	./maketext PERL=$(PERL) $? >$@;
	$(CHMOD) $(BINMODE) $@

fixwfwps.$(MANEXT): fixwfwps.man
	./maketext "MAN=$(MANPAGES)" $? > $@

fixwpps: fixwpps.pl
	./maketext PERL=$(PERL) $? >$@;
	$(CHMOD) $(BINMODE) $@

fixwpps.$(MANEXT): fixwpps.man
	./maketext "MAN=$(MANPAGES)" $? > $@

fixwwps: fixwwps.pl
	./maketext PERL=$(PERL) $? >$@;
	$(CHMOD) $(BINMODE) $@

fixwwps.$(MANEXT): fixwwps.man
	./maketext "MAN=$(MANPAGES)" $? > $@

fixdlsrps: fixdlsrps.pl
	./maketext PERL=$(PERL) $? >$@;
	$(CHMOD) $(BINMODE) $@

fixdlsrps.$(MANEXT): fixdlsrps.man
	./maketext "MAN=$(MANPAGES)" $? > $@

extractres: extractres.pl
	./maketext PERL=$(PERL) $? >$@;
	$(CHMOD) $(BINMODE) $@

extractres.$(MANEXT): extractres.man
	./maketext "MAN=$(MANPAGES)" $? > $@

includeres: includeres.pl
	./maketext PERL=$(PERL) INCLUDE=$(INCLUDEDIR) $? >$@;
	$(CHMOD) $(BINMODE) $@

includeres.$(MANEXT): includeres.man
	./maketext "MAN=$(MANPAGES)" INCLUDE=$(INCLUDEDIR) $? > $@

getafm.$(MANEXT): getafm.man
	./maketext "MAN=$(MANPAGES)" $? > $@

clean:
	$(RM) *$(obj)

veryclean realclean: clean
	$(RM) $(BIN) $(PERLSCRIPTS) $(MANPAGES)

install: install.bin install.script install.man install.include

install.bin: $(BIN)
	@for i in $(BIN); do \
		echo Installing $$i; \
		$(INSTALL) $$i $(BINDIR); \
	done

install.script: $(PERLSCRIPTS) $(SHELLSCRIPTS)
	@for i in $(PERLSCRIPTS) $(SHELLSCRIPTS); do \
		echo Installing $$i; \
		$(INSTALL) $$i $(SCRIPTDIR); \
	done

install.include: $(INCLUDES)
	@for i in $(INCLUDES); do \
		echo Installing $$i; \
		$(INSTALLMAN) $$i $(INCLUDEDIR); \
	done

install.man: $(MANPAGES)
	@for i in $(MANPAGES); do \
		echo Installing manual page for $$i; \
		$(INSTALLMAN) $$i $(MANDIR)/$$i; \
	done
