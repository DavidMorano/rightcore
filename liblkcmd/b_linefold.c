/* b_linefold */

/* this is a SHELL built-in version of 'cat(1)' */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_LOCSETENT	0		/* allow |locinfo_setentry()| */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written as a KSH built-in command.

	= 2016-07-27, David A­D­ Morano
	This subroutine was enhanced to include an approximation of the function
	previously embodoed in the FMT program.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ linefold [<file(s)> ...] [<options>]


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
#include	<char.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<userinfo.h>
#include	<vecstr.h>
#include	<linefold.h>
#include	<wordfill.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_linefold.h"
#include	"defs.h"
#include	"proglog.h"


/* local defines */

#define	CBUFLEN		COLUMNS

#ifndef	TABCOLS
#define	TABCOLS		8
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif /* LINEBUFLEN */

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	mkplogid(char *,int,cchar *,int) ;
extern int	mksublogid(char *,int,cchar *,int) ;
extern int	ncolstr(int,int,cchar *,int) ;
extern int	shio_writeblanks(SHIO *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthex(cchar *,int,cchar *,int) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnrchr(cchar *,int,int) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		linelen:1 ;
	uint		indent:1 ;
	uint		split:1 ;	/* "split" line mode on LINEFILL */
	uint		crown:1 ;	/* "crown" mode (not implemented) */
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PROGINFO	*pip ;
	int		linelen ;
	int		indent ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procfile(PROGINFO *,void *,char *,int,cchar *) ;
static int	procfill(PROGINFO *,void *,char *,int,cchar *) ;
static int	procfilldump(PROGINFO *,void *,WORDFILL *,int) ;
static int	procfold(PROGINFO *,void *,char *,int,cchar *) ;
static int	procfoldline(PROGINFO *,void *,cchar *,int) ;
static int	procoutline(PROGINFO *,void *,int,cchar *,int) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procuserinfo_logid(PROGINFO *) ;

static int	proclog_info(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_linelen(LOCINFO *,cchar *) ;
static int	locinfo_indent(LOCINFO *) ;
#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"pm",
	"sn",
	"af",
	"ef",
	"of",
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
	argopt_ef,
	argopt_of,
	argopt_overlast
} ;

static const PIVARS	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
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

static const char	*akonames[] = {
	"linelen",
	"indent",
	NULL
} ;

enum akonames {
	akoname_linelen,
	akoname_indent,
	akoname_overlast
} ;

static const uchar	aterms[] = {
	0x00, 0x2E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	*progmodes[] = {
	"linefold",
	"linefill",
	"fmt",
	"mfmt",
	NULL
} ;

enum progmodes {
	progmode_linefold,
	progmode_linefill,
	progmode_fmt,
	progmode_mfmt,
	progmode_overlast
} ;


/* exported subroutines */


int b_linefold(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_linefold) */


int b_linefill(int argc,cchar *argv[],void *contextp)
{
	return b_linefold(argc,argv,contextp) ;
}
/* end subroutine (b_linefill) */


int b_fmt(int argc,cchar *argv[],void *contextp)
{
	return b_linefold(argc,argv,contextp) ;
}
/* end subroutine (b_fmt) */


int b_mfmt(int argc,cchar *argv[],void *contextp)
{
	return b_linefold(argc,argv,contextp) ;
}
/* end subroutine (b_mfmt) */


int p_linefold(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_linefold) */


int p_linefill(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_linefill) */


int p_fmt(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_fmt) */


int p_mfmt(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_mfmt) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	SHIO		errfile ;

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

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*pm = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_linefold: starting DFD=%u\n",rs) ;
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

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->f.logprog = OPT_LOGPROG ;

	pip->lip = lip ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,0) ;
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
	        const int	ach = MKCHAR(argp[1]) ;

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

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program mode */
	                case argopt_pm:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pm = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pm = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
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

/* argument-list file */
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

/* output file name */
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

/* quiet mode */
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* "crown" mode (for LINEFILL) */
	                    case 'c':
	                        lip->f.crown = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.crown = (rs > 0) ;
	                            }
	                        }
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

/* quiet mode */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* "split"-line mode (for LINEFILL) */
	                    case 's':
	                        lip->f.split = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.split = (rs > 0) ;
	                            }
	                        }
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

/* line-length (width) */
	                    case 'w':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					rs = optvalue(argp,argl) ;
	                                lip->linelen = rs ;
				    }
	                        } else
	                            rs = SR_INVALID ;
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

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("b_linefold: args-out rs=%d\n",rs) ;
#endif

	if (rs < 0) goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_linefold: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

/* get our program mode */

	if (pm == NULL) pm = pip->progname ;

	if ((pip->progmode = matstr(progmodes,pm,-1)) >= 0) {
	    if (pip->debuglevel > 0) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: pm=%s (%u)\n" ;
	        shio_printf(pip->efp,fmt,pn,pm,pip->progmode) ;
	    }
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid program-mode (%s)\n" ;
	    shio_printf(pip->efp,fmt,pn,pm) ;
	    ex = EX_USAGE ;
	    rs = SR_INVALID ;
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
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_linefold: pr=%s\n",pip->pr) ;
	    debugprintf("b_linefold: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
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

/* some initialization */

	if (afname == NULL) afname = getourenv(pip->envv,VARAFNAME) ;

	if ((rs = procopts(pip,&akopts)) >= 0) {
	    if ((rs = locinfo_linelen(lip,argval)) >= 0) {
	        if ((rs = locinfo_indent(lip)) >= 0) {
		    if (pip->debuglevel > 0) {
			const int	lw = lip->linelen ;
			const int	ind = lip->indent ;
			cchar	*pn = pip->progname ;
			cchar	*fmt ;
			fmt = "%s: linelen=%u\n" ;
			shio_printf(pip->efp,fmt,pn,lw) ;
			fmt = "%s: indent=%u\n" ;
			shio_printf(pip->efp,fmt,pn,ind) ;
		    }
		}
	    }
	}

/* go */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
		    if ((rs = proglog_begin(pip,&u)) >= 0) {
		        if ((rs = proclog_info(pip)) >= 0) {
	        	    cchar	*ofn = ofname ;
	        	    cchar	*afn = afname ;
	        	    rs = procargs(pip,&ainfo,&pargs,ofn,afn) ;
		        } /* end if (proclog_info) */
		        rs1 = proglog_end(pip) ;
	      	        if (rs >= 0) rs = rs1 ;
		    } /* end if (proglog) */
	            rs1 = procuserinfo_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
		} /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: userinfo failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    } /* end if (userinfo) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

/* done! */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    default:
	        if (! pip->f.quiet) {
	            cchar	*pn = pip->progname ;
	            cchar	*fmt = "%s: could not process (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	        }
		break ;
	    case SR_PIPE:
		break ;
	    } /* end switch */
	    ex = mapex(mapexs,rs) ;
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_linefold: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_linefold: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,
	    "%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<files(s)> ...] [-af <afile>] [-of <ofile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-w <linelen>] [-o indent=<indent>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case akoname_linelen:
	                    if (! lip->final.linelen) {
	                        lip->have.linelen = TRUE ;
	                        lip->final.linelen = TRUE ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            lip->linelen = rs ;
	                        }
	                    }
	                    break ;
	                case akoname_indent:
	                    if (! lip->final.indent) {
	                        lip->have.indent = TRUE ;
	                        lip->final.indent = TRUE ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            lip->indent = rs ;
	                        }
	                    }
	                    break ;
	                default:
	                    rs = SR_INVALID ;
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } else
			rs = SR_INVALID ;

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	const int	ilen = (8*LINEBUFLEN) ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	char		*ibuf ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = uc_malloc((ilen+1),&ibuf)) >= 0) {
	    SHIO	ofile, *ofp = &ofile ;
	    if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	        int	pan = 0 ;
		int	cl ;
	        cchar	*cp ;

	        if (rs >= 0) {
	            int		ai ;
	            int		f ;
	            cchar	**argv = aip->argv ;
	            for (ai = 1 ; ai < aip->argc ; ai += 1) {

	                f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	                f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	                if (f) {
	                    cp = aip->argv[ai] ;
	                    if (cp[0] != '\0') {
	                        pan += 1 ;
	                        rs = procfile(pip,ofp,ibuf,ilen,cp) ;
	                        wlen += rs ;
	                    }
	                }

	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end for */
	        } /* end if (ok) */

	        if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	            SHIO	afile, *afp = &afile ;

	            if (strcmp(afn,"-") == 0) afn = STDINFNAME ;

	            if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	                const int	llen = LINEBUFLEN ;
	                char		lbuf[LINEBUFLEN + 1] ;

	                while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                    int	len = rs ;

	                    if (lbuf[len - 1] == '\n') len -= 1 ;
	                    lbuf[len] = '\0' ;

	                    if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
				lbuf[(cp+cl)-lbuf] = '\0' ;
	                        if (cp[0] != '#') {
	                            pan += 1 ;
	                            rs = procfile(pip,ofp,ibuf,ilen,cp) ;
	                            wlen += rs ;
	                        }
	                    }

	                    if (rs >= 0) rs = lib_sigterm() ;
	                    if (rs >= 0) rs = lib_sigintr() ;
	                    if (rs < 0) break ;
	                } /* end while (reading lines) */

	                rs1 = shio_close(afp) ;
	                if (rs >= 0) rs = rs1 ;
	            } else {
	                if (! pip->f.quiet) {
			    fmt = "%s: inaccessible argument-list (%d)\n" ;
	                    shio_printf(pip->efp,fmt,pn,rs) ;
	                    shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	                }
	            } /* end if */

	        } /* end if (procesing file argument file list) */

	        if ((rs >= 0) && (pan == 0)) {

	            cp = "-" ;
	            pan += 1 ;
	            rs = procfile(pip,ofp,ibuf,ilen,cp) ;
	            wlen += rs ;

	        } /* end if (standard-input) */

	        rs1 = shio_close(ofp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        fmt = "%s: inaccessible output (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	    }
	    uc_free(ibuf) ;
	} /* end if (m-a) */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp,"%s: written=%u\n",pn,wlen) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


/* process a file */
static int procfile(PROGINFO *pip,void *ofp,char *ibuf,int ilen,cchar *fn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	switch (pip->progmode) {
	case progmode_linefold:
	    rs = procfold(pip,ofp,ibuf,ilen,fn) ;
	    wlen += rs ;
	    break ;
	case progmode_linefill:
	case progmode_fmt:
	case progmode_mfmt:
	    if (lip->f.split) {
	        rs = procfold(pip,ofp,ibuf,ilen,fn) ;
	        wlen += rs ;
	    } else {
	        rs = procfill(pip,ofp,ibuf,ilen,fn) ;
	        wlen += rs ;
	    }
	    break ;
	} /* end switch */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */


static int procfill(PROGINFO *pip,void *ofp,char *ibuf,int ilen,cchar *fn)
{
	SHIO		ifile, *ifp = &ifile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_linefold/procfile: fn=%s\n",fn) ;
#endif

	if (fn[0] == '-') fn = STDINFNAME ;

	if ((rs = shio_open(ifp,fn,"r",0666)) >= 0) {
	    WORDFILL	w ;
	    int		f_par = FALSE ;
	    if ((rs = wordfill_start(&w,ibuf,0)) >= 0) {
		const int	tcols = TABCOLS ;
		int		icols = -1 ;
	        int		cl ;
	        cchar		*cp ;

	        while ((rs = shio_readline(ifp,ibuf,ilen)) > 0) {
		    int	len = rs ;

		    if (ibuf[len-1] == '\n') len -= 1 ;

		    if ((cl = sfshrink(ibuf,len,&cp)) > 0) {
		        f_par = TRUE ;
		        if (icols < 0) {
			    icols = ncolstr(tcols,0,ibuf,(cp-ibuf)) ;
			}
		        rs = wordfill_addline(&w,cp,cl) ;
		    } else {
		        if (f_par) {
			    f_par = FALSE ;
			    rs = procfilldump(pip,ofp,&w,icols) ;
			    wlen += rs ;
			    icols = -1 ;
		        }
		        if (rs >= 0) {
		            rs = procoutline(pip,ofp,0,ibuf,0) ;
		            wlen += rs ;
		        }
		    }

		    if (rs >= 0) rs = lib_sigterm() ;
	  	    if (rs >= 0) rs = lib_sigintr() ;
		    if (rs < 0) break ;
	        } /* end while */

		if ((rs >= 0) && f_par) {
			rs = procfilldump(pip,ofp,&w,icols) ;
			wlen += rs ;
		}

		rs1 = wordfill_finish(&w) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (wordfill) */

	    rs1 = shio_close(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    if (! pip->f.quiet) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt ;
		fmt = "%s: inaccessible source (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        shio_printf(pip->efp,"%s: file=%s\n",pn,fn) ;
	    }
	} /* end if (file-output) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_linefold/procfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfill) */


static int procfilldump(PROGINFO *pip,void *ofp,WORDFILL *wp,int icols)
{
	LOCINFO		*lip = pip->lip ;
	const int	clen = CBUFLEN ;
	int		rs = SR_OK ;
	int		lcols ;
	int		cl ;
	int		cbl ;
	int		wlen = 0 ;
	char		cbuf[CBUFLEN+1] ;

	lcols = lip->linelen ;
	cbl = MIN(clen,(lcols-icols)) ;

	        while (rs >= 0) {
	            cl = wordfill_mklinefull(wp,cbuf,cbl) ;
	            if ((cl == 0) || (cl == SR_NOTFOUND)) break ;
	            rs = cl ;

	            if (rs >= 0) {
	                rs = procoutline(pip,ofp,icols,cbuf,cl) ;
	                wlen += rs ;
	            }

	        } /* end while (full lines) */

	        if (rs >= 0) {
	            if ((cl = wordfill_mklinepart(wp,cbuf,cbl)) > 0) {
	                rs = procoutline(pip,ofp,icols,cbuf,cl) ;
	                wlen += rs ;
	            } else if (cl != SR_NOTFOUND) {
	                rs = cl ;
		    }
	        } /* end if (partial lines) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfilldump) */


static int procoutline(PROGINFO *pip,void *ofp,int icols,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (icols > 0) {
	    rs = shio_writeblanks(ofp,icols) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = shio_print(ofp,sp,sl) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procoutline) */


/* process a file (fold) */
static int procfold(PROGINFO *pip,void *ofp,char *ibuf,int ilen,cchar *fn)
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ifile, *ifp = &ifile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_linefold/procfile: fn=%s\n",fn) ;
#endif

	if (fn[0] == '-') fn = STDINFNAME ;

	if ((rs = shio_open(ifp,fn,"r",0666)) >= 0) {
	    const int	lw = lip->linelen ;
	    while ((rs = shio_readline(ifp,ibuf,ilen)) > 0) {
		int	len = rs ;
		int	cols ;

		if (ibuf[len-1] == '\n') len -= 1 ;

		cols = ncolstr(TABCOLS,0,ibuf,len) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("linefold/procfile: lw=%u ncols=%u\n",lw,cols) ;
#endif

		if (cols > lw) {
		    rs = procfoldline(pip,ofp,ibuf,len) ;
		    wlen += rs ;
		} else {
		    rs = shio_print(ofp,ibuf,len) ;
		    wlen += rs ;
		}

		if (rs >= 0) rs = lib_sigterm() ;
	  	if (rs >= 0) rs = lib_sigintr() ;
		if (rs < 0) break ;
	    } /* end while */
	    rs1 = shio_close(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    if (! pip->f.quiet) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt ;
		fmt = "%s: inaccessible source (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        shio_printf(pip->efp,"%s: file=%s\n",pn,fn) ;
	    }
	} /* end if (file-output) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_linefold/procfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfold) */


static int procfoldline(PROGINFO *pip,void *ofp,cchar sbuf[],int slen)
{
	LOCINFO		*lip = pip->lip ;
	LINEFOLD	f ;
	int		rs ;
	int		rs1 ;
	int		lw ;
	int		ind ;
	int		wlen = 0 ;
	lw = lip->linelen ;
	ind = lip->indent ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("linefold/procfoldline: ent lw=%u ind=%u\n",lw,ind) ;
#endif
	if ((rs = linefold_start(&f,lw,ind,sbuf,slen)) >= 0) {
	    int		i ;
	    cchar	*lp ;
	    for (i = 0 ; (rs = linefold_getline(&f,i,&lp)) > 0 ; i += 1) {
		const int	ll = rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("linefold/procfoldline: i=%u ll=%u\n",i,ll) ;
#endif
		if (i > 0) {
		    rs = shio_writeblanks(ofp,ind) ;
		    wlen += rs ;
		}
		if (rs >= 0) {
		    rs = shio_print(ofp,lp,ll) ;
		    wlen += rs ;
		}
		if (rs < 0) break ;
	    } /* end for */
	    rs1 = linefold_finish(&f) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (linefold) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfoldline) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->gecosname = uip->gecosname ;
	pip->realname = uip->realname ;
	pip->name = uip->name ;
	pip->fullname = uip->fullname ;
	pip->mailname = uip->mailname ;
	pip->org = uip->organization ;
	pip->logid = uip->logid ;
	pip->pid = uip->pid ;
	pip->uid = uip->uid ;
	pip->euid = uip->euid ;
	pip->gid = uip->gid ;
	pip->egid = uip->egid ;

#ifdef	COMMENT
	if (rs >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    cchar	*nn = pip->nodename ;
	    cchar	*dn = pip->domainname ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        cchar	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}
#endif /* COMMENT */

	if (rs >= 0) {
	    rs = procuserinfo_logid(pip) ;
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (procuserinfo_end) */


static int procuserinfo_logid(PROGINFO *pip)
{
	int		rs ;
	if ((rs = lib_runmode()) >= 0) {
	    if (rs & KSHLIB_RMKSH) {
	        if ((rs = lib_serial()) >= 0) {
	            const int	s = rs ;
	            const int	plen = LOGIDLEN ;
	            const int	pv = pip->pid ;
	            cchar	*nn = pip->nodename ;
	            char	pbuf[LOGIDLEN+1] ;
	            if ((rs = mkplogid(pbuf,plen,nn,pv)) >= 0) {
	                const int	slen = LOGIDLEN ;
	                char		sbuf[LOGIDLEN+1] ;
	                if ((rs = mksublogid(sbuf,slen,pbuf,s)) >= 0) {
	                    cchar	**vpp = &pip->logid ;
	                    rs = proginfo_setentry(pip,vpp,sbuf,rs) ;
	                }
	            }
	        } /* end if (lib_serial) */
	    } /* end if (runmode-KSH) */
	} /* end if (lib_runmode) */
	return rs ;
}
/* end subroutine (procuserinfo_logid) */


static int proclog_info(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	cchar		*fmt = "lw=%u ind=%u" ;
	return proglog_printf(pip,fmt,lip->linelen,lip->indent) ;
}
/* end subroutine (proclog_info) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->indent = -1 ;

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


static int locinfo_linelen(LOCINFO *lip,cchar *argval)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if ((rs >= 0) && (lip->linelen == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    lip->linelen = rs ;
	}
	if ((rs >= 0) && (lip->linelen == 0)) {
	    if (pip->progmode == progmode_mfmt) {
	        lip->linelen = MAILMSGLINELEN ;
	    }
	}
	if ((rs >= 0) && (lip->linelen == 0)) {
	    cchar	*cp = getourenv(pip->envv,VARCOLUMNS) ;
	    if ((cp != NULL) && (cp[0] != '\0')) {
	        rs = optvalue(cp,-1) ;
	        lip->linelen = rs ;
	    }
	}
	if ((rs >= 0) && (lip->linelen == 0)) {
	    lip->linelen = COLUMNS ;
	}
	return rs ;
}
/* end subroutine (locinfo_linelen) */


static int locinfo_indent(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if ((rs >= 0) && (lip->indent < 0)) {
	    cchar	*cp = getourenv(pip->envv,VARINDENT) ;
	    if ((cp != NULL) && (cp[0] != '\0')) {
	        rs = optvalue(cp,-1) ;
	        lip->indent = rs ;
	    }
	}
	if ((rs >= 0) && (lip->indent < 0)) {
	    lip->indent = DEFINDENT ;
	}
	return rs ;
}
/* end subroutine (locinfo_indent) */


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


