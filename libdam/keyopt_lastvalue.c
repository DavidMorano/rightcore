/* keyopt_lastvalue */

/* get the last value (among multiple values) from the key-options */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We get the last value (among multiple possible values) from the
	key-options.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<keyopt.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int keyopt_lastvalue(KEYOPT *kop,cchar *kp,cchar **rpp)
{
	KEYOPT_CUR	cur ;
	int		rs ;
	int		cl, vl ;
	const char	*cp, *vp ;

	if (kp == NULL) return SR_FAULT ;

	vp = NULL ;
	vl = 0 ;
	if ((rs = keyopt_curbegin(kop,&cur)) >= 0) {

	    while ((cl = keyopt_fetch(kop,kp,&cur,&cp)) >= 0) {
		vp = cp ;
		vl = cl ;
	    }

	    keyopt_curend(kop,&cur) ;
	} /* end if (cursor) */

	if ((rs >= 0) && (vp == NULL)) 
	    rs = SR_NOTFOUND ;

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? vp : NULL ;

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (keyopt_lastvalue) */


