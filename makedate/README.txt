MAKEDATE

This program is used to create build dates for object builds.

Synopsis (in a Makefile):
	makedate [-m] <module_name> > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) ...

Arguments:
<module_name>	is the name of the module being built

