/* msu-adjunct */

/* MSU adjunct subroutines */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_IPCPID	0		/* do not do this stuff */


/* revision history:

	= 2011-01-25, David A­D­ Morano
	This code was separated out from the main code (in 'pcsmain.c') due to
	conflicts over including different versions of the system socket
	structures.

*/

/* Copyright © 2004,2005,2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is adjunct code to the main MSU program.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<sys/msg.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stropts.h>
#include	<poll.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<logfile.h>
#include	<msfile.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#include	"msumain.h"
#include	"msulocinfo.h"
#include	"msulog.h"
#include	"defs.h"
#include	"msumsg.h"
#include	"shio.h"


/* local defines */

#ifndef	PROGINFO
#define	PROGINFO	PROGINFO
#endif

#define	IPCMSGINFO	struct ipcmsginfo

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	PBUFLEN
#define	PBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	IPCBUFLEN
#define	IPCBUFLEN	MSGBUFLEN
#endif

#ifndef	CMSGBUFLEN
#define	CMSGBUFLEN	256
#endif

#define	NIOVECS		1

#define	DEBUGFNAME	"/tmp/msu.deb"

#ifndef	TTYFNAME
#define	TTYFNAME	"/dev/tty"
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	listenusd(const char *,mode_t,int) ;
extern int	isNotValid(int) ;
extern int	isBadMsg(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */

union conmsg {
	struct cmsghdr	cm ;
	char		cmbuf[CMSGBUFLEN + 1] ;
} ;

struct ipcmsginfo {
	MSGHDR	ipcmsg ;
	struct iovec	vecs[NIOVECS] ;
	union conmsg	ipcconbuf ;
	SOCKADDRESS	ipcfrom ;
	int		ipcmsglen ;
	int		ns ;
	char		ipcbuf[IPCBUFLEN + 1] ;
} ;


/* forward references */

static int	procipcreqother(PROGINFO *,int) ;
static int	procipcreqmsg(PROGINFO *,int,MSFILE_ENT *) ;
static int	procipcreqmsger(PROGINFO *,IPCMSGINFO *,MSFILE_ENT *,uint) ;

static int	procipcreqmsg_getstatus(PROGINFO *,IPCMSGINFO *) ;
static int	procipcreqmsg_getsysmisc(PROGINFO *,IPCMSGINFO *,MSFILE_ENT *) ;
static int	procipcreqmsg_exit(PROGINFO *,IPCMSGINFO *) ;
static int	procipcreqmsg_mark(PROGINFO *,IPCMSGINFO *) ;
static int	procipcreqmsg_report(PROGINFO *,IPCMSGINFO *) ;
static int	procipcreqmsg_invalid(PROGINFO *,IPCMSGINFO *) ;

static int	ipcmsginfo_init(struct ipcmsginfo *) ;


/* local variables */


/* exported subroutines */


int procipcbegin(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("msuadj/procipcbegin: f_reuseaddr=%u\n",
	        lip->f.reuseaddr) ;
#endif

	lip->rfd = -1 ;
	if (lip->f.listen)  {

	    if (lip->reqfname == NULL) {
	        rs = locinfo_reqfname(lip) ;
	    }

/* careful to not destroy variable 'rs' until conditional branch below */

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("msuadj/procipcbegin: reqfname=%s\n",
	            lip->reqfname) ;
#endif

	    if (pip->debuglevel > 0) {
	        shio_printf(pip->efp,"%s: req=%s\n",
	            pip->progname,lip->reqfname) ;
	    }

	    if (pip->open.logprog)
	        logprintf(pip,"req=%s",lip->reqfname) ;

	    if (rs >= 0) {
	        const mode_t	om = 0666 ;
	        int	opts = 0 ;
	        if (lip->f.reuseaddr) opts |= 1 ;
	        if ((rs = listenusd(lip->reqfname,om,opts)) >= 0) {
	            int	fd = rs ;
	            if ((rs = uc_closeonexec(fd,TRUE)) >= 0) {
	                lip->rfd = fd ;
	                lip->open.listen = TRUE ;
	            }
	            if (rs < 0)
	                u_close(fd) ;
	        } /* end if (listenusd) */
	    } /* end if (ok) */

	} /* end if (enabled) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("msuadj/procipcbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procipcbegin) */


int procipcend(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->reqfname != NULL) {
	    if (lip->reqfname[0] != '\0') {
	        u_unlink(lip->reqfname) ;
	    }
	}

#if	CF_IPCPID
	rs1 = locinfo_ipcpid(lip,FALSE) ;
	if (rs >= 0) rs = rs1 ;
#endif

	if (lip->rfd >= 0) {
	    rs1 = u_close(lip->rfd) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->rfd = -1 ;
	}

	lip->open.listen = FALSE ;
	return rs ;
}
/* end subroutine (procipcend) */


int procipcreq(PROGINFO *pip,int re,MSFILE_ENT *mep)
{
	int		rs = SR_OK ;

	if (re != 0) {
	    if ((re & POLLIN) || (re & POLLPRI)) {
	        rs = procipcreqmsg(pip,re,mep) ;
	    } else {
	        rs = procipcreqother(pip,re) ;
	    }
	} /* end if (events) */

	return rs ;
}
/* end subroutine (procipcreq) */


/* local subroutines */


static int procipcreqother(PROGINFO *pip,int re)
{
	int		rs = SR_OK ;
	int		f_logged = FALSE ;
	const char	*ccp = NULL ;

	if (re & POLLHUP) {
	    ccp = "hangup" ;
	    rs = SR_HANGUP ;
	} else if (re & POLLERR) {
	    ccp = "error" ;
	    rs = SR_POLLERR ;
	} else if (re & POLLNVAL) {
	    ccp = "invalid" ;
	    rs = SR_NOTOPEN ;
	}

	if ((rs < 0) && (ccp != NULL) && pip->open.logprog) {
	    f_logged = TRUE ;
	    logfile_printf(&pip->lh,"%s: IPC port condition=%s",
	        pip->progname,ccp) ;
	}

	return (rs >= 0) ? f_logged : rs ;
}
/* end subroutine (procipcreqother) */


static int procipcreqmsg(PROGINFO *pip,int re,MSFILE_ENT *mep)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		f_logged = FALSE ;

	if ((re & POLLIN) || (re & POLLPRI)) {
	    IPCMSGINFO	mi, *mip = &mi ;
	    MSGHDR	*mp ;
	    const int	fdlen = sizeof(int) ;
	    int		*ip = NULL ;

	    ipcmsginfo_init(mip) ;

	    mp = &mip->ipcmsg ;
	    if ((rs = u_recvmsg(lip->rfd,mp,0)) >= 0) {
	        uint	mtype = UINT_MAX ;

	        mip->ipcmsglen = rs ;
	        if (mip->ipcmsglen > 0) {
		    mip->ipcmsglen = MKCHAR(mip->ipcbuf[0]) ;
		}

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progwatch/pollipc: mtype=%u\n",mtype) ;
#endif

/* check if we were passed a FD */

	        if (mp->msg_controllen > 0) {
	            struct cmsghdr	*cmp = CMSG_FIRSTHDR(mp) ;
	            while (cmp != NULL) {

	                ip = (int *) CMSG_DATA(cmp) ;
	                if ((cmp->cmsg_level == SOL_SOCKET) && 
	                    (cmp->cmsg_len == CMSG_LEN(fdlen)) &&
	                    (cmp->cmsg_type == SCM_RIGHTS) && (ip != NULL)) {

	                    if ((mip->ns < 0) && (mtype == UINT_MAX)) {
	                        mip->ns = *ip ;
	                    } else {
	                        u_close(*ip) ;
	                    }

	                } /* end if */
	                cmp = CMSG_NXTHDR(mp,cmp) ;

	            } /* end while */
	        } /* end if (had a control-part) */

/* handle the messages (according to our type-code) */

	        rs = procipcreqmsger(pip,mip,mep,mtype) ;

	        if (mip->ns >= 0) {
	            u_close(mip->ns) ;
	            mip->ns = -1 ;
	        }

	    } /* end if (u_recvmsg) */

	} /* end if (have some poll ready) */

	return (rs >= 0) ? f_logged : rs ;
}
/* end subroutine (procipcreqmsg) */


static int procipcreqmsger(PROGINFO *pip,IPCMSGINFO *mip,MSFILE_ENT *mep,
		uint mt)
{
	int		rs = SR_OK ;
	int		f_logged = FALSE ;
	switch (mt) {
	case msumsgtype_getstatus:
	    rs = procipcreqmsg_getstatus(pip,mip) ;
	    break ;
	case msumsgtype_getsysmisc:
	    rs = procipcreqmsg_getsysmisc(pip,mip,mep) ;
	    break ;
	case msumsgtype_exit:
	    rs = procipcreqmsg_exit(pip,mip) ;
	    break ;
	case msumsgtype_mark:
	    f_logged = TRUE ;
	    rs = procipcreqmsg_mark(pip,mip) ;
	    break ;
	case msumsgtype_report:
	    f_logged = TRUE ;
	    rs = procipcreqmsg_report(pip,mip) ;
	    break ;
	default:
	    f_logged = TRUE ;
	    rs = procipcreqmsg_invalid(pip,mip) ;
	    break ;
	} /* end switch */
	return (rs >= 0) ? f_logged : rs ;
}
/* end subroutine (procipcreqmsger) */


static int procipcreqmsg_getstatus(PROGINFO *pip,struct ipcmsginfo *mip)
{
	struct msumsg_status	m0 ;
	struct msumsg_getstatus	m1 ;
	LOCINFO		*lip = pip->lip ;
	const int	ipclen = IPCBUFLEN ;
	int		rs ;

	if ((rs = msumsg_getstatus(&m1,1,mip->ipcbuf,ipclen)) >= 0) {

#ifdef	OPTIONAL
	    memset(&m0,0,sizeof(struct msumsg_status)) ;
#endif

	    m0.tag = m1.tag ;
	    m0.pid = pip->pid ;
	    m0.rc = msumsgrc_ok ;

	    if ((rs = msumsg_status(&m0,0,mip->ipcbuf,ipclen)) >= 0) {
	        int	blen = rs ;

	        mip->ipcmsg.msg_control = NULL ;
	        mip->ipcmsg.msg_controllen = 0 ;
	        mip->vecs[0].iov_len = blen ;
	        rs = u_sendmsg(lip->rfd,&mip->ipcmsg,0) ;

	    } /* end if */

	} else if (isBadMsg(rs)) {
	    rs = SR_OK ;
	} /* end if (msumsg_getstatus) */

	return rs ;
}
/* end subroutine (procipcreqmsg_getstatus) */


static int procipcreqmsg_exit(PROGINFO *pip,struct ipcmsginfo *mip)
{
	struct msumsg_status	m0 ;
	struct msumsg_exit	m3 ;
	LOCINFO		*lip = pip->lip ;
	const int	ipclen = IPCBUFLEN ;
	int		rs ;

	if ((rs = msumsg_exit(&m3,1,mip->ipcbuf,ipclen)) >= 0) {
	    if ((rs = locinfo_reqexit(lip,"client")) >= 0) {

/* response */

	        m0.tag = m3.tag ;
	        m0.pid = pip->pid ;
	        m0.rc = msumsgrc_ok ;

	        if ((rs = msumsg_status(&m0,0,mip->ipcbuf,ipclen)) >= 0) {
	            int	blen = rs ;
	            mip->ipcmsg.msg_control = NULL ;
	            mip->ipcmsg.msg_controllen = 0 ;
	            mip->vecs[0].iov_len = blen ;
	            rs = u_sendmsg(lip->rfd,&mip->ipcmsg,0) ;
	        } /* end if */

	    } /* end if (locinfo_reqexit) */
	} else if (isBadMsg(rs)) {
	    rs = SR_OK ;
	} /* end if (msumsg_exit) */

	return rs ;
}
/* end subroutine (procipcreqmsg_exit) */


static int procipcreqmsg_mark(PROGINFO *pip,struct ipcmsginfo *mip)
{
	struct msumsg_status	m0 ;
	struct msumsg_mark	m4 ;
	LOCINFO		*lip = pip->lip ;
	long		lw ;
	const int	ipclen = IPCBUFLEN ;
	int		rs ;

	if ((rs = msumsg_mark(&m4,1,mip->ipcbuf,ipclen)) >= 0) {

/* this is the action */

	    lw = labs(pip->intrun - (pip->daytime - lip->ti_start)) ;
	    logmark(pip,lw) ;

/* response */

	    m0.tag = m4.tag ;
	    m0.pid = pip->pid ;
	    m0.rc = msumsgrc_ok ;

	    if ((rs = msumsg_status(&m0,0,mip->ipcbuf,ipclen)) >= 0) {
	        int	blen = rs ;

	        mip->ipcmsg.msg_control = NULL ;
	        mip->ipcmsg.msg_controllen = 0 ;
	        mip->vecs[0].iov_len = blen ;
	        rs = u_sendmsg(lip->rfd,&mip->ipcmsg,0) ;

	    } /* end if */

	} else if (isBadMsg(rs)) {
	    rs = SR_OK ;
	} /* end if (msumsg_mark) */

	return rs ;
}
/* end subroutine (procipcreqmsg_mark) */


static int procipcreqmsg_report(PROGINFO *pip,struct ipcmsginfo *mip)
{
	struct msumsg_status	m0 ;
	struct msumsg_report	m5 ;
	LOCINFO		*lip = pip->lip ;
	const int	ipclen = IPCBUFLEN ;
	int		rs ;

	if ((rs = msumsg_report(&m5,1,mip->ipcbuf,ipclen)) >= 0) {

/* this is the action */

	    logreport(pip) ;

/* response */

	    m0.tag = m5.tag ;
	    m0.pid = pip->pid ;
	    m0.rc = msumsgrc_ok ;

	    if ((rs = msumsg_status(&m0,0,mip->ipcbuf,ipclen)) >= 0) {
	        int	blen = rs ;

	        mip->ipcmsg.msg_control = NULL ;
	        mip->ipcmsg.msg_controllen = 0 ;
	        mip->vecs[0].iov_len = blen ;
	        rs = u_sendmsg(lip->rfd,&mip->ipcmsg,0) ;

	    } /* end if */

	} else if (isBadMsg(rs)) {
	    rs = SR_OK ;
	} /* end if (msumsg_report) */

	return rs ;
}
/* end subroutine (procipcreqmsg_report) */


static int procipcreqmsg_getsysmisc(pip,mip,mep)
PROGINFO		*pip ;
struct ipcmsginfo	*mip ;
MSFILE_ENT		*mep ;
{
	struct msumsg_sysmisc		m6 ;
	struct msumsg_getsysmisc	m2 ;
	LOCINFO		*lip = pip->lip ;
	const int	ipclen = IPCBUFLEN ;
	int		rs ;

	if ((rs = msumsg_getsysmisc(&m2,1,mip->ipcbuf,ipclen)) >= 0) {
	    int		i ;

	    memset(&m6,0,sizeof(struct msumsg_sysmisc)) ;
	    m6.tag = m2.tag ;
	    m6.pid = pip->pid ;
	    m6.utime = mep->utime ;
	    m6.btime = mep->btime ;
	    m6.ncpu = mep->ncpu ;
	    m6.nproc = mep->nproc ;
	    for (i = 0 ; i < 3 ; i += 1) {
	        m6.la[i] = mep->la[i] ;
	    }
	    m6.rc = msumsgrc_ok ;

	    if ((rs = msumsg_sysmisc(&m6,0,mip->ipcbuf,ipclen)) >= 0) {
	        int	blen = rs ;

	        mip->ipcmsg.msg_control = NULL ;
	        mip->ipcmsg.msg_controllen = 0 ;
	        mip->vecs[0].iov_len = blen ;
	        rs = u_sendmsg(lip->rfd,&mip->ipcmsg,0) ;

	    } /* end if */

	} else if (isBadMsg(rs)) {
	    rs = SR_OK ;
	} /* end if (msumsg_getsysmisc) */

	return rs ;
}
/* end subroutine (procipcreqmsg_getsysmisc) */


static int procipcreqmsg_invalid(pip,mip)
PROGINFO		*pip ;
struct ipcmsginfo	*mip ;
{
	struct msumsg_status	m0 ;
	LOCINFO		*lip = pip->lip ;
	const int	ipclen = IPCBUFLEN ;
	int		rs ;

/* response */

	memset(&m0,0,sizeof(struct msumsg_status)) ;
	m0.pid = pip->pid ;
	m0.tag = 0 ;
	m0.rc = msumsgrc_invalid ;

	if ((rs = msumsg_status(&m0,0,mip->ipcbuf,ipclen)) >= 0) {
	    int	blen = rs ;

	    mip->ipcmsg.msg_control = NULL ;
	    mip->ipcmsg.msg_controllen = 0 ;
	    mip->vecs[0].iov_len = blen ;
	    rs = u_sendmsg(lip->rfd,&mip->ipcmsg,0) ;

	} /* end if */

	return rs ;
}
/* end subroutine (procipcreqmsg_invalid) */


static int ipcmsginfo_init(mip)
struct ipcmsginfo	*mip ;
{
	int		size ;

#ifdef	OPTIONAL
	memset(mip,0,sizeof(struct ipcmsginfo)) ;
#endif

	mip->ipcmsglen = 0 ;
	mip->ns = -1 ;
	mip->ipcbuf[0] = '\0' ;

	size = NIOVECS * sizeof(struct iovec) ;
	memset(mip->vecs,0,size) ;

	mip->vecs[0].iov_base = mip->ipcbuf ;
	mip->vecs[0].iov_len = IPCBUFLEN ;

	memset(&mip->ipcmsg,0,sizeof(MSGHDR)) ;

	mip->ipcmsg.msg_name = &mip->ipcfrom ;
	mip->ipcmsg.msg_namelen = sizeof(SOCKADDRESS) ;
	mip->ipcmsg.msg_iov = mip->vecs ;
	mip->ipcmsg.msg_iovlen = NIOVECS ;
	mip->ipcmsg.msg_control = &mip->ipcconbuf ;
	mip->ipcmsg.msg_controllen = CMSGBUFLEN ;

	return SR_OK ;
}
/* end subroutine (ipcmsginfo_init) */


