/* standing */

/* this is the standing server code that is part of the main server program */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_MSFILE	1		/* enable MS handling */
#define	CF_CPUSPEED	1		/* get CPU speed */


/* revision history:

	= 1999-07-09, David A­D­ Morano
	This program was originally written.

	= 2002-10-31, David A­D­ Morano
        Solaris-8 has been out for more than a year and a half now. They added a
        cheapo way to get system load averages without having to use KSTAT. I
        finally updated this code to try to use the cheapo way to get load
        averages when we can.

*/

/* Copyright © 1999,2002 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

        This object is a helper object for the TCPMUXD program. This object is
        used to service certain types of requests from spawned servers (usually
        only the built-in servers) when some standing state associated with the
        request is either warranted or otherwise desireable.

	Extra notes for the observant:

        You might note that the KINFO object is NOT reentrant or thread safe.
        Yes, its true, they are not! Blame Sun Microsystems for that if you
        want.

        For that reason, we are careful to only have one thread ever interact
        with the KINFO object. That thread is mostly what executes in this code
        module with the execution (as far as I know) of the
        'standing_readdata()' subroutine. That subroutine is not called by other
        threads but by other entire processes! That subroutine doesn't touch the
        KINFO object stuff so we should be OK. The other cooperating processes
        interact with this thread through the internal message system
        ('muximsg').


***************************************************************************/


#define	STANDING_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<string.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<hostent.h>
#include	<sockaddress.h>
#include	<inetaddr.h>
#include	<serialbuf.h>
#include	<msfile.h>
#include	<msflag.h>
#include	<utmpacc.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"standing.h"
#include	"muximsg.h"
#include	"sysmisc.h"
#include	"listenspec.h"


/* local defines */

#define	O_MSOPEN	(O_RDWR | O_CREAT)

#define	TO_MININT	(5 * 60)
#define	TO_SYSMISC	5		/* time between system_misc gets */
#define	TO_LOADAVE	3		/* time between LOADAVE gets */
#define	TO_AFSCLOSE	(5 * 60)	/* time until an AF-socket close */
#define	TO_CHECKSPEED	(2 * 60)

#ifndef	TO_SPEED
#define	TO_SPEED	30
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	isNotPresent(int) ;
extern int	isBadMsg(int) ;

#if	CF_CPUSPEED
extern int	cpuspeed(const char *,const char *,int) ;
#endif

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */

enum cotypes {
	cotype_reploadave,
	cotype_overlast

} ;


/* forward references */

#ifdef	COMMENT
static int standing_boottime(STANDING *,int,char *,char *) ;
#endif

static int standing_undefined(STANDING *,time_t,char *,char *) ;
static int standing_getsysmisc(STANDING *,time_t,char *,char *) ;
static int standing_getloadave(STANDING *,time_t,char *,char *) ;
static int standing_reploadave(STANDING *,time_t,char *,char *) ;
static int standing_getlistener(STANDING *,time_t,char *,char *) ;

static int standing_checkopen(STANDING *,time_t,int) ;
static int standing_checkclose(STANDING *,time_t) ;
static int standing_cosend(STANDING *,time_t,struct standing_callout *,int) ;
static int standing_afsendto(STANDING *,time_t,int,const char *,int,
		SOCKADDRESS *,int) ;

static int standing_maintsysmisc(STANDING *,time_t) ;
static int standing_maintloadave(STANDING *,time_t) ;

static int standing_handlems(STANDING *,time_t) ;
static int standing_checkspeed(STANDING *,time_t) ;

static int	callout_start(struct standing_callout *) ;
static int	callout_finish(struct standing_callout *) ;

static int	afs_checkclose(struct standing_afsocket *,time_t) ;

#ifdef	COMMENT
static int	afs_close(struct standing_afsocket *) ;
#endif


/* local variables */


/* exported subroutines */


/* initialize this object */
int standing_start(sop,pip)
STANDING	*sop ;
PROGINFO	*pip ;
{
	int		rs ;
	int		rs1 ;

	if (sop == NULL) return SR_FAULT ;

	memset(sop,0,sizeof(STANDING)) ;
	sop->pip = pip ;

	rs = vecitem_start(&sop->callouts,10,0) ;
	if (rs < 0)
	    goto bad0 ;

	sop->afs_unix.fd = -1 ;
	sop->afs_inet4.fd = -1 ;
	sop->afs_inet6.fd = -1 ;

	sop->mininterval = TO_MININT ;

/* other stuff */

#if	CF_MSFILE
	if (pip->f.mspoll) {
	        sop->m.pagesize = getpagesize() ;

	        memset(&sop->m.e,0,sizeof(MSFILE_ENT)) ;

	        strwcpy(sop->m.e.nodename,pip->nodename,MSFILE_NODENAMELEN) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("standing_start: msfile_open() msfname=%s\n",
	                pip->msfname) ;
#endif

	        rs1 = msfile_open(&sop->ms,pip->msfname,O_MSOPEN,0666) ;
	        sop->f.msopen = (rs1 >= 0) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("standing_start: msfile_open() rs=%d\n",rs1) ;
#endif

	        if (rs1 == 0)
	            u_chmod(pip->msfname,0666) ;

	        if (sop->f.msopen) {

	            rs1 = msfile_match(&sop->ms,pip->daytime,
	                pip->nodename,-1, &sop->m.e) ;

#if	CF_CPUSPEED && 0
	            standing_checkspeed(sop,pip->daytime) ;
#endif

	            standing_handlems(sop,pip->daytime) ;

	        } /* end if (opened MS file) */

	} /* end if (MS) */
#endif /* CF_MSFILE */

/* we're done */
done:
	return rs ;

/* bad things */
bad0:
	goto done ;
}
/* end subroutine (standing_start) */


int standing_finish(sop)
STANDING	*sop ;
{
	PROGINFO	*pip ;
	struct standing_callout	*cop ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		rc = sysmiscrc_goingdown ;

	if (sop == NULL) return SR_FAULT ;

	pip = sop->pip ;

/* close extra stuff */

	if (sop->f.msopen) {
	    rs1 = msfile_close(&sop->ms) ;
	    if (rs >= 0) rs = rs1 ;
	}

/* loop through callout list */

	for (i = 0 ; vecitem_get(&sop->callouts,i,&cop) >= 0 ; i += 1) {
	    if (cop != NULL) {
	        if (cop->expire < pip->daytime) {
	            standing_cosend(sop,pip->daytime,cop,rc) ;
	        }
	        rs1 = callout_finish(cop) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	rs1 = vecitem_finish(&sop->callouts) ;
	if (rs >= 0) rs = rs1 ;

/* clean everything up */

	if (sop->afs_unix.fd >= 0) {
	    rs1 = u_close(sop->afs_unix.fd) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sop->afs_inet4.fd >= 0) {
	    rs1 = u_close(sop->afs_inet4.fd) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sop->afs_inet6.fd >= 0) {
	    rs1 = u_close(sop->afs_inet6.fd) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (standing_finish) */


/* handle an internal request */
int standing_request(sop,daytime,rcode,ipcbuf,resbuf)
STANDING	*sop ;
time_t		daytime ;
int		rcode ;
char		ipcbuf[] ;
char		resbuf[] ;
{
	PROGINFO	*pip ;
	int		rs = SR_INVALID ;
	int		len = 0 ;

	if (sop == NULL) return SR_FAULT ;
	pip = sop->pip ;
	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("standing_request: ent rcode=%d\n",rcode) ;
#endif

	if (rcode < muximsgtype_overlast) {
	    switch (rcode) {
	    case muximsgtype_getsysmisc:
	        rs = standing_getsysmisc(sop,daytime,ipcbuf,resbuf) ;
	        break ;
	    case muximsgtype_getloadave:
	        rs = standing_getloadave(sop,daytime,ipcbuf,resbuf) ;
	        break ;
	    case muximsgtype_reploadave:
	        rs = standing_reploadave(sop,daytime,ipcbuf,resbuf) ;
	        break ;
	    case muximsgtype_getlistener:
	        rs = standing_getlistener(sop,daytime,ipcbuf,resbuf) ;
	        break ;
	    } /* end switch */
	} else {
	    rs = standing_undefined(sop,daytime,ipcbuf,resbuf) ;
	}

	len = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("standing_request: ret rs=%d u=%u\n",
	        rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (standing_request) */


/* check the state for currency */
int standing_check(sop,daytime)
STANDING	*sop ;
time_t		daytime ;
{
	PROGINFO	*pip ;
	struct standing_callout	*cop ;
	int		rs = SR_OK ;
	int		i ;
	int		mininterval = 0 ;
	int		f_del = FALSE ;

#if	CF_DEBUG || CF_DEBUGS
	char		timebuf[TIMEBUFLEN + 1] ;
#endif

	if (sop == NULL) return SR_FAULT ;

	pip = sop->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("standing_check: ent nactive=%d\n",
	        sop->nactive) ;
#endif

/* run through the list of things to poll */

/* update the interval if necessary (this is a flagged maintenance thing) */

	if (sop->f.interval) {
	    sop->f.interval = FALSE ;
	    mininterval = sop->mininterval ;
	}

/* handle MS stuff if necessary */

	if (sop->f.msopen) {

	    if ((mininterval == 0) || (mininterval > pip->intpoll))
	        mininterval = pip->intpoll ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("standing_check: standing_handlems()\n") ;
#endif

	    rs = standing_handlems(sop,daytime) ;

	} /* end if */

	if (rs < 0)
	    goto ret0 ;

/* following are things that need clients (get out if no clients) */

	if (sop->nactive <= 0)
	    goto ret0 ;

/* check on the callouts */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("standing_check: checking callouts\n") ;
#endif

	for (i = 0 ; vecitem_get(&sop->callouts,i,&cop) >= 0 ; i += 1) {

	    if (cop == NULL) continue ;

	    if (cop->next <= daytime) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("standing_check: need cosend callout=%d\n", i) ;
	            debugprintf("standing_check: curent time=%s\n",
	                timestr_log(daytime,timebuf)) ;
	        }
#endif /* CF_DEBUG */

	        standing_cosend(sop,daytime,cop,sysmiscrc_ok) ;

	        cop->next += cop->interval ;
	        if (cop->next > cop->expire) {

	            callout_finish(cop) ;

	            f_del = TRUE ;
	            sop->nactive -= 1 ;
	            vecitem_del(&sop->callouts,i) ;

	        } /* end if (callout expiration) */

	    } /* end if (callout activated) */

	} /* end for */

/* if we had a deletion, recalculate the minimum check interval */

	if (f_del) {

	    mininterval = (5 * 60) ;
	    for (i = 0 ; vecitem_get(&sop->callouts,i,&cop) >= 0 ; i += 1) {
	        if (cop != NULL) {
	            if (cop->interval < mininterval) {
	                mininterval = cop->interval ;
		    }
	        }
	    } /* end for */
	    sop->mininterval = mininterval ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("standing_check: new mininterval=%d\n",
	            mininterval) ;
#endif

	} /* end if (minimum interval calculation) */

/* close out any old AF-sockets */

	standing_checkclose(sop,daytime) ;

/* we're out of here */
ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("standing_check: ret rs=%d mininterval=%d\n",
	        rs,mininterval) ;
#endif

	return (rs >= 0) ? mininterval : rs ;
}
/* end subroutine (standing_check) */


/* is our cached data still valid? (called by other threads!!!!) */
int standing_readdata(sop,dp)
STANDING		*sop ;
STANDING_SYSMISC	*dp ;
{
	PROGINFO	*pip ;
	time_t		dt ;
	const int	to = TO_SYSMISC ;
	int		rs = SR_OK ;

	if (sop == NULL) return SR_FAULT ;

	pip = sop->pip ;
	dt = pip->daytime ;

	sop->c.ti_access = dt ;
	if ((dt - sop->c.ti_sysmisc) > to) {
	    rs = SR_TIMEDOUT ;
	    goto ret0 ;
	}

	if (dp != NULL)
	    dp->boottime = 0 ;

	if (sop->c.d.boottime == 0) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	if (dp != NULL) {
	    *dp = sop->c.d ;
	}

ret0:
	return rs ;
}
/* end subroutine (standing_readdata) */


/* private subroutines */


static int standing_undefined(sop,daytime,ipcbuf,buf)
STANDING	*sop ;
time_t		daytime ;
char		ipcbuf[] ;
char		buf[] ;
{
	PROGINFO	*pip = sop->pip ;
	struct muximsg_unknown	mu ;
	struct muximsg_response	m0b ;
	int		rs ;
	int		blen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("standing_undefined: ent\n") ;
#endif

/* read the message we got to get its tag */
/* write back a message (w/ original tag) saying it was bad */

	if ((rs = muximsg_unknown(&mu,1,ipcbuf,MSGBUFLEN)) >= 0) {
	    m0b.pid = pip->spid ;
	    m0b.tag = 0 ;
	    m0b.rc = muximsgrc_invalid ;
	    blen = muximsg_response(&m0b,0,buf,MSGBUFLEN) ;
	}

	return (rs >= 0) ? blen : rs ;
}
/* end subroutine (standing_undefined) */


#ifdef	COMMENT

static int standing_boottime(sop,ipcbuf,buf)
STANDING	*sop ;
char		ipcbuf[] ;
char		buf[] ;
{
	PROGINFO	*pip = sop->pip ;
	time_t		ti_daytime ;
	uint		v ;
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("standing_boottime: ent, rcode=%d\n",rcode) ;
#endif

	if (sop->k.boottime == 0) {
	    ti_daytime = time(NULL) ;
	    rs = standing_maintsysmisc(sop,ti_daytime) ;
	}

	if ((rs >= 0) &&
	    ((rs = serialbuf_start(&retbuf,(char *) buf,MSGBUFLEN)) >= 0)) {

	    v = (uint) sop->k.boottime ;
	    serialbuf_wint(&retbuf,v) ;
    
	    len = serialbuf_finish(&retbuf) ;

	} /* end if */

	return len ;
}
/* end subroutine (standing_boottime) */

#endif /* COMMENT */


static int standing_getsysmisc(sop,daytime,ipcbuf,buf)
STANDING	*sop ;
time_t		daytime ;
char		ipcbuf[] ;
char		buf[] ;
{
	struct muximsg_getsysmisc	m4 ;
	struct muximsg_sysmisc		m5 ;
	time_t		ti_daytime = daytime ;
	int		rs ;
	int		blen = 0 ;

	if ((rs = standing_maintsysmisc(sop,ti_daytime)) >= 0) {
	    if ((rs = muximsg_getsysmisc(&m4,1,ipcbuf,MSGBUFLEN)) >= 0) {
	        m5.tag = m4.tag ;
	        m5.rc = (rs >= 0) ? muximsgrc_ok : muximsgrc_notavail ;
	        m5.la_1min = sop->c.d.la_1min ;
	        m5.la_5min = sop->c.d.la_5min ;
	        m5.la_15min = sop->c.d.la_15min ;
	        m5.boottime = sop->c.d.boottime ;
	        m5.nproc = sop->c.d.nproc ;
	        blen = muximsg_sysmisc(&m5,0,buf,MSGBUFLEN) ;
	    }
	}

	return (rs >= 0) ? blen : rs ;
}
/* end subroutine (standing_getsysmisc) */


static int standing_getloadave(sop,daytime,ipcbuf,buf)
STANDING	*sop ;
time_t		daytime ;
char		ipcbuf[] ;
char		buf[] ;
{
	PROGINFO	*pip = sop->pip ;
	struct muximsg_getloadave	m6 ;
	struct muximsg_loadave		m7 ;
	time_t		ti_daytime = daytime ;
	int		rs ;
	int		blen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("standing_getloadave: ent\n") ;
#endif

	if (pip == NULL) return SR_FAULT ; /* lint */

	if ((rs = standing_maintsysmisc(sop,ti_daytime)) >= 0) {
	    if ((rs = muximsg_getloadave(&m6,1,ipcbuf,MSGBUFLEN)) >= 0) {
	        m7.tag = m6.tag ;
	        m7.rc = muximsgrc_ok ;
	        m7.la_1min = sop->c.d.la_1min ;
	        m7.la_5min = sop->c.d.la_5min ;
	        m7.la_15min = sop->c.d.la_15min ;
	        blen = muximsg_loadave(&m7,0,buf,MSGBUFLEN) ;
	    }
	}

	return (rs >= 0) ? blen : rs ;
}
/* end subroutine (standing_getloadave) */


/* handle a request to repeat 'loadave' reports */
static int standing_reploadave(sop,daytime,ipcbuf,buf)
STANDING	*sop ;
time_t		daytime ;
char		ipcbuf[] ;
char		buf[] ;
{
	struct muximsg_loadave		m7 ;
	struct muximsg_reploadave	m8 ;
	struct standing_callout		co ;
	PROGINFO	*pip = sop->pip ;
	int		rs = SR_NOMEM ;
	int		size ;
	int		port ;
	int		blen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("standing_reploadave: ent\n") ;
#endif

	if (pip == NULL) return SR_FAULT ; /* lint */

/* parse the sub-server message (with the client information) */

	muximsg_reploadave(&m8,1,ipcbuf,MSGBUFLEN) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("standing_reploadave: addrfamily=%04x addrport=%04x\n",
	        m8.addrfamily,m8.addrport) ;
#endif

/* fill in the callout */

#ifdef	COMMENT
	memset(&co,0,sizeof(struct standing_callout)) ;
#else
	rs = callout_start(&co) ;
#endif

	if (rs < 0)
	    goto badavail0 ;

	co.type = cotype_reploadave ;
	co.tag = m8.utag ;
	co.duration = m8.duration ;
	co.interval = m8.interval ;
	co.next = daytime ;
	co.expire = daytime + m8.duration ;
	co.af = ntohs(m8.addrfamily) ;

	port = ntohs(m8.addrport) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("standing_reploadave: client af=%d port=%d\n",
	        co.af,port) ;
#endif

	rs = sockaddress_start(&co.sa,co.af,m8.addrhost,port,0) ;
	co.salen = rs ;
	if (rs < 0)
	    goto badavail1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("standing_reploadave: standing_checkopen()\n") ;
#endif

	rs = standing_checkopen(sop,daytime,co.af) ;
	if (rs < 0)
	    goto badavail2 ;

/* add the callout to the list */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("standing_reploadave: vecitem_add()\n") ;
#endif

	size = sizeof(struct standing_callout) ;
	rs = vecitem_add(&sop->callouts,&co,size) ;
	if (rs < 0)
	    goto badavail2 ;

	sop->nactive += 1 ;
	if (co.interval < sop->mininterval) {
	    sop->mininterval = co.interval ;
	    sop->f.interval = TRUE ;
	}

/* OK, gratuitously get updated LA values */

#ifdef	OPTIONAL
#endif /* OPTIONAL */

/* our work is done here */
ret1:
	m7.tag = m8.tag ;
	m7.rc = (rs >= 0) ? muximsgrc_ok : muximsgrc_notavail ;
	m7.la_1min = sop->c.d.la_1min ;
	m7.la_5min = sop->c.d.la_5min ;
	m7.la_15min = sop->c.d.la_15min ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("standing_reploadave: muximsgrc=%d\n",m7.rc) ;
#endif

	blen = muximsg_loadave(&m7,0,buf,BUFLEN) ;

	return (rs >= 0) ? blen : rs ;

/* bad things */
badavail2:
	sockaddress_finish(&co.sa) ;
	memset(&co.sa,0,sizeof(struct sockaddr)) ;

badavail1:
	callout_finish(&co) ;

badavail0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("standing_reploadave: badavail rs=%d\n",rs) ;
#endif

	goto ret1 ;
}
/* end subroutine (standing_reploadave) */


static int standing_getlistener(sop,daytime,ipcbuf,resbuf)
STANDING	*sop ;
time_t		daytime ;
char		ipcbuf[] ;
char		resbuf[] ;
{
	PROGINFO	*pip = sop->pip ;
	struct muximsg_getlistener	m9 ;
	struct muximsg_listener		m10 ;
	LISTENSPEC	*lsp ;
	LISTENSPEC_INFO	li ;
	int		rs ;
	int		blen = 0 ;

/* parse the sub-server message (with the client information) */

	if ((rs = muximsg_getlistener(&m9,1,ipcbuf,MSGBUFLEN)) >= 0) {
	    uint	rc = muximsgrc_notavail ;
	    int		idx = m9.idx ;

	    m10.rc = rc ;
	    m10.tag = m9.tag ;
	    m10.idx = m9.idx ;
	    m10.name[0] = '\0' ;
	    m10.addr[0] = '\0' ;

	    if ((rs = vecobj_get(&pip->listens,idx,&lsp)) >= 0) {
		if ((rs = listenspec_info(lsp,&li)) >= 0) {
		    rc = muximsgrc_ok ;
	            strwcpy(m10.name,li.type,MUXIMSG_LNAMELEN) ;
	            strwcpy(m10.addr,li.addr,MUXIMSG_LNAMELEN) ;
		} else {
		    rc = muximsgrc_error ;
		}
	    } else if (rs == SR_NOTFOUND) {
		rs = SR_OK ;
	    }

	    m10.rc = rc ;
	    blen = muximsg_listener(&m10,0,resbuf,MSGBUFLEN) ;

	} else if (isBadMsg(rs)) {
	    rs = SR_OK ;
	} /* end if */

	return (rs >= 0) ? blen : rs ;
}
/* end subroutine (standing_getlistener) */


/* check that the socket for the specified address family is open */
static int standing_checkopen(sop,daytime,af)
STANDING	*sop ;
time_t		daytime ;
int		af ;
{
	PROGINFO	*pip = sop->pip ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("standing_checkopen: af=%d\n",af) ;
#endif

	if (pip == NULL) return SR_FAULT ; /* lint */
	switch (af) {
	case AF_UNIX:
	    if (sop->afs_unix.fd < 0) {
	        if ((rs = u_socket(af,SOCK_DGRAM,0)) >= 0) {
	            sop->afs_unix.fd = rs ;
	            sop->afs_unix.lastaccess = daytime ;
	        }
	    }
	    break ;
	case AF_INET:
	    if (sop->afs_inet4.fd < 0) {
	        if ((rs = u_socket(af,SOCK_DGRAM,0)) >= 0) {
	            sop->afs_inet4.fd = rs ;
	            sop->afs_inet4.lastaccess = daytime ;
	        }
	    }
	    break ;
	case AF_INET6:
	    if (sop->afs_inet6.fd < 0) {
	        if ((rs = u_socket(af,SOCK_DGRAM,0)) >= 0) {
	            sop->afs_inet6.fd = rs ;
	            sop->afs_inet6.lastaccess = daytime ;
	        }
	    }
	    break ;
	default:
	    rs = SR_NOTSUP ;
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (standing_checkopen) */


/* check on if we can close some open sockets */
static int standing_checkclose(sop,daytime)
STANDING	*sop ;
time_t		daytime ;
{

	afs_checkclose(&sop->afs_unix,daytime) ;

	afs_checkclose(&sop->afs_inet4,daytime) ;

	afs_checkclose(&sop->afs_inet6,daytime) ;

	return SR_OK ;
}
/* end subroutine (standing_checkclose) */


/* send out a message to a client */
static int standing_cosend(sop,daytime,cop,rc)
STANDING	*sop ;
time_t		daytime ;
struct standing_callout	*cop ;
int		rc ;
{
	PROGINFO	*pip = sop->pip ;
	struct sysmisc_loadave	m6 ;
	int		rs = SR_OK ;
	int		salen ;
	int		blen = 0 ;
	char		buf[MSGBUFLEN + 1] ;

	switch (cop->type) {
	case cotype_reploadave:
	    if (rc == sysmiscrc_ok) {
	        rs = standing_maintloadave(sop,daytime) ;
	        rc = (rs >= 0) ? sysmiscrc_ok : sysmiscrc_notavail ;
	    }
	    if (rs >= 0) {
	        sop->c.ti_access = daytime ;
	        memset(&m6,0,sizeof(struct sysmisc_loadave)) ;
	        m6.tag = cop->tag ;
	        m6.rc = rc ;
	        m6.timestamp = daytime ;
	        m6.providerid = pip->providerid ;
	        m6.hostid = pip->hostid ;
	        m6.la_1min = sop->c.d.la_1min  ;
	        m6.la_5min = sop->c.d.la_5min ;
	        m6.la_15min = sop->c.d.la_15min ;
	    }
	    if (rs >= 0) {
	        rs = sysmisc_loadave(&m6,0,buf,BUFLEN) ;
	        blen = rs ;
	    }
	    if (rs >= 0) {
	        salen = cop->salen ;
	        rs = standing_afsendto(sop,daytime,cop->af,buf,blen,
	            &cop->sa,salen) ;
	    }
	    break ;
	default:
	    rs = SR_NOTSUP ;
	    break ;
	} /* end switch */

	return (rs >= 0) ? blen : rs ;
}
/* end subroutine (standing_cosend) */


static int standing_afsendto(sop,dt,af,buf,buflen,sap,salen)
STANDING	*sop ;
time_t		dt ;
int		af ;
const char	buf[] ;
int		buflen ;
SOCKADDRESS	*sap ;
int		salen ;
{
	int		rs ;
	int		fd = -1 ;

	if ((rs = standing_checkopen(sop,dt,af)) >= 0) {

	switch (af) {
	case AF_UNIX:
	    fd = sop->afs_unix.fd ;
	    break ;
	case AF_INET:
	    fd = sop->afs_inet4.fd ;
	    break ;
	case AF_INET6:
	    fd = sop->afs_inet6.fd ;
	    break ;
	default:
	    fd = -1 ;
	    rs = SR_NOTSUP ;
	} /* end switch */

	if (rs >= 0) {
	    struct sockaddr	*sabuf = (struct sockaddr *) sap ;
	    rs = u_sendto(fd,buf,buflen,0,sabuf,salen) ;
	}

	} /* end if (standing_checkopen) */

	return rs ;
}
/* end subroutine (standing_afsendto) */


/* maintenance the SYSMISC stuff */
static int standing_maintsysmisc(STANDING *sop,time_t dt)
{
	const int	to = TO_SYSMISC ;
	int		rs ;

#if	CF_DEBUG || CF_DEBUGS
	char	timebuf[TIMEBUFLEN + 1] ;
#endif

	if ((rs = standing_maintloadave(sop,dt)) >= 0) {
	    if ((sop->c.ti_sysmisc == 0) || ((dt - sop->c.ti_sysmisc) > to)) {
	        if (sop->c.d.boottime == 0) {
		    time_t	bt ;
		    rs = utmpacc_boottime(&bt) ;
		    sop->c.d.boottime = bt ;
	        }
	        if (rs >= 0) {
		    sop->c.d.nproc = 0 ;
	            if ((rs = uc_nprocs(0)) >= 0) {
		        sop->c.d.nproc = rs ;
		    } else if (isNotPresent(rs) || (rs == SR_NOSYS))
			rs = SR_OK ;
	        }
		if (rs >= 0) {
		    rs = uc_nprocessors(0) ;
		    sop->c.d.ncpu = rs ;
		}
	        if (rs >= 0) {
	            sop->c.ti_sysmisc = dt ;
	        }
	    } /* end if (getting SYSMISC) */
	} /* end if (loadavg) */

	return rs ;
}
/* end subroutine (standing_maintsysmisc) */


/* maintenance the LOADAVE stuff */
static int standing_maintloadave(STANDING *sop,time_t dt)
{
	const int	to = TO_LOADAVE ;
	int		rs = SR_OK ;

	if ((sop->c.ti_loadave == 0) || ((dt - sop->c.ti_loadave) > to)) {
	    uint	la[3] ;
	    sop->c.ti_loadave = dt ;
	    if ((rs = u_getloadavg(la,3)) >= 0) {
		sop->c.d.la_1min = la[0] ;
		sop->c.d.la_5min = la[1] ;
		sop->c.d.la_15min = la[2] ;
	    } else if (rs == SR_NOSYS)
		rs = SR_OK ;
	} /* end if (getting LOADAVE) */

	return rs ;
}
/* end subroutine (standing_maintloadave) */


/* handle the MS stuff */
static int standing_handlems(sop,daytime)
STANDING	*sop ;
time_t		daytime ;
{
	PROGINFO	*pip = sop->pip ;
	long		lw ;			/* long-word integer */
	int		rs = SR_OK ;
	int		ppm ;
	int		longint ;		/* long-time interval */
	int		f_long ;

#if	CF_DEBUG || CF_DEBUGS
	char		timebuf[TIMEBUFLEN + 1] ;
#endif

	if ((daytime - sop->m.ti_la) < pip->intpoll)
	    goto ret0 ;

/* update stuff depending on elapsed time since last update */

	longint = (pip->intpoll * 3) ;
	f_long = ((daytime - sop->m.ti_numbers) >= longint) ;

	if ((rs = standing_maintsysmisc(sop,daytime)) >= 0) {
	if (f_long || (sop->m.e.btime == 0)) {

#if	CF_CPUSPEED
	    standing_checkspeed(sop,daytime) ;
#endif

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("standing_handlems: doing long\n") ;
#endif

	    sop->m.e.btime = (uint) sop->c.d.boottime ;
	    sop->m.e.ncpu = sop->c.d.ncpu ;
	    sop->m.e.nproc = sop->c.d.nproc ;

	    sop->m.ti_numbers = daytime ;

	} /* end if (long) */
	sop->m.e.la[0] = sop->c.d.la_1min ;
	sop->m.e.la[1] = sop->c.d.la_5min ;
	sop->m.e.la[2] = sop->c.d.la_15min  ;
	} /* end if (standing_maintsysmisc) */

/* other light-weight stuff */

/* calculate pages-per-megabyte */

	ppm = (1024 * 1024) / sop->m.pagesize ;

/* OK, now calculate the megabytes of each type of memory */

	if (rs >= 0)
	    rs = uc_sysconf(_SC_PHYS_PAGES,&lw) ;

	sop->m.e.pmtotal = 1 ;
	if ((rs >= 0) && (ppm > 0))
	    sop->m.e.pmtotal = (lw / ppm) ;

	if (rs >= 0)
	    rs = uc_sysconf(_SC_AVPHYS_PAGES,&lw) ;

	sop->m.e.pmavail = 1 ;
	if ((rs >= 0) && (ppm > 0))
	    sop->m.e.pmavail = (lw / ppm) ;

/* as of 2002-01-01 new policy says: */
/* we need to participate in "count-down" */
/* updated note: count-down occurs in 'msfile_update()' when 'dtime=0' */
/* but ...: we will continue to do it ourselves since it's more efficient */

#if	defined(OPTIONAL) || 1
	if ((sop->m.e.dtime != 0) && (daytime >= sop->m.e.dtime)) {
	    sop->m.e.dtime = 0 ;
	    sop->m.e.flags &= (~ MSFLAG_MDISABLED) ;
	}
#else
	sop->m.e.dtime = 0
#endif

/* finish off with time stamps */

	sop->m.e.utime = (uint) daytime ;

/* update the MS entry */

	if (rs >= 0) {
	    rs = msfile_update(&sop->ms,daytime,&sop->m.e) ;
	}

/* update the time stamp */

	sop->m.ti_la = daytime ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("standing_handlems: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (standing_handlems) */


static int standing_checkspeed(sop,daytime)
STANDING	*sop ;
time_t		daytime ;
{
	PROGINFO	*pip = sop->pip ;
	int		rs = SR_OK ;
	int		rs1 = 0 ;

	if ((daytime - sop->m.ti_checkspeed) < TO_CHECKSPEED)
	    goto ret0 ;

#if	CF_CPUSPEED

#ifdef	COMMENT
	rs = msfile_match(&sop->ms,daytime,pip->nodename,-1,&sop->m.e) ;
#endif

	if ((rs == SR_NOTFOUND) || (sop->m.e.speed == 0) ||
	    ((daytime - sop->m.e.stime) >= TO_SPEED)) {

	    rs1 = cpuspeed(pip->pr,pip->speedname,0) ;

	    sop->m.e.speed = (rs1 >= 0) ? rs1 : 0 ;
	    sop->m.e.stime = daytime ;

	}

	sop->m.ti_checkspeed = daytime ;
#else
	rs = SR_NOTSUP ;
#endif /* CF_CPUSPEED */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("standing_checkspeed: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (standing_checkspeed) */


/* CALLOUT stuff */


static int callout_start(cop)
struct standing_callout	*cop ;
{

	memset(cop,0,sizeof(struct standing_callout)) ;

	return 0 ;
}
/* end subroutine (callout_start) */


static int callout_finish(cop)
struct standing_callout	*cop ;
{
	const int	salen = sizeof(struct sockaddr) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		f = FALSE ;
	const char	*ccp = (const char *) &cop->sa ;

	f = FALSE ;
	for (i = 0 ; i < salen ; i += 1) {
	    f = (ccp[i] != '\0') ;
	    if (f) break ;
	}
	if (f) {
	    rs1 = sockaddress_finish(&cop->sa) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (callout_finish) */


/* AFS object methods */
static int afs_checkclose(afsp,daytime)
struct standing_afsocket	*afsp ;
time_t				daytime ;
{
	const int	to = TO_AFSCLOSE ;
	int		rs = SR_OK ;

	if ((afsp->fd >= 0) && ((daytime - afsp->lastaccess) >= to)) {
	    rs = u_close(afsp->fd) ;
	    afsp->fd = -1 ;
	}

	return rs ;
}
/* end subroutine (afs_checkclose) */


#ifdef	COMMENT

/* unconditionally close this AF-socket */
static int afs_close(afsp)
struct standing_afsocket	*afsp ;
{
	int		rs = SR_OK ;

	if (afsp->fd >= 0) {
	    rs = u_close(afsp->fd) ;
	    afsp->fd = -1 ;
	}

	return rs ;
}
/* end subroutine (afs_close) */

#endif /* COMMENT */


