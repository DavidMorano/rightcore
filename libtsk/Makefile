# MAKEFILE

T= libtsk

#SRCROOT= $(LOCAL)


PCSLIB= $(PCS)/lib


CC= gcc

CCOPTS= -O3 -fpic -mcpu=ultrasparc -mno-app-regs

CFLAGS= -O


OBJ=	convdt.o \
	fetch.o \
	lowercase.o \
	match.o \
	num_words.o \
	partial.o \
	rmtab.o \
	split.o \
	undate.o \
	num_month.o \
	word.o \
	ischar.o \
	rmhead.o \
	rmtail.o \
	iswhite.o \
	rmcntrl.o


all:	$(T).a

.c.o:
	$(CC) -c $(CFLAGS) $*.c


$(T).a:	${OBJ}
	ar -cr $@ ${OBJ}
	ranlib $@

install: $(T).a
	 cp $(T).a ${PCSLIB}

clean:
	-rm -f *.o
	-rm -f tsklib.a


