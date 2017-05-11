/* rmext */

/* remove file-extension */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-17, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Remove any file extension.  We do this by returning a string length
	that does not include any (former) file extension.

	Synopsis:

	int rmext(cchar *fp,int fl)

	Arguments:

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

extern int	matstr(cchar **,cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
#endif

extern char	*strnchr(cchar *,int,int) ;
extern char	*strnrchr(cchar *,int,int) ;


/* forward references */


/* local variables */

static cchar	*exts[] = {
	"x",
	"s5",
	"s5u",
	"us5",
	"s4",
	"aout",
	"elf",
	"ksh",
	"sh",
	"ksh93",
	"csh",
	"osf",
	NULL
} ;


/* exported subroutines */


int rmext(cchar *sp,int sl)
{
	cchar		*tp ;

	if (sl < 0) sl = strlen(sp) ;

	if ((tp = strnrchr(sp,sl,'.')) != NULL) {
	    const int	el = ((sp+sl)-(tp+1)) ;
	    cchar	*ep = (tp+1) ;
	    if (matstr(exts,ep,el) >= 0) {
	        sl = (tp-sp) ;
	    }
	} /* end if */

	return sl ;
}
/* end subroutine (rmext) */


