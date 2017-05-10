# Makefile

T= mrfinger


OBJ= main.o


$(T):		$(T).x
	rm -f $(T)
	cp -p $(T).x $(T)

$(T).x:		$(OBJ)
	gcc -o $@ $(OBJ)

safe:
	makesafe -v=3 $(OBJ)

strip:		$(T).x
	strip $(T).x


main.o:		main.c config.h defs.h



