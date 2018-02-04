/* delenv */

/* delete an environment variable */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine is used to delete an environment variable permanently
        from the process environment list. There is no convenient way to
        un-delete this later.

	This is an incredibly stupid hack.  This subroutine should *never* be
	used.  It was only written to satisfy some stupid open-source programs
	that I occassionally stoop too low to want to use!


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<stddef.h>
#include	<string.h>


/* external variables */

extern char	**environ ;


/* forward references */

static int	namecmp(const char *,int,const char *) ;


/* exported subroutines */


const char *delenv(const char *name)
{
	int		len ;
	const char	**env = (const char **) environ ;
	const char	*np ;
	const char	*cp ;
	const char	**p2 ;
	const char	**p1 ;


	if ((name == NULL) || (environ == NULL))
	    return NULL ;

	for (np = name ; (*np != '\0') && (*np != '=') ; np += 1) ;

	len = np - name ;
	for (p1 = env ; *p1 != NULL ; p1 += 1) {
	    if (namecmp(name,len,*p1) == 0) break ;
	}

	if (*p1 == NULL)
	    return NULL ;

	cp = *p1 ;

/* find the last entry */

	for (p2 = p1 ; *p2 != NULL ; p2 += 1) ;

	p2 -= 1 ;

/* swap the last entry with this one */

	if (p1 != p2)
	    *p1 = *p2 ;

	*p2 = NULL ;
	return cp ;
}
/* end subroutine (delenv) */


/* local subroutines */


static int namecmp(const char *name,int namelen,const char *entry)
{
	const char	*nep = (name + namelen) ;
	const char	*np = name ;
	const char	*ep = entry ;

	while ((np < nep) && *ep && (*ep != '=')) {
	    if (*np++ != *ep++) return 1 ;
	}

	return 0 ;
}
/* end subroutine (namecmp) */


