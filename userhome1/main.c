/* main (userhome) */

/* generic front-end for SHELL built-ins */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	This was written to facilitate a KSH builtin command by the name
	of LOGDIR.  This is supposed to be more convenient than previous
	non-builtin versions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ userhome [<username>|-] [<qopt(s)>] [-af <afile>]


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"b_username.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	p_userhome(int,const char **,const char **,void *) ;


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


	ex = p_userhome(argc,argv,envv,NULL) ;

	return ex ;
}
/* end subroutine (main) */


