/* b_mailexpire */

/* front-end to MailExpire */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_LOCPRINTF	0		/* |locinfo_eprintf()| */
#define	CF_CONFIGCHECK	0		/* |config_check()| */


/* revision history:

	= 1999-06-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We expire old mail messages.

	Synopsis:

	$ mailexpire <mbfile(s)>


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
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<userinfo.h>
#include	<paramopt.h>
#include	<field.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<vecstr.h>
#include	<sysusernames.h>
#include	<mailbox.h>
#include	<toxc.h>
#include	<prsetfname.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_mailexpire.h"
#include	"defs.h"
#include	"mbcache.h"


/* local defines */

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifdef	ALIASNAMELEN
#define	ABUFLEN		ALIASNAMELEN
#else
#define	ABUFLEN		64
#endif

#ifndef	VBUFLEN
#ifdef	MAILADDRLEN
#define	VBUFLEN		MAILADDRLEN
#else
#define	VBUFLEN		2048
#endif
#endif /* VBUFLEN */

#ifndef	EBUFLEN
#define	EBUFLEN		(2 * MAXPATHLEN)
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	CONFIG		struct config

#define	PO_MAILDIRS	"maildirs"
#define	PO_MAILUSERS	"mailusers"


/* external subroutines */

extern uint	nextpowtwo(uint) ;

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkpath3w(char *,cchar *,cchar *,cchar *,int) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	permsched(cchar **,vecstr *,char *,int,cchar *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getusername(char *,int,uid_t) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
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
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		maildirs:1 ;
	uint		mailusers:1 ;
	uint		ofile:1 ;
	uint		linelen:1 ;
	uint		nshow:1 ;
	uint		sort:1 ;
	uint		sortrev:1 ;
	uint		msgto:1 ;
	uint		allusers:1 ;
	uint		nodel:1 ;
} ;

struct locinfo {
	PROGINFO	*pip ;
	cchar		*ofname ;
	LOCINFO_FL	have, f, final, changed ;
	LOCINFO_FL	open ;
	SHIO		ofile ;
	VECSTR		stores ;
	VECSTR		mailusers ;
	int		linelen ;
	int		nshow ;
	int		msgto ;
} ;

struct config {
	PROGINFO	*pip ;
	PARAMOPT	*app ;
	PARAMFILE	p ;
	EXPCOOK		cooks ;
	uint		f_p:1 ;
	uint		f_cooks:1 ;
} ;


/* forward references */

static int	mainsub(int,cchar *[],cchar *[],void *) ;

static int	usage(PROGINFO *) ;

static int	procuser_begin(PROGINFO *) ;
static int	procuser_end(PROGINFO *) ;
static int	procourconf_begin(PROGINFO *,PARAMOPT *,cchar *) ;
static int	procourconf_end(PROGINFO *) ;
static int	procout_begin(PROGINFO *,cchar *) ;
static int	procout_end(PROGINFO *) ;
static int	procout_beginer(PROGINFO *) ;
static int	procout_printf(PROGINFO *,cchar *,...) ;

static int	procopts(PROGINFO *,KEYOPT *,PARAMOPT *) ;
static int	procmailusers(PROGINFO *,PARAMOPT *) ;
static int	procmailusers_all(PROGINFO *) ;
static int	procmailusers_env(PROGINFO *,cchar *) ;
static int	procmailusers_arg(PROGINFO *,PARAMOPT *) ;
static int	procmailusers_def(PROGINFO *) ;
static int	procmailusers_add(PROGINFO *,cchar *,int) ;

static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procmailbox(PROGINFO *,cchar *,int) ;
static int	procmailboxes(PROGINFO *,cchar *,int) ;
static int	procmailboxone(PROGINFO *,cchar *,cchar *,int) ;
static int	procmailmsg(PROGINFO *,MBCACHE *,int,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#if	CF_LOCPRINTF
static int	locinfo_eprintf(LOCINFO *,cchar *,...) ;
#endif

static int	config_start(CONFIG *,PROGINFO *,PARAMOPT *,cchar *) ;
static int	config_findfile(CONFIG *,char *,cchar *) ;
static int	config_cookbegin(CONFIG *) ;
static int	config_cookend(CONFIG *) ;
static int	config_read(CONFIG *) ;
static int	config_reader(CONFIG *) ;
static int	config_finish(CONFIG *) ;
static int	config_setlfname(CONFIG *,cchar *,int) ;

#if	CF_CONFIGCHECK
static int	config_check(CONFIG *) ;
#endif /* CF_CONFIGCHECK */


/* local variables */

static const char	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"HELP",
	"ROOT",
	"sn",
	"af",
	"ef",
	"of",
	"cf",
	"to",
	"md",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_root,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_cf,
	argopt_to,
	argopt_md,
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
	"md",
	"sort",
	"nshow",
	"msgto",
	"allusers",
	"nodel",
	NULL
} ;

enum akonames {
	akoname_md,
	akoname_sort,
	akoname_nshow,
	akoname_msgto,
	akoname_allusers,
	akoname_nodel,
	akoname_overlast
} ;

static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

static const char	*cparams[] = {
	"maildir",
	"logsize",
	"logfile",
	NULL
} ;

enum cparams {
	cparam_maildir,
	cparam_logsize,
	cparam_logfile,
	cparam_overlast
} ;

static const char	*varmailusers[] = {
	VARMAILUSERSP,
	VARMAILUSERS,
	NULL
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


/* exported subroutines */


int b_mailexpire(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_mailexpire) */


int p_mailexpire(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_mailexpire) */


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
	int		rs = SR_OK ;
	int		rs1 ;
	int		v ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*sn = NULL ;
	cchar		*pr = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*cfname = NULL ;
	cchar		*cp ;

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

	if ((cp = getourenv(pip->envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->daytime = time(NULL) ;

	pip->f.logprog = TRUE ;

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

/* configuration file-name */
	                case argopt_cf:
	                    cp = NULL ;
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
	                    if (cp != NULL) {
	                        pip->have.cfname = TRUE ;
	                        pip->final.cfname = TRUE ;
	                        cfname = cp ;
	                    }
	                    break ;

/* time-out */
	                case argopt_to:
	                    cp = NULL ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                            cl = avl ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                cp = argp ;
	                                cl = argl ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    if ((rs >= 0) && (cp != NULL)) {
	                        lip->final.msgto = TRUE ;
	                        rs = cfdecti(cp,cl,&v) ;
	                        lip->msgto = v ;
	                    }
	                    break ;

/* mail directory */
	                case argopt_md:
	                    cp = NULL ;
	                    cl = -1 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                            cl = avl ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                cp = argp ;
	                                cl = argl ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    if ((rs >= 0) && (cp != NULL)) {
	                        cchar	*po = PO_MAILDIRS ;
	                        lip->final.maildirs = TRUE ;
	                        lip->have.maildirs = TRUE ;
	                        rs = paramopt_loads(&aparams,po,cp,cl) ;
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

	                    case 'C':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->have.cfname = TRUE ;
	                                pip->final.cfname = TRUE ;
	                                cfname = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'a':
	                        lip->f.allusers = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.allusers = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'n':
	                        lip->f.nodel = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.nodel = (rs > 0) ;
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

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->final.msgto = TRUE ;
	                                rs = cfdecti(argp,argl,&v) ;
	                                lip->msgto = v ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* alternate users */
	                    case 'u':
	                        cp = NULL ;
	                        cl = 0 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                cp = avp ;
	                                cl = avl ;
	                            }
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl) {
	                                    cp = argp ;
	                                    cl = argl ;
	                                }
	                            } else
	                                rs = SR_INVALID ;
	                        }
	                        if ((rs >= 0) && (cp != NULL)) {
				    PARAMOPT	*pop = &aparams ;
	                            cchar	*po = PO_MAILUSERS ;
	                            rs = paramopt_loads(pop,po,cp,cl) ;
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

	        } /* end if (digits or options) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

#if	CF_DEBUGS
	debugprintf("main: args-out rs=%d\n",rs) ;
#endif

	if (efname == NULL) efname = getourenv(pip->envv,VAREFNAME) ;
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
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
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
	} /* end if */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some initialization */

	if (afname == NULL) afname = getourenv(pip->envv,VARAFNAME) ;

	if (cfname == NULL) cfname = getourenv(pip->envv,VARCFNAME) ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getourenv(pip->envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if ((rs >= 0) && (lip->msgto <= 0) && (argval != NULL)) {
	    lip->final.msgto = TRUE ;
	    rs = cfdecti(argval,-1,&v) ;
	    lip->msgto = v ;
	}

	if (rs >= 0) {
	    rs = procopts(pip,&akopts,&aparams) ;
	}

	if ((rs >= 0) && (lip->msgto <= 0)) {
	    if ((cp = getourenv(pip->envv,VARMSGTO)) != NULL) {
	        rs = cfdecti(cp,-1,&v) ;
	        lip->msgto = v ;
	    }
	}

	if ((rs >= 0) && (lip->msgto <= 0)) {
	    lip->msgto = TO_MSGEXPIRE ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_mailexpire: def msgto=%d\n",lip->msgto) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: msgto=%d\n",pip->progname,lip->msgto) ;
	}

/* open the output file */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs == procuser_begin(pip)) >= 0) {
	        if (cfname != NULL) {
	            if (pip->euid != pip->uid) u_seteuid(pip->uid) ;
	            if (pip->egid != pip->gid) u_setegid(pip->gid) ;
	        }
	        if ((rs = procourconf_begin(pip,&aparams,cfname)) >= 0) {
	            if ((rs = procmailusers(pip,&aparams)) >= 0) {
	                ARGINFO	*aip = &ainfo ;
	                cchar	*ofn = ofname ;
	                cchar	*afn = afname ;
	                rs = procargs(pip,aip,&pargs,ofn,afn) ;
	            } /* end if (procmailusers) */
	            rs1 = procourconf_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procourconf) */
	        rs1 = procuser_end(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (procuser) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_INTR:
	        ex = EX_INTR ;
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
	if ((pip->debuglevel > 0) && (pip->efp != NULL)) {
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

	fmt = "%s: USAGE> %s [<mbox(es)>[=<age>]] [-af <afile>] [-t <age>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-md <maildir(s)>] [-u <user(s)>] [-a]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop,PARAMOPT *app)
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
	                int	v ;

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {

	                case akoname_md:
	                    if (! lip->final.maildirs) {
	                        if (vl > 0) {
	                            cchar	*po = PO_MAILDIRS ;
	                            lip->final.maildirs = TRUE ;
	                            lip->have.maildirs = TRUE ;
	                            rs = paramopt_loads(app,po,vp,vl) ;
	                        }
	                    }
	                    break ;

	                case akoname_sort:
	                    if (! lip->final.sort) {
	                        lip->final.sort = TRUE ;
	                        lip->have.sort = TRUE ;
	                        lip->f.sort = TRUE ;
	                        if (vl > 0) {
	                            int	ch = tolc(*vp & 0xff) ;
	                            switch (ch) {
	                            case '0':
	                            case 'n':
	                                lip->f.sort = FALSE ;
	                                break ;
	                            case 'r':
	                                lip->f.sortrev = TRUE ;
	                                break ;
	                            } /* end switch */
	                        }
	                    }
	                    break ;

	                case akoname_nshow:
	                    if (! lip->final.nshow) {
	                        if (vl) {
	                            lip->final.nshow = TRUE ;
	                            lip->have.nshow = TRUE ;
	                            rs = optvalue(vp,vl) ;
	                            lip->nshow = rs ;
	                        }
	                    }
	                    break ;

	                case akoname_msgto:
	                    if (! lip->final.msgto) {
	                        if (vl) {
	                            lip->final.msgto = TRUE ;
	                            lip->have.msgto = TRUE ;
	                            rs = cfdecti(vp,vl,&v) ;
	                            lip->msgto = v ;
	                        }
	                    }
	                    break ;

	                case akoname_allusers:
	                    if (! lip->final.allusers) {
	                        lip->final.allusers = TRUE ;
	                        lip->have.allusers = TRUE ;
	                        lip->f.allusers = TRUE ;
	                        if (vl) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.allusers = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_nodel:
	                    if (! lip->final.nodel) {
	                        lip->final.nodel = TRUE ;
	                        lip->have.nodel = TRUE ;
	                        lip->f.nodel = TRUE ;
	                        if (vl) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.nodel = (rs > 0) ;
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


static int procuser_begin(PROGINFO *pip)
{
	const int	ulen = USERNAMELEN ;
	int		rs = SR_OK ;
	char		ubuf[USERNAMELEN+1] ;

	if ((rs = ids_load(&pip->id)) >= 0) {
	    pip->uid = pip->id.uid ;
	    pip->euid = pip->id.euid ;
	    pip->gid = pip->id.gid ;
	    pip->egid = pip->id.egid ;
	    if ((rs = getusername(ubuf,ulen,-1)) >= 0) {
	        cchar	**vpp = &pip->username ;
	        if ((rs = proginfo_setentry(pip,vpp,ubuf,rs)) >= 0) {
	            char	nn[NODENAMELEN+1] ;
	            char	dn[MAXHOSTNAMELEN+1] ;
	            if ((rs = getnodedomain(nn,dn)) >= 0) {
	                const int	hlen = MAXHOSTNAMELEN ;
	                char		hbuf[MAXHOSTNAMELEN+1] ;
	                if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	                    int		vl = rs ;
	                    int		i ;
	                    cchar	*vp ;
	                    cchar	**vpp = NULL ;
	                    for (i = 0 ; i < 3 ; i += 1) {
	                        switch (i) {
	                        case 0:
	                            vpp = &pip->hostname ;
	                            vp = hbuf ;
	                            break ;
	                        case 1:
	                            vpp = &pip->nodename ;
	                            vp = nn ;
	                            break ;
	                        case 2:
	                            vpp = &pip->domainname ;
	                            vp = dn ;
	                            break ;
	                        } /* end switch */
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                        vl = -1 ;
	                        if (rs < 0) break ;
	                    } /* end for */
	                } /* end if (snsds) */
	            } /* end if (getnodedomain) */
	        } /* end if (set-entry) */
	    } /* end if (getusername) */
	} /* end if (ids_load) */

	return rs ;
}
/* end subroutine (procuser_begin) */


static int procuser_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

	rs1 = ids_release(&pip->id) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (procuser_end) */


static int procourconf_begin(PROGINFO *pip,PARAMOPT *app,cchar cfname[])
{
	const int	csize = sizeof(CONFIG) ;
	int		rs = SR_OK ;
	void		*p ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_mbproc/procourconf_begin: ent\n") ;
#endif

	if ((rs = uc_malloc(csize,&p)) >= 0) {
	    CONFIG	*csp = p ;
	    pip->config = csp ;
	    if ((rs = config_start(csp,pip,app,cfname)) >= 0) {
	        if ((rs = config_read(csp)) >= 0) {
	            rs = 1 ;
	        }
	        if (rs < 0)
	            config_finish(csp) ;
	    } /* end if (config) */
	    if (rs < 0) {
	        uc_free(p) ;
	        pip->config = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_mbproc/procourconf_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procourconf_begin) */


static int procourconf_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_mailexpire/procourconf_end: ent config=%u\n",
	        (pip->config != NULL)) ;
#endif

	if (pip->config != NULL) {
	    CONFIG	*csp = pip->config ;
	    rs1 = config_finish(csp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(pip->config) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->config = NULL ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_mailexpire/procourconf_end: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procourconf_end) */


static int procout_begin(PROGINFO *pip,cchar *ofn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if (pip->verboselevel >= 1) {
	    cchar	**vpp = &lip->ofname ;

	    if ((ofn == NULL) || (ofn [0] == '\0') || (ofn[0] == '-'))
	        ofn = STDOUTFNAME ;

	    rs = locinfo_setentry(lip,vpp,ofn,-1) ;

	} /* end if (opening the output file) */

	return rs ;
}
/* end subroutine (procout_begin) */


static int procout_end(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->open.ofile) {
	    rs1 = shio_close(&lip->ofile) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->open.ofile = FALSE ;
	}

	return rs ;
}
/* end subroutine (procout_end) */


static int procout_beginer(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if ((! lip->open.ofile) && (lip->ofname != NULL)) {
	    cchar	*ofname = lip->ofname ;
	    if ((rs = shio_open(&lip->ofile,ofname,"wct",0666)) >= 0) {
	        lip->open.ofile = TRUE ;
	    }
	}

	return rs ;
}
/* end subroutine (procout_beginner) */


static int procout_printf(PROGINFO *pip,cchar *fmt,...)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if ((pip->verboselevel >= 1) && (lip->ofname != NULL)) {
	    if ((rs = procout_beginer(pip)) >= 0) {
	        va_list	ap ;
	        va_begin(ap,fmt) ;
	        rs = shio_vprintf(&lip->ofile,fmt,ap) ;
	        va_end(ap) ;
	    } /* end if (active) */
	} /* end if (verboselevel) */

	return rs ;
}
/* end subroutine (procout_printf) */


static int procmailusers(PROGINFO *pip,PARAMOPT *app)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailexpire/procmailusers: ent\n") ;
#endif

	if (lip->f.allusers) {
	    rs = procmailusers_all(pip) ;
	    c += rs ;
	} else {
	    if ((rs = procmailusers_arg(pip,app)) >= 0) {
	        c += rs ;
	        if (c == 0) {
		    cchar	**vm = varmailusers ;
	            int		i ;
	            for (i = 0 ; vm[i] != NULL ; i += 1) {
	                rs = procmailusers_env(pip,varmailusers[i]) ;
			c += rs ;
			if (rs < 0) break ;
	    	    } /* end for */
		} /* end if (more) */
		if (rs >= 0) {
	    	    rs = procmailusers_def(pip) ;
	    	    c += rs ;
		}
	    } /* end if (procmailusers_arg) */
	} /* end if (which users) */
	
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailexpire/procmailusers: mid3 rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    vecstr	*mlp = &lip->mailusers ;
	    int		i ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: mailuser=%s\n" ;
	    cchar	*cp ;
	    for (i = 0 ; vecstr_get(mlp,i,&cp) >= 0 ; i += 1) {
	        if (cp != NULL) {
	            shio_printf(pip->efp,fmt,pn,cp) ;
		}
	    } /* end for */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailexpire/procmailusers: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers) */


static int procmailusers_all(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	SYSUSERNAMES	su ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = sysusernames_open(&su,NULL)) >= 0) {
	    vecstr	*mlp = &lip->mailusers ;
	    const int	ulen = USERNAMELEN ;
	    char	ubuf[USERNAMELEN+1] ;
	    while ((rs = sysusernames_readent(&su,ubuf,ulen)) > 0) {
	        c += 1 ;
	        rs = vecstr_add(mlp,ubuf,rs) ;
	        if (rs < 0) break ;
	    } /* end while */
	    rs1 = sysusernames_close(&su) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sysusernames) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers_all) */


static int procmailusers_env(PROGINFO *pip,cchar *var)
{
	LOCINFO		*lip = pip->lip ;
	VECSTR		*vlp ;
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*sp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailexpire/procmailusers_env: v=%s\n",var) ;
#endif

	vlp = &lip->mailusers ;
	if ((vlp == NULL) || (var == NULL))
	    return SR_FAULT ;

	if ((sp = getourenv(pip->envv,var)) != NULL) {
	    int		sl = strlen(sp) ;
	    int		cl ;
	    cchar	*tp, *cp ;

	    while ((tp = strnpbrk(sp,sl," :,\t\n")) != NULL) {

	        if ((cl = sfshrink(sp,(tp - sp),&cp)) > 0) {
	            if (cl > USERNAMELEN) cl = USERNAMELEN ;
	            rs = procmailusers_add(pip,cp,cl) ;
	            c += rs ;
	        }

	        sl -= ((tp + 1) - sp) ;
	        sp = (tp + 1) ;

	        if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (sl > 0)) {
	        if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	            rs = procmailusers_add(pip,cp,cl) ;
	            c += rs ;
	        }
	    } /* end if */

	} /* end if (getourenv) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers_env) */


static int procmailusers_arg(PROGINFO *pip,PARAMOPT *app)
{
	int		rs ;
	int		c = 0 ;
	cchar		*po = PO_MAILUSERS ;

	if ((rs = paramopt_havekey(app,po)) > 0) {
	    PARAMOPT_CUR	cur ;
	    int			cl ;
	    cchar		*cp ;

	    if ((rs = paramopt_curbegin(app,&cur)) >= 0) {

	        while ((cl = paramopt_enumvalues(app,po,&cur,&cp)) >= 0) {
	            if (cp != NULL) {
	                if ((cp[0] == '-') || (cp[0] == '+')) {
	                    cp = pip->username ;
	                    cl = -1 ;
	                }
	                rs = procmailusers_add(pip,cp,cl) ;
	                c += rs ;
		    }
	            if (rs < 0) break ;
	        } /* end while */

	        paramopt_curend(app,&cur) ;
	    } /* end if (paramopt-cur) */

	} /* end if (paramopt_havekey) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers_arg) */


static int procmailusers_def(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	vecstr		*mlp ;
	int		rs ;
	int		c = 0 ;
	int		f = TRUE ;
	cchar		*username = pip->username ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailexpire/procmailusers_def: ent\n") ;
#endif

	mlp = &lip->mailusers ;
	if ((rs = vecstr_count(mlp)) > 0) {
	    int		i ;
	    cchar	*cp ;
	    c = rs ;

	    f = FALSE ;
	    for (i = 0 ; vecstr_get(mlp,i,&cp) >= 0 ; i += 1) {
	        if (cp != NULL) {
	            if (strcmp(cp,"+") == 0) {
	                f = TRUE ;
	                c -= 1 ;
	                vecstr_del(mlp,i) ;
	            } else if (strcmp(cp,"-") == 0) {
	                c -= 1 ;
	                vecstr_del(mlp,i) ;
	            } /* end if */
		}
	    } /* end for */

	} /* end if (non-zero) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailexpire/procmailusers_def: mid1 rs=%d c=%u\n",
	        rs,c) ;
#endif

	if ((rs >= 0) && (f || (c == 0))) {
	    rs = procmailusers_add(pip,username,-1) ;
	    c += rs ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailexpire/procmailusers_def: ret rs=%d c=%u\n",
	        rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers_def) */


static int procmailusers_add(PROGINFO *pip,cchar *dp,int dl)
{
	LOCINFO		*lip = pip->lip ;
	vecstr		*mlp = &lip->mailusers ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (dp == NULL) return SR_FAULT ;

	if (dl < 0) dl = strlen(dp) ;

	if ((dl > 0) && (dp[0] != '\0')) {
	    if ((rs = vecstr_findn(mlp,dp,dl)) == SR_NOTFOUND) {
	        c += 1 ;
	        rs = vecstr_add(mlp,dp,dl) ;
	    } /* end if (not already) */
	} /* end if (non-zero) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers_add) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((rs = procout_begin(pip,ofn)) >= 0) {
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
	                    rs = procmailbox(pip,cp,-1) ;
	                    c += rs ;
	                }
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end for (looping through requested circuits) */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        SHIO	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	        if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	                    lbuf[(cp-lbuf)+cl] = '\0' ;
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procmailboxes(pip,cp,cl) ;
	                        c += rs ;
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

	    } /* end if (processing file argument file list) */

	    if ((rs >= 0) && (pan == 0)) {

	        cp = DEFMBOX ;
	        pan += 1 ;
	        rs = procmailbox(pip,cp,-1) ;
	        c += rs ;

	    } /* end if (standard input) */

	    rs1 = procout_end(pip) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    if (! pip->f.quiet) {
	        fmt = "%s: inaccessible output (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	    }
	} /* end if (ofile) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


static int procmailboxes(PROGINFO *pip,cchar *sp,int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		fl ;
	    cchar	*fp ;

	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procmailbox(pip,fp,fl) ;
	            c += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */

	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailboxes) */


static int procmailbox(PROGINFO *pip,cchar *mbp,int mbl)
{
	LOCINFO		*lip = pip->lip ;
	IDS		*idp = &pip->id ;
	int		rs = SR_OK ;
	int		ml = mbl ;
	int		to ;
	int		c = 0 ;
	cchar		*folder = FOLDER ;
	cchar		*tp ;
	cchar		*mp = mbp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_mailexpire/procmailbox: mbox=%t\n",mbp,mbl) ;
#endif

	to = lip->msgto ;
	if ((tp = strnchr(mbp,mbl,'=')) != NULL) {
	    ml = (tp-mbp) ;
	    rs = cfdecti((tp+1),-1,&to) ;
	}

	if ((rs >= 0) && (pip->verboselevel >= 2)) {
	    procout_printf(pip,"mb=%t to=%u\n",mp,ml,to) ;
	}

	if (rs >= 0) {
	    VECSTR	*ulp = &lip->mailusers ;
	    const int	am = (R_OK|W_OK) ;
	    const int	hlen = MAXPATHLEN ;
	    int		i ;
	    cchar	*mup ;
	    char	hbuf[MAXPATHLEN+1] ;
	    for (i = 0 ; vecstr_get(ulp,i,&mup) >= 0 ; i += 1) {
	        if (mup != NULL) {
	            if ((rs = getuserhome(hbuf,hlen,mup)) >= 0) {
	                char	mbuf[MAXPATHLEN+1] ;
	                if ((rs = mkpath3w(mbuf,hbuf,folder,mp,ml)) > 0) {
	                    struct ustat	sb ;
	                    if (u_stat(mbuf,&sb) >= 0) {
	                        if ((rs = sperm(idp,&sb,am)) >= 0) {
	                            rs = procmailboxone(pip,mup,mbuf,to) ;
	                            c += rs ;
	                        } else if (isNotPresent(rs))
	                            rs = SR_OK ;
	                    } /* end if (stat) */
	                } /* end if (mkpath) */
	            } /* end if (getuserhome) */
		}
	        if (rs < 0) break ;
	    } /* end for (mailusers) */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_mailexpire/procmailbox: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailbox) */


static int procmailboxone(PROGINFO *pip,cchar *un,cchar *mfname,int to)
{
	MAILBOX		mb ;
	int		rs ;
	int		rs1 ;
	int		mbopts ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_mailnew/procmailboxone: ent mfname=%s\n",mfname) ;
	    debugprintf("b_mailnew/procmailboxone: to=%d\n",to) ;
	}
#endif

	mbopts = MAILBOX_ORDWR ;
	if ((rs = mailbox_open(&mb,mfname,mbopts)) >= 0) {
	    MBCACHE	mc ;
	    const int	mbflags = MBCACHE_ORDWR ;
	    if ((rs = mbcache_start(&mc,mfname,mbflags,&mb)) >= 0) {
	        if ((rs = mbcache_count(&mc)) > 0) {
	            const int	mn = rs ;
	            int		mi ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_mailnew/procmailboxone: mn=%u\n",mn) ;
#endif

	            for (mi = 0 ; mi < mn ; mi += 1) {

	                rs = procmailmsg(pip,&mc,mi,to) ;
	                c += rs ;

	                if (rs < 0) break ;
	            } /* end for */

	        } /* end if (n-mesgs) */
	        rs1 = mbcache_finish(&mc) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mbcache) */
	    rs1 = mailbox_close(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_mailnew/procmailboxone: mailbox() rs=%d\n",rs) ;
#endif
	    if (isNotPresent(rs)) rs = SR_OK ;
	}

	if (rs >= 0) {
	    int		bl ;
	    cchar	*bp ;
	    cchar	*fmt ;
	    cchar	*pn = pip->progname ;
	    if ((bl = sfbasename(mfname,-1,&bp)) > 0) {
	        if (pip->debuglevel > 0) {
	            fmt = "%s: u=%s mb=%t to=%u deleted msgs=%u\n" ;
	            shio_printf(pip->efp,fmt,pn,un,bp,bl,to,c) ;
	        }
	        if (pip->verboselevel >= 2) {
	            fmt = "  u=%-8s deleted msgs=%u\n" ;
	            procout_printf(pip,fmt,un,c) ;
	        }
	    } /* end if (sfbasename) */
	} /* end if (print summary) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailnew/procmailboxone: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailboxone) */


static int procmailmsg(PROGINFO *pip,MBCACHE *mcp,int mi,int to)
{
	LOCINFO		*lip = pip->lip ;
	time_t		times[2] ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_mailnew/procmailmsg: mi=%u\n",mi) ;
	    debugprintf("b_mailnew/procmailmsg: to=%d\n",to) ;
	}
#endif

	if ((rs = mbcache_msgtimes(mcp,mi,times)) >= 0) {
	    time_t	dt = pip->daytime ;
	    time_t	t = 0 ;
	    if (t == 0) t = times[1] ; /* HDR */
	    if (t == 0) t = times[0] ; /* ENV */
	    if ((dt-t) >= to) {
	        c += 1 ;
	        if (! lip->f.nodel) {
	            rs = mbcache_msgdel(mcp,mi,TRUE) ;
	        }
	    }
	} /* end if (mbcache_msgtime) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailnew/procmailmsg: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailmsg) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;

	if ((rs = vecstr_start(&lip->mailusers,5,0)) >= 0) {
	    lip->open.mailusers = TRUE ;
	}

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->open.mailusers) {
	    lip->open.mailusers = FALSE ;
	    rs1 = vecstr_finish(&lip->mailusers) ;
	    if (rs >= 0) rs = rs1 ;
	}

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


#if	CF_LOCPRINTF
static int locinfo_eprintf(LOCINFO *lip,cchar *fmt,...)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (pip->debuglevel > 0) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = shio_vprintf(pip->efp,fmt,ap) ;
	    va_end(ap) ;
	}
	return rs ;
}
/* end subroutine (locinfo_eprintf) */
#endif /* CF_LOCPRINTF */


/* configuration maintenance */
static int config_start(CONFIG *csp,PROGINFO *pip,PARAMOPT *app,cchar *cfn)
{
	int		rs = SR_OK ;
	char		tmpfname[MAXPATHLEN+1] = { 0 } ;

	if (csp == NULL) return SR_FAULT ;

	if (cfn == NULL) cfn = CONFIGFNAME ;

	memset(csp,0,sizeof(CONFIG)) ;
	csp->pip = pip ;
	csp->app = app ;

	if (strchr(cfn,'/') == NULL) {
	    rs = config_findfile(csp,tmpfname,cfn) ;
	    if (rs > 0) cfn = tmpfname ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: mid rs=%d cfn=%s\n",rs,cfn) ;
#endif

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp,"%s: conf=%s\n",
	        pip->progname,cfn) ;
	}

	if (rs >= 0) {
	    if ((rs = paramfile_open(&csp->p,pip->envv,cfn)) >= 0) {
	        if ((rs = config_cookbegin(csp)) >= 0) {
	            csp->f_p = (rs >= 0) ;
	        }
	        if (rs < 0)
	            paramfile_close(&csp->p) ;
	    } else if (isNotPresent(rs))
	        rs = SR_OK ;
	} else if (isNotPresent(rs))
	    rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_start) */


static int config_findfile(CONFIG *op,char tbuf[],cchar cfname[])
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;
	int		tl = 0 ;

	tbuf[0] = '\0' ;
	if ((cfname != NULL) && (cfname[0] != '\0')) {
	    VECSTR	sv ;
	    if ((rs = vecstr_start(&sv,6,0)) >= 0) {
	        const int	tlen = MAXPATHLEN ;
	        int		i ;
	        int		vl ;
	        int		kch ;
	        cchar		*ks = "pen" ;
	        cchar		*vp ;
	        char		kbuf[2] ;
	        kbuf[1] = '\0' ;
	        for (i = 0 ; (rs >= 0) && (ks[i] != '\0') ; i += 1) {
	            kch = (ks[i] & 0xff) ;
	            kbuf[0] = kch ;
	            vp = NULL ;
	            vl = -1 ;
	            switch (kch) {
	            case 'p':
	                vp = pip->pr ;
	                break ;
	            case 'e':
	                vp = "etc" ;
	                break ;
	            case 'n':
	                vp = pip->searchname ;
	                break ;
	            } /* end switch */
	            if (rs >= 0) {
	                rs = vecstr_envset(&sv,kbuf,vp,vl) ;
	            }
	        } /* end for */
	        if (rs >= 0) {
	            rs = permsched(sched1,&sv,tbuf,tlen,cfname,R_OK) ;
	            tl = rs ;
	        }
	        vecstr_finish(&sv) ;
	    } /* end if (finding file) */
	} /* end if (non-null) */

	return (rs >= 0) ? tl : rs ;
}
/* end subroutine (config_findfile) */


static int config_cookbegin(CONFIG *op)
{
	PROGINFO	*pip = op->pip ;
	int		rs ;

	if ((rs = expcook_start(&op->cooks)) >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    int		i ;
	    int		kch ;
	    int		vl ;
	    cchar	*ks = "PSNDHRU" ;
	    cchar	*vp ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    char	kbuf[2] ;

	    kbuf[1] = '\0' ;
	    for (i = 0 ; (rs >= 0) && (ks[i] != '\0') ; i += 1) {
	        kch = MKCHAR(ks[i]) ;
	        vp = NULL ;
	        vl = -1 ;
	        switch (kch) {
	        case 'P':
	            vp = pip->progname ;
	            break ;
	        case 'S':
	            vp = pip->searchname ;
	            break ;
	        case 'N':
	            vp = pip->nodename ;
	            break ;
	        case 'D':
	            vp = pip->domainname ;
	            break ;
	        case 'H':
	            {
	                cchar	*nn = pip->nodename ;
	                cchar	*dn = pip->domainname ;
	                rs = snsds(hbuf,hlen,nn,dn) ;
	                vl = rs ;
	                if (rs >= 0) vp = hbuf ;
	            }
	            break ;
	        case 'R':
	            vp = pip->pr ;
	            break ;
	        case 'U':
	            vp = pip->username ;
	            break ;
	        } /* end switch */
	        if ((rs >= 0) && (vp != NULL)) {
	            kbuf[0] = kch ;
	            rs = expcook_add(&op->cooks,kbuf,vp,vl) ;
	        }
	    } /* end for */

	    if (rs >= 0) {
	        op->f_cooks = TRUE ;
	    } else
	        expcook_finish(&op->cooks) ;
	} /* end if (expcook_start) */

	return rs ;
}
/* end subroutine (config_cookbegin) */


static int config_cookend(CONFIG *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f_cooks) {
	    op->f_cooks = FALSE ;
	    rs1 = expcook_finish(&op->cooks) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (config_cookend) */


static int config_finish(CONFIG *csp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (csp == NULL) return SR_FAULT ;

	if (csp->f_p) {

	    rs1 = config_cookend(csp) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = paramfile_close(&csp->p) ;
	    if (rs >= 0) rs = rs1 ;

	    csp->f_p = FALSE ;
	} /* end if */

	return rs ;
}
/* end subroutine (config_finish) */


#if	CF_CONFIGCHECK
static int config_check(CONFIG *op)
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;

	if (op->f_p) {
	    if ((rs = paramfile_check(&op->p,pip->daytime)) > 0) {
	        rs = config_read(op) ;
	    }
	}

	return rs ;
}
/* end subroutine (config_check) */
#endif /* CF_CONFIGCHECK */


static int config_read(CONFIG *op)
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: f_p=%u\n",op->f_p) ;
#endif

	if (op->f_p) {
	    rs = config_reader(op) ;
	}

	return rs ;
}
/* end subroutine (config_read) */


static int config_reader(CONFIG *op)
{
	PROGINFO	*pip = op->pip ;
	LOCINFO		*lip ;
	PARAMFILE	*pfp ;
	PARAMFILE_CUR	cur ;
	PARAMOPT	*pop = op->app ;
	const int	vlen = VBUFLEN ;
	const int	elen = EBUFLEN ;
	int		rs = SR_OK ;
	int		i ;
	int		vl, el ;
	int		v ;
	char		vbuf[VBUFLEN + 1] ;
	char		ebuf[EBUFLEN + 1] ;
	pfp = &op->p ;
	lip = pip->lip ;
	for (i = 0 ; cparams[i] != NULL ; i += 1) {

	    if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {

	        while (rs >= 0) {
	            vl = paramfile_fetch(pfp,cparams[i],&cur,vbuf,vlen) ;
	            if (vl == SR_NOTFOUND) break ;
	            rs = vl ;
	            if (rs < 0) break ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("config_read: vbuf=>%t<\n",vbuf,vl) ;
#endif

	            ebuf[0] = '\0' ;
	            el = 0 ;
	            if (vl > 0) {
	                el = expcook_exp(&op->cooks,0,ebuf,elen,vbuf,vl) ;
	                if (el >= 0) ebuf[el] = '\0' ;
	            }

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("config_read: ebuf=>%t<\n",ebuf,el) ;
#endif

	            switch (i) {

	            case cparam_maildir:
	                if (! lip->final.maildirs) {
	                    if (el > 0) {
	                        cchar	*po = PO_MAILDIRS ;
	                        lip->final.maildirs = TRUE ;
	                        lip->have.maildirs = TRUE ;
	                        rs = paramopt_loads(pop,po,ebuf,el) ;
	                    }
	                }
	                break ;

	            case cparam_logsize:
	                if (el > 0) {
	                    if ((el == 1) && (ebuf[0] == '+')) {
	                        pip->logsize = LOGSIZE ;
	                    } else if ((rs = cfdecmfi(ebuf,el,&v)) >= 0) {
	                        if (v >= 0) {
	                            pip->logsize = v ;
	                        }
	                    } /* end if (valid number) */
	                } /* end if (non-zero) */
	                break ;

	            case cparam_logfile:
	                if (el > 0) {
	                    if (! pip->final.lfname) {
	                        pip->final.lfname = TRUE ;
	                        pip->have.lfname = TRUE ;
	                        rs = config_setlfname(op,ebuf,el) ;
	                    }
	                } /* end if (non-zero) */
	                break ;

	            } /* end switch */

	        } /* end while (fetching) */

	        paramfile_curend(pfp,&cur) ;
	    } /* end if (cursor) */

	    if (rs < 0) break ;
	} /* end for (parameters) */
	return rs ;
}
/* end subroutine (config_reader) */


static int config_setlfname(CONFIG *cfp,cchar *vp,int vl)
{
	PROGINFO	*pip = cfp->pip ;
	cchar		*pr ;
	cchar		*sn ;
	cchar		*lfn ;
	int		rs = SR_OK ;
	int		tl ;
	char		tbuf[MAXPATHLEN+1] ;

	pr = pip->pr ;
	sn = pip->searchname ;
	lfn = pip->lfname ;
	tl = prsetfname(pr,tbuf,vp,vl,TRUE,LOGCNAME,sn,"") ;

	if ((lfn == NULL) || (strcmp(lfn,tbuf) != 0)) {
	    cchar	**vpp = &pip->lfname ;
	    pip->changed.lfname = TRUE ;
	    rs = proginfo_setentry(pip,vpp,tbuf,tl) ;
	}

	return rs ;
}
/* end subroutine (config_setlfname) */


