/* main (KSHLD) */

/* **unfinished** */


/* revision history:

	= 2017-09-07, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This program tries to load the KSH program with the LD_LIBRARY_PATH set
        to something so that KSH doesn't f**k up on startup!


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<stdlib.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	int		rs ;
	int		i ;
	cchar		*cp ;
	char		execfname[MAXPATHLEN + 1] ;

	rs = u_execve(execfname,argc,argv,ev) ;

	return 0 ;
}
/* end subroutine (main) */


