/* getnprocessors */

/* get the number of currently configured (online) processors */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine gets and returns the current number of processors that
        are online.

	Synopsis:

	int getnprocessors(envv,w)
	const char	**envv ;
	int		w ;

	Arguments:

	- envv		pointer to process environment
	- w		which value:
				0=online
				1=configured

	Returns:

	>=0		the number of processors that are currently online
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARNCPU
#define	VARNCPU		"NCPU"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */

extern char	**environ ;


/* forward references */


/* exported subroutines */


int getnprocessors(cchar **envv,int w)
{
	int		rs = SR_OK ;
	int		n = 0 ;

	if (w < 0) return SR_INVALID ;

	if (envv == NULL) envv = (cchar **) environ ;

	if ((w == 0) && (n == 0)) { /* environment approximates ONLINE CPUs */
	    cchar	*vn = VARNCPU ;
	    cchar	*cp ;
	    if ((cp = getourenv(envv,vn)) != NULL) {
	        int	v ;
	        if (cfdeci(cp,-1,&v) >= 0) n = v ;
	    } /* end if (environment) */
	}

	if ((rs >= 0) && (n == 0)) {
	    rs = uc_nprocessors(w) ;
	    n = rs ;
	}

	if (n == 0) n = 1 ;

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (getnprocessors) */


