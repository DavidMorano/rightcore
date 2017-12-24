/* sfbaselib */

/* string-find a base-library name */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2017-12-08, David A­D­ Morano
        The subroutine was extracted from where it was first developed.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine finds a the base-library name (part) from a given 
	string.

	Synopsis:

	int sfbaselib(sp,sl,rpp)
	const char	*sp ;
	int		sl ;
	const char	**rpp ;

	Arguments:

	sp	supplied string to test
	sl	length of supplied string to test
	rpp	pointer to store result "thing" pointer

	Returns:

	>=0	length of result "thing" 
	<0	error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>		/* for MAXNAMELEN */
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	shbasename(cchar *,int,cchar **) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnsub(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int sfbaselib(cchar *pnp,int pnl,cchar **rpp)
{
	int		cl ;
	const char	*tp ;
	const char	*cp ;

	if ((cl = sfbasename(pnp,pnl,&cp)) > 0) {
	    pnp = cp ;
	    pnl = cl ;
	}

	if ((tp = strnrchr(pnp,pnl,'.')) != NULL) {
	    pnl = (tp-pnp) ;
	}

	if ((pnl > 3) && (strncmp(pnp,"lib",3) == 0)) {
	    pnp += 3 ;
	    pnl -= 3 ;
	}

	if (rpp != NULL) *rpp = (char *) pnp ;

	return pnl ;
}
/* end subroutine (sfbaselib) */


