# MAKEFILE 

T= deliver

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	makecp $(T).ksh $(T)

install:	install-raw

install-raw:	$(T).ksh
	rm -f $(T)
	makecp $(T).ksh $(T)
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r $(T) $(BINDIR)
	
clean:
	rm -f $(T)



