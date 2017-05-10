# MAKEFILE

T= pcscal

SRCROOT= $(PCS)


PCSBIN = /usr/add-on/pcs/bin
PCSLIB = /usr/add-on/pcs/lib

CFLAGS = -g -I/usr/add-on/pcs/include

PRODUCTS= 	pcscal qp caldate prmpt_date calday calpost

all:	$(PRODUCTS)

OBJS=	chk_entry.o \
	delete_entry.o \
	edit_entry.o \
	error.o \
	get_date.o \
	get_entry.o \
	get_time.o \
	get_wday.o \
	prnt_entry.o \
	rewrite.o \
	sort_entry.o

install:	$(PRODUCTS)
	mv $(PRODUCTS) ${PCSBIN}
	chmod +r,+x ${PCSBIN}/calpost ${PCSBIN}/prmpt_date  ${PCSBIN}/calday ${PCSBIN}/pcscal ${PCSBIN}/qp ${PCSBIN}/caldate

pcscal:	cal.o ${OBJS}
		cc -g cal.o ${OBJS} ${PCSLIB}/tsklib.a ${PCSLIB}/parserlib.a -o pcscal

calpost:
		cp calpost.sh calpost
		-chmod +x calpost

qp:		caldate
		cp qp.sh qp
		-chmod +x qp

caldate:	caldate.o get_date.o
	cc -g caldate.o get_date.o ${PCSLIB}/tsklib.a -o caldate

prmpt_date:	prmpt_date.o get_date.o
	cc -g prmpt_date.o get_date.o ${PCSLIB}/tsklib.a -o prmpt_date

calday:	calday.o get_date.o get_wday.o
	cc -g calday.o get_date.o get_wday.o ${PCSLIB}/tsklib.a -o calday
clean:
		-rm -f *.o
		-rm -f  $(PRODUCTS)


