/* getenver */

/* get the value of an environment variable */


/* revision history:

	= 1998-02-01, David A­D­ Morano
        This subroutine was written from scratch. There are (or may) be some
        other standard ones floating around like it but I could not find one
        that was exactly what I needed (sigh).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is like 'getenv(3c)' except that it also takes an
        additional argument that specifies the length of the environment
        variable string to look up in the environment array.

	Synposis:

	cchar		*getenver(cchar *np,int nl)

	Arguments:

	np		pointer to environment name (key) to lookup
	nl		length of environment name (key) to lookup

	Returns:

	-		pointer to value associated with given name
	NULL		name not found


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<string.h>
#include	<localmisc.h>


/* external variables */

extern cchar	**environ ;


/* external subroutines */

extern int	matkeystr(cchar **,cchar *,int) ;

extern char	*strnchr(cchar *,int,cchar **) ;


/* local variables */


/* exported subroutines */


cchar *getenver(cchar *np,int nl)
{
	cchar		*vp = NULL ;
	if (np != NULL) {
	    int		ei ;
	    cchar	*tp ;
	    if (nl < 0) nl = strlen(np) ;
	    if ((tp = strnchr(np,nl,'=')) != NULL) {
		nl = (tp-np) ;
	    }
	    if ((ei = matkeystr(environ,np,nl)) >= 0) {
	        if ((vp = strchr(environ[ei],'=')) != NULL) {
	            vp += 1 ;
		}
	    }
	} /* end if */
	return vp ;
}
/* end subroutine (getenver) */


