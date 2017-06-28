/* proginput */

/* process messages on the input stream */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGN	0		/* special */


/* revision history:

	= 2001-03-01, David A­D­ Morano
	The subroutine was adapted from other programs that do similar things.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine processes messages that are present on the input stream.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	<vsystem.h>
#include	<logfile.h>
#include	<dater.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"pingstatdb.h"
#include	"pingstatmsg.h"
#include	"msgbuf.h"


/* local defines */

#define	MSGBUFLEN	2048
#define	INBUFLEN	(2 * 1024)

#ifndef	LONGTIME
#define	LONGTIME	(5 * 60)
#endif

#ifndef	TO_READ
#define	TO_READ		1
#endif

#define	DEBFNAME	"/tmp/pingstat.deb"


/* external subroutines */

extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t, char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	procstream(PROGINFO *,int) ;
static int	procdatagram(PROGINFO *,int) ;
static int	procupdate(PROGINFO *,struct pingstatmsg_update *) ;
static int	procuptime(PROGINFO *,struct pingstatmsg_uptime *) ;
static int	procentry(PROGINFO *,const char *,PINGSTATDB_UP *) ;


/* local variables */


/* exported subroutines */


int proginput(PROGINFO *pip,int fd)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("proginput: type update=%u\n",
	        pingstatmsgtype_update) ;
	    debugprintf("proginput: type uptime=%u\n",
	        pingstatmsgtype_uptime) ;
	}
#endif /* CF_DEBUG */

	if (pip->f.dgram) {
	    rs = procdatagram(pip,fd) ;
	} else
	    rs = procstream(pip,fd) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("proginput: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (proginput) */


/* local subroutines */


int procstream(PROGINFO *pip,int fd)
{
	struct pingstatmsg_update	m0 ;
	struct pingstatmsg_uptime	m1 ;
	struct pingstatmsg_unknown	mu ;
	MSGBUF		mb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		to = TO_READ ;
	int		ti_start = pip->daytime ;
	int		msgtype ;
	int		size = 0 ;
	int		s ;
	int		n = 0 ;
	int		mlen ;
	int		loopcount = 0 ;

	if (pip->intmininput > 0) to = pip->intmininput ;

	if (pip->open.logprog && pip->f.logextra)
	    logfile_printf(&pip->lh,"read to=%u",to) ;

/* find maximum message size (or machine pagesize) */

	s = pip->pagesize ;
	if (s > size) size = s ;
	s = sizeof(struct pingstatmsg_uptime) ;
	if (s > size) size = s ;
	s = sizeof(struct pingstatmsg_update) ;
	if (s > size) size = s ;

/* initialize */

	if ((rs = msgbuf_start(&mb,fd,size,to)) >= 0) {
	    int		ml = 0 ;
	    cchar	*mp ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("proginput/procstream: while-before\n") ;
#endif

	    while (rs >= 0) {
		char	*bp ;
	        rs = msgbuf_read(&mb,&mp) ;
	        ml = rs ;

	        if (pip->open.logprog && pip->f.logextra) {
	            logfile_printf(&pip->lh,"read %d",rs) ;
		}

	        if (rs <= 0) break ;

		bp = (char *) mp ;
	        msgtype = MKCHAR(mp[0]) ;
	        pip->daytime = time(NULL) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("proginput/procstream: "
	                "msgbuf_read() ml=%u mt=%u\n",ml,msgtype) ;
	        }
#endif

	        switch (msgtype) {

	        case pingstatmsgtype_update:
	            mlen = pingstatmsg_update(&m0,1,bp,ml) ;
	            if (mlen > 0) {
	                n += 1 ;
	                rs = procupdate(pip,&m0) ;
	            }
	            break ;

	        case pingstatmsgtype_uptime:
	            mlen = pingstatmsg_uptime(&m1,1,bp,ml) ;
	            if (mlen > 0) {
	                n += 1 ;
	                rs = procuptime(pip,&m1) ;
	            }
	            break ;

	        default:
	            mlen = pingstatmsg_unknown(&mu,1,bp,ml) ;
	            if (mlen > 0)
	                mlen = mu.msglen ;
	            break ;

	        } /* end switch */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("proginput/procstream: "
	                "switch-after rs=%d mlen=%d\n",
	                rs,mlen) ;
#endif

	        if ((rs >= 0) && (pip->intrun >= 0) && (ml == mlen)) {
	            if ((pip->daytime - ti_start) >= pip->intrun) {
	                if (pip->open.logprog) {
	                    logfile_printf(&pip->lh,"runint to") ;
			}
	                break ;
	            }
	        }

	        if ((rs >= 0) && (mlen > 0)) {
	            rs = pingstatdb_check(&pip->ps,pip->daytime) ;
		}

	        if (pip->open.logprog && (mlen >= 0)) {
	            logfile_flush(&pip->lh) ;
		}

	        if (rs >= 0) {
	            rs = msgbuf_adv(&mb,mlen) ;
	            if (pip->open.logprog && pip->f.logextra) {
	                logfile_printf(&pip->lh,"adv %d\n",rs) ;
		    }
	        }

	        loopcount += 1 ;
	        if (rs < 0) break ;
	    } /* end while (processing messages) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("proginput/procstream: while-after\n") ;
#endif

	    rs1 = msgbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (msgbuf) */

	if (pip->open.logprog && pip->f.logextra) {
	    logfile_printf(&pip->lh,"while-out %d",rs) ;
	}

	if (rs == SR_TIMEDOUT)
	    rs = SR_OK ;

	if (rs >= 0) {
	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: updates=%u\n",pip->progname,n) ;
	    if (pip->open.logprog)
	        logfile_printf(&pip->lh,"updates=%u",n) ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("proginput/procstream: ret rs=%d n=%d\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procstream) */


int procdatagram(PROGINFO *pip,int fd)
{
	struct pingstatmsg_update	m0 ;
	struct pingstatmsg_uptime	m1 ;
	struct pingstatmsg_unknown	mu ;
	int		ti_start = pip->daytime ;
	int		ti_read = pip->daytime ;
	const int	to = TO_READ ;
	int		rs = SR_OK ;
	int		size = 0 ;
	int		msgtype ;
	int		bl = 0 ;
	int		mflags = 0 ;
	int		mopts = 0 ;
	int		loopcount = 0 ;
	int		s ;
	int		mlen ;
	int		n = 0 ;
	char		msgbuf[MSGBUFLEN + 1] ;
	char		*bp ;

#ifdef	COMMENT
	if (pip->intmininput > 0)
	    to = pip->intmininput ;
#endif

	if (pip->open.logprog && pip->f.logextra)
	    logfile_printf(&pip->lh,"read to=%u",to) ;

/* find maximum message size (or machine pagesize) */

	s = pip->pagesize ;
	if (s > size) size = s ;
	s = sizeof(struct pingstatmsg_uptime) ;
	if (s > size) size = s ;
	s = sizeof(struct pingstatmsg_update) ;
	if (s > size) size = s ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("proginput/procdatagram: while-before\n") ;
#endif

	while (rs >= 0) {

	    while (rs >= 0) {

	        bp = msgbuf ;
	        rs = uc_recve(fd,msgbuf,MSGBUFLEN,mflags,to,mopts) ;
	        bl = rs ;

	        pip->daytime = time(NULL) ;

	        if (pip->open.logprog && pip->f.logextra) {
	            logfile_printf(&pip->lh,"recve %d",rs) ;
		}

#if	CF_DEBUGN && defined(DEBFNAME)
	        {
	            pid_t	pid = getpid() ;
	            time_t	daytime = time(NULL) ;
	            char	timebuf[TIMEBUFLEN + 1] ;
	            nprintf(DEBFNAME,"%s pid=%u recve rs=%d\n",
	                timestr_log(daytime,timebuf),pid,rs) ;
	        }
#endif /* CF_DEBUGN */

	        if (rs <= 0) break ;

	        ti_read = pip->daytime ;
	        msgtype = (bp[0] & 0xff) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("proginput/procdatagram: "
	                "uc_recve() bl=%u mt=%u\n",
	                bl,msgtype) ;
#endif

	        while (bl > 0) {

	            switch (msgtype) {

	            case pingstatmsgtype_update:
	                mlen = pingstatmsg_update(&m0,1,bp,bl) ;
	                if (mlen > 0) {
	                    n += 1 ;
	                    rs = procupdate(pip,&m0) ;
	                }
	                break ;

	            case pingstatmsgtype_uptime:
	                mlen = pingstatmsg_uptime(&m1,1,bp,bl) ;
	                if (mlen > 0) {
	                    n += 1 ;
	                    rs = procuptime(pip,&m1) ;
	                }
	                break ;

	            default:
	                mlen = pingstatmsg_unknown(&mu,1,bp,bl) ;
	                if (mlen > 0)
	                    mlen = mu.msglen ;
	                break ;

	            } /* end switch */

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("proginput/procdatagram: "
	                    "switch-after rs=%d mlen=%d\n",
	                    rs,mlen) ;
#endif

	            if ((rs < 0) || (mlen < 0)) break ;

	            bp += mlen ;
	            bl -= mlen ;

	        } /* end while */

	        if ((rs >= 0) && (pip->intrun >= 0)) {
	            if ((pip->daytime - ti_start) >= pip->intrun) {
	                if (pip->open.logprog && pip->f.logextra)
	                    logfile_printf(&pip->lh,"runint to") ;
	                break ;
	            }
	        }

	        if (pip->open.logprog) {
	            logfile_flush(&pip->lh) ;
		}

	        loopcount += 1 ;
	        if (rs < 0) break ;
	    } /* end while (processing messages) */

	    if (rs >= 0) {
	        rs = pingstatdb_check(&pip->ps,pip->daytime) ;
	    }

#if	CF_DEBUGN && defined(DEBFNAME)
	    nprintf(DEBFNAME,"proginput/procdategram: "
	        "pingstadb_check() rs=%d\n",rs) ;
#endif

	    if ((pip->daytime - ti_read) >= pip->intmininput)
	        break ;

	    if ((rs >= 0) && (pip->intrun >= 0)) {
	        if ((pip->daytime - ti_start) >= pip->intrun) break ;
	    }

	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("proginput/procdatagram: while-after\n") ;
#endif

	if (pip->open.logprog && pip->f.logextra) {
	    logfile_printf(&pip->lh,"while-out %d",rs) ;
	}

	if (rs == SR_TIMEDOUT)
	    rs = SR_OK ;

	if (rs >= 0) {
	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: updates=%u\n",pip->progname,n) ;
	    }
	    if (pip->open.logprog) {
	        logfile_printf(&pip->lh,"updates=%u",n) ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("proginput/procdatagram: ret rs=%d n=%d\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procdatagram) */


static int procupdate(PROGINFO *pip,struct pingstatmsg_update *mp)
{
	int		rs = SR_OK ;

	if (mp->hostname[0] != '\0') {
	    PINGSTATDB_UP	u ;
	    memset(&u,0,sizeof(PINGSTATDB_UP)) ;
	    u.count = UINT_MAX ;
	    u.timestamp = mp->timestamp ;
	    u.timechange = pip->daytime ;
	    rs = procentry(pip,mp->hostname,&u) ;
	} else
	    rs = SR_BADFMT ;

	return rs ;
}
/* end subroutine (procupdate) */


static int procuptime(PROGINFO *pip,struct pingstatmsg_uptime *mp)
{
	int		rs = SR_OK ;

	if (mp->hostname[0] != '\0') {
	    PINGSTATDB_UP	u ;
	    memset(&u,0,sizeof(PINGSTATDB_UP)) ;
	    u.count = mp->count ;
	    u.timestamp = mp->timestamp ;
	    u.timechange = mp->timechange ;
	    rs = procentry(pip,mp->hostname,&u) ;
	} else
	    rs = SR_BADFMT ;

	return rs ;
}
/* end subroutine (procuptime) */


static int procentry(PROGINFO *pip,cchar *hostname,PINGSTATDB_UP *up)
{
	PINGSTATDB	*psp = &pip->ps ;
	PINGSTATDB_ENT	pe ;
	time_t		timestamp ;
	int		rs = SR_OK ;
	int		rs_match ;
	int		f_update = TRUE ;
	int		f_up = FALSE ;
	char		timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("proginput/procentry: hostname=%s\n",hostname) ;
#endif

	if (hostname == NULL) return SR_FAULT ;

	if (hostname[0] == '\0') return SR_INVALID ;

/* get the time stamp that we want to use */

	timestamp = (time_t) up->timestamp ;

	if ((timestamp == 0) || (timestamp > pip->daytime))
	    timestamp = pip->daytime ;

/* see if this host is already in the database */

	rs_match = pingstatdb_match(psp,hostname,&pe) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("proginput/procentry: rs_match=%d\n",rs_match) ;
#endif

#if	CF_DEBUGN && defined(DEBFNAME)
	nprintf(DEBFNAME,"proginput/procentry: "
	    "pingstadb_match() hn=>%s< rs=%d\n",
	    hostname,rs_match) ;
#endif

	if ((rs_match >= 0) && pe.f_up) {

	    if ((pip->daytime - pe.ti_ping) < pip->intminupdate)
	        f_update = FALSE ;

	} /* end if (deciding if an update was needed) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("proginput/procentry: f_update=%d\n",f_update) ;
#endif

	if (f_update) {
	    int		f_state0, f_state1 ;
	    int		f_present ;
	    const char	*s ;

	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: updating host=%s\n",
	            pip->progname,hostname) ;
	    }

	    f_state0 = FALSE ;
	    f_up = TRUE ;		/* always UP (for now) */

/* prior state if any */

	    f_present = FALSE ;
	    if (rs_match >= 0) {
	        f_present = TRUE ;
	        f_state0 = pe.f_up ;
	    }

/* new state */

	    f_state1 = f_up ;

/* update the DB with the new information */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("proginput/procentry: doing the update \n") ;
#endif

	    if (up->count != UINT_MAX) {

	        s = "uptime" ;
	        up->timestamp = timestamp ;
	        rs = pingstatdb_uptime(psp,hostname,up) ;

#if	CF_DEBUGN && defined(DEBFNAME)
	        nprintf(DEBFNAME,"proginput/procentry: "
	            "pingstadb_uptime() rs=%d\n",rs) ;
#endif

	    } else {

	        s = "update" ;
	        rs = pingstatdb_update(psp,hostname,f_state1,timestamp) ;

#if	CF_DEBUGN && defined(DEBFNAME)
	        nprintf(DEBFNAME,"proginput/procentry: "
	            "pingstadb_update() rs=%d\n",rs) ;
#endif

	    } /* end if */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("proginput/procentry: pingstatdb_%s() rs=%d\n",
	            s,rs) ;
#endif

/* record any changes */

	    if ((rs >= 0) && (! LEQUIV(f_state0,f_state1))) {
	        const char	*fmt ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("proginput/procentry: host state change\n") ;
#endif

	        if (pip->daytime == 0)
	            pip->daytime = time(NULL) ;

	        fmt = (f_present) ? "%s %s %s\n" : "%s %s %s (new)\n" ;
	        logfile_printf(&pip->lh,fmt,
	            timestr_logz(timestamp,timebuf),
	            ((f_state1) ? "U" : "D"),
	            hostname) ;

/* should we make an entry in the summary file? */

	        if (pip->sumfp != NULL) {

	            bprintf(pip->sumfp,fmt,
	                timestr_logz(timestamp,timebuf),
	                ((f_state1) ? "U" : "D"),
	                hostname) ;

	        } /* end if (summary file entry) */

	    } /* end if (any changes) */

	    if (pip->debuglevel > 0) {
		cchar	*pn = pip->progname ;
		bprintf(pip->efp,"%s: mode=%s\n",pn,s) ;
	    }

	} /* end if (update) */

	return rs ;
}
/* end subroutine (procentry) */


