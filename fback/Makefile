# MAKEFILE

T= back

ALL= fback iback cback

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

f$(T):		f$(T).ksh
	rm -f f$(T)
	ln -f f$(T).ksh f$(T)

i$(T):		i$(T).ksh
	rm -f i$(T)
	ln -f i$(T).ksh i$(T)

c$(T):		c$(T).ksh
	rm -f c$(T)
	ln -f c$(T).ksh c$(T)

install:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

clean:
	rm -f fback iback cback


