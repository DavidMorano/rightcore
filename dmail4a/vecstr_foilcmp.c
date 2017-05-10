/* vecstr_foilcmp */

/* perform a foil comparison of two VECSTR lists */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Perform a foil comparison between two VECSTR lists.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local variables */


/* exported subroutines */


int vecstr_foilcmp(lnp,rnp)
vecstr	*lnp, *rnp ;
{
	int	rs = SR_OK ;
	int	i, j ;
	int	f_match = FALSE ;

	const char	*np, *cp ;


/* do a foil comparison between the lists */

	for (i = 0 ; (rs = vecstr_get(lnp,i,&cp)) >= 0 ; i += 1) {
	    if (cp == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("vecstr_foilcmp: cp=%s\n",cp) ;
#endif

	    for (j = 0 ; vecstr_get(rnp,j,&np) >= 0 ; j += 1) {
	        if (np == NULL) continue ;

#if	CF_DEBUGS
	        debugprintf("vecstr_foilcmp: np=%s\n",np) ;
#endif

	        f_match = (cp[0] == np[0]) && (strcmp(cp,np) == 0) ;

#if	CF_DEBUGS
	        debugprintf("vecstr_foilcmp: strcmp() f_match=%u\n",
		f_match) ;
#endif

	        if (f_match)
	            break ;

	    } /* end for */

	    if (f_match) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("vecstr_foilcmp: ret rs=%d f_match=%u\n",rs,f_match) ;
#endif

	return (rs >= 0) ? f_match : rs ;
}
/* end subroutine (vecstr_foilcmp) */


