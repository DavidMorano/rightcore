/* getourenv */

/* get (retrieve) a local environment variables */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-11-04, David A­D­ Morano
        This code is generalized from one of my old programs. Why the need for
        this? Because with the introduction of loadable commands in Korn Shell
        (KSH), we also needed a local source for an environment.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is similar to the standard library 'getenv(3c)'
        subroutine but uses a local environment array instead.

	Synopsis:

	const char *getourenv(envv,key)
	const char	**envv ;
	const char	*key ;

	Arguments:

	envv		environment array
	var		key-string to lookup

	Returns:

	NULL		key not found in environment database
	-		pointer to value for given key


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<string.h>
#include	<vsystem.h>
#include	<localmisc.h>		/* extra types */


/* local defines */


/* external subroutines */

extern int	matkeystr(cchar **,cchar *,int) ;


/* local structures */


/* forward references */


/* exported subroutines */


cchar *getourenv(cchar **envv,cchar *key)
{
	cchar		*vp = NULL ;
	if ((envv != NULL) && (key[0] != '\0')) {
	    int		i ;
	    if ((i = matkeystr(envv,key,-1)) >= 0) {
		if ((vp = strchr(envv[i],'=')) != NULL) {
		    vp += 1 ;
		}
	    }
	}
	return vp ;
} 
/* end subroutine (getourenv) */


