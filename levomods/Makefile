# MAKEFILE

T= levomods

SRCROOT= $(NCMP)


CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


#CFLAGS= -I.

NM= nm
NMFLAGS= -xs -v


LIBSDIRS= -L$(LIBDIR)/lib/S5

LIBS= -ldam -lb -luc -lu -L$(GNU)/lib -lgcc -lsocket -lnsl

ARS= libdam.a libb.a libuc.a libu.a $(GNU)/lib/libgcc.a



BPINCS= bpload.h


OBJ= ssh.o vpred.o bpalpha.o yags.o eveight.o gspag.o tourna.o gskew.o

OBJS= $(OBJ) $(T).o



default:	$(T).obj


$(T).obj:	$(T).o
	cp -p $(T).o $(T).obj

$(T).o:		$(OBJ) Makefile
	$(LD) -o $@ -r $(OBJ) $(ARS)

$(T).nm:	$(T).obj
	$(NM) $(NMFLAGS) $(T).obj > $(T).nm

clean:
	rm -f $(T) *.o



ssh.o:		ssh.c ssh.h
	$(CC) -c $(CCOPTS) -I$(SRCROOT)/include $<

vpred.o:	vpred.c vpred.h
	$(CC) -c $(CCOPTS) -I$(SRCROOT)/include $<

bpalpha.o:	bpalpha.c bpalpha.h $(BPINCS)
	$(CC) -c $(CCOPTS) -I$(SRCROOT)/include $<

eveight.o:	eveight.c eveight.h $(BPINCS)
	$(CC) -c $(CCOPTS) -I$(SRCROOT)/include $<

yags.o:		yags.c yags.h $(BPINCS)
	$(CC) -c $(CCOPTS) -I$(SRCROOT)/include $<

gspag.o:	gspag.c gspag.h $(BPINCS)
	$(CC) -c $(CCOPTS) -I$(SRCROOT)/include $<

tourna.o:	tourna.c tourna.h $(BPINCS)
	$(CC) -c $(CCOPTS) -I$(SRCROOT)/include $<

gskew.o:	gskew.c gskew.h $(BPINCS)
	$(CC) -c $(CCOPTS) -I$(SRCROOT)/include $<



