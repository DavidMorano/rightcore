# MAKEFILE (users)

T= users

ALL= $(T)

SRCROOT= $(LOCAL)


FBINDIR= $(SRCROOT)/fbin


all:		$(ALL)

$(T):		$(T).fun
	ln -f $(T).fun $(T)

install:	$(T)
	rm -f $(FBINDIR)/$(T).ksh
	rm -f $(FBINDIR)/$(T).$(OFF)
	makenewer -r $(T) $(FBINDIR)
	
clean:
	rm -f $(T)


