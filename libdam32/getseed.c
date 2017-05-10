/* getseed */

/* get random data from the UNIX® kernel */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */
#define	CF_GETHRTIME	1		/* use |gethrtime(3c)| */


/* revision history:

	= 2001-04-11, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	
	Synopsis:

	int getseed(int seed)

	Arguments:

	seed		given value to add to the mix

	Returns:

	<0		error
	>0		returned number of bytes


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/time.h>		/* |gettimeofday(3c)| */
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARRANDOM
#define	VARRANDOM	"RANDOM"
#endif


/* external subroutines */

extern int	randlc(int) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfdeci(cchar *,int,int *) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getseed(int seed)
{
	struct timeval	tv ;
	const pid_t	pid = getpid() ;
	const uid_t	uid = getuid() ;
	uint		rv = 0 ;
	int		rs ;
	int		v1, v2 ;
	int		v3 = 0 ;
	cchar		*cp ;

	    gettimeofday(&tv,NULL) ;

	    v1 = getppid() ;

	    v2 = getpgrp() ;

	    if ((cp = getenv(VARRANDOM)) != NULL) {
		cfdeci(cp,-1,&v3) ;
	    }

	rv += randlc(tv.tv_usec) ;
	rv += randlc((int) pid) ;
	rv += randlc(v1) ;
	rv += randlc(v2) ;
	rv += randlc(v3) ;
	rv += randlc(tv.tv_sec) ;
	rv += randlc(uid) ;
	rv += randlc(seed) ;

#if	CF_GETHRTIME
	{
	    hrtime_t	h = gethrtime() ;
	    rv += (uint) h ;
	    h >>= sizeof(uint) ;
	    rv += (uint) h ;
	}
#endif /* CF_GETHRTIME */

	rs = (rv & INT_MAX) ;
	return rs ;
}
/* end subroutine (getseed) */


