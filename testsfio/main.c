/* main */

/* generic front-end for SHELL built-ins */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-28, David A­D­ Morano

	This subroutine was written for use as a front-end for Korn
	Shell (KSH) commands that are compiled as stand-alone programs.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ testsfio [file(s)]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	b_testsfio(int,const char **,void *) ;


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


	ex = b_testsfio(argc,argv,NULL) ;

	return ex ;
}
/* end subroutine (main) */



