/* getnfile */

/* get N-Files (available file-descriptors in UNIX®) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0	/* compile-time debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Here we get the number of file-descriptors that are available to the
	current process.  This will be the so-called "soft" limit of the number
	of descriptors.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/resource.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	NOFILE
#define	NOFILE		20
#endif

#define	GETNFILE_MAXFD	2048	/* our fake maximum limit */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getnfile(int w)
{
	struct rlimit	limit ;
	int		nf = 0 ;
	int		rs ;
	if ((rs = u_getrlimit(RLIMIT_NOFILE,&limit)) >= 0) {
	    switch (w) {
	    case 0:
	        nf = (int) limit.rlim_cur ;
		break ;
	    case 1:
	        nf = (int) limit.rlim_max ;
		break ;
	    case 2:
		nf = NOFILE ;
		break ;
	    default:
		rs = SR_INVALID ;
		break ;
	    } /* end switch */
	    if (rs >= 0) {
  	        if (nf == RLIM_INFINITY) {
		    nf = GETNFILE_MAXFD ;
	        } else if (nf < 0) {
		    nf = NOFILE ;
	        }
	    }
	}
	return (rs >= 0) ? nf : rs ;
}
/* end subroutine (getnfile) */


