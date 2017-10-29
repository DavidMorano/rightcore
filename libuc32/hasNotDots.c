/* hasNotDots */

/* does the given string not have the regular dots-type file names */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SWITCH	1		/* use switch */


/* revision history:

	= 2002-07-13, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines if the given string has neither of the
	following:

	+ one dot character
	+ two dot characters

	Synopsis:

	int hasNotDots(const char *sp,int sl)
	const char	*sp ;
	int		sl ;

	Arguments:

	sp	pointer to given string
	sl	length of given string

	Returns:

	==1	string does not have the standard dot-dirs
	==0	string has the standard dot-dirs


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<string.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int hasNotDots(const char *np,int nl)
{
	int		f = TRUE ;
	if (np[0] == '.') {
	    if (nl < 0) nl = strlen(np) ;
#if	CF_SWITCH
	    switch (nl) {
	    case 1:
	        f = FALSE ;
	        break ;
	    case 2:
	        f = (np[1] != '.') ;
	        break ;
	    }  /* end switch */
#else /* CF_SWITCH */
	    if (nl <= 2) {
	        f = (nl != 1) ;
	        if ((!f) && (nl == 2)) f = (np[1] != '.') ;
	    }
#endif /* CF_SWITCH */
	} /* end if (had a leading dot) */
	return f ;
}
/* end subroutine (hasNotDots) */


