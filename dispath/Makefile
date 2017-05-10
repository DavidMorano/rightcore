# MAKEFILE

T1= dispath
T2= dislibpath
T3= dismanpath

ALL= $(T1) $(T2) $(T3)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T1):		$(T1).ksh
	cp -p $(T1).ksh $(T1)

$(T2):		$(T2).ksh
	cp -p $(T2).ksh $(T2)

$(T3):		$(T3).ksh
	cp -p $(T3).ksh $(T3)

install:	$(ALL)
	bsdinstall $(ALL) $(BINDIR)

clean:
	rm -f $(ALL)


