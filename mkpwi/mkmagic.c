/* mkmagic */

/* create a string with a (pretty much) standard magic string  */


#define	CF_DEBUGS	0		/* switchable debug print-outs */


/* revision history:

	= 2004-02-01, David A­D­ Morano
        This code was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

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
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local subroutines */


/* exported subroutines */


int mkmagic(char *rbuf,int rlen,cchar *ms)
{
	const int	ml = (rlen-2) ;
	char		*bp ;
	bp = strwcpy(rbuf,ms,ml) ;
	*bp++ = '\n' ;
	*bp++ = '\0' ;
	memset(bp,0,(rbuf+ml)-bp) ;
	return rlen ;
}
/* end subroutine (mkmagic) */


