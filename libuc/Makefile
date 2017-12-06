# MAKEFILE

T= libuc
TT= libuctest

ALL= $(T).a $(T).so

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

LDCRTDIR= $(CGS_CRTDIR)
VALDIR= $(VGS_VALDIR)


CC= $(CGS_CC)
GPP= $(CGS_GPP)

GPPFLAGS= $(CGS_CCFLAGS)

CCOPTS_GCCOPTSGPP= $(CGS_CCFLAGS)
CCOPTS= $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)
#CCOPTS= -g -Wstrict-aliasing $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB) 
#CCOPTS= -g -Wall $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB) 


LDA=
# must be ON (no longer strictly necessary)
LDA += -z initarray=ucatfork_init -z finiarray=ucatfork_fini
# optional
#LDA += -z initarray=ucmemalloc_init -z finiarray=ucmemalloc_fini
#LDA += -z initarray=ucfork_init -z finiarray=ucfork_fini
#LDA += -z initarray=ugetpid_init -z finiarray=ugetpid_fini
#LDA += -z initarray=ugetpw_init -z finiarray=ugetpw_fini
#LDA += -z initarray=uinfo_init -z finiarray=uinfo_fini
#LDA += -z initarray=umask_init -z finiarray=umask_fini
#LDA += -z initarray=utmpacc_init -z finiarray=utmpacc_fini
#LDA += -z initarray=uproguser_init -z finiarray=uproguser_fini
#LDA += -z initarray=uprogdata_init -z finiarray=uprogdata_fini
#LDA += -z initarray=uclustername_init -z finiarray=uclustername_fini


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
CCFLAGS= $(GPPFLAGS) $(CFLAGS)

#LD= $(CC)

NM= nm
NMFLAGS= -sx -v

CPP= cpp

LORDER= lorder
TSORT= ${CCS}/bin/tsort

RM= rm -f
TOUCH= /bin/touch


LDRPATH= $(EXTRA)/lib

RLDFLAGS= $(LF_LDFLAGS)


SLDFLAGS= $(LF_LDFLAGS) -m $(LDA) -R$(LIBDIR):$(CGS_RPATH):$(LDRPATH)

SLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

SLIB0=
SLIB1=
SLIB2=
SLIB3= -lu
SLIB4= $(CGS_LIBS)
SLIB5= $(LF_LIBS) 
SLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
SLIB7= -ldl

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


XLDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(CGS_RPATH):$(LDRPATH)

XLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

XLIB0=
XLIB1= -ldam -lb
XLIB2= -ldebug
XLIB3= -luc -lu
XLIB4= $(CGS_LIBS)
XLIB5= $(LF_LIBS)
XLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
XLIB7= -ldl

XLIBS= $(XLIB0) $(XLIB1) $(XLIB2) $(XLIB3) $(XLIB4) $(XLIB5) $(XLIB6) $(XLIB7)

XLIBINFO= $(XLIBDIRS) $(XLIBS)


LINT= lint
LINTFLAGS= $(LF_LINTFLAGS) -uxn -Dlint


INCS= vsystem.h localmisc.h


OBJ000= uc_sysconf.o uc_confstr.o uc_gethostname.o
OBJ001= uc_memalloc.o uc_mallocstrw.o uc_mallocbuf.o
OBJ002= uc_open.o uc_openinfo.o 
OBJ003= uc_opensocket.o uc_openproto.o uc_openprog.o uc_openpass.o uc_openuser.o
OBJ004= uc_openfsvc.o uc_openfint.o uc_openusvc.o uc_opendialer.o
OBJ005= uc_opensys.o uc_opendev.o uc_create.o uc_createfile.o uc_joinus.o
OBJ006= uc_readn.o uc_reade.o uc_readline.o 
OBJ007= uc_write.o uc_writen.o uc_writefile.o uc_writedesc.o
OBJ008= uc_recve.o uc_recvfrome.o uc_recvmsge.o uc_peek.o
OBJ009= uc_remove.o uc_connecte.o uc_stat.o uc_lstat.o uc_statvfs.o uc_fstat.o
OBJ010= uc_fsync.o uc_fdatasync.o uc_fchmodsuid.o uc_chmodsuid.o uc_fsize.o
OBJ011= uc_moveup.o uc_closeonexec.o uc_setappend.o uc_keepalive.o
OBJ012= uc_nonblock.o uc_ndelay.o uc_msgdiscard.o uc_reuseaddr.o uc_linger.o
OBJ013= uc_aioread.o uc_aiowrite.o uc_aioreturn.o uc_aiolist.o uc_aiosuspend.o
OBJ014= uc_fattach.o uc_fdetach.o uc_mkfifo.o uc_msync.o uc_aiofsync.o 
OBJ015= uc_accepte.o uc_lockf.o uc_lockfile.o uc_lockend.o uc_fpassfd.o
OBJ016= uc_close.o
OBJ017= uc_realpath.o uc_readlink.o uc_rename.o
OBJ018= uc_getcwd.o uc_initgroups.o uc_getpuid.o uc_getnetname.o uc_procpid.o
OBJ019= uc_chmod.o uc_chown.o
OBJ020= uc_getproto.o uc_getnet.o uc_gethost.o uc_getserv.o 
OBJ021= uc_getipnodeby.o uc_getnameinfo.o uc_getaddrinfo.o 
OBJ022= uc_inetpton.o uc_inetntop.o
OBJ023= uc_ftime.o uc_mktime.o uc_clock.o
OBJ024= uc_getlogin.o uc_getrusage.o uc_getloadavg.o
OBJ025= uc_system.o uc_localtime.o uc_gmtime.o uc_ztime.o uc_sigset.o 
OBJ026=
OBJ027= uc_raise.o uc_killpg.o uc_sigqueue.o uc_syncer.o
OBJ028= uc_sigwaitinfo.o uc_sigpause.o
OBJ029= uc_sigdefault.o uc_sigignore.o uc_sighold.o uc_sigrelease.o
OBJ030= uc_settimeofday.o uc_gettimeofday.o
OBJ031= uc_getpw.o uc_getsp.o uc_getgr.o 
OBJ032= uc_getpj.o uc_getdefaultproj.o uc_inproj.o 
OBJ033= uc_getus.o uc_getuserattr.o uc_kvamatch.o
OBJ034= 
OBJ035= uc_cfsetispeed.o uc_cfsetospeed.o
OBJ036= uc_openpt.o uc_grantpt.o uc_unlockpt.o uc_ptsname.o uc_ttyname.o
OBJ037= uc_truncate.o uc_ftruncate.o
OBJ038=
OBJ039= uc_safesleep.o uc_atfork.o uc_atexit.o uc_exit.o 
OBJ040= uc_setpriority.o uc_getpriority.o 
OBJ041= uc_openshm.o uc_openshmto.o uc_unlinkshm.o 
OBJ042= uc_openpsem.o uc_unlinkpsem.o
OBJ043= uc_isatty.o uc_fminmod.o uc_minmod.o uc_link.o uc_symlink.o uc_unlink.o 
OBJ044= uc_mkdir.o uc_rmdir.o uc_access.o uc_utime.o uc_utimes.o uc_pathconf.o
OBJ045= 
OBJ046= uc_madvise.o uc_mlock.o uc_munlock.o uc_mlockall.o uc_munlockall.o
OBJ047= uc_plock.o 
OBJ048= uc_nprocessors.o uc_syspages.o uc_fork.o uc_execve.o uc_isaexecve.o 
OBJ049= uc_nprocs.o
OBJ050= uc_strtobase.o uc_strtod.o 
OBJ051= uc_waitwritable.o
OBJ052= 
OBJ053= uc_piper.o uc_thr.o uc_timer.o uc_timeout.o
OBJ054= uc_tcsetattr.o uc_tcgetattr.o uc_tcdrain.o uc_tcflush.o uc_tcflow.o
OBJ055= uc_tcsetpgrp.o uc_tcgetpgrp.o uc_tcgetsid.o
OBJ056= uc_libmemalloc.o
OBJ057= uc_getrandom.o uc_getsocktype.o
OBJ058=
OBJ059= pt_sigmask.o pt_atfork.o
OBJ060=
OBJ061= getrandom.o
OBJ062=
OBJ063=

OBJ064= getprotofamily.o geteaddrinfo.o getaf.o getaflen.o getpwd.o
OBJ065= getnodedomain.o getutmpent.o getax.o getuserhome.o getxusername.o
OBJ066= getbufsize.o gethz.o getngroups.o getproto.o getserv.o
OBJ067= getportnum.o getenver.o getourenv.o
OBJ068= inetpton.o perm.o getprogpath.o findxfile.o xfile.o
OBJ069= nextpowtwo.o ndigits.o ipow.o llpow.o findbit.o
OBJ070= isFailOpen.o isFailConn.o
OBJ071= isNotPresent.o isNotAccess.o isNotValid.o isIOError.o isOneOf.o

OBJ072= filebuf.o filebuf_writers.o
OBJ073=
OBJ074= vecstr.o vecstr_adduniq.o vecstr_adduniqs.o vecstr_addpath.o
OBJ075= vecstr_env.o vecstr_envfile.o vecstr_loadfile.o
OBJ076= vecpstr.o vecpstr_loadfile.o vecpstr_addpath.o
OBJ077= vecpstr_envadd.o vecpstr_envfile.o
OBJ078= pta.o ptm.o ptma.o ptc.o ptca.o psem.o lockrw.o envhelp.o
OBJ079= storeitem.o storebuf.o buffer.o field.o char.o mkprogenv.o

OBJ080= fsdir.o pmq.o sigblock.o userattr.o filemap.o uname.o 
OBJ081= vechand.o vecsorthand.o vecobj.o vecelem.o recarr.o
OBJ082= envlist.o hdb.o lookaside.o ciq.o pq.o ids.o linefold.o nulstr.o 
OBJ083= dirseen.o dirseen_not.o
OBJ084= strpack.o
OBJ085= varray.o dirlist.o randomvar.o strmgr.o sigign.o ucsem.o
OBJ086= sbuf.o sbuf_loadstrs.o sbuf_addquoted.o 
OBJ087= hostinfo.o hostaddr.o hostent.o inetaddr.o sockaddress.o upwcache.o

OBJ088= strnstr.o strncasestr.o strnsub.o strncasesub.o 
OBJ089= strncpylc.o strncpyuc.o strncpyfc.o
OBJ090= strcpyuc.o strcpylc.o strcpyfc.o
OBJ091= strnkeycmp.o strnleadcmp.o strnncmp.o strnlen.o strnnlen.o
OBJ092= strnchr.o strnrchr.o strnpbrk.o strnrpbrk.o
OBJ093= strncpyblanks.o strnset.o strnwcpy.o
OBJ094= strwcpy.o strwcpylc.o strwcpyuc.o
OBJ095= strdcpy.o strdcpyw.o

OBJ096= snwcpy.o snwcpyclean.o snwcpyshrink.o sntmtime.o snshellunder.o 
OBJ097= sncpy.o sncpyw.o sncpylc.o sncpyuc.o
OBJ098= snflags.o snfsflags.o snpollflags.o snopenflags.o
OBJ099= snsds.o snsd.o 
OBJ100= pathadd.o pathclean.o
OBJ101= mkpath.o mknpath.o mkpathw.o mknpathw.o
OBJ102= mkvarpath.o mkuserpath.o mkcdpath.o mksofname.o mkexpandpath.o
OBJ103= mkquoted.o 

OBJ104= sichr.o rmext.o
OBJ105= sfskipwhite.o
OBJ106= sfshrink.o sfdirname.o sfbasename.o sfwhitedot.o sfprogroot.o
OBJ107= nextfield.o 
OBJ108=
OBJ109= matstr.o matcasestr.o matostr.o matocasestr.o matkeystr.o 
OBJ110= strkeycmp.o strwcmp.o
OBJ111= vstrcmp.o vstrcmpr.o vstrkeycmp.o

OBJ112= 
OBJ113= 
OBJ114= 
OBJ115= upt.o uptspawn.o tmtime.o spawnproc.o dupup.o nlsdialassist.o
OBJ116= mkcleanline.o tabcols.o checkbase.o ncolchar.o ncolstr.o
OBJ117= mapex.o
OBJ118= calstrs.o
OBJ119= toxc.o base64.o 

OBJ120= uinfo.o umask.o ugetpw.o utmpacc.o uproguser.o uprogdata.o ugetpid.o
OBJ121= uclustername.o nisdomainname.o
OBJ122= opentmpfile.o opensysfs.o opensys_banner.o
OBJ123= opensockaddr.o openaddrinfo.o
OBJ124=
OBJ125= getuid_name.o getgid_name.o getpjid_name.o
OBJ126= passwdent.o shadowent.o groupent.o projectent.o userattrent.o
OBJ127= utmpaccent.o

OBJ128= sigaction.o sigevent.o 
OBJ129= timeval.o itimerspec.o timespec.o timeout.o
OBJ130= sigignores.o sigdefaults.o
OBJ131= isasocket.o isinetaddr.o isindomain.o
OBJ132= hasNotDots.o
OBJ133= nleadstr.o nleadcasestr.o nleadkeystr.o
OBJ134= format.o
OBJ135= getrootdname.o getdig.o randlc.o

OBJ136= ctbin.o ctoct.o ctdec.o cthex.o ctdecp.o ctdecf.o cvtdig.o
OBJ137= cfbin.o cfoct.o cfdec.o cfdecmf.o cfhex.o cfnum.o cfhexs.o hexval.o
OBJ138= intceil.o intfloor.o intsat.o
OBJ139= isalphalatin.o isalnumlatin.o isdigitlatin.o ishexlatin.o
OBJ140= isdict.o isprintlatin.o isprintbad.o
OBJ141= hasuc.o haslc.o haswhite.o hasalldig.o hasallbase.o
OBJ142= hasprintbad.o
OBJ143= hasnonpath.o hasvarpathprefix.o hascdpath.o

OBJ144= 
OBJ145= ucontext_rtn.o
OBJ146= dialopts.o
OBJ147=
OBJ148= dialuss.o dialussmux.o dialussnls.o
OBJ149= dialtcp.o dialtcpmux.o dialtcpnls.o 
OBJ150= dialudp.o dialusd.o dialpass.o
OBJ151= dialprog.o dialhttp.o dialfinger.o

OBJ152= openshmtmp.o
OBJ153=
OBJ154=
OBJ155=
OBJ156=
OBJ157=
OBJ158=
OBJ159=

OBJAa= $(OBJ000) $(OBJ001) $(OBJ002) $(OBJ003)
OBJAb= $(OBJ004) $(OBJ005) $(OBJ006) $(OBJ007) 
OBJA= $(OBJAa) $(OBJAb)

OBJBa= $(OBJ008) $(OBJ009) $(OBJ010) $(OBJ011)
OBJBb= $(OBJ012) $(OBJ013) $(OBJ014) $(OBJ015)
OBJB= $(OBJBa) $(OBJBb)

OBJCa= $(OBJ016) $(OBJ017) $(OBJ018) $(OBJ019)
OBJCb= $(OBJ020) $(OBJ021) $(OBJ022) $(OBJ023)
OBJC= $(OBJCa) $(OBJCb)

OBJDa= $(OBJ024) $(OBJ025) $(OBJ026) $(OBJ027)
OBJDb= $(OBJ028) $(OBJ029) $(OBJ030) $(OBJ031)
OBJD= $(OBJDa) $(OBJDb)

OBJEa= $(OBJ032) $(OBJ033) $(OBJ034) $(OBJ035)
OBJEb= $(OBJ036) $(OBJ037) $(OBJ038) $(OBJ039)
OBJE= $(OBJEa) $(OBJEb)

OBJFa= $(OBJ040) $(OBJ041) $(OBJ042) $(OBJ043)
OBJFb= $(OBJ044) $(OBJ045) $(OBJ046) $(OBJ047)
OBJF= $(OBJFa) $(OBJFb)

OBJGa= $(OBJ048) $(OBJ049) $(OBJ050) $(OBJ051)
OBJGb= $(OBJ052) $(OBJ053) $(OBJ054) $(OBJ055)
OBJG= $(OBJGa) $(OBJGb)

OBJHa= $(OBJ056) $(OBJ057) $(OBJ058) $(OBJ059)
OBJHb= $(OBJ060) $(OBJ061) $(OBJ062) $(OBJ063)
OBJH= $(OBJHa) $(OBJHb)

OBJIa= $(OBJ064) $(OBJ065) $(OBJ066) $(OBJ067)
OBJIb= $(OBJ068) $(OBJ069) $(OBJ070) $(OBJ071)
OBJI= $(OBJIa) $(OBJIb)

OBJJa= $(OBJ072) $(OBJ073) $(OBJ074) $(OBJ075)
OBJJb= $(OBJ076) $(OBJ077) $(OBJ078) $(OBJ079)
OBJJ= $(OBJJa) $(OBJJb)

OBJKa= $(OBJ080) $(OBJ081) $(OBJ082) $(OBJ083)
OBJKb= $(OBJ084) $(OBJ085) $(OBJ086) $(OBJ087)
OBJK= $(OBJKa) $(OBJKb)

OBJLa= $(OBJ088) $(OBJ089) $(OBJ090) $(OBJ091)
OBJLb= $(OBJ092) $(OBJ093) $(OBJ094) $(OBJ095)
OBJL= $(OBJLa) $(OBJLb)

OBJMa= $(OBJ096) $(OBJ097) $(OBJ098) $(OBJ099)
OBJMb= $(OBJ100) $(OBJ101) $(OBJ102) $(OBJ103)
OBJM= $(OBJMa) $(OBJMb)

OBJNa= $(OBJ104) $(OBJ105) $(OBJ106) $(OBJ107)
OBJNb= $(OBJ108) $(OBJ109) $(OBJ110) $(OBJ111)
OBJN= $(OBJNa) $(OBJNb)

OBJOa= $(OBJ112) $(OBJ113) $(OBJ114) $(OBJ115)
OBJOb= $(OBJ116) $(OBJ117) $(OBJ118) $(OBJ119)
OBJO= $(OBJOa) $(OBJOb)

OBJPa= $(OBJ120) $(OBJ121) $(OBJ122) $(OBJ123)
OBJPb= $(OBJ124) $(OBJ125) $(OBJ126) $(OBJ127)
OBJP= $(OBJPa) $(OBJPb)

OBJQa= $(OBJ128) $(OBJ129) $(OBJ130) $(OBJ131)
OBJQb= $(OBJ132) $(OBJ133) $(OBJ134) $(OBJ135)
OBJQ= $(OBJQa) $(OBJQb)

OBJRa= $(OBJ136) $(OBJ137) $(OBJ138) $(OBJ139)
OBJRb= $(OBJ140) $(OBJ141) $(OBJ142) $(OBJ143)
OBJR= $(OBJRa) $(OBJRb)

OBJSa= $(OBJ144) $(OBJ145) $(OBJ146) $(OBJ147)
OBJSb= $(OBJ148) $(OBJ149) $(OBJ150) $(OBJ151)
OBJS= $(OBJSa) $(OBJSb)

OBJTa= $(OBJ152) $(OBJ153) $(OBJ154) $(OBJ155)
OBJTb= $(OBJ156) $(OBJ157) $(OBJ158) $(OBJ159)
OBJT= $(OBJTa) $(OBJTb)


OBJg0= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG) $(OBJH) 
OBJg1= $(OBJI) $(OBJJ) $(OBJK) $(OBJL) $(OBJM) $(OBJN) $(OBJO) $(OBJP)
OBJg2= $(OBJQ) $(OBJR) $(OBJS) $(OBJT)

OBJ= $(OBJg0) $(OBJg1) $(OBJg2)


SOBJ0= obja.o objb.o objc.o objd.o obje.o objf.o objg.o objh.o obji.o objj.o
SOBJ1= objk.o objl.o objm.o objn.o objo.o objp.o objq.o objr.o objs.o objt.o

SOBJ= $(SOBJ0) $(SOBJ1)


TOBJ00= uc_sysconf.o uc_gethostid.o uc_confstr.o uc_gethostname.o

#TOBJ01a= uc_memalloc.o 
TOBJ01b= uc_mallocstrw.o uc_mallocbuf.o

TOBJ01= $(TOBJ01a) $(TOBJ01b) $(TOBJ01c) $(TOBJ01d)

#TOBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
#TOBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) 
# working
#TOBJA= $(OBJ02) $(OBJ03) 
# not-working
#TOBJA= $(OBJ00) $(OBJ01) 
# not-working
TOBJA= $(TOBJ01) 
# working
#TOBJA= $(TOBJ00) 

#TOBJG0= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG) $(OBJH) 
#TOBJG0= $(OBJE) $(OBJF) $(OBJG) $(OBJH) 
#TOBJG0= $(OBJA) $(OBJB) $(OBJC) $(OBJD) 
#TOBJG0= $(OBJA) $(OBJB) 
#TOBJG0= $(OBJB) 
TOBJG0= $(TOBJA) 
#TOBJG0= $(OBJC) $(OBJD) 
#TOBJG1= $(OBJI) $(OBJJ) $(OBJK) $(OBJL) $(OBJM) $(OBJN) $(OBJO) $(OBJP)
#TOBJG1= $(OBJI) $(OBJJ) $(OBJK) $(OBJL) 
#TOBJG1= $(OBJM) $(OBJN) $(OBJO) $(OBJP)

TOBJ= $(TOBJG0) $(TOBJG1)


.SUFFIXES:		.ls .i .cx .cs


default:		all

all:			$(ALL)

so:			$(T).so

a:			$(T).a

.cc.o:
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.c.ln:
	$(LINT) -c $(LINTFLAGS) $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $< > $(*).i


$(TT).a:		$(TOBJ)
	$(AR) -rc $@ $?

$(T).so:		$(SOBJ) Makefile $(T).a
	$(LD) -G -o $@ $(SLDFLAGS) $(SOBJ) $(SLIBINFO) > $(T).lm

$(T).a:			$(OBJ)
	$(AR) -rc $@ $?

$(T).nm:		$(T).so
	$(NM) $(NMFLAGS) $(T).so > $(T).nm

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

intall-pre:
	filefind . -s h | makenewer -af - -d $(INCDIR)

install:		$(ALL) Makefile install-incs
	ranlib $(T).a
	makenewer -r $(ALL) $(LIBDIR)

install-incs:
	makenewer -im -z -m o=h $(OBJA) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJB) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJC) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJD) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJE) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJF) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJG) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJH) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJI) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJJ) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJK) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJL) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJM) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJN) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJO) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJP) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJQ) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJR) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJS) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJT) -d $(INCDIR)

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJA)
	makesafe -v=3 -I $(INCDIR) $(OBJB)
	makesafe -v=3 -I $(INCDIR) $(OBJC)
	makesafe -v=3 -I $(INCDIR) $(OBJD)
	makesafe -v=3 -I $(INCDIR) $(OBJE)
	makesafe -v=3 -I $(INCDIR) $(OBJF)
	makesafe -v=3 -I $(INCDIR) $(OBJG)
	makesafe -v=3 -I $(INCDIR) $(OBJH)
	makesafe -v=3 -I $(INCDIR) $(OBJI)
	makesafe -v=3 -I $(INCDIR) $(OBJJ)
	makesafe -v=3 -I $(INCDIR) $(OBJK)
	makesafe -v=3 -I $(INCDIR) $(OBJL)
	makesafe -v=3 -I $(INCDIR) $(OBJM)
	makesafe -v=3 -I $(INCDIR) $(OBJN)
	makesafe -v=3 -I $(INCDIR) $(OBJO)
	makesafe -v=3 -I $(INCDIR) $(OBJP)
	makesafe -v=3 -I $(INCDIR) $(OBJQ)
	makesafe -v=3 -I $(INCDIR) $(OBJR)
	makesafe -v=3 -I $(INCDIR) $(OBJS)
	makesafe -v=3 -I $(INCDIR) $(OBJT)

again:
	rm -f $(ALL)

clean:
	rm -f $(ALL) *.o *.i *.ls *.ln *.lm

control:
	(uname -n ; date) > Control


obja.o:			$(OBJA)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJA)

objb.o:			$(OBJB)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJB)

objc.o:			$(OBJC)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJC)

objd.o:			$(OBJD)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJD)

obje.o:			$(OBJE)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJE)

objf.o:			$(OBJF)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJF)

objg.o:			$(OBJG)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJG)

objh.o:			$(OBJH)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJH)

obji.o:			$(OBJI)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJI)

objj.o:			$(OBJJ)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJJ)

objk.o:			$(OBJK)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJK)

objl.o:			$(OBJL)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJL)

objm.o:			$(OBJM)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJM)

objn.o:			$(OBJN)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJN)

objo.o:			$(OBJO)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJO)

objp.o:			$(OBJP)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJP)

objq.o:			$(OBJQ)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJQ)

objr.o:			$(OBJR)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJR)

objs.o:			$(OBJS)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJS)

objt.o:			$(OBJT)
	$(LD) -r -o $@ $(RLDFLAGS) $(OBJT)


uc_safesleep.o:		uc_safesleep.c

uc_getpuid.o:		uc_getpuid.c

uc_openinfo.o:		uc_openinfo.c opensysfs.h

uc_openuser.o:		uc_openuser.c opensysfs.h

uc_timeout.o:		uc_timeout.cc timeout.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) uc_timeout.cc


fsdir.o:		fsdir.c fsdir.h

# POSIX Thread Attribute
pta.o:			pta.c pta.h

# POSIX Thread Mutex Attribute
ptma.o:			ptma.c ptma.h

# POSIX Thread Condition Attribute
ptca.o:			ptca.c ptca.h

# POSIX Thread Mutex
ptm.o:			ptm.c ptm.h

# POSIX Thread Condition Variable
ptc.o:			ptc.c ptc.h

# POSIX Semaphore
psem.o:			psem.c psem.h

# POSIX Read-Write Lock (broken in dynamically loadable modeules)
ptrwlock.o:		ptrwlock.c ptrwlock.h

# POSIX Thread Read-Write Lock Attribute
ptrwa.o:		ptrwa.c ptrwa.h

# POSIX Message Queue
pmq.o:			pmq.c pmq.h

sighand.o:		sighand.c sighand.h

sigblock.o:		sigblock.c sigblock.h

sigign.o:		sigign.c sigign.h

userattr.o:		userattr.c userattr.h

ucsem.o:		ucsem.c ucsem.h

hostinfo.o:		hostinfo.c hostinfo.h

hostaddr.o:		hostaddr.c hostaddr.h

hostent.o:		hostent.c hostent.h

inetaddr.o:		inetaddr.c inetaddr.h

sockaddress.o:		sockaddress.c sockaddress.h

storeitem.o:		storeitem.c storeitem.h

storebuf.o:		storebuf.c storebuf.h

sbuf.o:			sbuf.c sbuf.h

sbuf_loadstrs.o:	sbuf_loadstrs.c sbuf.h

sbuf_addquoted.o:	sbuf_addquoted.c sbuf.h

sbuf_termconseq.o:	sbuf_termconseq.c sbuf.h

sbuf_blanks.o:		sbuf_blanks.c sbuf.h

buffer.o:		buffer.c buffer.h

ptca.o:			ptca.c ptca.h

field.o:		field.c field.h

char.o:			char.c char.h

filebuf.o:		filebuf.c filebuf.h

filebuf_writeblanks.o:	filebuf_writeblanks.c filebuf.h

filebuf_writefill.o:	filebuf_writefill.c filebuf.h

filebuf_writealign.o:	filebuf_writealign.c filebuf.h

filebuf_writezero.o:	filebuf_writezero.c filebuf.h

recarr.o:		recarr.c recarr.h

vecsorthand.o:		vecsorthand.c vecsorthand.h

vechand.o:		vechand.c vechand.h

vecstr.o:		vecstr.c vecstr.h

vecstr_adduniq.o:	vecstr_adduniq.c vecstr.h

vecstr_adduniqs.o:	vecstr_adduniqs.c vecstr.h

vecstr_addpath.o:	vecstr_addpath.c vecstr.h

vecstr_env.o:		vecstr_env.c vecstr.h

vecstr_envfile.o:	vecstr_envfile.c vecstr.h

vecstr_loadfile.o:	vecstr_loadfile.c vecstr.h

vecpstr.o:		vecpstr.c vecpstr.h

vecpstr_envadd.o:	vecpstr_envadd.c vecpstr.h

vecpstr_envfile.o:	vecpstr_envfile.c vecpstr.h

vecpstr_loadfile.o:	vecpstr_loadfile.c vecpstr.h

vecpstr_loadgrpusers.o:	vecpstr_loadgruses.c vecpstr.h

vecpstr_loadpjusers.o:	vecpstr_loadpjusers.c vecpstr.h

vecobj.o:		vecobj.c vecobj.h

vecelem.o:		vecelem.c vecelem.h

hdb.o:			hdb.c hdb.h

envlist.o:		envlist.c envlist.h

lookaside.o:		lookaside.c lookaside.h

strpack.o:		strpack.c strpack.h

ciq.o:			ciq.c ciq.h

pq.o:			pq.c pq.h

ids.o:			ids.c ids.h

linefold.o:		linefold.c linefold.h

envhelp.o:		envhelp.c envhelp.h

mkprogenv.o:		mkprogenv.c mkprogenv.h

spawner.o:		spawner.c spawner.h

nulstr.o:		nulstr.c nulstr.h

lockrw.o:		lockrw.c lockrw.h

upwcache.o:		upwcache.c upwcache.h recarr.h

filemap.o:		filemap.c filemap.h

uname.o:		uname.c uname.h

dirlist.o:		dirlist.c dirlist.h

dirseen.o:		dirseen.c dirseen.h

dirseen_not.o:		dirseen_not.c dirseen.h

varray.o:		varray.c varray.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -DVARRAY_LIBMEMALLOC=1 $<

randomvar.o:		randomvar.c randomvar.h

strmgr.o:		strmgr.c strmgr.h


sigevent.o:		sigevent.c sigevent.h

timeval.o:		timeval.c timeval.h

timespec.o:		timespec.c timespec.h

itimerspec.o:		itimerspec.c itimerspec.h

timeout.o:		timeout.c timeout.h

upt.o:			upt.c upt.h

base64.o:		base64.c base64.h

spawnproc.o:		spawnproc.c spawnproc.h

getxusername.o:		getxusername.c getxusername.h

getutmpent.o:		getutmpent.c getutmpent.h

getax.o:		getax.c getax.h

getpe.o:		getpe.c getpe.h

getse.o:		getse.c getse.h

getne.o:		getne.c getne.h

gethe.o:		gethe.c gethe.h

passwdent.o:		passwdent.c passwdent.h

shadowent.o:		shadowent.c shadowent.h

groupent.o:		groupent.c groupent.h

projectent.o:		projectent.c projectent.h

userattrent.o:		userattrent.c userattrent.h

utmpaccent.o:		utmpaccent.c utmpaccent.h

findbit.o:		findbit.c findbit.h

snflags.o:		snflags.c snflags.h

mkpath.o:		mkpath.c

sncpy.o:		sncpy.c


dialopts.o:		dialopts.c dialopts.h

dialprog.o:		dialprog.c dialopts.h

dialfinger.o:		dialfinger.c dialopts.h

dialudp.o:		dialudp.c dialopts.h

dialtcp.o:		dialtcp.c dialopts.h

dialtcpmux.o:		dialtcpmux.c dialopts.h

dialtcpnls.o:		dialtcpnls.c nlsdialassist.h dialopts.h

nlsdialassist.o:	nlsdialassist.c nlsdialassist.h dialopts.h

opensysfs.o:		opensysfs.c opensysfs.h

quoteshellarg.o:	quoteshellarg.c

mkquoted.o:		mkquoted.c

snsd.o:			snsd.c

snsds.o:		snsds.c

snfsflags.o:		snfsflags.c snflags.h

snopenflags.o:		snopenflags.c snflags.h

snpollflags.o:		snpollflags.c snflags.h

sntmtime.o:		sntmtime.c tmtime.h

intsat.o:		intsat.c intsat.h

tmtime.o:		tmtime.c tmtime.h

ctdecp.o:		ctdecp.c ctdecp.h

getnodedomain.o:	getnodedomain.c

inetpton.o:		inetpton.c

matstr.o:		matstr.c

strshrink.o:		strshrink.c

stwhitedot.o:		sfwhitedot.c

strnchr.o:		strnchr.c

mkvarpath.o:		mkvarpath.c

mallocstuff.o:		mallocstuff.c

ndigits.o:		ndigits.c


# special built-ins

uinfo.o:		uinfo.c uinfo.h

umask.o:		umask.c umask.h

ugetpw.o:		ugetpw.c ugetpw.h upwcache.h recarr.h

utmpacc.o:		utmpacc.c utmpacc.h recarr.h

uproguser.o:		uproguser.c uproguser.h

uprogdata.o:		uprogdata.c uprogdata.h

ugetpid.o:		ugetpid.c ugetpid.h

uclustername.o:		uclustername.c uclustername.h

mkcdpath.o:		mkcdpath.cc
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) mkcdpath.cc

m.o:			m.cc
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) m.cc


# testing

X01= testugetpw

X01OBJ= $(X01).o ugetpw.o upwcache.o recarr.o

$(X01).o:		$(X01).c ugetpw.h upwcache.h recarr.h

$(X01).x:		$(X01OBJ)
	$(CC) -o $@ $(XLDFLAGS) $(X01OBJ) $(XLIBINFO)


X02= testutmpacc

X02OBJ= $(X02).o utmpacc.o utmpaccent.o filemap.o recarr.o

$(X02).o:		$(X02).c utmpacc.h utmpaccent.h filemap.h recarr.h

$(X02).x:		$(X02OBJ)
	$(CC) -o $@ $(XLDFLAGS) $(X02OBJ) $(XLIBINFO)


#XDIAL0= dialticotsordnls.o dialticotsordmux.o dialticotsord.o 
XDIAL1= filebuf.o opensysfs.o
XDIAL2= nlsdialassist.o strnwcpy.o
XDIAL3= snopenflags.o
XDIAL4= uc_reade.o uc_openuser.o uc_opensys.o uc_opendev.o

XDIALERS= $(XDIAL0) $(XDIAL1) $(XDIAL2) $(XDIAL3) $(XDIAL4)


X03= testucopen

X03OBJ00= $(X03).o mkprogenv.o spawnproc.o
X03OBJ01= uc_openinfo.o uc_openproto.o uc_openprog.o 
X03OBJ02= uc_opendialer.o uc_openfsvc.o uc_openusvc.o uc_execve.o
X03OBJ03= $(XDIALERS)

X03OBJ04= attachso.o opensvc_hotd.o
X03OBJ05= mkexpandpath.o mkuserpath.o mkvarpath.o mkcdpath.o
X03OBJ06=
X03OBJ07= debug.o

X03OBJa= $(X03OBJ00) $(X03OBJ01) $(X03OBJ02) $(X03OBJ03) 
X03OBJb= $(X03OBJ04) $(X03OBJ05) $(X03OBJ06) $(X03OBJ07)
X03OBJ= $(X03OBJa) $(X03OBJb)

$(X03).o:		$(X03).c vsystem.h nlsdialassist.h

$(X03).x:		$(X03OBJ)
	$(CC) -o $@ $(XLDFLAGS) $(X03OBJ) $(XLIBINFO)


X04= testsafesleep

X04OBJ00= $(X04).o 
X04OBJ01=
X04OBJ02=
X04OBJ03=
X04OBJ04=
X04OBJ05= debug.o

X04OBJ= $(X04OBJ00) $(X04OBJ01) $(X04OBJ02) $(X04OBJ03) $(X04OBJ04) $(X04OBJ05)

$(X04).o:		$(X04).c vsystem.h

$(X04).x:		$(X04OBJ)
	$(CC) -o $@ $(XLDFLAGS) $(X04OBJ) $(XLIBINFO)


X05= testspawnproc

X05OBJ00= $(X05).o 
X05OBJ01= spawnproc.o
X05OBJ02=
X05OBJ03=
X05OBJ04=
X05OBJ05= debug.o

X05OBJ= $(X05OBJ00) $(X05OBJ01) $(X05OBJ02) $(X05OBJ03) $(X05OBJ04) $(X05OBJ05)

$(X05).o:		$(X05).c vsystem.h

$(X05).x:		$(X05OBJ)
	$(CC) -o $@ $(XLDFLAGS) $(X05OBJ) $(XLIBINFO)


X06= testugetpid

X06OBJ00= $(X06).o 
X06OBJ01= ugetpid.o
X06OBJ02= uc_atfork.o
X06OBJ03=
X06OBJ04=
X06OBJ05= debug.o
X06OBJ06=
X06OBJ07=

X06OBJa= $(X06OBJ00) $(X06OBJ01) $(X06OBJ02) $(X06OBJ03) 
X06OBJb= $(X06OBJ04) $(X06OBJ05) $(X06OBJ06) $(X06OBJ07)

X06OBJ= $(X06OBJa) $(X06OBJb)

$(X06).o:		$(X06).c vsystem.h

$(X06).x:		$(X06OBJ)
	$(CC) -o $@ $(XLDFLAGS) $(X06OBJ) $(XLIBINFO)


X07= testnaked

X07OBJ00= $(X07).o 
X07OBJ01=
X07OBJ02=
X07OBJ03=
X07OBJ04=
X07OBJ05=
X07OBJ06=
X07OBJ07=

X07OBJa= $(X07OBJ00) $(X07OBJ01) $(X07OBJ02) $(X07OBJ03) 
X07OBJb= $(X07OBJ04) $(X07OBJ05) $(X07OBJ06) $(X07OBJ07)

X07OBJ= $(X07OBJa) $(X07OBJb)

$(X07).o:		$(X07).c vsystem.h

$(X07).x:		$(X07OBJ)
	$(CC) -o $@ $(XLDFLAGS) $(X07OBJ) $(XLIBINFO)


