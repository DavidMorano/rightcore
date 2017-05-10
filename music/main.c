/* main (music tones) */


#define	CF_DEBUGS	0		/* compile-time */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<math.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NUMLIMIT	7
#define	FREQNAME	"a440"
#define	DIFFEPP		0.001


/* external subroutines */

extern int	sfbasename(cchar *,int,cchar **) ;
extern int	cfdeci(cchar *,int,int *) ;

extern char	*strcpyuc(char *,const char *) ;

extern char	*strbasename(char *) ;


/* local structures */

struct scale {
	double	freq ;
	double	interval ;
	int	num, den ;
	char	name[10] ;
	char	nameinterval[10] ;
} ;

struct freqinfo {
	char	*name ;
	int	index ;
	double	freq ;
} ;

struct ratio {
	int	n, d ;
	double	ref ;
	double	ratio ;
	double	diff ;
} ;


/* forward references */

static int findratio(struct ratio *,int,double,double) ;


/* local variables */

static const struct freqinfo 	freqs[] = {
	    { "a440", 9, 440.0 },
	    { "c256", 0, 256.0 },
	    { NULL, 0, 0.0 }} ;

enum freqs {
	freq_a440,
	freq_c256,
	freq_overlast
} ;

static const char	*names[] = {
	        "C",
	        "C#",
	        "D",
	        "D#",
	        "E",
	        "F",
	        "F#",
	        "G",
	        "G#",
	        "A",
	        "A#",
	        "B"
} ;

static const char	*nameints[] = {
	        "1st",
	        "1st#",
	        "2nd",
	        "2nd#",
	        "3rd",
	        "4th",
	        "4th#",
	        "5th",
	        "5th#",
	        "6th",
	        "6th#",
	        "7th",
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	struct ratio	r ;
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;

	double	scale[12] ;
	double	exponent ;
	double	basefreq ;

	int	i ;
	int	n, d ;
	int	incr = 0 ;
	int	fi ;
	int	numlimit = NUMLIMIT ;
	int	fd_debug ;

	cchar	*progname ;
	cchar	*freqname = FREQNAME ;
	cchar	*cp ;
	char	buf[100] ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	sfbasename(argv[0],-1,&progname) ;

	bopen(efp,BFILE_STDERR,"wca",0666) ;

	bopen(ofp,BFILE_STDOUT,"wct",0666) ;

#if	CF_DEBUGS
	debugprintf("main: argc=%d\n argv[1]=%t\n",argc,
	    argv[1],((argc > 1) ? strlen(argv[1]) : 0)) ;
#endif

	if (argc > 1) {

	    if (cfdeci(argv[1],-1,&numlimit) < 0)
	        numlimit = NUMLIMIT ;

#if	CF_DEBUGS
	    debugprintf("main: numlimit=%d\n",numlimit) ;
#endif

	}

/* find the frequency base we want to use */

	for (fi = 0 ; freqs[fi].name != NULL ; fi += 1) {

	    if (strcasecmp(freqname,freqs[fi].name) == 0)
	        break ;

	}

	if (freqs[fi].name == NULL)
	    goto badfreq ;

	basefreq = freqs[fi].freq ;


/* go do it */

	bprintf(ofp,"Key is in C\n") ;

	strcpyuc(buf,freqname) ;

	bprintf(ofp,"Frequency base %s\n", buf) ;

	bprintf(ofp,"Ratios to %d-limit\n",numlimit) ;

	for (i = incr ; i < 12 ; i += 1) {
	    int	en ;

	    en = i - freqs[fi].index - incr ;
	    exponent = ((double) en) / 12.0 ;
	    scale[(i % 12)] = basefreq * pow(2.0,exponent)  ;

/* find the closest "natural" ratio */

	    findratio(&r,numlimit,scale[0],scale[i % 12]) ;

/* print stuff out for this note */

	    bprintf(ofp,
	        "%2d %-2s %-5s %8.1f  %2d/%2d %8.1f %8.1f (%5.1f%%)\n",
	        i,names[(i % 12)], nameints[(i % 12)],scale[(i % 12)],
	        r.n,r.d,r.ratio,r.diff,(r.diff * 100.0 / r.ref)) ;

	} /* end for */

ret2:
	bclose(ofp) ;

retearly:
ret1:
	bclose(efp) ;

ret0:
	return EX_OK ;

/* bad things */
badfreq:
	bprintf(efp,
	    "%s: unknown base frequency specified (%s)\n",
	    progname, freqname) ;

	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int findratio(rp,numlimit,basefreq,reffreq)
struct ratio	*rp ;
int		numlimit ;
double		basefreq, reffreq ;
{
	double	fn, fd ;
	double	trial, diff, best ;

	int	bestnum, bestden ;
	int	n, d ;

	if (rp == NULL)
	    return -1 ;

	rp->ref = reffreq ;
	if (reffreq < basefreq) {
	    rp->ref /= 2.0 ;
	    basefreq /= 2.0 ;
	}

#if	CF_DEBUGS
	debugprintf("findratio: basefreq=%8.1f reffreq=%8.1f\n",
	    basefreq,reffreq) ;
#endif

	best = 200.0 ;
	for (n = 1 ; n < numlimit ; n += 1) {

	    for (d = 1 ; d < numlimit ; d += 1) {

	        fn = (double) n ;
	        fd = (double) d ;

	        trial = basefreq * (fn / fd) ;
	        diff = trial - reffreq ;

#if	CF_DEBUGS
	        debugprintf("findratio: n=%u d=%u\n",n,d) ;
	        debugprintf("findratio: trial=%8.1f diff=%8.1f\n",
	            trial,diff) ;
#endif

	        if (fabs(diff) < (fabs(best) - DIFFEPP)) {

#if	CF_DEBUGS
	            debugprintf("findratio: diff=%8.1f best=%8.1f\n",
			diff,best) ;
#endif

	            best = diff ;

	            rp->n = n ;
	            rp->d = d ;
	            rp->diff = diff ;
	            rp->ratio = trial ;
	        }

	    } /* end for */

	} /* end for */

	return 0 ;
}
/* end subroutine (findratio) */


