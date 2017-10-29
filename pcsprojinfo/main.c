/* main (pcsprojinfo) */

/* generic front-end for SHELL built-ins */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-28, David Morano

	This subroutine was written for use as a front-end for Korn Shell
	(KSH) commands that are compiled as stand-alone programs.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ pcsprojinfo [-u <username>] [-f] [-s <project>]


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"b_pcsprojinfo.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	p_pcsprojinfo(int,const char **,const char **,void *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	int	ex ;


	ex = p_pcsprojinfo(argc,argv,envv,NULL) ;

	return ex ;
}
/* end subroutine (main) */


