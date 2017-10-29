# MAKEFILE (testmake)



CC= gcc


UUCLIB= libuuc.a

OBJ= a.o b.o


default:	$(UUCLIB)

ucc(%o):	%.o


$(UUCLIB):	$(OBJ)
	$(AR) -r $@ $?



