# MAKEFILE for 'libterm.a'

T= libterm

PCS= /usr/add-on/pcs


PCSLIB= $(PCS)/lib


CC= gcc

CCOPTS= -O3 -fpic -mcpu=ultrasparc -mno-app-regs

CFLAGS= $(CCOPTS) -I$(PCS)/include

LIB= libterm.a


# termlib maker 
#
LIBOBJS= $(LIB)(ctags.o) \
	$(LIB)(termcap.o) \
	$(LIB)(tgoto.o) \
	$(LIB)(tputs.o)

$(LIB):: $(LIBOBJS)
	@echo $(LIB) now up-to-date

$(LIBOBJS): $(FRC)

FRC:
	rm -f $(LIB)

install: $(LIB)
	cp $(LIB) $(PCSLIB)/$(LIB)

clean:
	rm -f *.o
	rm -f termlib.a

clobber:
	-rm -f $(LIB)


.PRECIOUS: $(LIB)



