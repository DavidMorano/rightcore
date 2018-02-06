/* getarchitecture */

/* get the machine architecture */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine gets (retrieves) the architecture of the machine that
	it is running on.

	Synopsis:

	int getarchitecture(char *abuf,int alen)

	Arguments:

	- abuf		buffer to hold result
	- alen		length of result buffer

	Retruns:

	>=0		length of result (in bytes)
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/systeminfo.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARARCHITECTURE
#define VARARCHITECTURE	"ARCHITECTURE"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;

extern const char	*getourenv(const char **,const char *) ;


/* forward references */


/* external variables */


/* local variables */


/* exported subroutines */


int getarchitecture(abuf,alen)
char		abuf[] ;
int		alen ;
{
	int		rs = SR_OK ;
	const char	*a = getenv(VARARCHITECTURE) ;

	if (abuf == NULL) return SR_FAULT ;

	abuf[0] = '\0' ;
	if ((a != NULL) && (a[0] != '\0')) {
	    rs = sncpy1(abuf,alen,a) ;
	}

#ifdef	SI_ARCHITECTURE
	if ((rs >= 0) && (abuf[0] == '\0')) {
	    rs = u_sysinfo(SI_ARCHITECTURE,abuf,alen) ;
	}
#endif /* SI_ARCHITECTURE */

	return rs ;
}
/* end subroutine (getarchitecture) */


