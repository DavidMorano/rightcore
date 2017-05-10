/* getcodedtimed */

/* read a line from a file descriptor but time it */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-26, David A­D­ Morano

	This was first written to give a little bit to UNIX what we
	have in our own circuit pack OSes!  Note that this subroutine
	depends on another little one ('uc_reade()') that is used to
	provide an underlying extended 'read(2)' like capability.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Get a line code amount of data (data ending in an NL) and
	time it also so that we can abort if it times out.

	Synopsis:

	int getcodedtimed(fd,buf,buflen,timeout)
	int	fd ;
	char	buf[] ;
	int	buflen ;
	int	timeout ;

	Arguments:

	fd		file descriptor
	buf		user buffer to receive daa
	buflen		maximum amount of data the user wants
	timeout		time in seconds to wait

	Returns:

	>=0		amount of data returned
	<0		error


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	STRIP(c)	((c) & 0x7f)


/* external subroutines */


/* exported subroutines */


int getcodedtimed(fd,buf,buflen,timeout)
int	fd ;
char	buf[] ;
int	buflen ;
int	timeout ;
{
	int	rs1 ;
	int	i, len = 0 ;
	int	rlen = 0 ;


#ifdef	COMMENT
	if (timeout < 0)
	    timeout = INT_MAX ;
#endif

	while (rlen < buflen) {

	    len = uc_reade(fd,(buf + rlen),1,timeout,0) ;

#if	CF_DEBUGS
	debugprintf("getcodedtimed: uc_reade() rs=%d\n",len) ;
#endif

	    if (len <= 0)
		break ;

	    rlen += len ;
	    if (STRIP(buf[rlen - 1]) == '\n')
		break ;

	    if (STRIP(buf[rlen - 1]) == '\r')
		break ;

	} /* end while */

#if	CF_DEBUGS
	debugprintf("getcodedtimed: after loop len=%d\n",len) ;
#endif

	if (len >= 0) {
	    for (i = 0 ; i < rlen ; i += 1) {
		buf[i] = STRIP(buf[i]) ;
	    }
	}

	return (len >= 0) ? rlen : len ;
}
/* end subroutine (getcodedtimed) */


