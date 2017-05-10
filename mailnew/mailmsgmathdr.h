/* mailmsgmathdr */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILMSGMATHDR_INCLUDE
#define	MAILMSGMATHDR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>


#if	(! defined(MAILMSGMATHDR_MASTER)) || (MAILMSGMATHDR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsgmathdr(const char *,int,int *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGMATHDR_MASTER */

#endif /* MAILMSGMATHDR_INCLUDE */


