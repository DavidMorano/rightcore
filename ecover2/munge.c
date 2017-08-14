/* munge */

/* perform the munge and unmunge functions */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1999-05-06, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will do the munging and unmunging on the clear and
        scrambled data respectively.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

ULONG		cheaprand(ULONG) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


void munge(gp,n,vector,data,out)
PROGINFO	*gp ;
int		n ;
ULONG		vector[] ;
ULONG		data[] ;
ULONG		out[] ;
{
	ULONG		sum = 0 ;
	ULONG		extra ;
	int		i ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("munge: ent\n") ;
#endif

	for (i = 0 ; i < n ; i += 1) {
	    sum = sum + vector[i] ;
	}

	extra = cheaprand(sum) ;

	if (! gp->f.unscramble) {

	    for (i = 0 ; i < n ; i += 1) {

	        out[i] = vector[i] ^ data[i] ;
	        vector[i] = cheaprand(data[i]) + vector[i] + extra ;

		extra = cheaprand(extra) ;

	    } /* end for */

	} else {

	    for (i = 0 ; i < n ; i += 1) {

	        out[i] = vector[i] ^ data[i] ;
	        vector[i] = cheaprand(out[i]) + vector[i] + extra ;

		extra = cheaprand(extra) ;

	    } /* end for */

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("munge: ret\n") ;
#endif

}
/* end subroutine (munge) */


