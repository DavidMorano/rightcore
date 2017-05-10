/* writeto */

/* perform timed write operation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-02-01, David A­D­ Morano
        This object module was originally written to create a logging mechanism
        for PCS application programs.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine performs a timed write operation (to an FD).
	It is very much like 'u_write(3u)' but can take an optional
	time-out operand.


******************************************************************************/


#define	TERMNOTE_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	POLLMULT
#define	POLLMULT	1000
#endif

#define	TO_POLL		2


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int writeto(wfd,wbuf,wlen,wto)
int		wfd ;
const char	wbuf[] ;
int		wlen ;
int		wto ;
{
	struct pollfd	fds[2] ;

	time_t	daytime = time(NULL) ;
	time_t	ti_write ;

	int	rs = SR_OK ;
	int	i ;
	int	pt = TO_POLL ;
	int	pto ;
	int	tlen = 0 ;


	if (wfd < 0)
	    return SR_BADF ;

	if (wbuf == NULL)
	    return SR_FAULT ;

	if (wlen < 0)
	    wlen = strlen(wbuf) ;

	if (pt > wto)
	    pt = wto ;

	i = 0 ;
	fds[i].fd = wfd ;
	fds[i].events = POLLOUT ;
	i += 1 ;
	fds[i].fd = -1 ;
	fds[i].events = 0 ;

	ti_write = daytime ;
	pto = (pt * POLLMULT) ;
	while ((rs >= 0) && (tlen < wlen)) {

	    rs = u_poll(fds,1,pto) ;

	    daytime = time(NULL) ;
	    if (rs > 0) {
	        int	re = fds[0].revents ;


	        if (re & POLLOUT) {

	            rs = u_write(wfd,(wbuf+tlen),(wlen-tlen)) ;
	            tlen += rs ;
	            ti_write = daytime ;

	        } else if (re & POLLHUP) {
	            rs = SR_HANGUP ;
	        } else if (re & POLLERR) {
	            rs = SR_POLLERR ;
	        } else if (re & POLLNVAL) {
	            rs = SR_NOTOPEN ;
	        } /* end if (poll returned) */

	    } /* end if (got something) */

	    if (rs == SR_INTR)
	        rs = SR_OK ;

	    if ((daytime - ti_write) >= wto)
	        break ;

	} /* end while */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (writeto) */



