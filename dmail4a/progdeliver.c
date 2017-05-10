/* progdeliver */

/* progdelivers mail messages (data) to a mailbox spool file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used to deliver new mail to the mail spool file for
	a given recipient.

	Synopsis:

	int progdeliver(pip,tfd,rp)
	PROGINFO	*pip ;
	int		tfd ;
	RECIP		*rp ;

	Arguments:

	pip		program information pointer 
	tfd		file descriptor (FD) to target spool file
	rp		pointer to recipient container

	Returns:

	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<bfile.h>
#include	<sigblock.h>
#include	<localmisc.h>

#include	"mailspool.h"
#include	"lkmail.h"
#include	"recip.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		256
#endif

#define	LOCKBUFLEN	512

#define	FORWARDED	"Forward to "


/* external subroutines */

extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	recipcopyparts(RECIP *,int,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	debugprintfsize(cchar *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	progdeliverer(PROGINFO *,int,RECIP *,cchar *,int) ;
static int	progdeliver_mbo(PROGINFO *,RECIP *,int,int) ;
static int	progdeliver_lockinfo(PROGINFO *,MAILSPOOL *) ;
static int	mklockinfo(PROGINFO *,char *,int,time_t) ;


/* local variables */

static const int	sigblocks[] = {
	SIGALRM,
	SIGPOLL,
	SIGHUP,
	SIGTERM,
	SIGINT,
	SIGQUIT,
	SIGWINCH,
	SIGURG,
	0
} ;


/* exported subroutines */


int progdeliver(PROGINFO *pip,int tfd,RECIP *rp)
{
	int		rs ;
	int		tlen = 0 ;
	cchar		*md = rp->maildname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progdeliver: ent\n") ;
	    debugprintf("progdeliver: recip md=%s\n",rp->maildname) ;
	    debugprintf("progdeliver: recip name=%s\n",rp->recipient) ;
	    debugprintfsize("progdeliver tfd",tfd);
	}
#endif

	if ((rs = progdeliverer(pip,tfd,rp,md,TRUE)) >= 0) {
	    tlen = rs ;
	    if (pip->f.optcopy) {
		md = pip->copymaildname ;
		if (md != NULL) {
	            rs = progdeliverer(pip,tfd,rp,md,FALSE) ;
		}
	    } /* end if (opt-copy) */
	} /* end if (progdeliverer-primary) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progdeliver: ret rs=%d tlen=%u\n",rs,tlen) ;
	    debugprintfsize("progdeliver tfd",tfd);
	}
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (progdeliver) */


/* local subroutines */


static int progdeliverer(PROGINFO *pip,int tfd,RECIP *rp,cchar *md,int f)
{
	SIGBLOCK	blocks ;
	int		rs ;
	int		rs1 ;
	int		tlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    cchar	*r = rp->recipient ;
	    debugprintf("progdeliverer: ent\n") ;
	    debugprintf("progdeliverer: md=%s\n",md) ;
	    debugprintf("progdeliverer: r=%s\n",r) ;
	}
#endif

	if ((rs = sigblock_start(&blocks,sigblocks)) >= 0) {
	    if ((rs = u_rewind(tfd)) >= 0) {
	        MAILSPOOL	ms ;
		mode_t		om = 0660 ;
		const int	to = pip->to_spool ;
		int		of = (O_CREAT|O_RDWR|O_APPEND) ;
		cchar		*un = rp->recipient ;
	        if ((rs = mailspool_open(&ms,md,un,of,om,to)) >= 0) {
		    const int	mfd = rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progdeliverer: mailspool_open() rs=%d\n",rs) ;
#endif
		    if ((rs = progdeliver_lockinfo(pip,&ms)) >= 0) {
			if ((rs = u_seek(mfd,0L,SEEK_END)) >= 0) {
			    if ((rs = progdeliver_mbo(pip,rp,rs,f)) >= 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progdeliverer: mailspool_lockinfo() rs=%d\n",rs) ;
#endif
			        rs = recipcopyparts(rp,tfd,mfd) ;
			        tlen = rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progdeliverer: recipcopyparts() rs=%d\n",rs) ;
#endif
			    } /* end if (progdeliver_mbo) */
			} /* end if (u_seek) */
		    } /* end if (progdeliver_lockinfo) */
		    rs1 = mailspool_close(&ms) ;
	    	    if (rs >= 0) rs = rs1 ;
	        } /* end if (mailspool) */
	    } /* end if (u_rewind) */
	    rs1 = sigblock_finish(&blocks) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progdeliverer: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (progdeliverer) */


/* ARGSUSED */
static int progdeliver_mbo(PROGINFO *pip,RECIP *rp,int mbo,int f)
{
	int		rs = SR_OK ;
	if (f) {
	    rs = recip_mbo(rp,mbo) ;
	}
	return rs ;
}
/* end subroutine (progdeliver_mbo) */


static int progdeliver_lockinfo(PROGINFO *pip,MAILSPOOL *msp)
{
	const time_t	dt = pip->daytime ;
	const int	llen = LOCKBUFLEN ;
	int		rs ;
	char		lbuf[LOCKBUFLEN+1] ;
	if ((rs = mklockinfo(pip,lbuf,llen,dt)) > 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progdeliver: mklockinfo() rs=%d\n",rs) ;
#endif
	    rs = mailspool_setlockinfo(msp,lbuf,rs) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progdeliver: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (progdeliver_lockinfo) */


/* make (create) the information that goes into the mail-lock file */
static int mklockinfo(PROGINFO *pip,char *rbuf,int rlen,time_t ti_now)
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {

/* line 1 */

	    sbuf_deci(&b,pip->pid) ;
	    sbuf_char(&b,'\n') ;

/* line 2 */

	    sbuf_strw(&b,pip->lockaddr,-1) ;
	    sbuf_char(&b,'\n') ;

/* line 3 */

	    {
		char	tbuf[TIMEBUFLEN + 1] ;
	        timestr_logz(ti_now,tbuf) ;
	        sbuf_strw(&b,tbuf,-1) ;
	        sbuf_char(&b,' ') ;
	        sbuf_strw(&b,pip->progname,-1) ;
	        sbuf_char(&b,'\n') ;
	    }

/* line 4 */

	    sbuf_strw(&b,"logid=",-1) ;
	    sbuf_strw(&b,pip->logid,-1) ;
	    sbuf_char(&b,'\n') ;

/* done */

	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (mklockinfo) */


