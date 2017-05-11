/* sfrootname */

/* string-find root-name */


/* revision history:

	= 1998-03-17, David A­D­ Morano
	I enhanced this somewhat from my previous version.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Find the root-name from a program-root.

	Synopsis:

	int sfrootname(sp,sl,rpp)
	const char	*sp ;
	int		sl ;
	const char	**rpp ;

	Arguments:

	sp		string
	sl		string-length
	rpp		pointer to result pointer

	Returns:

	-		length of found string


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */

extern int	sfdirname(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;


/* local variables */


/* exported subroutines */


int sfrootname(cchar *sp,int sl,cchar **rpp)
{
	int	cl ;

	if (sp == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;

	cl = sfbasename(sp,sl,rpp) ;

	return cl ;
}
/* end subroutine (sfrootname) */


