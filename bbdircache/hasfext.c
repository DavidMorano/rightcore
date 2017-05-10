/* hasfext */

/* determine if file-name (just a string) has an approved file-extension */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Given a file-name (a counted string) we determine if it has a
        file-extension of a given class.

	Synopsis:

	int hasfext(exts,fp,fl)
	const char	*exts[] ;
	const char	fp[] ;
	int		fl ;

	Arguments:

	exts		arrays of strings (allowable extensions)
	fp		file-name string
	fl		file-name length

	Returns:

	==0		does not have an approved extension
	>0		has an approved extension (and this is base-str length)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<string.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnrchr(const char *,int,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int hasfext(const char **exts,const char *fp,int fl)
{
	const char	*tp ;
	int		si = 0 ;
	int		f = FALSE ;

	if (fl < 0) fl = strlen(fp) ;

	if ((tp = strnrchr(fp,fl,'.')) != NULL) {
	    const int	el = ((fp+fl)-(tp+1)) ;
	    const char	*ep = (tp+1) ;
	    si = (tp-fp) ;
	    if (el > 0) {
	        f = (matstr(exts,ep,el) >= 0) ;
	    }
	} else {
	    si = fl ;
	    f = (matstr(exts,fp,0) >= 0) ;
	}

#if	CF_DEBUGS
	debugprintf("hasfext: ret f=%u si=%u\n",f,si) ;
#endif
	return (f) ? si : 0 ;
}
/* end subroutine (hasfext) */


