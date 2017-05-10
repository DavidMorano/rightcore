# MAKEFILE (psboxsize)

T= psboxsize

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
SBINDIR= $(SRCROOT)/sbin
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help


$(T):		$(T).ksh
	rm -f $(T)
	ln -f $(T).ksh $(T)

install:	install-raw install-help install-man

install-raw:	$(T)
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r $(T) $(SBINDIR)

install-help:	$(T).help
	-mkdir $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

install-man:	$(T).man
	ln -f $(T).man $(T).1
	makenewer $(T).1 $(MANDIR)
	
clean:
	rm -f $(T) $(T).1


