/* main (filefilter) */

/* generic front-end */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.

	= 2006-04-05, David A­D­ Morano
	Enhanced to add dev-inode uniqueness checking.

*/

/* Copyright © 1998,2006 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ filefilter [<names(s)> ...] [-af <argfile>] [-u]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<bfile.h>
#include	<hdbstr.h>
#include	<hdb.h>
#include	<vecpstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 3),2048)
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	strwcmp(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUG || CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */

struct fileuniq {
	dev_t		dev ;
	ino64_t		ino ;
	uint		st_mode ;
	int		f_valid:1 ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *,PARAMOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *,cchar *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *) ;
static int	procin(PROGINFO *,bfile *,const char *) ;
static int	procfile(PROGINFO *,bfile *,const char *) ;
static int	procsufs(PROGINFO *,PARAMOPT *,const char *) ;
static int	procnoprog(PROGINFO *,struct fileuniq *,cchar *) ;

static int	procsuf_begin(PROGINFO *,PARAMOPT *,cchar *,cchar *) ;
static int	procsuf_end(PROGINFO *) ;
static int	procuniq_begin(PROGINFO *) ;
static int	procuniq_end(PROGINFO *) ;

static int	loadsuf(PROGINFO *,PARAMOPT *,cchar *,cchar *,int) ;

static int	cmpuniq(struct fileuniq *,struct fileuniq *,int) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"pm",
	"sn",
	"af",
	"if",
	"of",
	"ef",
	"sr",
	"sa",
	"option",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_af,
	argopt_if,
	argopt_of,
	argopt_ef,
	argopt_sr,
	argopt_sa,
	argopt_option,
	argopt_overlast
} ;

static const char	*progmodes[] = {
	"filefilter",
	"fileuniq",
	"filenoprog",
	"filtername",
	NULL
} ;

enum progmodes {
	progmode_filefilter,
	progmode_fileuniq,
	progmode_filenoprog,
	progmode_filtername,
	progmode_overlast
} ;

static const PIVARS	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;

static const MAPEX	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const char	*progopts[] = {
	"uniq",
	"name",
	"noprog",
	"noelf",
	"prog",
	"elf",
	"nosock",
	"sock",
	"nopipe",
	"pipe",
	"nodev",
	"dev",
	"nosuf",
	"noextra",
	"sr",
	"sa",
	NULL
} ;

enum progopts {
	progopt_uniq,
	progopt_name,
	progopt_noprog,
	progopt_noelf,
	progopt_prog,
	progopt_elf,
	progopt_nosock,
	progopt_sock,
	progopt_nopipe,
	progopt_pipe,
	progopt_nodev,
	progopt_dev,
	progopt_nosuf,
	progopt_noextra,
	progopt_sr,
	progopt_sa,
	progopt_overlast
} ;

static const char	*whiches[] = {
	"elf",
	"uniq",
	"name",
	"prog",
	"sock",
	"pipe",
	"dev",
	"nosock",
	"nopipe",
	"nodev",
	NULL
} ;

enum whiches {
	which_elf,
	which_uniq,
	which_name,
	which_prog,
	which_sock,
	which_pipe,
	which_dev,
	which_nosock,
	which_nopipe,
	which_nodev,
	which_overlast
} ;

static const char	*sufs[] = {
	PO_SUFACC,
	PO_SUFREJ,
	NULL
} ;

enum sufs {
	suf_acc,
	suf_rej,
	suf_overlast
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	const char	*po_sufacc = PO_SUFACC ;
	const char	*po_sufrej = PO_SUFREJ ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pmspec = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*efname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	if (rs >= 0) {
	    rs = keyopt_start(&akopts) ;
	    pip->open.akopts = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = paramopt_start(&aparams) ;
	    pip->open.aparams = (rs >= 0) ;
	}

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

/* program-root */
	                case argopt_root:
	                    if (argr >= 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose mode */
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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program mode */
	                case argopt_pm:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pmspec = avp ;
	                    } else {
	                        if (argr >= 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pmspec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* program search mode */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr >= 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sn = argp ;
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
	                        if (argr >= 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                efname = argp ;
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
	                        if (argr >= 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                afname = argp ;
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
	                        if (argr >= 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ofname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* input file */
	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ifname = avp ;
	                    } else {
	                        if (argr >= 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ifname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* suffix-accept */
	                case argopt_sa:
	                    if (argr >= 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            cchar	*po = po_sufacc ;
	                            rs = loadsuf(pip,&aparams,po,argp,argl) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* suffix-reject */
	                case argopt_sr:
	                    if (argr >= 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            cchar	*po = po_sufrej ;
	                            rs = loadsuf(pip,&aparams,po,argp,argl) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* the user specified some progopts */
	                case argopt_option:
	                    if (argr >= 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = keyopt_loads(&akopts,argp,argl) ;
				}
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

/* debug */
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'o':
	                        if (argr >= 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                rs = keyopt_loads(&akopts,argp,argl) ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* unique check mode */
	                    case 'u':
	                        pip->f.f_uniq = TRUE ;
	                        break ;

/* verbose mode */
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

	        } /* end if (digits as argument or not) */

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
	}

	if (rs < 0) {
	    ex = EX_USAGE ;
	    bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	        pip->progname,rs) ;
	    usage(pip) ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	}

/* get our program mode */

	if (pmspec == NULL)
	    pmspec = pip->progname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;

	if (pip->progmode < 0)
	    pip->progmode = progmode_filefilter ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0) {
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    } else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif /* CF_DEBUG */

	cp = NULL ;
	switch (pip->progmode) {
	case progmode_filefilter:
	    cp = BANNER_FILEFILTER ;
	    break ;
	case progmode_fileuniq:
	    cp = BANNER_FILEUNIQ ;
	    break ;
	case progmode_filtername:
	    cp = BANNER_FILTERNAME ;
	    break ;
	case progmode_filenoprog:
	    cp = BANNER_FILEPROG ;
	    break ;
	} /* end switch */

	if ((rs >= 0) && (cp != NULL)) {
	    proginfo_setbanner(pip,cp) ;
	}

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* progopts */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (rs >= 0) {
	    rs = procopts(pip,&akopts,&aparams) ;
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->f.f_noextra) {
	    pip->f.f_nodev = TRUE ;
	    pip->f.f_nopipe = TRUE ;
	    pip->f.f_nosock = TRUE ;
	}

/* if we don't have a request for something yet, use our progmode */

	switch (pip->progmode) {
	case progmode_filenoprog:
	    pip->f.f_noprog = TRUE ;
	    break ;
	case progmode_fileuniq:
	    pip->f.f_uniq = TRUE ;
	    break ;
	case progmode_filtername:
	default:
	    pip->f.f_name = TRUE ;
	    break ;
	} /* end switch */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: requests prog=%u uniq=%u name=%u\n",
	        pip->f.f_noprog,pip->f.f_uniq,pip->f.f_name) ;
#endif

	if (pip->debuglevel > 0) {
	    if (pip->f.f_noprog) {
	        bprintf(pip->efp,"%s: request> %s\n",
	            pip->progname,whiches[which_elf]) ;
	    }
	    if (pip->f.f_uniq) {
	        bprintf(pip->efp,"%s: request> %s\n",
	            pip->progname,whiches[which_uniq]) ;
	    }
	    if (pip->f.f_name) {
	        bprintf(pip->efp,"%s: request> %s\n",
	            pip->progname,whiches[which_name]) ;
	    }
	} /* end if */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

/* initialize what we need */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = procuniq_begin(pip)) >= 0) {
	        PARAMOPT	*pop = &aparams ;
	        if ((rs = procsuf_begin(pip,pop,po_sufacc,po_sufrej)) >= 0) {
	            ARGINFO	*aip = &ainfo ;
	            BITS	*bop = &pargs ;
	            cchar	*ofn = ofname ;
	            cchar	*ifn = ifname ;
	            cchar	*afn = afname ;

	            rs = process(pip,aip,bop,ofn,ifn,afn) ;

	            if ((rs >= 0) && (pip->debuglevel > 0)) {
	                bprintf(pip->efp,"%s: processed files=%d\n",
	                    pip->progname,rs) ;
	            }

	            rs1 = procsuf_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procsuf) */
	        rs1 = procuniq_end(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (procuniq) */
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,"%s: invalid usage (%d)\n",
	                pip->progname,rs) ;
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

/* early return thing */
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

	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&aparams) ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_shcat: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<file(s> ...] [-af <afile>] [-o <opt(s)>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s: [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(PROGINFO *pip,KEYOPT *kop,PARAMOPT *pop)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*cp ;

	if ((cp = getenv(VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(progopts,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                c += 1 ;
	                switch (oi) {
	                case progopt_uniq:
	                    pip->f.f_uniq = TRUE ;
	                    break ;
	                case progopt_name:
	                    pip->f.f_name = TRUE ;
	                    break ;
	                case progopt_noprog:
	                case progopt_noelf:
	                case progopt_prog:
	                case progopt_elf:
	                    pip->f.f_noprog = TRUE ;
	                    break ;
	                case progopt_nosock:
	                case progopt_sock:
	                    pip->f.f_nosock = TRUE ;
	                    break ;
	                case progopt_nopipe:
	                case progopt_pipe:
	                    pip->f.f_nopipe = TRUE ;
	                    break ;
	                case progopt_nodev:
	                case progopt_dev:
	                    pip->f.f_nodev = TRUE ;
	                    break ;
	                case progopt_noextra:
	                    if (! pip->final.f_noextra) {
	                        pip->final.f_noextra = TRUE ;
	                        pip->f.f_noextra = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.f_noextra = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_sa:
	                    if (vl > 0) {
	                        rs = loadsuf(pip,pop,PO_SUFACC,vp,vl) ;
	                    }
	                    break ;
	                case progopt_nosuf:
	                case progopt_sr:
	                    if (vl > 0) {
	                        rs = loadsuf(pip,pop,PO_SUFREJ,vp,vl) ;
	                    }
	                    break ;
	                } /* end switch */

	            } /* end if */

	            if (rs < 0) break ;
	        } /* end while */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procsuf_begin(PROGINFO *pip,PARAMOPT *app,cchar *poa,cchar *por)
{
	int		rs ;
	int		rs1 ;
	if ((rs = procsufs(pip,app,poa)) >= 0) {
	    rs = procsufs(pip,app,por) ;
	    if (rs < 0) {
	        if (pip->f.sufacc) {
	            pip->f.sufacc = FALSE ;
	            rs1 = vecpstr_finish(&pip->sufacc) ;
	            if (rs >= 0) rs = rs1 ;
	        }
	    }
	}
	return rs ;
}
/* end subroutine (procsuf) */


static int procsuf_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->f.sufrej) {
	    pip->f.sufrej = FALSE ;
	    rs1 = vecpstr_finish(&pip->sufrej) ;
	    if (rs >= 0) rs = rs1 ;
	}
	if (pip->f.sufacc) {
	    pip->f.sufacc = FALSE ;
	    rs1 = vecpstr_finish(&pip->sufacc) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (procsuf_end) */


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,
		cchar *ifn,cchar *afn)
{
	bfile		ofile, *ofp = &ofile ;
	const mode_t	om = 0664 ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",om)) >= 0) {
	    if ((rs = hdbstr_start(&pip->ndb,DEFNAMES)) >= 0) {
	        if ((rs = procargs(pip,aip,bop,afn)) >= 0) {
	            rs = procin(pip,ofp,ifn) ;
	            c += rs ;
	        }
	        rs1 = hdbstr_finish(&pip->ndb) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (hdbstr) */
	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: inaccessible output (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    bprintf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (process) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	cchar		*cp ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

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
	                rs = hdbstr_add(&pip->ndb,cp,-1,NULL,0) ;
	            }
	        }
	        if (rs < 0) {
		    fmt = "%s: processing error (%d) in file=%s\n" ;
	            bprintf(pip->efp,fmt,pn,rs,cp) ;
	            break ;
	        }
	    } /* end for */
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

	            if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	                    pan += 1 ;
	                    rs = hdbstr_add(&pip->ndb,cp,cl,NULL,0) ;
	                }
	            }

	            if (rs < 0) {
			fmt = "%s: processing error (%d) in file=%t\n" ;
	                bprintf(pip->efp,fmt,pn,rs,cp,cl) ;
	                break ;
	            }
	        } /* end while */

	        rs1 = bclose(afp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        if (! pip->f.quiet) {
	            fmt = "%s: inacessible argument-list (%d)\n" ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        }
	    } /* end if */

	} /* end if (argument file) */

	return rs ;
}
/* end subroutine (procargs) */


static int procin(PROGINFO *pip,bfile *ofp,cchar ifn[])
{
	bfile		ifile, *ifp = &ifile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((ifn == NULL) || (ifn[0] == '\0') || (ifn[0] == '-'))
	    ifn = BFILE_STDIN ;

	if ((rs = bopen(ifp,ifn,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	        len = rs ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;
	        lbuf[len] = '\0' ;

	        if (len > 0) {
	            c += 1 ;
	            rs = procfile(pip,ofp,lbuf) ;
	        }

	        if (rs < 0) break ;
	    } /* end while (reading lines) */

	    rs1 = bclose(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (opened-file) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procin) */


static int procfile(PROGINFO *pip,bfile *ofp,cchar fname[])
{
	struct fileuniq	uf, *ufp ;
	struct ustat	sb ;
	HDB_DATUM	key, val ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		bnl ;
	int		wlen = 0 ;
	int		f_filetype ;
	int		f_suf = TRUE ;
	int		f_isreg = FALSE ;
	int		f_accept = FALSE ;
	int		f_process = TRUE ;
	const char	*bnp ;

	uf.f_valid = FALSE ;

	if (f_suf && (pip->f.sufacc || pip->f.sufrej || pip->f.f_name)) {
	    bnl = sfbasename(fname,-1,&bnp) ;
	    if (bnl <= 0) f_suf = FALSE ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procfile: b=%t sa=%u sr=%u\n",
	        bnp,bnl,pip->f.sufacc,pip->f.sufrej) ;
#endif

/* check against the suffix lists */

	if (f_suf && (pip->f.sufacc || pip->f.sufrej)) {
	    int		sl ;
	    cchar	*tp, *sp ;
	    if ((tp = strnrchr(bnp,bnl,'.')) != NULL) {
	        sp = (tp+1) ;
	        sl = ((bnp+bnl)-(tp+1)) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procfile: suf=%t \n",sp,sl) ;
#endif

/* check against the suffix-acceptance list */

	        if (pip->f.sufacc) {
	            rs1 = vecpstr_findn(&pip->sufacc,sp,sl) ;
	            f_accept = (rs1 >= 0) ;
	        }

/* check against the suffix-rejectance list */

	        if (pip->f.sufrej && (! f_accept)) {
	            rs1 = vecpstr_findn(&pip->sufrej,sp,sl) ;
	            if (rs1 >= 0) f_process = FALSE ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procfile: suf fa=%u f_process=%u\n",
	                f_accept,f_process) ;
#endif

	    }
	} /* end if (suffix lists) */

/* check name against exclusion list */

	if (pip->f.f_name && f_process && (! f_accept)) {

	    if ((rs1 = hdbstr_fetch(&pip->ndb,bnp,bnl,NULL,NULL)) >= 0) {
	        f_process = FALSE ;
	    } else {
	        if (rs1 != SR_NOENT) rs = rs1 ;
	    }

	} /* end if (previously named file) */

/* continue with harder checks */

	if ((rs >= 0) && f_process) {
	    uf.f_valid = TRUE ;
	    if ((rs1 = u_lstat(fname,&sb)) >= 0) {
	        uf.dev = sb.st_dev ;
	        uf.ino = sb.st_ino ;
	        uf.st_mode = sb.st_mode ;
	        if (S_ISREG(uf.st_mode)) f_isreg = TRUE ;
	    } else {
	        f_process = FALSE ;
	    }
	} /* end if */

/* check for one of the restricted file types */

	f_filetype = FALSE ;
	f_filetype = f_filetype || pip->f.f_nosock ;
	f_filetype = f_filetype || pip->f.f_nopipe ;
	f_filetype = f_filetype || pip->f.f_nodev ;

	if ((rs >= 0) && f_filetype && f_process) {

	    if (f_process && pip->f.f_nosock &&
	        S_ISSOCK(uf.st_mode))
	        f_process = FALSE ;

	    if (f_process && pip->f.f_nopipe &&
	        S_ISFIFO(uf.st_mode))
	        f_process = FALSE ;

	    if (f_process && pip->f.f_nodev &&
	        (S_ISCHR(uf.st_mode) || S_ISBLK(uf.st_mode)))
	        f_process = FALSE ;

	} /* end if (special file type) */

/* check for unique file */

	if ((rs >= 0) && pip->f.f_uniq && f_process) {

	    key.buf = &uf ;
	    key.len = sizeof(struct fileuniq) ;

	    if ((rs1 = hdb_fetch(&pip->udb,key,NULL,NULL)) >= 0) {
	        f_process = FALSE ;
	    } else {
	        if (rs1 != SR_NOENT) rs = rs1 ;
	    }

	} /* end if (unique file) */

/* check for program (ELF) file */

	if ((rs >= 0) && pip->f.f_noprog && f_process && (! f_accept)) {
	    if (f_isreg) {
	        if (procnoprog(pip,&uf,fname) != 0) {
	            f_process = FALSE ;
		}
	    }
	} /* end if (program file) */

/* finally, do we have a hit? */

	if ((rs >= 0) && f_process) {

	    if (pip->f.f_uniq) {
	        int	size ;

	        ufp = NULL ;
	        size = sizeof(struct fileuniq) ;
	        rs = uc_malloc(size,&ufp) ;
	        if (rs >= 0) {
	            memcpy(ufp,&uf,size) ;
	            key.buf = ufp ;
	            key.len = sizeof(struct fileuniq) ;
	            memset(&val,0,sizeof(HDB_DATUM)) ;

	            rs = hdb_store(&pip->udb,key,val) ;
	            if ((rs < 0) && (ufp != NULL)) {
	                uc_free(ufp) ;
	                ufp = NULL ;
	            }
	        }

	    } /* end if (unique processing) */

	    if (rs >= 0) {
	        rs = bprintln(ofp,fname,-1) ;
	        wlen += rs ;
	    }

	} /* end if (had the file and printed it out) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */


static int procuniq_begin(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->f.f_uniq) {
	    hdbcmpfunc_t	cf = (hdbcmpfunc_t) cmpuniq ;
	    rs = hdb_start(&pip->udb,DEFLINKS,1,NULL,cf) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: hdb_start() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procuniq_begin) */


static int procuniq_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->f.f_uniq) {
	    HDB_CUR	cur ;
	    HDB_DATUM	key, val ;

	    pip->f.f_uniq = FALSE ;
	    if ((rs = hdb_curbegin(&pip->udb,&cur)) >= 0) {
	        struct fileuniq	*up ;
	        while (hdb_enum(&pip->udb,&cur,&key,&val) >= 0) {
	            up = (struct fileuniq *) key.buf ;
	            if (up != NULL) {
	                uc_free(up) ;
	            }
	        } /* end while */
	        hdb_curend(&pip->udb,&cur) ;
	    } /* end if (unique DB) */

	    rs1 = hdb_finish(&pip->udb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	return rs ;
}
/* end subroutine (procuniq_end) */


static int procsufs(PROGINFO *pip,PARAMOPT *pop,cchar *s)
{
	VECPSTR		*vlp ;
	int		rs = SR_OK ;
	int		si ;
	int		c = 0 ;
	int		f = FALSE ;

	si = matstr(sufs,s,-1) ;
	if (si < 0) {
	    rs = SR_NOANODE ;
	    goto ret0 ;
	}

	c = paramopt_countvals(pop,s) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsufs: s=%s c=%d\n",s,c) ;
#endif

	if (c > 0) {
	    PARAMOPT_CUR	cur ;
	    switch (si) {
	    case suf_acc:
	        f = pip->have.sufacc ;
	        if (f) {
	            pip->f.sufacc = TRUE ;
	            vlp = &pip->sufacc ;
	        }
	        break ;
	    case suf_rej:
	        f = pip->have.sufrej ;
	        if (f) {
	            pip->f.sufrej = TRUE ;
	            vlp = &pip->sufrej ;
	        }
	        break ;
	    } /* end switch */
	    if (f) {
	        if ((rs = vecpstr_start(vlp,5,0,0)) >= 0) {
	            if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {
	                int	vl ;
	                cchar	*vp ;
	                while ((vl = paramopt_fetch(pop,s,&cur,&vp)) >= 0) {
	                    if (vl != 0) {
#if	CF_DEBUG
	                        if (DEBUGLEVEL(3))
	                            debugprintf("main/procsufs: s=%t\n",vp,vl) ;
#endif
	                        rs = vecpstr_add(vlp,vp,vl) ;

			    }
	                    if (rs < 0) break ;
	                } /* end while */
	                paramopt_curend(pop,&cur) ;
	            } /* end if (cursor) */
	        } /* end if (vecstr-started) */
	    } /* end if */
	} /* end if (positive) */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procsufs) */


/* check a file for being a program */
static int procnoprog(PROGINFO *pip,struct fileuniq *ufp,cchar fname[])
{
	int		rs ;
	int		f = FALSE ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = u_open(fname,O_RDONLY,0666)) >= 0) {
	    struct ustat	sb ;
	    int			fd = rs ;

	    if (! ufp->f_valid) {
	        ufp->f_valid = TRUE ;
	        rs = u_fstat(fd,&sb) ;
	        ufp->dev = sb.st_dev ;
	        ufp->ino = sb.st_ino ;
	        ufp->st_mode = sb.st_mode ;
	    } /* end if */

	    if ((rs >= 0) && (! S_ISREG(ufp->st_mode)))
	        rs = SR_NOTSUP ;

	    if (rs >= 0) {
	        char	buf[10] ;
	        if ((rs = u_read(fd,buf,4)) >= 4) {
	            f = (memcmp(buf,"\177ELF",4) == 0) ;
		}
	    } /* end if (opened) */

	    u_close(fd) ;
	} /* end if (open-file) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procnoprog) */


static int loadsuf(PROGINFO *pip,PARAMOPT *pop,cchar *s,cchar *ap,int al)
{
	int		rs = SR_OK ;
	int		si ;
	int		c = 0 ;
	int		f_final = TRUE ;
	cchar		*var ;

	if (ap == NULL) goto ret0 ;

	si = matstr(sufs,s,-1) ;
	if (si < 0) {
	    rs = SR_NOANODE ; /* bug-check */
	    goto ret0 ;
	}

	switch (si) {
	case suf_acc:
	    f_final = pip->final.sufacc ;
	    pip->final.sufacc = TRUE ;
	    pip->have.sufacc = TRUE ;
	    var = VARSA ;
	    break ;
	case suf_rej:
	    f_final = pip->final.sufrej ;
	    pip->final.sufrej = TRUE ;
	    pip->have.sufrej = TRUE ;
	    var = VARSR ;
	    break ;
	} /* end switch */

	if ((! f_final) && (strwcmp("-",ap,al) != 0)) {
	    if (strwcmp("+",ap,al) == 0) {
	        ap = getenv(var) ;
	        al = -1 ;
	    }
	    if (ap != NULL) {
	        rs = paramopt_loads(pop,s,ap,al) ;
	        c = rs ;
	        if ((rs >= 0) && (c > 0)) {
	            switch (si) {
	            case suf_acc:
	                pip->have.sufacc = TRUE ;
	                break ;
	            case suf_rej:
	                pip->have.sufrej = TRUE ;
	                break ;
	            } /* end switch */
	        }
	    }
	} /* end if */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadsuf) */


/* ARGSUSED */
static int cmpuniq(u1p,u2p,len)
struct fileuniq	*u1p ;
struct fileuniq	*u2p ;
int		len ;
{
	int		f ;
	f = ((u1p->dev == u2p->dev) && (u1p->ino == u2p->ino)) ? 0 : 1 ;
	return f ;
}
/* end subroutine (cmpuniq) */


