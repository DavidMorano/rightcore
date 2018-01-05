/* b_tabsvt */

/* KSH built-in version of 's(1d)' */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_DEBUGHEXBUF	0
#define	CF_LINES25	0		/* 25 lines? */
#define	CF_DATEBLINK	0
#define	CF_BASIC	1		/* basic capabilities */
#define	CF_EC		1		/* enahanced character attributes */
#define	CF_SR		1		/* allow scroll-region */
#define	CF_VCV		1		/* VT cursor visibility */
#define	CF_ACV		1		/* ANSI cursor visibility */
#define	CF_SCV		1		/* SCREEN visibility */
#define	CF_SCS94	1
#define	CF_SCS94A	1
#define	CF_SCS96	1
#define	CF_CSR		1		/* general cursor save-restore */
#define	CF_VCSR		1		/* VT cursor save-restore */
#define	CF_ACSR		1		/* ANSI cursor save-restore */


/* revision history:

	= 2003-10-01, David A­D­ Morano
	This code is now a built-in command for the KSH shell.  This code (like
	almost all KSH built-in commands) can also compile as a stand-alone
	program (independent process under the UNIX® System).  The independent
	program created using this code replaces the old S program.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ tabsvt


*******************************************************************************/


#include	<envstandards.h>

#if	defined(SFIO) || defined(SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	defined(KSHBUILTIN) || defined(KSHBUILTIN > 0)
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<termios.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<vecstr.h>
#include	<keyopt.h>
#include	<sbuf.h>
#include	<termstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"tabsvt_config.h"
#include	"defs.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		2048
#endif

#undef	CODEBUFLEN
#define	CODEBUFLEN	40

#undef	MCBUFLEN
#define	MCBUFLEN	(4 + 4)

#undef	MNBUFLEN
#define	MNBUFLEN	60

#undef	HEXBUFLEN
#define	HEXBUFLEN	100

#define	DEFTERMSPEC	"default"

#define	TCF_MDEFAULT	0x0000	/* default */
#define	TCF_MEC		0x0001	/* enhanced character attributes */
#define	TCF_MVCV	0x0002	/* cursor visibility (VT) */
#define	TCF_MACV	0x0004	/* cursor visibility (ANSI) */
#define	TCF_MSCV	0x1000	/* cursor visibility (SCREEN) */
#define	TCF_MPSF	0x0008	/* has a prefered supplimental font */
#define	TCF_MSCS94	0x0010	/* supplemental character set 94 */
#define	TCF_MSCS96	0x0020	/* supplemental character set 96 */
#define	TCF_MSD		0x0040	/* has a status display (line) */
#define	TCF_MSCS94A	0x0080	/* supplemental character set 94a */
#define	TCF_MSR		0x0100	/* has setable line-scrolling regions */
#define	TCF_MSL		0x0400	/* setable number of lines */
#define	TCF_MVCSR	0x0200	/* cursor save-restore (VT) */
#define	TCF_MACSR	0x0800	/* cursor save-restore (ANSI) */
#define	TCF_MACSRS	0x2000	/* cursor save-restore (ANSI) is screwed */

#define	TCF_MBASIC	(TCF_MSR)
#define	TCF_MVT		(TCF_MSR | TCF_MVCSR)
#define	TCF_MVTE	(TCF_MSR | TCF_MVCSR | TCF_MEC)
#define	TCF_MVTADV	\
	(TCF_MVTE) | \
	(TCF_MPSF | TCF_MSCS94 | TCF_MSCS96 | TCF_MVCV | TCF_MSD) | \
	(TCF_MSL)

#define	TCF_MSCREEN	\
	(TCF_MVTE | TCF_MVCV | TCF_MACV | TCF_MSCV | TCF_MACSR)

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	tcgetlines(int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	pcsmailcheck(const char *,char *,int,const char *) ;
extern int	nusers(const char *) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_std(time_t,char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		init:1 ;
	uint		all:1 ;
	uint		sd:1 ;
	uint		clear:1 ;
	uint		date:1 ;
	uint		scroll:1 ;
	uint		la:1 ;
	uint		mailcheck:1 ;
	uint		nusers:1 ;
} ;

struct locinfo {
	char		*prpcs ;
	char		*utfname ;
	LOCINFO_FL	have, f, changed, final ;
} ;

struct termtype {
	const char	*name ;
	uint		flags ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,SHIO *,int,const char *) ;
static int	gettermflags(PROGINFO *,const char *,int *) ;
static int	terminit(PROGINFO *,const char *,int,int,char *,int) ;
static int	termclear(PROGINFO *,const char *,int,int,char *,int) ;
static int	termdate(PROGINFO *,const char *,int,int,char *,int,time_t) ;
static int	bufsd(PROGINFO *,int,SBUF *,int,const char *,int) ;
static int	bufdiv(PROGINFO *,int,SBUF *,int,int,const char *,int) ;
static int	loadstrs(SBUF *,const char **) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"PCSROOT",
	"sn",
	"af",
	"ef",
	"of",
	"utf",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_pcsroot,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_utf,
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
	"init",
	"all",
	"sd",
	"clear",
	"date",
	"scroll",
	"la",
	"mailcheck",
	"nusers",
	NULL
} ;

enum akonames {
	akoname_init,
	akoname_all,
	akoname_sd,
	akoname_clear,
	akoname_date,
	akoname_scroll,
	akoname_la,
	akoname_mailcheck,
	akoname_nusers,
	akoname_overlast
} ;

static const struct termtype	terms[] = {
	{ "sun", 0 },
	{ "vt100", (TCF_MVT) },
	{ "ansi", (TCF_MSR | TCF_MEC | TCF_MACV | TCF_MACSR | TCF_MACSRS) },
	{ "vt101", (TCF_MVTE) },
	{ "vt102", (TCF_MVTE) },
	{ "vt220", (TCF_MVTE | TCF_MSCS94) },
	{ "xterm", (TCF_MVTE) },
	{ "xterm-color", (TCF_MVTE) },
	{ "screen", (TCF_MSCREEN | TCF_MSCS94 | TCF_MSCS94A) },
	{ "screen94a", (TCF_MSCREEN | TCF_MSCS94 | TCF_MSCS94A) },
	{ "screen96", (TCF_MSCREEN | TCF_MSCS94 | TCF_MSCS96) },
	{ "vt420", (TCF_MVTADV) },
	{ "vt430", (TCF_MVTADV) },
	{ "vt440", (TCF_MVTADV) },
	{ "vt520", (TCF_MVTADV) },
	{ "vt530", (TCF_MVTADV) },
	{ "vt540", (TCF_MVTADV) },
	{ NULL, 0 }
} ;

static const char	*s_basic[] = {
	"\017",				/* shift-in */
	"\033>",			/* numeric keypad mode */
	"\033[?1l",			/* regular cursor keys */
	NULL
} ;

static const char	*s_scroll[] = {
	"\033[1;24r",
	NULL
} ;

static const char	*s_ec[] = {
	"\033[m",			/* all character attributes off */
	"\033[4l",			/* insert-mode off */
	NULL
} ;

static const char	*s_vcv[] = {
	TERMSTR_S_VCUR,			/* VT set cursor ON */
	NULL
} ;

static const char	*s_acv[] = {
	TERMSTR_S_ACUR,			/* ANSI set cursor ON */
	NULL
} ;

static const char	*s_scv[] = {
	TERMSTR_S_SCUR,			/* SCREEN set cursor ON */
	NULL
} ;

static const char	*s_psf[] = {
	"\033P1!uA\033\\",	/* designate ISO Latin-1 as supplimental */
	NULL
} ;

static const char	*s_scs94[] = {
	"\033(B",		/* map ASCII (94) to G0 */
	"\033)0",		/* DEC Special Graphic (94) as G1 */
	"\033+>",		/* map DEC Technical (94) to G3 */
	"\017",			/* ASCII SHIFT-IN (G0 into GL) */
	NULL
} ;

/* this is the holding place for the failings of the SCREEN program! */
static const char	*s_scs94a[] = {
	"\033*A",		/* map ISO Latin-1 (96) as G2 */
	"\033\175",		/* lock shift G2 into GR */
	NULL
} ;

static const char	*s_scs96[] = {
	"\033.A",		/* map ISO Latin-1 (96) as G2 */
	"\033\175",		/* lock shift G2 into GR */
	NULL
} ;

static const char	*s_clear[] = {
	"\033[H\033[J",			/* clear screen */
	NULL
} ;

static const char	blanks[] = "        " ;


/* exported subroutines */


int b_tabsvt(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    cchar	**envv = (const char **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_tabsvt) */


int p_tabsvt(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_tabsvt) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO	li, *lip = &li ;
	BITS		pargs ;
	KEYOPT		akopts ;
	SHIO		errfile ;
	SHIO		outfile, *ofp = &outfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		n, i, j ;
	int		size ;
	int		bl ;
	int		fd ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*prpcs = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*utfname = NULL ;
	const char	*termspec = NULL ;
	const char	*cp ;
	char		buf[BUFLEN + 1] ;
	char		pcsroot[MAXPATHLEN + 1] ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_tabsvt: starting DFD=%d\n",rs) ;
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

	pip->lip = &li ;
	memset(&li,0,sizeof(LOCINFO)) ;

	lip->have.clear = lip->f.clear = TRUE ;
	lip->have.scroll = lip->f.scroll = TRUE ;
	lip->have.la = lip->f.la = TRUE ;
	lip->have.mailcheck = lip->f.mailcheck = TRUE ;
	lip->have.nusers = lip->f.nusers = TRUE ;

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

	            argval = (argp + 1) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            aol = argl - 1 ;
	            akp = aop ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + aol - avp ;
	                f_optequal = TRUE ;

	            } else {

	                akl = aol ;
	                avl = 0 ;

	            }

/* keyword match or only key letters? */

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

/* program-root */
	                case argopt_root:
	                        if (argr > 0) {
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl)
	                        pr = argp ;
				} else
	                            rs = SR_INVALID ;
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
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

/* argument-list file name */
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

/* UTMP DB */
	                case argopt_utf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            utfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            utfname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* PCS program-root */
	                case argopt_pcsroot:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            prpcs = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            prpcs = argp ;
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

/* terminal specification (terminal type) */
	                    case 'T':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            termspec = argp ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'a':
	                        lip->have.all = TRUE ;
	                        lip->final.all = TRUE ;
	                        lip->f.all = TRUE ;
	                        break ;

	                    case 'd':
	                        lip->have.date = TRUE ;
	                        lip->final.date = TRUE ;
	                        lip->f.date = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.date = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'i':
	                        lip->have.init = TRUE ;
	                        lip->final.init = TRUE ;
	                        lip->f.init = TRUE ;
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

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_tabsvt: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version)
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

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
	    shio_printf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

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

	if (f_version || f_usage || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* load up the environment options */

	rs = procopts(pip,&akopts) ;

/* special option handling */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_tabsvt: f_init=%u\n",lip->f.init) ;
	    debugprintf("b_tabsvt: f_all=%u\n",lip->f.all) ;
	    debugprintf("b_tabsvt: f_clear=%u\n",lip->f.clear) ;
	    debugprintf("b_tabsvt: f_date=%u\n",lip->f.date) ;
	}
#endif

/* get program-root for PCS if it is needed */

	pcsroot[0] = '\0' ;
	if ((rs >= 0) && lip->f.mailcheck) {
	    if ((prpcs != NULL) && (prpcs[0] != '\0')) {
	        lip->prpcs = prpcs ;
	    } else {
	        rs1 = mkpr(pcsroot,MAXPATHLEN,VARPRPCS,pip->domainname) ;
	        if (rs1 >= 0)
	            lip->prpcs = pcsroot ;
	    }
	} /* end if (mailcheck) */

	if (lip->f.nusers && (lip->utfname == NULL)) {
	    lip->utfname = utfname ;
	} /* end if (nusers) */

/* do we have specified an extended operation? */

#ifdef	COMMENT
	for (ai = 0 ; ai <= ai_max ; ai += 1) {
	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (f) {
	        f_got = TRUE ;
	        break ;
	    }
	} /* end for (handling positional arguments) */
#endif /* COMMENT */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: f_all=%u\n",
	        pip->progname,lip->f.all) ;
	    shio_printf(pip->efp,"%s: f_date=%u\n",
	        pip->progname,lip->f.date) ;
	    shio_printf(pip->efp,"%s: f_scroll=%u\n",
	        pip->progname,lip->f.scroll) ;
	}

/* find the terminal type, if we have it */

	if (termspec == NULL) termspec = getourenv(envv,VARTERM) ;
	if (termspec == NULL) termspec = DEFTERMSPEC ;

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: term=%s\n",
	        pip->progname,termspec) ;
	}

/* check for an output */

	if (rs >= 0) {

	if ((ofname == NULL) || (ofname[0] == '\0'))
	    ofname = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofname,"wct",0666)) >= 0) {
	    if ((rs = shio_getfd(ofp)) >= 0) {
	        const int	fd = rs ;
	        if (isatty(fd)) {
	            rs = process(pip,ofp,fd,termspec) ;
	        } else {
	            rs = SR_BADFD ;
		}
	    } /* end if (got an FD) */
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (opened output) */

	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_NOENT:
	    case SR_BADFD:
	        ex = EX_CANTCREAT ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
		break ;
	    } /* end switch */
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
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_tabsvt: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-i] [-T <termtype>] [-d] [-a]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
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
	const char	*cp ;

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

	        case akoname_all:
	            if (! lip->final.all) {
	                lip->have.all = TRUE ;
	                lip->final.all = TRUE ;
	                lip->f.all = TRUE ;
	                if (vl > 0) {
			    rs = optbool(vp,vl) ;
	                    lip->f.all = (rs > 0) ;
			}
	            }
	            break ;

	        case akoname_init:
	            if (! lip->final.init) {
	                lip->have.init = TRUE ;
	                lip->final.init = TRUE ;
	                lip->f.init = TRUE ;
	                if (vl > 0) {
			    rs = optbool(vp,vl) ;
	                    lip->f.init = (rs > 0) ;
			}
	            }
	            break ;

	        case akoname_sd:
	            if (! lip->final.sd) {
	                lip->have.sd = TRUE ;
	                lip->final.sd = TRUE ;
	                lip->f.sd = TRUE ;
	                if (vl > 0) {
			    rs = optbool(vp,vl) ;
	                    lip->f.sd = (rs > 0) ;
			}
	            }
	            break ;

	        case akoname_clear:
	            if (! lip->final.clear) {
	                lip->have.clear = TRUE ;
	                lip->final.date = TRUE ;
	                lip->f.clear = TRUE ;
	                if (vl > 0) {
			    rs = optbool(vp,vl) ;
	                    lip->f.clear = (rs > 0) ;
			}
	            }
	            break ;

	        case akoname_date:
	            if (! lip->final.date) {
	                lip->have.date = TRUE ;
	                lip->final.date = TRUE ;
	                lip->f.date = TRUE ;
	                if (vl > 0) {
			    rs = (vp,vl) ;
	                    lip->f.date = (rs > 0) ;
			}
	            }
	            break ;

	        case akoname_scroll:
	            if (! lip->final.scroll) {
	                lip->have.scroll = TRUE ;
	                lip->final.scroll = TRUE ;
	                lip->f.scroll = TRUE ;
	                if (vl > 0) {
			    rs = optbool(vp,vl) ;
	                    lip->f.scroll = (rs > 0) ;
			}
	            }
	            break ;

	        case akoname_la:
	            if (! lip->final.la) {
	                lip->have.la = TRUE ;
	                lip->final.la = TRUE ;
	                lip->f.la = TRUE ;
	                if (vl > 0) {
			    rs = optbool(vp,vl) ;
	                    lip->f.la = (rs > 0) ;
			}
	            }
	            break ;

	        case akoname_mailcheck:
	            if (! lip->final.mailcheck) {
	                lip->have.mailcheck = TRUE ;
	                lip->final.mailcheck = TRUE ;
	                lip->f.mailcheck = TRUE ;
	                if (vl > 0) {
			    rs = optbool(vp,vl) ;
	                    lip->f.mailcheck = (rs > 0) ;
			}
	            }
	            break ;

	        case akoname_nusers:
	            if (! lip->final.nusers) {
	                lip->have.nusers = TRUE ;
	                lip->final.nusers = TRUE ;
	                lip->f.nusers = TRUE ;
	                if (vl > 0) {
			    rs = optbool(vp,vl) ;
	                    lip->f.nusers = (rs > 0) ;
			}
	            }
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


static int process(pip,ofp,fd,termspec)
PROGINFO	*pip ;
SHIO		*ofp ;
int		fd ;
const char	termspec[] ;
{
	LOCINFO	*lip = pip->lip ;
	int		rs, rs1 ;
	int		termflags, lines ;
	int		bl ;
	char		buf[BUFLEN + 1] ;
	char		*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_s/process: TERM=%s\n",termspec) ;
#endif

	rs = gettermflags(pip,termspec,&termflags) ;

	if (rs < 0)
	    goto ret0 ;

	lines = -1 ;
	if (termflags & TCF_MSR) {

	    rs1 = tcgetlines(fd) ;
	    lines = rs1 ;

	    if ((rs1 < 0) || (lines < 0)) {

	        rs1 = -1 ;
	        if ((cp = getourenv(pip->envv,VARLINES)) != NULL)
	            rs1 = cfdeci(cp,-1,&lines) ;

	        if ((rs1 < 0) || (lines < 0))
	            lines = DEFLINES ;

	    }

	} /* end if (getting lines for settable scrolling regions) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_s/process: lines=%d\n",lines) ;
#endif

	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: lines=%u\n",
	        pip->progname,lines) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_s/process: f_init=%u\n",lip->f.init) ;
#endif

	if ((rs >= 0) && lip->f.init) {

	    rs = terminit(pip,termspec,termflags,lines,buf,BUFLEN) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("b_s/process: terminit() rs=%d\n",rs) ;
#endif

	    bl = rs ;
	    if (rs >= 0)
	        rs = shio_write(ofp,buf,bl) ;

	} /* end if (terminit) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_s/process: f_clear=%u\n",lip->f.clear) ;
#endif

	if ((rs >= 0) && lip->f.clear) {

	    rs = termclear(pip,termspec,termflags,lines,buf,BUFLEN) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("b_s/process: termclear() rs=%d\n",rs) ;
#endif

	    bl = rs ;
	    if (rs >= 0)
	        rs = shio_write(ofp,buf,bl) ;

	} /* end if (termclear) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_s/process: f_date=%u\n",lip->f.date) ;
#endif

	if ((rs >= 0) && lip->f.date) {

	    time_t	daytime = time(NULL) ;

	    char	timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("b_s/process: f_sd=%u\n",lip->f.sd) ;
#endif

	    if (lip->f.sd) {

	        rs = termdate(pip,termspec,termflags,lines,buf,BUFLEN,
	            daytime) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b_s/process: termdate() rs=%d\n",rs) ;
#endif

	        bl = rs ;
	        if (rs >= 0)
	            rs = shio_write(ofp,buf,bl) ;

	    } else
	        rs = shio_printf(ofp,"%t\n",
	            timestr_std(daytime,timebuf),19) ;

	} /* end if (date) */

ret0:
	return rs ;
}
/* end subroutine (process) */


static int gettermflags(pip,termspec,rp)
PROGINFO	*pip ;
const char	termspec[] ;
int		*rp ;
{
	int		rs = SR_OK ;
	int		i ;
	int		termflags ;
	char		*cp = (char *) termspec ;

	termflags = TCF_MDEFAULT ;

	if (cp != NULL) {

	    for (i = 0 ; terms[i].name != NULL ; i += 1) {

	        if (strcmp(terms[i].name,cp) == 0)
	            break ;

	    } /* end for */

	    if (terms[i].name != NULL)
	        termflags = terms[i].flags ;

	} /* end if (had a terminal type) */

	if (rp != NULL)
	    *rp = termflags ;

	return rs ;
}
/* end subroutine (gettermflags) */


/* create the data to set (reset-fix) the terminal state */
static int terminit(pip,termspec,termflags,lines,buf,buflen)
PROGINFO	*pip ;
const char	termspec[] ;
int		termflags ;
int		lines ;
char		buf[] ;
int		buflen ;
{
	LOCINFO	*lip = pip->lip ;
	SBUF		b ;
	int		rs ;
	int		bl ;
	int		f_curvis = FALSE ;
	char		cbuf[CODEBUFLEN + 1] ;

/* load all of the output strings into a common buffer */

	rs = sbuf_start(&b,buf,buflen) ;

	if (rs < 0)
	    goto ret0 ;

/* basic stuff */

#if	CF_BASIC
	loadstrs(&b,s_basic) ;
#endif

/* enhanced character attributes */

#if	CF_EC
	if (termflags & TCF_MEC)
	    loadstrs(&b,s_ec) ;
#endif

/* cursor visibility (VT) */

#if	CF_VCV
	if ((termflags & TCF_MVCV) && (! f_curvis)) {
	    f_curvis = TRUE ;
	    loadstrs(&b,s_vcv) ;
	}
#endif

/* cursor visibility (ANSI) */

#if	CF_ACV
	if ((termflags & TCF_MACV) && (! f_curvis)) {
	    f_curvis = TRUE ;
	    loadstrs(&b,s_acv) ;
	}
#endif

/* cursor visibility (SCREEN) */

#if	CF_SCV
	if ((termflags & TCF_MSCV) && (! f_curvis)) {
	    f_curvis = TRUE ;
	    loadstrs(&b,s_scv) ;
	}
#endif

/* supplemental character set 94 */

#if	CF_SCS94
	if (termflags & TCF_MSCS94)
	    loadstrs(&b,s_scs94) ;
#endif

/* supplemental character set 94a */

#if	CF_SCS94A
	if (termflags & TCF_MSCS94A)
	    loadstrs(&b,s_scs94a) ;
#endif

/* supplemental character set 96 */

#if	CF_SCS96
	if (termflags & TCF_MSCS96)
	    loadstrs(&b,s_scs96) ;
#endif

/* set ISO-Latin1 as supplemental character set */

	if (termflags & TCF_MPSF)
	    loadstrs(&b,s_psf) ;

/* set scroll region to full number of terminal lines */

#if	CF_SR
	if ((termflags & TCF_MSR) && (termflags & TCF_MACSRS)) {
	    if (lip->f.scroll && (! lip->f.clear))
	        lines = -1 ;
	}

	if (lip->f.scroll && (termflags & TCF_MSR) && (lines > 0)) {

#if	CF_CSR
	    if (termflags & TCF_MVCSR)
	        sbuf_strw(&b,TERMSTR_VCURS,-1) ;

	    else if (termflags & TCF_MACSR)
	        sbuf_strw(&b,TERMSTR_ACURS,-1) ;
#endif

	    bl = bufprintf(cbuf,CODEBUFLEN,"\033[1;%ur",lines) ;

	    sbuf_strw(&b,cbuf,bl) ;

#if	CF_CSR
	    if (termflags & TCF_MVCSR)
	        sbuf_strw(&b,TERMSTR_VCURR,-1) ;

	    else if (termflags & TCF_MACSR)
	        sbuf_strw(&b,TERMSTR_ACURR,-1) ;
#endif

	} /* end if (adjustable line scrolling regions) */
#endif /* CF_SR */

/* clear screen */

	if (lip->f.clear) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("b_s/terminit: clear\n") ;
#endif

	    lip->f.clear = FALSE ;
	    loadstrs(&b,s_clear) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("b_s/terminit: f_all=%u TERM=%s F_MSD=%u\n",
	            lip->f.all,termspec,((termflags & TCF_MSD) ? 1 : 0)) ;
#endif

	    if (lip->f.all && (termflags & TCF_MSD)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b_s/terminit: clear SD\n") ;
#endif

	        bl = bufprintf(cbuf,CODEBUFLEN,"%s\r%s%s",
	            TERMSTR_S_SD,TERMSTR_ED,TERMSTR_R_SD) ;

#ifdef	COMMENT
	        bl += bufprintf((buf + bl),BUFLEN,"\033[26;1H\r%s\033[1;1H%s",
	            TERMSTR_ED,TERMSTR_ED) ;
#endif

	        sbuf_strw(&b,cbuf,bl) ;

	    }

	} /* end if (clear) */

/* done */

	bl = sbuf_getlen(&b) ;

	sbuf_finish(&b) ;

ret0:
	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (terminit) */


/* clear the terminal */
static int termclear(pip,termspec,termflags,lines,buf,buflen)
PROGINFO	*pip ;
const char	termspec[] ;
int		termflags ;
int		lines ;
char		buf[] ;
int		buflen ;
{
	LOCINFO	*lip = pip->lip ;
	SBUF		b ;
	int		rs ;
	int		bl ;
	char		cbuf[CODEBUFLEN + 1] ;

/* load all of the output strings into a common buffer */

	rs = sbuf_start(&b,buf,buflen) ;

	if (rs < 0)
	    goto ret0 ;

/* clear the main screen */

	loadstrs(&b,s_clear) ;

/* clear the status display if there is and additionally asked to do so */

	if (lip->f.all && (termflags & TCF_MSD)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("b_s/termclear: clear SD\n") ;
#endif

	    bl = bufprintf(cbuf,CODEBUFLEN,"%s\r%s%s",
	        TERMSTR_S_SD,TERMSTR_ED,TERMSTR_R_SD) ;

	    sbuf_strw(&b,cbuf,bl) ;

	} /* end if */

	bl = sbuf_finish(&b) ;

ret0:
	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (termclear) */


#ifdef	COMMENT

static int termlines(pip,termflags,lines,buf,buflen,daytime)
PROGINFO	*pip ;
int		termflags ;
int		lines ;
char		buf[] ;
int		buflen ;
time_t		daytime ;
{
	SBUF	b ;

	int	rs ;
	int	bl ;


/* load all of the output strings into a common buffer */

	rs = sbuf_start(&b,buf,buflen) ;

	if (rs < 0)
	    goto ret0 ;

/* SCROLL begin */

#if	CF_LINES25

	sbuf_strw(&b,"\033[25*|",-1) ;

	sbuf_strw(&b,"\033[25t",-1) ;

	sbuf_strw(&b,"\033[25;1H25? ",-1) ;

	sbuf_strw(&b,"\033[1;1H",-1) ;

#ifdef	COMMENT
	sbuf_strw(&b,"\033[24t",-1) ;

	sbuf_strw(&b,"\033[24*|",-1) ;
#endif

#endif /* CF_LINES25 */

/* SCROLL end */

/* status line */

	{
	    char	timebuf[TIMEBUFLEN + 1] ;


	    if (termflags & TCF_MSD) {

	        bufsd(pip,termflags,&b,(80 - 19),
	            timestr_std(daytime,timebuf),19) ;

	    } else if ((termflags & TCF_MVCSR) || (termflags & TCF_MACSR)) {

	        bufdiv(pip,termflags,&b,0,(80 - 19),
	            timestr_std(daytime,timebuf),19) ;

	    } else {

	        sbuf_strw(&b,
	            timestr_std(daytime,timebuf),19) ;
	        sbuf_char(&b,'\n') ;
	    }

	} /* end block */

/* done */

	bl = sbuf_finish(&b) ;

ret0:
	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (termlines) */

#endif /* COMMENT */


static int termdate(pip,termspec,termflags,lines,buf,buflen,daytime)
PROGINFO	*pip ;
const char	termspec[] ;
int		termflags ;
int		lines ;
char		buf[] ;
int		buflen ;
time_t		daytime ;
{
	LOCINFO		*lip = pip->lip ;
	SBUF		b, lb ;
	int		rs ;
	int		rs1 ;
	int		bl, i ;
	char		timebuf[TIMEBUFLEN + 1] ;

/* load all of the output strings into a common buffer */

	rs = sbuf_start(&b,buf,buflen) ;

	if (rs < 0)
	    goto ret0 ;

	if (termflags & TCF_MSD) {

	    int		len ;
	    int		ncols = 0 ;
	    int		nfcols ;

	    char	linebuf[LINEBUFLEN + 1] ;


/* initialize a local line buffer */

	    if ((rs = sbuf_start(&lb,linebuf,LINEBUFLEN)) >= 0) {

/* number of users logged into machine (3-column field) */

	        nfcols = 3 ;
	        ncols += nfcols ;
	        if (rs >= 0) {

		    int		n ;

	            char	buf[7 + 1] ;
	            char	*pbp = (char *) blanks ;


		    rs1 = SR_OVERFLOW ;
	            if (lip->f.nusers) {

			rs1 = nusers(lip->utfname) ;
			n = rs1 ;
			if (rs1 >= 0) {

			    if (n > 99) n = 99 ;

			    rs1 = bufprintf(buf,7,"%2u ",n) ;

#if	CF_DEBUG && CF_DEBUGHEXBUF
	            if (DEBUGLEVEL(3)) {
			char	hexbuf[HEXBUFLEN + 1] ;
			mkhexstr(hexbuf,HEXBUFLEN,buf,nfcols) ;
	                debugprintf("b_s/termdate: nusers 1 hb=>%s<\n",
				hexbuf) ;
			}
#endif /* CF_DEBUG */

	                    if (rs1 >= 0)
	                        pbp = buf ;

			}

	            } /* end if (nusers) */

	            sbuf_strw(&lb,pbp,nfcols) ;

#if	CF_DEBUG && CF_DEBUGHEXBUF
	            if (DEBUGLEVEL(3)) {
			char	hexbuf[HEXBUFLEN + 1] ;
			mkhexstr(hexbuf,HEXBUFLEN,pbp,nfcols) ;
	                debugprintf("b_s/termdate: nusers nfcols=%u\n",
				nfcols) ;
	                debugprintf("b_s/termdate: nusers hb=>%s<\n",
				hexbuf) ;
	                debugprintf("b_s/termdate: nusers rs1=%d buf=>%t<\n",
	                    rs1,pbp,nfcols) ;
		}
#endif /* CF_DEBUG */

	        } /* end if (3-column field) */

/* machine load-average (4-column field) */

	        nfcols = 4 ;
	        ncols += nfcols ;
	        if (rs >= 0) {
	            char	labuf[12 + 1] ;
	            char	*pbp = (char *) blanks ;


	            if (lip->f.la) {

	                double	la[3] ;


	                rs1 = uc_getloadavg(la,3) ;

			if (rs1 >= 0) {

	                    for (i = 0 ; i < 3 ; i += 1) {
	                        if (la[i] > 9.9) la[i] = 9.9 ;
	                    }

	                    rs1 = bufprintf(labuf,12,"%3.1f ",
	                        la[0],la[1],la[2]) ;

	                    if (rs1 >= 0)
	                        pbp = labuf ;

	                } /* end if */

	            } /* end if (la) */

	            sbuf_strw(&lb,pbp,nfcols) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_s/termdate: labuf=>%t<\n",
	                    pbp,nfcols) ;
#endif

	        } /* end if (4-column field) */

/* number of mail messages (4-column field) */

	        nfcols = 4 ;
	        ncols += nfcols ;
	        if (rs >= 0) {

	            char	mcbuf[MCBUFLEN + 1] ;
	            char	mnbuf[MNBUFLEN + 1] ;
	            char	*pbp = (char *) blanks ;


#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_s/termdate: mailcheck=%u\n",
	                    lip->f.mailcheck) ;
#endif

	            mnbuf[0] = '\0' ;
	            mcbuf[0] = '\0' ;
	            if (lip->f.mailcheck) {

	                rs1 = pcsmailcheck(lip->prpcs,mnbuf,MNBUFLEN,"-") ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("b_s/termdate: pcsmailcheck() rs=%d\n",
	                        rs1) ;
#endif

	                if (rs1 > 0) {

	                    if (rs1 > 99) rs1 = 99 ;

	                    rs1 = bufprintf(mcbuf,MCBUFLEN,"%2u%c ",rs1,0xB6) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("b_s/termdate: bufprintf() rs=%d\n",
	                            rs1) ;
#endif

	                    if (rs1 >= 0)
	                        pbp = mcbuf ;

	                }

	            } /* end if (mailcheck) */

	            sbuf_strw(&lb,pbp,nfcols) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_s/termdate: mcbuf=>%t<\n",
	                    pbp,nfcols) ;
#endif

	        } /* end if (4-column field) */

/* time-of-day (19-column field) */

	        nfcols = 19 ;
	        ncols += nfcols ;
	        if (rs >= 0) {

	            sbuf_strw(&lb,
	                timestr_std(daytime,timebuf),nfcols) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_s/termdate: timebuf=>%t<\n",
	                    timebuf,nfcols) ;
#endif

	        } /* end if (19-column field) */

	        rs1 = sbuf_finish(&lb) ;

	        len = rs1 ;
	        if (rs >= 0)
	            rs = rs1 ;

	    } /* end if (linebuf) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("b_s/termdate: ncols=%u\n",ncols) ;
	        debugprintf("b_s/termdate: sdbuf=>%t<\n",linebuf,len) ;
	    }
#endif

	    if (rs >= 0)
	        rs = bufsd(pip,termflags,&b,(80 - ncols),
	            linebuf,len) ;

	} else if ((termflags & TCF_MVCSR) || (termflags & TCF_MACSR)) {

	    rs = bufdiv(pip,termflags,&b,0,(80 - 19),
	        timestr_std(daytime,timebuf),19) ;

	} else {

	    sbuf_strw(&b,
	        timestr_std(daytime,timebuf),19) ;
	    sbuf_char(&b,'\n') ;

	}

/* done */

	bl = sbuf_finish(&b) ;

ret0:
	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (termdate) */


static int bufsd(pip,termflags,bufp,x,buf,buflen)
PROGINFO	*pip ;
int		termflags ;
SBUF		*bufp ;
int		x ;
const char	buf[] ;
int		buflen ;
{
	int		rs ;
	int		len_s, len_e, len ;
	int		cf_adv = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bufsd: ent x=%d\n",x) ;
#endif

	if (x < 0) x = 0 ;

	rs = sbuf_getlen(bufp) ;

	len_s = rs ;
	if (rs < 0)
	    goto ret0 ;

/* status line */

#if	CF_ACV
	cf_adv = TRUE ;
#endif

	if (termflags & TCF_MVCV)
	    sbuf_strw(bufp,TERMSTR_R_VCUR,-1) ;

	else if (cf_adv && (termflags & TCF_MACV))
	    sbuf_strw(bufp,TERMSTR_R_ACUR,-1) ;

	sbuf_strw(bufp,TERMSTR_VCURS,-1) ;

	sbuf_strw(bufp,TERMSTR_S_SD,-1) ;

	{
	    int		codebuflen ;

	    char	codebuf[CODEBUFLEN + 1] ;


	    codebuflen = bufprintf(codebuf,CODEBUFLEN,
	        "\r\033[%uC",x) ;

	    if (codebuflen > 0)
	        sbuf_strw(bufp,codebuf,codebuflen) ;

	    sbuf_strw(bufp,buf,buflen) ;

	} /* end block */

	sbuf_strw(bufp,TERMSTR_R_SD,-1) ;

	sbuf_strw(bufp,TERMSTR_VCURR,-1) ;

	if (termflags & TCF_MVCV)
	    sbuf_strw(bufp,TERMSTR_S_VCUR,-1) ;

	else if (cf_adv && (termflags & TCF_MACV))
	    sbuf_strw(bufp,TERMSTR_S_ACUR,-1) ;

/* finish */

	rs = sbuf_getlen(bufp) ;

	len_e = rs ;
	if (rs < 0)
	    goto ret0 ;

	len = len_e - len_s ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bufsd: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (bufsd) */


static int bufdiv(pip,termflags,bufp,y,x,buf,buflen)
PROGINFO	*pip ;
int		termflags ;
SBUF		*bufp ;
int		y, x ;
const char	buf[] ;
int		buflen ;
{
	int		rs ;
	int		len_s, len_e, len ;
	int		cf_adv = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("bufdiv: ent x=%d\n",x) ;
	    debugprintf("bufdiv: buf=>%t<\n",buf,buflen) ;
	}
#endif

#if	CF_ACV
	cf_adv = TRUE ;
#endif

	if (x < 0) x = 0 ;
	if (y < 0) y = 0 ;

	rs = sbuf_getlen(bufp) ;

	len_s = rs ;
	if (rs < 0)
	    goto ret0 ;

/* status line */

	if (termflags & TCF_MVCV)
	    sbuf_strw(bufp,TERMSTR_R_VCUR,-1) ;

	else if (cf_adv && (termflags & TCF_MACV))
	    sbuf_strw(bufp,TERMSTR_R_ACUR,-1) ;

	sbuf_strw(bufp,TERMSTR_VCURS,-1) ;

	{
	    int	codebuflen ;

	    char	codebuf[CODEBUFLEN + 1] ;


	    codebuflen = bufprintf(codebuf,CODEBUFLEN,
	        "\033[%u;%uH", (y + 1), (x + 1)) ;

	    if (codebuflen > 0)
	        sbuf_strw(bufp,codebuf,codebuflen) ;

	    sbuf_strw(bufp,buf,buflen) ;

	} /* end block */

	sbuf_strw(bufp,TERMSTR_VCURR,-1) ;

	if (termflags & TCF_MVCV)
	    sbuf_strw(bufp,TERMSTR_S_VCUR,-1) ;

	else if (cf_adv && (termflags & TCF_MACV))
	    sbuf_strw(bufp,TERMSTR_S_ACUR,-1) ;

/* finish */

	rs = sbuf_getlen(bufp) ;

	len_e = rs ;
	if (rs < 0)
	    goto ret0 ;

	len = len_e - len_s ;

ret0:
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (bufdiv) */


/* load a set of strings into the buffer */
static int loadstrs(sbp,spp)
SBUF		*sbp ;
const char	**spp ;
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (spp != NULL) {

	    while (*spp != NULL) {

	        rs = sbuf_strw(sbp,*spp,-1) ;

	        if (rs < 0)
	            break ;

	        c += 1 ;
	        spp += 1 ;

	    } /* end while */

	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadstrs) */


