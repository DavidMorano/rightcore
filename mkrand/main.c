/* main (mkrand) */

/* test the random number generator */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_RANDOMVAR	0		/* use our RV generator? */
#define	CF_RANDOM	0		/* UNIX random() ? */
#define	CF_RANDOMSEED	0		/* use random() seed? */
#define	CF_FIXRANDOM	0		/* fix up the UNIX random() ? */
#define	CF_CHEAPER	0		/* cheaper? */
#define	CF_FIXCHEAP	0		/* fix up the cheap thing */
#define	CF_RAND		0		/* UNIX rand() */
#define	CF_RANDMWC	1		/* MWC thing */
#define	CF_RANDOM2	0		/* thread-safe random() */
#define	CF_SETPOLY	0		/* set polynomial */


/* revision history:

	= 1998-02-05, David A­D­ Morano
        I put this together to see if this old object still works as intended!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine is the main part of a program to test the random
        variable generator object.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"randomvar.h"
#include	"cheap.h"
#include	"randmwc.h"
#include	"random.h"
#include	"config.h"


/* local defines */

#define	NWORDS		(8 * 1024)


/* external subroutines */

extern uint	cheaper(uint) ;

extern int	cfdecmfi(char *,int,int *) ;

extern char	*strbasename(char *) ;


/* local structures */


/* forward references */


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"CONFIG",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"LOGFILE",
	"HELP",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_config,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_logfile,
	argopt_help,
	argopt_overlast
} ;

static cchar	*randtypes[] = {
	"randomvar",
	"randmwc",
	"random",
	"cheap",
	NULL
} ;

enum randtypes {
	randtype_randomvar,
	randtype_randmwc,
	randtype_random,
	randtype_cheap,
	randtype_overlast
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	RANDOMVAR	rng ;
	CHEAP		cng ;
	RANDMWC		mwc ;
	RANDOM		std ;
	bfile		outfile, *ofp = &outfile ;
	ULONG		words[NWORDS] ;
	ULONG		rv1, rv2 ;
	time_t		daytime ;
	uint		uiw ;
	uint		ihi, ilo ;

	int	rs ;
	int	ex = 0 ;
	int	len, i, j ;
	int	size, iw ;
	int	flen, seed ;
	int	ex = EX_DATAERR ;

	char	*progname ;
	char	buf[MAXPATHLEN + 1] ;
	char	*rsfname ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	progname = strbasename(argv[0]) ;

	if ((argc < 2) || (argv[1] == NULL) || (argv[1][0] == '\0')) {
	    ex = EX_NOINPUT ;
	    goto ret0 ;
	}

	if (bopen(ofp,argv[1],"wct",0666) < 0) {
	    ex = EX_NOINPUT ;
	    goto ret0 ;
	}

#if	CF_DEBUGS
	debugprintf("main: entered\n") ;
#endif

	flen = -1 ;
	if ((argc >= 3) && (argv[2] != NULL)) {

	    if (cfdecmfi(argv[2],-1,&flen) < 0)
	        flen = -1 ;

#if	CF_DEBUGS
	    debugprintf("main: flen=%d\n",flen) ;
#endif

	} /* end if (getting a file size) */

	if (flen < 0)
	    flen = 13 * 1024 * 1024 ;

	seed = 0 ;

#if	CF_RANDOMSEED
	if ((cp = getenv("RANDOM")) != NULL)
		cfdeci(cp,-1,&seed) ;
#endif


#if	CF_DEBUGS
	debugprintf("main: starting\n") ;
#endif

#if	CF_RANDOMSEED
	srandom(seed) ;
#endif

#ifdef	COMMENT
	if (seed == 0) {

	    daytime = time(NULL) ;

	    seed = (int) daytime ;
	}
#endif /* COMMENT */

	randomvar_start(&rng,TRUE,seed) ;

#if	CF_SETPOLY
	randomvar_setpoly(&rng,66,23) ;
#endif


	cheap_start(&cng,seed) ;

	randmwc_start(&mwc,2,seed) ;

	random_start(&std,4,0) ;


	uiw = 0 ;
	iw = seed ;


#if	CF_DEBUGS
	debugprintf("main: sampling\n") ;
#endif

	len = 0 ;
	while (len < flen) {
		int	wlen ;

	    for (i = 0 ; i < NWORDS ; i += 1) {

#if	CF_RANDOMVAR
	        randomvar_getulong(&rng,words + i) ;
#else

#if	CF_RANDOM

#if	CF_FIXRANDOM
	        randomvar_getuint(&rng,&uiw) ;
#endif

	        rv1 = (ULONG) random_orig() ;

	        rv2 = (ULONG) random_orig() ;

#if	CF_FIXRANDOM
	        rv1 = rv1 | ((uiw & 1) << 31) ;
	        rv2 = rv2 | ((uiw & 2) << 30) ;
#endif

#if	CF_DEBUGS
	        debugprintf("main: rv1=%016llx rv2=%016llx\n",rv1,rv2) ;
#endif

	        words[i] = (rv1 << 32) ^ rv2 ;

#else

#if	CF_CHEAPER

#if	CF_FIXCHEAP
	        randomvar_getuint(&rng,&uiw) ;
#endif

#if	CF_DEBUGS
	        debugprintf("main: i=%d iw=%08x\n",i,iw) ;
#endif

	        ilo = cheaper(iw) ;

	        ihi = cheaper(ilo) ;

#if	CF_DEBUGS
	        debugprintf("main: cheaper ihi=%08x ilo=%08x\n",ihi,ilo) ;
#endif

	        iw = ihi ;

	        rv1 = (ULONG) ihi ;
	        rv2 = (ULONG) ilo ;

#if	CF_FIXCHEAP
	        rv1 = rv1 | ((uiw & 1) << 31) ;
	        rv2 = rv2 | ((uiw & 2) << 30) ;
#endif

#if	CF_DEBUGS
	        debugprintf("main: rv1=%016llx rv2=%016llx\n",rv1,rv2) ;
#endif

	        words[i] = (rv1 << 32) | rv2 ;

#else

#if	CF_RAND

	        ihi = rand_r(&iw) ;

	        ilo = rand_r(&ihi) ;

	        iw = ihi ;

	        rv1 = (ULONG) ihi ;
	        rv2 = (ULONG) ilo ;
	        words[i] = (rv1 << 32) | rv2 ;

#else

#if	CF_RANDMWC

	        randmwc_getulong(&mwc,words + i) ;

#else

#if	CF_RANDOM2

	        random_getuint(&std,&ihi) ;

	        random_getuint(&std,&ilo) ;

	        rv1 = (ULONG) ihi ;
	        rv2 = (ULONG) ilo ;

	        words[i] = (rv1 << 32) ^ rv2 ;

#else

	        cheap_getulong(&cng,words + i) ;

#endif /* CF_RANDOM2 */

#endif /* CF_RANDMWC */

#endif /* CF_RAND */

#endif /* CF_CHEAPER */

#endif /* CF_RANDOM */

#endif /* CF_RANDOMVAR */


#if	CF_DEBUGS
	        debugprintf("main: i=%d rv=%016llx\n",i,words[i]) ;
#endif


	    } /* end for */

	size = (NWORDS * sizeof(ULONG)) ;
	wlen = MIN((flen - len),size) ;
	    bwrite(ofp,words,wlen) ;
	    len += wlen ;
	} /* end while */

	randomvar_finish(&rng) ;

badprogstart:

#if	CF_DEBUGS
	debugprintf("main: done\n") ;
#endif

	ex = EX_OK ;

done:
	bclose(ofp) ;

ret0:
	return ex ;
}
/* end subroutine (main) */


