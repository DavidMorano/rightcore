/* testuss */


/* revision history:

	= 2002-09-07, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>


/* local defines */

#define	ADDRESS	"/tmp/unix"



extern int	listenuss(const char *,int,int) ;



int main()
{
	bfile		outfile ;
	int		rs ;

	if ((rs = bopen(&outfile,BFILE_STDOUT,"dwct",0666)) >= 0) {
	    if ((rs = listenuss(ADDRESS,0666,0)) >= 0) {
		bprintf(&outfile,"rs=%d\n",rs) ;
		u_close(rs) ;
	    }
	    bclose(&outfile) ;
	}

	return 0 ;
}
/* end subroutine (main) */


