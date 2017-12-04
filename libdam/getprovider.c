/* getprovider */

/* get the machine provider string */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 2001-04-11, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get and return the machine provider string.

	Synopsis:

	int getprovider(char *rbuf,int rlen)

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
#include	<uinfo.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARPROVIDER
#define	VARPROVIDER	"PROVIDER"
#endif

#ifndef	PROVIDER
#define	PROVIDER	"Rightcore Network Services"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy1w(char *,int,cchar *,int) ;
extern int	shshrink(cchar *,int,cchar **) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getprovider(char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	cchar		*vp ;

	rbuf[0] = '\0' ;
	if ((vp = getenv(VARPROVIDER)) != NULL) { /* environment */
	    cchar	*cp ;
	    int		cl ;
	    if ((cl = sfshrink(vp,-1,&cp)) > 0) {
	        rs = sncpy1w(rbuf,rlen,cp,cl) ;
	    }
	}

	if ((rs >= 0) && (rbuf[0] == '\0')) { /* process cache */
	    UINFO_AUX	aux ;
	    if ((rs = uinfo_aux(&aux)) >= 0) {
		if (aux.provider != NULL) {
	            rs = sncpy1(rbuf,rlen,aux.provider) ;
		}
	    }
	}

	if ((rs >= 0) && (rbuf[0] == '\0')) { /* otherwise */
	    rs = sncpy1(rbuf,rlen,PROVIDER) ;
	}

	return rs ;
}
/* end subroutine (getprovider) */


