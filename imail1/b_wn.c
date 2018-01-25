/* b_wn */

/* SHELL built-in similar to 'who(1)' */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_FULLNAME	0		/* use fullname? */
#define	CF_DOTUSER	0		/* allow "dot" users? */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was written as a KSH builtin.  Of course, it is inspired by
	'who(1)'.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  It should also be able to
	be made into a stand-alone program without much (if almost any)
	difficulty, but I have not done that yet.

	This built-in is pretty straight forward.  We used the supplied UNIX®
	System subroutines for accessing UTMPX because it is more portable and
	speed is not paramount, like it would be if we were continuously
	scanning the UTMPX DB for changed events.  In short, the system UTMPX
	subroutines are fast enough for a one-shot scan through the UTMPX DB
	like we are doing here.

	One interesting thing to note is that we do maintain a cache of the
	username-to-realname translations.  Even though we are only scanning
	the UTMPX DB once, there may be several repeat logins by the same
	username.  Using the cache, we bypass asking the system for every
	username for those usernames that we've seen before.  I think that the
	cost of maintaining the cache is not as bad as asking the system for
	every username.  Your mileage may vary!

	Synopsis:

	$ wn [-l] [-u] [-h] [-nh] [<username(s)> ...]


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
#include	<netdb.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<osetstr.h>
#include	<hdbstr.h>
#include	<tmpx.h>
#include	<realname.h>
#include	<field.h>
#include	<getxusername.h>
#include	<grmems.h>
#include	<sysrealname.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_wn.h"
#include	"defs.h"
#include	"namecache.h"


/* local defines */

#ifndef	DEVDNAME
#define	DEVDNAME	"/dev"
#endif

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	USERNAMELEN
#define	USERNAMELEN	32
#endif

#define	COLS_USERNAME	8
#define	COLS_REALNAME	39

#define	MAXOUT(f)	if ((f) > 99.9) (f) = 99.9

#define	LOCINFO			struct locinfo
#define	LOCINFO_FL		struct locinfo_flags
#define	LOCINFO_GMCUR		struct locinfo_gmcur
#define	LOCINFO_RNCUR		struct locinfo_rncur
#define	LOCINFO_CACHESTATS	NAMECACHE_STATS


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath2w(char *,cchar *,cchar *,int) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	ctdeci(cchar *,int,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	mkgecosname(char *,int,cchar *) ;
extern int	termwritable(cchar *) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	hasMeAlone(cchar *,int) ;
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

struct ustats {
	uint		total ;
	uint		cachehits ;
} ;

struct locinfo_gmcur {
	GRMEMS_CUR	gmcur ;
} ;

struct locinfo_rncur {
	SYSREALNAME_CUR	rncur ;
} ;

struct locinfo_flags {
	uint		hdr:1 ;
	uint		linebuf:1 ;
	uint		fmtlong:1 ;
	uint		fmtshort:1 ;
	uint		fmtline:1 ;
	uint		uniq:1 ;
	uint		users:1 ;
	uint		msg:1 ;
	uint		biff:1 ;
	uint		restricted:1 ;
	uint		all:1 ;
	uint		self:1 ;
	uint		typesort:1 ;
	uint		namecache:1 ;
	uint		gm:1 ;
	uint		rn:1 ;
} ;

struct locinfo {
	cchar		*utfname ;
	PROGINFO	*pip ;
	struct ustats	s ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	NAMECACHE	nc ;
	GRMEMS		gm ;
	SYSREALNAME	rn ;
	pid_t		sid ;
	int		to_cache ;
	int		max ;
	char		unbuf[USERNAMELEN+1] ;
	char		gnbuf[GROUPNAMELEN+1] ;
	char		typesort[4] ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,void *,cchar *) ;
static int	procloadnames(PROGINFO *,OSETSTR *,cchar *,int) ;
static int	procloadname(PROGINFO *,OSETSTR *,cchar *,int) ;
static int	procents(PROGINFO *,SHIO *,OSETSTR *) ;
static int	procgetdb(PROGINFO *,OSETSTR *,VECOBJ *) ;
static int	procbiffable(PROGINFO *,TMPX_ENT *) ;
static int	procsort(PROGINFO *,VECOBJ *) ;
static int	procout(PROGINFO *,SHIO *,VECOBJ *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_utfname(LOCINFO *,cchar *) ;
static int	locinfo_username(LOCINFO *) ;
static int	locinfo_groupname(LOCINFO *) ;
static int	locinfo_namecache(LOCINFO *) ;
static int	locinfo_lookup(LOCINFO *,cchar *,cchar **) ;
static int	locinfo_cachestats(LOCINFO *,LOCINFO_CACHESTATS *) ;
static int	locinfo_gmcurbegin(LOCINFO *,LOCINFO_GMCUR *) ;
static int	locinfo_gmcurend(LOCINFO *,LOCINFO_GMCUR *) ;
static int	locinfo_gmlook(LOCINFO *,LOCINFO_GMCUR *,cchar *,int) ;
static int	locinfo_gmread(LOCINFO *,LOCINFO_GMCUR *,char *,int) ;
static int	locinfo_rncurbegin(LOCINFO *,LOCINFO_RNCUR *) ;
static int	locinfo_rncurend(LOCINFO *,LOCINFO_RNCUR *) ;
static int	locinfo_rnlook(LOCINFO *,LOCINFO_RNCUR *,cchar *,int) ;
static int	locinfo_rnread(LOCINFO *,LOCINFO_RNCUR *,char *,int) ;

static int	vcmpname(const void *,const void *) ;
static int	vcmpfor(const void *,const void *) ;
static int	vcmprev(const void *,const void *) ;


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
	"nh",
	"utf",
	"db",
	"sort",
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
	argopt_nh,
	argopt_utf,
	argopt_db,
	argopt_sort,
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
	"header",
	"hdr",
	"long",
	"short",
	"uniq",
	"users",
	"all",
	"line",
	"sort",
	"msg",
	"mesg",
	"biff",
	NULL
} ;

enum akonames {
	akoname_header,
	akoname_hdr,
	akoname_long,
	akoname_short,
	akoname_uniq,
	akoname_users,
	akoname_all,
	akoname_line,
	akoname_sort,
	akoname_msg,
	akoname_mesg,
	akoname_biff,
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


/* exported subroutines */


int b_wn(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_wn) */


int p_wn(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_wn) */


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
	int		cl ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*utfname = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_wn: starting DFD=%d\n",rs) ;
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

#if	CF_DEBUGS
	    debugprintf("b_wn: ai=%u a=>%t<\n",ai,argp,argl) ;
#endif

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

	                case argopt_nh:
	                    lip->final.hdr = TRUE ;
	                    lip->f.hdr = FALSE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.hdr = (rs == 0) ;
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

/* UTMP file */
	                case argopt_utf:
	                case argopt_db:
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

/* type-sort */
	                case argopt_sort:
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
	                    if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
	                        strwcpy(lip->typesort,cp,cl) ;
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

#if	CF_DEBUGS
		debugprintf("b_wn: kc=%c\n",kc) ;
#endif

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

	                    case 'H':
	                        lip->have.hdr = TRUE ;
	                        lip->f.hdr = TRUE ;
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

	                    case 'a':
	                        lip->have.all = TRUE ;
	                        lip->f.all = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.all = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* biffing only */
	                    case 'b':
	                        lip->final.biff = TRUE ;
	                        lip->have.biff = TRUE ;
	                        lip->f.biff = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.biff = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* print header */
	                    case 'h':
	                        lip->final.hdr = TRUE ;
	                        lip->have.hdr = TRUE ;
	                        lip->f.hdr = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.hdr = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* long mode */
	                    case 'l':
	                        lip->final.fmtlong = TRUE ;
	                        lip->have.fmtlong = TRUE ;
	                        lip->f.fmtlong = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.fmtlong = (rs > 0) ;
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

/* short mode */
	                    case 's':
	                        lip->f.fmtshort = TRUE ;
	                        lip->have.fmtshort = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.fmtshort = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* unique mode */
	                    case 'u':
	                        lip->f.uniq = TRUE ;
	                        lip->have.uniq = TRUE ;
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
	if (DEBUGLEVEL(2))
	    debugprintf("b_wn: debuglevel=%u\n",pip->debuglevel) ;
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

/* load up the environment options */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (ofname == NULL) ofname = getourenv(envv,VAROFNAME) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* argument defaults */

	if (lip->f.fmtshort && lip->f.uniq)
	    lip->f.users = TRUE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_wn: f_quick=%u\n",lip->f.fmtshort) ;
#endif

	if ((rs >= 0) && (lip->max == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    lip->max = rs ;
	}

/* other initilization */

	if (utfname == NULL) utfname = getourenv(envv,VARUTFNAME) ;

	if (rs >= 0) {
	    rs = locinfo_utfname(lip,utfname) ;
	}

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: hdr=%u\n" ;
	    shio_printf(pip->efp,fmt,pn,lip->f.hdr) ;
	    fmt = "%s: fmtlong=%u\n" ;
	    shio_printf(pip->efp,fmt,pn,lip->f.fmtlong) ;
	}

#ifdef	COMMENT
	if ((cp = getourenv(envv,VARUSERNAME)) != NULL)
	    strwcpy(lip->username,cp,USERNAMELEN) ;
#endif

/* load up argument information */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;
	ainfo.argv = argv ;

	if (rs >= 0) {
	    cchar	*ofn = ofname ;
	    cchar	*afn = afname ;
	    rs = process(pip,&ainfo,&pargs,ofn,afn) ;
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	}

	if ((rs >= 0) && (pip->debuglevel > 0) && lip->open.namecache) {
	    LOCINFO_CACHESTATS	s ;
	    cchar		*pn = pip->progname ;
	    cchar		*fmt ;

	    fmt = "%s: cache accesses=%u\n" ;
	    shio_printf(pip->efp,fmt,pn,lip->s.total) ;

	    if ((rs = locinfo_cachestats(lip,&s)) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf("b_wn: namecache_stats() rs=%d\n",rs1) ;
	            if (r >= 0) {
	                debugprintf("b_wn: cache total=%u\n", s.total) ;
	                debugprintf("b_wn: cache phits=%u\n", s.phits) ;
	                debugprintf("b_wn: cache nhits=%u\n", s.nhits) ;
	            }
	        }
#endif /* CF_DEBUG */

		fmt = "%s: cache phits=%u\n" ;
	        shio_printf(pip->efp,fmt,pn,s.phits) ;
		fmt = "%s: cache nhits=%u\n" ;
	        shio_printf(pip->efp,fmt,pn,s.nhits) ;
	    } /* end if (locinfo_cachestats) */

	} /* end if (summary) */

/* done */
	if ((rs < 0) && (! pip->f.quiet)) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: could not perform function (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	}

	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
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
	    debugprintf("b_wn: final mallout=%u\n",(mo-mo_start)) ;
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

	fmt = "%s: USAGE> %s [-h[=<b>]] [-s|-l] [-u] [<username(s)> ...]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [<groupspec(s)>] [-o <opt(s)>] [-utf <utmp>]\n" ;
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
	                case akoname_header:
	                case akoname_hdr:
	                    if (! lip->final.hdr) {
	                        lip->final.hdr = TRUE ;
	                        lip->f.hdr = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.hdr = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_long:
	                    if (! lip->final.fmtlong) {
	                        lip->have.fmtlong = TRUE ;
	                        lip->final.fmtlong = TRUE ;
	                        lip->f.fmtlong = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.fmtlong = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_short:
	                    if (! lip->final.fmtshort) {
	                        lip->have.fmtshort = TRUE ;
	                        lip->final.fmtshort = TRUE ;
	                        lip->f.fmtshort = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.fmtshort = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_uniq:
	                    if (! lip->final.uniq) {
	                        lip->have.uniq = TRUE ;
	                        lip->final.uniq = TRUE ;
	                        lip->f.uniq = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.uniq = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_users:
	                    if (! lip->final.users) {
	                        lip->have.users = TRUE ;
	                        lip->final.users = TRUE ;
	                        lip->f.users = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.users = (rs > 0) ;
	                        }
	                    }
	                    break ;
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
	                case akoname_line:
	                    if (! lip->final.fmtline) {
	                        lip->have.fmtline = TRUE ;
	                        lip->final.fmtline = TRUE ;
	                        lip->f.fmtline = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.fmtline = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_msg:
	                case akoname_mesg:
	                    if (! lip->final.msg) {
	                        lip->have.msg = TRUE ;
	                        lip->final.msg = TRUE ;
	                        lip->f.msg = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.msg = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_biff:
	                    if (! lip->final.biff) {
	                        lip->have.biff = TRUE ;
	                        lip->final.biff = TRUE ;
	                        lip->f.biff = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.biff = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_sort:
	                    if (! lip->final.typesort) {
	                        lip->have.typesort = TRUE ;
	                        lip->final.typesort = TRUE ;
	                        lip->typesort[0] = 'f' ;
	                        if (vl > 0) {
	                            strwcpy(lip->typesort,vp,MIN(3,vl)) ;
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


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {

	    if (lip->f.linebuf)
	        rs = shio_control(ofp,SHIO_CSETBUFLINE,TRUE) ;

	    if (rs >= 0) {
	        rs = procargs(pip,aip,bop,ofp,afn) ;
	        wlen += rs ;
	    } /* end if (ok) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,void *ofp,cchar *afn)
{
	OSETSTR		ss ;
	const int	n = 20 ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((rs = osetstr_start(&ss,n)) >= 0) {
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
	                    rs = procloadname(pip,&ss,cp,-1) ;
	                }
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
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
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procloadnames(pip,&ss,cp,cl) ;
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

#ifdef	COMMENT
	    if ((rs >= 0) && (pan == 0)) {

	        cp = "-" ;
	        pan += 1 ;
	        rs = procloadname(pip,&ss,cp,-1) ;

	    } /* end if (default) */
#endif /* COMMENT */

	    if (rs >= 0) {
	        rs = procents(pip,ofp,&ss) ;
	        wlen += rs ;
	    } /* end if (ok) */

	    rs1 = osetstr_finish(&ss) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (osetstr) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procloadnames(PROGINFO *pip,OSETSTR *nlp,cchar *sp,int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

	if (nlp == NULL) return SR_FAULT ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		fl ;
	    cchar	*fp ;

	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procloadname(pip,nlp,fp,fl) ;
	            c += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */

	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procloadnames) */


static int procloadname(PROGINFO *pip,OSETSTR *nlp,cchar np[],int nl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if (np == NULL) return SR_FAULT ;

	if (np[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/procloadname: ent rn=%t\n",np,nl) ;
#endif

	if (nl < 0) nl = strlen(np) ;

	if ((np[0] == '\0') || hasMeAlone(np,nl)) {
	    if ((rs = locinfo_username(lip)) >= 0) {
	        np = lip->unbuf ;
	        nl = rs ;
	        rs = osetstr_add(nlp,np,nl) ;
	        c += rs ;
	    } /* end if (locinfo_username) */
	} else {
	    const int	nch = MKCHAR(np[0]) ;
	    cchar	*tp ;
	    if ((tp = strnchr(np,nl,'+')) != NULL) {
	        nl = (tp-np) ;
	    }
	    if (strnchr(np,nl,'.') != NULL) {
	        LOCINFO_RNCUR	rnc ;
	        if ((rs = locinfo_rncurbegin(lip,&rnc)) >= 0) {
	            if ((rs = locinfo_rnlook(lip,&rnc,np,nl)) > 0) {
	                const int	ul = USERNAMELEN ;
	                char		ub[USERNAMELEN+1] ;
	                while ((rs = locinfo_rnread(lip,&rnc,ub,ul)) > 0) {
	                    rs = osetstr_add(nlp,ub,rs) ;
	        	    c += rs ;
	                    if (rs < 0) break ;
	                } /* end while (reading entries) */
	            } /* end if (locinfo_rnlook) */
	            rs1 = locinfo_rncurend(lip,&rnc) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (srncursor) */
	    } else if (nch == MKCHAR('¡')) {
	        LOCINFO_GMCUR	gc ;
	        cchar		*gnp = (np+1) ;
	        int		gnl = (nl-1) ;
		if (gnl == 0) {
		    rs = locinfo_groupname(lip) ;
		    gnl = rs ;
		    gnp = lip->gnbuf ;
		}
		if (rs >= 0) {
	            if ((rs = locinfo_gmcurbegin(lip,&gc)) >= 0) {
	                if ((rs = locinfo_gmlook(lip,&gc,gnp,gnl)) > 0) {
	                    const int	ul = USERNAMELEN ;
	                    char	ub[USERNAMELEN+1] ;
	                    while ((rs = locinfo_gmread(lip,&gc,ub,ul)) > 0) {
	                        rs = osetstr_add(nlp,ub,rs) ;
	        		c += rs ;
	                        if (rs < 0) break ;
	                    } /* end while */
	                } /* end if */
	                rs1 = locinfo_gmcurend(lip,&gc) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (gmcursor) */
		} /* end if (ok) */
	    } else {
	        if (nch == '!') {
	            np += 1 ;
	            nl -= 1 ;
	        }
		if (nl == 0) {
	    	    if ((rs = locinfo_username(lip)) >= 0) {
	        	np = lip->unbuf ;
			nl = rs ;
		    }
		}
	        if ((rs >= 0) && (nl > 0)) {
	            rs = osetstr_add(nlp,np,nl) ;
	            c += rs ;
		}
	    } /* end if */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/procloadname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procloadname) */


static int procents(PROGINFO *pip,SHIO *ofp,OSETSTR *nlp)
{
	LOCINFO		*lip = pip->lip ;
	vecobj		entries, *elp = &entries ;
	const int	esize = sizeof(TMPX_ENT) ;
	int		rs ;
	int		rs1 ;

	if ((rs = vecobj_start(elp,esize,20,0)) >= 0) {

	    if ((rs = osetstr_count(nlp)) > 0) {
	        lip->f.restricted = TRUE ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_wn/process: do the DB lookups rs=%d\n",rs) ;
#endif

/* do the DB look-ups (access system UTMPX database) */

	    if (rs >= 0) {
	        rs = procgetdb(pip,nlp,elp) ;
	    }

	    if ((rs >= 0) && (lip->typesort[0] != '\0')) {
	        rs = procsort(pip,elp) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_wn/process: process and print rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        rs = procout(pip,ofp,elp) ;
	    }

	    rs1 = vecobj_finish(elp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (entries) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_wn/process: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procents) */


/* extract our names from the system UTMPX database */
static int procgetdb(PROGINFO *pip,OSETSTR *nlp,VECOBJ *elp)
{
	LOCINFO		*lip = pip->lip ;
	TMPX		ut ;
	TMPX_ENT	ute, *up = &ute ;
	const int	of = O_RDONLY ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = tmpx_open(&ut,lip->utfname,of)) >= 0) {
	    int		ei ;
	    int		cl ;
	    int		f ;
	    cchar	*cp ;

	    for (ei = 0 ; (rs = tmpx_read(&ut,ei,up)) > 0 ; ei += 1) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_wn/procgetdb: user=%s\n",up->ut_user) ;
#endif

	        f = (up->ut_type == UTMPX_TUSERPROC) ;

	        if ((! f) && (! lip->f.restricted) && lip->f.all) {
	            f = (up->ut_type == UTMPX_TINITPROC) ;
	            f = f || (up->ut_type == UTMPX_TLOGINPROC) ;
	        } /* end if */

#if	CF_DOTUSER
#else
	        if (f) f = (up->ut_user[0] != '.') ;
#endif /* CF_DOTUSER */

	        if (f && lip->f.restricted) {

	            if (lip->f.self) {
	                f = (lip->sid == up->ut_pid) ;
	            } else {
	                const int	ml = MIN(UTMPX_LUSER,LOGNAMELEN) ;
	                cp = up->ut_user ;
	                cl = strnlen(up->ut_user,ml) ;
	                rs = osetstr_already(nlp,cp,cl) ;
	                f = (rs > 0) ;
	            }

	        } /* end if */

	        if ((rs >= 0) && f && (lip->f.msg || lip->f.biff)) {
	            rs = procbiffable(pip,up) ;
	            f = rs ;
	        }

	        if ((rs >= 0) && f && lip->f.uniq) {
	            rs1 = vecobj_search(elp,up,vcmpname,NULL) ;
	            if (rs1 >= 0) f = FALSE ;
	        }

	        if ((rs >= 0) && f) {
	            c += 1 ;
	            lip->s.total += 1 ;
	            rs = vecobj_add(elp,up) ;
	        } /* end if (entered) */

	        if (rs < 0) break ;
	    } /* end for */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_wn/procgetdb: for-out rs=%d ei=%u\n",rs,ei) ;
#endif

	    rs1 = tmpx_close(&ut) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (tmpx) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_wn/procgetdb: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procgetdb) */


static int procbiffable(PROGINFO *pip,TMPX_ENT *up)
{
	LOCINFO		*lip = pip->lip ;
	const int	ullen = UTMPX_LLINE ;
	int		rs ;
	int		f = FALSE ;
	char		dname[MAXPATHLEN+1] ;

	if ((rs = mkpath2w(dname,DEVDNAME,up->ut_line,ullen)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = u_stat(dname,&sb)) >= 0) {
	        const mode_t	tm = sb.st_mode ;
	        f = (tm & S_IWGRP) ;
	        if (f && lip->f.biff) {
	            f = (tm & S_IXUSR) ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	} /* end if (mkpath) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procbiffable) */


static int procsort(PROGINFO *pip,VECOBJ *elp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (lip->typesort[0] != '\0') {
	    if ((rs = vecobj_count(elp)) > 0) {
	        int	(*sortfunc)() ;
	        c = rs ;
	        if (lip->typesort[0] == 'r') {
	            sortfunc = vcmprev ;
	        } else {
	            sortfunc = vcmpfor ;
		}
	        rs = vecobj_sort(elp,sortfunc) ;
	    }
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procsort) */


/* process the entries and print them out */
static int procout(PROGINFO *pip,SHIO *ofp,VECOBJ *elp)
{
	LOCINFO		*lip = pip->lip ;
	TMPX_ENT	*up ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ml, cl, rl ;
	int		tmch = ' ' ;
	int		i ;
	int		c = 0 ;
	cchar		*fmt ;
	cchar		*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		timebuf[TIMEBUFLEN + 1] ;

	if (lip->f.hdr && (! lip->f.fmtshort) && (! lip->f.fmtline)) {

	    if (lip->f.fmtlong) {
	        fmt = "USER       LINE         LOGIN          "
	            "  ID    SID SN HOST\n" ;

	    } else
	        fmt = "USER       LINE         LOGIN          NAME\n" ;

	    rs = shio_printf(ofp,fmt) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_wn/procout: shio_print() rs=%d\n",rs) ;
#endif

	} /* end if (header was requested) */

/* do the post-processing and print out */

	for (i = 0 ; (rs >= 0) && (vecobj_get(elp,i,&up) >= 0) ; i += 1) {
	    char	ut_userbuf[UTMPX_LUSER + 1] ;
	    char	ut_linebuf[UTMPX_LLINE + 1] ;

	    if (up == NULL) continue ;

	    if ((lip->max > 0) && (c++ >= lip->max)) break ;

	    if (lip->f.fmtshort) {

	        cp = up->ut_user ;
	        cl = strnlen(cp,MIN(COLS_USERNAME,UTMPX_LUSER)) ;

	        rs = shio_printf(ofp,"%t\n",cp,cl) ;

	    } else if (lip->f.fmtline) {

	        cp = up->ut_line ;
	        cl = strnlen(cp,UTMPX_LLINE) ;

	        rs = shio_printf(ofp,"%t\n",cp,cl) ;

	    } else {

	        ml = MIN(COLS_USERNAME,UTMPX_LUSER) ;
	        strwcpy(ut_userbuf,up->ut_user,ml) ;

	        ml = UTMPX_LLINE ;
	        strwcpy(ut_linebuf,up->ut_line,ml) ;

/* is the terminal writable? */

	        if ((rs1 = mkpath2(tmpfname,DEVDNAME,ut_linebuf)) >= 0) {
	            if ((rs1 = termwritable(tmpfname)) >= 0) {
	                switch (rs1) {
	                case 1:
	                    tmch = '+' ;
	                    break ;
	                case 2:
	                    tmch = '1' + 128 ;
	                    break ;
	                default:
	                    tmch = ' ' ;
	                    break ;
	                } /* end switch */
	            } /* end if */
	        } /* end if */

/* print whichever output format */

	        if (lip->f.fmtlong) {
	            char	ut_idbuf[UTMPX_LID + 1] ;
	            char	ut_hostbuf[UTMPX_LHOST + 1] ;

	            strwcpy(ut_idbuf,up->ut_id,MIN(4,UTMPX_LID)) ;

	            strwcpy(ut_hostbuf,up->ut_host,UTMPX_LHOST) ;

	            timestr_log(up->ut_tv.tv_sec,timebuf),
	            rs = shio_printf(ofp,
	                "%-8s %c %-12s %s %4s %6u %2u %s\n",
	                ut_userbuf,tmch,
	                ut_linebuf,
	                timebuf,
	                ut_idbuf,
	                up->ut_pid,
	                MIN(up->ut_session,99),
	                ut_hostbuf
	                ) ;

	        } else {
	            int		f = (up->ut_type == UTMPX_TUSERPROC) ;
		    cchar	*rp ;

	            rl = 0 ;
	            if (f) {
	                rs1 = locinfo_lookup(lip,ut_userbuf,&rp) ;
	                rl = rs1 ;
	            }
	            if (rl > 0) {
	                if (rl > COLS_REALNAME) rl = COLS_REALNAME ;
	                fmt = "%-8s %c %-12s %s (%s)\n" ;
	            } else {
	                fmt = "%-8s %c %-12s %s\n" ;
		    }

	            if (rs >= 0) {
	                cchar	*tb = timebuf ;
	                cchar	*ub = ut_userbuf ;
	                cchar	*lb = ut_linebuf ;
	                timestr_log(up->ut_tv.tv_sec,timebuf) ;
	                rs = shio_printf(ofp,fmt,ub,tmch,lb,tb,rp) ;
	            }

	        } /* end if */

	    } /* end if (short) */

	} /* end for (entries) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_wn/procout: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procout) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->sid = getsid(0) ;		/* should not fail! */

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.rn) {
	    lip->open.rn = FALSE ;
	    rs1 = sysrealname_close(&lip->rn) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.gm) {
	    lip->open.gm = FALSE ;
	    rs1 = grmems_finish(&lip->gm) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.namecache) {
	    lip->open.namecache = FALSE ;
	    rs1 = namecache_finish(&lip->nc) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_utfname(LOCINFO *lip,cchar *utfname)
{

	if (lip == NULL) return SR_FAULT ;

	lip->utfname = utfname ;
	return SR_OK ;
}
/* end subroutine (locinfo_utfname) */


static int locinfo_username(LOCINFO *lip)
{
	int		rs ;
	if (lip == NULL) return SR_FAULT ;
	if (lip->unbuf[0] == '\0') {
	    rs = getusername(lip->unbuf,USERNAMELEN,-1) ;
	} else {
	    rs = strlen(lip->unbuf) ;
	}
	return rs ;
}
/* end subroutine (locinfo_username) */


static int locinfo_groupname(LOCINFO *lip)
{
	const int	gnlen = GROUPNAMELEN ;
	int		rs ;
	if (lip == NULL) return SR_FAULT ;
	if (lip->gnbuf[0] == '\0') {
	    rs = getgroupname(lip->gnbuf,gnlen,-1) ;
	} else {
	    rs = strlen(lip->gnbuf) ;
	}
	return rs ;
}
/* end subroutine (locinfo_groupname) */


static int locinfo_namecache(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	if (! lip->open.namecache) {
	    const int	to = lip->to_cache ;
	    if ((rs = namecache_start(&lip->nc,VARUSERNAME,0,to)) >= 0) {
	        cchar	*up = getourenv(pip->envv,VARUSERNAME) ;
	        lip->open.namecache = TRUE ;
	        if (up != NULL) {
	            cchar	*cp = getourenv(pip->envv,VARFULLNAME) ;
	            if ((cp == NULL) || (cp[0] == '\0')) {
	                cp = getourenv(pip->envv,VARNAME) ;
	            }
	            if ((cp != NULL) && (cp[0] != '\0')) {
	                rs = namecache_add(&lip->nc,up,cp,-1) ;
	            }
	        }
	        if (rs < 0) {
	            lip->open.namecache = FALSE ;
	            namecache_finish(&lip->nc) ;
	        }
	    } /* end if (namecache) */
	} /* end if (needed-construction) */

	return rs ;
}
/* end subroutine (locinfo_namecache) */


static int locinfo_lookup(LOCINFO *lip,cchar *un,cchar **rpp)
{
	int		rs = SR_OK ;

	if (! lip->open.namecache) {
	    rs = locinfo_namecache(lip) ;
	}

	if (rs >= 0) {
	    rs = namecache_lookup(&lip->nc,un,rpp) ;
	}

	return rs ;
}
/* end subroutine (locinfo_lookup) */


static int locinfo_cachestats(LOCINFO *lip,LOCINFO_CACHESTATS *sp)
{
	int		rs = SR_OK ;

	memset(sp,0,sizeof(LOCINFO_CACHESTATS)) ;

	if (lip->open.namecache) {
	    rs = namecache_stats(&lip->nc,sp) ;
	}

	return rs ;
}
/* end subroutine (locinfo_cachestats) */


int locinfo_gmcurbegin(LOCINFO *lip,LOCINFO_GMCUR *curp)
{
	int		rs = SR_OK ;

	if (curp == NULL) return SR_FAULT ;

	if (! lip->open.gm) {
	    const int	max = 20 ;
	    const int	ttl = (12*3600) ;
	    rs = grmems_start(&lip->gm,max,ttl) ;
	    lip->open.gm = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = grmems_curbegin(&lip->gm,&curp->gmcur) ;
	}

	return rs ;
}
/* end subroutine (locinfo_gmcurbegin) */


int locinfo_gmcurend(LOCINFO *lip,LOCINFO_GMCUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (curp == NULL) return SR_FAULT ;

	rs1 = grmems_curend(&lip->gm,&curp->gmcur) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_gmcurend) */


int locinfo_gmlook(LOCINFO *lip,LOCINFO_GMCUR *curp,cchar *gnp,int gnl)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;

	if ((rs = grmems_lookup(&lip->gm,&curp->gmcur,gnp,gnl)) >= 0) {
	    rs = 1 ;
	} else if (rs == rsn) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (locinfo_gmlook) */


int locinfo_gmread(LOCINFO *lip,LOCINFO_GMCUR *curp,char *ubuf,int ulen)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;

	if ((rs = grmems_lookread(&lip->gm,&curp->gmcur,ubuf,ulen)) == rsn) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (locinfo_gmread) */


int locinfo_rncurbegin(LOCINFO *lip,LOCINFO_RNCUR *curp)
{
	int		rs = SR_OK ;

	if (curp == NULL) return SR_FAULT ;

	if (! lip->open.rn) {
	    rs = sysrealname_open(&lip->rn,NULL) ;
	    lip->open.rn = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = sysrealname_curbegin(&lip->rn,&curp->rncur) ;
	}

	return rs ;
}
/* end subroutine (locinfo_rncurbegin) */


int locinfo_rncurend(LOCINFO *lip,LOCINFO_RNCUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (curp == NULL) return SR_FAULT ;

	rs1 = sysrealname_curend(&lip->rn,&curp->rncur) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_rncurend) */


int locinfo_rnlook(LOCINFO *lip,LOCINFO_RNCUR *curp,cchar *gnp,int gnl)
{
	PROGINFO	*pip = lip->pip ;
	const int	rsn = SR_NOTFOUND ;
	const int	fo = 0 ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;
	if (pip == NULL) return SR_FAULT ;

	if ((rs = sysrealname_look(&lip->rn,&curp->rncur,fo,gnp,gnl)) >= 0) {
	    rs = 1 ;
	} else if (rs == rsn) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rnlook: sysrealname_look() rs=%d\n",
	        rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_rnlook) */


int locinfo_rnread(LOCINFO *lip,LOCINFO_RNCUR *curp,char *ubuf,int ulen)
{
	PROGINFO	*pip = lip->pip ;
	const int	rsn = SR_NOTFOUND ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;
	if (pip == NULL) return SR_FAULT ;

	if ((ulen >= 0) && (ulen < USERNAMELEN)) return SR_OVERFLOW ;

	if ((rs = sysrealname_lookread(&lip->rn,&curp->rncur,ubuf)) == rsn) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rnread: sysrealname_lookread() rs=%d\n",
	        rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_rnread) */


static int vcmpname(const void *v1pp,const void *v2pp)
{
	TMPX_ENT	**e1pp = (TMPX_ENT **) v1pp ;
	TMPX_ENT	**e2pp = (TMPX_ENT **) v2pp ;
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = strcmp((*e1pp)->ut_user,(*e2pp)->ut_user) ;
	        } else
	            rc = 1 ;
	    } else
	        rc = -1 ;
	}
	return rc ;
}
/* end subroutine (vcmpname) */


static int vcmpfor(const void *v1pp,const void *v2pp)
{
	TMPX_ENT	**e1pp = (TMPX_ENT **) v1pp ;
	TMPX_ENT	**e2pp = (TMPX_ENT **) v2pp ;
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = ((*e1pp)->ut_tv.tv_sec - (*e2pp)->ut_tv.tv_sec) ;
	            if (rc == 0) {
	                rc = ((*e1pp)->ut_tv.tv_usec - (*e2pp)->ut_tv.tv_usec) ;
		    }
	        } else
		    rc =  -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (vcmpfor) */


static int vcmprev(const void *v1pp,const void *v2pp)
{
	TMPX_ENT	**e1pp = (TMPX_ENT **) v1pp ;
	TMPX_ENT	**e2pp = (TMPX_ENT **) v2pp ;
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = - ((*e1pp)->ut_tv.tv_sec - (*e2pp)->ut_tv.tv_sec) ;
	            if (rc == 0)
	            rc = - ((*e1pp)->ut_tv.tv_usec - (*e2pp)->ut_tv.tv_usec) ;
	        } else
		    rc = -1 ;
	    } else
		rc = 1 ;
	}
	return rc ;
}
/* end subroutine (vcmprev) */


