/* b_sysvar */

/* set the "system" variables at boot-up time */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_DEBUGFORK	0		/* debug fork problem */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This is a complete rewrite of the previous code that performed this
	function.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is the front-end for the program that sets the "system"
	varialbes at machine boot-up time.

	Synopsis:

	$ sysvar [-s] [-f <file(s)>] [<var(s)>]


	This code can be a built-in command to the KSH shell.  But it really
	needs to be SUID to either user or group to really work properly, so it
	is always made to be stand-alone for the time being.


*******************************************************************************/


#include	<envstandards.h>

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
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<nulstr.h>
#include	<vecstr.h>
#include	<hdbstr.h>
#include	<filebuf.h>
#include	<field.h>
#include	<char.h>
#include	<ids.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_sysvar.h"
#include	"defs.h"
#include	"varmk.h"
#include	"var.h"


/* local defines */

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	USERNAMELEN
#define	USERNAMELEN	32
#endif

#ifndef	GNAMELEN
#define	GNAMELEN	80		/* GECOS name length */
#endif

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	120		/* key-buffer length */
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	BUFLEN		MAX((4 * MAXPATHLEN),LINEBUFLEN)

#define	WORDEXPORT	"export"

#ifndef	TO_OPEN
#define	TO_OPEN		5
#endif

#ifndef	TO_READ
#define	TO_READ		5
#endif

#ifndef	DEFNVARS
#define	DEFNVARS	(200 * 1000)
#endif

#define	DEBFNAME	"sysvar.deb"
#define	PO_FILENAME	"file"

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	matpstr(cchar **,int,cchar *,int) ;
extern int	sichr(cchar *,int,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	vecstr_envfile(vecstr *,cchar *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	isalnumlatin(int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(const char *,int,const void *,int) ;
extern int	nprintf(cchar *,cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

#if	(CF_DEBUG || CF_DEBUGS) && CF_DEBUGFORK
extern int	debugfork(cchar *) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		query:1 ;
	uint		set:1 ;
	uint		dump:1 ;
	uint		list:1 ;
	uint		audit:1 ;
	uint		uniq:1 ;
	uint		star:1 ;
} ;

struct locinfo {
	PROGINFO	*pip ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	cchar		*dumpfname ;
	HDBSTR		vars ;
	vecstr		queries ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_qkey(LOCINFO *,cchar *,int) ;
static int	locinfo_addvar(LOCINFO *,cchar *,int) ;
static int	locinfo_finish(LOCINFO *) ;

#ifdef	COMMENT
static int	locinfo_varcount(LOCINFO *) ;
#endif

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,
			cchar *,cchar *,cchar *,cchar *) ;
static int	procfiles(PROGINFO *,PARAMOPT *) ;
static int	procfile(PROGINFO *,cchar *,int) ;
static int	procsysdefs(PROGINFO *) ;
static int	procsysdef(PROGINFO *,cchar *) ;
static int	procvarfile(PROGINFO *,cchar *,int) ;
static int	procset(PROGINFO *,cchar *) ;
static int	procseter(PROGINFO *,cchar *,gid_t) ;
static int	process(PROGINFO *,cchar *,void *,cchar *) ;
static int	procgetfile(PROGINFO *,void *,VAR *,cchar *) ;
static int	procqkey(PROGINFO *,void *,VAR *,cchar *,int) ;
static int	procdumpfile(PROGINFO *,VAR *,cchar *) ;
static int	procoutall(PROGINFO *,VAR *,void *) ;

static int	hasweird(cchar *,int) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"LOGFILE",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"gf",
	"db",
	"df",
	"dump",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_logfile,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_gf,
	argopt_db,
	argopt_df,
	argopt_dump,
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
	"set",
	"list",
	"dump",
	"audit",
	"star",
	NULL
} ;

enum akonames {
	akoname_set,
	akoname_list,
	akoname_dump,
	akoname_audit,
	akoname_star,
	akoname_overlast
} ;

static const uchar	qterms[] = {
	0x00, 0x3E, 0x00, 0x00,
	0x09, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const uchar	fterms[32] = {
	0x00, 0x3A, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x20,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	*wstrs[] = {
	"TZ",
	"LANG",
	"UMASK",
	"PATH",
	NULL
} ;

static const char	*pstrs[] = {
	"LC_",
	NULL
} ;


/* exported subroutines */


int b_sysvar(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_sysvar) */


int p_sysvar(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_sysvar) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	SHIO		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		n ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	char		tmpdbfname[MAXPATHLEN + 1] ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*gfname = NULL ;
	cchar		*dbname = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_sysvar: starting DFD=%d\n",rs) ;
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
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,0) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

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

/* output name */
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

	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            cp = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                cp = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* get-file */
	                case argopt_gf:
	                    lip->f.query = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            gfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                gfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* DB name */
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            dbname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                dbname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* dump file */
	                case argopt_df:
	                case argopt_dump:
	                    lip->have.dump = TRUE ;
	                    lip->final.dump = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lip->dumpfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->dumpfname = argp ;
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

/* specify a "set" file */
	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                PARAMOPT	*pop = &aparams ;
	                                cchar		*po = PO_FILENAME ;
	                                rs = paramopt_loads(pop,po,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* get a variable by name */
	                    case 'g':
	                    case 'n':
	                        lip->f.query = TRUE ;
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                rs = locinfo_qkey(lip,argp,argl) ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* list mode */
	                    case 'l':
	                    case 'a':
	                        lip->have.list = TRUE ;
	                        lip->final.list = TRUE ;
	                        lip->f.list = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.list = (rs > 0) ;
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

/* set mode */
	                    case 's':
	                        lip->have.set = TRUE ;
	                        lip->final.set = TRUE ;
	                        lip->f.set = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.set = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* unique mode */
	                    case 'u':
	                        lip->have.uniq = TRUE ;
	                        lip->final.uniq = TRUE ;
	                        lip->f.uniq = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.uniq = (rs > 0) ;
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

	if (rs < 0) goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_sysvar: debuglevel=%u\n",pip->debuglevel) ;
	    debugprintf("b_sysvar: version=%s\n",VERSION) ;
	}
#endif

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

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (ofname == NULL) ofname = getourenv(envv,VAROFNAME) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

	if (dbname == NULL) dbname = getourenv(envv,VARDBNAME) ;
	if (dbname == NULL) dbname = DBNAME ;

	if ((rs >= 0) && (strchr(dbname,'/') == NULL)) {
	    rs = mkpath2(tmpdbfname,DBDNAME,dbname) ;
	    dbname = tmpdbfname ;
	}

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: dbname=%s\n",pip->progname,dbname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_sysvar: f_flist=%u f_list=%u f_set=%u f_query=%u\n",
	        lip->final.list,lip->f.list,lip->f.set,lip->f.query) ;
#endif

/* continue */

	        memset(&ainfo,0,sizeof(ARGINFO)) ;
	        ainfo.argc = argc ;
	        ainfo.ai = ai ;
	        ainfo.argv = argv ;
	        ainfo.ai_max = ai_max ;
	        ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	if ((rs = ids_load(&pip->id)) >= 0) {
	    int	f = FALSE ;

/* process all set-files */

	    rs = procfiles(pip,&aparams) ;
	    n = rs ;

/* load the system default variables if we are in "set" mode */

	    if ((rs >= 0) && ((n > 0) || lip->f.set)) {

	        lip->f.set = TRUE ;
	        if ((rs = procsysdefs(pip)) >= 0) {
	            rs = procset(pip,dbname) ;
		}

	        if (pip->debuglevel > 0) {
		    cchar	*pn = pip->progname ;
		    cchar	*fmt ;
	            if (rs >= 0) {
			fmt = "%s: loaded variables=%u\n" ;
	                shio_printf(pip->efp,fmt,pn,rs) ;
	            } else {
			fmt = "%s: load failure (%d)\n" ;
	                shio_printf(pip->efp,fmt,pn,rs) ;
		    }
	        }

	    } /* end if (set mode) */

	    f = (lip->f.list || lip->f.query || lip->f.audit) ;
	    if ((rs >= 0) && ((! lip->f.set) || f)) {
	        if (rs >= 0) {
	            cchar	*ofn = ofname ;
	            cchar	*dfn = dbname ;
	            cchar	*afn = afname ;
	            cchar	*gfn = gfname ;
	            rs = procargs(pip,&ainfo,&pargs,ofn,dfn,afn,gfn) ;
	        }
	    } /* end if */

	    ids_release(&pip->id) ;
	} /* end if (ids) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	    if (! pip->f.quiet) {
	        shio_printf(pip->efp,
	            "%s: could not perform function (%d)\n",
	            pip->progname,rs) ;
	    }
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

/* early return thing */
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
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final mallout=%u\n",mdiff) ;
	    if (mdiff > 0) {
	        UCMALLREG_CUR	cur ;
	        UCMALLREG_REG	reg ;
	        const int	size = (10*sizeof(uint)) ;
	        cchar		*ids = "main" ;
	        uc_mallinfo(mi,size) ;
	        debugprintf("main: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("main: MIoutnummax=%u\n",mi[ucmallreg_outnummax]) ;
	        debugprintf("main: MIoutsize=%u\n",mi[ucmallreg_outsize]) ;
	        debugprintf("main: MIoutsizemax=%u\n",
	            mi[ucmallreg_outsizemax]) ;
	        debugprintf("main: MIused=%u\n",mi[ucmallreg_used]) ;
	        debugprintf("main: MIusedmax=%u\n",mi[ucmallreg_usedmax]) ;
	        debugprintf("main: MIunder=%u\n",mi[ucmallreg_under]) ;
	        debugprintf("main: MIover=%u\n",mi[ucmallreg_over]) ;
	        debugprintf("main: MInotalloc=%u\n",mi[ucmallreg_notalloc]) ;
	        debugprintf("main: MInotfree=%u\n",mi[ucmallreg_notfree]) ;
	        ucmallreg_curbegin(&cur) ;
	        while (ucmallreg_enum(&cur,&reg) >= 0) {
	            debugprintf("main: MIreg.addr=%p\n",reg.addr) ;
	            debugprintf("main: MIreg.size=%u\n",reg.size) ;
	            debugprinthexblock(ids,80,reg.addr,reg.size) ;
	        }
	        ucmallreg_curend(&cur) ;
	    }
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
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

	fmt = "%s: USAGE> %s [-s] [-f <file(s)>[,<...>]] [<var(s)>|-a]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
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

	                case akoname_set:
	                    if (! lip->final.set) {
	                        lip->have.set = TRUE ;
	                        lip->final.set = TRUE ;
	                        lip->f.set = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.set = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_list:
	                    if (! lip->final.list) {
	                        lip->have.list = TRUE ;
	                        lip->final.list = TRUE ;
	                        lip->f.list = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.list = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_dump:
	                    if (! lip->final.dump) {
	                        lip->have.dump = TRUE ;
	                        lip->final.dump = TRUE ;
	                        if (vl > 0)
	                            lip->dumpfname = vp ;
	                    }
	                    break ;

	                case akoname_audit:
	                    if (! lip->final.audit) {
	                        lip->have.audit = TRUE ;
	                        lip->final.audit = TRUE ;
	                        lip->f.audit = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.audit = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_star:
	                    if (! lip->final.star) {
	                        lip->have.star = TRUE ;
	                        lip->final.star = TRUE ;
	                        lip->f.star = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.star = (rs > 0) ;
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


static int procargs(pip,aip,bop,ofn,dbn,afn,gfn)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
cchar		*ofn ;
cchar		*dbn ;
cchar		*afn ;
cchar		*gfn ;
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    LOCINFO	*lip = pip->lip ;
	    int		pan = 0 ;
	    int		cl ;
	    cchar	*cp ;

	    if (rs >= 0) {
	        int	ai ;
	        int	f ;
	        for (ai = 1 ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
	                if (cp[0] != '\0') {
	                    pan += 1 ;
	                    lip->f.query = TRUE ;
	                    rs = locinfo_qkey(lip,cp,-1) ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        SHIO	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0)
	            afn = STDINFNAME ;

	        if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        lip->f.query = TRUE ;
	                        rs = locinfo_qkey(lip,cp,cl) ;
	                    }
	                }

	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = shio_close(afp) ;
	            if (rs >= 0) rs = rs1 ;
	        } else {
		    fmt = "%s: inaccessible argument-list (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	            shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    if (rs >= 0) {
	        rs = process(pip,dbn,ofp,gfn) ;
	        c += rs ;
	    }

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


static int procfiles(PROGINFO *pip,PARAMOPT *app)
{
	PARAMOPT_CUR	cur ;
	int		rs ;
	int		c = 0 ;
	cchar		*po_name = PO_FILENAME ;

	if ((rs = paramopt_curbegin(app,&cur)) >= 0) {
	    int		nl ;
	    cchar	*np ;

	    while (rs >= 0) {
	        nl = paramopt_fetch(app,po_name,&cur,&np) ;
	        if (nl < 0) break ;
	        if (nl == 0) continue ;
	        rs = nl ;

	        if (rs >= 0) {
	            rs = procfile(pip,np,nl) ;
	            c += rs ;
	        }

	        if (rs >= 0) rs = lib_sigterm() ;
	        if (rs >= 0) rs = lib_sigintr() ;
	    } /* end while */

	    paramopt_curend(app,&cur) ;
	} /* end if (paramopt-cur) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procfiles) */


static int procfile(PROGINFO *pip,cchar fnp[],int fnl)
{
	int		rs ;
	int		c = 0 ;

	if (fnp == NULL) return SR_FAULT ;

	rs = procvarfile(pip,fnp,fnl) ;
	c = rs ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procfile) */


static int procsysdefs(PROGINFO *pip)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsysdefs: ent\n") ;
#endif

	if (rs >= 0)
	    rs = procsysdef(pip,DEFINITFNAME) ;

	if (rs >= 0)
	    rs = procsysdef(pip,DEFLOGFNAME) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsysdefs: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procsysdefs) */


static int procsysdef(PROGINFO *pip,cchar fname[])
{
	LOCINFO	*lip = pip->lip ;
	struct ustat	usb ;
	struct ustat	*sbp ;
	vecstr		lvars ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;
	int		f ;
	cchar		*tp, *cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsysdef: fname=%s\n",fname) ;
#endif

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	sbp = &usb ;
	rs1 = u_stat(fname,sbp) ;
	if (rs1 >= 0)
	    rs1 = sperm(&pip->id,sbp,R_OK) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsysdef: ACCESS rs1=%d\n",rs1) ;
#endif

	if (rs1 >= 0) {
	    if ((rs = vecstr_start(&lvars,10,0)) >= 0) {

	        if ((rs = vecstr_envfile(&lvars,fname)) >= 0) {

	            for (i = 0 ; vecstr_get(&lvars,i,&cp) >= 0 ; i += 1) {
	                if (cp == NULL) continue ;

	                if ((tp = strchr(cp,'=')) == NULL) continue ;

	                f = (matstr(wstrs,cp,(tp - cp)) >= 0) ;
	                f = f || (matpstr(pstrs,10,cp,(tp - cp)) >= 0) ;
	                if (f) {
	                    c += 1 ;
	                    rs = locinfo_addvar(lip,cp,-1) ;
	                    if (rs < 0) break ;
	                } /* end if */

	            } /* end for */

	        } /* end if (vecstr_envfile) */

	        vecstr_finish(&lvars) ;
	    } /* end if (lvars) */
	} else {
	    if (! pip->f.quiet) {
		cchar	*pn = pip->progname ;
		cchar	*fmt = "%s: inaccessible fname=%s (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,fname,rs1) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsysdef: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procsysdef) */


static int process(PROGINFO *pip,cchar dbname[],void *ofp,cchar gfname[])
{
	LOCINFO		*lip = pip->lip ;
	VAR		sv ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = var_open(&sv,dbname)) >= 0) {

	    if (lip->f.audit) {

	        rs = var_audit(&sv) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("b_sysvar/process: var_audit() rs=%d\n",rs) ;
#endif

	        if (pip->debuglevel > 0) {
	            shio_printf(pip->efp,"%s: DB audit (%d)\n",
	                pip->progname,rs) ;
		}

	    } /* end if */

	    if ((rs >= 0) && lip->f.list) {

	        rs = procoutall(pip,&sv,ofp) ;

	    } /* end if (list mode) */

	    if ((rs >= 0) && (lip->dumpfname != NULL)) {

	        rs = procdumpfile(pip,&sv,lip->dumpfname) ;

	    } /* end if (dump-file mode) */

	    if ((rs >= 0) && lip->f.query) {
	        VECSTR	*qlp = &lip->queries ;
	        int	i ;
	        cchar	*kp ;

	        for (i = 0 ; vecstr_get(qlp,i,&kp) >= 0 ; i += 1) {
	            if (kp != NULL) {
	                c += 1 ;
	                rs = procqkey(pip,ofp,&sv,kp,-1) ;
		    }
	            if (rs < 0) break ;
	        } /* end for */

	        if ((rs >= 0) && (gfname != NULL)) {
	            rs = procgetfile(pip,ofp,&sv,gfname) ;
	        }

	    } /* end if (query mode) */

	    rs1 = var_close(&sv) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (var-open) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysvar/process: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (process) */


static int procdumpfile(PROGINFO *pip,VAR *svp,cchar *dumpfname)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if (dumpfname == NULL) return SR_FAULT ;

	if (dumpfname[0] != '\0') {
	    SHIO	dfile, *dfp = &dfile ;
	    if (dumpfname[0] == '-') dumpfname = STDOUTFNAME ;
	    if ((rs = shio_open(dfp,dumpfname,"wct",0666)) >= 0) {
	        rs = procoutall(pip,svp,dfp) ;
	        c += rs ;
	        rs1 = shio_close(dfp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (dumpfile) */
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdumpfile) */


/* output all variables (sorted by key-name) */
static int procoutall(PROGINFO *pip,VAR *svp,void *ofp)
{
	vecstr		keys, *klp = &keys ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = vecstr_start(klp,0,0)) >= 0) {
	    VAR_CUR	cur ;
	    const int	klen = KBUFLEN ;
	    const int	vlen = VBUFLEN ;
	    int		kl ;
	    int		vl ;
	    char	kbuf[KBUFLEN + 1] ;
	    char	vbuf[VBUFLEN + 1] ;

	    if ((rs = var_curbegin(svp,&cur)) >= 0) {

	        while (rs >= 0) {
	            vl = var_enum(svp,&cur,kbuf,klen,NULL,vlen) ;
	            if (vl == SR_NOTFOUND) break ;
	            rs = vl ;

	            if (rs >= 0) {
	                kl = sichr(kbuf,-1,'=') ;
	                rs = vecstr_adduniq(klp,kbuf,kl) ;
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	        } /* end while */

	        var_curend(svp,&cur) ;
	    } /* end if (cursor) */

	    if (rs >= 0)
	        rs = vecstr_sort(klp,NULL) ;

	    if (rs >= 0) {
	        cchar	*kp ;
	        int	i ;
	        for (i = 0 ; vecstr_get(klp,i,&kp) >= 0 ; i += 1) {
	            if (kp == NULL) continue ;

	            if ((rs = var_curbegin(svp,&cur)) >= 0) {

	                while (rs >= 0) {

	                    vl = var_fetch(svp, kp,-1, &cur, vbuf,vlen) ;
	                    if (vl == SR_NOTFOUND) break ;
	                    rs = vl ;

	                    if (rs >= 0) {
	                        c += 1 ;
	                        rs = shio_printf(ofp,"%s=%t\n",kp,vbuf,vl) ;
	                    }

	                    if (rs >= 0) rs = lib_sigterm() ;
	                    if (rs >= 0) rs = lib_sigintr() ;
	                } /* end while */

	                var_curend(svp,&cur) ;
	            } /* end if (cursor) */

	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

	    rs1 = vecstr_finish(klp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: %u variables\n",pip->progname,c) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procoutall) */


static int procgetfile(PROGINFO *pip,void *ofp,VAR *vfp,cchar gfname[])
{
	FIELD		fsb ;
	SHIO		getfile, *gfp = &getfile ;
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		len ;
	int		cl, kl ;
	int		n = 0 ;
	cchar		*kp ;
	cchar		*cp ;
	char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysvar/procgetfile: gfname=%s\n",gfname) ;
#endif

	if (gfname == NULL)
	    goto ret0 ;

	if (gfname[0] == '\0')
	    goto ret0 ;

	if (strcmp(gfname,"-") == 0)
	    gfname = STDINFNAME ;

	if ((rs = shio_open(gfp,gfname,"r",0666)) >= 0) {

	    while ((rs = shio_readline(gfp,lbuf,llen)) > 0) {
	        len = rs ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;

	        cp = lbuf ;
	        cl = len ;

	        if ((rs = field_start(&fsb,cp,cl)) >= 0) {
	            while ((kl = field_get(&fsb,qterms,&kp)) >= 0) {
	                if (kl > 0) {
	                    rs = procqkey(pip,ofp,vfp,kp,kl) ;
	                    n += rs ;
	                }
	                if (fsb.term == '#') break ;
	                if (rs < 0) break ;
	            } /* end while (reading lines) */
	            field_finish(&fsb) ;
	        } /* end if (field) */

	        if (rs < 0) break ;
	    } /* end while */

	    rs1 = shio_close(gfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (shio) */

ret0:
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procgetfile) */


static int procqkey(PROGINFO *pip,void *ofp,VAR *vfp,cchar *kp,int kl)
{
	LOCINFO		*lip = pip->lip ;
	VAR_CUR		cur ;
	const int	vlen = VBUFLEN ;
	int		rs = SR_OK ;
	int		vl ;
	int		n = 0 ;
	char		vbuf[VBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysvar/procqkey: k=>%t<\n",
	        kp,strlinelen(kp,kl,40)) ;
#endif

	if ((rs = var_curbegin(vfp,&cur)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_sysvar/procqkey: while-before\n") ;
#endif

	    while (rs >= 0) {
	        vl = var_fetch(vfp,kp,kl,&cur,vbuf,vlen) ;
	        if (vl == SR_NOTFOUND) break ;
	        rs = vl ;

	        if ((rs >= 0) && (n++ > 0))
	            rs = shio_printf(ofp," ") ;

	        if ((rs >= 0) && (vl >= 0))
	            rs = shio_write(ofp,vbuf,vl) ;

	        if (rs >= 0) rs = lib_sigterm() ;
	        if (rs >= 0) rs = lib_sigintr() ;
	    } /* end while */

	    var_curend(vfp,&cur) ;
	} /* end if (cursor) */

	if (rs >= 0) {

	    if ((n == 0) && (pip->debuglevel > 0))
	        shio_printf(pip->efp,"%s: notfound=%s\n",
	            pip->progname,kp) ;

	    if (lip->f.star && (n == 0))
	        rs = shio_printf(ofp,"*") ;

	    if (rs >= 0)
	        rs = shio_printf(ofp,"\n") ;

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysvar/procqkey: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procqkey) */


static int procset(PROGINFO *pip,cchar dbname[])
{
	struct ustat	sb ;
	int		rs ;
	if (dbname == NULL) return SR_FAULT ;
	if ((rs = u_stat(pip->pr,&sb)) >= 0) {
	    const gid_t	gid = sb.st_gid ;
	    rs = procseter(pip,dbname,gid) ;
	} /* end if (stat) */
	return rs ;
}
/* end subroutine (procset) */


static int procseter(PROGINFO *pip,cchar dbname[],gid_t gid)
{
	LOCINFO		*lip = pip->lip ;
	HDBSTR_CUR	cur ;
	VARMK		svars ;
	const mode_t	om = 0664 ;
	const int	n = DEFVARS ;
	const int	of = 0 ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = varmk_open(&svars,dbname,of,om,n)) >= 0) {
	    if (rs == 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_sysvar/procset: varmk_open() rs=%d\n",rs) ;
#endif

	    if ((rs = varmk_chgrp(&svars,gid)) >= 0) {
	        int	kl ;
	        int	vl = 0 ;
	        cchar	*kp, *vp ;
	        if ((rs = hdbstr_curbegin(&lip->vars,&cur)) >= 0) {

	            while (rs >= 0) {
	                kl = hdbstr_enum(&lip->vars,&cur,&kp,&vp,&vl) ;
	                if (kl == SR_NOTFOUND) break ;
	                rs = kl ;

#if	CF_DEBUG
	                debugprintf("b_sysvar/procset: k=%s v=>%t<\n",
	                    kp,vp,vl) ;
#endif

	                if (rs >= 0) {
	                    c += 1 ;
	                    rs = varmk_addvar(&svars,kp,vp,vl) ;
	                }

	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	            } /* end while */

	            hdbstr_curend(&lip->vars,&cur) ;
	        } /* end if (cursor) */
	    } /* end if (chgrp) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_sysvar/procset: done rs=%d\n",rs) ;
#endif

/* done */

	    if (rs < 0)
	        varmk_abort(&svars) ;

	    } /* end if (needed) */
	    rs1 = varmk_close(&svars) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (varmk) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysvar/procset: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procseter) */


static int procvarfile(PROGINFO *pip,cchar *fnp,int fnl)
{
	LOCINFO		*lip = pip->lip ;
	HDBSTR		*varp ;
	FIELD		fsb ;
	FILEBUF		dfile, *dfp = &dfile ;
	const int	llen = LINEBUFLEN ;
	const int	vlen = VBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;
	int		len, cl ;
	int		kl, vl ;
	int		to = TO_READ ;
	int		c = 0 ;
	cchar		*kp ;
	cchar		*vp ;
	cchar		*cp ;
	char		fname[MAXPATHLEN + 1] ;
	char		lbuf[LINEBUFLEN + 1] ;
	char		vbuf[VBUFLEN + 1] ;

	if (pip == NULL) return SR_FAULT ;
	if (fnp == NULL) return SR_FAULT ;

	if ((fnl < 0) || (fnp[fnl] != '\0')) {
	    mkpath1w(fname,fnp,fnl) ;
	    fnp = fname ;
	}

	varp = &lip->vars ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procvarfile: fname=%s\n",fnp) ;
#endif

	if ((rs = u_open(fnp,O_RDONLY,0666)) >= 0) {
	    int	fd = rs ;

	    if ((rs = filebuf_start(dfp,fd,0L,BUFLEN,0)) >= 0) {

	        while ((rs = filebuf_readline(dfp,lbuf,llen,to)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            cp = lbuf ;
	            cl = len ;
	            while ((cl > 0) && CHAR_ISWHITE(*cp)) {
	                cp += 1 ;
	                cl -= 1 ;
	            }

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            if ((rs = field_start(&fsb,cp,cl)) >= 0) {
	                cchar	*we = WORDEXPORT ;

	                kl = field_get(&fsb,fterms,&kp) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("procvarfile: k=>%t<\n",kp,kl) ;
#endif

	                if ((kl == 6) && (strncasecmp(we,kp,kl) == 0)) {

	                    kl = field_get(&fsb,fterms,&kp) ;

	                } /* end if (elimination of 'export') */

	                if ((kl > 0) && (! hasweird(kp,kl))) {

	                    rs1 = SR_OK ;
	                    if (lip->f.uniq)
	                        rs1 = hdbstr_fetch(varp,kp,kl,NULL,NULL) ;

	                    if ((rs1 == SR_NOTFOUND) || (! lip->f.uniq)) {

	                        vp = vbuf ;
	                        vl = 0 ;
	                        if (fsb.term != '#') {
	                            rs1 = field_sharg(&fsb,fterms,vbuf,vlen) ;
	                            if (rs1 >= 0)
	                                vl = rs1 ;
	                        }

#if	CF_DEBUG
	                        if (DEBUGLEVEL(3))
	                            debugprintf("procvarfile: v=>%t<\n",
	                                vp,vl) ;
#endif

	                        c += 1 ;
	                        rs = hdbstr_add(varp,kp,kl,vp,vl) ;

	                    } /* end if (didn't have it already) */

	                } /* end if (have a variable keyname) */

	                field_finish(&fsb) ;
	            } /* end if (fields) */

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        filebuf_finish(dfp) ;
	    } /* end if (filebuf) */

	    u_close(fd) ;
	} /* end if (file-open) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procvarfile) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;

	if ((rs = hdbstr_start(&lip->vars,DEFNVARS)) >= 0) {
	    if ((rs = vecstr_start(&lip->queries,10,0)) >= 0) {
	        lip->f.uniq = TRUE ;
	    }
	    if (rs < 0)
	        hdbstr_finish(&lip->vars) ;
	} /* end if (hdbstr-start) */

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecstr_finish(&lip->queries) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = hdbstr_finish(&lip->vars) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_finish) */


#ifdef	COMMENT
static int locinfo_varcount(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = hdbstr_count(&lip->vars) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_finish) */
#endif /* COMMENT */


static int locinfo_qkey(LOCINFO *lip,cchar *qp,int ql)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

	if ((rs = field_start(&fsb,qp,ql)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,qterms,&fp)) > 0) {
	        if (fl > 0) {
	            c += 1 ;
	            rs = vecstr_add(&lip->queries,fp,fl) ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_qkey) */


static int locinfo_addvar(LOCINFO *lip,cchar *cp,int cl)
{
	HDBSTR		*varp = &lip->vars ;
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;
	int		kl, vl ;
	int		c = 0 ;
	cchar		*tp ;
	cchar		*kp, *vp ;

	kp = cp ;
	kl = cl ;
	vp = NULL ;
	vl = 0 ;
	if ((tp = strnchr(cp,cl,'=')) != NULL) {
	    vp = (tp + 1) ;
	    vl = -1 ;
	    kl = (tp - cp) ;
	}

	rs1 = SR_NOTFOUND ;
	if (lip->f.uniq)
	    rs1 = hdbstr_fetch(varp,kp,kl,NULL,NULL) ;

	if ((rs1 == SR_NOTFOUND) || (! lip->f.uniq)) {
	    c += 1 ;
	    rs = hdbstr_add(varp,kp,kl,vp,vl) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_addvar) */


static int hasweird(cchar *sp,int sl)
{
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; (i != sl) && (sp[i] != '\0') ; i += 1) {
	    const int	ch = MKCHAR(sp[i]) ;
	    f = ((! isalnumlatin(ch)) && (ch != '_')) ;
	    if (f) break ;
	} /* end if */

	return f ;
}
/* end subroutine (hasweird) */


