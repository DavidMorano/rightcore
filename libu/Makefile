# MAKEFILE

T= libu

ALL= $(T).a $(T).so

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

LDCRTDIR= $(SRCROOT)/lib
VALDIR= $(CGS_VALDIR)


CC= $(CGS_CC)
GPP= $(CGS_GPP)


GPPFLAGS= $(CGS_CCFLAGS)

CCOPTS_GCCOPTSGPP= $(CGS_CCFLAGS)
CCOPTS= $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)
#CCOPTS= -g -Wstrict-aliasing $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB) 


DEF0=
DEF1=
DEF2=
DEF3=
DEF4=
DEF5=
DEF6=
DEF7= $(LF_DEFS)

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5) $(DEF6) $(DEF7)


INCDIRS= -I$(INCDIR)


CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS) $(LF_CFLAGS)

#LD= $(CC)


LDRPATH= /usr/lib/lwp

LDFLAGS= $(LF_LDFLAGS) -R$(CGS_RPATH):$(LDRPATH)


SLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

SLIB0=
SLIB1=
SLIB2=
SLIB3=
SLIB4= $(CGS_LIBS)
SLIB5= $(LF_LIBS) 
SLIB6=
SLIB7=

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


XLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

XLIB0=
XLIB1= -ldam -lb
XLIB2= -ldebug
XLIB3= -luc -lut -lu
XLIB4= $(CFS_LIBS)
XLIB5= 
XLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
XLIB7= -ldl

XLIBS= $(XLIB0) $(XLIB1) $(XLIB2) $(XLIB3) $(XLIB4) $(XLIB5) $(XLIB6) $(XLIB7)

XLIBINFO= $(XLIBDIRS) $(XLIBS)


VALUES= $(VALDIR)/$(CGS_VALUES)

CRT1= $(LDCRTDIR)/crt1.o
CRTI= $(LDCRTDIR)/crti.o
CRTBEGIN= $(LDCRTDIR)/crtbegin.o
MCRT1= $(LDCRTDIR)/mcrt1.o
GCRT1= $(LDCRTDIR)/gcrt1.o
CRTEND= $(LDCRTDIR)/crtend.o
CRTN= $(LDCRTDIR)/crtn.o

CRTFRONT= $(CRT1) $(CRTI) $(VALUES) $(CRTBEGIN)
CRTBACK= $(CRTEND) $(CRTN)

# for regular (no profiling)
CRT0= $(CRT1) $(CRTI) $(VALUES)
# for 'prof(1)'
MCRT0= $(CRTI) $(MCRT1) $(VALUES)
# for 'gprof(1)'
GCRT0= $(CRTI) $(GCRT1) $(VALUES)

LINT= lint
LINTFLAGS= $(LF_LINTFLAGS) -uxn -Dlint

NM= nm
NMFLAGS= -sx -v

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f
TOUCH= /usr/bin/touch


INSTALLINCS= vsystem.h localmisc.h syshas.h

INCS= localmisc.h syshas.h vsystem.h


OBJ00= u_getdents.o
OBJ01= u_mkdir.o u_rmdir.o u_link.o u_symlink.o u_unlink.o u_rename.o u_mknod.o
OBJ02= u_chdir.o u_chmod.o u_lchown.o u_chown.o u_acl.o
OBJ03= u_access.o u_utime.o u_utimes.o u_pathconf.o u_fpathconf.o
OBJ04= u_creat.o u_open.o u_close.o u_dup.o u_dup2.o u_pipe.o 
OBJ05= u_stat.o u_fstat.o u_lstat.o u_statvfs.o u_fstatvfs.o u_readlink.o
OBJ06= u_fcntl.o u_sync.o u_fsync.o u_poll.o u_lockf.o u_ioctl.o u_fchdir.o
OBJ07= u_read.o u_pread.o u_readv.o u_write.o u_pwrite.o u_writev.o
OBJ08= u_seek.o u_seeko.o u_rewind.o u_tell.o
OBJ09= 
OBJ10=
OBJ11=
OBJ12= u_msgget.o u_msgctl.o u_msgsnd.o u_msgrcv.o
OBJ13= u_shmget.o u_shmctl.o u_shmat.o u_shmdt.o
OBJ14=
OBJ15= u_getsid.o u_getpgid.o 
OBJ16= u_stime.o u_adjtime.o u_getloadavg.o u_times.o
OBJ17= u_execvp.o u_execv.o u_execve.o
OBJ18= u_getgroups.o
OBJ19= u_setid.o
OBJ20=
OBJ21= u_sysinfo.o u_uname.o
OBJ22= u_getrlimit.o u_setrlimit.o u_ulimit.o
OBJ23= u_socket.o u_socketpair.o u_listen.o u_accept.o u_bind.o u_connect.o
OBJ24= u_getsockname.o u_getpeername.o u_shutdown.o
OBJ25= u_getsockopt.o u_setsockopt.o
OBJ26= u_recv.o u_recvfrom.o u_recvmsg.o
OBJ27= u_send.o u_sendto.o u_sendmsg.o
OBJ28= u_writen.o u_readn.o
OBJ29= u_mmap.o u_munmap.o u_mprotect.o u_memcntl.o u_mincore.o 
OBJ30= u_mlockall.o u_munlockall.o u_mlock.o u_munlock.o u_plock.o
OBJ31= u_brk.o u_sbrk.o
OBJ32= u_waitpid.o u_waitid.o u_wait.o u_fork.o u_vfork.o u_exit.o
OBJ33= u_sigaction.o u_sigprocmask.o u_sigwait.o u_sigsuspend.o u_sigpending.o
OBJ34= u_sigsend.o u_sigsendset.o u_kill.o u_alarm.o u_pause.o 
OBJ35= u_sigaltstack.o
OBJ36= u_fchmod.o u_fchown.o u_facl.o u_closeonexec.o
OBJ37= u_resolvepath.o
OBJ38= u_nice.o 
OBJ39= 
OBJ40=
OBJ41= u_putmsg.o u_putpmsg.o u_getmsg.o u_getpmsg.o
OBJ42=
OBJ43= 
OBJ44=
OBJ45= msleep.o mtime.o
OBJ46= unixtime.o 
OBJ47=

OBJ48= u_lf64.o 
OBJ49=
OBJ50=
OBJ51=
OBJ52=
OBJ53=
OBJ54=
OBJ55=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)
OBJE= $(OBJ32) $(OBJ33) $(OBJ34) $(OBJ35) $(OBJ36) $(OBJ37) $(OBJ38) $(OBJ39)
OBJF= $(OBJ40) $(OBJ41) $(OBJ42) $(OBJ43) $(OBJ44) $(OBJ45) $(OBJ46) $(OBJ47)
OBJG= $(OBJ48) $(OBJ49) $(OBJ50) $(OBJ51) $(OBJ52) $(OBJ53) $(OBJ54) $(OBJ55)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG)

OBJS= obja.o objb.o objc.o objd.o obje.o objf.o objg.o


.SUFFIXES:		.ls .i .cx .cs


default:		all

all:			$(ALL)

.cc.o:
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.c.ln:
	$(LINT) -c -u $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $< > $(*).i


obja.o:			$(OBJA)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJA)

objb.o:			$(OBJB)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJB)

objc.o:			$(OBJC)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJC)

objd.o:			$(OBJD)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJD)

obje.o:			$(OBJE)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJE)

objf.o:			$(OBJF)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJF)

objg.o:			$(OBJG)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJG)


$(T).a:			$(OBJ)
	$(AR) -rc $(T).a $?

$(T).so:		$(OBJS) Makefile localmisc.h
	$(LD) -G -m -o $@ $(LDFLAGS) $(OBJS) $(SLIBINFO) > $(T).lm

$(T).nm:		$(T).so
	$(NM) $(NMFLAGS) $(T).so > $(T).nm

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

install-pre:
	filefind . -s h | makenewer -af - -d $(INCDIR)

install:		$(ALL) Makefile install-incs
	ranlib $(T).a
	makenewer -r $(ALL) $(LIBDIR)

install-incs:		$(INSTALLINCS)
	makenewer $(INSTALLINCS) $(INCDIR)

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

again:
	rm -f $(ALL)

clean:
	rm -f *.x *.o $(T).a

control:
	(uname -n ; date) > Control


u_poll.o:		u_poll.c $(INCS)

u_statvfs.o:		u_statvfs.c $(INCS)

u_fstatvfs.o:		u_fstatvfs.c $(INCS)


mtime.o:		mtime.c mtime.h


# testing

X01= testsigpending

X01OBJ= $(X01).o u_sigpending.o

X01OBJS= $(CRTFRONT) $(X01OBJ) $(CRTC) $(CRTBACK)

$(X01).o:		$(X01).c

$(X01).x:		$(X01OBJ) Makefile
	$(LD) -o $@ $(X01OBJS) $(XLIBDIRS) $(XLIBS) -lc


