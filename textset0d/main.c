/* main (textset) */

/* format a TEXT file into TROFF source input language */


#define	CF_DEBUGS	0		/* compiltime-time debug print-outs */
#define	CF_DEBUG	1		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 1997-09-10, David A­D­ Morano
        This subroutine was originally written but it was probably started from
        any one of the numerous subroutine which perform a similar
        "file-processing" fron end.

	= 2018-02-01, David A­D­ Morano
	Some refactoring.

*/

/* Copyright © 1997 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program will read the input file and format it into TROFF constant
        width font style source input language.

	Synopsis:

	$ textset [<ifile>] [-DV] [-o <offset>] [-l <lines>] 
		[-f <font>] [-p <point_size>] [-v <vertical_space>]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bits.h>
#include	<bfile.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		(MAXPATHLEN + (2 * LINEBUFLEN))
#endif


/* external subroutines */

extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	progfile(PROGINFO *,cchar *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;

static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procleader(PROGINFO *,bfile *) ;
static int	proctrailer(PROGINFO *,bfile *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,bfile *,cchar *) ;

static int	procdata_begin(PROGINFO *pip,int blines) ;
static int	procdata_end(PROGINFO *pip) ;


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
	"offset",
	"fo",
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
	argopt_offset,
	argopt_fo,
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

static const char	*stdfonts[] = {
	"R",
	"I",
	"B",
	"BI",
	"CW",
	NULL
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		pagelines = 0 ;
	int		blanklines = DEFBLANKLINES ;
	int		coffset = 0 ;		/* column offset */
	int		pointlen = 0 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*pointstring  = NULL ;
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
	rs = proginfo_setbanner(pip,cp) ;

/* initialization */

	pip->verboselevel = 1 ;
	pip->fontname = "CW" ;
	pip->ps = DEFPOINT ;

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
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
	                            pip->pr = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->pr = argp ;
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
	                    f_help  = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->helpfname = avp ;
	                    }
	                    break ;

/* offset */
	                case argopt_offset:
	                case argopt_fo:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = optvalue(argp,argl) ;
	                            coffset = rs ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* progream search-name */
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
	                                rs = optvalue(argp,argl) ;
	                                blanklines = rs ;
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
	                                pip->fontname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* output page headers */
	                    case 'h':
	                        pip->f.headers = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->headerstring = avp ;
	                        }
	                        break ;

	                    case 'l':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                pagelines = rs ;
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
	                                pointstring = argp ;
	                                pointlen = argl ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'v':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                pip->vs = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'w':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ofname = argp ;
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

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

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
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* how many lines per page */

	if ((pagelines == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pagelines = rs ;
	}

	if (pagelines < 1) pagelines = DEFLINES ;
	if (pagelines > MAXLINES) pagelines = MAXLINES ;

	pip->pagelines = pagelines ;

/* establish an offset if any */

	if (coffset < 0) {
	    coffset = 0 ;
	} else if (coffset > LINEBUFLEN) {
	    coffset = LINEBUFLEN ;
	}

	pip->coffset = coffset ;

/* establish working point size and vertical spacing */

	if ((rs >= 0) && (pointstring != NULL)) {
	    int		i ;

	    i = substring(pointstring,pointlen,".") ;

	    if (i < 0) {
	        i = substring(pointstring,pointlen,"/") ;
	    }

	    if (i >= 0) {
	        if (i > 0) {
	            rs = optvalue(pointstring,i) ;
	            pip->ps = rs ;
	        }
	        if ((rs >= 0) && ((pointlen - i) > 1)) {
	            rs = optvalue((pointstring + i + 1),(pointlen - i - 1)) ;
	            pip->vs = rs ;
	        }
	    } else {
	        rs = optvalue(pointstring,pointlen) ;
	        pip->ps = rs ;
	    }

	} /* end if (handling the 'pointstring') */

	if (pip->ps < 2) pip->ps = 6 ;

	if (pip->vs == 0) {
	    pip->vs = (pip->ps + 2) ;
	} else if (pip->vs < pip->ps) {
	    pip->vs = (pip->ps + 1) ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp, "%s: ps=%u vs=%u\n",
	        pip->progname,pip->ps,pip->vs) ;
	}

/* continue */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    pip->fncol = FNCOL ;
	    if ((rs = procdata_begin(pip,blanklines)) >= 0) {
		{
	            ARGINFO	*aip = &ainfo ;
	            BITS	*bop = &pargs ;
	            rs = process(pip,aip,bop,ofname,afname) ;
		}
		rs1 = procdata_end(pip) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (procdata) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: pages processed - %d\n" ;
	    bprintf(pip->efp,fmt,pn,pip->pages) ;
	}

/* done */
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

/* let's get out of here! */
retearly:
	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",pn,ex,rs) ;
	}

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

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

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-l <lines>] [-<lines>] [<infile> [...]]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-f <font>] [-o <offset>] [-b <toplines>] [-DV]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-p <point_size>] [-v <vertical_space>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-of <outfile>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s: where\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s: -l lines    number of lines per page\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s: -lines      number of lines per page\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s: infile      input file\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s: -f font     TROFF type font specification\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s: -o offset   left margin shift offset\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s: -of outfile output file\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s: -D          debugging flag\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s: -V          program version\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	bfile		ofile, *ofp = &ofile ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0666)) >= 0) {
	    pip->ofp = ofp ;

	    if ((rs = procleader(pip,ofp)) >= 0) {
	        if ((rs = procargs(pip,aip,bop,ofp,afn)) >= 0) {
	            rs = proctrailer(pip,ofp) ;
	        }
	    }

	    pip->ofp = NULL ;
	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: inaccessible output (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    bprintf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procleader(PROGINFO *pip,bfile *ofp)
{
	int		rs = SR_OK ;

	bprintf(ofp,".\\\"_ textset start-of-job\n") ;

	bprintf(ofp,".nf\n") ;

	if (matstr(stdfonts,pip->fontname,-1) < 0) {
	    bprintf(ofp,".fp 9 %s\n",pip->fontname) ;
	}

	bprintf(ofp,".ft %s\n",pip->fontname) ;

/* change to running point size */

	bprintf(ofp,".ps %d\n",pip->ps) ;

	bprintf(ofp,".vs %d\n",pip->vs) ;

	return rs ;
}
/* end subroutine (procleader) */


static int proctrailer(PROGINFO *pip,bfile *ofp)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	rs = bprintf(ofp,".\\\"_ textset end-of-job\n") ;
	wlen += rs ;

	return rs ;
}
/* end subroutine (proctrailer) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,bfile *ofp,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	int		wlen = 0 ;
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
	                wlen += rs ;
	            }
	        }
	        if (rs < 0) break ;
	    } /* end for (got a positional argument) */
	} /* end if (ok) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if ((afn == NULL) || (afn[0] == '-')) afn = BFILE_STDIN ;

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
	                    lbuf[(cp+cl)-lbuf] = '\0' ;
	                    rs = progfile(pip,cp) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(&afile) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        if (! pip->f.quiet) {
	            cchar	*pn = pip->progname ;
	            cchar	*fmt = "%s: inaccessible argument-list (%d)\n" ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pan == 0)) {

	    cp = "-" ;
	    pan += 1 ;
	    rs = progfile(pip,cp) ;
	    wlen += rs ;

	} /* end if (standard input) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procdata_begin(PROGINFO *pip,int blines)
{
	int		rs ;
	char		*bp ;
	if (blines > MAXBLANKLINES) blines = MAXBLANKLINES ;
	if ((rs = uc_malloc((blines+1),&bp)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		i ;
	    char	*lp ;
	    pip->blanklines = bp ;
	    for (i = 0 ; i < blines ; i += 1) {
	        *bp++ = '\n' ;
	    }
	    *bp = '\0' ;
	    if ((rs = uc_malloc((llen+1),&lp)) >= 0) {
	        pip->blanks = lp ;
	        for (i = 0 ; i < llen ; i += 1) {
	            lp[i] = ' ' ;
	        }
		lp[i] = '\0' ;
	    } /* end if (m-a) */
	    if (rs < 0) {
		uc_free(bp) ;
		pip->blanklines = NULL ;
	    }
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (procdata_begin) */


static int procdata_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->blanklines != NULL) {
	    rs1 = uc_free(pip->blanklines) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->blanklines = NULL ;
	}

	if (pip->blanks != NULL) {
	    rs1 = uc_free(pip->blanks) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->blanks = NULL ;
	}

	return rs ;
}
/* end subroutine (procdata_end) */


