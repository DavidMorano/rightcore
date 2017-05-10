#################################################
#                                               #
# BWNFSD Makefile Version 3.0g 09/10/92     fjw #
#                                               #
#################################################
#                                               #
# One can enable various debugging options here #
#                                               #
#################################################
CC= gcc
CC2= gcc

##########################
#                        #
# Dependencies Section:  #
#                        #
##########################
make:
	@echo 'Make what?  You must tell which system to make BWNFSD for:'
	@echo 'make sun     - Sun or compatible running Sun-OS'
	@echo 'make bsd     - Generic Berkeley Unix'
	@echo 'make sys3    - Generic ATT System III or System V'
	@echo 'make sys5    - Generic ATT System III or System V'
	@echo 'make svr4    - ATT System V.4 UNIX'
	@echo 'make univel  - Univel System VR4 variant'
	@echo 'make solaris - Solaris 2.1'
	@echo 'make hpux    - HPUX 8.07 System (Possibly others)'
	@echo 'make sgi     - Silicon Graphics machine'
	@echo 'make sco     - Santa Cruz Organization (SCO)'
	@echo 'make aix     - IBM AIX for RS/6000'


################################
#                              #
# Please add your system here  #
#                              #
################################
OBJS    = bwnfsd.o bwprint.o bwlock.o

all:		bwnfsd

bwnfsd:		$(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS)

clean:
	rm -f $(OBJS)


########################################
#
# Possible defines and their uses include:
# INT_SIG   - Indicates that your system wants to see the signal() function
#             take an integer functional argument, rather than void.
# BSD_QUEUE - Indicates that the commands to manipulate the print queues
#             are lpq, lprm, etc.
# AIX_QUEUE - Indicates that the commands to manipulate the print queues
#             are enq and such
# SVR4_QUEUE - Indicates that the commands to manipulate the print queues
#             are lpstat, cancel, etc.
# SCO_QUEUE - Indicates that the commands to manipulate the print queues
#             are lpstat, cancel, etc., but different from SVR4
# SVR4_PRINTING - Indicates that /usr/spool/lp/... files are used to determine
#                 what print queues are available.  If not defined, then
#                 /etc/printcap is parsed.
# FOURTEEN_CHAR - Indicates that there is a fourteen character limit on
#                 filenames.
# HP8_PRINTING - Indicates that SVR4 type utilities are to be used to print
#                with, but /etc/printcap mechanism is to be used to get the
#                printers.  Should be modified to do HP properly at a later
#                date.

# SYSV32 - Indicates a System V release III derivative system
# AIX - Indicates an AIX system
# SCO - Indicates an SCO system
# SGI - Indicates an SGI system
# PORTMAP - Indicates that a Univel machine should include some include files
#           that it otherwise doesn't
# SHADW - Indicates that a Shadow password file is in use
# Make commands for specific systems:
#
########################################
#Sun
sun:
	make bwnfsd "CFLAGS= -O -DBSD_QUEUE"

#Berkeley Unix
bsd:
	make bwnfsd "CFLAGS= -O -DBSD_QUEUE"

#Generic ATT System III or System V (not R4)
sys5:
	make bwnfsd "CFLAGS= -O -DSYSV32 -DBSD_QUEUE -DFOURTEEN_CHAR"

#Generic ATT System III or System V (not R4)
sys3:
	make bwnfsd "CFLAGS= -O -DSYSV32 -DBSD_QUEUE -DFOURTEEN_CHAR"

#Generic System V R4
svr4:
	make bwnfsd "CFLAGS= -O -DINT_SIG -DFOURTEEN_CHAR -DSVR4 -DSVR4_PRINTING -DSVR4_QUEUE -DSHADW -lrpcsvc -lsocket -lresolv -lnsl -lx -lnls  /usr/ucblib/libucb.a"

#Univel System V R4
univel:
	make bwnfsd "CFLAGS= -O -DPORTMAP -DSVR4 -DSVR4_PRINTING -DSVR4_QUEUE -DSHADW -lrpcsvc -lsocket -lresolv -lnsl -lx -lnls  /usr/ucblib/libucb.a"

#Solaris 2.1
solaris:
	make bwnfsd "CFLAGS= -w -O -DINT_SIG -DSVR4 -DSVR4_PRINTING -DSVR4_QUEUE -DSHADW -lrpcsvc -lsocket -lresolv -lnsl -lnls  /usr/ucblib/libucb.a"

#IBM RS/6000 or PS/2 running AIX
aix:
	make bwnfsd "CFLAGS= -O -DAIX -DAIX_QUEUE"

#Silicon Graphics machine
sgi:
	make bwnfsd "CFLAGS= -O -cckr -I/usr/include/sun -I/usr/include/bsd -DSYSV32 -DSGI -DBSD_QUEUE -lrpcsvc -lsun -lbsd"

#Intel hardware running Santa Cruz Operation's SCO Unix
sco:
	make bwnfsd "CFLAGS= -O -DSYSV32 -DSCO -DSVR4_PRINTING -DSCO_QUEUE -DFOURTEEN_CHAR -lprot -lrpcsvc -lrpc -lsocket -lc_s"

#HPUX Version 8.07 (possibly others)
hpux:
	make bwnfsd "CFLAGS= -O -DSVR4_QUEUE -DHP8_PRINTING"


