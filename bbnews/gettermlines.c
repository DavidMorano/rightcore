/* gettermlines */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1995-05-01, David A­D­ Morano
        This code module was completely rewritten to replace any original
        garbage that was here before.

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1995,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We get some terminal lines here.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	VARLINES
#define	VARLINES	"LINES"
#endif

#ifndef	VARTERMCAP
#define	VARTERMCAP	"TERMCAP"
#endif


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int gettermlines(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		n = pip->termlines ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("gettermlines: ent termlines=%d\n",
	        pip->termlines) ;
#endif

	if (n <= 0) {
	const char	*cp ;
	if ((cp = getenv(VARLINES)) == NULL) {

/* next try the termcap variable */

	    if ((cp = getenv(VARTERMCAP)) != NULL) {

	        while ((cp = strchr(cp,':')) != NULL) {
	            cp += 1 ;
	            if (strncmp(cp,"li#",3) == 0 )
	                n = atoi(cp + 3) ;
	        } /* end while */

	    } /* end if */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("gettermlines: TERMCAP, termlines=%d\n",
	            pip->termlines) ;
#endif

	} else {
	    n = atoi(cp) ;
	}
	pip->termlines = n ;
	} /* end if (needed) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("gettermlines: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (n >= 0) ? n : rs ;
}
/* end subroutine (gettermlines) */


