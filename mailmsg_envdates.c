/* mailmsg_envdates */

/* extract the environment date (if any) from a message */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine searches through the various envelope dates and returns
        the earliest one found. Message header dates are ignored.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<mailmsg.h>
#include	<dater.h>
#include	<localmisc.h>


/* local defines */

#ifndef	MAILMSG_ENV
#define	MAILMSG_ENV	struct mailmsg_env
#endif


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
#endif

#if	CF_DEBUGS
extern char	*timestr_log(time_t,char *) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mailmsg_envdates(MAILMSG *msgp,struct timeb *nowp,char *zname,time_t *rp)
{
	DATER		d ;
	int		rs ;
	int		n = 0 ;

#if	CF_DEBUGS
	char		timebuf[TIMEBUFLEN + 1] ;
#endif


#if	CF_DEBUGS
	debugprintf("mailmsg_envdates: ent\n") ;
#endif

	if (msgp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mailmsg_envdates: continuing\n") ;
#endif

	if ((rs = dater_start(&d,nowp,zname,-1)) >= 0) {
	    MAILMSG_ENV	*envp ;
	    time_t	envtime ;
	    time_t	posttime = 0 ;
	    int		i ;

	    for (i = 0 ; (rs = mailmsg_envdate(msgp,i,&envp)) >= 0 ; i += 1) {
	        if (envp != NULL) {

#if	CF_DEBUGS
	        debugprintf("mailmsg_envdates: envelope %d\n",i) ;
#endif

	        if ((envp->date != NULL) && (envp->date[0] != '\0')) {

#if	CF_DEBUGS
	            debugprintf("mailmsg_envdates: date string\n") ;
	            debugprintf("mailmsg_envdates: date string=%s\n",
	                envp->date) ;
#endif

	            if ((rs = dater_setstd(&d,envp->date,-1)) >= 0) {

	                dater_gettime(&d,&envtime) ;

#if	CF_DEBUGS
	                debugprintf("mailmsg_envdates: got a good date\n") ;
	                debugprintf("mailmsg_envdates: %s\n",
	                    timestr_log(envtime,timebuf)) ;
#endif

	                n += 1 ;
	                if (posttime != 0) {
	                    if (envtime < posttime)
	                        posttime = envtime ;
	                } else
	                    posttime = envtime ;

	            } /* end if (had a good date) */

#if	CF_DEBUGS
	            debugprintf("mailmsg_envdates: past if\n") ;
	            debugprintf("mailmsg_envdates: envelope datepost=%s\n",
	                timestr_log(posttime,timebuf)) ;
#endif

	        } /* end if (we got a good date) */

		} /* end if (non-null) */
	        if (rs < 0) break ;
	    } /* end for (looping through envelopes) */

	    if (rp != NULL) {
	        *rp = NULL ;
	        if (posttime != 0) *rp = posttime ;
#if	CF_DEBUGS
	        debugprintf("mailmsg_envdates: date=%s\n",
	            timestr_log(posttime,timebuf)) ;
#endif
	    } /* end if */

	    dater_finish(&d) ;
	} /* end if (dater) */

#if	CF_DEBUGS
	debugprintf("mailmsg_envdates: ret rs=%d n=%d\n",
	    rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (mailmsg_envdates) */


