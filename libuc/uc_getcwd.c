/* uc_getcwd */

/* interface component for UNIX® library-3c */
/* get the current working directory */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine returns the Current Working Directory (CWD). If you
        wanted the Present Working Directory (PWD), you should be calling
        'getpwd()'.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */


/* exported subroutines */


int uc_getcwd(cwbuf,cwlen)
char		cwbuf[] ;
int		cwlen ;
{
	int	rs = SR_OK ;


	if (cwbuf == NULL)
	    return SR_FAULT ;

	if (getcwd(cwbuf,cwlen) == NULL)
	    rs = (- errno) ;

	if (rs >= 0)
	    rs = strnlen(cwbuf,cwlen) ;

	return rs ;
}
/* end subroutine (uc_getcwd) */


