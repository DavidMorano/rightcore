# MAKEFILE (logsession)


T= logsession

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


default:	$(T)

$(T):		$(T).ksh
	rm -f $(T)
	cp -p $(T).ksh $(T)

install:	$(T)
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r $(T) $(BINDIR)

clean:
	rm -f $(T)



