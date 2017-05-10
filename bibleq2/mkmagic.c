/* mkmagic */

/* make the magic sequence of bytes for (usually) an index-type file */


#define	CF_DEBUGS 	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine reads and write a bible-verse-index file.

	Synopsis:

	int mkmagic(mbuf,mp,nl)
	char		mbuf[] ;
	const char	mp[] ;
	int		ml ;

	Arguments:

	- mbuf		buffer receiving bytes
	- mp		magic string pointer
	- ml		magic string length

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkmagic(char *rbuf,cchar *mp,int ms)
{
	char		*bp = rbuf ;

	bp = strwcpy(bp,mp,(ms- 1)) ;
	*bp++ = '\n' ;
	memset(bp,0,(ms - (bp - rbuf))) ;

	return ms ;
}
/* end subroutine (mkmagic) */


