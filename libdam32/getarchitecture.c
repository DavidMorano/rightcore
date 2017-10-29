/* getarchitecture */

/* get the machine architecture string */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 2001-04-11, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get and return the machine architecture string.

	Synopsis:

	int getarchitecture(char *rbuf,int rlen)

	Arguments:

	rbuf		result buffer
	rlen		result buffer length

	Returns:

	>=0		number of characters returned
	<0		error


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/systeminfo.h>
#include	<stdlib.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARARCHITECTURE
#define	VARARCHITECTURE		"ARCHITECTURE"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy1w(char *,int,cchar *,int) ;
extern int	shsjrink(cchar *,int,cchar **) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getarchitecture(char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	const char	*vp ;

	rbuf[0] = '\0' ;
	if ((vp = getenv(VARARCHITECTURE)) != NULL) {
	    cchar	*cp ;
	    int		cl ;
	    if ((cl = sfshrink(vp,-1,&cp)) > 0) {
	        rs = sncpy1w(rbuf,rlen,cp,cl) ;
	    }
	}

#ifdef	SI_ARCHITECTURE
	if ((rs >= 0) && (rbuf[0] == '\0')) {
	    const int	cmd = SI_ARCHITECTURE ;
	    rs = u_sysinfo(cmd,rbuf,rlen) ;
	}
#endif /* SI_ARCHITECTURE */

	return rs ;
}
/* end subroutine (getarchitecture) */


