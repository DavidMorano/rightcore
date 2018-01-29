/* b_isfile */

/* SHELL built-in: determine if a file has certain attributes */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is sort of a replacement for the 'test(1)' program (or the various
	SHELL built-in versions).  Except that this version does not
	discriminate against a file if it is a symbolic link and the link is
	dangling.

	Synopsis:

	$ isfile [<file(s)>] [-<intage>] [<opt(s)>]


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
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<vecobj.h>
#include	<ids.h>
#include	<getax.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_isfile.h"
#include	"defs.h"


/* local defines */

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags
#define	LOCINFO_FTS	struct locinfo_ftypes

#define	PO_TYPE		"type"
#define	PO_SUFFIX	"suffix"


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	getgid_group(cchar *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	fileobject(const char *) ;
extern int	filebinary(const char *) ;
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
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		ftypes:1 ;
	uint		id:1 ;
	uint		intage:1 ;
	uint		same:1 ;
	uint		group:1 ;
	uint		zero:1 ;
} ;

struct locinfo_ftypes {
	uint		e:1 ;		/* file actually exists */
	uint		l:1 ;		/* file symbolic link */
	uint		f:1 ;		/* file regular */
	uint		d:1 ;		/* file directory */
	uint		b:1 ;		/* file block */
	uint		c:1 ;		/* file character */
	uint		p:1 ;		/* file pipe or FIFO */
	uint		s:1 ;		/* file socket */
	uint		D:1 ;		/* file door */
	uint		r:1 ;		/* file readable */
	uint		w:1 ;		/* file writable */
	uint		x:1 ;		/* file executable */
	uint		obj:1 ;		/* file is an object file */
	uint		bin:1 ;		/* file has binary data */
	uint		group:1 ;	/* file has group <group> */
	uint		sgid:1 ;	/* file is SGID */
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	LOCINFO_FTS	ft ;
	PROGINFO	*pip ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	dev_t		same_d ;
	uino_t		same_i ;
	IDS		id ;
	gid_t		gid_tar ;
	const char	*group_tar ;
	int		nsame ;
	int		intage ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_procopts(LOCINFO *) ;
static int	locinfo_ftypes(LOCINFO *) ;
static int	locinfo_ids(LOCINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_getgroup(LOCINFO *) ;

static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procfile(PROGINFO *,const char *) ;

static int	isNeedStat(PROGINFO *) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"obj",
	"bin",
	"same",
	"group",
	"sgid",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_obj,
	argopt_bin,
	argopt_same,
	argopt_group,
	argopt_sgid,
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
	{ SR_NOENT, EX_NOINPUT },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_EXIT, EX_TERM },
	{ SR_INTR, EX_INTR },
	{ 0, 0 }
} ;

static const char	*akonames[] = {
	"quiet",
	"intage",
	"same",
	"zero",
	NULL
} ;

enum akonames {
	akoname_quiet,
	akoname_intage,
	akoname_same,
	akoname_zero,
	akoname_overlast
} ;

static const char	*ftypes[] = {
	"file",
	"link",
	"exists",
	"directory",
	"block",
	"character",
	"pipe",
	"fifo",
	"socket",
	"door",
	"read",
	"write",
	"xecute",
	"regular",
	NULL
} ;

enum ftypes {
	ftype_file,
	ftype_link,
	ftype_exists,
	ftype_directory,
	ftype_block,
	ftype_character,
	ftype_pipe,
	ftype_fifo,
	ftype_socket,
	ftype_door,
	ftype_read,
	ftype_write,
	ftype_execute,
	ftype_regular,
	ftype_overlast
} ;


/* exported subroutines */


int b_isfile(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    const char	**envv = (const char **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_isfile) */


int p_isfile(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_isfile) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	SHIO		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		argvalue = -1 ;
	int		rs, rs1 ;
	int		maxage = -1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_exit = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*efname = NULL ;
	const char	*intagespec = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_isfile: starting DFD=%d\n",rs) ;
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
	pip->daytime = time(NULL) ;

	pip->lip = &li ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

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

	                case argopt_obj:
	                    lip->ft.obj = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->ft.obj = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_bin:
	                    lip->ft.bin = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->ft.bin = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_same:
	                    lip->final.same = TRUE ;
	                    lip->have.same = TRUE ;
	                    lip->f.same = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.same = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_sgid:
	                    lip->ft.sgid = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->ft.sgid = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_group:
	                    lip->ft.group = TRUE ;
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            lip->ft.group = TRUE ;
	                            lip->group_tar = argp ;
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

	                    case 'i':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                intagespec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'e':
	                        lip->ft.e = TRUE ;
	                        break ;

	                    case 'l':
	                        lip->ft.l = TRUE ;
	                        break ;

	                    case 'f':
	                        lip->ft.f = TRUE ;
	                        break ;

	                    case 'd':
	                        lip->ft.d = TRUE ;
	                        break ;

	                    case 'b':
	                        lip->ft.b = TRUE ;
	                        break ;

	                    case 'c':
	                        lip->ft.c = TRUE ;
	                        break ;

	                    case 'p':
	                        lip->ft.p = TRUE ;
	                        break ;

	                    case 's':
	                        lip->ft.s = TRUE ;
	                        break ;

	                    case 'r':
	                        lip->ft.r = TRUE ;
	                        break ;

	                    case 'w':
	                        lip->ft.w = TRUE ;
	                        break ;

	                    case 'x':
	                        lip->ft.x = TRUE ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					KEYOPT	*kop = &lip->akopts ;
	                                rs = keyopt_loads(kop,argp,argl) ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* file types */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                PARAMOPT	*pop = &lip->aparams ;
	                                const char	*po = PO_TYPE ;
	                                rs = paramopt_loads(pop,po,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
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

/* allow zero arguments */
	                    case 'z':
	                        lip->final.zero = TRUE ;
	                        lip->have.zero = TRUE ;
	                        lip->f.zero = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.zero = (rs > 0) ;
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
	    debugprintf("b_isfile: debuglevel=%u\n",pip->debuglevel) ;
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

/* process the program options */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if ((rs = locinfo_procopts(lip)) >= 0) {
	    rs = locinfo_ftypes(lip) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_isfile: f_s=%u\n",lip->ft.s) ;
	    debugprintf("b_isfile: f_r=%u\n",lip->ft.r) ;
	    debugprintf("b_isfile: f_w=%u\n",lip->ft.w) ;
	    debugprintf("b_isfile: f_x=%u\n",lip->ft.x) ;
	}
#endif /* CF_DEBUG */

/* check up on the arguments */

	if ((rs >= 0) && (argval != NULL)) {
	    rs = cfdecti(argval,-1,&argvalue) ;
	}

	if ((rs >= 0) && (intagespec != NULL) && (intagespec[0] != '\0')) {
	    rs = cfdecti(intagespec,-1,&maxage) ;
	}

	if ((maxage < 0) && (argvalue > 0)) {
	    maxage = argvalue ;
	}

	if (rs < 0) goto badarg ;

	lip->intage = maxage ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("b_isfile: daytime=%s\n",
	        timestr_logz(pip->daytime,timebuf)) ;
	    debugprintf("b_isfile: intage=%d\n",
	        lip->intage) ;
	}
#endif /* CF_DEBUG */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    const char	*ofn = ofname ;
	    const char	*afn = afname ;
	    rs = procargs(pip,&ainfo,&pargs,ofn,afn) ;
	    f_exit = rs ;
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("b_isfile: f_exit=%u\n",f_exit) ;
	    debugprintf("b_isfile: f0 rs=%d ex=%u\n",rs,ex) ;
	}
#endif

	if ((rs >= 0) && (ex == EX_OK) && f_exit) ex = 1 ;

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    const char	*fmt = "%s: f_exit=%u\n" ;
	    shio_printf(pip->efp,fmt,pip->progname,f_exit) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_isfile: f1 rs=%d ex=%u\n",rs,ex) ;
#endif

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_isfile: exiting ex=%u (%d)\n",ex,rs) ;
#endif

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

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_isfile: final mallout=%u\n",(mo-mo_start)) ;
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

	fmt = "%s: USAGE> %s <file(s)> [<opt(s)>] [-<intage>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* ARGSUSED */
static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;
	int		cl ;
	int		f_exit = FALSE ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	const char	*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procargs: ent argc=%u\n",aip->argc) ;
	    debugprintf("main/procargs: ai_max=%u\n",aip->ai_max) ;
	}
#endif

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
	                rs = procfile(pip,cp) ;
	                f_exit = (rs == 0) ;
	            }
	        }

	        if (f_exit) break ;
	        if (rs < 0) break ;
	    } /* end for (handling positional arguments) */
	} /* end if (ok) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    SHIO	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0)
	        afn = STDINFNAME ;

	    if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	                    pan += 1 ;
	                    lbuf[(cp+cl)-lbuf] = '\0' ;
	                    rs = procfile(pip,cp) ;
	                    f_exit = (rs == 0) ;
	                }
	            }

	            if (f_exit) break ;
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

	if ((rs >= 0) && (pan == 0)) {
	    if (! lip->f.zero) {
	        rs = SR_INVALID ;
	        fmt = "%s: no files specified\n" ;
	        shio_printf(pip->efp,fmt,pn) ;
	    }
	}

	return (rs >= 0) ? f_exit : rs ;
}
/* end subroutine (procargs) */


/* process a file (carefully follow the logic in this subroutine) */
static int procfile(PROGINFO *pip,cchar fname[])
{
	struct ustat	usb ;
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_link = FALSE ;
	int		f_needstat = FALSE ;
	int		f_ok = FALSE ;

	if (fname == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b/procfile: fname=%s intage=%d\n",
	        fname,lip->intage) ;
#endif /* CF_DEBUG */

	if ((rs1 = u_lstat(fname,&usb)) >= 0) {
	    f_ok = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        char	timebuf[TIMEBUFLEN + 1] ;
	        debugprintf("b/procfile: mtime=%s\n",
	            timestr_logz(usb.st_mtime,timebuf)) ;
	    }
#endif /* CF_DEBUG */

	    f_link = S_ISLNK(usb.st_mode) ;

	    if (lip->ft.l) f_ok = f_ok && f_link ;

	    f_needstat = isNeedStat(pip) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("b/procfile: f_needstat=%u\n",f_needstat) ;
#endif

	    if (f_link && f_needstat) {
	        f_ok = FALSE ;
	        rs1 = u_stat(fname,&usb) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b/procfile: u_stat() rs=%d\n",rs1) ;
#endif
	        if (rs1 >= 0) f_ok = TRUE ;
	    }

	    if ((rs1 >= 0) && f_ok) {
	        if (lip->ft.f) f_ok = f_ok && S_ISREG(usb.st_mode) ;
	        if (lip->ft.d) f_ok = f_ok && S_ISDIR(usb.st_mode) ;
	        if (lip->ft.b) f_ok = f_ok && S_ISBLK(usb.st_mode) ;
	        if (lip->ft.c) f_ok = f_ok && S_ISCHR(usb.st_mode) ;
	        if (lip->ft.p) f_ok = f_ok && S_ISFIFO(usb.st_mode) ;
	        if (lip->ft.s) f_ok = f_ok && S_ISSOCK(usb.st_mode) ;
	        if (lip->ft.D) f_ok = f_ok && S_ISDOOR(usb.st_mode) ;
	        if (lip->ft.sgid) {
	            f_ok = f_ok && (usb.st_mode & S_ISGID) ;
	        }
	    }

	    if ((rs1 >= 0) && f_ok) {
	        if (lip->ft.r || lip->ft.w || lip->ft.x) {
	            f_ok = FALSE ;
	            if ((rs = locinfo_ids(lip)) >= 0) {
	                int	am = 0 ;
	                if (lip->ft.r) am |= R_OK ;
	                if (lip->ft.w) am |= W_OK ;
	                if (lip->ft.x) am |= X_OK ;
	                rs1 = sperm(&lip->id,&usb,am) ;
	                if (rs1 >= 0) f_ok = TRUE ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("b_isfile/procfile: sperm() rs=%d\n",
	                        rs1) ;
#endif

	            }
	        }
	    } /* end if (R-W-X) */

	    if ((rs >= 0) && (rs1 >= 0) && f_ok && lip->ft.obj) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b/procfile: is_obj?\n") ;
#endif
	        f_ok = FALSE ;
	        if ((rs = fileobject(fname)) > 0) {
	            f_ok = TRUE ;
	        } else if (isNotPresent(rs))
	            rs = SR_OK ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b/procfile: is_obj=%u\n",f_ok) ;
#endif
	    }

	    if ((rs >= 0) && (rs1 >= 0) && f_ok && lip->ft.bin) {
	        f_ok = FALSE ;
	        if ((rs = filebinary(fname)) > 0) {
	            f_ok = TRUE ;
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	    }

	    if ((rs >= 0) && (rs1 >= 0) && f_ok && lip->ft.group) {
	        f_ok = FALSE ;
	        if ((rs = locinfo_getgroup(lip)) >= 0) {
	            f_ok = (usb.st_gid == lip->gid_tar) ;
	        }
	    }

	    if ((rs >= 0) && (rs1 >= 0) && f_ok && lip->f.same) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b/procfile: nsame=%u\n",lip->nsame) ;
#endif
	        if (lip->nsame++ == 0) {
	            lip->same_i = usb.st_ino ;
	            lip->same_d = usb.st_dev ;
	        } else {
	            f_ok = f_ok && (lip->same_i == usb.st_ino) ;
	            f_ok = f_ok && (lip->same_d == usb.st_dev) ;
	        }
#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b/procfile: f_ok=%u\n",f_ok) ;
#endif
	    }

	    if ((rs >= 0) && (lip->intage > 0)) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b/procfile: intage=%u\n",lip->intage) ;
#endif
	        if (rs1 >= 0) {
	            if (f_ok)
	                f_ok = ((pip->daytime - usb.st_mtime) >= lip->intage) ;
	        } else if (isNotPresent(rs1)) {
	            if (! f_needstat) f_ok = TRUE ;
	        } else {
	            rs = rs1 ;
	        }
	    } /* end if (intage) */

	} else {
	    if (isNotPresent(rs1)) {
	        if (lip->intage > 0) f_ok = TRUE ;
	    } else {
	        rs = rs1 ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b/procfile: ret rs=%d f_ok=%u\n",rs,f_ok) ;
#endif

	return (rs >= 0) ? f_ok : rs ;
}
/* end subroutine (procfile) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->gid_tar = -1 ;

	if ((rs = keyopt_start(&lip->akopts)) >= 0) {
	    pip->open.akopts = TRUE ;
	    if ((rs = paramopt_start(&lip->aparams)) >= 0) {
	        pip->open.aparams = TRUE ;
	    }
	    if (rs < 0) {
	        pip->open.akopts = FALSE ;
	        keyopt_finish(&lip->akopts) ;
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->f.id) {
	    lip->f.id = FALSE ;
	    rs1 = ids_release(&lip->id) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    rs1 = paramopt_finish(&lip->aparams) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    rs1 = keyopt_finish(&lip->akopts) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


/* process the program options */
static int locinfo_procopts(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	KEYOPT		*kop = &lip->akopts ;
	KEYOPT_CUR	kcur ;
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {
	                int	v ;

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {

	                case akoname_quiet:
	                    if (! pip->final.quiet) {
	                        pip->have.quiet = TRUE ;
	                        pip->final.quiet = TRUE ;
	                        pip->f.quiet = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.quiet = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_intage:
	                    if (! lip->final.intage) {
	                        lip->have.intage = TRUE ;
	                        lip->final.intage = TRUE ;
	                        lip->f.intage = TRUE ;
	                        if (vl > 0) {
	                            rs = cfdecti(vp,vl,&v) ;
	                            lip->intage = v ;
	                        }
	                    }
	                    break ;

	                case akoname_same:
	                    if (! lip->final.same) {
	                        lip->have.same = TRUE ;
	                        lip->final.same = TRUE ;
	                        lip->f.same = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.same = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_zero:
	                    if (! lip->final.zero) {
	                        lip->have.zero = TRUE ;
	                        lip->final.zero = TRUE ;
	                        lip->f.zero = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.zero = (rs > 0) ;
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
/* end subroutine (locinfo_procopts) */


static int locinfo_ftypes(LOCINFO *lip)
{
	PARAMOPT_CUR	cur ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = paramopt_curbegin(&lip->aparams,&cur)) >= 0) {
	    int		vl ;
	    int		fti ;
	    const char	*vp ;

	    while (rs >= 0) {

	        vl = paramopt_fetch(&lip->aparams,PO_TYPE,&cur,&vp) ;
	        if (vl == SR_NOTFOUND) break ;
	        if (vl == 0) continue ;
	        rs = vl ;
	        if (rs < 0) break ;

	        if ((fti = matostr(ftypes,1,vp,vl)) >= 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_ismail/locinfo_ftypes: v=%t\n",vp,vl) ;
#endif
	            switch (fti) {
	            case ftype_exists:
	                lip->ft.e = TRUE ;
	                break ;
	            case ftype_link:
	                lip->ft.l = TRUE ;
	                break ;
	            case ftype_regular:
	            case ftype_file:
	                lip->ft.f = TRUE ;
	                break ;
	            case ftype_directory:
	                lip->ft.d = TRUE ;
	                break ;
	            case ftype_block:
	                lip->ft.b = TRUE ;
	                break ;
	            case ftype_character:
	                lip->ft.c = TRUE ;
	                break ;
	            case ftype_pipe:
	            case ftype_fifo:
	                lip->ft.p = TRUE ;
	                break ;
	            case ftype_socket:
	                lip->ft.s = TRUE ;
	                break ;
	            case ftype_door:
	                lip->ft.D = TRUE ;
	                break ;
	            case ftype_read:
	                lip->ft.r = TRUE ;
	                break ;
	            case ftype_write:
	                lip->ft.w = TRUE ;
	                break ;
	            case ftype_execute:
	                lip->ft.x = TRUE ;
	                break ;
	            } /* end switch */
	            c += 1 ;
	        } /* end if */

	    } /* end while */

	    rs1 = paramopt_curend(&lip->aparams,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (paramfile-_cur) */

	if (c > 0) lip->f.ftypes = TRUE ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_ftypes) */


static int locinfo_ids(LOCINFO *lip)
{
	int		rs = SR_OK ;

	if (! lip->f.id) {
	    rs = ids_load(&lip->id) ;
	    lip->f.id = (rs > 0) ;
	}

	return rs ;
}
/* end subroutine (locinfo_ids) */


static int locinfo_getgroup(LOCINFO *lip)
{
	int		rs = SR_OK ;

	if (! lip->have.group) {
	    lip->have.group = TRUE ;
	    if (lip->gid_tar < 0) {
	        cchar	*tg = lip->group_tar ;
	        if ((rs = getgid_group(tg,-1)) >= 0) {
	            lip->gid_tar = rs ;
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	    } /* end if (needed target group-id) */
	} /* end if (needed to get group) */

	return rs ;
}
/* end subroutine (locinfo_getgroup) */


static int isNeedStat(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		f_needstat = FALSE ;

	f_needstat = f_needstat || lip->ft.e ;
	f_needstat = f_needstat || lip->ft.f ;
	f_needstat = f_needstat || lip->ft.d ;
	f_needstat = f_needstat || lip->ft.b ;
	f_needstat = f_needstat || lip->ft.c ;
	f_needstat = f_needstat || lip->ft.p ;
	f_needstat = f_needstat || lip->ft.s ;
	f_needstat = f_needstat || lip->ft.D ;
	f_needstat = f_needstat || lip->ft.r ;
	f_needstat = f_needstat || lip->ft.w ;
	f_needstat = f_needstat || lip->ft.x ;
	f_needstat = f_needstat || lip->ft.sgid ;
	f_needstat = f_needstat || lip->ft.group ;

	return f_needstat ;
}
/* end subroutine (isNeedStat) */


