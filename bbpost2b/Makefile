# MAKEFILE

ALL= bbpost datepost

#PCS= $(PCS)

PCSBIN= $(PCS)/bin
PCSINC= $(PCS)/include
PCSLIB= $(PCS)/lib


#CC= gcc

DEFINES= -D$(OS)=1 -DSUN=1

INCDIRS= -I$(PCSINC)

CFLAGS= -O $(DEFINES) $(INCDIRS)

LD= $(CC)

LDFLAGS=

LIBDIRS= -L$(PCSLIB)

LIBS=


all:			$(ALL)

bbpost:			bbpost.sh
	cp -p bbpost.sh bbpost

datepost:		datepost.sh
	cp -p datepost.sh datepost

bbclean:		bbclean.sh
	cp -p bbclean.sh bbclean

install:		$(ALL)
	bsdinstall $(ALL) $(PCSBIN)

clean:
	-rm -f *.o $(ALL)

cflow:
	cflow -I$(PCS)/include *.c > bb.cflow

control:
	uname -n > Control
	date >> Control


bbfile.o:
	$(CC) -c $(CFLAGS) bbfile.c

main.o:		main.c bb.h config.h
	$(CC) -c $(CFLAGS) main.c


