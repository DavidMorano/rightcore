# makefile for SYSCAD local area functions

INCS= schema.h

INCDIRS= -I$(SRCROOT)/include
LIBDIRS=
LIBS=

DEFINES= -DSOLARIS=1 -D$(OS)=1
CFLAGS= -O $(DEFINES) $(INCDIRS) 


.c.o:		Makefile
	$(CC) $(CFLAGS) -c $<
	-chmod o+rw $@


ALL0= audit.o
ALL1= disco.o
ALL2= test.o test_show.o test_select.o
ALL= $(ALL0) $(ALL1) $(ALL2)


all:		$(ALL) $(INCS) bitops.h

clean:
	rm -f *.o


bitops.h:	/home/dam/include/bitops.h
	cp /home/dam/include/bitops.h bitops.h


# do not copy over the distribution version since it has a bug in it
#schema.h:	$(SCHEMARUN)/schema.h
#	cp $(SCHEMARUN)/schema.h .

discofunc.li:	discofunc.c
	lint -uxn $(INCDIRS) discofunc.c

extract_st.li:	extract_st.c
	lint -uxn $(INCDIRS) extract_st.c



