# MAKEFILE

T= stripopgarb

#SRCROOT= $(LOCAL)

OFF= irix


CCOPTS= -O


OBJS= main.o fgetline.o


$(T).$(OFF):	$(OBJS)
	$(CC) -o $@ main.o fgetline.o

clean:
	rm -f $(T) *.o


main.o:		main.c
	$(CC) -c $(CCOPTS) $<

fgetline.o:	fgetline.c
	$(CC) -c $(CCOPTS) $<


