/* matusername */

/* match a user name */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1995-03-13, David A­D­ Morano
	This code module was completely rewritten to replace any original
	garbage that was here before.

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1995,1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<string.h>
#include	<pwd.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"artlist.h"
#include	"defs.h"


/* local defines */


/* external subroutines */


/* local forward references */


/* external variables */


/* global variables */


/* exported subroutines */


int user_match(uid,ulist)
uid_t	uid ;
char	*ulist[] ;
{
	struct passwd	*pp ;

	int	i = 0 ;

	char	*un ;


	if (uid < 0) 
	    uid = getuid() ;

	un = ulist[i] ;
	while ((un != NULL) && (*un != '\0')) {

	    if (((pp = getpwnam(un)) != NULL) &&
	        (pp->pw_uid == uid)) return TRUE ;

	    un = ulist[i++] ;

	} /* end while */

	return FALSE ;
}
/* end subroutine (user_match) */



