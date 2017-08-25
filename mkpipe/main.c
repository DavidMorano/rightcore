/* main */

/* make a FIFO special filo */


#define	CF_DEBUGS	0


/* revision history:

	= 1998-06-01, David A­D­ Morano
        This is originally written. There is no standard program (or any program
        at all) to make named FIFOs on UNIX!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/* ************************************************************************

	This program will create a FIFO special file.


****************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	int		rs = SR_OK ;
	int		ex = EX_OK ;
	const char	*progname = "mkpipe" ;
	const char	*fname ;
	char		*s ;

	if (argc >= 2) {
	    int	i ;
	    for (i = 1 ; (rs >= 0) && (i < argc) ; i += 1) {
	        fname = argv[i] ;
	        if (fname[0] != '\0') {
		    rs = u_mknod(fname,0010666,0) ;
	        }
	    } /* end for */

	} else {
	    fprintf(stderr,"%s: not enough arguments\n",progname) ;
	    ex = EX_DATAERR ;
	}

	return ex ;
}
/* end subroutine (main) */


