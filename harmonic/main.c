/* main */


#define	F_DEBUGS	0



#include	<sys/types.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<stdio.h>

#include	<exitcodes.h>

#include	"misc.h"



/* local defines */

#define		MAXENTRIES	30

#ifndef	LINELEN
#define	LINELEN		200
#endif


/* external subroutines */

extern int	nextfield(const char *,int,const char **) ;

extern double	fhm(double *,int) ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	double	a[MAXENTRIES + 1] ;
	double	result ;

	int	ex = EX_INFO ;
	int	rs, i ;
	int	n = 0 ;
	int	sl, cl ;
	int	ll ;
	int	f_bad = FALSE ;

	const char	*sp, *cp ;

	char	linebuf[LINELEN + 1] ;


	if (argc < 2) {

	    while ((! f_bad) && ((ll = fgetline(stdin,linebuf,LINELEN)) > 0)) {

	        sp = linebuf ;
	        sl = ll ;
	        while ((! f_bad) && ((cl = nextfield(sp,sl,&cp)) > 0)) {

	            if (n > MAXENTRIES)
	                break ;

	            cp[cl] = '\0' ;

#if	F_DEBUGS
		fprintf(stderr,"fl=%d str=%s\n",cl,cp) ;
#endif

	            errno = 0 ;
	            a[n++] = atof(cp) ;

	            if (errno != 0) {

	                f_bad = TRUE ;
	                break ;
	            }

	            sl -= (cp + cl + 1 - sp) ;
	            sp = (cp + cl + 1) ;

#if	F_DEBUGS
		fprintf(stderr,"llr=%d\n",sl) ;
#endif

	        } /* end while */

#ifdef	COMMENT
	        if (sl > 0) {

	            if (n > MAXENTRIES)
	                break ;

#if	F_DEBUGS
		fprintf(stderr,"fl=%d str=%s\n",sl,sp) ;
#endif

	            errno = 0 ;
	            a[n++] = atof(sp) ;

	            if (errno != 0) {

	                f_bad = TRUE ;
	                break ;
	            }


	        }
#endif /* COMMENT */

	    } /* end while */

	} else {

	    for (i = 1 ; argv[i] != NULL ; i += 1) {

#if	F_DEBUGS
		fprintf(stderr,"str=%s\n",argv[i]) ;
#endif

	        errno = 0 ;
	        a[n++] = atof(argv[i]) ;

	        if (errno != 0) {

	            f_bad = TRUE ;
	            break ;
	        }

	        if (n > MAXENTRIES)
	            break ;

	    } /* end for */

	} /* end if */

	if ((! f_bad) && (n < MAXENTRIES)) {

	    if (n > 0) {

#if	F_DEBUGS
		for (i = 0 ; i < n ; i += 1)
			fprintf(stderr,"n[%d]=%12.4f\n",
				i,a[i]) ;
#endif

	        result = fhm(a,n) ;

	        fprintf(stdout,"%12.4f\n",result) ;

	    } else
	        fprintf(stdout,"no numbers specified\n") ;

	} else {

		if (f_bad)
	    fprintf(stdout,"a bad number was given\n") ;

		else
	    fprintf(stdout,"too many number were given\n") ;

	}

	fclose(stdout) ;

	fclose(stderr) ;

	return 0 ;
}
/* end subroutine (main) */



