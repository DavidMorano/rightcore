/* uc_isatty */

/* interface component for UNIX® library-3c */
/* get the filename (path) of a terminal device */


/* revision history:

	= 1998-11-28, David A­D­ Morano
	How did we get along without this for over 10 years?

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine determines if a supplied file-descriptor is associated
        with a terminal or not.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<unistd.h>
#include	<vsystem.h>


/* local defines */


/* forward references */


/* exported subroutines */


int uc_isatty(int fd)
{
	int	rs ;

	if ((rs = isatty(fd)) != 0) {
	    rs = (- rs) ;
	}

	return rs ;
}
/* end subroutine (uc_isatty) */


