/* sfprogroot */

/* try to get a program-root */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine tries to extract a program-root (directory) out of a
	directory path.  It looks for "bin" and "sbin" in the directory name in
	order to guess if a program-root is present or not.

	Synopsis:

	int sfprogroot(pp,pl,rpp)
	const char	pp[] ;
	int		pl ;
	const char	**rpp ;

	Arguments:

	pp		directory to search
	pl		length of directory to search
	rpp		pointer to hold pointer to result (if found)

	Returns:

	<0		program-root not found
	>=		program-root found and this is its length

	Example:

	{
		const char	*execname = "/usr/bin/sleep" ;
		const char	*dp ;
		int		dl ;

		if ((dl = sfdirname(execname,-1,&dp)) > 0) {
		    int		rl ;
		    const char	*rp ;
		    if ((rl = sfprogroot(dp,dl,&rp)) > 0) {
		        fprintf(stdout,"pr=%s\n",rp) ;
		    }
		}
	}


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int sfprogroot(cchar *pp,int pl,cchar **rpp)
{
	int		dl ;
	int		sl = -1 ;
	cchar		*dp ;
	cchar		*sp = NULL ;
	if ((dl = sfdirname(pp,pl,&dp)) > 0) {
	    int		f ;
	    cchar	*bp ;
	    int		bl ;
	    bl = sfbasename(dp,dl,&bp) ;
	    f = ((bl == 3) && (strncmp(bp,"bin",bl) == 0)) ;
	    if (! f) {
	        f = ((bl == 4) && (strncmp(bp,"sbin",bl) == 0)) ;
	    }
	    if (f) {
	        sl = sfdirname(dp,dl,&sp) ;
	    }
	} /* end if */
	if (rpp != NULL) {
	    *rpp = (sl >= 0) ? sp : NULL ;
	}
	return sl ;
}
/* end subroutine (sfprogroot) */


int getpr(cchar *pp,int pl,cchar **rpp)
{
	return sfprogroot(pp,pl,rpp) ;
}
/* end subroutine (getpr) */


