/* sigaction */

/* subroutines to manipulate SIGACTION values */


/* revision history:


	= 2017-10-24, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We manage (a little bit) the SIGACTION object.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<signal.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	SIGACTION
#define	SIGACTION	struct sigaction
#endif


/* type-defs */

#ifndef	TYPEDEF_SIGINFOHAND
#define	TYPEDEF_SIGINFOHAND	1
typedef void (*siginfohand_t)(int,siginfo_t *,void *) ;
#endif


/* exported subroutines */


int sigaction_load(SIGACTION *sap,sigset_t *ssp,int fl,siginfohand_t hand)
{
	if (sap == NULL) return SR_FAULT ;
	if (ssp == NULL) return SR_FAULT ;
	memset(sap,0,sizeof(SIGACTION)) ;
	sap->sa_mask = *ssp ;
	sap->sa_flags = fl ;
	sap->sa_handler = hand ;
	return SR_OK ;
}
/* end subroutine (sigaction_load) */


