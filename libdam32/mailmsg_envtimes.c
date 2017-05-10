/* mailmsg_envtimes */

/* extract all environment times (if any) from a message */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will extract all of the times (from the dates) out of
        all envelopes that may be present in the message.

	Symopsis:

	int mailmsg_envtimes(MAILMSG *msgp,DATER *dp,time_t *ta,int nents)

	Arguments:

	msgp		pointer to MAILMSG object
	dp		pointer to DATER object
	ta		array of 'time_t' entries
	nents		number of entries in 'time_t' array

	Returns:

	>=0		number of entries returned
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<mailmsg.h>
#include	<dater.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(char *,int,const char *,...) ;
#endif

#if	CF_DEBUGS
extern char	*timestr_log() ;
#endif


/* external variables */


/* forward references */


/* exported subroutines */


int mailmsg_envtimes(MAILMSG *msgp,DATER *dp,time_t *ta,int nents)
{
	time_t		t ;
	int		rs ;
	int		i ;
	int		n = 0 ;
	const char	*cp ;

#if	CF_DEBUGS
	debugprintf("mailmsg_envtimes: ent\n") ;
#endif

	if (msgp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = mailmsg_envdate(msgp,i,&cp)) >= 0 ; i += 1) {
	    if ((cp != NULL) && (cp[0] != '\0')) {

#if	CF_DEBUGS
	    debugprintf("mailmsg_envtimes: envelope %d\n",i) ;
#endif


#if	CF_DEBUGS
	        debugprintf("mailmsg_envtimes: date string\n") ;
	        debugprintf("mailmsg_envtimes: date string=>%s<\n",cp) ;
	        debugprintf("mailmsg_envtimes: calling 'getabsdate()'\n") ;
#endif

		if ((rs = dater_setstd(dp,cp,-1)) >= 0) {

#if	CF_DEBUGS
		    {
		        char	timebuf[TIMEBUFLEN + 1] ;
	                debugprintf("mailmsg_envtimes: got a good date\n") ;
	                debugprintf("mailmsg_envtimes: %s\n",
	                    timestr_log(t,timebuf)) ;
		    }
#endif /* CF_DEBUGS */

		    if ((rs = dater_gettime(dp,&t)) >= 0) {
	                if (n < nents) ta[n++] = t ;
		    }

		} else if (rs == SR_INVALID) {
		    rs = SR_OK ;
	        } /* end if (got a date) */

	    } /* end if (we got a good date) */
	    if (n >= nents) break ;
	    if (rs < 0) break ;
	} /* end for (looping through envelopes) */

	if ((n > 0) || (rs == SR_NOTFOUND)) {
	    rs = SR_OK ;
	}

#if	CF_DEBUGS
	debugprintf("mailmsg_envtimes: ret rs=%d n=%d\n", rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (mailmsg_envtimes) */


