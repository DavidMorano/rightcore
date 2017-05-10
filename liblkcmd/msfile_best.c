/* msfile_best */

/* find the "best" machine entry */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_GETEXECNAME	1		/* get the 'exec(2)' name */
#define	CF_FLOAT	0		/* use floating point */
#define	CF_CPUSPEED	0		/* no use here */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	It cracks me up how I take one program and make another from it!  This
	program is now a built-in command (MSU) to the KSH program to update
	the machine status for the current node in the cluster.

	= 2004-01-12, David A­D­ Morano
	This program is now the MSINFO KSH built-in command.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine provides an additional method to the MSFILE object.  It
	finds the best of the entries given a couple of optional restrictions.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<netdb.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<vecobj.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"msfile.h"
#include	"msflag.h"


/* local defines */

#ifndef	MSFLAG_DISABLED
#define	MSFLAG_DISABLED		0x0001
#endif


/* external subroutines */


/* exported subroutines */


int msfile_best(msp,daytime,flags,ep)
MSFILE		*msp ;
time_t		daytime ;
uint		flags ;
MSFILE_ENT	*ep ;
{
	MSFILE_CUR	cur ;
	MSFILE_ENT	be ;
	double		capacity, used ;
	double		bestavail, avail, empty ;
	const int	nl = MSFILE_NODENAMELEN ;
	int		rs ;
	int		minspeed = INT_MAX ;
	int		c ;

	bestavail = -1000.0 ;

/* first find the minimum speed of all nodes */

	c = 0 ;
	msfile_curbegin(msp,&cur) ;

	while (TRUE) {

	    rs = msfile_enum(msp,&cur,&be) ;

	    if (rs < 0)
	        break ;

	    if (be.speed != 0) {

		c += 1 ;
	        if (be.speed < minspeed)
	            minspeed = be.speed ;

	    }

	} /* end while */

	msfile_curend(msp,&cur) ;

	if ((rs == SR_NOENT) && (c > 0))
	    rs = SR_OK ;

/* then find the one with the most available computation */

	if ((rs >= 0) && (c > 0)) {

	    c = 0 ;
	    msfile_curbegin(msp,&cur) ;

	    while (TRUE) {

	        rs = msfile_enum(msp,&cur,&be) ;

	        if (rs < 0)
	            break ;

#ifdef	COMMENT
		if (be.flags & MSFLAG_DISABLED)
			continue ;
#endif

	        capacity = (double) (be.speed * be.ncpu) ;
	        used = ((double) be.la[0]) / FSCALE ;
		empty = ((double) be.ncpu) - used ;
	        used *= ((double) minspeed) ;

	        avail = capacity - used ;

#if	CF_DEBUGS
	        debugprintf("msfile_best: n=%-10s c=%8.2f u=%8.2f avail=%8.2f\n",
	            be.nodename,capacity,used,avail) ;
#endif

		if (((! (flags & 1)) || (! (be.flags & MSFLAG_DISABLED))) &&
			((! (flags & 2)) || (empty > 0.0))) {

	            c += 1 ;

#if	CF_DEBUGS
	            debugprintf("msfile_best: best node=%s avail=%8.2f\n",
	                be.nodename,avail) ;
#endif

	        if (avail > bestavail) {

	            bestavail = avail ;
	            memcpy(ep,&be,sizeof(MSFILE_ENT)) ;

	        } /* end if (better) */

		} /* end if (found one at all) */

	    } /* end while */

	    msfile_curend(msp,&cur) ;

	    if (rs == SR_NOENT)
	        rs = SR_OK ;

	} /* end if */

	if (c == 0)
		rs = SR_NOENT ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (msfile_best) */


