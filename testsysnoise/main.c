/* main */

/* test the 'sysnoise()' subroutine */


#define	CF_DEBUGS	1		/* non-switchable debug print-outs */


/* revision history:

	= 1995-02-05, David A­D­ Morano

	I put this together to see if this old object still works
	as intended !


*/

/* Copyright © 1995 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine is the main part of a program to test the
	'sysnoise()' subroutine.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<dirent.h>
#include	<string.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	sysnoise(uint *,int);


/* forward references */


/* exported subroutines */


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	outfile, *ofp = &outfile ;

	uint	noise[10] ;

	int	rs, i, n ;
	int	ex = EX_DATAERR ;
	int	fd_debug ;
	int	seed ;

	char	*progname ;
	char	buf[MAXPATHLEN + 1] ;
	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	progname = argv[0] ;

	if (bopen(ofp,BFILE_STDOUT,"dwct",0666) >= 0)
	    bcontrol(ofp,BC_LINEBUF,0) ;


#if	CF_DEBUGS
	debugprintf("main: entered\n") ;
#endif


	seed = 0 ;
	if ((argc >= 2) && (argv[1] != NULL)) {

	    cfdeci(argv[1],-1,&seed) ;

#if	CF_DEBUGS
	    debugprintf("main: seed=%d\n",seed) ;
#endif

	}


	n = sysnoise(noise,10) ;

#if	CF_DEBUGS
	    debugprintf("main: n=%d\n",n) ;
#endif

	bprintf(ofp,"n=%d\n",n) ;

	for (i = 0 ; i < n ; i += 1)
		bprintf(ofp,"%08x\n",noise[i]) ;


#if	CF_DEBUGS
	debugprintf("main: done\n") ;
#endif


	ex = EX_OK ;

done:
	bclose(ofp) ;

	return ex ;
}
/* end subroutine (main) */



