/* uc_realpath */

/* interface component for UNIX® library-3c */
/* resolve a path without symbolic or relative components */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine takes an existing path and creates a new path
	that does not contain either symbol or relative components.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */


int uc_realpath(input,output)
const char	input[] ;
char		output[] ;
{
	int	rs = SR_OK ;


	if ((input == NULL) || (output == NULL))
		return SR_FAULT ;

	if (realpath(input,output) == NULL)
		rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_realpath) */


