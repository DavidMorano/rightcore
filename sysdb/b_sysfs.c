/* b_sysfs */

/* SHELL built-in to create the sys-users database */
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

	Synopsis:

	$ sysfs [<w>]


*******************************************************************************/


#include	<envstandards.h>	/* must be first to configure */

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
#include	<getbufsize.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<setstr.h>
#include	<bwops.h>
#include	<field.h>
#include	<nulstr.h>
#include	<getax.h>
#include	<getus.h>
#include	<getua.h>
#include	<passwdent.h>
#include	<shadowent.h>
#include	<groupent.h>
#include	<projectent.h>
#include	<userattrent.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_sysfs.h"
#include	"defs.h"
#include	"opensysfs.h"


/* local defines */

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	PROCUSERS	struct procusers
#define	PROCUSERS_FL	struct procusers_flags

#define	SYSMODE		0664


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mknpath2(char *,int,cchar *,cchar *) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	pathadd(char *,int,cchar *) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	mktmpfile(char *,mode_t,cchar *) ;
extern int	hasalldig(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		wargs:1 ;
	uint		dbfname:1 ;
	uint		to:1 ;
} ;

struct locinfo {
	PROGINFO	*pip ;
	cchar		*dbfname ;
	cchar		*fname ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	init, open ;
	vecstr		stores ;
	uint		wargs ;
	int		pagesize ;
	int		to ;
} ;

struct procusers_flags {
	uint		users:1 ;
} ;

struct procusers {
	bfile		utfile ;
	bfile		ptfile ;
	setstr		users ;
	PROCUSERS_FL	open ;
	int		pwlen ;
	int		plen ;
	int		llen ;
	int		uhl ;
	const void	*a ;
	char		*pwbuf ;
	char		*uhbuf ;
	char		*utfname ;
	char		*ptfname ;
	char		*lbuf ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procsysdir(PROGINFO *) ;
static int	procdirgroup(PROGINFO *,cchar *,struct ustat *) ;
static int	procuhdir(PROGINFO *,char *,int,cchar *) ;
static int	proctmpfile(PROGINFO *,char *,cchar *,mode_t) ;
static int	process(PROGINFO *,int) ;
static int	procwarg(PROGINFO *,int) ;
static int	procusers(PROGINFO *,int) ;
static int	procusers_begin(PROGINFO *,PROCUSERS *) ;
static int	procusers_allocbegin(PROGINFO *,PROCUSERS *) ;
static int	procusers_allocend(PROGINFO *,PROCUSERS *) ;
static int	procusers_beginer(PROGINFO *,PROCUSERS *,cchar *,mode_t) ;
static int	procusers_ent(PROGINFO *,PROCUSERS *,struct passwd *) ;
static int	procusers_end(PROGINFO *,PROCUSERS *) ;
static int	procuserlink(PROGINFO *,char *,int,struct passwd *) ;
static int	procgroups(PROGINFO *,int) ;
static int	procgroups_load(PROGINFO *,bfile *,bfile *) ;
static int	procgroups_ent(PROGINFO *,bfile *,bfile *,
			char *,int,struct group *) ;
static int	procprojects(PROGINFO *,int) ;
static int	procprojects_load(PROGINFO *,bfile *,bfile *) ;
static int	procshells(PROGINFO *,int) ;
static int	procshadow(PROGINFO *,int) ;
static int	procuserattr(PROGINFO *,int) ;

static int	procopts(PROGINFO *,KEYOPT *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_warg(LOCINFO *,int) ;
static int	locinfo_dbfname(LOCINFO *,cchar *) ;
static int	locinfo_to(LOCINFO *,int) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_defaults(LOCINFO *) ;
static int	locinfo_finish(LOCINFO *) ;


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
	"db",
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
	argopt_db,
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
	"utf",
	"db",
	NULL
} ;

enum akonames {
	akoname_utf,
	akoname_db,
	akoname_overlast
} ;

/* define the configuration keywords */
static const char	*wopts[] = {
	"userhomes",
	"usernames",
	"groupnames",
	"projectnames",
	"passwd",
	"group",
	"project",
	"realname",
	"shells",
	"shadow",
	"userattr",
	NULL
} ;

enum wopts {
	wopt_userhomes,
	wopt_usernames,
	wopt_groupnames,
	wopt_projectnames,
	wopt_passwd,
	wopt_group,
	wopt_project,
	wopt_realname,
	wopt_shells,
	wopt_shadow,
	wopt_userattr,
	wopt_overlast
} ;


/* exported subroutines */


int b_sysfs(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_sysfs) */


int p_sysfs(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_sysfs) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	BITS		pargs ;
	KEYOPT		akopts ;
	SHIO		errfile ;
#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		to = -1 ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f ;
	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*efname = NULL ;
	cchar		*afname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*dbfname = NULL ;
	cchar		*wspec = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_sysfs: starting DFD=%d\n",rs) ;
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

/* DB file */
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            dbfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                dbfname = argp ;
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

/* file-name (for FS operations) */
	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->fname = argp ;
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

/* time-out */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
	                                to = v ;
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

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_sysfs: debuglevel=%u\n",pip->debuglevel) ;
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
	    shio_printf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help) {
#if	CF_SFIO
	    rs = printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    rs = printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	} /* end if */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialization */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if ((rs = locinfo_dbfname(lip,dbfname)) >= 0) {
	    if ((rs = locinfo_to(lip,to)) >= 0) {
	        if ((rs = procopts(pip,&akopts)) >= 0) {
	    	    rs = locinfo_defaults(lip) ;
		}
	    }
	}

/* process */

	if (rs >= 0) {
	    if ((rs = procsysdir(pip)) >= 0) {
	        int	w = wopt_userhomes ;

	        for (ai = 1 ; ai < argc ; ai += 1) {

	            f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	            f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	            if (f) {
	                wspec = argv[ai] ;
	                if (wspec != NULL) {
	                    int	wch = MKCHAR(wspec[0]) ;
	                    pan += 1 ;
	                    if (isdigitlatin(wch)) {
	                        rs = optvalue(wspec,-1) ;
	                        w = rs ;
	                    } else {
	                        if ((w = matostr(wopts,1,wspec,-1)) < 0) {
	                            rs = SR_INVALID ;
				}
	                    }
	                    if (rs >= 0)
	                        rs = process(pip,w) ;
	                } /* end if (non-NULL w-spec) */
	            } /* end if */

	            if (rs < 0) break ;
	        } /* end for (arguments) */

	        if ((rs >= 0) && (pan == 0)) {
	            rs = process(pip,w) ;
	        }

	    } /* end if (procsysdir) */
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
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            shio_printf(pip->efp,"%s: invalid query (%d)\n",
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

baderropen:
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
	    debugprintf("b_sysfs: final mallout=%u\n",(mo-mo_start)) ;
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
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<w>]\n" ;
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
	                case akoname_utf:
	                case akoname_db:
	                    if (! lip->final.dbfname) {
	                        lip->have.dbfname = TRUE ;
	                        lip->final.dbfname = TRUE ;
	                        if (vl > 0) {
	                            cchar	**vpp = &lip->dbfname ;
	                            rs = locinfo_setentry(lip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } else {
	                rs = SR_INVALID ;
		    }

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procsysdir(PROGINFO *pip)
{
	struct ustat	sb ;
	const mode_t	dm = (0777 | S_ISGID|S_IXGRP) ;
	int		rs ;
	cchar		*sdname = OPENSYSFS_SYSDNAME ;

	if ((rs = u_stat(sdname,&sb)) >= 0) {
	    if ((sb.st_mode & S_IAMB) != dm) {
	        uid_t	euid = geteuid() ;
	        if (sb.st_uid == euid) {
	            if ((rs = uc_minmod(sdname,dm)) >= 0) {
	                rs = procdirgroup(pip,sdname,&sb) ;
	            }
	        }
	    }
	} else if (rs == SR_NOENT) {
	    if ((rs = mkdirs(sdname,dm)) >= 0) {
	        if ((rs = uc_minmod(sdname,dm)) >= 0) {
	            if ((rs = u_stat(sdname,&sb)) >= 0) {
	                rs = procdirgroup(pip,sdname,&sb) ;
	            }
	        }
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysfs/procsysdir: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procsysdir) */


static int procdirgroup(PROGINFO *pip,cchar sdname[],struct ustat *sbp)
{
	struct ustat	psb ;
	int		rs = SR_OK ;
	int		cl ;
	int		f = FALSE ;
	cchar		*cp ;

	if (pip == NULL) return SR_FAULT ;

	if ((cl = sfdirname(sdname,-1,&cp)) > 0) {
	    NULSTR	dstr ;
	    cchar	*dname ;
	    if ((rs = nulstr_start(&dstr,cp,cl,&dname)) >= 0) {
	        if ((rs = u_stat(dname,&psb)) >= 0) {
	            if (psb.st_gid != sbp->st_gid) {
	                if ((u_chown(sdname,-1,psb.st_gid)) >= 0) {
	                    f = TRUE ;
	                }
	            }
	        } /* end if (stat) */
	        nulstr_finish(&dstr) ;
	    } /* end if (nulstr) */
	} /* end if (sfdirname) */

	return (rs >= 0) ? f : rs ;
}
/* end subrotine (procdirgroup) */


static int process(PROGINFO *pip,int w)
{
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysfs/process: w=%u\n",w) ;
#endif

	if ((rs = procwarg(pip,w)) > 0) {
	    if (pip->debuglevel > 0) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: processing ­%s­ (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,wopts[w],w) ;
	    }
	    switch (w) {
	    case wopt_userhomes:
	    case wopt_usernames:
	    case wopt_passwd:
	        rs = procusers(pip,w) ;
	        break ;
	    case wopt_groupnames:
	    case wopt_group:
	        rs = procgroups(pip,w) ;
	        break ;
	    case wopt_projectnames:
	    case wopt_project:
	        rs = procprojects(pip,w) ;
	        break ;
	    case wopt_shells:
	        rs = procshells(pip,w) ;
	        break ;
	    case wopt_shadow:
	        rs = procshadow(pip,w) ;
	        break ;
	    case wopt_userattr:
	        rs = procuserattr(pip,w) ;
	        break ;
	    default:
	        rs = SR_INVALID ;
	        break ;
	    } /* end switch */
	} /* end if (proc-warg) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysfs/process: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (process) */


static int procwarg(PROGINFO *pip,int w)
{
	LOCINFO		*lip = pip->lip ;
	const int wusers[] = { wopt_userhomes, wopt_usernames, 
	            wopt_passwd, -1 } ;
	const int wgroups[] = { wopt_groupnames, wopt_group, -1 } ;
	const int wprojects[] = { wopt_projectnames, wopt_project, -1 } ;
	int		rs ;
	int		f = FALSE ;

	if ((rs = locinfo_warg(lip,w)) > 0) {
	    const int	*was = NULL ;
	    f = rs ;
	    switch (w) {
	    case wopt_userhomes:
	    case wopt_usernames:
	    case wopt_passwd:
	        was = wusers ;
	        break ;
	    case wopt_groupnames:
	    case wopt_group:
	        was = wgroups ;
	        break ;
	    case wopt_projectnames:
	    case wopt_project:
	        was = wprojects ;
	        break ;
	    } /* end switch */
	    if (was != NULL) {
	        int	i ;
	        for (i = 0 ; (rs >= 0) && (was[i] >= 0) ; i += 1) {
	            rs = locinfo_warg(lip,was[i]) ;
	        } /* end for */
	    }
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procwarg) */


static int procusers(PROGINFO *pip,int w)
{
	PROCUSERS	pu ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = procusers_begin(pip,&pu)) >= 0) {
	    struct passwd	pw ;
	    const int		pwlen = pu.pwlen ;
	    char		*pwbuf = pu.pwbuf ;
	    if ((rs = getpw_begin()) >= 0) {
	        while ((rs = getpw_ent(&pw,pwbuf,pwlen)) > 0) {
	            rs = procusers_ent(pip,&pu,&pw) ;
		    c += rs ;
	            if (rs < 0) break ;
	        } /* end while */
	        getpw_end() ;
	    } /* end if (getpw) */
	    rs1 = procusers_end(pip,&pu) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (procusers) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_sysfs/procusers: out rs=%d dl=%u c=%u\n",
	        rs,pip->debuglevel,c) ;
#endif

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp,"%s: (%u) nusers=%u\n",pip->progname,w,c) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_sysfs/procusers: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procusers) */


static int procusers_begin(PROGINFO *pip,PROCUSERS *pup)
{
	int		rs ;
	cchar		*sd = OPENSYSFS_SYSDNAME ;
	cchar		*uh = OPENSYSFS_FUSERHOMES ;

	if ((rs = procusers_allocbegin(pip,pup)) >= 0) {
	    const int	uhlen = pup->plen ;
	    if ((rs = procuhdir(pip,pup->uhbuf,uhlen,uh)) >= 0) {
		const mode_t	om = SYSMODE ;
	        char		*uf = pup->utfname ;
	        pup->uhl = rs ;
	        if ((rs = proctmpfile(pip,uf,sd,om)) >= 0) {
		    rs = procusers_beginer(pip,pup,sd,om) ;
	            if (rs < 0)
	                uc_unlink(pup->utfname) ;
	        } /* end if (tmp-file) */
	    } /* end if (proc-user-dir) */
	    if (rs < 0)
		procusers_allocend(pip,pup) ;
	} /* end if (procusers_allocbegin) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_sysfs/procusers_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutines (procusers_begin) */


static int procusers_allocbegin(PROGINFO *pip,PROCUSERS *pup)
{
	const int	pwlen = getbufsize(getbufsize_pw) ;
	const int	plen = MAXPATHLEN ;
	int		rs ;
	int		llen ;
	int		size = 0 ;
	char		*bp ;
	if (pip == NULL) return SR_FAULT ;
	llen = (pwlen+LINEBUFLEN) ;
	size += (pwlen+1) ;
	size += (3*(plen+1)) ;
	size += (llen+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    pup->a = bp ;
	    pup->pwbuf = bp ; bp += (pwlen+1) ;
	    pup->pwlen = pwlen ;
	    pup->uhbuf = bp ; bp += (plen+1) ;
	    pup->utfname = bp ; bp += (plen+1) ;
	    pup->ptfname = bp ; bp += (plen+1) ;
	    pup->plen = plen ;
	    pup->lbuf = bp ; bp += (llen+1) ;
	    pup->llen = llen ;
	}
	return rs ;
}
/* end subroutines (procusers_allocbegin) */


static int procusers_allocend(PROGINFO *pip,PROCUSERS *pup)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip == NULL) return SR_FAULT ;
	if (pup->a != NULL) {
	    rs1 = uc_free(pup->a) ;
	    if (rs >= 0) rs = rs1 ;
	    pup->a = NULL ;
	    pup->pwlen = 0 ;
	    pup->plen = 0 ;
	    pup->llen = 0 ;
	}
	return rs ;
}
/* end subroutines (procusers_allocend) */


static int procusers_beginer(PROGINFO *pip,PROCUSERS *pup,cchar *sd,mode_t om)
{
	const int	n = 50 ;
	int		rs ;
	cchar		*ufn = pup->utfname ;
	if ((rs = bopenmod(&pup->utfile,ufn,"wct",om)) >= 0) {
	    char	*pfn = pup->ptfname ;
	    if ((rs = proctmpfile(pip,pfn,sd,om)) >= 0) {
		if ((rs = bopenmod(&pup->ptfile,pfn,"wct",om)) >= 0) {
		    if ((rs = setstr_start(&pup->users,n)) >= 0) {
			pup->open.users = TRUE ;
		    }
		    if (rs < 0) {
			bclose(&pup->ptfile) ;
		    }
		} /* end if (bopenmod) */
	 	if (rs < 0)
	 	    uc_unlink(pup->ptfname) ;
	    } /* end if (tmp-file) */
	    if (rs < 0)
		bclose(&pup->utfile) ;
	} /* end if (file-open) */
	return rs ;
}
/* end subroutine (procusers_beginer) */


static int procusers_end(PROGINFO *pip,PROCUSERS *pup)
{
	int		rs = SR_OK ;
	int		rs1 ;
	cchar		*sdname = OPENSYSFS_SYSDNAME ;

	if (pip == NULL) return SR_FAULT ;

	if (pup->open.users) {
	    pup->open.users = FALSE ;
	    rs1 = setstr_finish(&pup->users) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if ((rs1 = bclose(&pup->ptfile)) >= 0) {
	    cchar	*pcname = OPENSYSFS_FPASSWD ;
	    char	pfname[MAXPATHLEN+1] ;
	    if ((rs1 = mkpath2(pfname,sdname,pcname)) >= 0) {
	        rs1 = u_rename(pup->ptfname,pfname) ;
	    }
	}
	if (rs1 < 0) {
	    u_unlink(pup->ptfname) ;
	}
	if (rs >= 0) rs = rs1 ;

	if ((rs1 = bclose(&pup->utfile)) >= 0) {
	    cchar	*ucname = OPENSYSFS_FUSERNAMES ;
	    char	ufname[MAXPATHLEN+1] ;
	    if ((rs1 = mkpath2(ufname,sdname,ucname)) >= 0) {
	        rs1 = u_rename(pup->utfname,ufname) ;
	    }
	}
	if (rs1 < 0) {
	    u_unlink(pup->utfname) ;
	}
	if (rs >= 0) rs = rs1 ;

	rs1 = procusers_allocend(pip,pup) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (procusers_end) */


static int procusers_ent(PROGINFO *pip,PROCUSERS *pup,struct passwd *pwp)
{
	setstr		*usp = &pup->users ;
	bfile		*ufp = &pup->utfile ;
	bfile		*pfp = &pup->ptfile ;
	const int	unl = strlen(pwp->pw_name) ;
	int		rs ;
	int		c = 0 ;
	cchar		*unp = pwp->pw_name ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_sysfs/procuser: ent\n") ;
#endif

	if ((rs = setstr_add(usp,unp,unl)) > 0) {
	    c = 1 ;
	    if ((rs = bprintln(ufp,unp,unl)) >= 0) {
	        const int	llen = pup->llen ;
	        char		*lbuf = pup->lbuf ;
	        if ((rs = passwdent_format(pwp,lbuf,llen)) >= 0) {
	            const int	ll = rs ;
	            if ((rs = bprintln(pfp,lbuf,ll)) >= 0) {
	                const int	uhl = pup->uhl ;
	                char		*uhbuf = pup->uhbuf ;
	                rs = procuserlink(pip,uhbuf,uhl,pwp) ;
	            }
	        }
	    } /* end if (print-out) */
	} /* end if (setstr_add) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_sysfs/procuser: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procuser_ent) */


static int procuserlink(PROGINFO *pip,char uhbuf[],int uhlen,struct passwd *pwp)
{
	int		rs ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_sysfs/procuserlink: w=%s\n",pwp->pw_name) ;
	    debugprintf("b_sysfs/procuserlink: uhlen=%u uhbuf=%t\n",
	        uhlen,uhbuf,uhlen) ;
	}
#endif

	if ((rs = pathadd(uhbuf,uhlen,pwp->pw_name)) >= 0) {
	    const int	rlen = MAXPATHLEN ;
	    char	rbuf[MAXPATHLEN+1] ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_sysfs/procuserlink: read uhbuf=%s\n",uhbuf) ;
#endif
	    if ((rs = u_readlink(uhbuf,rbuf,rlen)) >= 0) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("b_sysfs/procuserlink: u_readlink() rs=%d\n",
	                rs) ;
	            debugprintf("b_sysfs/procuserlink: rbuf=%s\n",rbuf) ;
	        }
#endif
	        if (strcmp(rbuf,pwp->pw_dir) != 0) {
	            if ((rs = u_unlink(uhbuf)) >= 0) {
	                rs = u_symlink(pwp->pw_dir,uhbuf) ;
	            }
	        }
	    } else if (rs == SR_NOENT) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_sysfs/procuserlink: no-entry\n") ;
#endif
	        rs = u_symlink(pwp->pw_dir,uhbuf) ;
	    }
	} /* end if (mkpath) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_sysfs/procuserlink: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procuserlink) */


static int procgroups(PROGINFO *pip,int w)
{
	const mode_t	om = SYSMODE ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*sd = OPENSYSFS_SYSDNAME ;
	char		gstfname[MAXPATHLEN+1] ;

	if ((rs = proctmpfile(pip,gstfname,sd,om)) >= 0) {
	    bfile	gsfile, *gsfp = &gsfile ;
	    if ((rs = bopenmod(gsfp,gstfname,"wct",om)) >= 0) {
	        char	grtfname[MAXPATHLEN+1] ;
	        if ((rs = proctmpfile(pip,grtfname,sd,om)) >= 0) {
	            bfile	grfile, *grfp = &grfile ;
	            if ((rs = bopenmod(grfp,grtfname,"wct",om)) >= 0) {
	                rs = procgroups_load(pip,gsfp,grfp) ;
	                c += rs ;
	                rs1 = bclose(grfp) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (grfile-open) */
	            if (rs >= 0) {
	                cchar	*grcname = OPENSYSFS_FGROUP ;
	                char	grfname[MAXPATHLEN+1] ;
	                if ((rs = mkpath2(grfname,sd,grcname)) >= 0) {
	                    rs = u_rename(grtfname,grfname) ;
	                }
	            }
	            if (rs < 0)
	                u_unlink(grtfname) ;
	        } /* end if (proc-grt-file) */
	        rs1 = bclose(gsfp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (gsfile-open) */
	    if (rs >= 0) {
	        cchar	*gscname = OPENSYSFS_FGROUPNAMES ;
	        char	gsfname[MAXPATHLEN+1] ;
	        if ((rs = mkpath2(gsfname,sd,gscname)) >= 0) {
	            rs = u_rename(gstfname,gsfname) ;
	        }
	    }
	    if (rs < 0)
	        u_unlink(gstfname) ;
	} /* end if (proc-gst-file) */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp,"%s: (%u) ngroups=%u\n",pip->progname,w,c) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysfs/procgroups: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procgroups) */


static int procgroups_load(PROGINFO *pip,bfile *gsfp,bfile *grfp)
{
	struct group	gr ;
	setstr		groups, *gsp = &groups ;
	const int	n = 40 ;
	const int	grlen = getbufsize(getbufsize_gr) ;
	int		rs ;
	int		rs1 ;
	int		llen ;
	int		size = 0 ;
	int		c = 0 ;
	char		*grbuf ;
	char		*lbuf ;
	char		*bp ;
	if (pip == NULL) return SR_FAULT ;
	llen = (grlen+LINEBUFLEN) ;
	size = ((grlen+1)+(llen+1)) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    grbuf = (bp+0) ;
	    lbuf = (bp+(grlen+1)) ;
	    if ((rs = setstr_start(gsp,n)) >= 0) {
	        if ((rs = getgr_begin()) >= 0) {
	            while ((rs = getgr_ent(&gr,grbuf,grlen)) > 0) {
	                cchar	*gn = gr.gr_name ;
		        if ((rs = setstr_add(gsp,gn,-1)) > 0) {
			    rs = procgroups_ent(pip,gsfp,grfp,lbuf,llen,&gr) ;
			    c += 1 ;
		        }
	                if (rs < 0) break ;
	            } /* end while */
	            getgr_end() ;
	        } /* end if (getgr) */
		rs1 = setstr_finish(gsp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (setstr) */
	    uc_free(bp) ;
	} /* end if (memory-allocation) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procgroups_load) */


static int procgroups_ent(PROGINFO *pip,bfile *gsfp,bfile *grfp,
	char *lbuf,int llen,struct group *grp)
{
	int		rs ;
	int		c = 0 ;
	cchar		*gn = grp->gr_name ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = bprintln(gsfp,gn,-1)) >= 0) {
	    if ((rs = groupent_format(grp,lbuf,llen)) >= 0) {
		c = 1 ;
		rs = bprintln(grfp,lbuf,rs) ;
	    }
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procgroups_ent) */


static int procprojects(PROGINFO *pip,int w)
{
	const mode_t	om = SYSMODE ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*sd = OPENSYSFS_SYSDNAME ;
	char		pntfname[MAXPATHLEN+1] ;

	if ((rs = proctmpfile(pip,pntfname,sd,om)) >= 0) {
	    bfile	pnfile, *pnfp = &pnfile ;
	    if ((rs = bopenmod(pnfp,pntfname,"wct",om)) >= 0) {
	        char	pjtfname[MAXPATHLEN+1] ;
	        if ((rs = proctmpfile(pip,pjtfname,sd,om)) >= 0) {
	            bfile	pjfile, *pjfp = &pjfile ;
	            if ((rs = bopenmod(pjfp,pjtfname,"wct",om)) >= 0) {
	                rs = procprojects_load(pip,pnfp,pjfp) ;
	                c += rs ;
	                rs1 = bclose(pjfp) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (pjfile-open) */
	            if (rs >= 0) {
	                cchar	*pjcname = OPENSYSFS_FPROJECT ;
	                char	pjfname[MAXPATHLEN+1] ;
	                if ((rs = mkpath2(pjfname,sd,pjcname)) >= 0) {
	                    rs = u_rename(pjtfname,pjfname) ;
	                }
	            }
	            if (rs < 0)
	                u_unlink(pjtfname) ;
	        } /* end if (proc-pjt-file) */
	        rs1 = bclose(pnfp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (pnfile-open) */
	    if (rs >= 0) {
	        cchar	*pncname = OPENSYSFS_FPROJECTNAMES ;
	        char	pnfname[MAXPATHLEN+1] ;
	        if ((rs = mkpath2(pnfname,sd,pncname)) >= 0) {
	            rs = u_rename(pntfname,pnfname) ;
	        }
	    }
	    if (rs < 0)
	        u_unlink(pntfname) ;
	} /* end if (proc-pnt-file) */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp,"%s: (%u) nprojects=%u\n",pip->progname,w,c) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysfs/procprojects: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procprojects) */


static int procprojects_load(PROGINFO *pip,bfile *pnfp,bfile *pjfp)
{
	struct project	pj ;
	const int	pjlen = getbufsize(getbufsize_pj) ;
	int		rs ;
	int		c = 0 ;
	int		llen ;
	int		size ;
	char		*pjbuf ;
	char		*lbuf ;
	char		*bp ;
	if (pip == NULL) return SR_FAULT ;
	llen = (pjlen+LINEBUFLEN) ;
	size = ((pjlen+1)+(llen+1)) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    pjbuf = (bp+0) ;
	    lbuf = (bp+(pjlen+1)) ;
	    if ((rs = getpj_begin()) >= 0) {
	        while ((rs = getpj_ent(&pj,pjbuf,pjlen)) > 0) {
	            cchar	*pn = pj.pj_name ;
	            c += 1 ;
	            if ((rs = bprintln(pnfp,pn,-1)) >= 0) {
	                if ((rs = projectent_format(&pj,lbuf,llen)) >= 0) {
	                    rs = bprintln(pjfp,lbuf,rs) ;
	                }
	            }
	            if (rs < 0) break ;
	        } /* end while */
	        getpj_end() ;
	    } /* end if (getpj) */
	    uc_free(bp) ;
	} /* end if (memory-allocation) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procprojects_load) */


static int procshells(PROGINFO *pip,int w)
{
	const mode_t	om = SYSMODE ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*sdname = OPENSYSFS_SYSDNAME ;
	char		tmpusfname[MAXPATHLEN+1] ;

	if ((rs = proctmpfile(pip,tmpusfname,sdname,om)) >= 0) {
	    bfile	usfile, *usfp = &usfile ;
	    if ((rs = bopenmod(usfp,tmpusfname,"wct",om)) >= 0) {
	        const int	rlen = MAXPATHLEN ;
	        char		rbuf[MAXPATHLEN+1] ;
	        if ((rs = getus_begin()) >= 0) {
	            while ((rs = getus_ent(rbuf,rlen)) > 0) {
	                c += 1 ;
	                rs = bprintln(usfp,rbuf,rs) ;
	                if (rs < 0) break ;
	            } /* end while */
	            rs1 = getus_end() ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (getus) */
	        rs1 = bclose(usfp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (shells-process) */
	    if (rs >= 0) {
	        cchar	*uscname = OPENSYSFS_FSHELLS ;
	        char	usfname[MAXPATHLEN+1] ;
	        if ((rs = mkpath2(usfname,sdname,uscname)) >= 0) {
	            rs = u_rename(tmpusfname,usfname) ;
	        }
	    }
	    if (rs < 0)
	        u_unlink(tmpusfname) ;
	} /* end if (proc-tmpus-file) */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp,"%s: (%u) nshells=%u\n",pip->progname,w,c) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysfs/procshells: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procshells) */


static int procshadow(PROGINFO *pip,int w)
{
	const mode_t	om = 0660 ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*sdname = OPENSYSFS_SYSDNAME ;
	char		tmpfname[MAXPATHLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysfs/procshadow: ent\n") ;
#endif

	if ((rs = proctmpfile(pip,tmpfname,sdname,om)) >= 0) {
	    bfile	sfile, *sfp = &sfile ;
	    if ((rs = bopenmod(sfp,tmpfname,"wct",om)) >= 0) {
	        struct spwd	sp ;
	        const int	splen = getbufsize(getbufsize_sp) ;
	        char		*spbuf ;
		if ((rs = uc_malloc((splen+1),&spbuf)) >= 0) {
	            if ((rs = getsp_begin()) >= 0) {
	                while ((rs = getsp_ent(&sp,spbuf,splen)) > 0) {
	                    const int	llen = LINEBUFLEN ;
	                    char	lbuf[LINEBUFLEN+1] ;
	                    if ((rs = shadowent_format(&sp,lbuf,llen)) >= 0) {
	                        c += 1 ;
	                        rs = bprintln(sfp,lbuf,rs) ;
	                    }
	                    if (rs < 0) break ;
	                } /* end while */
	                rs1 = getsp_end() ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (getsp) */
		    uc_free(spbuf) ;
		} /* end if (memory-allocation) */
	        rs1 = bclose(sfp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (shadow-process) */
	    if (rs >= 0) {
	        cchar	*cname = OPENSYSFS_FSHADOW ;
	        char	sfname[MAXPATHLEN+1] ;
	        if ((rs = mkpath2(sfname,sdname,cname)) >= 0) {
	            rs = u_rename(tmpfname,sfname) ;
	        }
	    }
	    if (rs < 0)
	        u_unlink(tmpfname) ;
	} /* end if (proc-tmp-file) */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp,"%s: (%u) nshadow=%u\n",pip->progname,w,c) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysfs/procshadow: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procshadow) */


static int procuserattr(PROGINFO *pip,int w)
{
	const mode_t	om = 0664 ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*sdname = OPENSYSFS_SYSDNAME ;
	char		tmpfname[MAXPATHLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysfs/procuserattr: ent\n") ;
#endif

	if ((rs = proctmpfile(pip,tmpfname,sdname,om)) >= 0) {
	    bfile	sfile, *sfp = &sfile ;
	    if ((rs = bopenmod(sfp,tmpfname,"wct",om)) >= 0) {
	        userattr_t	ua ;
	        const int	ualen = getbufsize(getbufsize_ua) ;
	        char		*uabuf ;
		if ((rs = uc_malloc((ualen+1),&uabuf)) >= 0) {
	            if ((rs = getua_begin()) >= 0) {
	                while ((rs = getua_ent(&ua,uabuf,ualen)) > 0) {
	                    const int	llen = LINEBUFLEN ;
	                    char	lbuf[LINEBUFLEN+1] ;
	                    if ((rs = userattrent_format(&ua,lbuf,llen)) > 0) {
	                        c += 1 ;
	                        rs = bprintln(sfp,lbuf,rs) ;
	                    }
	                    if (rs < 0) break ;
	                } /* end while */
	                rs1 = getua_end() ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (getua) */
		    uc_free(uabuf) ;
		} /* end if (memory-allocation) */
	        rs1 = bclose(sfp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (shadow-process) */
	    if (rs >= 0) {
	        cchar	*cname = OPENSYSFS_FUSERATTR ;
	        char	sfname[MAXPATHLEN+1] ;
	        if ((rs = mkpath2(sfname,sdname,cname)) >= 0) {
	            rs = u_rename(tmpfname,sfname) ;
	        }
	    }
	    if (rs < 0)
	        u_unlink(tmpfname) ;
	} /* end if (proc-tmp-file) */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp,"%s: (%u) nuserattr=%u\n",pip->progname,w,c) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_sysfs/procuserattr: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procuserattr) */


static int procuhdir(PROGINFO *pip,char *uhbuf,int uhlen,cchar *uhfname)
{
	const mode_t	dm = 0777 ;
	int		rs ;
	int		unl = 0 ;
	cchar		*sdname = OPENSYSFS_SYSDNAME ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = mknpath2(uhbuf,uhlen,sdname,uhfname)) >= 0) {
	    struct ustat	sb ;
	    unl = rs ;

	    if ((rs = u_stat(uhbuf,&sb)) >= 0) {
	        if ((sb.st_mode & S_IAMB) != dm) {
	            uid_t	euid = geteuid() ;
	            if (sb.st_uid == euid) {
			rs = uc_minmod(uhbuf,dm) ;
		    }
	        }
	    } else if (rs == SR_NOENT) {
	        if ((rs = mkdirs(uhbuf,dm)) >= 0) {
	            rs = uc_minmod(uhbuf,dm) ;
	        }
	    }

	} /* end if (mkpath user-home dir) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_sysfs/prouhdir: ret rs=%d unl=%u\n",rs,unl) ;
#endif

	return (rs >= 0) ? unl : rs ;
}
/* end subroutine (procuhdir) */


static int proctmpfile(PROGINFO *pip,char *tbuf,cchar *sdname,mode_t om)
{
	int		rs ;
	cchar		*cname = ".sysfsXXXXXXXX" ;
	char		template[MAXPATHLEN+1] ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = mkpath2(template,sdname,cname)) >= 0) {
	    rs = mktmpfile(tbuf,om,template) ;
	}

	return rs ;
}
/* end subroutine (proctmpfile) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to = -1 ;

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


static int locinfo_warg(LOCINFO *lip,int w)
{
	int		rs ;
	int		f = FALSE ;

	if ((rs = bwtst(lip->wargs,w)) == 0) {
	    f = TRUE ;
	    rs = bwset(lip->wargs,w) ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_warg) */


static int locinfo_dbfname(LOCINFO *lip,cchar *dbfname)
{

	if (lip == NULL)
	    return SR_FAULT ;

	if (dbfname != NULL) {
	    lip->have.dbfname = TRUE ;
	    lip->final.dbfname = TRUE ;
	    lip->dbfname = dbfname ;
	}

	return SR_OK ;
}
/* end subroutine (locinfo_dbfname) */


static int locinfo_to(LOCINFO *lip,int to)
{

	if (to < 0) to = TO_CACHE ;

	lip->to = to ;
	return SR_OK ;
}
/* end subroutine (locinfo_to) */


static int locinfo_defaults(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip == NULL)
	    return SR_FAULT ;

	if ((lip->dbfname == NULL) && (! lip->final.dbfname)) {
	    cchar	*cp = getourenv(pip->envv,VARDBFNAME) ;
	    if (cp != NULL) {
	        cchar	**vpp = &lip->dbfname ;
	        rs = locinfo_setentry(lip,vpp,cp,-1) ;
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_defaults) */


