# MAKEFILE

T= promptsn

ALL= $(T)

SRCROOT= $(LOCAL)


FBINDIR= $(SRCROOT)/fbin


all:		$(ALL)

$(T):		$(T).fun
	rm -f $(T)
	cp -p $(T).fun $(T)

install:	$(T).fun
	makenewer -r -o rmsuf $(T).fun $(FBINDIR)

clean:
	rm -f $(T)


