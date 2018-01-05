/* b_logfile */

/* utility to log messages to a file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  This little program looks
	up a number in a database and returns the corresponding string.

	Synopsis:

	$ logfile <logfile> [-c[=<b>]] [-n <name>[:<version>]] 
		[-s <logsize>] [-if <infile>] [-V]


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
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<linefold.h>
#include	<expcook.h>
#include	<tmtime.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_logfile.h"
#include	"defs.h"


/* local defines */

#ifndef	LOGCOLS
#define	LOGCOLS		(80-16)		/* log-columns */
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 2),2048)
#endif

#ifndef	COMBUFLEN
#define	COMBUFLEN	1024		/* maximum length (?) */
#endif

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	80
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		256
#endif

#ifndef	OUTBUFLEN
#define	OUTBUFLEN	(2*LINEBUFLEN)
#endif

#define	COLBUFLEN	(LOGCOLS + 10)

#ifndef	TO_READ
#define	TO_READ		(5*60)		/* read time-out */
#endif

#ifndef	INTFLUSH
#define	INTFLUSH	4		/* flush interval (4 seconds) */
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	EOFSTR		"*EOF*"


/* external subroutines */

extern int	sntmtime(char *,int,TMTIME *,cchar *) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	matpcasestr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	getnodename(char *,int) ;
extern int	getusername(char *,int,uid_t) ;
extern int	getuid_user(cchar *,int) ;
extern int	mklogid(char *,int,cchar *,int,int) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	isalphalatin(int) ;
extern int	isdigitlatin(int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,cchar *,cchar *) ;
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
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strncasestr(const char *,int,const char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		audit:1 ;
	uint		name:1 ;
	uint		create:1 ;
} ;

struct locinfo {
	PROGINFO	*pip ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	const char	*facname ;
	const char	*facversion ;
	const char	*alloc ;
	int		to_read ;
	char		timebuf[TIMEBUFLEN + 1] ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,EXPCOOK *,const char *) ;
static int	procline(PROGINFO *,const char *,int) ;
static int	procsubs(PROGINFO *,EXPCOOK *,char *,int,cchar *,int) ;
static int	loadcooks(PROGINFO *,EXPCOOK *) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;

static int	mkreport(PROGINFO *,int,cchar **,int) ;
static int	mkreportfile(PROGINFO *,char *,cchar *) ;
static int	mkreportout(PROGINFO *,cchar *,cchar *,int,cchar **,int) ;
static int	mktmpreportdir(char *,cchar *,cchar *,mode_t) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_facility(LOCINFO *,const char *) ;
static int	locinfo_logbegin(LOCINFO *,USERINFO *,const char *) ;
static int	locinfo_logend(LOCINFO *) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"HELP",
	"sn",
	"af",
	"ef",
	"if",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_if,
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
	"audit",
	"create",
	"logsize",
	NULL
} ;

enum akonames {
	akoname_audit,
	akoname_create,
	akoname_logsize,
	akoname_overlast
} ;


/* exported subroutines */


int b_logfile(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_logfile) */


int p_logfile(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_logfile) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	SHIO		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		v ;
	int		to_read = TO_READ ;
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
	const char	*afname = NULL ;
	const char	*ifname = NULL ;
	const char	*efname = NULL ;
	const char	*lfname = NULL ;
	const char	*namespec = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_logfile: starting DFD=%d\n",rs) ;
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

/* local information */

	pip->lip = &li ;
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

/* input file */
	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ifname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ifname = argp ;
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

/* create-logfile */
	                    case 'c':
	                        lip->have.create = TRUE ;
	                        lip->final.create = TRUE ;
	                        lip->f.create = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.create = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* facility-name */
	                    case 'n':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->have.name = TRUE ;
	                                lip->final.name = TRUE ;
	                                namespec = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
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

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* log-size */
	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->have.logsize = TRUE ;
	                                pip->final.logsize = TRUE ;
	                                rs = cfdecmfi(argp,argl,&v) ;
	                                pip->logsize = v ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* read time-out */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
	                                to_read = v ;
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

	if (rs < 0) {
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	        pip->progname,rs) ;
	    usage(pip) ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_logfile: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
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

/* program search name */

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
	} /* end if (help) */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* load up the environment options */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if ((rs >= 0) && (pip->logsize == 0) && (argval != NULL)) {
	    pip->have.logsize = TRUE ;
	    pip->final.logsize = TRUE ;
	    rs = cfdecmfi(argval,-1,&v) ;
	    pip->logsize = v ;
	}

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

	if (to_read < 1) to_read = 1 ;
	lip->to_read = to_read ;

	if (pip->lfname == NULL) pip->lfname = getourenv(envv,VARLFNAME) ;
	if (pip->lfname == NULL) pip->lfname = getourenv(envv,VARLOGFNAME) ;

	if ((rs >= 0) && (pip->logsize == 0)) {
	    if ((cp = getourenv(envv,VARLOGSIZE)) != NULL) {
	        rs = cfdecmfi(cp,-1,&v) ;
	        pip->logsize = v ;
	    }
	}

/* give the answers */

	if (rs >= 0) {
	    for (ai = 1 ; ai < argc ; ai += 1) {
	        f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	        f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            cp = argv[ai] ;
	            pan += 1 ;
	            lfname = cp ;
	            break ;
	        }
	    } /* end for (looping through positional arguments) */
	} /* end if (ok) */

/* log-file determination */

	if (lfname == NULL)
	    lfname = getourenv(envv,VARLOGFNAME) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_logfile: lfname=%s\n",
	        ((lfname != NULL) ? lfname : "NULL")) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: lfname=%s\n",
	        pip->progname,((lfname != NULL) ? lfname : "NULL")) ;
	}

	if ((rs >= 0) && ((lfname == NULL) || (lfname[0] == '\0'))) {
	    rs = SR_INVALID ;
	    ex = EX_CANTCREAT ;
	}

	if ((rs >= 0) && lip->f.create) {
	    int	oflags = (O_CREAT | O_EXCL | O_WRONLY) ;
	    rs1 = u_open(lfname,oflags,0666) ;
	    if (rs1 >= 0)
	        u_close(rs1) ;
	} /* end if (create-logfile) */

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

/* optional information for logging */

	if (rs >= 0) {
	    if ((rs = locinfo_facility(lip,namespec)) >= 0) {
	        USERINFO	u ;
	        if ((rs = userinfo_start(&u,NULL)) >= 0) {
	            if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	                if ((rs = locinfo_logbegin(lip,&u,lfname)) >= 0) {
	                    EXPCOOK	cooker ;
	                    if ((rs = expcook_start(&cooker)) >= 0) {
	                        if ((rs = loadcooks(pip,&cooker)) >= 0) {
	                            rs = process(pip,&cooker,ifname) ;
	                            if ((rs >= 0) && (pip->debuglevel > 0)) {
					cchar	*pn = pip->progname ;
				        cchar	*fmt = "%s: bytes=%d\n" ;
	                                shio_printf(pip->efp,fmt,pn,rs) ;
	                            }
	                        }
	                        rs1 = expcook_finish(&cooker) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (expcook) */
	                    rs1 = locinfo_logend(lip) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (log) */
	                rs1 = procuserinfo_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (procuserinfo) */
	            rs1 = userinfo_finish(&u) ;
	            if (rs >= 0) rs = rs1 ;
	        } else {
	            cchar	*pn = pip->progname ;
	            cchar	*fmt ;
	            ex = EX_NOUSER ;
	            fmt = "%s: userinfo failure (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	        } /* end if (userinfo) */
	    } /* end if (facility) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	    if (! pip->f.quiet) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: could not perform function (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    }
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
	    debugprintf("b_logfile: exiting ex=%u (%d)\n",ex,rs) ;
#endif

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
	if (rs < 0) {
	    mkreport(pip,argc,argv,rs) ;
	}
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_logfile: final mallout=%u\n",mo-mo_start) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<logfile>] [-c[=<b>]] [-n <name>[:<ver>]]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-s <logsize>] [-if <infile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-names */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	v ;
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
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
	                case akoname_create:
	                    if (! lip->final.create) {
	                        lip->have.create= TRUE ;
	                        lip->final.create = TRUE ;
	                        lip->f.create = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.create = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_logsize:
	                    if (! pip->final.logsize) {
	                        pip->have.logsize = TRUE ;
	                        pip->final.logsize = TRUE ;
	                        if (vl > 0) {
	                            rs = cfdecmfi(vp,vl,&v) ;
	                            pip->logsize = v ;
	                        }
	                    }
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } else
			rs = SR_INVALID ;

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        rs1 = keyopt_curend(kop,&kcur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


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

	if (rs >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    const char	*nn = pip->nodename ;
	    const char	*dn = pip->domainname ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        const char	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}

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


static int process(PROGINFO *pip,EXPCOOK *clp,cchar ifname[])
{
	LOCINFO		*lip = pip->lip ;
	time_t		ti_lastcheck = pip->daytime ;
	const int	llen = LINEBUFLEN ;
	const int	olen = OUTBUFLEN ;
	const int	to = lip->to_read ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		eofstrlen ;
	int		ll ;
	int		obl ;
	int		tlen = 0 ;
	int		f_stdin = FALSE ;
	int		f_fifo = FALSE ;
	const char	*eofstr = EOFSTR ;
	const char	*obp ;
	char		lbuf[LINEBUFLEN + 1] ;
	char		obuf[OUTBUFLEN + 1] ;

	eofstrlen = strlen(eofstr) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_logfile/process: to=%d\n",to) ;
#endif

	if ((ifname == NULL) || (ifname[0] == '\0') || (ifname[0] == '-')) {
	    f_stdin = TRUE ;
	    ifname = STDINFNAME ;
	}

	if (! f_stdin) {
	    struct ustat	sb ;
	    int	rs1 = u_stat(ifname,&sb) ;
	    if (rs1 >= 0) f_fifo = S_ISFIFO(sb.st_mode) ;
	}
	if (rs >= 0) {
	    SHIO	infile, *ifp = &infile ;
	    int		i = 0 ;
	    lbuf[i++] = 'r' ;
	    if (f_fifo) lbuf[i++] = 'n' ;
	    lbuf[i] = '\0' ;
	    if ((rs = shio_open(ifp,ifname,lbuf,0666)) >= 0) {

	        if (f_fifo) {
	            rs = shio_control(ifp,SHIO_CNONBLOCK,TRUE) ;
	        }

	        if (rs >= 0) {
	            while ((rs = shio_readlinetimed(ifp,lbuf,llen,to)) > 0) {
	                ll = rs ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("process: l=>%t<\n",
	                        lbuf,strlinelen(lbuf,ll,50)) ;
#endif

	                if (strncmp(lbuf,eofstr,eofstrlen) == 0) break ;

	                obp = obuf ;
	                if ((rs = procsubs(pip,clp,obuf,olen,lbuf,ll)) >= 0) {
	                    obl = rs ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("process: p=>%t<\n",
	                            obuf,strlinelen(obuf,obl,50)) ;
#endif

	                    if (obl == 0) {
	                        obp = lbuf ;
	                        obl = ll ;
	                    }

	                    if (obl > 0) {
	                        rs = procline(pip,obp,obl) ;
	                        tlen += rs ;
	                    }

	                } /* end if (procsubs) */

	                pip->daytime = time(NULL) ;
	                if (rs >= 0) {
	                    const int	to = INTFLUSH ;
	                    if ((pip->daytime - ti_lastcheck) >= to) {
	                        ti_lastcheck = pip->daytime ;
	                        rs = logfile_check(&pip->lh,pip->daytime) ;
	                    }
	                }

	                if (rs == SR_TIMEDOUT) rs = SR_OK ;
	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end while (reading) */
	        } /* end if (ok) */

	        rs1 = shio_close(ifp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (file-input) */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_logfile/process: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (process) */


static int procline(PROGINFO *pip,cchar obuf[],int olen)
{
	LINEFOLD	lf ;
	const int	cols = LOGCOLS ;
	const int	indent = 2 ;
	int		rs ;
	int		rs1 ;
	int		tlen = 0 ;

	if ((rs = linefold_start(&lf,cols,indent,obuf,olen)) >= 0) {
	    int		i ;
	    int		ll ;
	    const char	*lp ;
	    for (i = 0 ; (ll = linefold_get(&lf,i,&lp)) >= 0 ; i += 1) {
	        tlen += ll ;
	        rs = logfile_write(&pip->lh,lp,ll) ;
	        if (rs < 0) break ;
	    } /* end for */
	    rs1 = linefold_finish(&lf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (linefold) */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procline) */


static int procsubs(pip,ckp,obuf,olen,lbuf,llen)
PROGINFO	*pip ;
EXPCOOK		*ckp ;
char		obuf[] ;
int		olen ;
const char	lbuf[] ;
int		llen ;
{
	LOCINFO		*lip = pip->lip ;
	const time_t	dt = time(NULL) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		obl = 0 ;

	if (strnchr(lbuf,llen,'%') != NULL) {

	    if ((pip->daytime != dt) || (lip->timebuf[0] == '\0')) {
	        pip->daytime = dt ;
	        timestr_logz(dt,lip->timebuf) ;
	        rs = expcook_add(ckp,"T",lip->timebuf,-1) ;
	    }

	    if (rs >= 0) {
	        rs1 = expcook_exp(ckp,0,obuf,olen,lbuf,llen) ;
	        obl = rs1 ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            debugprintf("b_logfile/procsubs: ec_ex() rs=%d\n",rs1) ;
	            debugprintf("b_logfile/procsubs: obuf=>%t<\n",
	                obuf,strlinelen(obuf,obl,40)) ;
	        }
#endif

	        if (rs1 < 0)
	            obl = sncpy1(obuf,olen,"*truncated*") ;
	    }

	} /* end if (needed substitution) */

	return (rs >= 0) ? obl : rs ;
}
/* end subroutine (procsubs) */


static int loadcooks(PROGINFO *pip,EXPCOOK *ckp)
{
	int		rs = SR_OK ;

	if (rs >= 0)
	    rs = expcook_add(ckp,"N",pip->nodename,-1) ;

	if (rs >= 0)
	    rs = expcook_add(ckp,"D",pip->domainname,-1) ;

	if (rs >= 0)
	    rs = expcook_add(ckp,"U",pip->username,-1) ;

	if (rs >= 0)
	    rs = expcook_add(ckp,"NAME",pip->name,-1) ;

	if (rs >= 0) {
	    int		rs1 ;
	    const char	*nn = pip->nodename ;
	    const char	*dn = pip->domainname ;
	    char	hostname[MAXHOSTNAMELEN + 1] ;
	    rs1 = snsds(hostname,MAXHOSTNAMELEN,nn,dn) ;
	    if (rs1 > 0)
	        rs = expcook_add(ckp,"H",hostname,rs1) ;
	}

	return rs ;
}
/* end subroutine (loadcooks) */


static int mkreport(PROGINFO *pip,int argc,cchar **argv,int rv)
{
	const int	ulen = USERNAMELEN ;
	int		rs ;
	char		ubuf[USERNAMELEN+1] ;

	if (pip->daytime == 0) pip->daytime = time(NULL) ;

	if ((rs = getusername(ubuf,ulen,-1)) >= 0) {
	    const mode_t	m = 0777 ;
	    cchar		*dname = pip->progname ;
	    cchar		*oun = pip->username ;
	    char		rbuf[MAXPATHLEN+1] ;
	    pip->username = ubuf ;
	    if ((rs = mktmpreportdir(rbuf,ubuf,dname,m)) >= 0) {
	        char	fbuf[MAXPATHLEN+1] ;
	        if ((rs = mkreportfile(pip,fbuf,rbuf)) >= 0) {
	            const int	nlen = NODENAMELEN ;
	            char	nbuf[NODENAMELEN+1] ;
	            if (pip->pid == 0) pip->pid = getpid() ;
	            if ((rs = getnodename(nbuf,nlen)) >= 0) {
	                const int	llen = LOGIDLEN ;
	                const int	v = pip->pid ;
	                cchar		*onn = pip->nodename ;
	                char		lbuf[LOGIDLEN+1] ;
	                pip->nodename = nbuf ;
	                if ((rs = mklogid(lbuf,llen,nbuf,rs,v)) >= 0) {
	                    {
	                        rs = mkreportout(pip,fbuf,lbuf,argc,argv,rv) ;
	                    }
	                } /* end if (mklogid) */
	                pip->nodename = onn ;
	            } /* end if (getnodename) */
	        } /* end if (mkreportfile) */
	    } /* end if (mktmpuserdir) */
	    pip->username = oun ;
	} /* end if (getusername) */

	return rs ;
}
/* end subroutine (mkreport) */


static int mkreportfile(PROGINFO *pip,char *fbuf,cchar *rbuf)
{
	TMTIME		mt ;
	const time_t	dt = pip->daytime ;
	int		rs ;

	if ((rs = tmtime_localtime(&mt,dt)) >= 0) {
	    const int	tlen = TIMEBUFLEN ;
	    cchar	*fmt = "r%y%m%d%H%M%S" ;
	    char	tbuf[TIMEBUFLEN+1] ;
	    if ((rs = sntmtime(tbuf,tlen,&mt,fmt)) >= 0) {
	        rs = mkpath2(fbuf,rbuf,tbuf) ;
	    } /* end if (sntmtime) */
	} /* end if (localtime) */

	return rs ;
}
/* end subroutine (mkreportfile) */


static int mkreportout(PROGINFO *pip,cchar *fbuf,cchar *id,
	int ac,cchar **av,int rv)
{
	bfile		rfile, *rfp = &rfile ;
	const time_t	dt = pip->daytime ;
	int		rs ;
	cchar		*fmt ;
	char		tbuf[TIMEBUFLEN+1] ;
	timestr_logz(dt,tbuf) ;
	if ((rs = bopen(rfp,fbuf,"wct",0666)) >= 0) {
	    const int	al = DISARGLEN ;
	    int		v = pip->pid ;
	    int		i ;

	    fmt = "%-15s %s junk report (%d)\n" ;
	    bprintf(rfp,fmt,id,tbuf,rv) ;

	    fmt = "%-15s node=%s\n" ;
	    bprintf(rfp,fmt,id,pip->nodename) ;

	    fmt = "%-15s user=%s\n" ;
	    bprintf(rfp,fmt,id,pip->username) ;

	    fmt = "%-15s pid=%u\n" ;
	    bprintf(rfp,fmt,id,v) ;

	    fmt = "%-15s argc=%u args¬\n" ;
	    bprintf(rfp,fmt,id,ac) ;

	    fmt = "%-15s a%02u=>%t<\n" ;
	    for (i = 0 ; (i < ac) && (av[i] != NULL) ; i += 1) {
	        cchar	*ap = av[i] ;
	        rs = bprintf(rfp,fmt,id,i,ap,al) ;
	        if (rs < 0) break ;
	    } /* end if */

	    fmt = "%-15s done\n" ;
	    bprintf(rfp,fmt,id) ;

	    bclose(rfp) ;
	} /* end if (file) */
	return rs ;
}
/* end subroutine (mkreportout) */


/* ARGSUSED */
static int mktmpreportdir(char *rbuf,cchar *ubuf,cchar *dname,mode_t m)
{
	cchar		*rdname = REPORTDNAME ;
	int		rs ;
	int		rl = 0 ;
	if ((rs = mkdirs(rdname,m)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = uc_stat(rdname,&sb)) >= 0) {
	        const mode_t	dm = (m|S_ISVTX) ;
		const uid_t	uid = getuid() ;
		const uid_t	u = sb.st_uid ;
		if (u == uid) {
		    if ((rs = uc_minmod(rdname,dm)) >= 0) {
			cchar	*adm = ADMINUSER ;
			if ((rs = getuid_user(adm,-1)) >= 0) {
			    const uid_t	uid_admin = rs ;
			    rs = uc_chown(rdname,uid_admin,-1) ;
			} else if (isNotPresent(rs)) {
			    rs = SR_OK ;
			}
		    }
		}
		if (rs >= 0) {
		    if ((rs = mkpath2(rbuf,rdname,dname)) >= 0) {
	    	        rl = rs ;
	    	        if ((rs = mkdirs(rbuf,m)) >= 0) {
	        	    rs = uc_minmod(rbuf,dm) ;
	    	        }
		    }
		}
	    } /* end if (stat) */
	} /* end if (mkdirs) */
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (mktmpreportdir) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to_read = TO_READ ;

	return SR_OK ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->alloc != NULL) {
	    rs1 = uc_free(lip->alloc) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->alloc = NULL ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_facility(LOCINFO *lip,cchar *namespec)
{
	int		rs = SR_OK ;

	if ((namespec != NULL) && (namespec[0] != '\0')) {
	    const char	*tp, *cp ;
	    lip->facname = namespec ;
	    if ((tp = strchr(namespec,':')) != NULL) {
	        lip->facversion = (tp+1) ;
	        if ((rs = uc_mallocstrw(namespec,(tp-namespec),&cp)) >= 0) {
	            lip->facname = cp ;
	            lip->alloc = cp ;
	        }
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_facility) */


static int locinfo_logbegin(LOCINFO *lip,USERINFO *uip,const char *lfname)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	const char	*logid = pip->logid ;

	if ((rs = logfile_open(&pip->lh,lfname,0,0666,logid)) >= 0) {
	    pip->open.logprog = TRUE ;
	    if (pip->daytime == 0) pip->daytime = time(NULL) ;
	    if (pip->logsize > 0) {
	        rs = logfile_checksize(&pip->lh,pip->logsize) ;
	    }
	    if (rs >= 0) {
	        time_t		dt = pip->daytime ;
	        const char	*fac = lip->facname ;
	        const char	*ver = lip->facversion ;
	        rs = logfile_userinfo(&pip->lh,uip,dt,fac,ver) ;
	    }
	    if (rs < 0) {
	        pip->open.logprog = FALSE ;
	        logfile_close(&pip->lh) ;
	    }
	} /* end if (logfile-open) */

	return rs ;
}
/* end subroutine (locinfo_logbegin) */


static int locinfo_logend(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->open.logprog) {
	    pip->open.logprog = FALSE ;
	    rs1 = logfile_close(&pip->lh) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_logend) */


