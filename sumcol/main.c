/* main */

/* generic front-end */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 2004-02-01, David A­D­ Morano

	The program was written from scratch.


*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a fairly generic front-end subroutine for a program.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>
#include	<math.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	fmeanvaral(ULONG *,int,double *,double *) ;

extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	process(struct proginfo *,KEYOPT *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* forward references */

static int	usage(struct proginfo *) ;


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"option",
	"set",
	"follow",
	"af",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_option,
	argopt_set,
	argopt_follow,
	argopt_af,
	argopt_of,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ 0, 0 }
} ;

static const char	*akopts[] = {
	"average",
	"reduce",
	NULL
} ;

enum akopts {
	akopt_average,
	akopt_reduce,
	agopt_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	KEYOPT	akopts ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	ULONG	lx, ly ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, i ;
	int	nfiles = 0 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	const char	*pr = NULL ;
	const char	*searchname = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;


	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}


/* early things to initialize */

	pip->ofp = ofp ;

	pip->verboselevel = 1 ;
	pip->ncol = 2 ;
	pip->compressor = 0 ;

/* process program arguments */

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto ret2 ;
	}

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
	    argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {

	        if (isdigit(argp[1])) {

	            rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	        } else if (argp[1] == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                f_optequal = TRUE ;
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	            } else {
	                avp = NULL ;
	                avl = 0 ;
	                akl = aol ;
	            }

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,aop,aol)) >= 0) {

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;

	                    break ;

/* verbose */
	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

	                    }

	                    break ;

/* temporary directory */
	                case argopt_tmpdir:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->tmpdname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pip->tmpdname = argp ;

	                    }

	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* the user specified some key options */
	                case argopt_option:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }

	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        rs = keyopt_loads(&akopts,
	                            argp,argl) ;

	                    break ;

/* argument files */
	                case argopt_af:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            afname = argp ;

	                    }

	                    break ;

/* output file */
	                case argopt_of:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            ofname = argp ;

	                    }

	                    break ;

/* follow symbolic links */
	                case argopt_follow:
	                    pip->f.follow = TRUE ;
	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *aop) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl, 
	                                    &pip->debuglevel) ;

	                        }

	                        break ;

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* optional division */
	                    case 'd':
	                        pip->f.divide = TRUE ;
	                        break ;

/* ignore mode */
	                    case 'i':
	                        pip->f.ignore = TRUE ;
	                        break ;

/* no-change */
	                    case 'n':
	                        pip->f.nochange = TRUE ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            rs = keyopt_loads(&akopts,argp,argl) ;

	                        break ;

/* reduce domain variable */
	                    case 'c':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            rs = cfdeci(argp,argl,
	                                &pip->compressor) ;

	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                        }

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        bprintf(pip->efp,"%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

	                    } /* end switch */

	                    akp += 1 ;
	                    if (rs < 0)
	                        break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits or key options) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (rs < 0)
	    goto badarg ;

/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: finished parsing command line arguments\n") ;
	debugprintf("main: npa=%d\n",npa) ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,
	        "%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bcontrol(pip->efp,BC_LINEBUF,0) ;

	    bflush(pip->efp) ;

	}

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,searchname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pr=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    bprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	}

/* help file */

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_help || f_version || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if (argvalue >= 1)
	    pip->ncol = argvalue ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* get ready */

#ifdef	COMMENT
	if ((rs = paramopt_havekey(&aparams,PO_SUFFIX)) > 0) {
	    pip->f.suffix = TRUE ;
	} /* end if */

	if ((rs = paramopt_havekey(&aparams,PO_OPTION)) > 0) {
	    PARAMOPT_CUR	cur ;

	    paramopt_curbegin(&aparams,&cur) ;

	    while (paramopt_enumvalues(&aparams,PO_OPTION,&cur,&cp) >= 0) {

	        if (cp == NULL) continue ;

	        if ((kwi = matostr(argopts,2,cp,-1)) >= 0) {

	            switch (kwi) {

	            case progopt_follow:
	                pip->f.follow = TRUE ;
	                break ;

	            case progopt_nofollow:
	                pip->f.follow = FALSE ;
	                break ;

	            } /* end switch */

	        } /* end if (progopts) */

	    } /* end while */

	    paramopt_curend(&aparams,&cur) ;

	} /* end if (progopts) */
#endif /* COMMENT */

/* pre-processing */

	pip->pairs = NULL ;

/* OK, we do it */

	nfiles = 0 ;
	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = process(pip,&akopts,cp) ;
	    if (rs < 0)
	        break ;

	    nfiles += 1 ;

	} /* end for (looping through requested circuits) */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {

	    bfile	afile ;


	    if (strcmp(afname,"-") != 0)
	        rs = bopen(&afile,afname,"r",0666) ;

	    else
	        rs = bopen(&afile,BFILE_STDIN,"dr",0666) ;

	    if (rs >= 0) {

	        int	len ;

	        char	linebuf[LINEBUFLEN + 1] ;


	        while ((rs = breadline(&afile,linebuf,LINEBUFLEN)) > 0) {
	            len = rs ;

	            if (linebuf[len - 1] == '\n') len -= 1 ;
	            linebuf[len] = '\0' ;

		    cp = linebuf ;
	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            pan += 1 ;
	            rs = process(pip,&akopts,cp) ;
	            if (rs < 0)
	                break ;

	            nfiles += 1 ;

	        } /* end while (reading lines) */

	        bclose(&afile) ;

	    } else {

	        bprintf(pip->efp,
	            "%s: could not process argument list file (%d)\n",
	            pip->progname,rs) ;

	        bprintf(pip->efp,"\trs=%d argfile=%s\n",rs,afname) ;

	    }

	} /* end if (processing file argument file list) */

	if (pan == 0)
	    goto badnofiles ;

	if (rs < 0)
	    goto done ;

/* post-processing */

	if ((ofname == 0) || (ofname[0] == '\0'))
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0644) ;

	else
	    rs = bopen(ofp,ofname,"wct",0644) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: could not open output (%d)\n",
	        pip->progname,rs) ;
	    goto badoutopen ;
	}

/* optional debugging output */

	if (pip->debuglevel > 0) {

	    ULONG	*a ;

	    int		size ;

	    double	mean, var, sd ;


	    size = pip->n * sizeof(ULONG) ;
	    if ((rs = uc_malloc(size,&a)) >= 0) {

	        for (i = 0 ; i < pip->n ; i += 1)
	            a[i] = (ULONG) pip->pairs[i].sum ;

	        fmeanvaral(a,pip->n,&mean,&var) ;

	        sd = sqrt(var) ;

	        bprintf(pip->efp,
	            "%s: mean=%12.4f var=%12.4f sd=%12.4f\n",
	            pip->progname,mean,var,sd) ;

	        uc_free(a) ;

	    } /* end if (allocation) */

	} /* end if (debugging -- mean/var) */

/* optional reduction */

	if ((rs >= 0) && (pip->compressor > 0)) {

	    int		j, k ;

	    double	sum ;


	    i = 0 ;
	    k = 0 ;
	    while (i < pip->n) {

	        sum = 0.0 ;
	        j = 0 ;
	        while ((j < pip->compressor) && (i < pip->n)) {

	            sum += pip->pairs[i].sum ;
	            i += 1 ;
	            j += 1 ;

	        } /* end while */

	        pip->pairs[k].sum = sum ;
	        k += 1 ;

	    } /* end while */

	    pip->n = k ;

	} /* end if (reduce by factor) */

/* optional divide */

	if ((rs >= 0) && pip->f.divide && (nfiles > 0)) {

	    double	dnom ;


	    dnom = (double) nfiles ;
	    for (i = 0 ; i < pip->n ; i += 1)
	        pip->pairs[i].sum = pip->pairs[i].sum / dnom ;

	} /* end if (division) */

/* output */

	if (rs >= 0) {

	    for (i = 0 ; i < pip->n ; i += 1) {

	        if (pip->f.fdec) {

	            bprintf(ofp,"%12.4f %12.4f\n",
	                pip->pairs[i].x,pip->pairs[i].sum) ;

	        } else {

	            lx = (ULONG) pip->pairs[i].x ;
	            ly = (ULONG) pip->pairs[i].sum ;

	            bprintf(ofp,"%12llu %12llu\n",lx,ly) ;

	        }

	    } /* end for */

	} /* end if */

	bclose(ofp) ;

	if (pip->pairs != NULL) {
	    uc_free(pip->pairs) ;
	    pip->pairs = NULL ;
	}

badoutopen:
done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

/* we are out of here */
retearly:
	if (pip->open.akopts)
	    keyopt_finish(&akopts) ;

ret2:
	if (pip->efp != NULL)
	    bclose(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	goto retearly ;

badnofiles:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: no files were specified\n",
	    pip->progname) ;

	goto ret1 ;


}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	wlen = 0 ;
	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [<file(s)> ...] [-af <argfile>] [-d] [-Vv]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s: \t[-c factor] [-d]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */



