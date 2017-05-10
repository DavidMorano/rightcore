/* field_svcargs */

/* parse a FIELD object into service envelope items */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1
#define	CF_SRVSHELLARG	0


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This subroutine will take a 'FIELD' object and create a string
	list of server arguments from it.


***************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<string.h>

#include	<vsystem.h>
#include	<field.h>
#include	<vecstr.h>

#include	"localmisc.h"



/* local defines */



/* external subroutines */


/* external variables */


/* forward references */


/* local variables */





/* parse the service (envelope) arguments if there are any */
int field_svcargs(fbp,sap)
struct field	*fbp ;
VECSTR		*sap ;
{
	int	rs = SR_OK ;
	int	al ;
	int	c = 0 ;

	char	argbuf[BUFLEN + 1] ;


	if (fbp == NULL)
	    return SR_FAULT ;

	if (sap == NULL)
	    return SR_FAULT ;

	while ((al = field_sharg(fbp,NULL,argbuf,BUFLEN)) >= 0) {

	    c += 1 ;
	    rs = vecstr_add(sap,argbuf,al) ;

	    if (rs < 0) break ;
	} /* end while */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (field_svcargs) */



