/* uc_ttyname */

/* interface component for UNIX® library-3c */
/* get the filename (path) of a terminal device */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        This is written to get a portable (reentrant) version of TTYNAME.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine gets the filepath of a terminal device under '/dev/'.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#include	<vsystem.h>


/* local defines */


/* forward references */


/* exported subroutines */


int uc_ttyname(fd,dbuf,dlen)
int	fd ;
char	dbuf[] ;
int	dlen ;
{
	int	rs ;
	int	len = 0 ;


/* oooh, not like the others! */

#if	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
	{
	    char	*p ;

	    rs = 0 ;
	    errno = 0 ;
	    if ((p = ttyname_r(fd,dbuf,dlen)) == NULL)
		rs = (- errno) ;

	}
#else /* defined(DARWIN) */
	if ((rs = ttyname_r(fd,dbuf,dlen)) != 0)
		rs = (- rs) ;
#endif /* defined(DARWIN) */

/* continue */

	if (rs >= 0)
	    len = strnlen(dbuf,dlen) ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uc_ttyname) */


