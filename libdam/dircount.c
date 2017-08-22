/* dircount */

/* return the count of (non-overhead) files in the given directory */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We return the count of non-overhead files in the given directory.

	Synopsis:

	int dircount(cchar *dname)

	Arguments:

	dname		directory name (as a string)

	Returns:

	<0		error
	>=		count of files


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<fsdir.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	mkpath1(char *,cchar *) ;
extern int	pathadd(char *,int,cchar *) ;
extern int	hasNotDots(cchar *,int) ;
extern int	isNotValid(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external variables */


/* global variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int dircount(cchar *dname)
{
	FSDIR		d ;
	FSDIR_ENT	de ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if (dname == NULL) return SR_FAULT ;
	if (dname[0] == '\0') return SR_FAULT ;
	if ((rs = fsdir_open(&d,dname)) >= 0) {
	    while ((rs = fsdir_read(&d,&de)) > 0) {
	       if (hasNotDots(de.name,rs) > 0) {
		    c += 1 ;
		} /* end (not dots) */
		if (rs < 0) break ;
	    } /* end while */
	    rs1 = fsdir_close(&d) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (fsdir) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (dircount) */


