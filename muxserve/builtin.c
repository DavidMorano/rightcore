/* builtin */

/* built-in services */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* switchable */
#define	CF_CHECKDIR	0		/* check for existence of dir */
#define	CF_HOSTINFO	1		/* send HOSTINFO instead of LOADAVE */


/* revision history:

	= 1999-07-21, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module contains the code for the built-in servers. There are not a
        lot of built in servers because there is no way (currently) to shut them
        off except to define a new one by the same name in the regular srver
        table file.


*******************************************************************************/


#define	BUILTIN_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/systeminfo.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */

#include	<vsystem.h>
#include	<vecstr.h>
#include	<buffer.h>
#include	<storebuf.h>
#include	<sockaddress.h>
#include	<netorder.h>
#include	<field.h>
#include	<svcfile.h>
#include	<serialbuf.h>
#include	<connection.h>
#include	<localmisc.h>

#include	"nistinfo.h"
#include	"config.h"
#include	"defs.h"
#include	"clientinfo.h"
#include	"builtin.h"
#include	"standing.h"
#include	"muximsg.h"
#include	"sysmisc.h"


/* local defines */

#define	IPC_MAGIC	0x83726154

#define	IPCDIRMODE	0777
#define	EPOCHDIFF	(86400 * ((365 * 70) + 17))
#define	TI_RECVMSG	3
#define	TIMEOUT		30

#ifndef	SVCNAMELEN
#define	SVCNAMELEN	32
#endif

#ifndef	SVCBUFLEN
#define	SVCBUFLEN	(4 * MAXPATHLEN)
#endif

#ifndef	FBUFLEN
#define	FBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	BUFLEN
#define	BUFLEN		1024
#endif

#if	CF_DEBUGS
#define	BUILTIN_DEBUG	"/tmp/builtin.deb"
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	chmods(const char *,mode_t) ;
extern int	getproviderid(const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;
extern int	bufprintf(char *,int,const char *,...) ;

extern int	progtmpdir(PROGINFO *,char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_nist(time_t,struct nistinfo *,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* local structures */

struct ipc {
	ulong		magic ;
	SOCKADDRESS	sa ;
	PROGINFO	*pip ;
	int		fd ;
	char		sfname[MAXPATHLEN + 1] ;
} ;

struct ipcmsg_testint {
	char		*sp ;
	int		sl ;
	int		a_timelen ;
	int		a_timeint ;
} ;


/* forward refereces */

static uint	mknettime(time_t) ;

static int	builtin_help(BUILTIN *,STANDING *,
			CLIENTINFO *,const char **) ;
static int	builtin_daytime(BUILTIN *,STANDING *,
			CLIENTINFO *,const char **) ;
static int	builtin_time(BUILTIN *,STANDING *,
			CLIENTINFO *,const char **) ;
static int	builtin_sysmisc(BUILTIN *,STANDING *,
			CLIENTINFO *,const char **) ;
static int	builtin_test1(BUILTIN *,STANDING *,
			CLIENTINFO *,const char **) ;
static int	builtin_test2(BUILTIN *,STANDING *,
			CLIENTINFO *,const char **) ;

#ifdef	COMMENT
static int	builtin_test3(BUILTIN *,STANDING *,
			CLIENTINFO *,const char **) ;
#endif

static int	ipc_open(struct ipc *,PROGINFO *) ;
static int	ipc_send(struct ipc *,const char *,int) ;
static int	ipc_recv(struct ipc *,char *,int) ;
static int	ipc_close(struct ipc *) ;

static int	scall_sysmisc(BUILTIN *,struct ipc *,CLIENTINFO *,
			STANDING_SYSMISC *) ;

#ifdef	COMMENT
static int	scall_testint(BUILTIN *,struct ipc *,CLIENTINFO *,
			struct ipcmsg_testint *) ;
#endif


/* local variables */

static const char	*bisvcs[] = {
	"help",
	"daytime",
#ifdef	P_TCPMUXD
	"time",
	"sysmisc",
#endif
#ifdef	COMMENT
	"test1",
	"test2",
	"test_daytime",
	"test_sysmisc",
	"test3",
#endif
	NULL
} ;

enum bisvcs {
	bisvc_help,
	bisvc_daytime,
	bisvc_time,
	bisvc_sysmisc,
	bisvc_test1,
	bisvc_test2,
	bisvc_testdaytime,
	bisvc_testsysmisc,
#ifdef	COMMENT
	bisvc_test3,
#endif
	bisvc_overlast
} ;


/* exported subroutines */


int builtin_start(bip,pip)
BUILTIN		*bip ;
PROGINFO	*pip ;
{
	int		rs = SR_OK ;

	if (bip == NULL) return SR_FAULT ;

	memset(bip,0,sizeof(BUILTIN)) ;

	bip->nentries = -1 ;
	bip->pip = pip ;
	bip->sfp = &pip->stab ;
	bip->hostnamelen = strlen(pip->hostname) ;

/* initialize the cached data area */

	memset(&bip->c,0,sizeof(struct builtin_cache)) ;

	bip->magic = BUILTIN_MAGIC ;

	return rs ;
}
/* end subroutine (builtin_start) */


int builtin_finish(bip)
BUILTIN		*bip ;
{

	if (bip == NULL) return SR_FAULT ;

	bip->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (builtin_finish) */


int builtin_enum(bip,ei,spp)
BUILTIN		*bip ;
int		ei ;
const char	**spp ;
{
	int		i ;

	if (bip == NULL) return SR_FAULT ;
	if (spp == NULL) return SR_FAULT ;

	if (bip->nentries < 0) {
	    for (i = 0 ; bisvcs[i] != NULL ; i += 1) ;
	    bip->nentries = i ;
	}

	*spp = NULL ;
	if (ei > bip->nentries)
	    return SR_NOTFOUND ;

	*spp = bisvcs[ei] ;
	return ei ;
}
/* end subroutine (builtint_enum) */


int builtin_match(bip,service)
BUILTIN		*bip ;
const char	service[] ;
{
	PROGINFO	*pip = bip->pip ;
	int		rs = SR_NOTFOUND ;
	int		i ;

	if (pip == NULL) return SR_FAULT ;

	for (i = 0 ; bisvcs[i] != NULL ; i += 1) {
	    if (strcasecmp(service,bisvcs[i]) == 0) {
	        rs = SR_OK ;
	        break ;
	    }
	} /* end for (looping through entries) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (builtin_match) */


/* execute a builtin service request */
int builtin_execute(bip,ourp,cip,si,sargv)
BUILTIN		*bip ;
STANDING	*ourp ;
CLIENTINFO	*cip ;
int		si ;			/* service index */
const char	*sargv[] ;
{
	PROGINFO	*pip = bip->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	switch (si) {
	case bisvc_help:
	    rs = builtin_help(bip,ourp,cip,sargv) ;
	    break ;
	case bisvc_daytime:
	    rs = builtin_daytime(bip,ourp,cip,sargv) ;
	    break ;
	case bisvc_time:
	    rs = builtin_time(bip,ourp,cip,sargv) ;
	    break ;
	case bisvc_sysmisc:
	    rs = builtin_sysmisc(bip,ourp,cip,sargv) ;
	    break ;
	case bisvc_test1:
	case bisvc_testdaytime:
	    rs = builtin_test1(bip,ourp,cip,sargv) ;
	    break ;
	case bisvc_test2:
	case bisvc_testsysmisc:
	    rs = builtin_test2(bip,ourp,cip,sargv) ;
	    break ;
#ifdef	COMMENT
	case bisvc_test3:
	    rs = builtin_test3(bip,ourp,cip,sargv) ;
	    break ;
#endif
	default:
	    rs = SR_NOTFOUND ;
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (builtin_execute) */


/* local subroutines */


static int builtin_help(bip,ourp,cip,sargv)
BUILTIN		*bip ;
STANDING	*ourp ;
CLIENTINFO	*cip ;
const char	*sargv[] ;
{
	PROGINFO	*pip = bip->pip ;
	BUFFER		bo ;
	int		rs ;
	int		rs1 ;
	int		blen = 0 ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = buffer_start(&bo,200)) >= 0) {
	    SVCFILE_CUR	cur ;
	    const int	svclen = SVCNAMELEN ;
	    int		i ;
	    cchar	*bp ;
	    char	svcbuf[SVCNAMELEN + 1] ;

/* list out the builtin servers */

	for (i = 0 ; bisvcs[i] != NULL ; i += 1) {
	    if (svcfile_match(bip->sfp,bisvcs[i]) < 0) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("builtin_help: bisvc=%s\n",bisvcs[i]) ;
#endif
	        buffer_buf(&bo,bisvcs[i],-1) ;
	        buffer_char(&bo,'\n') ;
	    } /* end if */
	} /* end for */

/* list out the servers in the current server table file */

	svcfile_curbegin(bip->sfp,&cur) ;

	while ((i = svcfile_enumsvc(bip->sfp,&cur,svcbuf,svclen)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("builtin_help: svclen=%u svc=%s\n",i,svcbuf) ;
#endif

	    if (matstr(bisvcs,svcbuf,-1) < 0) {
	        buffer_buf(&bo,svcbuf,-1) ;
	        buffer_char(&bo,'\n') ;
	    } /* end if (not already listed) */

	} /* end while */

	svcfile_curend(bip->sfp,&cur) ;

	if ((rs = buffer_get(&bo,&bp)) >= 0) {
	    blen = rs ;
	    rs = uc_writen(cip->fd_output,bp,blen) ;
	}

	    rs1 = buffer_finish(&bo) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (buffer) */

	return (rs >= 0) ? blen : rs ;
}
/* end subroutine (builtin_help) */


static int builtin_daytime(bip,ourp,cip,sargv)
BUILTIN		*bip ;
STANDING	*ourp ;
CLIENTINFO	*cip ;
const char	*sargv[] ;
{
	PROGINFO	*pip = bip->pip ;
	struct nistinfo	ni ;
	int		rs = SR_OK ;
	int		bl ;
	char		tbuf[NISTINFO_BUFLEN+1+1] ;

	memset(&ni,0,sizeof(struct nistinfo)) ;

	strncpy(ni.org,pip->orgcode,NISTINFO_ORGLEN) ;

	pip->daytime = time(NULL) ;

	timestr_nist(pip->daytime,&ni,tbuf) ;

	bl = strlen(tbuf) ;
	buf[bl++] = '\n' ;

	rs = uc_writen(cip->fd_output,tbuf,bl) ;

	return rs ;
}
/* end subroutine (builtin_daytime) */


static int builtin_time(bip,ourp,cip,sargv)
BUILTIN		*bip ;
STANDING	*ourp ;
CLIENTINFO	*cip ;
const char	*sargv[] ;
{
	PROGINFO	*pip = bip->pip ;
	int		rs = SR_OK ;
	uint		nettime ;
	char		buf[8] ;


	pip->daytime = time(NULL) ;

	nettime = mknettime(pip->daytime) ;

	netorder_wuint(buf,nettime) ;

	rs = uc_writen(cip->fd_output,buf,4) ;

	return rs ;
}
/* end subroutine (builtin_time) */


/* handle the 'sysmisc' service */
static int builtin_sysmisc(bip,ourp,cip,sargv)
BUILTIN		*bip ;
STANDING	*ourp ;
CLIENTINFO	*cip ;
const char	*sargv[] ;
{
	struct sysmisc_request	m0 ;
	struct sysmisc_loadave	m1 ;
	struct sysmisc_extra	m2 ;
	struct sysmisc_hostinfo	m3 ;
	struct muximsg_loadave	i7 ;
	struct muximsg_reploadave	i8 ;
	STANDING_SYSMISC	sdata ;
	PROGINFO	*pip = bip->pip ;
	struct ipc	server ;
	int		rs, rs1 ;
	int		len, rlen, blen ;
	int		rc ;
	int		f_ipc = FALSE ;
	int		f_needrefresh = FALSE ;
	int		f_notavail = FALSE ;
	int		f_repudp ;
	int		f_invalid = FALSE ;
	char		buf[MSGBUFLEN + 1], *bp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin_sysmisc: ent, mtype=%ld\n",
	        cip->mtype) ;
#endif

/* read the client request */

	rlen = SYSMISC_SREQUEST ;
	rs = uc_reade(cip->fd_input,buf,rlen,TIMEOUT,FM_EXACT) ;
	len = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin_sysmisc: uc_reade() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

/* determine what the client wants */

	rs1 = sysmisc_request(&m0,1,buf,len) ;
	if (rs1 < 0)
	    goto badrequest ;

	if (m0.msgtype != sysmisctype_request)
	    goto badrequest ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin_sysmisc: af=%d port=%d\n",
	        ntohs(m0.addrfamily),ntohs(m0.addrport)) ;
#endif

	if ((m0.addrfamily == 0) || (m0.addrport == 0))
	    goto badrequest ;

/* OK, I guess a hacker might not get this far -- continue */

	f_repudp = (m0.duration > 0) && (m0.opts & SYSMISC_MUDP) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("builtin_sysmisc: f_repudp=%d opts=%04x\n",
	        f_repudp,m0.opts) ;
	    debugprintf("builtin_sysmisc: duration=%d interval=%d\n",
	        m0.duration,m0.interval) ;
	}
#endif

/* do we have some data already? */

	rs1 = standing_readdata(ourp,&sdata) ;
	f_needrefresh = (rs1 < 0) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("builtin_sysmisc: readdata rs=%d\n",rs1) ;
#endif

	if (f_needrefresh || f_repudp) {

	    rs = ipc_open(&server,pip) ;
	    f_ipc = (rs > 0) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("builtin_sysmisc: ipc_open() rs=%d\n",rs) ;
#endif

	} /* end if (initialized channel to super-server) */

	blen = 0 ;
	if (f_repudp) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("builtin_sysmisc: got REPUDP\n") ;
#endif

	    i8.tag = 0 ;
	    i8.utag = m0.tag ;
	    i8.duration = m0.duration ;
	    i8.interval = (m0.interval != 0) ? m0.interval : 1 ;
	    i8.addrfamily = m0.addrfamily ;
	    i8.addrport = m0.addrport ;

	    if (m0.addrhost[0] != 0) {

	        i8.addrhost[0] = m0.addrhost[0] ;
	        i8.addrhost[1] = m0.addrhost[1] ;
	        i8.addrhost[2] = m0.addrhost[2] ;
	        i8.addrhost[3] = m0.addrhost[3] ;

	    } else {
	        int	af = sockaddress_getaf(&cip->sa) ;

	        rs = SR_INVALID ;
	        if ((af == AF_INET) || (af == AF_INET6))
	            rs = sockaddress_getaddr(&cip->sa,&i8.addrhost[0],16) ;

	        f_invalid = (rs < 0) ;
	    }

	    if (! f_invalid) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("builtin_sysmisc: afamily=%04x aport=%04x\n",
	                i8.addrfamily,i8.addrport) ;
	            debugprintf("builtin_sysmisc: addrhost[0]=%08x\n",
	                i8.addrhost) ;
	        }
#endif /* CF_DEBUG */

	        blen = muximsg_reploadave(&i8,0,buf,MSGBUFLEN) ;

	        rs = ipc_send(&server,buf,blen) ;
	        f_notavail = (rs < 0) ;

/* wait for response */

	        if (rs >= 0) {

	            rs = ipc_recv(&server,buf,BUFLEN) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("builtin_sysmisc: UDP ipc_recv() rs=%d\n",
	                    rs) ;
#endif

	            f_notavail = TRUE ;
	            if (rs >= 0) {

	                muximsg_loadave(&i7,1,buf,rs) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("builtin_sysmisc: REPUDP rc=%d\n",
	                        i7.rc) ;
#endif

	                f_notavail = (i7.rc != muximsgrc_ok) ;
	            }
	        }

	        rc = (f_notavail) ? sysmiscrc_notavail : sysmiscrc_ok ;

	    } else
	        rc = sysmiscrc_invalid ;

/* form and send a response to the client */

#if	CF_HOSTINFO

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("builtin_sysmisc: REPUDP sending type=3\n") ;
#endif

	    m3.tag = m0.tag ;
	    m3.rc = (uchar) rc ;
	    m3.timestamp = cip->stime ;
	    m3.providerid = pip->providerid ;
	    m3.hostid = pip->hostid ;
	    m3.la_1min = i7.la_1min ;
	    m3.la_5min = i7.la_5min ;
	    m3.la_15min = i7.la_15min ;
	    m3.boottime = sdata.boottime ;
	    m3.nproc = sdata.nproc ;

	    strwcpy(m3.provider,pip->provider,SYSMISC_PROVIDERLEN) ;

	    m3.hostnamelen = bip->hostnamelen ;
	    strwcpy(m3.hostname,pip->hostname,bip->hostnamelen) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("builtin_sysmisc: len=%d hostname=%s\n",
	            m3.hostnamelen,m3.hostname) ;
#endif

	    blen = sysmisc_hostinfo(&m3,0,buf,BUFLEN) ;

#else /* CF_HOSTINFO */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("builtin_sysmisc: REPUDP sending type=1\n") ;
#endif

	    m1.tag = m0.tag ;
	    m1.rc = (uchar) rc ;
	    m1.timestamp = cip->stime ;
	    m1.providerid = pip->providerid ;
	    m1.hostid = pip->hostid ;
	    m1.la_1min = i7.la_1min ;
	    m1.la_5min = i7.la_5min ;
	    m1.la_15min = i7.la_15min ;
	    blen = sysmisc_loadave(&m1,0,buf,BUFLEN) ;

#endif /* CF_HOSTINFO */

	} else {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("builtin_sysmisc: simple non-repeating\n") ;
#endif

/* build our message to the super-server */

	    if (f_needrefresh) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("builtin_sysmisc: need refresh\n") ;
#endif

	        rs = scall_sysmisc(bip,&server,cip,&sdata) ;
	        f_notavail = (rs != 0) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("builtin_sysmisc: scall_sysmisc() rs=%d\n",
	                rs) ;
	            debugprintf("builtin_sysmisc: la_15min=%d\n",
	                sdata.la_15min) ;
	        }
#endif /* CF_DEBUG */

	    } /* end if (communicating w/ server) */

	    rc = (f_notavail) ? sysmiscrc_notavail : sysmiscrc_ok ;

/* form and send appropriate response to client */

	    if (! (m0.opts & SYSMISC_MEXTRA)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("builtin_sysmisc: not-extra ls_15min=%d\n",
	                sdata.la_15min) ;
#endif

#if	CF_HOSTINFO

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("builtin_sysmisc: one-time sending type=3\n") ;
#endif

	        m3.tag = m0.tag ;
	        m3.rc = (uchar) rc ;
	        m3.timestamp = cip->stime ;
	        m3.providerid = pip->providerid ;
	        m3.hostid = pip->hostid ;
	        m3.la_1min = sdata.la_1min ;
	        m3.la_5min = sdata.la_5min ;
	        m3.la_15min = sdata.la_15min ;
	        m3.boottime = sdata.boottime ;
	        m3.nproc = sdata.nproc ;
	        strwcpy(m3.provider,pip->provider,SYSMISC_PROVIDERLEN) ;

	        m3.hostnamelen = bip->hostnamelen ;
	        strwcpy(m3.hostname,pip->hostname,bip->hostnamelen) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("builtin_sysmisc: len=%d hostname=%s\n",
	                m3.hostnamelen,m3.hostname) ;
#endif

	        blen = sysmisc_hostinfo(&m3,0,buf,BUFLEN) ;

#else

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("builtin_sysmisc: one-time sending type=1\n") ;
#endif

	        m1.tag = m0.tag ;
	        m1.rc = rc ;
	        m1.timestamp = cip->stime ;
	        m1.providerid = pip->providerid ;
	        m1.hostid = pip->hostid ;
	        m1.la_1min = sdata.la_1min ;
	        m1.la_5min = sdata.la_5min ;
	        m1.la_15min = sdata.la_15min ;
	        blen = sysmisc_loadave(&m1,0,buf,BUFLEN) ;

#endif /* CF_HOSTINFO */

	    } else {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("builtin_sysmisc: one-time sending type=2\n") ;
#endif

	        m2.tag = m0.tag ;
	        m2.rc = rc ;
	        m2.timestamp = cip->stime ;
	        m2.providerid = pip->providerid ;
	        m2.hostid = pip->hostid ;
	        m2.la_1min = sdata.la_1min ;
	        m2.la_5min = sdata.la_5min ;
	        m2.la_15min = sdata.la_15min ;
	        m2.boottime = sdata.boottime ;
	        m2.nproc = sdata.nproc ;
	        blen = sysmisc_extra(&m2,0,buf,BUFLEN) ;

	    }

	} /* end if (handled locally) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin_sysmisc: client send len=%d\n",blen) ;
#endif

	if (blen > 0)
	    rs = uc_writen(cip->fd_output,buf,blen) ;

ret1:
	if (f_ipc)
	    ipc_close(&server) ;

ret0:
	return rs ;

/* bad thing */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("builtin_sysmisc: bad thing, rs=%d\n",rs) ;
#endif

#if	defined(P_TCPMUXD) && (P_TCPMUXD == 1)
#ifdef	COMMENT
	bp = "- built-in server not found\r\n" ;
#else
	bp = "" ;
#endif
#else
	bp = "no user\n" ;
#endif

	rs = uc_writen(cip->fd_output,bp,strlen(bp)) ;

	goto ret1 ;

/* send back negative acknoweldgement for bad request */
badrequest:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin_sysmisc: bad request\n") ;
#endif

	memset(&m1,0,sizeof(struct sysmisc_loadave)) ;

	m1.tag = m0.tag ;
	m1.rc = sysmiscrc_invalid ;
	blen = sysmisc_loadave(&m1,0,buf,BUFLEN) ;

	rs = uc_writen(cip->fd_output,buf,blen) ;

	goto ret0 ;
}
/* end subroutine (builtin_sysmisc) */


/* testing services */
static int builtin_test1(bip,ourp,cip,sargv)
BUILTIN		*bip ;
STANDING	*ourp ;
CLIENTINFO	*cip ;
const char	*sargv[] ;
{
	STANDING_SYSMISC	sdata ;
	PROGINFO	*pip = bip->pip ;
	struct ipc	server ;
	time_t		boottime ;
	int		rs = SR_OK ;
	int		bi ;
	int		f_ipc = FALSE ;
	char		buf[BUFLEN + 1], *bp ;
	char		timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin_test1: ent, mtype=%ld\n",
	        cip->mtype) ;
#endif

/****
	
	We have two levels of cache available:
	1) ourselves, stored in 'bip->c'
	2) calling the *local* standing server for its cached data
	
	Else, we have to ask the standing server in the super-server
	for the data!  Sigh.  I think that it has a cache also
	but that is its business, not ours! :-)
	
****/

/* do we have to go back to the super-server for answers? */

	rs = SR_OK ;
	if (bip->c.boottime == 0) {

#if	CF_DEBUGS
	    nprintf(BUILTIN_DEBUG,"1 zero\n") ;
#endif

	    rs = standing_readdata(ourp,&sdata) ;

	    if (rs >= 0)
	        bip->c.boottime = sdata.boottime ;

#if	CF_DEBUGS
	    nprintf(BUILTIN_DEBUG,"2 rs=%d boot=%s\n",
	        rs,timestr_logz(sdata.boottime,timebuf)) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("builtin_test1: standing says rs=%d boot=%s\n",
	            rs,timestr_logz(sdata.boottime,timebuf)) ;
#endif

	} /* end if */


#if	CF_DEBUGS
	nprintf(BUILTIN_DEBUG,"3 rs=%d\n",rs) ;
#endif

	if (rs < 0) {

	    if (! f_ipc) {

	        rs = ipc_open(&server,pip) ;
	        f_ipc = (rs > 0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	            debugprintf("builtin_test1: ipc_open() rs=%d\n",rs) ;
#endif

	    }

/* send message to the super-server */

	    rs = scall_sysmisc(bip,&server,cip,&sdata) ;
	    if (rs != 0)
	        goto bad ;

	    bip->c.boottime = sdata.boottime ;
	    boottime = sdata.boottime ;

	} else
	    boottime = bip->c.boottime ;

#if	CF_DEBUGS
	nprintf(BUILTIN_DEBUG,"4 boot=%s\n",
	    timestr_logz(boottime,timebuf)) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin_test1: answer boot=%s\n",
	        timestr_logz(boottime,timebuf)) ;
#endif


/* we're good */

	bp = buf ;
	bi = 0 ;

	bp += bufprintf(bp,(BUFLEN - bi),"%s\n",
	    timestr_logz(boottime,timebuf)) ;

#if	CF_DEBUGS
	nprintf(BUILTIN_DEBUG,"5 boot=%s\n",bp) ;
#endif

	bi = bp - buf ;
	rs = uc_writen(cip->fd_output,buf,bi) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin_test1: final write to client, rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	nprintf(BUILTIN_DEBUG,"6 rs=%d\n",rs) ;
#endif

ret1:
	if (f_ipc)
	    ipc_close(&server) ;

	return rs ;

/* bad thing */
bad:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin_test1: bad thing, rs=%d\n",rs) ;
#endif

#if	defined(P_TCPMUXD) && (P_TCPMUXD == 1)
#ifdef	COMMENT
	bp = "- built-in server not found\n" ;
#else
	bp = "" ;
#endif
#else
	bp = "no user\n" ;
#endif

	rs = uc_writen(cip->fd_output,bp,strlen(bp)) ;

	goto ret1 ;
}
/* end subroutine (builtin_test1) */


/* more testing services */
static int builtin_test2(bip,ourp,cip,sargv)
BUILTIN		*bip ;
STANDING	*ourp ;
CLIENTINFO	*cip ;
const char	*sargv[] ;
{
	STANDING_SYSMISC	sdata ;
	PROGINFO	*pip = bip->pip ;
	struct ipc	server ;
	int		rs = SR_OK ;
	int		bi ;
	int		f_ipc = FALSE ;
	char		buf[BUFLEN + 1], *bp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin_test2: ent mtype=%ld\n",
	        cip->mtype) ;
#endif

/****
	
	We have one level of cache available:
	1) calling the *local* standing server for its cached data
	Else, we have to ask the standing server in the super-server
	for the data!  Sigh.  I think that it has a cache also
	but that is its business, not ours! :-)
	
****/

/* do we have to go back to the super-server for answers? */

	rs = standing_readdata(ourp,&sdata) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin_test2: readdata rs=%d\n",rs) ;
#endif

	if (rs < 0) {

	    if (! f_ipc) {

	        rs = ipc_open(&server,pip) ;
	        f_ipc = (rs > 0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	            debugprintf("builtin_test2: ipc_open() rs=%d\n",rs) ;
#endif

	    }

/* build our message to the super-server */

	    rs = scall_sysmisc(bip,&server,cip,&sdata) ;

	    if (rs != 0)
	        goto bad ;

	} /* end if (communicating w/ server) */

/* we're good */

	bp = buf ;
	bi = 0 ;

	bp += bufprintf(bp,(BUFLEN - bi),"%7d\n",
	    sdata.nproc) ;

	{
	    int	la_i, la_f ;


	    la_i = sdata.la_1min >> FSHIFT ;
	    la_f = +((sdata.la_1min & (FSCALE - 1)) * 1000) / FSCALE ;

	    bp += bufprintf(bp,(BUFLEN - bi),"%3d.%03d\n",
	        la_i,la_f) ;

	    la_i = sdata.la_5min >> FSHIFT ;
	    la_f = +((sdata.la_5min & (FSCALE - 1)) * 1000) / FSCALE ;

	    bp += bufprintf(bp,(BUFLEN - bi),"%3d.%03d\n",
	        la_i,la_f) ;

	    la_i = sdata.la_15min >> FSHIFT ;
	    la_f = +((sdata.la_15min & (FSCALE - 1)) * 1000) / FSCALE ;

	    bp += bufprintf(bp,(BUFLEN - bi),"%3d.%03d\n",
	        la_i,la_f) ;

	} /* end block */

	bi = bp - buf ;
	rs = uc_writen(cip->fd_output,buf,bi) ;

ret1:
	if (f_ipc)
	    ipc_close(&server) ;

	return rs ;

/* bad thing */
bad:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin_test2: bad thing, rs=%d\n",rs) ;
#endif

#if	defined(P_TCPMUXD) && (P_TCPMUXD == 1)
#ifdef	COMMENT
	bp = "- built-in server not found\r\n" ;
#else
	bp = "" ;
#endif
#else
	bp = "no user\n" ;
#endif

	rs = uc_writen(cip->fd_output,bp,strlen(bp)) ;

	goto ret1 ;
}
/* end subroutine (builtin_test2) */


#ifdef	COMMENT

static int builtin_test3(bip,ourp,cip,sargv)
BUILTIN		*bip ;
STANDING	*ourp ;
CLIENTINFO	*cip ;
const char	*sargv[] ;
{
	STANDING_SYSMISC	sdata ;
	PROGINFO	*pip = bip->pip ;
	struct ipc	server ;
	FIELD		fb ;
	int		rs = SR_OK ;
	int		bi, len ;
	int		sl, cl ;
	int		a_timelen, a_timeint ;
	int		ch ;
	int		f_ipc = FALSE ;
	int		f_bad = FALSE ;
	const char	*sp ;
	const char	*cp ;
	char		buf[BUFLEN + 1], *bp ;
	char		timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin_test3: ent mtype=%ld\n",
	        cip->mtype) ;
#endif

/* print hello */

	cp = "hello, enter your stuff\n" ;
	rs = uc_writen(ofd,cp,strlen(cp)) ;

/* read whatever the client sent us */

	if (rs >= 0) {
	    rs = uc_readlinetimed(cip->fd_input,buf,BUFLEN,TIMEOUT) ;
	    len = rs ;
	}

	if (rs <= 0)
	    goto badnodata ;

/* parse it for 1) arbitrary string 2) length_of_time 3) interval */

	if ((rs = field_start(&fb,buf,len)) >= 0) {

	    sl = field_get(&fb,NULL) ;

	    sp = fb.fp ;

	    cl = field_get(&fb,NULL) ;

	    rs = cfdeci(fb.fp,fb.flen,&a_timelen) ;
	    if (rs < 0)
	        f_bad = TRUE ;

	    cl = field_get(&fb,NULL) ;

	    if (rs >= 0)
	        rs = cfdeci(fb.fp,fb.flen,&a_timeint) ;
	    if (rs < 0)
	        f_bad = TRUE ;

	    cl = field_finish(&fb) ;

	    rs = (! f_bad) ? 0 : SR_INVALID ;

	} /* end block */

	if (rs < 0)
	    goto baddata ;

/* open IPC channel back to the super-server (we're the child server!) */

	if (! f_ipc) {

	    rs = ipc_open(&server,pip) ;
	    f_ipc = (rs > 0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("builtin_test3: ipc_open() rs=%d\n",rs) ;
#endif

	}

/* build our message to the super-server */

	{
	    struct ipcmsg_testint	a ;

	    a.sp = sp ;
	    a.sl = sl ;
	    a.a_timelen = a_timelen ;
	    a.a_timeint = a_timeint ;
	    rs = scall_testint(bip,&server,cip,&a) ;
	    if (rs != 0)
	        goto bad ;

	} /* end block */

	cl = bufprintf(buf,BUFLEN,"the server says rc=%d\n",rs) ;

	rs = uc_writen(ofd,buf,cl) ;

ret1:
	if (f_ipc)
	    ipc_close(&server) ;

ret0:
	return rs ;

/* bad thing */
bad:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin_test3: bad thing, rs=%d\n",rs) ;
#endif

#if	defined(P_TCPMUXD) && (P_TCPMUXD == 1)
#ifdef	COMMENT
	bp = "- built-in server not found\r\n" ;
#else
	bp = "" ;
#endif
#else
	bp = "no user\n" ;
#endif

	uc_writen(FD_STDOUT,bp,strlen(bp)) ;

	goto ret1 ;

baddata:
	cp = "sorry, bad data\n" ;
	uc_writen(ofd,cp,strlen(cp)) ;

	goto ret1 ;

badnodata:
	cp = "sorry, no go\n" ;
	uc_writen(ofd,cp,strlen(cp)) ;

	goto ret1 ;
}
/* end subroutine (builtin_test3) */

#endif /* COMMENT */


/* private object */


static int ipc_open(ip,pip)
struct ipc	*ip ;
PROGINFO	*pip ;
{
	PROGINFO_IPC	*ipp = &pip->ipc ;
	struct sockaddr	*sap ;
	mode_t		operms ;
	int		rs ;
	int		oflags = O_RDWR ;
	int		f_open = FALSE ;
	char		dname[MAXPATHLEN + 1] ;
	char		template[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("builtin/ipc_open: ip=%p\n",ip) ;
	debugprintf("builtin/ipc_open: pip=%p\n",pip) ;
#endif

	ip->magic = 0 ;
	ip->pip = pip ;
	ip->fd = -1 ;
	ip->sfname[0] = '\0' ;

/* do some checking for the directory we want to use */

	rs = mkpath3(dname,pip->tmpdname,pip->rootname,pip->searchname) ;

	if (rs >= 0)
	    rs = mkpath2(template,dname,"tsXXXXXXXXXXXX") ;

	if (rs < 0)
	    goto bad0 ;

	operms = (S_IFSOCK | 0600) ;
	rs = opentmpusd(template,oflags,operms,ip->sfname) ;
	if (rs < 0) {

#if	CF_CHECKDIR

#ifdef	OPTIONAL

	    rs = progtmpdir(pip,dname) ;
	    if (rs >= 0) {

	        rs = mkpath2(template,dname,"tsXXXXXXXXXXXX") ;

		if (rs >= 0)
	        rs = opentmpusd(template,oflags,operms,ip->sfname) ;

	    }

#else /* OPTIONAL */
	{
	    struct ustat	sb ;

	    rs = u_stat(dname,&sb) ;
	    if ((rs < 0) || ((sb.st_mode & S_ISVTX) != S_ISVTX)) {
	        const mode_t	cmode = (IPCDIRMODE | S_ISVTX) ;

	        if ((rs = mkdirs(dname,cmode)) >= 0) {
	            chmods(dname,cmode) ;
		}
	    }

	    if (rs >= 0)
	        rs = opentmpusd(template,oflags,oflags,ip->sfname) ;

	} /* end block */
#endif /* OPTIONAL */

#endif /* CF_CHECKDIR */

	    if (rs < 0)
	        goto bad0 ;

	} /* end if */

	ip->fd = rs ;
	rs = sockaddress_start(&ip->sa,AF_UNIX,ip->sfname,0,0) ;
	if (rs < 0)
	    goto bad1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    int	cl ;
	    char	topath[MAXPATHLEN + 1] ;
	    cl = sockaddress_getaddr(&ipp->sa,topath,(MAXPATHLEN - 1)) ;
	    debugprintf("builtin/ipc_open: salen=%d cl=%d topath=%s\n",
	        ipp->salen,cl,topath) ;
	}
#endif /* CF_DEBUG */

	sap = (struct sockaddr *) &ipp->sa ;
	rs = u_connect(ip->fd,sap,ipp->salen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin/ipc_open: u_connect() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad2 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("builtin/ipc_open: ret FD=%d\n",ip->fd) ;
#endif

	ip->magic = IPC_MAGIC ;
	f_open = TRUE ;

ret0:
	return (rs >= 0) ? f_open : rs ;

/* bad stuff comes here */
bad2:
	sockaddress_finish(&ipp->sa) ;

bad1:
	u_close(ip->fd) ;
	ip->fd = -1 ;

	u_unlink(ip->sfname) ;
	ip->sfname[0] = '\0' ;
bad0:
	goto ret0 ;
}
/* end subroutine (ipc_open) */


static int ipc_close(struct ipc *ip)
{
	PROGINFO	*pip = ip->pip ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("builtin/ipc_close: not NULL\n") ;
#endif

	sockaddress_finish(&ip->sa) ;

#if	CF_DEBUGS
	debugprintf("builtin/ipc_close: sfname=%s\n",ip->sfname) ;
#endif

	if (ip->sfname[0] != '\0') {
	    u_unlink(ip->sfname) ;
	    ip->sfname[0] = '\0' ;
	}

	if (ip->fd >= 0) {
	    u_close(ip->fd) ;
	    ip->fd = -1 ;
	}

	ip->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (ipc_close) */


static int ipc_send(ip,buf,buflen)
struct ipc	*ip ;
const char	buf[] ;
int		buflen ;
{
	PROGINFO	*pip = ip->pip ;
	int		rs ;

	if (pip == NULL) return SR_FAULT ;

	rs = u_send(ip->fd,buf,buflen,0) ;

	return rs ;
}
/* end subroutine (ipc_send) */


static int ipc_recv(ip,buf,buflen)
struct ipc	*ip ;
char		buf[] ;
int		buflen ;
{
	PROGINFO	*pip = ip->pip ;
	int		rs ;

	if (pip == NULL) return SR_FAULT ; /* lint */

	rs = uc_recve(ip->fd,buf,buflen,0,TI_RECVMSG,0) ;

	return rs ;
}
/* end subroutine (ipc_recv) */


/* call the server */
static int scall_sysmisc(bip,ip,cip,dp)
BUILTIN		*bip ;
struct ipc	*ip ;
CLIENTINFO	*cip ;
STANDING_SYSMISC	*dp ;
{
	PROGINFO	*pip = bip->pip ;
	struct muximsg_getsysmisc	m1 ;
	struct muximsg_sysmisc	m2 ;
	int		rs ;
	int		rc ;
	int		blen ;
	char		buf[MSGBUFLEN + 1] ;

	if (pip == NULL) return SR_FAULT ;

	m1.tag = 0 ;
	blen = muximsg_getsysmisc(&m1,0,buf,MSGBUFLEN) ;

/* send it to the super-server */

	rs = ipc_send(ip,buf,blen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("scall_sysmisc: ipc_send() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad ;

/* wait for response */

	rs = ipc_recv(ip,buf,MSGBUFLEN) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("scall_sysmisc: ipc_recv() rs=%d\n",rs) ;
#endif

	blen = rs ;
	if (rs < 0)
	    goto bad ;

/* parse the response */

	muximsg_sysmisc(&m2,1,buf,blen) ;

	if (m2.msgtype == muximsgtype_sysmisc) {

	    dp->la_1min = m2.la_1min ;
	    dp->la_5min = m2.la_5min ;
	    dp->la_15min = m2.la_15min ;
	    dp->boottime = m2.boottime ;
	    dp->nproc = m2.nproc ;
	    rc = m2.rc ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("scall_sysmisc: rc=%d\n",rc) ;
#endif

	} else
	    rs = SR_INVALID ;

bad:
	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (scall_sysmisc) */


#ifdef	COMMENT

static int scall_testint(bip,ip,cip,ap)
BUILTIN		*bip ;
struct ipc	*ip ;
CLIENTINFO	*cip ;
struct ipcmsg_testint	*ap ;
{
	PROGINFO	*pip = bip->pip ;
	SERIALBUF	reqbuf ;
	int		rs ;
	int		rlen, len ;
	int		rc ;
	char		buf[MSGBUFLEN + 1] ;
	char		ch ;


	serialbuf_start(&reqbuf,(char *) buf,BUFLEN) ;

	serialbuf_wchar(&reqbuf,100) ;

	serialbuf_wlong(&reqbuf,cip->mtype) ;

	serialbuf_wstrn(&reqbuf,ap->sp,MIN(ap->sl,100)) ;


	rlen = serialbuf_finish(&reqbuf) ;

/* send it to the super-server */

	rs = ipc_send(ip,buf,rlen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("scall_testint: ipc_send() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad ;

/* wait for response */

	rs = ipc_recv(ip,buf,MSGBUFLEN) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("scall_testint: ipc_recv() rs=%d\n",rs) ;
#endif

	len = rs ;
	if (rs < 0)
	    goto bad ;

/* parse the response */

	serialbuf_start(&reqbuf,buf,len) ;


	serialbuf_rchar(&reqbuf,&ch) ;

	rc = (int) ((signed char) ch) ;

	serialbuf_finish(&reqbuf) ;

bad:
	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (scall_testint) */

#endif /* COMMENT */


static uint mknettime(daytime)
time_t	daytime ;
{
	uint	nettime = (((365 * 70) + 17) * 86400U) ;


	nettime += ((uint) daytime) ;
	return nettime ;
}
/* end subroutine (mknettime) */


