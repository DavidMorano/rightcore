/* progmailbox */

/* copy the mail out of the spool area and into the user's MB */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1994-01-23, David A­D­ Morano

	This module was copied and modified from the original with VMAIL.


	= 1996-06-18, David A­D­ Morano

	I did:
		- remove old mail spool locks


	= 1996-07-24, David A­D­ Morano

	I rewrote the "get_newmail" subroutine in part to:
		- lock user's "new" mailbox when fetching new mail
		  NOTE: This has to be removed when real proper
			mailbox handling is implemented.
		- guard against corrupted "new" mailbox on new mail
		  from file system full
		- added full binary compatibility for new mail


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine copies the mail data out of the spool file and into the
	user specified mailbox file (or whatever if it wasn't specified).

	Synopsis:

	int progmailbox(pip,mfd,f_lock)
	PROGINFO	*pip ;
	int		mfd ;
	int		f_lock ;

	Arguments:

	pip		pointer to program information
	mfd		mailbox file descriptor
	f_lock		flag indicating if the mailbox file needs to be locked

	Returns:

	>=0 		if successful
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<signal.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<sigblock.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* external subroutines */

extern int	lockfile(int,int,offset_t,offset_t,int) ;
extern int	lockend(int,int,int,int) ;
extern int	progspool(PROGINFO *,int,vecstr *,cchar *,cchar *) ;


/* external variables */


/* forward references */

static int	getthem(PROGINFO *,int,VECSTR *) ;


/* local variables */


/* exported subroutines */


int progmailbox(pip,mfd,f_lock)
PROGINFO	*pip ;
int		mfd ;
int		f_lock ;
{
	SIGBLOCK	blocker ;
	int		rs ;
	int		rs1 ;
	int		tlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmailbox: mfd=%d\n",mfd) ;
#endif

	if ((rs = sigblock_start(&blocker,NULL)) >= 0) {
	    offset_t	moff ;

/* lock the user's mailbox file */

	    if (f_lock) {
	        if (isatty(mfd) > 0) f_lock = FALSE ;
	    }

	    if (f_lock) {
	        rs = lockend(mfd,TRUE,0,pip->to_mailbox) ;
	    }

	    if (rs >= 0) {

/* go to the end ... */

	        u_seek(mfd,0L,SEEK_END) ;

	        if (f_lock)
	            u_tell(mfd,&moff) ;

/* perform the copy-out operation */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progmailbox: getthem() \n") ;
#endif

	        if (rs >= 0) {
	            rs = getthem(pip,mfd,NULL) ;
	            tlen = rs ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progmailbox: getthem() rs=%d\n",rs) ;
#endif

/* unlock the user's mail box file */

	        if (f_lock) {
	            rs1 = lockfile(mfd,F_UNLOCK,moff,0L,0) ;
		    if (rs > 0) rs = rs1 ;
		}

	    } /* end if (ok) */

	    sigblock_finish(&blocker) ;
	} /* end if (sigblock) */

/* cleanup */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmailbox: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (progmailbox) */


/* local subroutines */


static int getthem(pip,mfd,flp)
PROGINFO	*pip ;
int		mfd ;
VECSTR		*flp ;
{
	int		rs = SR_OK ;
	int		i, j ;
	int		len ;
	int		tlen = 0 ;
	const char	*mdp, *mup ;

	for (i = 0 ; vecstr_get(&pip->maildirs,i,&mdp) >= 0 ; i += 1) {
	    if (mdp == NULL) continue ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp, "%s: maildir=%s\n",pip->progname,mdp) ;

	    if (pip->open.logprog)
	        logfile_printf(&pip->lh, "maildir=%s",mdp) ;

	    for (j = 0 ; vecstr_get(&pip->mailusers,j,&mup) >= 0 ; j += 1) {
	        if (mup == NULL) continue ;

	        rs = progspool(pip,mfd,flp,mdp,mup) ;
	        len = rs ;
	        if (rs == SR_NOENT) {
	            len = 0 ;
	            rs = SR_OK ;
	        }

	        tlen += len ;
	        if (rs < 0) break ;

	        if (len > 0) {
	            if (pip->debuglevel > 0) {
	                bprintf(pip->efp,
	                    "%s: mailuser=%s (%u)\n",
	                    pip->progname,mup,len) ;
		    }
	            if (pip->open.logprog) {
	                logfile_printf(&pip->lh,
	                    "mailuser=%s (%u)",mup,len) ;
		    }
	        } /* end if */

	    } /* end for (mailusers) */

	    if (rs < 0) break ;
	} /* end for (maildirs) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process/getthem: ret rs=%d tlen=%u\n",
	        rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (getthem) */


