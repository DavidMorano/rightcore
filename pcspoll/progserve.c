/* progserve */

/* handle a connect request for a service */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time print-outs */
#define	CF_DEBUG	0		/* run-time print-outs */
#define	CF_CHECKACCESS	1		/* check access permissions */
#define	CF_ALLDEF	0		/* always allow default permissions */


/* revision history:

	= 2008-07-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine processes a new connection that just came in.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<stropts.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<pwd.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<field.h>
#include	<vecstr.h>
#include	<hostent.h>
#include	<sockaddress.h>
#include	<inetaddr.h>
#include	<pwfile.h>
#include	<svcfile.h>
#include	<connection.h>
#include	<storebuf.h>
#include	<char.h>
#include	<expcook.h>
#include	<getax.h>
#include	<ids.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"proglog.h"
#include	"clientinfo.h"
#include	"builtin.h"
#include	"standing.h"
#include	"svckey.h"
#include	"procse.h"


/* local defines */

#ifndef	VARNLSADDR
#define	VARNLSADDR	"NLSADDR"
#endif

#ifndef	VARHOME
#define	VARHOME		"HOME"
#endif

#ifndef	VARTERM
#define	VARTERM		"TERM"
#endif

#ifndef	PEERNAMELEN
#ifdef	CONNECTION_PEERNAMELEN
#define	PEERNAMELEN	CONNECTION_PEERNAMELEN
#else
#define	PEERNAMELEN	MAX(MAXHOSTNAMELEN,MAXPATHLEN)
#endif
#endif

#ifndef	INETXADDRLEN
#define	INETXADDRLEN	sizeof(struct in_addr)
#endif

#ifndef	INET4_ADDRSTRLEN
#define	INET4_ADDRSTRLEN	16
#endif

#ifndef	INET6_ADDRSTRLEN
#define	INET6_ADDRSTRLEN	46	/* Solaris® says this is 46! */
#endif

#ifndef	INETX_ADDRSTRLEN
#define	INETX_ADDRSTRLEN	MAX(INET4_ADDRSTRLEN,INET6_ADDRSTRLEN)
#endif

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
#endif

#ifndef	STEBUFLEN
#define	STEBUFLEN	MAXNAMELEN
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	LOGBUFLEN	(LOGNAMELEN + 20)

#define	O_FLAGS		(O_CREAT | O_RDWR | O_TRUNC)

#define	SHLIBENTRY	"main"


/* external subroutines */

extern int	sninetaddr(char *,int,int,const void *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	mkpath3w(char *,const char *,const char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkbasename(char *,const char *,int) ;
extern int	mkshlibname(char *,cchar *,int) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	sfbaselib(cchar *,int,cchar **) ;
extern int	nextfield(cchar *,int,cchar **) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	nlspeername(const char *,const char *,char *) ;
extern int	getprogpath(IDS *,vecstr *,char *,const char *,int) ;
extern int	netgroupcheck(const char *,vecstr *,vecstr *) ;
extern int	mkquoted(char *,int,cchar *,int) ;
extern int	optbool(const char *,int) ;
extern int	isasocket(int) ;
extern int	uc_waitwritable(int,int) ;

extern int	progsrvargs(PROGINFO *,vecstr *,const char *) ;
extern int	progshlib(PROGINFO *,cchar *,cchar *,vecstr *,cchar *,int) ;
extern int	progexec(PROGINFO *,cchar *,cchar *,vecstr *) ;

#if	defined(P_FINGERS) && (P_FINGERS > 0)
extern int	proguseracctmat(PROGINFO *,struct passwd *,char *,int,cchar *) ;
extern int	proguseracctexec(PROGINFO *,CLIENTINFO *,struct passwd *) ;
#endif /* P_FINGERS */

extern int	xfile(IDS *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strdcpy1(char *,int,cchar *,int) ;
extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*strbasename(char *) ;


/* external variables */


/* forward references */

static int	procserver(PROGINFO *,CLIENTINFO *,
			SVCFILE_ENT *,cchar **) ;
static int	procserverpass(PROGINFO *,CLIENTINFO *,
			PROCSE *, VECSTR *,cchar *) ;
static int	procserverlib(PROGINFO *,CLIENTINFO *,
			PROCSE *, VECSTR *,cchar *) ;
static int	procserverexec(PROGINFO *,CLIENTINFO *,
			PROCSE *, VECSTR *,cchar *) ;
static int	procfindprog(PROGINFO *,vecstr *,cchar **,
			char *,cchar *,int) ;

#if	CF_CHECKACCESS
static int	procaccperm(PROGINFO *,CLIENTINFO *,PROCSE *) ;
#endif

#ifdef	COMMENT
static int	procusersetup(PROGINFO *,CLIENTINFO *,SVCFILE_ENT *) ;
#endif

static int	loadcooks(PROGINFO *,CLIENTINFO *,cchar **) ;
static int	loadpeernames(PROGINFO *,CLIENTINFO *,vecstr *) ;
static int	loadaccgroups(PROGINFO *,vecstr *,cchar *,int) ;


/* local variables */

static const uchar	gterms[] = {
	0x00, 0x02, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static cchar	*prbins[] = {
	"bin",
	"sbin",
	NULL
} ;

static cchar	*prlibs[] = {
	"lib",
	NULL
} ;


/* exported subroutines */


int progserve(pip,sop,bop,cip,nelp,svcspec,sav)
PROGINFO	*pip ;
STANDING	*sop ;
BUILTIN		*bop ;
CLIENTINFO	*cip ;
vecstr		*nelp ;
const char	svcspec[] ;
const char	*sav[] ;
{
	SVCFILE_ENT	ste ;
	int		rs = SR_OK ;
	int		rs1 = 0 ;
	int		si ;
	int		svcspeclen ;
	int		f_served = FALSE ;
	const char	*svcspecp ;
	const char	*tp, *cp ;
	char		svcbuf[SVCSPECLEN + 1] ;
	char		stebuf[STEBUFLEN + 1] ;
	char		timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUGS
	if (DEBUGLEVEL(5))
	    debugprintf("progserve: ent peername=%s\n",
	        cip->peername) ;
#endif

/* has a service been passed to us (overrides normal service) */

	svcspecp = svcspec ;
	if ((pip->svcpass != NULL) && (pip->svcpass[0] != '\0'))
	    svcspecp = pip->svcpass ;

	svcspeclen = strlen(svcspecp) ;

/* do we have a subservice? */

	cip->service = svcspecp ;
	cip->subservice = svcspecp + svcspeclen ; /* setup for none */
	if ((tp = strnchr(svcspecp,svcspeclen,'+')) != NULL) {

	    cip->subservice = (tp + 1) ;
	    svcspeclen = (tp - svcspecp) ;
	    strwcpy(svcbuf,svcspecp,svcspeclen) ;
	    cip->service = svcbuf ;

	} /* end if (subservice) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("progserve: service=%s\n",cip->service) ;
	    debugprintf("progserve: subservice=%s\n",cip->subservice) ;
	}
#endif

/* do we need a default service? */

	if (cip->service[0] == '\0') {
	    if (pip->f.defsvc) {
	        cp = (pip->defsvc[0] != '\0') ? pip->defsvc : DEFSVC ;
	        cip->service = cp ;
	    }
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: %s svc=%s\n",pip->progname,
	        timestr_logz(pip->daytime,timebuf), cip->service) ;
	    if (cip->subservice != NULL)
	        bprintf(pip->efp,"%s: subsvc=%s\n", pip->progname,
	            cip->subservice) ;
	}

	if (pip->open.logprog) {
	    proglog_printf(pip,"%s svc=%s\n",
	        timestr_logz(pip->daytime,timebuf), cip->service) ;
	    if (cip->subservice != NULL)
	        proglog_printf(pip,"subsvc=%s\n", cip->subservice) ;
	}

	if (cip->service[0] == '\0') {
	    if (pip->open.logprog)
	        proglog_printf(pip,"no service\n") ;
	    goto badnosvc ;
	}

/* some environment management */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progserve: environment management\n") ;
#endif

	if (rs >= 0)
	    rs = vecstr_envadd(&pip->exports,VARHOME,pip->homedname,-1) ;

	if (rs >= 0) {
	    const char	*varterm = VARTERM ;

	    rs1 = vecstr_search(&pip->exports,varterm,vstrkeycmp,&cp) ;
	    if ((rs1 == SR_NOTFOUND) && (nelp != NULL)) {
	        rs1 = vecstr_search(nelp,varterm,vstrkeycmp,&cp) ;
	        if (rs1 >= 0)
	            rs = vecstr_add(&pip->exports,cp,-1) ;
	    }

	} /* end if (adding VARTERM to the child environment) */

	if (rs < 0)
	    goto ret1 ;

	if (pip->open.logprog)
	    proglog_flush(pip) ;

/* search for the service */

	if ((rs >= 0) && (! f_served) && pip->open.svcfname) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progserve: service-table lookup\n") ;
#endif

	    rs1 = svcfile_fetch(&pip->stab,cip->service,NULL,
	        &ste,stebuf,STEBUFLEN) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progserve: svcfile_fetch() rs=%d\n",rs1) ;
#endif

	    if (rs1 >= 0) {
	        rs = procserver(pip,cip,&ste,sav) ;
	        f_served = (rs > 0) ;
	    } /* end if (matched) */

	} /* end if (svcfile) */

/* username service (when using FINGER)? */

#if	defined(P_FINGERS) && (P_FINGERS > 0)
	if ((rs >= 0) && (! f_served) && pip->f.loginsvc) {
	    if (pip->f.useracct && (strcmp(cip->service,"help") != 0)) {
	        struct passwd	pw ;
	        const int	pwlen = getbufsize(getbufsize_pw) ;
	        char		*pwbuf ;
	        if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	            cchar	*svc = cip->service ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("progserve: user? svc=%s\n",svc) ;
#endif

	            if ((rs1 = proguseracctmat(pip,&pw,pwbuf,pwlen,svc)) >= 0) {

	                rs = proguseracctexec(pip,cip,&pw) ;
	                f_served = (rs > 0) ;

	            } /* end if (matched) */

	            uc_free(pwbuf) ;
	        } /* end if (m-a) */
	    } /* end if */
	} /* end if (loginsvc) */
#endif /* P_FINGERS */

/* built-in service (when using FINGER)? */

	if ((rs >= 0) && (! f_served)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progserve: builtin?\n") ;
#endif

	    if ((si = builtin_match(bop,cip->service)) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("progserve: builtin found\n") ;
#endif

	        f_served = TRUE ;
	        if (pip->open.logprog) {
	            proglog_printf(pip,"server=INTERNAL(%u)",si) ;
	            proglog_flush(pip) ;
	        }

#if	defined(P_TCPMUXD) && (P_TCPMUXD == 1)
	        rs = u_write(cip->fd_output,"+\r\n",3) ;
#endif

	        if (rs >= 0) {
	            rs = builtin_execute(bop,sop,cip,si,sav) ;
	            f_served = (rs > 0) ;
	        }

	    } /* end if (matched) */

	} /* end if (builtin) */

/* done */
badnosvc:
	if ((rs >= 0) && (! f_served)) {

	    if (pip->open.logprog)
	        proglog_printf(pip,"service not found\n") ;

#if	defined(P_FINGERS) && (P_FINGERS > 0)
	    if (pip->f.defnoserver) {
	        cp = "no user\n" ;
	        rs = uc_writen(cip->fd_output,cp,strlen(cp)) ;
	    }
#endif /* defined(P_FINGERS) */

#if	defined(P_TCPMUXD) && (P_TCPMUXD > 0)
	    cp = "-\r\n" ;
	    rs = uc_writen(cip->fd_output,cp,strlen(cp)) ;
#endif /* defined (P_FINGERS) */

	} /* end if (no service match) */

/* we are out of here */
ret2:
	if (cip->fd_input >= 3) {
	    u_close(cip->fd_input) ;
	    cip->fd_input = -1 ;
	}

	if (cip->fd_output >= 3) {
	    u_close(cip->fd_output) ;
	    cip->fd_output = -1 ;
	}

ret1:
bad1:
	if (pip->open.logprog)
	    proglog_flush(pip) ;

bad0:
ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progserve: ret rs=%d f_served=%u\n",
	        rs,f_served) ;
#endif

	return (rs >= 0) ? f_served : rs ;
}
/* end subroutine (progserve) */


/* local subroutines */


static int procserver(pip,cip,step,sav)
PROGINFO	*pip ;
CLIENTINFO	*cip ;
SVCFILE_ENT	*step ;
const char	*sav[] ;
{
	PROCSE		se ;
	PROCSE_ARGS	sea ;
	SVCKEY		sekeys ;
	vecstr		alist ;
	int		rs = SR_OK ;
	int		rs1 = 0 ;
	int		opts ;
#if	defined(P_TCPMUXD) && (P_TCPMUXD == 1)
	int		cs ;
#endif
	int		f_served = FALSE ;
	int		f_failcont = TRUE ;
	int		f_cont = TRUE ;
	const char	*argz ;

	if (cip == NULL) return SR_FAULT ;
	if (step == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procserver: service=%s\n",cip->service) ;
	    debugprintf("procserver: subservice=%s\n",cip->subservice) ;
	}
#endif

	rs = loadcooks(pip,cip,sav) ;
	if (rs < 0)
	    goto ret0 ;

/* load up service values for further processing */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procserver: svkey_load()\n") ;
#endif

	svckey_load(&sekeys,step) ;	/* convert to our form */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procserver: sekeys.pass=>%s<\n",sekeys.pass) ;
	    debugprintf("procserver: sekeys.so=>%s<\n",sekeys.so) ;
	    debugprintf("procserver: sekeys.p=>%s<\n",sekeys.p) ;
	    debugprintf("procserver: sekeys.a=>%s<\n",sekeys.a) ;
	}
#endif

	memset(&sea,0,sizeof(PROCSE_ARGS)) ;
	sea.failcont = sekeys.failcont ;
	sea.passfile = sekeys.pass ;
	sea.sharedobj = sekeys.so ;
	sea.program = sekeys.p ;
	sea.srvargs = sekeys.a ;
	sea.username = sekeys.u ;
	sea.groupname = sekeys.g ;
	sea.options = sekeys.opts ;
	sea.access = sekeys.acc ;

	rs = procse_start(&se,pip->envv,&pip->subs,&sea) ;
	if (rs < 0)
	    goto badsestart ;

	rs = procse_process(&se,&pip->cooks) ;
	if (rs < 0)
	    goto badseproc ;

/* check access permissions */

#if	CF_CHECKACCESS
	if (cip->salen > 0) {
	    rs = procaccperm(pip,cip,&se) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("procserver: procaccperm() rs=%d\n",rs) ;
#endif

	    if (rs < 0) goto badprocacc ;
	    if (rs == 0) goto badnoperm ;
	}
#endif /* CF_CHECKACCESS */

/* OK, find the server program file path and create its program arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procserver: se passfile=>%s<\n",se.a.passfile) ;
	    debugprintf("procserver: se sharedobj=>%s<\n",se.a.sharedobj) ;
	    debugprintf("procserver: se program=>%s<\n",se.a.program) ;
	    debugprintf("procserver: se srvargs=>%s<\n",se.a.srvargs) ;
	}
#endif

/* break the server arguments up so that we can find the first one */

	opts = VECSTR_OSTATIONARY ;
	rs = vecstr_start(&alist,10,opts) ;
	if (rs < 0)
	    goto badargstart ;

	argz = NULL ;
	if (se.a.srvargs != NULL) {

	    rs = progsrvargs(pip,&alist,se.a.srvargs) ;

	    if (rs > 0)
	        vecstr_get(&alist,0,&argz) ;

	} /* end if (service-arguments) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procserver: progsrvargs() rs=%d\n",rs) ;
	    debugprintf("procserver: argz=%s\n",argz) ;
	}
#endif

	if (rs < 0)
	    goto badnoprog ;

/* get fail-cont indicator */

	if (se.a.failcont != NULL) {
	    cchar	*tp ;
	    f_failcont = TRUE ;
	    if ((tp = strchr(se.a.failcont,'=')) != NULL) {
	        rs1 = optbool((tp+1),-1) ;
	        if (rs1 >= 0) f_failcont = (rs1 > 0) ;
	    }
	}

/* decide on how to execute! */

	if (f_cont && (! f_served)) {
	    rs1 = procserverpass(pip,cip,&se,&alist,argz) ;
	    f_served = (rs1 > 0) ;
	    if ((rs >= 0) || (rs1 >= 0)) rs = rs1 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("procserver: procserverpass() rs=%d\n",rs) ;
#endif

	    if ((rs1 < 0) && f_failcont) {
	        f_cont = FALSE ;
	        f_cont = f_cont || (rs1 == SR_NOENT) ;
	        f_cont = f_cont || (rs1 == SR_INTR) || (rs1 == SR_NXIO) ;
	        if (f_cont && pip->open.logprog)
	            proglog_printf(pip,"failcont from PASS") ;
	    }
	}

	if (f_cont && (! f_served)) {
	    rs1 = procserverlib(pip,cip,&se,&alist,argz) ;
	    f_served = (rs1 > 0) ;
	    if ((rs >= 0) || (rs1 >= 0)) rs = rs1 ;
	    if ((rs1 < 0) && f_failcont) {
	        f_cont = FALSE ;
	        f_cont = f_cont || (rs1 == SR_NOENT) ;
	        f_cont = f_cont || (rs1 == SR_INTR) || (rs1 == SR_NXIO) ;
	        f_cont = f_cont || (rs1 == SR_LIBACC) || (rs1 == SR_NOEXEC) ;
	        if (f_cont && pip->open.logprog)
	            proglog_printf(pip,"failcont from SOLIB") ;
	    }
	}

	if (f_cont && (! f_served)) {
	    rs1 = procserverexec(pip,cip,&se,&alist,argz) ;
	    f_served = (rs1 > 0) ;
	    if ((rs >= 0) || (rs1 >= 0)) rs = rs1 ;
	}

#if	defined(P_TCPMUXD) && (P_TCPMUXD == 1)
	if (rs < 0)
	    rs = u_write(cip->fd_output,"-\r\n",3) ;
#endif

	if (! f_served) {
	    const char	*msg = "no server configured" ;
	    if (pip->open.logprog)
	        proglog_printf(pip,"%s (%d)",msg,rs) ;
	    bprintf(pip->efp,"%s: %s (%d)\n",pip->progname,msg,rs) ;
	}

badnoprog:
	rs1 = vecstr_finish(&alist) ;
	if (rs >= 0) rs = rs1 ;

badseproc:
badargstart:
	rs1 = procse_finish(&se) ;
	if (rs >= 0) rs = rs1 ;

badsestart:
	if (pip->open.logprog) {
	    proglog_flush(pip) ;
	}

badnoperm:
badprocacc:
ret0:
	return (rs >= 0) ? f_served : rs ;
}
/* end subroutine (procserver) */


static int procserverpass(pip,cip,sep,alp,argz)
PROGINFO	*pip ;
CLIENTINFO	*cip ;
PROCSE		*sep ;
VECSTR		*alp ;
const char	*argz ;
{
	struct ustat	sb ;
	mode_t		operms ;
	int		rs = SR_NOENT ;
	int		pfd ;
	int		oflags ;
	int		to = TO_SENDFD ;
	int		f_served = FALSE ;
	const char	*passfname = sep->a.passfile ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progserve/procserverpass: passfname=%s\n",
	        passfname) ;
#endif

	if (passfname == NULL)
	    goto ret0 ;

	if (passfname[0] == '\0')
	    goto ret0 ;

	oflags = (O_WRONLY | O_NDELAY) ;
	operms = 0666 ;
	rs = u_open(passfname,oflags,operms) ;
	pfd = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progserve/procserverpass: u_open() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	rs = u_fstat(pfd,&sb) ;
	if (rs < 0)
	    goto ret1 ;

	if ((rs >= 0) && (! S_ISFIFO(sb.st_mode)))
	    rs = SR_NOENT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progserve/procserverpass: u_fstat() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    rs = uc_waitwritable(pfd,to) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progserve/procserverpass: "
	            "uc_waitwritable() rs=%d\n",
	            rs) ;
#endif

	    if (rs >= 0)
	        rs = u_ioctl(pfd,I_SENDFD,cip->fd_input) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progserve/procserverpass: u_ioctl() rs=%d\n",rs) ;
#endif

	}

	if (rs >= 0) f_served = TRUE ;

ret1:
	if (pfd >= 0) {
	    u_close(pfd) ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progserve/procserverpass: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_served : rs ;
}
/* end subroutine (procserverpass) */


static int procserverlib(pip,cip,sep,alp,argz)
PROGINFO	*pip ;
CLIENTINFO	*cip ;
PROCSE		*sep ;
VECSTR		*alp ;
const char	*argz ;
{
	const int	nlen = MAXNAMELEN ;
	int		rs = SR_OK ;
	int		pnl ;
	int		enl ;
	const char	*program = sep->a.sharedobj ;
	const char	*pnp ;
	const char	*tp ;
	const char	*enp ;		/* shlib entry-point */
	const char	*cp ;
	char		progfname[MAXPATHLEN + 1] ;
	char		shlibname[MAXNAMELEN + 1] ;
	char		argzbuf[MAXNAMELEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procserver/procserverlib: ent argz=%s\n",argz) ;
#endif

/* get a program if we do not have one already */

	if ((program == NULL) || (program[0] == '\0'))
	    goto ret0 ;

/* parse out the components of the 'shlib' specification */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procserver/procserverlib: program=>%s<\n",
	        program) ;
#endif

	pnl = sfshrink(program,-1,&pnp) ;

	enp = NULL ;
	enl = 0 ;
	if ((tp = strnchr(pnp,pnl,':')) == NULL) {
	    tp = strnpbrk(pnp,pnl," \t") ;
	}

	if (tp != NULL) {
	    enl = sfshrink((tp+1),((pnp+pnl)-(tp+1)),&enp) ;
	    pnl = (tp-pnp) ;
	    while ((pnl > 0) && CHAR_ISWHITE(pnp[pnl-1])) pnl -= 1 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procserver/procserverlib: shlib=%t\n",pnp,pnl) ;
	    debugprintf("procserver/procserverlib: entry=%t\n",enp,enl) ;
	}
#endif

	if (pnl == 0) {
	    rs = SR_NOENT ;
	    goto badnoprog ;
	}

	if ((enp == NULL) || (enl == 0)) {
	    if ((argz != NULL) && (argz[0] != '\0')) {
	        enp = argz ;
	        enl = strlen(argz) ;
	    } else {
	        enl = sfbaselib(pnp,pnl,&enp) ;
	    }
	    if ((enp == NULL) || (enl == 0)) {
	        pnp = SHLIBENTRY ;
	        pnl = -1 ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procserver/procserverlib: 2 entry=%t\n",enp,enl) ;
	}
#endif

	if (pip->open.logprog) {
	    proglog_printf(pip,"server=%t",enp,enl) ;
	}
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: server=%t\n",pip->progname,enp,enl) ;
	}

/* can we execute this service daemon? */

	if (strnchr(pnp,pnl,'.') == NULL) {
	    rs = mkshlibname(shlibname,pnp,pnl) ;
	    pnl = rs ;
	    pnp = shlibname ;
	}

	if (rs >= 0) {
	    rs = procfindprog(pip,&pip->pathlib,prlibs,progfname,pnp,pnl) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procserver/procserverlib: "
	        "procfindprog() rs=%d\n",rs) ;
	    debugprintf("procserver/procserverlib: progfname=%s\n",
	        progfname) ;
	}
#endif

	if (rs == 0) {
	    if (pnp[0] != '/') {
	        if ((rs = mkpath2w(progfname,pip->pwd,pnp,pnl)) >= 0) {
	            rs = xfile(&pip->id,progfname) ;
		}
	    } else {
	        rs = mkpath1w(progfname,pnp,pnl) ;
	    }
	}

	if (rs < 0) {
	    cp = "SHLIB server not found" ;
	    if (pip->open.logprog) {
	        proglog_printf(pip,cp) ;
	    }
	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: %s\n",pip->progname,cp) ;
	    }
	    goto badnoprog ;
	} /* end if (could not find program) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procserver: past X check\n") ;
	    debugprintf("procserver: shlib=%t\n",pnp,pnl) ;
	}
#endif

/* get a basename for ARG0 */

	if (argz != NULL) {
	    if ((argz[0] == '+') && (argz[1] == '\0')) {
	        argz = argzbuf ;
		if (enl > nlen) enl = nlen ; /* prevent buffer overflow */
	        strwcpy(argzbuf,enp,enl) ;
	        if ((rs = vecstr_del(alp,0)) >= 0) {
	            rs = vecstr_insert(alp,0,argz,-1) ;
		}
	    }
	} else {
	    argz = argzbuf ;
	    if (enl > nlen) enl = nlen ; /* prevent buffer overflow */
	    strwcpy(argzbuf,enp,enl) ;
	    if (vecstr_count(alp) < 1) {
	        rs = vecstr_add(alp,argz,-1) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procserver/procserverlib: final argz=%s\n",argz) ;
#endif

	if (pip->open.logprog) {
	    proglog_printf(pip,"shlib=%s",progfname) ;
	    proglog_printf(pip,"argz=%s",argz) ;
	}

	if (rs < 0)
	    goto badadd1 ;

#ifdef	COMMENT
	rs = procusersetup(pip,cip,step) ;
#endif

/* we are good! */

#if	defined(P_TCPMUXD) && (P_TCPMUXD == 1)
	rs = u_write(cip->fd_output,"+\r\n",3) ;
#else
	rs = SR_OK ;
#endif

	if (rs < 0)
	    goto badadd2 ;

/* do it! */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procserver/procserverlib: progfname=%s\n",
	        progfname) ;
#endif

	pip->daytime = time(NULL) ;

	if (pip->open.logprog) {
	    proglog_end(pip) ;
	}

	{
	    PROGINFO_IPC	*ipp = &pip->ipc ;
	    if (ipp->fd_req >= 0) {
	        u_close(ipp->fd_req) ;
	        ipp->fd_req = -1 ;
	    }
	    if (cip->fd_input >= 3) {
	        u_close(cip->fd_input) ;
	        cip->fd_input = -1 ;
	    }
	    if (cip->fd_output >= 3) {
	        u_close(cip->fd_output) ;
	        cip->fd_output = -1 ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procserver/procserverlib: progshlib()\n") ;
#endif

	rs = progshlib(pip,progfname,argz,alp,enp,enl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procserver: progshlib() rs=%d\n",rs) ;
#endif

	if (pip->f.logprog) {
	    proglog_begin(pip,pip->uip) ;
	    if (pip->open.logprog && (rs < 0)) {
	        proglog_printf(pip,
	            "SHLIB could not execute server program (%d)\n",rs) ;
	        proglog_printf(pip,
	            "shlib=%s\n",progfname) ;
	    }
	}

/* we are out of here */
done:
badadd2:
badadd1:
badnoprog:
ret0:
	return rs ;
}
/* end subroutine (procserverlib) */


static int procserverexec(pip,cip,sep,alp,argz)
PROGINFO	*pip ;
CLIENTINFO	*cip ;
PROCSE		*sep ;
VECSTR		*alp ;
const char	*argz ;
{
	int		rs = SR_OK ;
	int		pnl ;
	int		cl ;
	const char	*program ;
	const char	*pnp ;
	const char	*ccp ;
	const char	*cp ;
	char		progfname[MAXPATHLEN + 1] ;
	char		argzbuf[MAXNAMELEN + 1] ;

/* get a program if we do not have one already */

	program = sep->a.program ;
	if ((program == NULL) && (argz != NULL)) {
	    program = argz ;
	    if ((argz[0] == '+') || (argz[0] == '-'))
	        program += 1 ;
	}

/* check the program (a little bit) */

	if ((program == NULL) || (program[0] == '\0'))
	    goto ret0 ;

	if (rs >= 0) {
	    ccp = program ;
	    if ((ccp[0] == '+') || (ccp[0] == '-')) {
	        rs = SR_LIBSCN ;
	        cp = "EXEC bad server configured" ;
	    }
	}

	if (rs < 0) {
	    if (pip->open.logprog) proglog_printf(pip,cp) ;
	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: %s\n",pip->progname,cp) ;
	    goto badnoprog ;
	}

/* log some information on the server program (name) */

	cl = sfbasename(program,-1,&cp) ;

	while ((cl > 0) && (cp[cl-1] == '/')) cl -= 1 ;

	if (pip->open.logprog)
	    proglog_printf(pip,"server=%t",cp,cl) ;
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: server=%t",pip->progname,cp,cl) ;

/* can we execute this service daemon? */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procserver: X check pr=%s prog=%s\n",
	        pip->pr,program) ;
#endif

	pnp = program ;
	pnl = strlen(program) ;
	while ((pnl > 0) && (pnp[pnl-1] == '/')) pnl -= 1 ;

	rs = procfindprog(pip,&pip->pathexec,prbins,progfname,pnp,pnl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procserver: rs=%d progfname=%s\n",
	        rs,progfname) ;
#endif

	if (rs == 0) {
	    if (program[0] != '/') {
	        mkpath2w(progfname,pip->pwd,pnp,pnl) ;
	        rs = xfile(&pip->id,progfname) ;
	    } else {
	        rs = mkpath1w(progfname,pnp,pnl) ;
	    }
	}

	if (rs < 0) {
	    cp = "EXEC server not found" ;
	    if (pip->open.logprog) {
	        proglog_printf(pip,cp) ;
	    }
	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: %s\n",pip->progname,cp) ;
	    }
	    goto badnoprog ;
	} /* end if (could not find program) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procserver: past X check\n") ;
	    debugprintf("procserver: program=%t\n",pnp,pnl) ;
	}
#endif

/* get a basename for ARG0 */

	if (argz != NULL) {
	    if ((argz[0] == '+') && (argz[1] == '\0')) {
	        argz = argzbuf ;
	        cl = mkbasename(argzbuf,pnp,pnl) ;
	        if ((rs = vecstr_del(alp,0)) >= 0) {
	            rs = vecstr_insert(alp,0,argz,cl) ;
		}
	    }
	} else {
	    argz = argzbuf ;
	    cl = mkbasename(argzbuf,pnp,pnl) ;
	    if (vecstr_count(alp) < 1) {
	        rs = vecstr_add(alp,argz,cl) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procserver: final argz=%s\n",argz) ;
#endif

	if (pip->open.logprog) {
	    proglog_printf(pip,"pf=%s",progfname) ;
	    proglog_printf(pip,"argz=%s",argz) ;
	}

	if (rs < 0)
	    goto badadd1 ;

#ifdef	COMMENT
	rs = procusersetup(pip,cip,step) ;
#endif

/* we are good! */

#if	defined(P_TCPMUXD) && (P_TCPMUXD == 1)
	rs = u_write(cip->fd_output,"+\r\n",3) ;
#else
	rs = SR_OK ;
#endif

	if (rs < 0)
	    goto badadd2 ;

/* do it! */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procserver: executing progfname=%s\n",program) ;
#endif

	pip->daytime = time(NULL) ;

	if (pip->open.logprog) {
#ifdef	COMMENT
	    proglog_printf(pip,"server=%s\n",strbasename(program)) ;
#endif
	    proglog_end(pip) ;
	}

	{
	    PROGINFO_IPC	*ipp = &pip->ipc ;
	    if (ipp->fd_req >= 0) {
	        u_close(ipp->fd_req) ;
	        ipp->fd_req = -1 ;
	    }
	    if (cip->fd_input >= 3) {
	        u_close(cip->fd_input) ;
	        cip->fd_input = -1 ;
	    }
	    if (cip->fd_output >= 3) {
	        u_close(cip->fd_output) ;
	        cip->fd_output = -1 ;
	    }
	}

	rs = progexec(pip,progfname,argz,alp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procserver: execute() rs=%d\n",rs) ;
#endif

	if (pip->f.logprog) {
	    proglog_begin(pip,pip->uip) ;
	    if (pip->open.logprog) {
	        proglog_printf(pip,
	            "EXEC could not execute server program (%d)\n",rs) ;
	        proglog_printf(pip,
	            "prog=%s\n",progfname) ;
	    }
	}

/* we are out of here */
done:
badadd2:
badadd1:
badnoprog:
ret0:
	return rs ;
}
/* end subroutines (procserverexec) */


static int procfindprog(pip,plp,prdirs,progfname,pnp,pnl)
PROGINFO	*pip ;
char		progfname[] ;
vecstr		*plp ;
const char	*prdirs[] ;
const char	pnp[] ;
int		pnl ;
{
	int		rs = SR_OK ;
	int		i ;
	int		rlen = 0 ;

	if (pnl < 0) pnl = strlen(pnp) ;

	while (pnl && (pnp[pnl-1] == '/')) pnl -= 1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progserve/procfindprog: pn=%t\n",pnp,pnl) ;
#endif

	if (pnp[0] == '/') {

	    if ((rs = mkpath1w(progfname,pnp,pnl)) >= 0) {
	        rlen = rs ;
	        rs = xfile(&pip->id,progfname) ;
	    }

	} else {

	    for (i = 0 ; prdirs[i] != NULL ; i += 1) {
		cchar	*dir = prdirs[i] ;
	        if ((rs = mkpath3w(progfname,pip->pr,dir,pnp,pnl)) >= 0) {
	            rlen = rs ;
	            rs = xfile(&pip->id,progfname) ;
		}
	        if (rs >= 0) break ;
	    } /* end for */

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progserve/procfindprog: mid rs=%d\n",rs) ;
#endif

	    if ((rs == SR_NOENT) || (rs == SR_ACCESS)) {
	        rs = getprogpath(&pip->id,plp,progfname,pnp,pnl) ;
	        rlen = rs ;
	    }

	} /* end if */

	return (rs >= 0) ? rlen : rs ;
}
/* end subroutines (procfindprog) */


#if	CF_CHECKACCESS

static int procaccperm(PROGINFO *pip,CLIENTINFO *cip,PROCSE *sep)
{
	vecstr		netgroups, names ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = TRUE ;
	cchar		*acc ;

	if ((sep->a.access == NULL) && (pip->defacc == NULL))
	    goto ret0 ;

	if (cip->f_local)
	    goto ret0 ;

	f = FALSE ;
	rs = vecstr_start(&netgroups,4,0) ;
	if (rs < 0)
	    goto ret1 ;

	rs = vecstr_start(&names,4,0) ;
	if (rs < 0)
	    goto ret2 ;

	acc = (sep->a.access != NULL) ? sep->a.access : pip->defacc ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progserve/procaccperm: accgroups=>%s<\n",acc) ;
#endif

/* process the server access list */

	if ((rs = loadaccgroups(pip,&netgroups,acc,-1)) >= 0) {
	    cchar	*defacc = "DEFAULT" ;

#if	CF_ALLDEF
	    if (vecstr_find(&netgroups,defacc) < 0)
	        vecstr_add(&netgroups,defacc,-1) ;
#else
	    if (vecstr_count(&netgroups) <= 0)
	        vecstr_add(&netgroups,defacc,-1) ;
#endif

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    int		i ;
	    cchar	*cp ;
	    debugprintf("progserve/procaccperm: netgroups:\n") ;
	    for (i = 0 ; vecstr_get(&netgroups,i,&cp) >= 0 ; i+= 1) {
	        debugprintf("progserve/procaccperm: ng=>%s<\n",cp) ;
	    }
	}
#endif /* CF_DEBUG */

	if (rs >= 0) {
	    rs = loadpeernames(pip,cip,&names) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    int	i ;
	    cchar	*cp ;
	    debugprintf("progserve/procaccperm: loadpeernames() rs=%d\n",
	        rs) ;
	    for (i = 0 ; vecstr_get(&names,i,&cp) >= 0 ; i += 1) {
	        debugprintf("progserve/procaccperm: mname=>%s<\n",cp) ;
	    }
	}
#endif /* CF_DEBUG */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progserve/procaccperm: open.accfname=%u\n",
		pip->open.accfname) ;
#endif

/* try our own netgroups */

	if ((rs >= 0) && pip->open.accfname) {
	    rs = acctab_anyallowed(&pip->atab,&netgroups,&names,NULL,NULL) ;
	    f = (rs > 0) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progserve/procaccperm: "
	            "acctab_anyallowed() rs=%d\n", rs) ;
#endif

	} /* end if */

/* try the system netgroups (UNIX does not have one simple call as above!) */

	if ((rs >= 0) && ((! pip->open.accfname) || (! f))) {
	    rs1 = netgroupcheck(pip->domainname,&netgroups,&names) ;
	    f = (rs1 > 0) ;
	}

ret4:
ret3:
	vecstr_finish(&names) ;

ret2:
	vecstr_finish(&netgroups) ;

ret1:
	if ((rs >= 0) && (! f)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progserve: access denied\n") ;
#endif

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: access denied\n",pip->progname) ;

	    if (pip->open.logprog)
	        proglog_printf(pip, "access denied\n") ;

	} /* end if */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progserve: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procaccperm) */

#endif /* CF_CHECKACCESS */


static int loadcooks(PROGINFO *pip,CLIENTINFO *cip,cchar **sav)
{
	EXPCOOK		*ecp = &pip->cooks ;
	int		rs = SR_OK ;
	int		rs1 ;

	if ((rs >= 0) && (cip->salen > 0)) {
	    SOCKADDRESS	*sap = &cip->sa ;
	    const int	alen = MAXPATHLEN ;
	    const int	vlen = INETX_ADDRSTRLEN ;
	    int		af = sockaddress_getaf(&cip->sa) ;
	    const char	*name = "ipaddr" ;
	    char	abuf[MAXPATHLEN+1] ;
	    char	vbuf[INETX_ADDRSTRLEN+1] = { 0 } ;
	    switch (af) {
	    case AF_UNIX:
	    case AF_INET4:
	    case AF_INET6:
	        rs = sockaddress_getaddr(sap,abuf,alen) ;
	        if (rs >= 0) {
	            rs1 = sninetaddr(vbuf,vlen,af,abuf) ;
	            if (rs1 >= 0)
	                rs = expcook_add(ecp,name,vbuf,rs1) ;
	        }
	        break ;
	    } /* end switch */
	}

	if ((rs >= 0) && (cip->peername != NULL))
	    rs = expcook_add(&pip->cooks,"P",cip->peername,-1) ;

	if ((rs >= 0) && (cip->peername != NULL))
	    rs = expcook_add(&pip->cooks,"h",cip->peername,-1) ;

	if (rs >= 0)
	    rs = expcook_add(&pip->cooks,"s",cip->service,-1) ;

	if ((rs >= 0) && (cip->subservice != NULL))
	    rs = expcook_add(&pip->cooks,"ss",cip->subservice,-1) ;

	if ((rs >= 0) && (cip->netuser != NULL))
	    rs = expcook_add(&pip->cooks,"u",cip->netuser,-1) ;

	if ((rs >= 0) && (cip->netpass != NULL))
	    rs = expcook_add(&pip->cooks,"p",cip->netpass,-1) ;

	if ((rs >= 0) && (sav != NULL)) {
	    int		size = 1 ;
	    int		i, cr ;
	    char	*svcargs, *cp ;

	    for (i = 0 ; sav[i] != NULL ; i += 1) {
	        size += ((2 * strlen(sav[i])) + 2 + 1) ;
	    }

	    if ((rs = uc_malloc(size,&svcargs)) >= 0) {
	        svcargs[0] = '\0' ;
	        cp = svcargs ;
	        cr = (size - 1) ;
	        for (i = 0 ; (rs >= 0) && (sav[i] != NULL) ; i += 1) {
	            if (i > 0) *cp++ = ' ' ;
	            if ((rs = mkquoted(cp,cr,sav[i],-1)) >= 0) {
	                cp += rs ;
	                *cp = '\0' ;
	            }
	        } /* end for */
	        if (rs >= 0) {
	            rs = expcook_add(&pip->cooks,"a",svcargs,-1) ;
	        }
	        uc_free(svcargs) ;
	    } /* end if (memory-allocation) */

	} /* end if */

	if (rs >= 0) {
	    char	wbuf[2] ;
	    wbuf[0] = ((cip->f_long) ? '1' : '0') ;
	    wbuf[1] = '\0' ;
	    rs = expcook_add(&pip->cooks,"w",wbuf,1) ;
	}

	if (rs >= 0) {
	    char	wbuf[2] ;
	    wbuf[0] = ((cip->f_long) ? '2' : '1') ;
	    wbuf[1] = '\0' ;
	    rs = expcook_add(&pip->cooks,"ww",wbuf,2) ;
	}

	return rs ;
}
/* end subroutine (loadcooks) */


static int loadpeernames(PROGINFO *pip,CLIENTINFO *cip,vecstr *nlp)
{
	CONNECTION	conn ;
	int		rs ;
	int		rs1 = 0 ;
	int		n = 0 ;
	cchar		*cp ;
	char		peername[MAXHOSTNAMELEN + 1] ;

	if (cip == NULL) return SR_FAULT ;
	if (nlp == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("loadpeernames: ent\n") ;
#endif

	peername[0] = '\0' ;

/* use 'connection(3dam)' */

	if ((rs = connection_start(&conn,pip->domainname)) >= 0) {

	    rs1 = 0 ;
	    if (cip->salen > 0) {
	        rs1 = connection_peername(&conn,&cip->sa,cip->salen,peername) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("loadpeernames: connection_peername() rs=%d\n",
	                rs1) ;
#endif
	    }

	    if (rs1 >= 0) {
	        rs1 = connection_mknames(&conn,nlp) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("loadpeernames: connection_mknames() rs=%d\n",
	                rs1) ;
#endif
	        if (rs1 > 0) n += rs1 ;
	    }

	    if ((rs1 >= 0) && (peername[0] != '\0')) {
	        rs = vecstr_adduniq(nlp,peername,-1) ;
	        if (rs < INT_MAX)
	            n += 1 ;
	    }

	    connection_finish(&conn) ;
	} /* end if */

/* use 'nlspeername(3dam)' */

	if ((rs >= 0) && (rs1 < 0) && (! pip->f.daemon)) {
	    if ((cp = getourenv(pip->envv,VARNLSADDR)) != NULL) {
	        rs1 = nlspeername(cp,pip->domainname,peername) ;
	        if (rs1 >= 0) {
	            rs = vecstr_adduniq(nlp,peername,-1) ;
	            if (rs < INT_MAX)
	                n += 1 ;
	        }
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("loadpeernames: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutines (loadpeernames) */


static int loadaccgroups(PROGINFO *pip,vecstr *glp,cchar *accbuf,int acclen)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (glp == NULL) return SR_FAULT ;
	if (accbuf == NULL) return SR_FAULT ;

	if (acclen < 0) acclen = strlen(accbuf) ;

	if ((acclen > 0) && (accbuf[0] != '\0')) {
	    FIELD	af ;

	    if ((rs = field_start(&af,accbuf,acclen)) >= 0) {
	        int	fl, cl ;
	        cchar	*fp, *cp ;

	        while ((fl = field_get(&af,gterms,&fp)) >= 0) {
		    if (fl > 0) {
	                int	bl = fl ;
	                cchar	*bp = fp ;
	                while ((cl = nextfield(bp,bl,&cp)) > 0) {
#if	CF_DEBUG
	                    if (DEBUGLEVEL(5))
	                        debugprintf("loadaccgroups: accgroup=>%s<\n",
					cp) ;
#endif
	                    rs = vecstr_adduniq(glp,cp,cl) ;
	                    if (rs < INT_MAX) c += 1 ;
	                    bl -= ((cp + cl) - bp) ;
	                    bp = (cp + cl) ;
	                    if (rs < 0) break ;
	                } /* end while */
		    } /* end if (positive) */
		    if (af.term == '#') break ;
	            if (rs < 0) break ;
	        } /* end while (fielding) */

	        field_finish(&af) ;
	    } /* end if (field) */

	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadaccgroups) */


