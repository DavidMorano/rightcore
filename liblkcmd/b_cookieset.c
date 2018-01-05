/* b_cookieset (KSH builtin) */

/* typeset a cookie file to TROFF source */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_NOFILL	0		/* turn on no-fill mode */
#define	CF_LEADER	0		/* print leader */
#define	CF_LOCSETENT	0		/* compile |locinfo_setentry()| */


/* revision history:

	= 2000-09-10, David A­D­ Morano
        This subroutine was originally written but it was probably started from
        any one of the numerous subroutines which perform a similar
        "file-processing" fron end.

	= 2017-09-14, David A­D­ Morano
	Converted to a KSH built-in.

*/

/* Copyright © 2000,2014 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program will read the input file and format it into 'troff'
        constant width font style source input language.

	Synopsis:

	$ cookieset <input_file> > <out.dwb>


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<estrings.h>
#include	<char.h>
#include	<bits.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_cookieset.h"
#include	"defs.h"


/* local defines */

#define	DEFMAXLINES	66
#define	MAXLINES	180
#define	DEFPOINT	10

#define	BUFLEN		(MAXPATHLEN + (2 * LINEBUFLEN))

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	BUFLEN
#define	BUFLEN		(LINEBUFLEN + LINEBUFLEN)
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sfshrink(cchar *,int,cchar **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optvalue(cchar *,int) ;
extern int	optbool(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;

extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	printhelp(void *,cchar *,cchar *,cchar *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		headers:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PROGINFO	*pip ;
	cchar		*homedname ;
	cchar		*fontname ;
	cchar		*headerstring ;
	cchar		*pointstring ;
	int		nfiles ;
	int		npages ;
	int		maxlines ;
	int		blanklines ;
	int		pointlen ;
	int		ps ;
	int		vs ;
	int		coffset ;
	int		xoffset ;
	int		yoffset ;
	char		blanks[LINEBUFLEN + 1] ;
	char		blankstring[MAXBLANKLINES + 1] ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,SHIO *,cchar *) ;
static int	procfile(PROGINFO *,SHIO *,cchar *,int,int) ;

static int	procout_begin(PROGINFO *,SHIO *) ;
static int	procout_end(PROGINFO *,SHIO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_defs(LOCINFO *) ;
static int	locinfo_blanks(LOCINFO *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif /* CF_LOCSETENT */


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"DEBUG",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"co",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_debug,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_co,
	argopt_overlast
} ;

static const PIVARS	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;

static const MAPEX	mapexs[] = {
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ 0, 0 }
} ;


/* exported subroutines */


int b_cookieset(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    cchar	**envv = (cchar **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_cookieset) */


int p_cookieset(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_cookieset) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar **argv,cchar **envv,void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	SHIO		efile ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
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

	pip->verboselevel = 1 ;

	pip->lip = &li ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,0) ;
	if (rs < 0) goto badpargs ;

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
	        const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp + 1) ;

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

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program root */
	                case argopt_root:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pr = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }

	                    break ;

/* debug level */
	                case argopt_debug:
	                    pip->debuglevel = 1 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
	                            pip->debuglevel = rs ;
	                        }
	                    }
	                    break ;

	                case argopt_version:
	                    f_version = TRUE ;
	                    break ;

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

/* help file */
	                case argopt_help:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->hfname = avp ;
	                    }
	                    f_help  = TRUE ;
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sn = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* argument file */
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

/* output file-name */
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

	                case argopt_co:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = optvalue(avp,avl) ;
	                            lip->coffset = rs ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

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

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* program-root */
	                    case 'R':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* blank lines at the top of a page */
	                    case 'b':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(avp,avl) ;
	                                lip->blanklines = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->fontname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* output page headers */
	                    case 'h':
	                        lip->f.headers = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                lip->headerstring = avp ;
	                        }
	                        break ;

	                    case 'l':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(avp,avl) ;
	                                lip->maxlines = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'p':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->pointstring = argp ;
	                                lip->pointlen = argl ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

	                    case 'v':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(avp,avl) ;
	                                lip->vs = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'x':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(avp,avl) ;
	                                lip->xoffset = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'y':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(avp,avl) ;
	                                lip->yoffset = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* print a brief usage summary (and then exit!) */
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

	        } /* end if (digits as argument or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&efile,efname,"wca",0666)) >= 0) {
	    pip->efp = &efile ;
	    pip->open.errfile = TRUE ;
	    shio_control(pip->efp,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0) goto badarg ;

#if	CF_DEBUGS
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp,"%s: debuglevel=%u\n",pn,pip->debuglevel) ;
	}

/* continue w/ the trivia argument processing stuff */

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

/* get the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	    shio_printf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help) {
#if	CF_SFIO
	    printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* continue */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (rs >= 0) {
	    if ((rs = locinfo_defs(lip)) >= 0) {
	        rs = locinfo_blanks(lip) ;
	    }
	}

/* processing the input file arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checking for positional arguments\n") ;
#endif

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    ARGINFO	*aip = &ainfo ;
	    BITS	*bop = &pargs ;
	    cchar	*afn = afname ;
	    cchar	*ofn = ofname ;
	    rs = process(pip,aip,bop,ofn,afn) ;
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

/* finish up */

	if ((rs < 0) && (! pip->f.quiet)) {
	    cchar	*fmt ;
	    if (cp != NULL) {
	        if (*cp == '-') cp = "*STDIN*" ;
		fmt = "%s: error on file=%s (%d)\n" ;
	        shio_printf(pip->efp,fmt,pip->progname,cp,rs) ;
	    } else {
		fmt = "%s: error on afile=%s (%d)\n" ;
	        shio_printf(pip->efp,fmt,pip->progname,afname,rs) ;
	    }
	} /* end if (error report) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
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

/* let's get out of here! */

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp,"%s: files processed - %d\n",pn,lip->nfiles) ;
	    shio_printf(pip->efp,"%s: pages processed - %d\n",pn,lip->npages) ;
	}

/* close off and get out! */
retearly:
	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

	bits_finish(&pargs) ;

badpargs:
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad arguments */
badarg:
	{
	    ex = EX_USAGE ;
	    cchar	*fmt = "%s: invalid argument specified (%d)\n" ;
	    shio_printf(pip->efp,fmt,pip->progname,rs) ;
	    usage(pip) ;
	}
	goto retearly ;

}
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<file(s)> ...]\n" ;
	rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-l <lines>] [-<lines>] [-w <outfile>]\n" ;
	rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    if ((rs = procout_begin(pip,ofp)) >= 0) {
	        wlen += rs ;
	        if ((rs = procargs(pip,aip,bop,ofp,afn)) >= 0) {
	            wlen += rs ;
	            rs = procout_end(pip,ofp) ;
	            wlen += rs ;
	        }
	    }
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (shio) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,SHIO *ofp,cchar *afn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;
	int		cl ;
	int		wlen = 0 ;
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
	                const int	pspec = (lip->npages & 1) ;
	                pan += 1 ;
	                rs = procfile(pip,ofp,cp,(pan + 1),pspec) ;
	                wlen += rs ;
	            }
	        }

	        if (rs < 0) break ;
	    } /* end for */
	} /* end for (loading positional arguments) */

/* process any files in the argument filename list file */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    SHIO	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	    if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	                    const int	pspec = (lip->npages & 1) ;
	                    lbuf[(cp+cl)-lbuf] = '\0' ;
	                    pan += 1 ;
	                    rs = procfile(pip,ofp,cp,(pan + 1),pspec) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = shio_close(afp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        if (! pip->f.quiet) {
	            fmt = "%s: inaccessigle argument-list (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	            fmt = "%s: afile=%s\n" ;
	            shio_printf(pip->efp,fmt,pn,afn) ;
	        }
	    }

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pan == 0)) {
	    const int	pspec = (lip->npages & 1) ;

	    cp = "-" ;
	    pan += 1 ;
	    rs = procfile(pip,ofp,cp,(pan + 1),pspec) ;
	    wlen += rs ;

	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procfile(PROGINFO *pip,SHIO *ofp,cchar *fname,int fn,int f_eject)
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ifile, *ifp = &ifile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procfile: ent fname=%s\n",fname) ;
#endif

	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = shio_open(ifp,fname,"r",0666)) >= 0) {
	    USTAT	sb ;
	    if ((rs = shio_stat(ifp,&sb)) >= 0) {
	        if (! S_ISDIR(sb.st_mode)) {
	            const int	llen = LINEBUFLEN ;
	            int		c = 0 ;
	            int		len ;
	            int		sl, cl ;
	            int		f_exit = FALSE ;
	            cchar	*sp, *cp, *tp ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            rs = shio_printf(ofp,".\\\"_ fn=%s\n",fname) ;
	            wlen += rs ;

/* output the pages */

	            while (! f_exit) {

/* skip leading blank lines */

	                while ((rs = shio_readline(ifp,lbuf,llen)) > 0) {
	                    len = rs ;

	                    sl = sfshrink(lbuf,len,&sp) ;

	                    if ((sl > 0) && (sp[0] != '%')) break ;
	                } /* end while (skipping leading blank lines) */

	                if (rs <= 0) break ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("procfile: line=>%t<\n",sp,sl) ;
#endif

/* put out the stuff for this next cookie */

	                c += 1 ;
	                shio_printf(ofp,".SP\n") ;

#if	F_DSBLOCK
	                shio_printf(ofp,".DS CB F \\\\n%caI\n",CH_LPAREN) ;
#else
	                shio_printf(ofp,".QS\n") ;
#endif

/* copy over the first line */

	                shio_printf(ofp,"%t\n",sp,sl) ;

/* copy over the lines until we reach the author line (if there is one) */

	                sl = 0 ;
	                while ((rs = shio_readline(ifp,lbuf,llen)) > 0) {
	                    len = rs ;

	                    if (lbuf[len - 1] == '\n') len -= 1 ;
	                    lbuf[len] = '\0' ;

	                    sp = lbuf ;
	                    sl = len ;
	                    while ((sl > 0) && CHAR_ISWHITE(sp[sl - 1])) {
	                        sl -= 1 ;
			    }

#if	CF_DEBUG
	                    if (DEBUGLEVEL(2))
	                        debugprintf("procfile: copy line=>%t<\n",
				sp,sl) ;
#endif

	                    if ((sl > 0) &&
	                        ((sp[0] == '%') || 
				(strncmp(sp,"\t\t--",4) == 0))) break ;

	                    rs = shio_printf(ofp,"%t\n",sp,sl) ;

	                    if (rs < 0) break ;
	                } /* end while (skipping blank lines) */

	                if (rs <= 0)
	                    f_exit = TRUE ;

/* do we have an "author" line? */

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("procfile: AC line=>%t<\n",sp,sl) ;
#endif

	                if ((sl > 0) && (strncmp(sp,"\t\t--",4) == 0)) {

	                    shio_printf(ofp,".br\n") ;

	                    shio_printf(ofp,".in +5\n") ;

	                    sp += 4 ;
	                    sl -= 4 ;
	                    while ((sl > 0) && CHAR_ISWHITE(*sp)) {
	                        sp += 1 ;
	                        sl -= 1 ;
	                    }

	                    if ((tp = strnchr(sp,sl,CH_LPAREN)) != NULL) {

	                        shio_printf(ofp,"-- %t\n",sp,(tp - sp)) ;

	                        shio_printf(ofp,".br\n") ;

	                        cp = (tp + 1) ;
	                        cl = ((sp + sl) - (tp + 1)) ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(2))
	                            debugprintf("procfile: rest A line=>%t<\n",
					cp, cl) ;
#endif

	                        if ((tp = strnchr(cp,cl,CH_RPAREN)) != NULL) {
	                            cl = tp - cp ;
	                        }

	                        shio_printf(ofp,"%t\n",cp,cl) ;

/* read lines until something makes us break out */

	                        while (rs >= 0) {
				    const int	rch = CH_RPAREN ;
				    rs = shio_readline(ifp,lbuf,llen) ;
				    if (rs <= 0) break ;
	                            len = rs ;

	                            sl = sfshrink(lbuf,len,&sp) ;

	                            if ((sl == 0) || (sp[0] == '%'))
	                                break ;

	                            if ((tp = strnchr(sp,sl,rch)) != NULL) {
	                                sl = tp - sp ;
	                            }

	                            rs = shio_printf(ofp,"%t\n",sp,sl) ;

	                            if (rs < 0) break ;
	                        } /* end while */

	                    } else {
	                        shio_printf(ofp,"-- %t\n",sp,sl) ;
	                    }

	                    shio_printf(ofp,".in -5\n") ;

	                } /* end if (had an author) */

/* finish the last c */

#if	F_DSBLOCK
	                shio_printf(ofp,".DE\n") ;
#else
	                shio_printf(ofp,".QE\n") ;
#endif

	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            lip->nfiles += 1 ;
	        } /* end if (normal file) */
	    } /* end if (shio_stat) */
	    rs1 = shio_close(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (shio) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */


static int procout_begin(PROGINFO *pip,SHIO *ofp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (lip == NULL) return SR_FAULT ;

#if	CF_LEADER

/* output the header stuff */

#if	CF_NOFILL
	if (rs >= 0) rs = shio_printf(ofp,".nf\n") ;
	wlen += rs ;
#endif

	if (rs >= 0) rs = shio_printf(ofp,".fp 5 %s\n",lip->fontname) ;
	wlen += rs ;

	if (rs >= 0) rs = shio_printf(ofp,".ft %s\n",lip->fontname) ;
	wlen += rs ;

/* specify "no header" */

	if (rs >= 0) rs = shio_printf(ofp,".nr N 4\n") ;
	wlen += rs ;

/* change to running point size */

	if (rs >= 0) rs = shio_printf(ofp,".S %d\n",ps) ;
	wlen += rs ;

#ifdef	COMMENT
	if (rs >= 0) rs = shio_printf(ofp,".ps %d\n",ps) ;
	wlen += rs ;

	if (rs >= 0) rs = shio_printf(ofp,".vs %d\n",vs) ;
	wlen += rs ;
#endif /* COMMENT */

#endif /* CF_LEADER */

	if (rs >= 0) rs = shio_printf(ofp,".nr iJ 5\n") ;
	wlen += rs ;

	if (rs >= 0) {
	    cchar	*fmt = ".if \\n%ciI>0 .nr iJ \\n%ciI\n" ;
	    rs = shio_printf(ofp,fmt,CH_LPAREN,CH_LPAREN) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout_begin) */


static int procout_end(PROGINFO *pip,SHIO *ofp)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	if (pip == NULL) return SR_FAULT ;
	if (ofp == NULL) return SR_FAULT ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout_end) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->maxlines = DEFMAXLINES ;
	lip->blanklines = DEFBLANKLINES ;
	lip->fontname = "CW" ;
	lip->ps = DEFPOINT ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


#if	CF_LOCSETENT
int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
{
	VECSTR		*slp ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	slp = &lip->stores ;
	if (! lip->open.stores) {
	    rs = vecstr_start(slp,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
	        oi = vecstr_findaddr(slp,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(slp,vp,len,epp) ;
	    } else {
	        *epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(slp,oi) ;
	    }
	} /* end if (ok) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCSETENT */


static int locinfo_defs(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip->maxlines < 1) lip->maxlines = DEFMAXLINES ;
	if (lip->maxlines > MAXLINES) lip->maxlines = MAXLINES ;

/* establish an offset if any */

	if (lip->coffset < 0) {
	    lip->coffset = 0 ;
	} else if (lip->coffset > LINEBUFLEN) {
	    lip->coffset = LINEBUFLEN ;
	}

/* establish working point size and vertical spacing */

	if (lip->pointstring != NULL) {
	    int	i ;

	    i = substring(lip->pointstring,lip->pointlen,".") ;

	    if (i < 0) {
	        i = substring(lip->pointstring,lip->pointlen,"/") ;
	    }

	    if (i >= 0) {

	        if (i > 0) {
	            rs = optvalue(lip->pointstring,i) ;
	            lip->ps = rs ;
	        }

	        if ((lip->pointlen - i) > 1) {
	            rs = optvalue(lip->pointstring+i+1,lip->pointlen-i-1) ;
	            lip->vs = rs ;
	        }

	    } else {

	        rs = optvalue(lip->pointstring,lip->pointlen) ;
	        lip->ps = rs ;

	    }

	} /* end if (handling the 'pointstring') */

	if (lip->ps < 2) lip->ps = 6 ;

	if (lip->vs == 0) {
	    lip->vs = (lip->ps + 2) ;
	} else if (lip->vs < lip->ps) {
	    lip->vs = (lip->ps + 1) ;
	}

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp, "%s: ps %d - vs %d\n",
	        pip->progname,lip->ps,lip->vs) ;
	}

	return rs ;
}
/* end subroutine (locinfo_defs) */


static int locinfo_blanks(LOCINFO *lip)
{
	const int	max = MAXBLANKLINES ;
	int		rs = SR_OK ;
	int		i ;
	char		*bp = lip->blankstring ;

	for (i = 0 ; i < LINEBUFLEN ; i += 1) {
	    lip->blanks[i] = ' ' ;
	}

	for (i = 0 ; (i < lip->blanklines) && (i < max) ; i += 1) {
	    *bp++ = '\n' ;
	}
	*bp = '\0' ;

	return rs ;
}
/* end subroutine (locinfo_blanks) */


