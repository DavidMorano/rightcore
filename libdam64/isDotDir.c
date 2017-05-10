/* isDotDir */

/* is a directory entry a dot-directory? */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The subroutine was adapted from others programs that did similar types
        of functions.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines if a directory entry is a dot-directory.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int isDotDir(const char *dname)
{
	int		f = FALSE ;

	if (dname[0] == '.') {
	    f = f || (dname[1] == '\0') ;
	    f = f || ((dname[1] == '.') && (dname[2] == '\0')) ;
	}

	return f ;
}
/* end subroutine (isDotDir) */


