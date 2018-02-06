/* util */

/* utility subroutines */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1994-01-10 David A.D. Morano
        Oh my! What a piece of garbage! I am taking this on to try and clean
        some of it up!

*/

/* Copyright © 1994 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module provides some (rarely used) utility functions.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<time.h>
#include	<signal.h>
#include	<string.h>
#include	<curses.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int periph_beep(pip)
struct proginfo	*pip ;
{
	int	rs ;

	char	buf[2] ;


	buf[0] = CH_BEL ;
	buf[1] = '\0' ;
	rs = u_write(pip->tfd,buf,1) ;

	return rs ;
}
/* end subroutine (periph_beep) */



