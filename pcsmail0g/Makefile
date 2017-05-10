# Makefile Thu May 31 16:45:42 EDT 1984
#
# Makefile for PCS mail sending program which serves the functions of
# sending mail, info, pc and mail receiving deamon


PCS= $(HW)/add-on/pcs

PCSMAIL= pcsmail
RPCSMAIL= rpcsmail
SMAIL= smail
PC= pc
PCSINFO= pcsinfo
INFO= info
SENDMAIL= sendmail

ALL= $(PCSMAIL) $(RPCSMAIL) $(SMAIL) $(PC) $(PCSINFO) $(SENDMAIL)



PCSBIN= $(PCS)/bin
PCSLIB= $(PCS)/lib

#CC= cc 
CFLAGS= -g -I$(PCS)/include -DSUN=1

LIBDIRS= -L$(PCS)/lib
LIBS= -lparser -lpcs -lmisc -lb -lc
LDFLAGS=


HDRS=   config.h header.h prompt.h smail.h

SRC=	defs.c bld_links.c\
	checkit.c checkname.c checklist.c cmpnames.c getname.c cookname.c \
	deliv_mbag.c deliverem.c doinfo.c editit.c fappend.c fetchfrom.c \
	findname.c  genname.c getdate.c \
	getheader.c getnames.c getpchdr.c getoptions.c gettable.c getvars.c \
	header.c help.c mailit.c  nmalloc.c\
	pipeit.c prompt.c putheader.c redomsg.c \
	reviewit.c rmtemp.c main.c \
	mm_getfield.c getfield.c



OBJ= $(SRC:.c=.o)


.c.o:	config.h
	$(CC) -c $(CFLAGS) $*.c


all:	$(ALL)

$(PC):	$(PCSMAIL)
	-ln $(PCSMAIL) $(PC) 2> /dev/null

$(SENDMAIL):	$(PCSMAIL)
	-ln $(PCSMAIL) $(SENDMAIL) 2> /dev/null

$(PCSINFO):	$(PCSMAIL)
	-ln $(PCSMAIL) $(PCSINFO) 2> /dev/null

$(SMAIL):	$(PCSMAIL)
	-ln $(PCSMAIL) $(SMAIL) 2> /dev/null

$(RPCSMAIL):	$(OBJ) Makefile
	-ln $(PCSMAIL) $(RPCSMAIL) 2> /dev/null

$(PCSMAIL):	$(OBJ) Makefile
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIBDIRS) $(LIBS)


defs.o:		defs.c smail.h

main.o:	main.c $(HDRS)
	$(CC) -c $(CFLAGS) -DMASTER=1 main.c

bld_links.o: smail.h config.h

checkit.o: smail.h config.h header.h

checkname.o: smail.h config.h

checklist.o: smail.h config.h

cmpnames.o: smail.h config.h

getname.o: smail.h config.h

cookname.o: smail.h config.h

deliv_mbag.o: smail.h config.h 

deliverem.o: smail.h config.h

doinfo.o: smail.h config.h

editit.o: smail.h config.h header.h

fappend.o: smail.h config.h

fetchfrom.o: smail.h config.h

findname.o: smail.h config.h

genname.o: smail.h config.h

getheader.o: smail.h config.h header.h

getnames.o: smail.h config.h

getoptions.o: smail.h config.h

getpchdr.o: smail.h config.h header.h

gettable.o: smail.h config.h

getvars.o: smail.h config.h

header.o: smail.h config.h header.h

mailit.o: smail.h config.h  header.h

netpost.o: smail.h config.h header.h

nmalloc.o: smail.h config.h

pipeit.o: smail.h config.h header.h

prompt.o: smail.h config.h prompt.h header.h

putheader.o: smail.h config.h

redomsg.o: smail.h config.h header.h

reviewit.o: smail.h config.h header.h



clean:
	-rm -f *.o $(PCSMAIL)

install:	$(ALL)
	install -m 775 $(PCSMAIL) $(PCSBIN)/npcsmail
	install -m 775 $(PCSMAIL) $(PCSBIN)/nrpcsmail
	install -m 775 $(PCSMAIL) $(PCSBIN)/nsmail
	install -m 775 $(PCSMAIL) $(PCSBIN)/npc
	install -m 775 $(PCSMAIL) $(PCSBIN)/npcsinfo
	install -m 775 $(PCSMAIL) $(PCSBIN)/nsendmail



