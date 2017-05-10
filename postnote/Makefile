# MAKEFILE

T= postnote

BINDIR= $(PCS)/bin


ALL= $(T)


all:		$(ALL)

$(T):
	cp -p $(T).sh $(T)

install:	$(ALL)
	bsdinstall $(ALL) $(BINDIR)

clean:
	rm -f $(T)



