# MAKEFILE

T= trans

#SRCROOT= $(LOCAL)


BINDIR= $(HOME)/bin


CC= gcc


$(T):		main.c
	$(CC) -o $@ main.c


install:	$(T)
	cp -p $(T) $(BINDIR)

strip:		$(T)
	strip $(T)

clean:
	rm -f $(T) *.o


