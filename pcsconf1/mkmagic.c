/* mkmagic */

/* create a string with a (pretty much) standard magic string  */


#define	CF_DEBUGS	0		/* switchable debug print-outs */


/* revision history:

	= 2004-02-01, David A­D­ Morano
        This code was originally written.

	= 2017-07-17, David A­D­ Morano
	I added the code to return overflow if the given string cannot fit into
	the result buffer.

*/

/* Copyright © 2004,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int mkmagic(rbuf,rlen,ms)
	char		rbuf[] ;
	int		rlen ;
	const char	*ms ;

	Arguments:

	rbuf		result buffer
	rlen		result buffer length
	ms		source string

	Returns:

	<0		error
	>=0		length of resulting string


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local subroutines */


/* exported subroutines */


int mkmagic(char *rbuf,int rlen,cchar *ms)
{
	const int	mslen = strlen(ms) ;
	int		rs = SR_OK ;
#if	CF_DEBUGS
	debugprintf("mkmagic: ent rlen=%u msl=%u\n",rlen,mslen) ;
#endif
	if ((mslen+1) <= rlen) {
	    char	*bp = strwcpy(rbuf,ms,-1) ;
	    *bp++ = '\n' ;
	    *bp++ = '\0' ;
	    if (((rbuf+rlen)-bp) > 0) {
	        memset(bp,0,(rbuf+rlen)-bp) ;
	    }
	} else {
	    if (rlen > 0) rbuf[0] = '\0' ;
	    rs = SR_OVERFLOW ;
	}
#if	CF_DEBUGS
	debugprintf("mkmagic: ret rs=%d rlen=%u\n",rs,rlen) ;
#endif
	return (rs >= 0) ? rlen : rs ;
}
/* end subroutine (mkmagic) */


