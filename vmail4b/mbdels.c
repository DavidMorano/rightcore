/* mbdels */

/* check if there are messages to delete */


#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_MAILBOXDEL	0		/* mailbox delete */


/* revision history:

	= 1994-01-20, David A­D­ Morano
        Oh, what a mess. I took over this code from nobody since it has been
        lying dormant for about 13 or more years! I have fixed countless
        problems and made a couple of enhancements.

	= 2001-01-03, David A­D­ Morano
        I intend to provide some redimentary locking mechanism on the mailbox
        file. This would have been handled automatically if plans proceeded to
        move to the new mailbox object that was planned back in 1994! It is
        amazing how the years fly by with no progress on some old outstanding
        issues!! Needless to say, I am rather forced to do this hack since I
        really do need correct mailbox file locking from time to time.

*/

/* Copyright © 1994,2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine checks to see if there are any messages to delete.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local (forward) subroutines */


/* local variables */







int mbdels(mbp)
struct mailbox	*mbp ;
{
	int	rs = SR_OK ;
	int	i ;
	int	f ;


	if (mbp == NULL)
	    return SR_FAULT ;

	for (i = 0 ; i < mbp->total ; i += 1) {

	    if (messdel[i]) 
		break ;

	} /* end for */

	f = (i < mbp->total) ? TRUE : FALSE ;
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mbdels) */



