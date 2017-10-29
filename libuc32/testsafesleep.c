/* testsafesleep (C89) */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/time.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<mtime.h>
#include	<cfdec.h>
#include	<localmisc.h>

int main(int argc,cchar **argv,cchar **envv)
{
	FILE		*ofp = stdout ;
	hrtime_t	st, et ;
	int		rs = SR_OK ;
	int		ai ;
	const char	*pn = argv[0] ;

	for (ai = 1 ; ai < argc ; ai += 1) {
	    cchar	*ap = argv[ai] ;
	    if ((ap != NULL) && (ap[0] != '\0')) {
	        int		v ;
		if ((rs = cfdecti(ap,-1,&v)) >= 0) {
		    printf("v=%u\n",v) ;
		    st = gethrtime() ;
		    rs = uc_safesleep(v) ;
		    et = gethrtime() ;
		    fprintf(ofp,"e=%llu\n",(et-st)) ;
		}
	    }
	    if (rs < 0) break ;
	} /* end for */

	if (rs < 0)
	fprintf(stderr,"%s: exiting (%d)\n",pn,rs) ;

	return 0 ;
}
/* end subroutine (main) */

