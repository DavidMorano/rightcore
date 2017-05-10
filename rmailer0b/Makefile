# MAKEFILE (rmailer)

T= rmailer

ALL= $(T) $(T).ksh

#SRCROOT= $(LOCAL)


BINDIR= $(PCS)/bin


default:	$(ALL)

all:		$(ALL)

$(T):		$(T).ee
	rm -f $(T)
	cp -p $(T).ee $(T)

install:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

clean:
	rm -f $(T)



