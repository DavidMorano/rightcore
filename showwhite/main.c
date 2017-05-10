/* main */

/* show the bits set representing terminating characters */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_ISWHITE	1


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The program was written from scratch to do what the previous
	program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a program to show the field terminator characters from
	the C language syntax of the terminator block code.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<ascii.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LINELEN
#define	LINELEN		200
#endif


/* external subroutines */

extern int	sfbasename(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfhexi(const char *,int,int *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;
extern char	*strshrink(char *) ;


/* external variables */


/* local structures */

struct adesc {
	uchar	array[32] ;
	int	i ;
} ;

struct chpairs {
	int		ch ;
	const char	*name ;
} ;


/* local references */

static int	adesc_init(struct adesc *) ;
static int	adesc_line(struct adesc *,char *,int) ;
static int	adesc_free(struct adesc *) ;


/* forward references */


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_overlast
} ;

static const struct chpairs	names[] = {
	{ CH_TAB, "TAB" },
	{ CH_SPACE, "SPACE" },
	{ CH_CR, "CR" },
	{ CH_NL, "NL" },
	{ CH_BS, "BS" },
	{ CH_VT, "VT" },
	{ CH_FF, "FF" },
	{ 0, NULL }
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	struct adesc	array ;

	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	infile, *ifp = &infile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	rs, i, j, len, c ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) == NULL) {
	    if ((cp = getourenv(envv,VARDEBUGFD1)) == NULL)
	        cp = getourenv(envv,VARDEBUGFD2) ;
	}
	if (cp != NULL)
		debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	sfbasename(argv[0],-1,&cp) ;
	pip->progname = cp ;

	pip->efp = efp ;
	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;


/* early things to initialize */

	pip->tmpdname = NULL ;

	pip->debuglevel = 0 ;

	pip->f.verbose = FALSE ;


/* process program arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1)
	    argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	                f_optequal = TRUE ;
	            } else {
	                avl = 0 ;
	                akl = aol ;
	            }

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,aop,aol)) >= 0) {

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal) {
				rs = SR_INVALID ;
				break ;
			    }
	                    break ;

/* verbose */
	                case argopt_verbose:
	                    pip->f.verbose = TRUE ;
	                    if (f_optequal) {
				rs = SR_INVALID ;
				break ;
			    }
	                    break ;

/* temporary directory */
	                case argopt_tmpdir:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->tmpdname = avp ;

	                    } else {

	                        if (argr <= 0)
	                            goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pip->tmpdname = argp ;

	                    }

	                    break ;

/* default action and user specified help */
	                default:
	                    ex = EX_USAGE ;
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
			    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {

	                                rs = cfdec(avp,avl, &pip->debuglevel) ;

	                            }
	                        }

	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->f.verbose = TRUE ;
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        ex = EX_USAGE ;
	                        f_usage = TRUE ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

			    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } else {

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                ex = EX_USAGE ;
	                f_extra = TRUE ;
	                bprintf(efp,"%s: extra arguments specified\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */

	if (rs < 0) goto badarg ;

/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: finished parsing command line arguments\n") ;
#endif

	if (f_version) {
	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto done ;


/* check a few more things */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* OK, we do it */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: opening output\n") ;
#endif

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;

	rs = bopen(ofp,ofname,"dwct",0644) ;

	if (rs < 0) goto badoutopen ;

	for (i = 0 ; i < 256 ; i += 1) {

		int	f ;


#if	CF_ISWHITE
		f = CHAR_ISWHITE(i) ;
#else
	    	f = isspace(i) ;
#endif

		if (f) {

	        bprintf(ofp,"%02x",i) ;
	        for (j = 0 ; names[j].name != NULL ; j += 1) {

	            if (names[j].ch == i)
	                break ;

	        }

	        if (names[j].name != NULL)
	            bprintf(ofp," %s",names[j].name) ;

	        bprintf(ofp,"\n") ;

	    } /* end if */

	} /* end for */


	bclose(ofp) ;


	ex = EX_OK ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: exiting\n") ;
#endif


/* good return from program */
goodret:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    bprintf(pip->efp,"%s: program exiting\n",
	        pip->progname) ;
#endif

/* we are out of here */
done:

retearly:
ret1:
	bclose(pip->efp) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* program usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [-v]\n",
	    pip->progname,pip->progname) ;

	goto retearly ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

badarg:

#if	CF_DEBUGS
	debugprintf("main: exiting program BAD\n") ;
#endif

	ex = EX_USAGE ;
	goto done ;


badoutopen:
	bprintf(pip->efp,"%s: could not open output (rs=%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badinopen:
	bprintf(pip->efp,"%s: could not open standard input (rs=%d)\n",
	    pip->progname,rs) ;



badret:
	ex = EX_DATAERR ;
	goto done ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



/* process a line of input */
static int adesc_line(adp,linebuf,linelen)
struct adesc	*adp ;
char	linebuf[] ;
int	linelen ;
{
	FIELD	fsb ;

	int	rs ;
	int	ai ;
	int	f_hex ;
	int	v ;

	uchar	bterms[32] ;


	ai = adp->i ;

#if	CF_DEBUGS
	debugprintf("adesc_line: index=%d\n",ai) ;
#endif

	fieldterms(bterms,0,",# \t") ;

	if ((rs = field_start(&fsb,linebuf,linelen)) >= 0) {
	    int		fl ;
	    const char	*fp ;

	while ((fl = field_get(&fsb,bterms,&fp)) >= 0) {
	    if (fl == 0) continue ;

#if	CF_DEBUGS
	    debugprintf("adesc_line: fl=%d\n",fl) ;
#endif

#if	CF_DEBUGS
	    debugprintf("adesc_line: non-empty field> %W\n",
	        fsb.fp,fsb.flen) ;
#endif

	    f_hex = FALSE ;
	    f_hex = f_hex || (strncasecmp(fp,"0x",2) == 0) ;

#if	CF_DEBUGS
	    debugprintf("adesc_line: f_hex=%d\n",f_hex) ;
#endif

	    if (f_hex) {

	        rs = SR_EMPTY ;
	        if ((fl -= 2) > 0) {

	            fp += 2 ;
	            rs = cfhexi(fp,fl,&v) ;

#if	CF_DEBUGS
	            debugprintf("adesc_line: hex rs=%d v=\\x%02X\n",rs, v) ;
#endif

	            if (rs >= 0)
	                adp->array[ai++] = (v & 0xFF) ;

	        }

	    } else {

	        rs = cfdeci(fp,fl,&v) ;

	        if (rs >= 0)
	            adp->array[ai++] = (v & 0xFF) ;

	    }

#if	CF_DEBUGS
	    debugprintf("adesc_line: rs=%d\n",rs) ;
#endif

	} /* end while */

	field_finish(&fsb) ;
	} /* end if (field) */

#if	CF_DEBUGS
	debugprintf("adesc_line: final index=%d\n",ai) ;
#endif

	adp->i = ai ;
	return ai ;
}
/* end subroutine (adesc_line) */


static int adesc_init(adp)
struct adesc	*adp ;
{
	int	i ;


	for (i = 0 ; i < 32 ; i += 1)
	    adp->array[i] = 0 ;

	adp->i = 0 ;
	return SR_OK ;
}


static int adesc_free(adp)
struct adesc	*adp ;
{


	return SR_OK ;
}



