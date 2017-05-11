/* uc_strtod */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/uio.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<vsystem.h>


/* exported subroutines */


int uc_strtod(startp,endpp,rp)
const char	startp[] ;
char		**endpp ;
double		*rp ;
{
	int	rs ;


#if	CF_DEBUGS
	debugprintf("uc_strtod: ent\n") ;
#endif

	if (rp == NULL)
	    return SR_FAULT ;

	errno = 0 ;
	*rp = strtod(startp,endpp) ;

	rs = (errno != 0) ? (- errno) : 0 ;

#if	CF_DEBUGS
	debugprintf("uc_strtod: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_strtod) */


