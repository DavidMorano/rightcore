/* opendefstds */

/* open default standard files */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano
        This subroutine was borrowed and modified from previous generic
        front-end 'main' subroutines!

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine opens the default standard files up to a certain
	file-descriptor number (generally should be '3').


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int opendefstds(int n)
{
	int		rs = SR_OK ;
	int		i ;
	for (i = 0 ; (rs >= 0) && (i < n) ; i += 1) {
	    if (u_fcntl(i,F_GETFD,0) == SR_BADF) {
	        int of = (i == 0) ? O_RDONLY : O_WRONLY ;
	        rs = u_open(NULLFNAME,of,0666) ;
	    }
	} /* end if */
	return rs ;
}
/* end subroutine (opendefstds) */


