/* process_input */

/* process messages on the input stream */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1


/* revision history:

	= 96/03/01, David A­D­ Morano

	The subroutine was adapted from other programs that
	do similar things.


*/


/******************************************************************************

	This subroutine processes messages that are present on the
	input stream.



******************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<logfile.h>
#include	<varsub.h>
#include	<dater.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	MSGBUFLEN	2048
#define	INBUFLEN	(2 * 1024)

#ifndef	LONGTIME
#define	LONGTIME	(5 * 60)
#endif

#ifndef	TO_READ
#define	TO_READ		120
#endif


/* external subroutines */

extern char	*timestr_logz(time_t, char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int process_input(pip,minpingint)
struct proginfo	*pip ;
int		minpingint ;
{
	time_t	daytime ;
	time_t	lastcheck ;

	int	rs = SR_OK ;
	int	len, blen, mlen ;
	int	type, n ;
	int	loopcount ;
	int	ifd = FD_STDIN ;
	int	f_bad = FALSE ;

	char	inbuf[INBUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*bp, *cp ;


	n = 0 ;

	if (pip->mininputint >= 0)
		rs = uc_reade(ifd,inbuf,INBUFLEN,pip->mininputint,0) ;

	else
		rs = u_read(ifd,inbuf,INBUFLEN) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process_input: 1 u_read[e]() rs=%d\n",rs) ;
#endif

	len = rs ;
	blen = rs ;
	bp = inbuf ;

	while ((rs > 0) && (blen > 0)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("process_input: top blen=%d \n",blen) ;
#endif




	    if ((rs < 0) || f_bad)
	        break ;

	    if (mlen > 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process_input: bot mlen=%d\n",mlen) ;
#endif

	        blen -= mlen ;
	        bp += mlen ;
	        n += 1 ;

	    } /* end if (we used up some data -- update buffer) */

	    rs = 0 ;
	    if ((! pip->f.dgram) && ((mlen <= 0) || (blen == 0))) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process_input: need more \n") ;
#endif

	        if (blen > 0) {

	            int	i ;


#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("process_input: extra blen=%d\n",blen) ;
#endif

	            for (i = 0 ; i < blen ; i += 1)
	                inbuf[i] = *bp++ ;

	        } /* end if (had some data left over) */

/* set up parameters for another read */

		bp = (inbuf + blen) ;
		len = (INBUFLEN - blen) ;

		if (pip->mininputint >= 0) {

		    rs = uc_reade(ifd,bp,len,pip->mininputint,0) ;

		    if (rs == SR_AGAIN)
			rs = SR_OK ;

		} else
	            rs = u_read(ifd,bp,len) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process_input: 2 u_read() rs=%d\n",rs) ;
#endif

	        if (rs <= 0)
	            break ;

	        len = rs ;
	        blen += len ;
		bp = inbuf ;

	    } /* end if (need more input) */

	} /* end while (had input data) */

/* what about any bad message received ? */

	if (f_bad) {

	    logfile_printf(&pip->lh,"got a bad/unknown message") ;

#ifdef	COMMENT
	    rs = SR_INVALID ;
#endif

	} /* end if (bad message received) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process_input: ret rs=%d n=%d\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (process_input) */



/* LOCAL SUBROUTINES */



