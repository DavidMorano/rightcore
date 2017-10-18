/* main (sumcol) */

/* generic front-end */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 2004-02-01, David A­D­ Morano
	The program was written from scratch.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generic front-end subroutine for a program.

	We sum (generally) the second columns of each specfied file.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<math.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	fmeanvaral(ULONG *,int,double *,double *) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;

extern int	proginfo_setpiv(PROGINFO *,cchar *,const PIVARS *) ;
extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	progfile(PROGINFO *,cchar *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;


/* external variables */


/* forward references */

static int	usage(PROGINFO *) ;

static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *) ;
static int	procerrstat(PROGINFO *) ;
static int	procout(PROGINFO *,cchar *) ;
static int	procreduce(PROGINFO *) ;
static int	procdivide(PROGINFO *,int) ;


/* local variables */

static cchar	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"option",
	"set",
	"follow",
	"sn",
	"af",
	"ef",
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
	argopt_sn,
	argopt_af,
	argopt_ef,
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

#ifdef	COMMENT

static cchar	*akopts[] = {
	"average",
	"reduce",
	NULL
} ;

enum akopts {
	akopt_average,
	akopt_reduce,
	agopt_overlast
} ;

#endif /* COMMENT */


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* early things to initialize */

	pip->verboselevel = 1 ;
	pip->ncol = 2 ;
	pip->compressor = 0 ;

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
	        const int ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp+1) ;

	        } else if (ach == '-') {

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
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
	                            pip->verboselevel = rs ;
	                        }
	                    }
	                    break ;

/* temporary directory */
	                case argopt_tmpdir:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->tmpdname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->tmpdname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* the user specified some key options */
	                case argopt_option:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
				    KEYOPT	*kop = &akopts ;
	                            rs = keyopt_loads(kop,argp,argl) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* argument files */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                afname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                efname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* output file */
	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ofname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* follow symbolic links */
	                case argopt_follow:
	                    pip->f.follow = TRUE ;
	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*aop) ;

	                    switch (kc) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->debuglevel = rs ;
	                            }
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
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
				        KEYOPT	*kop = &akopts ;
	                                rs = keyopt_loads(kop,argp,argl) ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* reduce domain variable */
	                    case 'c':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                pip->compressor = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->verboselevel = rs ;
	                            }
	                        }
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits or key options) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: finished parsing command line arguments\n") ;
	debugprintf("main: npa=%d\n",npa) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp, "%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;
	    bcontrol(pip->efp,BC_LINEBUF,0) ;
	    bflush(pip->efp) ;
	}

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

/* get the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pr=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
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

	if (argval != NULL) {
	    rs = optvalue(argval,-1) ;
	    pip->ncol = rs ;
	}

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* OK, we do it */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    ARGINFO	*aip = &ainfo ;
	    BITS	*bop = &pargs ;
	    cchar	*ofn = ofname ;
	    cchar	*afn = afname ;
	    if ((rs = procargs(pip,aip,bop,afn)) >= 0) {
	        const int	nfiles = rs ;
	        if ((rs = procerrstat(pip)) >= 0) {
	            if ((rs = procreduce(pip)) >= 0) {
	                if ((rs = procdivide(pip,nfiles)) >= 0) {
	                    rs = procout(pip,ofn) ;
	                }
	            } /* end if (procreduce) */
	        } /* end if (procerrstat) */
	    } /* end if (procargs) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

	if (pip->pairs != NULL) {
	    uc_free(pip->pairs) ;
	    pip->pairs = NULL ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            cchar	*fmt = "%s: invalid usage (%d)\n" ;
	            bprintf(pip->efp,fmt,pip->progname,rs) ;
	        }
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_AGAIN:
	        ex = EX_TEMPFAIL ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

/* we are out of here */
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

badarg:
	{
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    ex = EX_USAGE ;
	    fmt = "%s: invalid argument specified (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	}
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<file(s)> ...] [-af <afile>] [-d]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-c <factor>] [-d]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		nfiles = 0 ;
	int		pan = 0 ;
	int		cl ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	cchar		*cp ;

	if (rs >= 0) {
	    int		ai ;
	    int		f ;
	    cchar	**argv = aip->argv ;
	    for (ai = 1 ; ai < aip->argc ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            cp = argv[ai] ;
	            if (cp[0] != '\0') {
	                pan += 1 ;
	                rs = progfile(pip,cp) ;
	                nfiles += 1 ;
	            }
	        }

	        if (rs < 0) break ;
	    } /* end for (looping through requested circuits) */
	} /* end if (ok) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	    if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	                    pan += 1 ;
	                    lbuf[((cp-lbuf)+cl)] = '\0' ;
	                    rs = progfile(pip,cp) ;
	                    nfiles += 1 ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(afp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        fmt = "%s: inaccessible argument-list (%d)\n" ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	        bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	    }

	} /* end if (processing file argument file list) */

	return (rs >= 0) ? nfiles : rs ;
}
/* end subroutine (procargs) */


static int procerrstat(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->debuglevel > 0) {
	    ULONG	*a ;
	    const int	size = pip->n * sizeof(ULONG) ;
	    double	mean, var, sd ;

	    if ((rs = uc_malloc(size,&a)) >= 0) {
	        int	i ;

	        for (i = 0 ; i < pip->n ; i += 1) {
	            a[i] = (ULONG) pip->pairs[i].sum ;
	        }

	        fmeanvaral(a,pip->n,&mean,&var) ;

	        sd = sqrt(var) ;

	        bprintf(pip->efp,
	            "%s: mean=%12.4f var=%12.4f sd=%12.4f\n",
	            pip->progname,mean,var,sd) ;

	        rs1 = uc_free(a) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (m-a-f) */

	} /* end if (debugging -- mean-var) */
	return rs ;
}
/* end subroutine (procerrstat) */


static int procout(PROGINFO *pip,cchar *ofn)
{
	bfile		ofile, *ofp = &ofile ;
	ULONG		lx, ly ;
	int		rs = SR_OK ;
	int		rs1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0')) ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0644)) >= 0) {
	    int	i ;

	    for (i = 0 ; i < pip->n ; i += 1) {

	        if (pip->f.fdec) {

	            bprintf(ofp,"%12.4f %12.4f\n",
	                pip->pairs[i].x,pip->pairs[i].sum) ;

	        } else {

	            lx = (ULONG) pip->pairs[i].x ;
	            ly = (ULONG) pip->pairs[i].sum ;

	            bprintf(ofp,"%12llu %12llu\n",lx,ly) ;

	        }

	        if (rs < 0) break ;
	    } /* end for */

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    bprintf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return rs ;
}
/* end subroutine (procout) */


static int procreduce(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->compressor > 0) {
	    int		i = 0 ;
	    int		j = 0 ;
	    int		k = 0 ;
	    double	sum ;

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
	return rs ;
}
/* end subroutine (procreduce) */


static int procdivide(PROGINFO *pip,int nfiles)
{
	int		rs = SR_OK ;
	if (pip->f.divide && (nfiles > 0)) {
	    const double	dnom = (double) nfiles ;
	    int			i ;

	    for (i = 0 ; i < pip->n ; i += 1) {
	        pip->pairs[i].sum = pip->pairs[i].sum / dnom ;
	    }

	} /* end if (division) */
	return rs ;
}
/* end subroutine (procdivide) */


