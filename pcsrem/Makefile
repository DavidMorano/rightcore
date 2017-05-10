# makefile for reminder service modules
# J. A. Kutsch  ho 43233  x-3059
# January 1981
# Modified by Carole Strada
# July 1981

REMDIR = /usr/add-on/pcs/lib/remdata
PCSBIN = /usr/add-on/pcs/bin
CFLAGS = -O -I/usr/add-on/pcs/include

PRODUCTS=rem_pgm cal2rem  rem rem_add rem_mon mailon mailoff remday files remtoday

all:	$(PRODUCTS)

install: 	$(PRODUCTS)
	if [ ! -d $(REMDIR) ] ; then mkdir $(REMDIR); fi
	-chmod +x $(REMDIR)
	cp rem_mon ${REMDIR}/rem_mon
	cp rem_add ${REMDIR}/rem_add
	cp mailon ${REMDIR}/mailon
	cp mailoff ${REMDIR}/mailoff
	cp remday ${PCSBIN}/remday
	cp remtoday ${REMDIR}/remtoday
	cp cal2rem ${PCSBIN}/cal2rem
	mv rem_pgm ${REMDIR}/rem_pgm
	mv rem ${PCSBIN}/rem
	-chmod +r,+x ${REMDIR}/rem_pgm ${PCSBIN}/rem ${PCSBIN}/cal2rem ${PCSBIN}/remday ${REMDIR}/mailoff ${REMDIR}/mailon ${REMDIR}/rem_add ${REMDIR}/rem_mon ${REMDIR}/remtoday

rem_pgm:	rem_pgm.o mc.o notify.o
		cc rem_pgm.o mc.o notify.o -o rem_pgm



rem_pgm.o:	remfile.h


rem:	rem.o parse.o tdata.o index.o
	cc rem.o parse.o tdata.o index.o -o rem

rem.o:	remfile.h

cal2rem:	 
	cp cal2rem.sh cal2rem

remday:	
	cp remday.sh remday

remtoday:
	cp remtoday.sh remtoday

mailon:	
	cp mailon.sh mailon

mailoff:	
	cp mailoff.sh mailoff

rem_add:	
	cp rem_add.sh rem_add
rem_mon:	
	cp rem_mon.sh rem_mon

files:
	if [ ! -d $(REMDIR) ] ; then mkdir $(REMDIR); fi
	-chmod +x $(REMDIR)
	touch ${REMDIR}/remfile
	-chmod +rw ${REMDIR}/remfile
	touch ${REMDIR}/mc_users
	-chmod +rw ${REMDIR}/mc_users

clean:
	-rm -f *.o
	-rm -f cal2rem remday mailoff mailon rem_add rem_mon remtoday

lpr:
	-pr Makefile remfile.h *.c *.sh | lpr



