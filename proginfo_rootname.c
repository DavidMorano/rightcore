/* proginfo_rootname */

/* program information (extracting the distribution root-name) */


/* revision history:

	= 1998-03-17, David A­D­ Morano
	I enhanced this somewhat from my previous version.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Here we simply extract the root-name of the software distribution from
        our program-root directory.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"defs.h"


/* external subroutines */

extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;


/* local variables */


/* exported subroutines */


int proginfo_rootname(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (pip->pr == NULL) return SR_FAULT ;

	if (pip->rootname == NULL) {
	    int		cl ;
	    cchar	*cp ;
	    if ((cl = sfbasename(pip->pr,-1,&cp)) > 0) {
		cchar	**vpp = &pip->rootname ;
		rs = proginfo_setentry(pip,vpp,cp,cl) ;
		len = rs ;
	    }
	} else {
	    len = strlen(pip->rootname) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (proginfo_rootname) */


