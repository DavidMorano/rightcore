# MAKEFILE (notify)

T= notify

ALL= $(T)

SRCROOT= /usr/apache


CGIBINDIR= $(SRCROOT)/cgi


all:		$(ALL)

$(T):		$(T).cgi
	rm -f $(T)
	ln -f $(T).cgi $(T)

install:	$(T).cgi
	makenewer -r $(T).cgi $(CGIBINDIR)
	ln -f $(CGIBINDIR)/${T}.cgi $(CGIBINDIR)/${T} 

clean:
	rm -f $(T)


