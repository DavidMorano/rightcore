/* uc_resolvepath */

/* interface component for UNIX® library-3c */
/* resolve a path without symbolic components */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine takes an existing path and creates a new path that does
        not contain any symbolic components.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */


/* exported subroutines */


int uc_resolvepath(input,output,bufsize)
const char	input[] ;
char		output[] ;
int		bufsize ;
{
	int	rs ;


	if ((input == NULL) || (output == NULL))
		return SR_FAULT ;

	if ((rs = resolvepath(input,output,bufsize)) < 0)
		rs = (- errno) ;

	if ((rs >= 0) && (rs < bufsize))
		output[rs] = '\0' ;

	return rs ;
}
/* end subroutine (uc_resolvepath) */


