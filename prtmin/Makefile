# MAKEFILE

T= prtmin

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(LOCAL)/bin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	cp -p $(T).ksh $(T)

install:	$(T).ksh
	makenewer -r -o rmsuf $(T).ksh $(BINDIR)

clean:
	rm -f $(T)


