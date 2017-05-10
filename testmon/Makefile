# MAKEFILE

T= testmon

#SRCROOT= $(LOCAL)


CCOPTS= -pg
LDOPTS= -pg


OBJ= main.o


default:	$(T)

$(T):		$(OBJ)
	$(CC) -# -o $@ $(LDOPTS) $(LDFLAGS) $(OBJ) -lm

clean:
	rm -f *.o


main.o:		main.c
	$(CC) -c $(CCOPTS) $(CFLAGS) main.c


