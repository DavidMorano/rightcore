/* main */


#define	CF_DEBUGS	1		/* compile-time */
#define	CF_DEBUG	1		/* run-time */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdio.h>

#include	"msghdrfold.h"


/* local defines */

#define	VARDEBUGFNAME	"TESTMSGHDRFOLD_DEBUGFILE"

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* exported subroutines */


int main()
{
	MSGHDRFOLD	hf ;

	int	rs ;
	int	ll ;
	int	linelen = LINEBUFLEN ;
	int	in = 1 ;
	int	cols = 76 ;

	const char	*hdr = "here is 	thing to watch"
		" and play with other time seeing as things are indeed"
		" as bad as they are there\n"
		" before long "
		"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
		"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
		" but not so bad as to make a mends\n" ;

	char	linebuf[100] ;
	char	*lp ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = msghdrfold_init(&hf,hdr,-1) ;
	if (rs >= 0) {

		while ((ll = msghdrfold_get(&hf,(cols-in),in,&lp)) > 0) { 

#if	CF_DEBUGS
	debugprintf("main: ll=%u line=>%t<\n",ll,
		lp,strlinelen(lp,ll,40)) ;
#endif

			snwcpy(linebuf,LINEBUFLEN,lp,ll) ;
			fprintf(stdout," %s\n",linebuf) ;

		} /* end while */

		msghdrfold_free(&hf) ;

	} /* end if */
	
	fflush(stdout) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (main) */


