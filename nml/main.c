/* main */

/* calculate number of ML BBs before DEE */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	= 2002-01-28, David A­D­ Morano

	I started this as a fast way to calculte this thing.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<math.h>
#include	<stdio.h>

#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* external subroutines */

extern int	cfdecf(const char *,int,double *) ;

extern char	*strbasename(char *) ;






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	double	bpa, bma ;
	double	lbpa, lbma ;
	double	nf ;
	double	cumprob ;

	int	rs, i ;
	int	ex = EX_INFO ;
	int	fd_deb ;

	char	*progname ;
	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
		cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_deb) >= 0))
	    debugsetfd(fd_deb) ;



	progname = strbasename(argv[0]) ;

	if (argc < 2)
		return EX_USAGE ;

	cfdecf(argv[1],-1,&bpa) ;

	if (bpa > 1.0)
		goto badargval ;

	bma = 1.0 - bpa ;

	fprintf(stdout,"bpa=%12.4f bma=%12.4f\n",bpa,bma) ;

	lbma = log(bma) ;

	lbpa = log(bpa) ;

	fprintf(stdout,"lbpa=%12.4f lbma=%12.4f\n",lbpa,lbma) ;

	nf = log(bma) / log(bpa) ;

	fprintf(stdout,"bb=%12.4f\n",nf) ;

/* integer iterative method */

	i = 1 ;
	cumprob = bpa ;
	while (TRUE) {

		cumprob = cumprob * bpa ;

#if	CF_DEBUGS
		debugprintf("main: cp(%d)=%17.4f\n",i,cumprob) ;
#endif

		if (cumprob < bma)
			break ;

		i += 1 ;

	} /* end while */

	fprintf(stdout,"bb=%d\n",i) ;

/* human check */

	{
		double	fi = (double) i ;


		cumprob = pow(bpa,fi) ;

	} /* end block */

	fprintf(stdout,"cp(%d)=%12.4f\n",i,cumprob) ;

	fprintf(stdout,"cp(%d)=%12.4f\n",(i + 1),(cumprob * bpa)) ;

/* Professor Uht's formulation */

	{
		double	l, hdee ;


		hdee = 4.0 ;
		l = hdee + ( log(1.0 - bpa) / log(bpa) ) - 1.0 ;

		fprintf(stderr,"hdee=%8.4f l=%8.4f\n",
			hdee,l) ;

	} /* end block */



	ex = EX_OK ;

ret2:
	fclose(stdout) ;

ret1:
	fclose(stderr) ;

	return ex ;

badargval:
	ex = EX_USAGE ;
	fprintf(stderr,"%s: invalid branch prediction accuracy given\n",
		progname) ;

	goto ret1 ;
}
/* end subroutine (main) */




