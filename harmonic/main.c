/* main */

/* calculate the harmonic mean of given input number sequence */


#define	CF_DEBUGS	0


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<cfdec.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#define	MAXENTRIES	30

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	nextfield(const char *,int,const char **) ;

extern double	fhm(double *,int) ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	double		a[MAXENTRIES + 1] ;
	double		result ;
	const int	llen = LINEBUFLEN ;
	int		ex = EX_OK ;
	int		rs = SR_OK ;
	int		i ;
	int		n = 0 ;
	int		sl, cl ;
	int		ll ;
	int		f_bad = FALSE ;
	const char	*sp, *cp ;
	char		lbuf[LINEBUFLEN + 1] ;

	if (argc < 2) {

	    while ((! f_bad) && ((ll = fgetline(stdin,lbuf,llen)) > 0)) {

	        sp = lbuf ;
	        sl = ll ;
	        while ((! f_bad) && ((cl = nextfield(sp,sl,&cp)) > 0)) {
		    double	fv ;
	            if (n > MAXENTRIES) break ;

		    rs = cfdecf(cp,cl,&fv) ;
	            a[n++] = fv ;

	            sl -= (cp + cl + 1 - sp) ;
	            sp = (cp + cl + 1) ;

		    if (rs < 0) break ;
	        } /* end while */

		if (rs < 0) break ;
	    } /* end while */

	} else {
	    double	fv ;

	    for (i = 1 ; argv[i] != NULL ; i += 1) {
	        if (n > MAXENTRIES) break ;

#if	CF_DEBUGS
		fprintf(stderr,"str=%s\n",argv[i]) ;
#endif

		    rs = cfdecf(cp,cl,&fv) ;
	            a[n++] = fv ;

		if (rs < 0) break ;
	    } /* end for */

	} /* end if */

	if (rs >= 0) {
	    if (n < MAXENTRIES) {
	        if (n > 0) {

#if	CF_DEBUGS
		    for (i = 0 ; i < n ; i += 1) {
			fprintf(stderr,"n[%d]=%12.4f\n", i,a[i]) ;
		    }
#endif

	            result = fhm(a,n) ;
	            fprintf(stdout,"%12.4f\n",result) ;

	        } else {
	            fprintf(stdout,"no numbers specified\n") ;
		    rs = SR_INVALID ;
		}
	    } else {
	        fprintf(stdout,"too many number were given\n") ;
		rs = SR_TOOBIG ;
	    }
	} else {
	    fprintf(stdout,"a bad number was given\n") ;
	}

	if (rs < 0) ex = EX_DATAERR ;
	return ex ;
}
/* end subroutine (main) */


