/* b_makesafe */

/* generic short program front-end */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_DEBUGENV	0		/* debug environment */
#define	CF_ALWAYS	1		/* always update the target? */
#define	CF_LOCSETENT	1		/* compile |locinfo_setentry()| */
#define	CF_TESTSLEEP	0		/* insert sleep for testing */
#define	CF_WRFILE	0		/* compile |wrfile()| */
#define	CF_DISPABORT	0		/* compile |disp_abort()| */


/* revision history:

	= 2004-05-14, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did. It used pieces from other (similar in some ways)
        programs. It is linted out and should be very clean -- we depend on this
        everyday to do what we need. Pieces not used in their full like where
        they originally were, are sort of hacked up to minimal code. Try not to
        get your knickers in a bunch over that.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is the main subroutine for MAKESAFE. This was a fairly generic
        subroutine adpapted for this program. Note that parallel processing is
        enabled by default. If you do not want parallel processing for some
        reason use the '-o' invocation option to set the maximum parallelism to
        '1'.


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
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>
#include	<time.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<tmtime.h>
#include	<bfile.h>
#include	<filebuf.h>
#include	<vecstr.h>
#include	<vecpstr.h>
#include	<vecobj.h>
#include	<sbuf.h>
#include	<ids.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<psem.h>
#include	<fsi.h>
#include	<fsdir.h>
#include	<dirlist.h>
#include	<cachetime.h>
#include	<upt.h>
#include	<spawnproc.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_makesafe.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((2 * MAXPATHLEN),2048)
#endif

#define	FBUFLEN		(2*LINEBUFLEN)

#define	NDEPS		128		/* default values */

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags
#define	LOCINFO_CUR	struct locinfo_cur

#define	DISP		struct disp_head
#define	DISP_ARGS	struct disp_args
#define	DISP_THR	struct disp_thr

#define	CPPERR		struct cpperr

#define	LSTATE		struct lstate


/* external subroutines */

extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath2w(char *,cchar *,cchar *,int) ;
extern int	mkaltext(char *,cchar *,cchar *) ;
extern int	sfsub(cchar *,int,cchar *,cchar **) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	sfdequote(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	nextfield(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	msleep(int) ;
extern int	findfilepath(cchar *,char *,cchar *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	vecpstr_adduniq(vecpstr *,cchar *,int) ;
extern int	mktmpuserdir(char *,cchar *,cchar *,mode_t) ;
extern int	getnprocessors(cchar **,int) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	rmdirfiles(cchar *,cchar *,int) ;
extern int	hasnonwhite(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isFailOpen(int) ;
extern int	isStrEmpty(cchar *,int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_cur {
	DIRLIST_CUR	c ;
} ;

struct locinfo_flags {
	uint		stores:1 ;
	uint		maint:1 ;	/* user requested maintenance */
	uint		tmpmaint:1 ;	/* thread spawned */
	uint		id:1 ;
	uint		incs:1 ;
	uint		cache:1 ;
	uint		dirs:1 ;
	uint		mtdb:1 ;
	uint		zero:1 ;
	uint		remote:1 ;
	uint		nochange:1 ;
	uint		print:1 ;
	uint		optpar:1 ;
	uint		optdebug:1 ;
} ;

struct locinfo {
	vecstr		stores ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	IDS		id ;
	DIRLIST		incs ;
	CACHETIME	mtdb ;
	PTM		efm ;			/* mutex file-error */
	PTM		ofm ;			/* mutex file-output */
	PROGINFO	*pip ;
	void		*ofp ;
	const char	*jobdname ;		/* tmp-user directory */
	const char	*suffix_from ;
	const char	*suffix_to ;
	const char	*prog_cpp ;
	pthread_t	tid ;
	uid_t		uid_pr ;
	gid_t		gid_pr ;
	int		ncpu ;
	int		npar ;
	int		to_tmpfiles ;
	int		to_delete ;
	int		c_processed ;
	int		c_updated ;
} ;

struct disp_args {
	PROGINFO	*pip ;
	SHIO		*ofp ;
	int		npar ;
} ;

struct disp_thr {
	pthread_t	tid ;
	uint		f_active ;
	volatile int	f_exiting ;
} ;

struct disp_head {
	PROGINFO	*pip ;
	DISP_THR	*threads ;
	DISP_ARGS	a ;
	FSI		wq ;		/* work queue */
	PSEM		wq_sem ;	/* signal-semaphore for workers */
	PTM		m ;		/* object */
	PTC		cond ;		/* condition variable */
	volatile int	f_exit ;	/* signal force exit */
	volatile int	f_done ;	/* signal end of new work */
	volatile int	f_wakeup ;	/* wait flag */
	volatile int	f_ready ;
	int		nthr ;		/* number of threads */
	int		tasks ;		/* count of completed tasks */
} ;

struct lstate {
	int		f_continue ;
} ;

struct cpperr {
	const char	*fname ;
	const char	*ifname ;
	int		line ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procsubprog(PROGINFO *,const char *) ;
static int	proctouchfile(PROGINFO *,const char *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,DISP *) ;
static int	procargfile(PROGINFO *,DISP *,cchar *) ;
static int	procfiler(PROGINFO *,const char *) ;
static int	procfilerdeps(PROGINFO *,cchar *,time_t) ;
static int	procfilerfinds(PROGINFO *,cchar *,time_t) ;
static int	procfilefind(PROGINFO *,cchar *,time_t,cchar *) ;
static int	procfiletell(PROGINFO *,cchar *,cchar *) ;
static int	procfile(PROGINFO *,DISP *,cchar *) ;
static int	proceprintf(PROGINFO *,cchar *,...) ;

static int procdeps_check(PROGINFO *,char *,time_t,cchar *) ;
static int procdeps_checker(PROGINFO *,char *,time_t,vecpstr *,cchar *) ;
static int procdeps_get(PROGINFO *,vecpstr *,VECOBJ *,const char *) ;
static int procdeps_loadargs(PROGINFO *,vecstr *,cchar *) ;
static int procdeps_incargs(PROGINFO *,vecstr *) ;
static int proclines(PROGINFO *,vecpstr *,int) ;
static int procline(PROGINFO *,vecpstr *,LSTATE *,cchar *,int) ;
static int procerr(PROGINFO *,VECOBJ *,int) ;
static int procerrline(PROGINFO *,VECOBJ *,const char *,int) ;

static int	procout_begin(PROGINFO *,void *,cchar *) ;
static int	procout_end(PROGINFO *) ;
static int	procout_printf(PROGINFO *,cchar *,...) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_jobdname(LOCINFO *) ;
static int	locinfo_tmpcheck(LOCINFO *) ;
static int	locinfo_tmpmaint(LOCINFO *) ;
static int	locinfo_tmpdone(LOCINFO *) ;
static int	locinfo_fchmodown(LOCINFO *,int,struct ustat *,mode_t) ;
static int	locinfo_loadprids(LOCINFO *) ;
static int	locinfo_alreadybegin(LOCINFO *) ;
static int	locinfo_alreadyend(LOCINFO *) ;
static int	locinfo_alreadystat(LOCINFO *) ;
static int	locinfo_incdirs(LOCINFO *) ;
static int	locinfo_incadds(LOCINFO *,cchar *,int) ;
static int	locinfo_alreadylookup(LOCINFO *,cchar *,int,time_t *) ;

static int	locinfo_incbegin(LOCINFO *,LOCINFO_CUR *) ;
static int	locinfo_incenum(LOCINFO *,LOCINFO_CUR *,char *,int) ;
static int	locinfo_incend(LOCINFO *,LOCINFO_CUR *) ;
static int	locinfo_ncpu(LOCINFO *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif

static int	disp_start(DISP *,DISP_ARGS *) ;
static int	disp_starter(DISP *) ;
static int	disp_waiting(DISP *) ;
static int	disp_addwork(DISP *,cchar *,int) ;
static int	disp_worker(DISP *) ;
static int	disp_exiting(DISP *) ;
static int	disp_allexiting(DISP *) ;
static int	disp_notready(DISP *,int *) ;
static int	disp_taskdone(DISP *) ;
static int	disp_finish(DISP *,int) ;
static int	disp_getourthr(DISP *,DISP_THR **) ;
static int	disp_readyset(DISP *) ;
static int	disp_readywait(DISP *) ;

#if	CF_DISPABORT
static int	disp_abort(DISP *) ;
#endif

static int cpperr_start(CPPERR *,int,const char *,int) ;
static int cpperr_ifname(CPPERR *,const char *,int) ;
static int cpperr_finish(CPPERR *) ;

#if	CF_DEBUG && CF_DEBUGENV
static int debugdumpenv(cchar **) ;
#endif

#if	CF_WRFILE
static int	wrfile(cchar *,int) ;
#endif


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"cpp",
	"jd",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_cpp,
	argopt_jd,
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
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const char	*aknames[] = {
	"cache",
	"cpp",
	"npar",
	"parallel",
	"debug",
	"maint",
	NULL
} ;

enum aknames {
	akname_cache,
	akname_cpp,
	akname_npar,
	akname_par,
	akname_debug,
	akname_maint,
	akname_overlast
} ;

static const char	*progcpps[] = {
	"/usr/ccs/lib/cpp",
	"/usr/lib/cpp",
	"/usr/add-on/ncmp/bin/cpp",
	NULL
} ;

static const char	errsub1[] = ", line " ;
static const char	errsub2[] = ": Can't find include file " ;

static const char	*deps[] = {
	"c",
	"cc",
	"h",
	"f",
	"y",
	"cpp",
	NULL
} ;


/* exported subroutines */


int b_makesafe(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_makesafe) */


int p_makesafe(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_makesafe) */


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
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_version = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*touchfname = NULL ;
	const char	*progcpp = NULL ;
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

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* early things to initialize */

	pip->verboselevel = 1 ;

	pip->lip = &li ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
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

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose */
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

/* temporary directory */
	                case argopt_tmpdir:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->tmpdname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->tmpdname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* help */
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

/* CPP program */
	                case argopt_cpp:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            progcpp = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                progcpp = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;
/* job directory */
	                case argopt_jd:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lip->jobdname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->jobdname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;


/* default action and user specified help */
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

	                    case 'I':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = locinfo_incadds(lip,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

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

	                    case 'V':
	                        f_version = TRUE ;
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

/* no-change */
	                    case 'n':
	                        lip->f.nochange = TRUE ;
	                        break ;

/* print something !! */
	                    case 'p':
	                        lip->f.print = TRUE ;
	                        break ;

	                    case 'r':
	                        lip->f.remote = TRUE ;
	                        break ;

/* touch file */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                touchfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* verbose output */
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

/* zero files (are OK) */
	                    case 'z':
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

	        } /* end if (digits or options) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = getourenv(envv,VARERRORFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0) goto badarg ;

	if (pip->debuglevel == 0) {
	    if ((cp = getourenv(envv,VARDEBUGLEVEL)) != NULL) {
	        if (hasnonwhite(cp,-1)) {
		    rs = optvalue(cp,-1) ;
		    pip->debuglevel = rs ;
	        }
	    }
	} /* end if */

/* check arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    int	pid = getpid() ;
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
	    debugprintf("main: pid=%u\n",pid) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

/* program root */

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

/* help */

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

/* initialize */

	if ((rs >= 0) && (lip->npar == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    lip->npar = rs ;
	}

	if ((rs >= 0) && (lip->npar == 0)) {
	    if ((cp = getourenv(envv,VARNPAR)) != NULL) {
	        rs = optvalue(cp,-1) ;
	        lip->npar = rs ;
	    }
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (ofname == NULL) ofname = getourenv(envv,VAROFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* procopts */

	if (rs >= 0) {
	    if ((rs = procopts(pip,&akopts)) >= 0) {
	        if ((rs = procsubprog(pip,progcpp)) >= 0) {
	            if ((rs = locinfo_jobdname(lip)) >= 0) {
	                if ((rs = locinfo_tmpcheck(lip)) >= 0) {
	                    rs = locinfo_incdirs(lip) ;
	                }
	            }
	        }
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    KEYOPT	*kop = &akopts ;
	    KEYOPT_CUR	cur ;
	    if ((rs = keyopt_curbegin(kop,&cur)) >= 0) {
	        while (rs >= 0) {
	            rs1 = keyopt_enumkeys(kop,&cur,&cp) ;
	            if (rs1 < 0) break ;
	            debugprintf("main: akopt key=%s\n",cp) ;
	        } /* end while */
	        keyopt_curend(kop,&cur) ;
	    } /* end if (keyopt-cur) */
	}
#endif /* CF_DEBUG */

	if ((rs >= 0) && (lip->npar == 0)) {
	    rs = locinfo_ncpu(lip) ;
	    lip->npar = (rs+1) ; /* normally add one */
	}

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: cache mode=%u\n" ;
	    shio_printf(pip->efp,fmt,pn,lip->f.cache) ;
	    fmt = "%s: allowed parallelism=%u\n" ;
	    shio_printf(pip->efp,fmt,pn,lip->npar) ;
	} /* end if */

/* check a few more things */

	if (pip->to_read <= 0) pip->to_read = TO_READ ;

/* find the CPP program (if we do not already have one) */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: CPP=%s\n",
	        pip->progname,lip->prog_cpp) ;
	}

/* continue */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;
	ainfo.argv = argv ;

	if (rs >= 0) {
	    SHIO	ofile ;
	    if ((rs = procout_begin(pip,&ofile,ofname)) >= 0) {
	        DISP		disp ;
	        DISP_ARGS	wa ;

	        memset(&wa,0,sizeof(DISP_ARGS)) ;
	        wa.pip = pip ;
	        wa.ofp = lip->ofp ;
	        wa.npar = lip->npar ;

	        if ((rs = disp_start(&disp,&wa)) >= 0) {
	            ARGINFO	*aip = &ainfo ;
	            BITS	*bop = &pargs ;
	            int		pan = 0 ;
	            cchar	*afn = afname ;
	            if ((rs = procargs(pip,aip,bop,&disp)) >= 0) {
	                pan += rs ;
	                rs = procargfile(pip,&disp,afn) ;
	                pan += rs ;
	            }

	            if (rs >= 0) {
	                if (pan > 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("b_makenewer: waiting\n") ;
#endif
	                    while ((rs = disp_waiting(&disp)) > 0) {
	                        if (rs >= 0) rs = lib_sigterm() ;
	                        if (rs >= 0) rs = lib_sigintr() ;
	                        if (rs < 0) break ;
	                    } /* end while */
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("b_makenewer: waiting-out\n") ;
#endif
	                } else if ((pan == 0) && (! lip->f.zero)) {
	                    rs = SR_INVALID ;
	                    ex = EX_USAGE ;
	                    if (! pip->f.quiet) {
				cchar	*pn = pip->progname ;
	                        cchar	*fmt ;
	                        fmt = "%s: no files were specified\n" ;
	                        shio_printf(pip->efp,fmt,pn) ;
	                    }
	                }
	            } /* end if (ok) */

	            rs1 = disp_finish(&disp,(rs<0)) ;
	            if (rs >= 0) rs = rs1 ;
	            if (rs > 0) lip->c_updated += rs ;
	        } /* end if (disp) */

	        if (rs >= 0) {
		    cchar	*pn = pip->progname ;
	            cchar	*fmt ;
	            if (pip->debuglevel > 0) {
	                fmt = "%s: safefiles processed=%u updated=%u\n" ;
	                shio_printf(pip->efp,fmt,pn,
			    lip->c_processed,lip->c_updated) ;
	            }
	            if (pip->verboselevel > 0) {
	                fmt = "safefiles processed=%u updated=%u\n" ;
	                shio_printf(lip->ofp,fmt,
	                    lip->c_processed,lip->c_updated) ;
	            }
	        } /* end if */

	        locinfo_alreadystat(lip) ; /* cache statistics */

	        rs1 = procout_end(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: output unavailable (%d)\n" ;
	        ex = EX_CANTCREAT ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    } /* end if */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: done rs=%d\n",rs) ;
	    debugprintf("main: processed=%d updated=%d\n",
	        lip->c_processed,lip->c_updated) ;
	}
#endif /* CF_DEBUG */

/* done */
	if (rs >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: good rs=%d processed=%d updated=%d\n",
	            rs,lip->c_processed,lip->c_updated) ;
#endif

	    if ((touchfname != NULL) && (touchfname[0] != '\0')) {
	        rs = proctouchfile(pip,touchfname) ;
	        if (rs == SR_NOENT) ex = EX_CANTCREAT ;
	    } /* end if (touch-file) */

	} /* end if */

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
	} /* end if */

/* we are out of here */
retearly:
	if ((pip->debuglevel > 0) || lip->f.optdebug) {
	    cchar	*pn = pip->progname ;
	    if (lip->f.optdebug) {
	        proginfo_pwd(pip) ;
	        shio_printf(pip->efp,"%s: dir=%s\n",pn,pip->pwd) ;
	    }
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",pn,ex,rs) ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(1))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
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
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (mainsub) */


/* print out (standard error) some short usage */
static int usage(PROGINFO *pip)
{
	int		rs = SR_NOTOPEN ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	if (pip->efp != NULL) {

	    fmt = "%s: USAGE> %s [<objfile(s)> ...] [[-I <incdir(s)>] ...]\n" ;
	    if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	    wlen += rs ;

	    fmt = "%s:  [-t <target>] [-z]\n" ;
	    if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;

	    fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-V]\n" ;
	    if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;

	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int proceprintf(PROGINFO *pip,cchar *fmt,...)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		len = 0 ;
	if (pip->debuglevel > 0) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    if ((rs = ptm_lock(&lip->efm)) >= 0) {
	        {
	            rs = shio_vprintf(pip->efp,fmt,ap) ;
	            len = rs ;
	        }
	        rs1 = ptm_unlock(&lip->efm) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	    va_end(ap) ;
	} /* end if */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (proceprintf) */


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
	    KEYOPT_CUR	cur ;
	    if ((rs = keyopt_curbegin(kop,&cur)) >= 0) {
	        int	ki ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&cur,&kp)) >= 0) {

	            if ((ki = matostr(aknames,2,kp,kl)) >= 0) {
	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (ki) {
	                case akname_cache:
	                    if (! lip->final.cache) {
	                        lip->have.cache = TRUE ;
	                        lip->f.cache = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.cache = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akname_cpp:
	                    if ((vl > 0) && (lip->prog_cpp == NULL)) {
	                        cchar	**vpp = &lip->prog_cpp ;
	                        rs = locinfo_setentry(lip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case akname_npar:
	                case akname_par:
	                    if (! lip->final.optpar) {
	                        lip->have.optpar = TRUE ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            lip->npar = rs ;
	                        }
	                    }
	                    break ;
	                case akname_debug:
	                    if (! lip->final.optdebug) {
	                        lip->have.optdebug = TRUE ;
	                        lip->f.optdebug = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.optdebug = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akname_maint:
	                    if (! lip->final.maint) {
	                        lip->have.maint = TRUE ;
	                        lip->f.maint = TRUE ;
	                        lip->f.zero = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.maint = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                } /* end switch */
	                c += 1 ;
	            } /* end if (valid option) */

	            if (rs < 0) break ;
	        } /* end while (procopts) */

	        keyopt_curend(kop,&cur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/procopts: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procsubprog(PROGINFO *pip,cchar *progcpp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	const char	*cp ;
	const char	**vpp = &lip->prog_cpp ;

	if (progcpp != NULL) {
	    rs = locinfo_setentry(lip,vpp,progcpp,-1) ;
	}

	if ((rs >= 0) && (lip->prog_cpp == NULL)) {
	    if ((cp = getourenv(pip->envv,VARCPP)) != NULL) {
	        rs = locinfo_setentry(lip,vpp,cp,-1) ;
	    }
	}

	if ((rs >= 0) && (lip->prog_cpp == NULL)) {
	    int		sl = -1 ;
	    const char	*sp = CPPFNAME ;
	    char	tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: need to find CPP\n") ;
#endif

	    if ((rs = findfilepath(NULL,tmpfname,sp,X_OK)) >= 0) {
	        if (rs > 0) {
	            sl = rs ;
	            sp = tmpfname ;
	        } else {
	            sl = -1 ;
	        }
	    } else if (isNotPresent(rs)) {
	        int	i ;

	        for (i = 0 ; progcpps[i] != NULL ; i += 1) {
	            sp = progcpps[i] ;
	            rs = perm(sp,-1,-1,NULL,X_OK) ;
	            if (rs >= 0) break ;
	        } /* end for */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main: perm() rs=%d\n",rs) ;
#endif

	    } /* end if */

	    if (rs >= 0) {
	        rs = proginfo_setentry(pip,vpp,sp,sl) ;
	    }

	} /* end if */

	if (isNotPresent(rs)) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: CPP program unavailable\n" ;
	    shio_printf(pip->efp,fmt,pn) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("b_makesafe/procsubprog: ret rs=%d\n",rs) ;
	    debugprintf("b_makesafe/procsubprog: cpp=%s\n",lip->prog_cpp) ;
	}
#endif

	return rs ;
}
/* end subroutine (procsubprog) */


static int proctouchfile(PROGINFO *pip,cchar *touchfname)
{
	bfile		touchfile ;
	int		rs = SR_OK ;
	int		f_write = FALSE ;
	char		timebuf[TIMEBUFLEN + 1] ;

	if (pip == NULL) return SR_FAULT ;
	if (touchfname == NULL) return SR_FAULT ;

	if (touchfname[0] == '\0') return SR_INVALID ;

	pip->daytime = time(NULL) ;

	rs = bopen(&touchfile,touchfname,"w",0666) ;

	if (isNotPresent(rs)) {
	    f_write = TRUE ;
	    rs = bopen(&touchfile,touchfname,"wct",0666) ;
	}

	if (rs >= 0) {

#if	CF_ALWAYS
	    bprintf(&touchfile,"%s\n",
	        timestr_logz(pip->daytime,timebuf)) ;
#else
	    if (f_write || (lip->c_updated > 0)) {
	        bprintf(&touchfile,"%s\n",
	            timestr_logz(pip->daytime,timebuf)) ;
	    }
#endif /* CF_ALWAYS */

	    bclose(&touchfile) ;
	} /* end if (file) */

	return (rs >= 0) ? f_write : rs ;
}
/* end subroutine (proctouchfile) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,DISP *dop)
{
	int		rs = SR_OK ;
	int		ai ;
	int		pan = 0 ;
	int		f ;
	const char	*cp ;

	if (pip == NULL) return SR_FAULT ;

	for (ai = 1 ; ai < aip->argc ; ai += 1) {
	    f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	    f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	    if (f) {
	        cp = aip->argv[ai] ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main/procargs: a=%s\n",cp) ;
#endif
	        if (cp[0] != '\0') {
	            pan += 1 ;
	            rs = procfile(pip,dop,cp) ;
	        }
	    }
	    if (rs >= 0) rs = lib_sigterm() ;
	    if (rs >= 0) rs = lib_sigintr() ;
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/procargs: ret rs=%d pan=%u\n",rs,pan) ;
#endif

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargs) */


static int procargfile(PROGINFO *pip,DISP *dop,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;

	if ((afn != NULL) && (afn[0] != '\0')) {
	    SHIO	afile, *afp = &afile ;

	    if (afn[0] == '-') afn = STDINFNAME ;

	    if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        int		cl ;
	        const char	*cp ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	                    lbuf[(cp+cl)-lbuf] = '\0' ;
	                    pan += 1 ;
	                    rs = procfile(pip,dop,cp) ;
	                }
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = shio_close(afp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else if (! pip->f.quiet) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt ;
	        fmt = "%s: unaccessible (%d) afile=%s\n" ;
	        shio_printf(pip->efp,fmt,pn,rs,afn) ;
	    } /* end if */

	} /* end if (non-zero) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/procargfile: ret rs=%d pan=%u\n",rs,pan) ;
#endif

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargfile) */


static int procfile(PROGINFO *pip,DISP *dop,cchar *fname)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	lip->c_processed += 1 ;
	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: file=%s\n" ;
	    shio_printf(pip->efp,fmt,pn,fname) ;
	}

	if (fname[0] != '-') {
	    USTAT	sb ;
	    if ((rs = u_stat(fname,&sb)) >= 0) {
	        if ((rs = sperm(&lip->id,&sb,R_OK)) >= 0) {
	            rs = disp_addwork(dop,fname,-1) ;
	        } else if (isNotAccess(rs)) {
	            rs = SR_OK ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	} /* end if (flie check) */

	return rs ;
}
/* end subroutine (procfile) */


static int procfiler(PROGINFO *pip,cchar *name)
{
	USTAT		sb ;
	int		rs ;
	int		f_remove = FALSE ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if (name == NULL) return SR_FAULT ;

	if ((name[0] == '\0') || (name[0] == '-')) return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procfiler: ent name=%s\n",name) ;
#endif 

/* is the file there at all? */

	if ((rs = u_stat(name,&sb)) >= 0) {
	    if (S_ISREG(sb.st_mode)) {
	        const time_t	mo = sb.st_mtime ;
		if ((rs = procfilerdeps(pip,name,mo)) > 0) {
		    f_remove = TRUE ;
		} else if (rs == 0) {
	            if ((rs = procfilerfinds(pip,name,mo)) > 0) {
	                f_remove = TRUE ;
	            }
	        } /* end if (procfilerdeps) */
	    } /* end if (regular file) */
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	} /* end if (u_stat) */

	if (pip->debuglevel > 0) {
	    fmt = "%s: procfiler (%d:%u)\n" ;
	    proceprintf(pip,fmt,pn,rs,f_remove) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procfiler: ret rs=%d f=%u\n",rs,f_remove) ;
#endif 

	return (rs >= 0) ? f_remove : rs ;
}
/* end subroutine (procfiler) */


static int procfilerdeps(PROGINFO *pip,cchar *name,time_t mo)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		i ;
	int		f_done = FALSE ;
	for (i = 0 ; deps[i] != NULL ; i += 1) {
	    cchar	*dep = deps[i] ;
	    char	dbuf[MAXPATHLEN+1] ;
	    if ((rs = mkaltext(dbuf,name,dep)) >= 0) {
	        USTAT	sb ;
	        if ((rs = u_stat(dbuf,&sb)) >= 0) {
	            const time_t 	mc = sb.st_mtime ;
	            if (mc > mo) {
	                f_done = TRUE ;
	                if (! lip->f.nochange) rs = u_unlink(name) ;
	                if (rs >= 0) {
	                    rs = procfiletell(pip,dbuf,name) ;
	                }
		    } /* end if (dep is newer) */
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        } /* end if (u_stat) */
	    } /* end if (mkaltext) */
	    if (f_done) break ;
	    if (rs < 0) break ;
	} /* end for */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procfilerdeps: ret rs=%d f_done=%u\n",rs,f_done) ;
#endif 
	return (rs >= 0) ? f_done : rs ;
}
/* end subroutine (procfilerdeps) */


static int procfilerfinds(PROGINFO *pip,cchar *name,time_t mo)
{
	int		rs = SR_OK ;
	int		i ;
	int		f_done = FALSE ;
	for (i = 0 ; deps[i] != NULL ; i += 1) {
	    cchar	*dep = deps[i] ;
	    char	dbuf[MAXPATHLEN+1] ;
	    if (strcmp(dep,"h") != 0) {
	        if ((rs = mkaltext(dbuf,name,dep)) >= 0) {
	            USTAT	sb ;
	            if ((rs = u_stat(dbuf,&sb)) >= 0) {
		        if ((rs = procfilefind(pip,name,mo,dbuf)) > 0) {
			    f_done = TRUE ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procfilerfinds: mid2 rs=%d\n",rs) ;
#endif 
			}
		    } else if (isNotPresent(rs)) {
		        rs = SR_OK ;
		    }
	        } /* end if (mkaltext) */
	    } /* end if (not a header file) */
	    if (f_done) break ;
	    if (rs < 0) break ;
	} /* end for */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procfilerfinds: ret rs=%d f_done=%u\n",rs,f_done) ;
#endif 
	return (rs >= 0) ? f_done : rs ;
}
/* end subroutine (procfilerfinds) */


static int procfilefind(PROGINFO *pip,cchar *name,time_t mo,cchar *dbuf)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		f_remove = FALSE ;
	char		obuf[MAXPATHLEN+1] ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procfilefind: ent name=%s\n",name) ;
#endif 
	if ((rs = procdeps_check(pip,obuf,mo,dbuf)) > 0) {
	    cchar	*dname = dbuf ;
	    f_remove = TRUE ;
	    if (! lip->f.nochange) rs = u_unlink(name) ;
	    if (rs >= 0) {
	        if (obuf[0] != '\0') {
	            dname = obuf ;
	            if (strncmp(obuf,"./",2) == 0) {
	                dname = (obuf+ 2) ;
	            }
	        } /* end if */
	        rs = procfiletell(pip,dname,name) ;
	    } /* end if (ok) */
	} /* end if (procdeps_check) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procfiler: ret rs=%d f=%u\n",rs,f_remove) ;
#endif 
	return (rs >= 0) ? f_remove : rs ;
}
/* end subroutine (procfilefind) */


static int procfiletell(PROGINFO *pip,cchar *dbuf,cchar *name)
{
	int		rs = SR_OK ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	if (dbuf == NULL) dbuf = "" ;
	if (pip->debuglevel > 0) {
	    fmt = "%s: removing=%s dep=%s\n" ;
	    proceprintf(pip,fmt,pn,name,dbuf) ;
	}
	if (pip->verboselevel > 0) {
	    if (pip->verboselevel == 2) {
	        procout_printf(pip,"%s\n",name) ;
	    } else if (pip->verboselevel > 2) {
	        fmt = "%s %s\n" ;
	        procout_printf(pip,fmt,name,dbuf) ;
	    }
	} /* end if */
	return rs ;
}
/* end subroutine (procfiletell) */


/* check the dependencies against the original (object) file */
static int procdeps_check(PROGINFO *pip,char *rbuf,time_t mo,cchar *dname)
{
	vecpstr		deps ;
	const int	cs = LINEBUFLEN ;
	const int	vo = 0 ;
	int		rs ;
	int		rs1 ;
	int		f_remove = FALSE ;
	cchar		*pn = pip->progname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    LOCINFO	*lip = pip->lip ;
	    debugprintf("b_makesafe/procdeps_check: ent open-cache=%u\n",
	        lip->open.cache) ;
	    debugprintf("b_makesafe/procdeps_check: dname=%s\n",dname) ;
	}
#endif

	if (rbuf != NULL) rbuf[0] = '\0' ;

/* prepare to get the dependencies */

	if ((rs = vecpstr_start(&deps,NDEPS,cs,vo)) >= 0) {
	    if ((rs = procdeps_checker(pip,rbuf,mo,&deps,dname)) > 0) {
	        f_remove = TRUE ;
	    }
	    rs1 = vecpstr_finish(&deps) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

	if (pip->debuglevel > 0) {
	    proceprintf(pip,"%s: procdeps_check (%d)\n",pn,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_makesafe/procdeps_check: ret rs=%d f_rem=%u\n",
	        rs,f_remove) ;
#endif

	return (rs >= 0) ? f_remove : rs ;
}
/* end subroutine (procdeps_check) */


static int procdeps_checker(PROGINFO *pip,char *rbuf,time_t mo,
		vecpstr *dp,cchar *dname)
{
	LOCINFO		*lip = pip->lip ;
	VECOBJ		errs ;
	const int	size = sizeof(CPPERR) ;
	int		rs ;
	int		rs1 ;
	int		f_remove = FALSE ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procdeps_checker: ent n=%s\n",dname) ;
#endif 
	if ((rs = vecobj_start(&errs,size,10,0)) >= 0) {
	    if ((rs = procdeps_get(pip,dp,&errs,dname)) >= 0) {
	        USTAT	sb ;
	        int	i ;
	        cchar	*cp ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            for (i = 0 ; vecpstr_get(dp,i,&cp) >= 0 ; i += 1) {
	                if (cp != NULL) {
	                    debugprintf("b_makesafe/procdeps_check: "
	                        "¤ dfname=%s\n",cp) ;
	                }
	            }
	        }
#endif /* CF_DEBUG */
/* pop our file if any dependency is younger */
	        for (i = 0 ; vecpstr_get(dp,i,&cp) >= 0 ; i += 1) {
	            if (cp != NULL) {
	                time_t	mtime = 0 ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("b_makesafe/procdeps_check: "
				"dep=%s\n",cp) ;
#endif
	                if (lip->open.cache) {
	                    rs = locinfo_alreadylookup(lip,cp,-1,&mtime) ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("b_makesafe/procdeps_check: "
	                            "locinfo_alreadylookup() rs=%d\n",rs) ;
#endif
	                } else {
	                    rs = u_stat(cp,&sb) ;
	                    mtime = sb.st_mtime ;
	                } /* end if */
#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    char	timebuf[TIMEBUFLEN + 1] ;
	                    debugprintf("b_makesafe/procdeps_check: "
	                        "stat rs=%d mtime=%s\n",
	                        rs,timestr_log(mtime,timebuf)) ;
	                }
#endif
	                if ((rs >= 0) && (mtime > mo)) {
#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("b_makesafe/procdeps_check: "
					"remove\n") ;
#endif
	                    if (rbuf != NULL) mkpath1(rbuf,cp) ;
	                    f_remove = TRUE ;
	                    break ;
	                } else if (isNotPresent(rs)) {
	                    rs = SR_OK ;
	                }
	            }
	            if (rs < 0) break ;
	        } /* end for */
/* print out any errors that there may be */
	        if ((rs >= 0) && (! pip->f.quiet)) {
	            CPPERR	*ep ;
	            fmt = "%s: dep=%s:%u missing inc=%s\n" ;
	            for (i = 0 ; vecobj_get(&errs,i,&ep) >= 0 ; i += 1) {
	                if (ep != NULL) {
			    cchar	*ifn = ep->ifname ;
	                    proceprintf(pip,fmt,pn,ep->fname,ep->line,ifn) ;
	                }
	            } /* end for */
	        } /* end if (missing include files) */
	        {
	            CPPERR	*ep ;
	            for (i = 0 ; vecobj_get(&errs,i,&ep) >= 0 ; i += 1) {
	                if (ep != NULL) {
	                    cpperr_finish(ep) ;
	                }
	            } /* end for */
	        } /* end block */
	    } /* end if (procdeps_get) */
	    rs1 = vecobj_finish(&errs) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecobj_start) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procdeps_checker: ret rs=%d f=%u\n",rs,f_remove) ;
#endif 
	return (rs >= 0) ? f_remove : rs ;
}
/* end subroutine (procdeps_checker) */


/* get the dependencies for the given file */
static int procdeps_get(PROGINFO *pip,vecpstr *dp,VECOBJ *errp,cchar *fname)
{
	LOCINFO		*lip = pip->lip ;
	const mode_t	operms = 0664 ;
	int		rs ;
	int		rs1 ;
	int		oflags ;
	char		tbuf[MAXPATHLEN + 1] ;
	char		efname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_makesafe/procdeps_get: ent fn=%s\n",fname) ;
#endif

/* prepare STDERR for the program */

	efname[0] = '\0' ;
	mkpath2(tbuf,lip->jobdname,"seXXXXXXXXXX") ;

	oflags = O_RDWR ;
	if ((rs = opentmpfile(tbuf,oflags,operms,efname)) >= 0) {
	    const int	efd = rs ;
	    if (efname[0] != '\0') {
	        rs = u_unlink(efname) ;
	        efname[0] = '\0' ;
	    }
	    if (rs >= 0) {
	        VECSTR		args ;
	        const int	vo = VECSTR_OCOMPACT ;
	        if ((rs = vecstr_start(&args,10,vo)) >= 0) {
	            if ((rs = procdeps_loadargs(pip,&args,fname)) >= 0) {
	                cchar	**av ;
	                if ((rs = vecstr_getvec(&args,&av)) >= 0) {
	                    SPAWNPROC	psa ;
	                    cchar	*pf = lip->prog_cpp ;
	                    cchar	**ev = pip->envv ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(5)) {
	                        int	i ;
	                        for (i = 0 ; av[i] != NULL ; i += 1) {
	                            debugprintf("procdeps_get: arg%u=>%s<\n",
	                                i,av[i]) ;
	                        }
	                    }
#endif
	                    memset(&psa,0,sizeof(SPAWNPROC)) ;
	                    psa.disp[0] = SPAWNPROC_DNULL ;
	                    psa.disp[1] = SPAWNPROC_DCREATE ;
	                    psa.disp[2] = SPAWNPROC_DDUP ;
	                    psa.fd[2] = efd ;
	                    if ((rs = spawnproc(&psa,pf,av,ev)) >= 0) {
	                        const pid_t	pid = rs ;
	                        const int	w = WUNTRACED ;
	                        const int	ofd = psa.fd[1] ;
				int		cstat ;
#if	CF_DEBUG
	                        if (DEBUGLEVEL(5)) {
	                            debugprintf("b_makesafe/procdeps_get: "
	                                "u_wait() rs=%d\n", rs) ;
	                        }
#endif
	                        if ((rs = proclines(pip,dp,ofd)) >= 0) {
	                            rs = 0 ;
	                            while (rs == 0) {
	                                rs = u_waitpid(pid,&cstat,w) ;
	                                if (rs == SR_INTR) rs = SR_OK ;
	                            }
	                            if (rs >= 0) {
	                                rs = procerr(pip,errp,efd) ;
	                            }
			        } else {
	                            u_waitpid(pid,&cstat,w) ;
	                        }
	                        u_close(ofd) ;
	                    } /* end if (spawnproc) */
	                } /* end if (vecstr_getvec) */
	            } /* end if (procdeps_loadargs) */
	            rs1 = vecstr_finish(&args) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (vecstr-args) */
	    } /* end if (ok) */
	    u_close(efd) ;
	} /* end if (opentmpfile) */

/* done */

	if ((pip->debuglevel > 0) && (rs < 0)) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: procdeps_get (%d)\n" ;
	    proceprintf(pip,fmt,pn,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procdeps_get: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdeps_get) */


static int procdeps_loadargs(PROGINFO *pip,vecstr *alp,cchar *fn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		cl ;
	cchar		*cp ;
	if ((cl = sfbasename(lip->prog_cpp,-1,&cp)) > 0) {
	    vecstr_add(alp,cp,cl) ;
	    vecstr_add(alp,"-M",-1) ;
	    if ((rs = procdeps_incargs(pip,alp)) >= 0) {
	        rs = vecstr_add(alp,fn,-1) ;
	    }
	} else {
	    rs = SR_NOENT ;
	}
	return rs ;
}
/* end subroutine (procdeps_loadargs) */


static int procdeps_incargs(PROGINFO *pip,vecstr *alp)
{
	const int	alen = (MAXNAMELEN+MAXPATHLEN) ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	char		*abuf ;
	if ((rs = uc_malloc((alen+1),&abuf)) >= 0) {
	    LOCINFO	*lip = pip->lip ;
	    LOCINFO_CUR	cur ;
	    strwcpy(abuf,"-I",2) ;
	    if ((rs = locinfo_incbegin(lip,&cur)) >= 0) {
	        char	*ap = (abuf+2) ;
	        int	al = (alen-2) ;
	        while (rs >= 0) {
	            int	rs1 = locinfo_incenum(lip,&cur,ap,al) ;
	            if (rs1 == SR_NOTFOUND) break ;
	            rs = rs1 ;
	            if (rs >= 0) {
#if	CF_DEBUG
	                if (DEBUGLEVEL(5))
	                    debugprintf("b_makesafe/procdeps_get: add inc=%t\n",
	                        ap,rs) ;
#endif
	                c += 1 ;
	                rs = vecstr_add(alp,abuf,-1) ;
	            }
	        } /* end while */
	        rs1 = locinfo_incend(lip,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */
	    rs1 = uc_free(abuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (m-a-f) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdeps_incargs) */


/* process the lines that contain dependency names */
static int proclines(PROGINFO *pip,vecpstr *dp,int fd)
{
	FILEBUF		buf ;
	const int	to = pip->to_read ;
	int		rs ;
	int		rs1 ;
	int		tlen = 0 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    struct ustat	sb ;
	    int	rs1 ;
#if	CF_TESTSLEEP
	    sleep(5) ;
#endif
	    debugprintf("proclines: ent to=%u\n",to) ;
	    rs1 = u_fstat(fd,&sb) ;
	    debugprintf("proclines: u_fstat() rs=%d\n",rs1) ;
	    debugprintf("proclines: fsize=%llu\n",sb.st_size) ;
	    debugprintf("proclines: mode=\\o%08o\n",sb.st_mode) ;
	}
#endif /* CF_DEBUG */

	if ((rs = filebuf_start(&buf,fd,0L,FBUFLEN,0)) >= 0) {
	    LSTATE	ls ;
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    memset(&ls,0,sizeof(struct lstate)) ;
	    while ((rs = filebuf_readline(&buf,lbuf,llen,to)) > 0) {
	        tlen += rs ;
	        len = rs ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("proclines: line=>%t<\n",
	                lbuf,
	                ((lbuf[len - 1] == '\n') ? (len - 1) : len)) ;
#endif /* CF_DEBUG */

	        rs = procline(pip,dp,&ls,lbuf,len) ;
	        c += rs ;

	        if (rs < 0) break ;
	    } /* end while */

	    rs1 = filebuf_finish(&buf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if (rs >= 0) {
	        fmt = "%s: proclines (%d)\n" ;
	        proceprintf(pip,fmt,pn,tlen) ;
	    } else {
	        fmt = "%s: proclines (%d)\n" ;
	        proceprintf(pip,fmt,pn,rs) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("b_makesafe/proclines: ret rs=%d c=%u\n",rs,c) ;
	    debugprintf("b_makesafe/proclines: ret tlen=%u\n",tlen) ;
	}
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proclines) */


/* process a dependecy line */
static int procline(PROGINFO *pip,vecpstr *dp,LSTATE *lsp,cchar *lbuf,int len)
{
	int		rs = SR_OK ;
	int		sl, cl ;
	int		f_continue = FALSE ;
	int		c = 0 ;
	const char	*tp, *sp, *cp ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    const int	f = lsp->f_continue ;
	    debugprintf("b_makesafe/procline: ent\n") ;
	    debugprintf("b_makesafe/procline: l=%t\n",lbuf,len) ;
	    debugprintf("b_makesafe/procline: f_con=%u\n",f) ;
	}
#endif

	if ((len > 1) && (lbuf[len - 1] == CH_BSLASH)) {
	    f_continue = TRUE ;
	    len -= 1 ;
	}

	sp = lbuf ;
	sl = len ;
	if (! lsp->f_continue) {
	    if ((tp = strnchr(sp,sl,':')) != NULL) {
	        sl -= ((tp+1) - sp) ;
	        sp = (tp+1) ;
	    }
	}

	while ((cl = nextfield(sp,sl,&cp)) > 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("b_makesafe/procline: dep=%t\n",cp,cl) ;
#endif

	    rs = vecpstr_adduniq(dp,cp,cl) ;
	    if (rs < INT_MAX) c += 1 ;

	    sl -= ((cp + cl) - sp) ;
	    sp = (cp + cl) ;

	    if (rs < 0) break ;
	} /* end while */
	lsp->f_continue = f_continue ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_makesafe/procline: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procline) */


/* process the error output */
static int procerr(PROGINFO *pip,VECOBJ *errp,int fd_err)
{
	struct ustat	sb ;
	FILEBUF		buf ;
	const int	fsize = FBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		to = pip->to_read ;

	if ((rs = u_fstat(fd_err,&sb)) >= 0) {
	    if (sb.st_size > 0) {
	        rs = u_rewind(fd_err) ;
	    }
	}

	if ((rs >= 0) && (sb.st_size > 0)) {
	    if ((rs = filebuf_start(&buf,fd_err,0L,fsize,0)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b_makesafe/procdeps_get: have error size=%u\n",
	                sb.st_size) ;
#endif

	        while (rs >= 0) {
	            rs = filebuf_readline(&buf,lbuf,llen,to) ;
	            len = rs ;
	            if (rs <= 0) break ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("b_makesafe/procdeps_get: line> %t\n",
	                    lbuf,len) ;
#endif

	            rs = procerrline(pip,errp,lbuf,len) ;

	        } /* end while */

/* print out the errors */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            int	i ;
	            CPPERR	*ep ;
	            debugprintf("depget: errors\n") ;
	            for (i = 0 ; vecobj_get(errp,i,&ep) >= 0 ; i += 1) {
	                if (ep == NULL) continue ;
	                debugprintf("b_makesafe/procerr: fname=%s:%u inc=%s\n",
	                    ep->fname,ep->line,ep->ifname) ;
	            }
	        }
#endif /* CF_DEBUG */

	        rs1 = filebuf_finish(&buf) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (filebuf) */
	} /* end if (stat) */

	return rs ;
}
/* end subroutine (procerr) */


/* process an error line */
static int procerrline(PROGINFO *pip,VECOBJ *errp,cchar *lbuf,int len)
{
	int		rs = SR_OK ;
	int		cl, cl1, cl2 ;
	const char	*cp, *cp1, *cp2 ;

	if (pip == NULL) return SR_FAULT ;
	if ((cl2 = sfsub(lbuf,len,errsub2,&cp2)) >= 0) {
	    if ((cl1 = sfsub(lbuf,MIN(len,(cp2-lbuf)),errsub1,&cp1)) > 0) {
	        if ((cl = sfdequote(lbuf,(cp1-lbuf),&cp)) > 0) {
	            const int	dl = (cp2 - (cp1 + cl1)) ;
		    int		line ;
	            const char	*dp = (cp1 + cl1) ;
	            if ((rs = cfdeci(dp,dl,&line)) >= 0) {
	                CPPERR	e ;
	                if ((rs = cpperr_start(&e,line,cp,cl)) >= 0) {
	                    int		sl = (len-((cp2+cl2)-lbuf)) ;
	                    cchar	*sp = (cp2 + cl2) ;
	                    if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	                        if ((rs = cpperr_ifname(&e,cp,cl)) >= 0) {
	                            rs = vecobj_add(errp,&e) ;
	                        }
	                    }
	                    if (rs < 0)
	                        cpperr_finish(&e) ;
	                } /* end if (cpperr_start) */
	            } /* end if (cfdeci) */
	        } /* end if (sfdequote) */
	    } /* end if (sfsub) */
	} /* end if (sfsub) */

	return rs ;
}
/* end subroutine (procerrline) */


static int procout_begin(PROGINFO *pip,void *ofp,const char *ofn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if (lip->f.print || (pip->verboselevel > 0)) {
	    if ((rs = shio_open(ofp,ofn,"wct",0644)) >= 0) {
	        if ((rs = ptm_create(&lip->ofm,NULL)) >= 0) {
	            lip->ofp = ofp ;
	        }
	        if (rs < 0) {
	            shio_close(lip->ofp) ;
	        }
	    } /* end if (shio) */
	} /* end if */

	return rs ;
}
/* end subroutine (procout_begin) */


static int procout_end(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->f.print || (pip->verboselevel > 0)) {
	    rs1 = ptm_destroy(&lip->ofm) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = shio_close(lip->ofp) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->ofp = NULL ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procout_end: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procout_end) */


static int procout_printf(PROGINFO *pip,cchar *fmt,...)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if ((lip->ofp != NULL) && (pip->verboselevel > 0)) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    if ((rs = ptm_lock(&lip->ofm)) >= 0) {
	        rs = shio_vprintf(lip->ofp,fmt,ap) ;
	        len = rs ;
	        ptm_unlock(&lip->ofm) ;
	    } /* end if (mutex) */
	    va_end(ap) ;
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procout_printf) */


static int cpperr_start(CPPERR *ep,int line,cchar *fp,int fl)
{
	int		rs ;
	const char	*cp ;

	if (fp == NULL) return SR_FAULT ;

	ep->line = line ;
	ep->fname = NULL ;
	ep->ifname = NULL ;
	if ((rs = uc_mallocstrw(fp,fl,&cp)) >= 0) {
	    ep->fname = cp ;
	}

	return rs ;
}
/* end subroutine (cpperr_start) */


static int cpperr_ifname(CPPERR *ep,cchar *fp,int fl)
{
	int		rs ;
	const char	*cp ;

	if (fp == NULL) return SR_FAULT ;

	if ((rs = uc_mallocstrw(fp,fl,&cp)) >= 0) {
	    ep->ifname = cp ;
	}

	return rs ;
}
/* end subroutine (cpperr_ifname) */


static int cpperr_finish(CPPERR *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep->fname != NULL) {
	    rs1 = uc_free(ep->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->fname = NULL ;
	}

	if (ep->ifname != NULL) {
	    rs1 = uc_free(ep->ifname) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->ifname = NULL ;
	}

	ep->line = 0 ;
	return rs ;
}
/* end subroutine (cpperr_finish) */


static int disp_start(DISP *dop,DISP_ARGS *wap)
{
	PROGINFO	*pip ;
	int		rs ;

	if (dop == NULL) return SR_FAULT ;
	if (wap == NULL) return SR_FAULT ;

	pip = wap->pip ;

	memset(dop,0,sizeof(DISP)) ;
	dop->pip = pip ;
	dop->a = *wap ;
	dop->nthr = wap->npar ;

	if ((rs = fsi_start(&dop->wq)) >= 0) {
	    if ((rs = psem_create(&dop->wq_sem,FALSE,0)) >= 0) {
	        if ((rs = ptm_create(&dop->m,NULL)) >= 0) {
	            if ((rs = ptc_create(&dop->cond,NULL)) >= 0) {
	                const int	size = (dop->nthr * sizeof(DISP_THR)) ;
	                void		*p ;
	                if ((rs = uc_malloc(size,&p)) >= 0) {
	                    dop->threads = p ;
			    memset(p,0,size) ;
	                    rs = disp_starter(dop) ;
	                    if (rs < 0) {
	                        uc_free(dop->threads) ;
	                        dop->threads = NULL ;
	                    }
	                } /* end if (m-a) */
	                if (rs < 0)
	                    ptc_destroy(&dop->cond) ;
	            } /* end if (ptc_create) */
	            if (rs < 0)
	                ptm_destroy(&dop->m) ;
	        } /* end if (ptm_create) */
	        if (rs < 0)
	            psem_destroy(&dop->wq_sem) ;
	    } /* end if (psem_create) */
	    if (rs < 0)
	        fsi_finish(&dop->wq) ;
	} /* end if (fsi) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/disp_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (disp_start) */


static int disp_starter(DISP *dop)
{
	PROGINFO	*pip = dop->pip ;
	pthread_t	tid ;
	int		rs = SR_OK ;
	int		i ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/disp_starter: ent nthr=%u\n",dop->nthr) ;
#endif

	for (i = 0 ; (rs >= 0) && (i < dop->nthr) ; i += 1) {
	    uptsub_t	fn = (uptsub_t) disp_worker ;
	    if ((rs = uptcreate(&tid,NULL,fn,dop)) >= 0) {
	        dop->threads[i].tid = tid ;
	        dop->threads[i].f_active = TRUE ;
	    }
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/disp_starter: i=%u uptcreate() rs=%d tid=%u\n",
		i,rs,tid) ;
#endif
	} /* end for */
	    if (rs >= 0) {
		rs = disp_readyset(dop) ;
	    }

	if (rs < 0) {
	    int		n = i ;
	    dop->f_exit = TRUE ;
	    for (i = 0 ; i < n ; i += 1) {
	        psem_post(&dop->wq_sem) ;
	    }
	    for (i = 0 ; i < n ; i += 1) {
	        tid = dop->threads[i].tid ;
	        uptjoin(tid,NULL) ;
	        dop->threads[i].f_active = FALSE ;
	    }
	} /* end if (failure) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/disp_starter: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (disp_starter) */


static int disp_finish(DISP *dop,int f_abort)
{
	PROGINFO	*pip = dop->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;

	if (pip == NULL) return SR_FAULT ; /* lint */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_makesafe/disp_finish: ent f_abort=%u\n",f_abort) ;
#endif

	dop->f_done = TRUE ;		/* exit when no more work */
	if (f_abort) dop->f_exit = TRUE ;

	for (i = 0 ; i < dop->nthr ; i += 1) {
	    rs1 = psem_post(&dop->wq_sem) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_makesafe/disp_finish: mid1 rs=%d\n",rs) ;
#endif

	if (dop->threads != NULL) {
	    DISP_THR	*dtp ;
	    pthread_t	tid ;
	    int		trs ;
	    for (i = 0 ; i < dop->nthr ; i += 1) {
	        dtp = (dop->threads+i) ;
	        if (dtp->f_active) {
	            dtp->f_active = FALSE ;
	            tid = dtp->tid ;
	            rs1 = uptjoin(tid,&trs) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    	debugprintf("b_makesafe/disp_finish: "
			"i=%u uptjoin() tid=%u rs=%d trs=%d\n",i,tid,rs,trs) ;
#endif
	            if (rs >= 0) rs = rs1 ;
	            if (rs >= 0) rs = trs ;
	            if (rs > 0) c += trs ;
	        } /* end if (active) */
	    } /* end for */
	    rs1 = uc_free(dop->threads) ;
	    if (rs >= 0) rs = rs1 ;
	    dop->threads = NULL ;
	} /* end if (threads) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_makesafe/disp_finish: mid2 rs=%d\n",rs) ;
#endif

	rs1 = ptc_destroy(&dop->cond) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&dop->m) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = psem_destroy(&dop->wq_sem) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = fsi_finish(&dop->wq) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/disp_finish: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (disp_finish) */


static int disp_addwork(DISP *dop,cchar *tagbuf,int taglen)
{
	int		rs ;
	if ((rs = fsi_add(&dop->wq,tagbuf,taglen)) >= 0) {
	    rs = psem_post(&dop->wq_sem) ;
	}
	return rs ;
}
/* end subroutine (disp_addwork) */


/* this is the worker thread */
static int disp_worker(DISP *dop)
{
	PROGINFO	*pip = dop->pip ;
	const int	rlen = MAXPATHLEN ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	char		rbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    pthread_t	tid = pthread_self() ;
	    debugprintf("mkkey/worker: ent tid=%u\n",tid) ;
	}
#endif

	while ((rs = psem_wait(&dop->wq_sem)) >= 0) {
	    if (dop->f_exit) break ;

	    if ((rs = fsi_remove(&dop->wq,rbuf,rlen)) >= 0) {
	        if ((rs = procfiler(pip,rbuf)) >= 0) {
	            if (rs > 0) c += 1 ;
	            rs = disp_taskdone(dop) ;
	        }
	    } else if (rs == SR_NOTFOUND) {
	        rs = SR_OK ;
	        if (dop->f_done) break ;
	    }

	    if (rs < 0) break ;
	} /* end while (server loop) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    pthread_t	tid = pthread_self() ;
	    debugprintf("mkkey/worker: tid=%u ret rs=%d c=%u\n",tid,rs,c) ;
	}
#endif

	rs1 = disp_exiting(dop) ;
	if (rs >= 0) rs = rs1 ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (disp_worker) */


/* worker thread calls this to register a task completion */
static int disp_taskdone(DISP *dop)
{
	PTM		*mp = &dop->m ;
	int		rs ;
	int		rs1 ;
	if ((rs = ptm_lock(mp)) >= 0) {
	    dop->tasks += 1 ;
	    if (! dop->f_wakeup) {
	        dop->f_wakeup = TRUE ;
	        rs = ptc_signal(&dop->cond) ;
	    }
	    rs1 = ptm_unlock(mp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */
	return rs ;
}
/* end subroutine (disp_taskdone) */


static int disp_exiting(DISP *dop)
{
	DISP_THR	*dtp ;
	int		rs ;
	int		i = 0 ;
	if ((rs = disp_getourthr(dop,&dtp)) >= 0) {
	    i = rs ;
	    dtp->f_exiting = TRUE ;
	    rs = ptc_signal(&dop->cond) ;
	} /* end if (disp_getourthr) */
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (disp_exiting) */


static int disp_allexiting(DISP *dop)
{
	DISP_THR	*threads = dop->threads ;
	int		rs = SR_OK ;
	int		i ;
	int		f = TRUE ;
	for (i = 0 ; i < dop->nthr ; i += 1) {
	    f = f && threads[i].f_exiting ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (disp_allexiting) */


/* |main| calls this to intermittently wait for worker completion */
static int disp_waiting(DISP *dop)
{
	PTM		*mp = &dop->m ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if ((rs = ptm_lock(mp)) >= 0) {
	    while ((rs = disp_notready(dop,&c)) > 0) {
	        rs = ptc_wait(&dop->cond,mp) ;
	        if (rs < 0) break ;
	    } /* end while */
	    dop->f_wakeup = FALSE ;
	    rs1 = ptm_unlock(mp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */
#if	CF_DEBUGS
	debugprintf("b_makenewer/disp_waiting: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (disp_waiting) */


/* helper function for |disp_waiting()| above */
static int disp_notready(DISP *dop,int *cp)
{
	int		rs ;
	int		f = FALSE ;
#if	CF_DEBUGS
	debugprintf("b_makesafe/disp_notready: ent f_wakeup=%u\n",
		dop->f_wakeup) ;
#endif
	*cp = 0 ;
	if ((rs = fsi_count(&dop->wq)) > 0) {
	    *cp = rs ;
#if	CF_DEBUGS
	    debugprintf("b_makesafe/disp_notready: c=%u\n",rs) ;
#endif
	    if (! dop->f_wakeup) {
	        if ((rs = disp_allexiting(dop)) == 0) {
#if	CF_DEBUGS
	            debugprintf("b_makesafe/disp_notready: not-allexiting\n") ;
#endif
		    f = TRUE ;
		}
	    }
	}
#if	CF_DEBUGS
	debugprintf("b_makesafe/disp_notready: ret rs=%d f=%u c=%u\n",
		rs,f,*cp) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (disp_notready) */


static int disp_getourthr(DISP *dop,DISP_THR **rpp)
{
	int		rs ;
	int		i = 0 ;
	if ((rs = disp_readywait(dop)) >= 0) {
	    DISP_THR	*dtp ;
	    pthread_t	tid = pthread_self() ;
	    int		f = FALSE ;
	    for (i = 0 ; i < dop->nthr ; i += 1) {
	        dtp = (dop->threads+i) ;
	        f = uptequal(dtp->tid,tid) ;
	        if (f) break ;
	    } /* end for */
	    if (f) {
	        if (rpp != NULL) *rpp = dtp ;
	    } else {
	        rs = SR_BUGCHECK ;
	    }
	} /* end if (disp_readywait) */
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (disp_getourthr) */


/* main-thread calls this to indicate sub-threads can read completed object */
static int disp_readyset(DISP *dop)
{
	PTM		*mp = &dop->m ;
	int		rs ;
	int		rs1 ;
	if ((rs = ptm_lock(mp)) >= 0) {
	    {
	        dop->f_ready = TRUE ;
	        rs = ptc_broadcast(&dop->cond) ; /* 0-bit semaphore */
	    }
	    rs1 = ptm_unlock(mp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */
	return rs ;
}
/* end subroutine (disp_readyset) */


/* sub-threads call this to wait until object is ready */
static int disp_readywait(DISP *dop)
{
	PTM		*mp = &dop->m ;
	int		rs ;
	int		rs1 ;
	if ((rs = ptm_lock(mp)) >= 0) {
	    while ((! dop->f_ready) && (! dop->f_exit)) {
	        rs = ptc_wait(&dop->cond,mp) ;
		if (rs < 0) break ;
	    } /* end while */
	    rs1 = ptm_unlock(mp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */
	return rs ;
}
/* end subroutine (disp_readywait) */


#if	CF_DISPABORT
static int disp_abort(DISP *dop)
{
	dop->f_exit = TRUE ;
	return SR_OK ;
}
/* end subroutine (disp_abort) */
#endif /* CF_DISPABORT */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to_tmpfiles = TO_TMPFILES ;
	lip->f.cache = OPT_CACHE ;

	if ((rs = ids_load(&lip->id)) >= 0) {
	    if ((rs = ptm_create(&lip->efm,NULL)) >= 0) {
	        if (( rs = dirlist_start(&lip->incs)) >= 0) {
	            lip->open.incs = TRUE ;
	            rs = locinfo_alreadybegin(lip) ;
	            if (rs < 0)
	                dirlist_finish(&lip->incs) ;
	        }
	        if (rs < 0)
	            ptm_destroy(&lip->efm) ;
	    } /* end if (ptm_create) */
	    if (rs < 0)
	        ids_release(&lip->id) ;
	} /* end if (ids) */

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = locinfo_alreadyend(lip) ;
	if (rs >= 0) rs = rs1 ;

	if (lip->open.incs) {
	    rs1 = dirlist_finish(&lip->incs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = ptm_destroy(&lip->efm) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ids_release(&lip->id) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = locinfo_tmpdone(lip) ;
	if (rs >= 0) rs = rs1 ;

	if (lip->jobdname != NULL) {
	    rs1 = uc_free(lip->jobdname) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->jobdname = NULL ;
	}

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


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


static int locinfo_jobdname(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	if (lip->jobdname == NULL) {
	    cchar	*pn = pip->progname ;
	    char	udname[MAXPATHLEN + 1] ;
	    if ((rs = mktmpuserdir(udname,"-",pn,0775)) >= 0) {
	        cchar	**vpp = &lip->jobdname ;
	        rs = locinfo_setentry(lip,vpp,udname,rs) ;
	    }
	} else {
	    rs = strlen(lip->jobdname) ;
	}
	return rs ;
}
/* end subroutine (locinfo_jobdname) */


/* this conditionally spawns an independent thread for maintenance */
static int locinfo_tmpcheck(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip->jobdname != NULL) {
	    TMTIME	t ;
	    if ((rs = tmtime_localtime(&t,pip->daytime)) >= 0) {
	        if ((t.hour >= HOUR_MAINT) && lip->f.maint) {
	            uptsub_t	thr = (uptsub_t) locinfo_tmpmaint ;
	            pthread_t	tid ;
	            if ((rs = uptcreate(&tid,NULL,thr,lip)) >= 0) {
	                rs = 1 ;
	                lip->tid = tid ;
	                lip->f.tmpmaint = TRUE ;
	            } /* end if (uptcreate) */
	        } /* end if (after hours) */
	    } /* end if (tmtime_localtime) */
	} /* end if (jobdname) */

	return rs ;
}
/* end subroutine (locinfo_tmpcheck) */


/* this runs as an independent thread */
static int locinfo_tmpmaint(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	const int	to = lip->to_tmpfiles ;
	int		rs ;
	int		c = 0 ;
	int		f_need = lip->f.maint ;
	cchar		*dname = lip->jobdname ;
	char		tsfname[MAXPATHLEN+1] ;

	if ((rs = mkpath2(tsfname,dname,TSFNAME)) >= 0) {
	    const mode_t	om = 0666 ;
	    const int		of = (O_WRONLY|O_CREAT) ;
	    if ((rs = u_open(tsfname,of,om)) >= 0) {
	        USTAT		usb ;
	        const int	fd = rs ;
	        if ((rs = u_fstat(fd,&usb)) >= 0) {
	            time_t	dt = pip->daytime ;
	            if ((rs = locinfo_fchmodown(lip,fd,&usb,om)) >= 0) {
	                int	maintlapse = (dt - usb.st_mtime) ;
	                f_need = f_need || (usb.st_size == 0) ;
	                f_need = f_need || (maintlapse >= to) ;
	                if (f_need) {
	                    int		tl ;
	                    char	timebuf[TIMEBUFLEN + 3] ;
	                    timestr_log(dt,timebuf) ;
	                    tl = strlen(timebuf) ;
	                    timebuf[tl++] = '\n' ;
	                    rs = u_write(fd,timebuf,tl) ;
	                } /* end if (timed-out) */
	            } /* end if (locinfo_fchmodown) */
	        } /* end if (stat) */
	        u_close(fd) ;
	    } /* end if (open file) */
	} /* end if (mkpath timestamp) */

	if ((rs >= 0) && f_need) {
	    rs = rmdirfiles(dname,NULL,to) ;
	    c = rs ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_tmpmaint) */


static int locinfo_tmpdone(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->f.tmpmaint) {
	    int	trs ;
	    rs1 = uptjoin(lip->tid,&trs) ;
	    if (rs >= 0) rs = rs1 ;
	    if (rs >= 0) rs = trs ;
	}
	return rs ;
}
/* end subroutine (locinfo_tmpdone) */


static int locinfo_fchmodown(LOCINFO *lip,int fd,struct ustat *sbp,mode_t mm)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if ((sbp->st_size == 0) && (pip->euid == sbp->st_uid)) {
	    if ((sbp->st_mode & S_IAMB) != mm) {
	        if ((rs = locinfo_loadprids(lip)) >= 0) {
	            if ((rs = uc_fminmod(fd,mm)) >= 0) {
	                const uid_t	uid_pr = lip->uid_pr ;
	                const gid_t	gid_pr = lip->gid_pr ;
	                const int	n = _PC_CHOWN_RESTRICTED ;
	                if ((rs = u_fpathconf(fd,n,NULL)) == 0) {
	                    f = TRUE ;
	                    u_fchown(fd,uid_pr,gid_pr) ; /* may fail */
	                } else if (rs == SR_NOSYS) {
	                    rs = SR_OK ;
	                }
	            }
	        } /* end if (locinfo_loadprids) */
	    } /* end if (need change) */
	} /* end if (zero-file) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_fchmodown) */


static int locinfo_loadprids(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (lip->uid_pr < 0) {
	    USTAT	sb ;
	    if ((rs = u_stat(pip->pr,&sb)) >= 0) {
	        lip->uid_pr = sb.st_uid ;
	        lip->gid_pr = sb.st_gid ;
	    } /* end if (u_stat) */
	} /* end if (needed) */
	return rs ;
}
/* end subroutine (locinfo_loadprids) */


static int locinfo_alreadybegin(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (lip->f.cache) {
	    if ((rs = cachetime_start(&lip->mtdb)) >= 0) {
	        lip->open.cache = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (locinfo_alreadybegin) */


static int locinfo_alreadyend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.cache) {
	    lip->open.cache = FALSE ;
	    rs1 = cachetime_finish(&lip->mtdb) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (locinfo_alreadyend) */


static int locinfo_incdirs(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;
	if ((cp = getourenv(pip->envv,VARINCDIRS)) != NULL) {
	    rs = dirlist_adds(&lip->incs,cp,-1) ;
	    c = rs ;
	} /* end if */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_incdirs) */


static int locinfo_incadds(LOCINFO *lip,cchar *sp,int sl)
{
	DIRLIST		*dlp = &lip->incs ;
	int		rs = SR_OK ;

	if (lip->open.incs) {
	    rs = dirlist_adds(dlp,sp,sl) ;
	} else {
	    rs = SR_NOTOPEN ;
	}

	return rs ;
}
/* end subroutine (locinfo_incadds) */


static int locinfo_alreadystat(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if ((pip->verboselevel > 0) || (pip->debuglevel > 0)) {
	    if (lip->open.cache) {
	        CACHETIME_STATS	stat ;

	        if ((rs = cachetime_stats(&lip->mtdb,&stat)) >= 0) {
	            cchar	*pn = pip->progname ;
	            cchar	*fmt ;
	            if (pip->verboselevel >= 3) {
	                fmt = "cache requests=%u hits=%u misses=%u\n" ;
	                shio_printf(lip->ofp,fmt,stat.req,stat.hit,stat.miss) ;
	            }
	            if (pip->debuglevel > 0) {
	                fmt = "%s: cache requests=%u hits=%u misses=%u\n" ;
	                shio_printf(pip->efp,fmt,pn,
	                    stat.req,stat.hit,stat.miss) ;
	            }
	        } /* end if (successful) */

	    } /* end if (cache) */
	} /* end if (verbose or debug) */
	return rs ;
}
/* end subroutine (locinfo_alreadystat) */


static int locinfo_alreadylookup(LOCINFO *lip,cchar *cp,int cl,time_t *rtp)
{
	int		rs = SR_OK ;
	if (lip->open.cache) {
	    rs = cachetime_lookup(&lip->mtdb,cp,cl,rtp) ;
	} /* end if (cache) */
	return rs ;
}
/* end subroutine (locinfo_alreadylookup) */


static int locinfo_incbegin(LOCINFO *lip,LOCINFO_CUR *curp)
{
	DIRLIST		*dlp = &lip->incs ;
	DIRLIST_CUR	*dcp = &curp->c ;
	int		rs ;

	rs = dirlist_curbegin(dlp,dcp) ;

	return rs ;
}
/* end subroutine (locinfo_incbegin) */


static int locinfo_incenum(LOCINFO *lip,LOCINFO_CUR *curp,char *rbuf,int rlen)
{
	DIRLIST		*dlp = &lip->incs ;
	DIRLIST_CUR	*dcp = &curp->c ;
	int		rs ;

	rs = dirlist_enum(dlp,dcp,rbuf,rlen) ;

	return rs ;
}
/* end subroutine (locinfo_incenum) */


static int locinfo_incend(LOCINFO *lip,LOCINFO_CUR *curp)
{
	DIRLIST		*dlp = &lip->incs ;
	DIRLIST_CUR	*dcp = &curp->c ;
	int		rs ;

	rs = dirlist_curend(dlp,dcp) ;

	return rs ;
}
/* end subroutine (locinfo_incend) */


static int locinfo_ncpu(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (lip->ncpu == 0) {
	    PROGINFO	*pip = lip->pip ;
	    rs = getnprocessors(pip->envv,0) ;
	    lip->ncpu = rs ;
	} else {
	    rs = lip->ncpu ;
	}
	return rs ;
}
/* end subroutine (locinfo_ncpu) */


#if	CF_DEBUG && CF_DEBUGENV
static int debugdumpenv(cchar **envv)
{
	int		i = 0 ;
	debugprintf("main/debugdumpenv: env¬ {%p}\n",envv) ;
	for (i = 0 ; envv[i] != NULL ; i += 1) {
	    cchar	*ep = envv[i] ;
	    debugprintf("main/debugdumpenv: e=%t\n",
	        ep,strlinelen(ep,-1,40)) ;
	}
	return i ;
}
#endif /* CF_DEBUG */


#if	CF_WRFILE
static int wrfile(cchar *fn,int rfd)
{
	const int	of = (O_WRONLY|O_CREAT) ;
	const int	to = 30 ;
	const int	ro = FM_TIMED ;
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		tlen = 0 ;
	char		lbuf[LINEBUFLEN+1] ;
	if ((rs = u_open(fn,of,0666)) >= 0) {
	    const int	wfd = rs ;
	    while ((rs = uc_reade(rfd,lbuf,llen,to,ro)) > 0) {
	        tlen += rs ;
#if	CF_DEBUGS
	        debugprintf("b_makesafe/wrfile: u_read() rs=%d\n",rs) ;
#endif
	        rs = uc_writen(wfd,lbuf,rs) ;
	        if (rs < 0) break ;
	    } /* end while */
	    u_close(wfd) ;
	} /* end if (file) */
	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (wrfile) */
#endif /* CF_WRFILE */


