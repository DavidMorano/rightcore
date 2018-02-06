/* boxname */

/* make a mailbox name out of something */


#define	CF_DEBUG	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This file contains several short functions relating mailbox names to
        UNIX® filenames.

*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<string.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;


/* external variables */


/* exported subroutines */


char *maildir(pip)
struct proginfo	*pip ;
{


	return pip->folderdname ;
}


/* returns complete UNIX pathname (fullname) of specified mailbox (boxname) */
int full_boxname(pip,fullname, boxname)
struct proginfo	*pip ;
char		fullname[] ;
const char	boxname[] ;
{
	struct ustat	ss ;

	int	rs = SR_NOENT ;
	int	cl ;

	char	*cp ;


	if (fullname == NULL)
	    return SR_FAULT ;

	fullname[0] = '\0' ;
	if (boxname == NULL)
	    return SR_FAULT ;

	cl = sfshrink(boxname,-1,&cp) ;

	if (cl && (cp[0] == '\0') && (pip->mbname_cur != NULL)) {
		cp = pip->mbname_cur ;
		cl = strlen(cp) ;
	}

	if (cl > 0)
	rs = mkpath2(fullname, pip->folderdname,cp) ;

ret0:
	return rs ;
}
/* end subroutine (full_boxname) */


/* returns mailbox name (boxname) from complete UNIX pathname (fullname) */
int mail_boxname(boxname, fullname)
char		boxname[] ;
const char	fullname[] ;
{
	int	rs = SR_NOENT ;
	int	cl ;

	char	*cp ;


	if (boxname == NULL)
	    return SR_FAULT ;

	boxname[0] = '\0' ;
	if (fullname == NULL)
	    return SR_FAULT ;

	boxname[0] = '\0' ;
	cl = sfbasename(fullname,-1,&cp) ;

	if (cl > 0)
		rs = sncpy1(boxname,MAXNAMELEN,cp) ;

ret0:
	return rs ;
}
/* end subroutine (mail_boxname) */



