/* b_homepage (HOMEPAGE) */

/* this is a generic "main" module */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_DEBUGMOUT	0		/* debug memory-allocations */
#define	CF_DEBUGLEVEL	0		/* default debug-level */
#define	CF_PROCARGS	0		/* |procargs()| */
#define	CF_MULTI	1		/* run in parallel */
#define	CF_STACKCHECK	1		/* check stacks afterwards */
#define	CF_UCREADE	1		/* use |uc_reade(3uc)| */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a simple program (of some sort!).

	This program appends either specified files or the standarf
	input to a specified file.

	Synopsis:

	$ homepage [-V]

	where:

	-V		print program version to standard-error and then exit


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#if	defined(SFIO) || defined(KSHBUILTIN)
#undef	CF_SFIO
#define	CF_SFIO	1
#else
#ifndef	CF_SFIO
#define	CF_SFIO	0
#endif
#endif

#if	CF_SFIO
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<userinfo.h>
#include	<vecstr.h>
#include	<vecpstr.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<svcfile.h>
#include	<ascii.h>
#include	<nulstr.h>
#include	<vechand.h>
#include	<findbit.h>
#include	<pta.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<upt.h>
#include	<bwops.h>
#include	<filebuf.h>
#include	<termout.h>
#include	<ucmallreg.h>
#include	<intceil.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_homepage.h"
#include	"defs.h"
#include	"cgi.h"
#include	"htm.h"


/* local defines */

#define	STACKSIZE	(1*1024*1024)
#define	STACKGUARD	(2*1024)

#define	TBUFLEN		120

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	CONFIG		struct config
#define	CONFIG_MAGIC	0x23FFEEDD

#define	GATHER		struct gather

#define	FILER		struct filer
#define	FILER_DSIZE	(2*1024) ;

#define	PO_OPTION	"option"

#define	SVCLEN		(MAXNAMELEN)
#define	SVCENTLEN	(2*LINEBUFLEN)


/* typedefs */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif

typedef	int (*tworker)(void *) ;


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,const char *,
			const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	mkpath3w(char *,const char *,const char *,const char *,int) ;
extern int	mkpath4w(char *,const char *,const char *,const char *,
			const char *,int) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfdequote(const char *,int,const char **) ;
extern int	nextfieldterm(const char *,int,const char *,const char **) ;
extern int	mkplogid(char *,int,const char *,int) ;
extern int	mksublogid(char *,int,const char *,int) ;
extern int	nchr(const char *,int,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;
extern int	cfdecmful(const char *,int,ulong *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	progloger_begin(struct proginfo *,const char *,USERINFO *) ;
extern int	progloger_end(struct proginfo *) ;
extern int	proguserlist_begin(struct proginfo *,const char *,USERINFO *) ;
extern int	proguserlist_end(struct proginfo *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(const char *,int,void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */

extern char	**environ ;


/* local structures */

struct gather {
	vechand		ents ;
	PTM		m ;
	PTC		c ;
	const char	*termtype ;
	int		nout ;
	int		cols ;
} ;

struct filer {
	const char	*a ;
	const char	*fname ;
	const char	*svc ;
	const char	*termtype ;
	char		*dbuf ;
	caddr_t		saddr ;
	caddr_t		maddr ;
	size_t		ssize ;
	size_t		msize ;
	pthread_t	tid ;
	int		dsize ;
	int		dl ;
	int		to ;
	int		lines ;
	int		cols ;
	int		f_running ;
	int		f_stackcheck ;
} ;

struct locinfo_flags {
	uint		stores:1 ;
	uint		lbuf:1 ;
	uint		trunc:1 ;
	uint		start:1 ;
	uint		basedname:1 ;
	uint		dbfname:1 ;
	uint		hfname:1 ;
	uint		svcs:1 ;
	uint		intcache:1 ;
} ;

struct locinfo {
	VECSTR		stores ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	init, open ;
	SVCFILE		s ;
	GATHER		g ;
	struct proginfo	*pip ;
	void		*svcs ;
	const char	*basedname ;
	const char	*wfname ;		/* log-welcome file-name */
	const char	*dbfname ;
	const char	*hfname ;		/* HEAD file-name */
	const char	*sfname ;		/* SVC file-name */
	const char	*termtype ;
	int		start ;
	int		nproc ;
	int		cols ;
	int		intcache ;		/* cache time-out */
} ;

struct config {
	uint		magic ;
	struct proginfo	*pip ;
	PARAMOPT	*app ;
	EXPCOOK		cooks ;
	uint		f_p:1 ;
	uint		f_cooks:1 ;
} ;


/* forward references */

int		p_homepage(int,const char **,const char **,void *) ;

static int	usage(struct proginfo *) ;

static int	locinfo_start(struct locinfo *,struct proginfo *) ;
static int	locinfo_finish(struct locinfo *) ;
static int	locinfo_setentry(struct locinfo *,const char **,
			const char *,int) ;
static int	locinfo_basedir(LOCINFO *,const char *,int) ;
static int	locinfo_dbinfo(LOCINFO *,const char *,const char *) ;
static int	locinfo_wfname(LOCINFO *,const char *) ;
static int	locinfo_sethead(LOCINFO *,const char *,int) ;
static int	locinfo_svclistbegin(LOCINFO *) ;
static int	locinfo_svclistadd(LOCINFO *,const char *,int) ;
static int	locinfo_svclistend(LOCINFO *) ;
static int	locinfo_svclistget(LOCINFO *,int,const char **) ;
static int	locinfo_svclistdel(LOCINFO *,int) ;
static int	locinfo_defaults(LOCINFO *) ;
static int	locinfo_defsvc(LOCINFO *) ;
static int	locinfo_svcsbegin(LOCINFO *) ;
static int	locinfo_svcsend(LOCINFO *) ;
static int	locinfo_gatherbegin(LOCINFO *) ;
static int	locinfo_gatherbeginall(LOCINFO *) ;
static int	locinfo_gatherbeginsome(LOCINFO *) ;
static int	locinfo_gatherbeginer(LOCINFO *,SVCFILE_ENT *) ;
static int	locinfo_gatherend(LOCINFO *) ;

#ifdef	COMMENT
static int	locinfo_logfile(LOCINFO *,const char *,int) ;
#endif

static int	config_start(struct config *,struct proginfo *,PARAMOPT *,
			const char *) ;
static int	config_cookbegin(struct config *) ;
static int	config_cookend(struct config *) ;
static int	config_load(CONFIG *,const char *) ;
static int	config_havefile(CONFIG *,vecstr *,char *,const char *,
			const char *) ;
static int	config_read(struct config *,const char *) ;
static int	config_reader(struct config *,PARAMFILE *) ;
static int	config_finish(struct config *) ;

#ifdef	COMMENT
static int	config_check(struct config *) ;
#endif /* COMMENT */

static int	procopts(struct proginfo *,KEYOPT *) ;
static int	procuserinfo_begin(struct proginfo *,USERINFO *) ;
static int	procuserinfo_end(struct proginfo *) ;
static int	procuserinfo_logid(struct proginfo *) ;
static int	procourconf_begin(struct proginfo *,PARAMOPT *,const char *) ;
static int	procourconf_end(struct proginfo *) ;

#if	CF_PROCARGS
static int	procargs(struct proginfo *,struct arginfo *,BITS *,
			const char *,const char *,const char *) ;
#endif

static int	procresp(struct proginfo *,struct arginfo *,BITS *,
			const char *,const char *,const char *) ;
static int	procdoc(struct proginfo *,struct arginfo *,BITS *,
			const char *,const char *,const char *) ;
static int	procdocbody(struct proginfo *,HTM *) ;
static int	procdocbodyhdr(struct proginfo *,HTM *) ;
static int	procdocbodyhdrfile(PROGINFO *,char *,int,const char *) ;
static int	procdocbodymain(struct proginfo *,HTM *) ;
static int	procdocbodyfooter(struct proginfo *,HTM *) ;
static int	procdocbodyfooterleft(struct proginfo *,HTM *) ;
static int	procdocbodyfooterext(struct proginfo *,HTM *) ;
static int	procdocbodymain_svcs(PROGINFO *,HTM *) ;
static int	procdocbodymain_svcer(PROGINFO *,HTM *,GATHER *,SVCFILE_ENT *) ;
static int	procdocbodymain_svcerinc(PROGINFO *,HTM *,
			const char *,int) ;
static int	procdocbodymain_svcerfile(PROGINFO *,HTM *,GATHER *,
			SVCFILE_ENT *) ;

static int	procdocbodymain_svcprint(PROGINFO *,HTM *,GATHER *,SVCFILE *) ;
static int	prochdrs(struct proginfo *,CGI *,int) ;

static int	mkourname(char *,PROGINFO *,const char *,const char *,int) ;
static int	mkincfname(char *,PROGINFO *,const char *,int) ;
static int	setfname(struct proginfo *,char *,const char *,int,
			int,const char *,const char *,const char *) ;

static int	svckey(const char *(*)[2],int,const char *,const char **) ;
static int	svckey_dequote(const char *(*)[2],int,const char *,
			const char **) ;
static int	svckey_isfile(const char *(*)[2],int,const char **) ;
static int	svckey_isexec(const char *(*)[2],int,const char **) ;
static int	svckey_svcopts(const char *(*)[2],int) ;

static int gather_start(GATHER *,const char *,int) ;
static int gather_finish(GATHER *) ;
static int gather_file(GATHER *,const char *,int,const char *,int) ;
static int gather_entfins(GATHER *) ;
static int gather_getlines(GATHER *,const char *) ;
static int gather_getbuf(GATHER *,const char *,const char **) ;

static int filer_start(FILER *,const char *,int,const char *,const char *,int) ;
static int filer_worker(FILER *) ;
static int filer_workread(FILER *,int) ;
static int filer_workreadreg(FILER *,int) ;
static int filer_workreadterm(FILER *,int) ;
static int filer_workreadtermline(FILER *,TERMOUT *,char *,const char *,int) ;
static int filer_getlines(FILER *) ;
static int filer_getbuf(FILER *,const char **) ;
static int filer_havesvc(FILER *,const char *) ;
static int filer_finish(FILER *) ;
static int filer_stackbegin(FILER *) ;
static int filer_stackend(FILER *) ;
static int filer_stackfill(FILER *) ;
static int filer_stackused(FILER *) ;


/* local variables */

static const char	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"ROOT",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"cf",
	"lf",
	"wf",
	"db",
	"qs",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_root,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_cf,
	argopt_lf,
	argopt_wf,
	argopt_db,
	argopt_qs,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;

static const struct mapex	mapexs[] = {
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

static const char	*akonames[] = {
	"print",
	"add",
	"inc",
	"base",
	"log",
	"head",
	"svcs",
	"welcome",
	"to",
	NULL
} ;

enum akonames {
	akoname_print,
	akoname_add,
	akoname_inc,
	akoname_base,
	akoname_log,
	akoname_head,
	akoname_svcs,
	akoname_welcome,
	akoname_to,
	akoname_overlast
} ;

static const char	*csched[] = {
	"%p/etc/%n/%n.%f",
	"%p/etc/%n/%f",
	"%p/etc/%n.%f",
	NULL
} ;

static const char	*cparams[] = {
	"basedir",
	"basedb",
	"logfile",
	"logsize",
	"head",
	"svcs",
	"intcache",
	NULL
} ;

enum cparams {
	cparam_basedir,
	cparam_basedb,
	cparam_logfile,
	cparam_logsize,
	cparam_head,
	cparam_svcs,
	cparam_intcache,
	cparam_overlast
} ;

static char	*isexecs[] = {
	"so",
	"p",
	"a",
	NULL
} ;

static const char	*svcopts[] = {
	"termout",
	NULL
} ;

enum svcopts {
	svcopt_termout,
	svcopt_overlast
} ;


/* exported subroutines */


int b_homepage(argc,argv,contextp)
int		argc ;
const char	*argv[] ;
void		*contextp ;
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp)) >= 0) {
	    const char	**envv = (const char **) environ ;
	    ex = p_homepage(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_homepage) */


int p_homepage(argc,argv,envv,contextp)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
void		*contextp ;
{
	struct proginfo	pi, *pip = &pi ;
	struct locinfo	li, *lip = &li ;
	struct arginfo	ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	SHIO		errfile ;
	USERINFO	u ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos, ai_continue ;
	int	rs, rs1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*cfname = NULL ;
	const char	*lfname = NULL ;
	const char	*wfname = NULL ;
	const char	*qs = NULL ;
	const char	*dbfname = NULL ;
	const char	*un = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_homepage: starting DFD=%d\n",rs) ;
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
	proginfo_setbanner(pip,BANNER) ;

/* early things to initialize */

	pip->debuglevel = CF_DEBUGLEVEL ;
	pip->verboselevel = 1 ;
	pip->to_open = -1 ;
	pip->f.logprog = OPT_LOGPROG ;

	pip->lip = lip ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	if (rs >= 0) {
	    rs = keyopt_start(&akopts) ;
	    pip->open.akopts = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = paramopt_start(&aparams) ;
	    pip->open.aparams = (rs >= 0) ;
	}

	ai_max = 0 ;
	ai_pos = 0 ;
	ai_continue = 1 ;
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
	        const int ch = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ch)) {

	            argval = (argp + 1) ;

	        } else if (argp[1] == '-') {

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

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

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

	                case argopt_help:
	                    f_help = TRUE ;
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

/* program searcn-name */
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

/* configuration file-name */
	                case argopt_cf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            cfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* log file-name */
	                case argopt_lf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* log welcome-name */
	                case argopt_wf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            wfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                wfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* database file */
	                case argopt_db:
	                    cp = NULL ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                        }
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
	                        lip->final.dbfname = TRUE ;
	                        dbfname = cp ;
	                    }
	                    break ;

/* quert string */
	                case argopt_qs:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            qs = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                qs = argp ;
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
	                    int	kc = (*akp & 0xff) ;

	                    switch (kc) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'C':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;


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

/* quiet */
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

/* terminal-type */
	                    case 'T':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->termtype = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* take input file arguments from STDIN */
	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            afname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                rs = keyopt_loads(&akopts,argp,argl) ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* quiet */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* starting offset */
	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                ulong	ulw ;
	                                rs = cfdecmful(argp,argl,&ulw) ;
	                                lip->start = ulw ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* truncate */
	                    case 't':
	                        lip->f.trunc = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.trunc = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'u':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                un = argp ;
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

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        f_usage = TRUE ;
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if */

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
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_homepage: 0 rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_homepage: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_homepage: pr=%s\n",pip->pr) ;
	    debugprintf("b_homepage: sn=%s\n",pip->searchname) ;
	}
#endif

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

/* check a few more things */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (qs == NULL) qs = getourenv(envv,VARQS) ;
	if (qs == NULL) qs = getourenv(envv,VARQUERYSTRING) ;

	if (cfname == NULL) cfname = getourenv(envv,VARCFNAME) ;

	if (wfname == NULL) wfname = getourenv(envv,VARWFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (rs >= 0)
	    rs = locinfo_dbinfo(lip,NULL,dbfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_homepage: 1 rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    rs = procopts(pip,&akopts) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_homepage: 2 rs=%d\n",rs) ;
#endif

	if (wfname == NULL) wfname = LWFNAME ;

	if (rs >= 0)
	    rs = locinfo_wfname(lip,wfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_homepage: 3 rs=%d\n",rs) ;
#endif

	if (rs < 0) goto badarg ;

	if (pip->debuglevel > 0) {
	    const char	*pn = pip->progname ;
	    const char	*fmt ;
	    fmt = "%s: cf=%s\n" ;
	    shio_printf(pip->efp,fmt,pn,cfname) ;
	    fmt = "%s: wf=%s\n" ;
	    shio_printf(pip->efp,fmt,pn,wfname) ;
	}

/* go */

	memset(&ainfo,0,sizeof(struct arginfo)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;
	ainfo.ai_continue = ai_continue ;

	if ((rs = userinfo_start(&u,un)) >= 0) {
	    if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	        if (cfname != NULL) {
	            if (pip->euid != pip->uid) u_seteuid(pip->uid) ;
	            if (pip->egid != pip->gid) u_setegid(pip->gid) ;
	        }
	        if ((rs = procourconf_begin(pip,&aparams,cfname)) >= 0) {
	            if ((rs = progloger_begin(pip,lfname,&u)) >= 0) {
	                if ((rs = proguserlist_begin(pip,sn,&u)) >= 0) {
	                    struct arginfo	*aip = &ainfo ;
	                    BITS		*bop = &pargs ;
	                    const char		*afn = afname ;
	                    const char		*ofn = ofname ;
	                    const char		*ifn = ifname ;

	                    if ((rs = procresp(pip,aip,bop,ofn,ifn,afn)) >= 0) {
	                        if (pip->debuglevel > 0) {
	                            shio_printf(pip->efp,"%s: written=%u\n",
	                                pip->progname,rs) ;
	                        }
	                    }

	                    rs1 = proguserlist_end(pip) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (proguserlist) */
	                rs1 = progloger_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (proglogfile) */
	            rs1 = procourconf_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procourconf) */
	        rs1 = procuserinfo_end(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (procuserinfo) */
	    rs1 = userinfo_finish(&u) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    ex = EX_NOUSER ;
	    shio_printf(pip->efp,
	        "%s: userinfo failure (%d)\n",
	        pip->progname,rs) ;
	}

	if (rs < 0) {
	    shio_printf(pip->efp,"%s: could not process (%d)\n",
	        pip->progname,rs) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
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
	} else if (rs >= 0) {
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
	    debugprintf("b_homepage: exiting ex=%u (%d)\n",ex,rs) ;
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
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    int		f_out = CF_DEBUGMOUT ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("b_homepage: final mallout=%u\n",mdiff) ;
	    if ((mdiff > 0) || f_out) {
	        UCMALLREG_CUR	cur ;
	        UCMALLREG_REG	reg ;
	        const int	size = (10*sizeof(uint)) ;
	        const char	*ids = "main" ;
	        uc_mallinfo(mi,size) ;
	        debugprintf("b_homepage: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("b_homepage: MIoutnummax=%u\n",
	            mi[ucmallreg_outnummax]) ;
	        debugprintf("b_homepage: MIoutsize=%u\n",
	            mi[ucmallreg_outsize]) ;
	        debugprintf("b_homepage: MIoutsizemax=%u\n",
	            mi[ucmallreg_outsizemax]) ;
	        debugprintf("b_homepage: MIused=%u\n",mi[ucmallreg_used]) ;
	        debugprintf("b_homepage: MIusedmax=%u\n",
	            mi[ucmallreg_usedmax]) ;
	        debugprintf("b_homepage: MIunder=%u\n",mi[ucmallreg_under]) ;
	        debugprintf("b_homepage: MIover=%u\n",mi[ucmallreg_over]) ;
	        debugprintf("b_homepage: MInotalloc=%u\n",
	            mi[ucmallreg_notalloc]) ;
	        debugprintf("b_homepage: MInotfree=%u\n",
	            mi[ucmallreg_notfree]) ;
	        ucmallreg_curbegin(&cur) ;
	        while (ucmallreg_enum(&cur,&reg) >= 0) {
	            debugprintf("b_homepage: MIreg.addr=%p\n",reg.addr) ;
	            debugprintf("b_homepage: MIreg.size=%u\n",reg.size) ;
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

/* bad stuff comes here */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int		rs ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-of <outfile>]\n" ;
	rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-af <afile>] [-t]\n" ;
	rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int locinfo_start(lip,pip)
struct locinfo	*lip ;
struct proginfo	*pip ;
{
	int		rs = SR_OK ;

	if (lip == NULL)
	    return SR_FAULT ;

	memset(lip,0,sizeof(struct locinfo)) ;
	lip->pip = pip ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(lip)
struct locinfo	*lip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL)
	    return SR_FAULT ;

	rs1 = locinfo_svclistend(lip) ;
	if (rs >= 0) rs = rs1 ;

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


int locinfo_setentry(lip,epp,vp,vl)
struct locinfo	*lip ;
const char	**epp ;
const char	vp[] ;
int		vl ;
{
	int		rs = SR_OK ;
	int		vnlen = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! lip->open.stores) {
	    rs = vecstr_start(&lip->stores,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;

	    if (*epp != NULL) oi = vecstr_findaddr(&lip->stores,*epp) ;

	    if (vp != NULL) {
	        vnlen = strnlen(vp,vl) ;
	        rs = vecstr_store(&lip->stores,vp,vnlen,epp) ;
	    } else
	        *epp = NULL ;

	    if ((rs >= 0) && (oi >= 0))
	        vecstr_del(&lip->stores,oi) ;

	} /* end if */

	return (rs >= 0) ? vnlen : rs ;
}
/* end subroutine (locinfo_setentry) */


static int locinfo_dbinfo(lip,basedname,dbfname)
LOCINFO		 *lip ;
const char	*basedname ;
const char	*dbfname ;
{
	struct proginfo	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/locinfo_dbinfo: basedb=%s\n",basedname) ;
	    debugprintf("main/locinfo_dbinfo: dbfname=%s\n",dbfname) ;
	}
#endif

	if ((rs >= 0) && (lip->basedname == NULL)) {
	    if (basedname != NULL) {
	        rs = locinfo_basedir(lip,basedname,-1) ;
	    }
	}

	if ((rs >= 0) && (lip->dbfname == NULL)) {
	    if (dbfname != NULL) {
	        const char	**vpp = &lip->dbfname ;
	        lip->final.dbfname = TRUE ;
	        rs = locinfo_setentry(lip,vpp,dbfname,-1) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/locinfo_dbinfo: ret basedb=%s\n",lip->basedname) ;
	    debugprintf("main/locinfo_dbinfo: ret dbfname=%s\n",lip->dbfname) ;
	    debugprintf("main/locinfo_dbinfo: ret rs=%d\n",rs) ;
	}
#endif

	return rs ;
}
/* end subroutine (locinfo_dbinfo) */


static int locinfo_basedir(LOCINFO *lip,const char *vp,int vl)
{
	struct proginfo	*pip = lip->pip ;
	int	rs = SR_OK ;

	if (lip->basedname == NULL) {
	    const char	*inter = ETCCNAME ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkourname(tbuf,pip,inter,vp,vl)) >= 0) {
	        const char	**vpp = &lip->basedname ;
	        rs = locinfo_setentry(lip,vpp,tbuf,rs) ;
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_basedir) */


static int locinfo_wfname(LOCINFO *lip,const char *wfname)
{
	int		rs = SR_OK ;
	if ((lip->wfname == NULL) && (wfname != NULL)) {
	    const char	**vpp = &lip->wfname ;
	    rs = locinfo_setentry(lip,vpp,wfname,-1) ;
	}
	return rs ;
}
/* end subroutine (locinfo_wfname) */


static int locinfo_sethead(LOCINFO *lip,const char *vp,int vl)
{
	struct proginfo	*pip = lip->pip ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/locinfo_sethead: v=%t\n",vp,vl) ;
#endif

	if (lip->hfname == NULL) {
	    const char	*inter = ETCCNAME ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkourname(tbuf,pip,inter,vp,vl)) >= 0) {
	        const char	**vpp = &lip->hfname ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/locinfo_sethead: tbuf=%t\n",tbuf,rs) ;
#endif
	        rs = locinfo_setentry(lip,vpp,tbuf,rs) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/locinfo_sethead: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_sethead) */


static int locinfo_svclistbegin(LOCINFO *lip)
{
	struct proginfo	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (lip->svcs == NULL) {
	    const int	size = sizeof(VECPSTR) ;
	    void	*p ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        VECPSTR		*slp = p ;
	        lip->svcs = p ;
	        rs = vecpstr_start(slp,5,0,0) ;
	        if (rs < 0) {
	            uc_free(lip->svcs) ;
	            lip->svcs = NULL ;
	        }
	    } /* end if (m-a) */
	} /* end if (svcs) */
	return rs ;
}
/* end subroutine (locinfo_svclistbegin) */


static int locinfo_svclistend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->svcs != NULL) {
	    VECPSTR	*slp = lip->svcs ;
	    rs1 = vecpstr_finish(slp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(lip->svcs) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->svcs = NULL ;
	}
	return rs ;
}
/* end subroutine (locinfo_svclistend) */


static int locinfo_svclistadd(LOCINFO *lip,const char *vp,int vl)
{
	struct proginfo	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/locinfo_svclistadd: v=%t\n",vp,vl) ;
#endif

	if (lip->svcs == NULL) {
	    rs = locinfo_svclistbegin(lip) ;
	} /* end if (svcs) */

	if (rs >= 0) {
	    VECPSTR	*slp = lip->svcs ;
	    int		sl ;
	    const char	st[] = {
	                    CH_FS, 0 				} ;
	    const char	*sp ;
	    if (vl < 0) vl = strlen(vp) ;
	    while (vl > 0) {
	        if ((sl = nextfieldterm(vp,vl,st,&sp)) < 0) break ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/locinfo_svclistadd: s=>%t<\n",sp,sl) ;
#endif
	        if (sl > 0) {
	            rs = vecpstr_adduniq(slp,sp,sl) ;
	            if (rs < INT_MAX) c += 1 ;
	        }
	        vl -= ((sp+sl+1)-vp) ;
	        vp = (sp+sl+1) ;
	        if (rs < 0) break ;
	    } /* end while */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/locinfo_svclistadd: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_svclistadd) */


static int locinfo_svclistget(LOCINFO *lip,int i,const char **rpp)
{
	int		rs = SR_OK ;
	if (lip->svcs != NULL) {
	    VECPSTR	*slp = lip->svcs ;
	    rs = vecpstr_get(slp,i,rpp) ;
	} else
	    rs = SR_NOTFOUND ;
	return rs ;
}
/* end subroutine (locinfo_svclistget) */


static int locinfo_svclistdel(LOCINFO *lip,int i)
{
	int		rs = SR_OK ;
	if (lip->svcs != NULL) {
	    VECPSTR	*slp = lip->svcs ;
	    rs = vecpstr_del(slp,i) ;
	}
	return rs ;
}
/* end subroutine (locinfo_svclistdel) */


#ifdef	COMMENT
static int locinfo_logfile(LOCINFO *lip,const char *vp,int vl)
{
	struct proginfo	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (! pip->final.lfname) {
	    const char	*inter = LOCCNAME ;
	    char	tbuf[MAXPATHLEN+1] ;
	    pip->final.lfname = TRUE ;
	    if ((rs = mkourname(tbuf,pip,inter,vp,vl)) >= 0) {
	        const char	**vpp = &pip->lfname ;
	        rs = proginfo_setentry(pip,vpp,tbuf,rs) ;
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_logfile) */
#endif /* COMMENT */


static int locinfo_defaults(LOCINFO *lip)
{
	struct proginfo	*pip = lip->pip ;
	int		rs = SR_OK ;

	if ((rs >= 0) && (lip->hfname == NULL)) {
	    const char	*inter = ETCCNAME ;
	    const char	*fn = HEADFNAME ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkourname(tbuf,pip,inter,fn,-1)) >= 0) {
	        int	tl = rs ;
	        if ((rs = perm(tbuf,-1,-1,NULL,R_OK)) >= 0) {
	            const char	**vpp = &lip->hfname ;
	            rs = locinfo_setentry(lip,vpp,tbuf,tl) ;
	        } else if (isNotPresent(rs))
	            rs = SR_OK ;
	    } /* end if (mkourname) */
	}

	if (rs >= 0) {
	    rs = locinfo_defsvc(lip) ;
	}

	if (lip->termtype == NULL) lip->termtype = DEFTERMTYPE ;

	if ((rs >= 0) && (lip->cols == 0)) {
	    const char	*vp = getourenv(pip->envv,VARCOLUMNS) ;
	    if ((vp != NULL) && (vp[0] != '\0')) {
	        rs = optvalue(vp,-1) ;
	        lip->cols = rs ;
	    }
	    if (lip->cols == 0) lip->cols = COLUMNS ;
	} /* end if (columns) */

	return rs ;
}
/* end subroutine (locinfo_defaults) */


static int locinfo_defsvc(LOCINFO *lip)
{
	struct proginfo	*pip = lip->pip ;
	int		rs = SR_OK ;

	if ((rs >= 0) && (lip->sfname == NULL)) {
	    char	tbuf[MAXPATHLEN+1] ;
	    const char	*inter = ETCCNAME ;
	    const char	*fn = SVCFNAME ;
	    if ((rs = mkourname(tbuf,pip,inter,fn,-1)) >= 0) {
	        int	tl = rs ;
	        if (perm(tbuf,-1,-1,NULL,R_OK) >= 0) {
	            const char	**vpp = &lip->sfname ;
	            rs = locinfo_setentry(lip,vpp,tbuf,tl) ;
	        }
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_defsvc) */


static int locinfo_svcsbegin(LOCINFO *lip)
{
	SVCFILE		*sfp = &lip->s ;
	int		rs ;
	if ((rs = svcfile_open(sfp,lip->sfname)) >= 0) {
	    GATHER	*glp = &lip->g ;
	    rs = gather_start(glp,lip->termtype,lip->cols) ;
	    if (rs < 0)
	        svcfile_close(sfp) ;
	} /* end if */
	return rs ;
}
/* end subroutine (locinfo_svcsbegin) */


static int locinfo_svcsend(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		used ;

	rs1 = gather_finish(&lip->g) ;
	if (rs >= 0) rs = rs1 ;
	used = rs1 ;

	rs1 = svcfile_close(&lip->s) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_STACKCHECK
	if ((rs >= 0) && (used >= (STACKSIZE-STACKGUARD))) {
	    PROGINFO	*pip = lip->pip ;
	    const char	*fmt = "%s: stack-guard violated used=%d\n" ;
	    shio_printf(pip->efp,fmt,pip->progname,used) ;
	}
#endif

	return (rs >= 0) ? used : rs ;
}
/* end subroutine (locinfo_svcsend) */


static int locinfo_gatherbegin(LOCINFO *lip)
{
	int		rs ;
	if (lip->svcs != NULL) {
	    rs = locinfo_gatherbeginsome(lip) ;
	} else {
	    rs = locinfo_gatherbeginall(lip) ;
	}
	return rs ;
}
/* end subroutine (locinfo_gatherbegin) */


static int locinfo_gatherbeginall(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	SVCFILE		*sfp = &lip->s ;
	GATHER		*glp = &lip->g ;
	SVCFILE_CUR	c ;
	SVCFILE_ENT	se ;
	int		rs ;
	int		rs1 ;

	if ((rs = svcfile_curbegin(sfp,&c)) >= 0) {
	    const int	slen = SVCLEN ;
	    const int	el = SVCENTLEN ;
	    char	sbuf[SVCLEN+1] ;
	    char	eb[SVCENTLEN+1] ;
	    void	*n = NULL ;
	    while ((rs1 = svcfile_enumsvc(sfp,&c,sbuf,slen)) > 0) {
	        if ((rs = locinfo_svclistadd(lip,sbuf,rs1)) >= 0) {
	            if ((rs = svcfile_fetch(sfp,sbuf,n,&se,eb,el)) > 0) {
	                rs = locinfo_gatherbeginer(lip,&se) ;
	            }
	        } /* end if (locinfo_svclistadd) */
	        if (rs < 0) break ;
	    } /* end while */
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("locinfo_gatherbegin: while-out "
	            "rs=%d rs1=%d\n",rs,rs1) ;
#endif
	    if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	    rs1 = svcfile_curend(sfp,&c) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (svcfile-cur) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_gatherbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_gatherbeginall) */


static int locinfo_gatherbeginsome(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	SVCFILE		*sfp = &lip->s ;
	GATHER		*glp = &lip->g ;
	const int	nrs = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		i ;
	const char	*snp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_gatherbegin: ent\n") ;
#endif

	for (i = 0 ; locinfo_svclistget(lip,i,&snp) >= 0 ; i += 1) {
	    if (snp != NULL) {
	        SVCFILE_ENT	se ;
	        const int	el = SVCENTLEN ;
	        void		*n = NULL ;
	        char		eb[SVCENTLEN+1] ;
	        if ((rs = svcfile_fetch(sfp,snp,n,&se,eb,el)) > 0) {
	            rs = locinfo_gatherbeginer(lip,&se) ;
	        }
	        if (rs == nrs) {
	            rs = locinfo_svclistdel(lip,i) ;
	        }
	    } /* end if (non-null) */
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_gatherbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_gatherbeginsome) */


static int locinfo_gatherend(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_homepage/locinfo_gatherend: ret rs=%d\n",
	        rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_gatherend) */


static int locinfo_gatherbeginer(LOCINFO *lip,SVCFILE_ENT *sep)
{
	PROGINFO	*pip = lip->pip ;
	GATHER		*glp = &lip->g ;
	const int	n = sep->nkeys ;
	int		rs = SR_OK ;
	int		vl ;
	const char	*(*kv)[2] = sep->keyvals ;
	const char	*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_gatherbeginer: ent\n") ;
#endif

	if ((vl = svckey_dequote(kv,n,"h",&vp)) > 0) {
	    const char	*fp ;
	    if ((rs = svckey_isfile(kv,n,&fp)) > 0) {
	        const char	*svc = sep->svc ;
	        int		fl = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("locinfo_gatherbeginer: "
	                "svckey_svcopts()\n") ;
#endif

	        if ((rs = svckey_svcopts(kv,n)) >= 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("locinfo_gatherbeginer: rs=\\b%04ß\n",rs) ;
#endif
	            const int	f_to = bwtst(rs,svcopt_termout) ;
	            rs = gather_file(glp,svc,f_to,fp,fl) ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("locinfo_gatherbeginer: "
	                "gather_file() rs=%d\n",rs) ;
#endif

	    } /* end if (svckey_isfile) */
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("locinfo_gatherbeginer: "
	            "svckey-isfile-out rs=%d\n",rs) ;
#endif
	} /* end if (svckey) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_gatherbeginer: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_gatherbeginer) */


/* configuration maintenance */
static int config_start(csp,pip,app,cfname)
struct config	*csp ;
struct proginfo	*pip ;
PARAMOPT	*app ;
const char	*cfname ;
{
	int		rs ;

	if (csp == NULL) return SR_FAULT ;
	if (app == NULL) return SR_FAULT ;

	memset(csp,0,sizeof(struct config)) ;
	csp->pip = pip ;
	csp->app = app ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: cfname=%s\n",cfname) ;
#endif

#ifdef	COMMENT
	if ((pip->debuglevel > 0) && (cfname != NULL)) {
	    shio_printf(pip->efp,"%s: cf=%s\n",
	        pip->progname,cfname) ;
	}
#endif /* COMMENT */

	if ((rs = config_cookbegin(csp)) >= 0) {
	    if ((cfname != NULL) && (cfname[0] != '\0')) {
	        rs = config_read(csp,cfname) ;
	    } else {
	        cfname = CONFIGFNAME ;
	        rs = config_load(csp,cfname) ;
	    }
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("config_start: mid rs=%d\n",rs) ;
#endif
	    if (rs >= 0) {
	        csp->f_p = TRUE ;
	        csp->magic = CONFIG_MAGIC ;
	    } else {
	        config_cookend(csp) ;
	    }
	} /* end if (config-cookbegin) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: ret rs=%d f=%u\n",rs,csp->f_p) ;
#endif

	return rs ;
}
/* end subroutine (config_start) */


static int config_finish(CONFIG *csp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (csp == NULL) return SR_FAULT ;
	if (csp->magic != CONFIG_MAGIC) return SR_NOTOPEN ;

	if (csp->f_p) {
	    if (csp->f_cooks) {
	        rs1 = config_cookend(csp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    csp->f_p = FALSE ;
	} /* end if */

	return rs ;
}
/* end subroutine (config_finish) */


static int config_cookbegin(CONFIG *csp)
{
	struct proginfo	*pip = csp->pip ;
	int		rs ;

	if ((rs = expcook_start(&csp->cooks)) >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    int		i ;
	    int		kch ;
	    int		vl ;
	    const char	*ks = "PSNDHRU" ;
	    const char	*vp ;
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
	                const char	*nn = pip->nodename ;
	                const char	*dn = pip->domainname ;
	                rs = snsds(hbuf,hlen,nn,dn) ;
	                vl = rs ;
	                vp = hbuf ;
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
	            rs = expcook_add(&csp->cooks,kbuf,vp,vl) ;
	        }
	    } /* end for */

	    if (rs >= 0) {
	        csp->f_cooks = TRUE ;
	    } else
	        expcook_finish(&csp->cooks) ;
	} /* end if (expcook_start) */

	return rs ;
}
/* end subroutine (config_cookbegin) */


static int config_cookend(CONFIG *csp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (csp->f_cooks) {
	    csp->f_cooks = FALSE ;
	    rs1 = expcook_finish(&csp->cooks) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (config_cookend) */


#ifdef	COMMENT
static int config_check(CONFIG *csp)
{
	struct proginfo	*pip = csp->pip ;
	int		rs = SR_OK ;

	if (csp == NULL) return SR_FAULT ;
	if (csp->magic != CONFIG_MAGIC) return SR_NOTOPEN ;

	if (csp->f_p) {
	    rs = paramfile_load(&csp->p) ;
	}

	return rs ;
}
/* end subroutine (config_check) */
#endif /* COMMENT */


static int config_load(CONFIG *csp,const char *cfn)
{
	struct proginfo	*pip = csp->pip ;
	vecstr		sv ;
	int		rs ;

	if ((rs = vecstr_start(&sv,3,0)) >= 0) {
	    int		i ;
	    const char	*pn = pip->progname ;
	    const char	*sn = pip->searchname ;
	    const char	*pr ;
	    char	cbuf[MAXPATHLEN+1] ;
	    if ((rs = vecstr_envset(&sv,"n",sn,-1)) >= 0) {
	        for (i = 0 ; i < 2 ; i += 1) {
	            switch (i) {
	            case 0:
	                pr = pip->username ;
	                break ;
	            case 1:
	                pr = pip->pr ;
	                break ;
	            } /* end switch */
	            if ((rs = config_havefile(csp,&sv,cbuf,pr,cfn)) > 0) {
#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("config_load: cfn=%s\n",cbuf) ;
#endif
	                if (pip->debuglevel > 0) {
	                    shio_printf(pip->efp,"%s: cf=%s\n",pn,cbuf) ;
	                }

	                rs = config_read(csp,cbuf) ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("config_load: _read() rs=%d\n",rs) ;
#endif
	            }
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (vecstr_envset) */
	    vecstr_finish(&sv) ;
	} /* end if (finding file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_load: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_load) */


static int config_havefile(csp,svp,cbuf,pr,cfname)
CONFIG		*csp ;
vecstr		*svp ;
char		cbuf[] ;
const char	*pr ;
const char	*cfname ;
{
	const int	clen = MAXPATHLEN ;
	int		rs ;
	int		pl = 0 ;

	if (csp == NULL) return SR_FAULT ;

	if ((rs = vecstr_envset(svp,"p",pr,-1)) >= 0) {
	    if ((rs = permsched(csched,svp,cbuf,clen,cfname,R_OK)) >= 0) {
	        pl = rs ;
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	        cbuf[0] = '\0' ;
	    }
	} /* end if (str-add) */

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (config_havefile) */


static int config_read(CONFIG *csp,const char *cfname)
{
	struct proginfo	*pip = csp->pip ;
	PARAMFILE	p ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: ent cfn=%s\n",cfname) ;
#endif

#ifdef	COMMENT
	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: cf=%s\n",pip->progname,cfname) ;
#endif

	if ((rs = paramfile_open(&p,pip->envv,cfname)) >= 0) {

	    rs = config_reader(csp,&p) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("config_read: _reader() rs=%d\n",rs) ;
#endif

	    rs1 = paramfile_close(&p) ;
	    if (rs >= 0) rs = rs1 ;
	} else if (isNotPresent(rs))
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (config_read) */


static int config_reader(CONFIG *csp,PARAMFILE *pfp)
{
	struct proginfo	*pip = csp->pip ;
	struct locinfo	*lip ;
	PARAMFILE_CUR	cur ;
	const int	vlen = VBUFLEN ;
	const int	elen = EBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		ml, vl, el ;
	int		v ;
	char		vbuf[VBUFLEN + 1] ;
	char		ebuf[EBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_reader: f_p=%u\n",csp->f_p) ;
#endif

	lip = pip->lip ;
	if (lip == NULL) return SR_FAULT ;

	for (i = 0 ; cparams[i] != NULL ; i += 1) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("config_reader: cp=%s\n",cparams[i]) ;
#endif

	    if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {

	        while (rs >= 0) {
	            vl = paramfile_fetch(pfp,cparams[i],&cur,vbuf,vlen) ;
	            if (vl == SR_NOTFOUND) break ;
	            rs = vl ;
	            if (rs < 0) break ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("config_reader: vbuf=>%t<\n",vbuf,vl) ;
#endif

	            ebuf[0] = '\0' ;
	            el = 0 ;
	            if (vl > 0) {
	                el = expcook_exp(&csp->cooks,0,ebuf,elen,vbuf,vl) ;
	                if (el >= 0) ebuf[el] = '\0' ;
	            }

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("config_reader: ebuf=>%t<\n",ebuf,el) ;
#endif

	            if (el > 0) {
	                const char	*sn = pip->searchname ;
	                char		tbuf[MAXPATHLEN + 1] ;

	                switch (i) {

	                case cparam_logsize:
			    if (! pip->final.logsize) {
	                        if ((rs = cfdecmfi(ebuf,el,&v)) >= 0) {
	                            if (v >= 0) {
	                                switch (i) {
	                                case cparam_logsize:
	                                    pip->logsize = v ;
	                                    break ;
	                                } /* end switch */
	                            }
	                        } /* end if (valid number) */
			    }
	                    break ;

	                case cparam_basedir:
	                case cparam_basedb:
	                    if (! lip->final.basedname) {
	                        if (el > 0) {
	                            lip->final.basedname = TRUE ;
	                            rs = locinfo_basedir(lip,ebuf,el) ;
	                        }
	                    }
	                    break ;

	                case cparam_logfile:
	                    if (! pip->final.lfname) {
	                        const char *lfn = pip->lfname ;
	                        const char	*tfn = tbuf ;
	                        pip->final.lfname = TRUE ;
	                        pip->have.lfname = TRUE ;
	                        ml = setfname(pip,tbuf,ebuf,el,TRUE,
	                            LOGCNAME,sn,"") ;
	                        if ((lfn == NULL) || 
	                            (strcmp(lfn,tfn) != 0)) {
	                            const char	**vpp = &pip->lfname ;
	                            pip->changed.lfname = TRUE ;
	                            rs = proginfo_setentry(pip,vpp,tbuf,ml) ;
	                        }
	                    }
	                    break ;

	                case cparam_head:
	                    if (! lip->final.hfname) {
	                        if (el > 0) {
	                            lip->final.hfname = TRUE ;
	                            rs = locinfo_sethead(lip,ebuf,el) ;
	                        }
	                    }
	                    break ;

	                case cparam_svcs:
	                    if (! lip->final.svcs) {
	                        if (el > 0) {
	                            lip->have.svcs = TRUE ;
	                            rs = locinfo_svclistadd(lip,ebuf,el) ;
	                        }
	                    }
	                    break ;

	                case cparam_intcache:
			    if (! lip->final.intcache) {
	                        if ((rs = cfdecti(ebuf,el,&v)) >= 0) {
	                            if (v >= 0) {
	                                switch (i) {
	                                case cparam_intcache :
	                                    lip->intcache = v ;
	                                    break ;
	                                } /* end switch */
	                            }
	                        } /* end if (valid number) */
			    }
	                    break ;

	                } /* end switch */

	            } /* end if (got one) */

	        } /* end while (fetching) */

	        rs1 = paramfile_curend(pfp,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (paramfile-cur) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("config_reader: bot rs=%d\n",rs) ;
#endif

	    if (rs < 0) break ;
	} /* end for (parameters) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_reader: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_reader) */


/* process the program ako-options */
static int procopts(pip,kop)
struct proginfo	*pip ;
KEYOPT		*kop ;
{
	struct locinfo	*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int		oi ;
	        int		kl, vl ;
	        const char	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                switch (oi) {

	                case akoname_base:
	                    if (! lip->final.basedname) {
	                        if (vl > 0) {
	                            lip->final.basedname = TRUE ;
	                            rs = locinfo_basedir(lip,vp,vl) ;
	                        }
	                    }
	                    break ;

	                case akoname_log:
	                    if (! pip->final.logprog) {
	                        pip->f.logprog = TRUE ;
	                        if (vl > 0) {
	                            pip->final.logprog = TRUE ;
	                            rs = optbool(vp,vl) ;
	                            pip->f.logprog = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_head:
	                    if (! lip->final.hfname) {
	                        if (vl > 0) {
	                            lip->final.hfname = TRUE ;
	                            rs = locinfo_sethead(lip,vp,vl) ;
	                        }
	                    }
	                    break ;

	                case akoname_svcs:
	                    if (! lip->final.svcs) {
	                        if (vl > 0) {
	                            lip->have.svcs = TRUE ;
	                            rs = locinfo_svclistadd(lip,vp,vl) ;
	                        }
	                    }
	                    break ;

	                } /* end switch */

	                c += 1 ;
	            } /* end if (valid option) */

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        rs1 = keyopt_curend(kop,&kcur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procuserinfo_begin(pip,uip)
struct proginfo	*pip ;
USERINFO	*uip ;
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

	if (rs >= 0) {
	    rs = procuserinfo_logid(pip) ;
	} /* end if (ok) */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: username=%s\n",
	        pip->progname,pip->username) ;
	}

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(pip)
struct proginfo	*pip ;
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
	            const char	*nn = pip->nodename ;
	            char		pbuf[LOGIDLEN+1] ;
	            if ((rs = mkplogid(pbuf,plen,nn,pv)) >= 0) {
	                const int	slen = LOGIDLEN ;
	                char	sbuf[LOGIDLEN+1] ;
	                if ((rs = mksublogid(sbuf,slen,pbuf,s)) >= 0) {
	                    const char	**vpp = &pip->logid ;
	                    rs = proginfo_setentry(pip,vpp,sbuf,rs) ;
	                }
	            }
	        } /* end if (lib_serial) */
	    } /* end if (runmode-KSH) */
	} /* end if (lib_runmode) */
	return rs ;
}
/* end subroutine (procuserinfo_logid) */


static int procourconf_begin(pip,app,cfname)
struct proginfo	*pip ;
PARAMOPT	*app ;
const char	cfname[] ;
{
	const int	csize = sizeof(struct config) ;
	int		rs ;
	void		*p ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_homepage/procourconf_begin: ent\n") ;
#endif

	if ((rs = uc_malloc(csize,&p)) >= 0) {
	    CONFIG	*csp = p ;
	    pip->config = csp ;
	    if ((rs = config_start(csp,pip,app,cfname)) >= 0) {
	        LOCINFO	*lip = pip->lip ;
	        rs = locinfo_defaults(lip) ;
	    } /* end if (config) */
	    if (rs < 0) {
	        uc_free(p) ;
	        pip->config = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_homepage/procourconf_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procourconf_begin) */


static int procourconf_end(struct proginfo *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->config != NULL) {
	    CONFIG	*csp = pip->config ;
	    rs1 = config_finish(csp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(pip->config) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->config = NULL ;
	}

	return rs ;
}
/* end subroutine (procourconf_end) */


#if	CF_PROCARGS
static int procargs(pip,aip,bop,ofname,ifname,afname)
struct proginfo	*pip ;
struct arginfo	*aip ;
BITS		*bop ;
const char	*ofname ;
const char	*ifname ;
const char	*afname ;
{
	SHIO		ofile, *ofp = &ofile ;
	const int	to_open = pip->to_open ;
	int		rs ;
	int		wlen = 0 ;

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = STDOUTFNAME ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procargs: ofname=%s\n",ofname) ;
#endif

	if ((rs = shio_opene(ofp,ofname,"wc",0666,to_open)) >= 0) {
	    LOCINFO	*lip = pip->lip ;
	    int		pan = 0 ;
	    int		cl ;
	    const char	*cp ;

	    if (lip->f.trunc) lip->start = 0L ;

	    if ((rs = shio_isseekable(ofp)) > 0) {
	        if (lip->start >= 0) {
	            offset_t	o = lip->start ;
	            rs = shio_seek(ofp,o,SEEK_SET) ;
	        } else
	            rs = shio_seek(ofp,0L,SEEK_END) ;
	    } /* end if (stat) */

#ifdef	COMMENT
	    if (pip->f.bufnone)
	        shio_control(ofp,SHIO_CSETBUFNONE,TRUE) ;

	    if (pip->have.bufline)
	        shio_control(ofp,SHIO_CSETBUFLINE,pip->f.bufline) ;

	    if (pip->have.bufwhole)
	        shio_control(ofp,SHIO_CSETBUFWHOLE,pip->f.bufwhole) ;
#endif /* COMMENT */

	    if (rs >= 0) {
	        int	ai ;
	        int	f ;
	        const char	**argv = aip->argv ;
	        for (ai = aip->ai_continue ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	            if (! f) continue ;

	            cp = aip->argv[ai] ;
	            pan += 1 ;
	            rs = procfile(pip,ofp,cp) ;
	            wlen += rs ;

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	        SHIO	afile, *afp = &afile ;

	        if (strcmp(afname,"-") == 0) afname = STDINFNAME ;

	        if ((rs = shio_open(afp,afname,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                int	len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                cp = lbuf ;
	                cl = len ;
	                if (cl == 0) continue ;

	                pan += 1 ;
	                rs = procfile(pip,ofp,cp) ;
	                wlen += rs ;

	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            shio_close(afp) ;
	        } else {
	            shio_printf(pip->efp,
	                "%s: inaccessible argument-list (%d)\n",
	                pip->progname,rs) ;
	            shio_printf(pip->efp,"%s: afile=%s\n",
	                pip->progname,afname) ;
	        } /* end if */

	    } /* end if (procesing file argument file list) */

	    if ((rs >= 0) && (pan == 0)) {

	        cp = (ifname != NULL) ? ifname : "-" ;
	        pan += 1 ;
	        rs = procfile(pip,ofp,cp) ;
	        wlen += rs ;

	    } /* end if (standard-input) */

	    lip->nproc = pan ;
	    shio_close(ofp) ;
	} else {
	    shio_printf(pip->efp,"%s: inaccessible output (%d)\n",
	        pip->progname,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procargs: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */
#endif /* CF_PROCARGS */


static int procresp(pip,aip,bop,ofn,ifn,afn)
struct proginfo	*pip ;
struct arginfo	*aip ;
BITS		*bop ;
const char	*ofn ;
const char	*ifn ;
const char	*afn ;
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		used = 0 ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procresp: ent ofn=%s\n",ofn) ;
#endif
	if ((rs = locinfo_svcsbegin(lip)) >= 0) {
	    if ((rs = locinfo_gatherbegin(lip)) >= 0) {
	        const mode_t	om = 0666 ;
	        if ((rs = shio_open(ofp,ofn,"wct",om)) >= 0) {
	            const char	*tcn = "homepageXXXXXX" ;
	            char	template[MAXPATHLEN+1] ;
	            char	tbuf[MAXPATHLEN+1] ;
	            if ((rs = mkpath2(template,pip->tmpdname,tcn)) >= 0) {
	                const mode_t	om = 0664 ;
	                if ((rs = mktmpfile(tbuf,om,template)) >= 0) {
	                    if ((rs = procdoc(pip,aip,bop,tbuf,ifn,afn)) >= 0) {
			        if (pip->verboselevel > 0) {
	                            CGI	hdrs, *hp = &hdrs ;
	                            int	clen = rs ;
	                            if ((rs = cgi_start(hp,ofp)) >= 0) {
	                                if ((rs = prochdrs(pip,hp,clen)) >= 0) {
	                                    wlen += rs ;
	                                    rs = shio_writefile(ofp,tbuf) ;
	                                    wlen += rs ;
	                                } /* end if (prochdrs) */
	                                rs1 = cgi_finish(hp) ;
	                                if (rs >= 0) rs = rs1 ;
	                            } /* end if (cgi) */
				} /* end if (verbose) */
	                    } /* end if (procdoc) */
	                    uc_unlink(tbuf) ;
	                } /* end if (mktmpfile) */
	            } /* end if (mkpath-template) */
	            rs1 = shio_close(ofp) ;
	            if (rs >= 0) rs = rs1 ;
	        } else {
	            const char	*pn = pip->progname ;
	            const char	*fmt = "%s: inaccessible output (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	        } /* end if (file-output) */
	        rs1 = locinfo_gatherend(lip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (procdocbodymain-gather) */
	    rs1 = locinfo_svcsend(lip) ;
	    if (rs >= 0) rs = rs1 ;
	    used = rs1 ;
	} /* end if (locinfo-svcs) */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: max stack used=%u\n",pn,used) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procresp: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procresp) */


static int procdoc(pip,aip,bop,ofn,ifn,afn)
struct proginfo	*pip ;
struct arginfo	*aip ;
BITS		*bop ;
const char	*ofn ;
const char	*ifn ;
const char	*afn ;
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	HTM		h ;
	const mode_t	om = 0666 ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	const char	*lang = "en" ;

	if (ofn == NULL) return SR_FAULT ;
	if (ofn[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procdoc: ent ofn=%s\n",ofn) ;
#endif
	if ((rs = shio_open(ofp,ofn,"wct",om)) >= 0) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procdoc: yes?\n") ;
#endif
	    if ((rs = htm_start(&h,ofp,lang)) >= 0) {
	        const char	*hfname = lip->hfname ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("main/procdoc: opened() rs=%d\n",rs) ;
	            debugprintf("main/procdoc: hfname=%s\n",lip->hfname) ;
	        }
#endif
	        if ((rs = htm_headbegin(&h,hfname)) >= 0) {
	            rs1 = htm_headend(&h) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (htm-head) */
	        if (rs >= 0) {
	            if ((rs = htm_bodybegin(&h,NULL)) >= 0) {
	                rs = procdocbody(pip,&h) ;
	                rs1 = htm_bodyend(&h) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (htm-body) */
	        } /* end if (ok) */
	        wlen = htm_finish(&h) ;
	        if (rs >= 0) rs = wlen ;
	    } /* end if (htm) */
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procdoc: shio_open out rs=%d\n",rs) ;
#endif
	    shio_close(ofp) ;
	} /* end if (file-output) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procdoc: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procdoc) */


static int procdocbody(pip,hdp)
struct proginfo	*pip ;
HTM		*hdp ;
{
	int		rs ;
	int		rs1 ;

	if ((rs = htm_tagbegin(hdp,"div","particular",NULL,NULL)) >= 0) {
	    if (rs >= 0) {
	        rs = procdocbodyhdr(pip,hdp) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procdocbody: _bodyheader() rs=%d\n",rs) ;
#endif
	    } /* end if (ok) */
	    if (rs >= 0) {
	        rs = procdocbodymain(pip,hdp) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procdocbody: _bodymain() rs=%d\n",rs) ;
#endif
	    } /* end if (ok) */
	    if (rs >= 0) {
	        rs = procdocbodyfooter(pip,hdp) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procdocbody: _bodyfooter() rs=%d\n",rs) ;
#endif
	    } /* end if (ok) */
	    rs1 = htm_tagend(hdp,"div") ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (htm-div) */

	return rs ;
}
/* end subroutine (procdocbody) */


static int procdocbodyhdr(pip,hdp)
struct proginfo	*pip ;
HTM		*hdp ;
{
	int		rs ;
	int		rs1 ;

	if ((rs = htm_tagbegin(hdp,"header",NULL,NULL,NULL)) >= 0) {
	    if ((rs = htm_tagbegin(hdp,"h1",NULL,NULL,NULL)) >= 0) {
	        LOCINFO		*lip = pip->lip ;
	        const int	hlen = TBUFLEN ;
	        const char	*fn ;
	        char		hbuf[TBUFLEN+1] ;
	        fn = lip->wfname ;
	        if ((rs = procdocbodyhdrfile(pip,hbuf,hlen,fn)) >= 0) {
	            rs = htm_printline(hdp,hbuf,rs) ;
	        } else if (isNotPresent(rs)) {
	            const char	*hp = pip->org ;
	            if (hp == NULL) hp = "home page" ;
	            rs = htm_printline(hdp,hp,-1) ;
	        }
	        rs1 = htm_tagend(hdp,"h1") ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (htm-h1) */
	    rs1 = htm_tagend(hdp,"header") ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (htm-header) */

	return rs ;
}
/* end subroutine (procdocbodyhdr) */


static int procdocbodyhdrfile(PROGINFO *pip,char *hbuf,int hlen,const char *fn)
{
	bfile		hfile, *hfp = &hfile ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	if ((fn == NULL) || (fn[0] == '\0')) return SR_NOENT ; /* special */
	if ((rs = bopen(hfp,fn,"r",0666)) >= 0) {
	    int		cl ;
	    const char	*cp ;
	    while ((rs = breadline(hfp,hbuf,hlen)) > 0) {
	        len = rs ;
	        if (hbuf[len-1] == '\n') len -= 1 ;
	        if ((cl = sfshrink(hbuf,len,&cp)) > 0) {
	            if (cp != hbuf) {
	                memmove(hbuf,cp,cl) ;
	                hbuf[cl] = '\0' ;
	            }
	            len = cl ;
	        } else
	            len = 0 ;
	        if (len > 0) break ;
	    } /* end while */
	    rs1 = bclose(hfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* enbd if (bfile) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procdocbodyhdrfile) */


static int procdocbodymain(pip,hdp)
struct proginfo	*pip ;
HTM		*hdp ;
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_homepage/procdocbodyb_homepage: ent\n") ;
#endif

	if (lip->sfname != NULL) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        debugprintf("b_homepage/procdocbodyb_homepage: svcs¬\n") ;
	        if (lip->svcs != NULL) {
	            VECPSTR	*slp = lip->svcs ;
	            int		i ;
	            const char	*cp ;
	            for (i = 0 ; vecpstr_get(slp,i,&cp) >= 0 ; i += 1) {
	                if (cp == NULL) continue ;
	                debugprintf("b_homepage/procdocbodyb_homepage: svc=%s\n",cp) ;
	            }
	        }
	    }
#endif /* CF_DEBUG */
	    if ((rs = htm_tagbegin(hdp,"main",NULL,NULL,NULL)) >= 0) {

	        rs = procdocbodymain_svcs(pip,hdp) ;

	        rs1 = htm_tagend(hdp,"main") ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (htm-main) */
	} /* end if (svc-fname) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_homepage/procdocbodyb_homepage: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdocbodymain) */


static int procdocbodyfooter(pip,hdp)
struct proginfo	*pip ;
HTM		*hdp ;
{
	int		rs ;
	int		rs1 ;

	if ((rs = htm_tagbegin(hdp,"footer",NULL,NULL,NULL)) >= 0) {
	    if (rs >= 0) {
	        rs = htm_hr(hdp,NULL,NULL) ;
	    } /* end if (ok) */
	    if (rs >= 0) {
	        rs = procdocbodyfooterleft(pip,hdp) ;
	    } /* end if (ok) */
	    if (rs >= 0) {
	        rs = procdocbodyfooterext(pip,hdp) ;
	    } /* end if (ok) */
	    rs1 = htm_tagend(hdp,"footer") ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (htm-footer) */

	return rs ;
}
/* end subroutine (procdocbodyfooter) */


static int procdocbodyfooterleft(pip,hdp)
struct proginfo	*pip ;
HTM		*hdp ;
{
	int		rs ;
	int		rs1 ;
	if ((rs = htm_tagbegin(hdp,"div","left",NULL,NULL)) >= 0) {
	    if ((rs = htm_tagbegin(hdp,"p","center",NULL,NULL)) >= 0) {
	        const char	*n = NULL ;
	        const char	*class = "center" ;
	        const char	*href = "mailto:webmaster@rightcore.com" ;
	        const char	*title = "webmaster@rightcore.com" ;
	        if ((rs = htm_abegin(hdp,class,n,href,title)) >= 0) {
	            rs = htm_printline(hdp,"email webmaster",-1) ;
	            rs1 = htm_aend(hdp) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (htm_abegin) */
	        rs1 = htm_tagend(hdp,"p") ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (htm-p) */
	    if (rs >= 0) {
	        if ((rs = htm_tagbegin(hdp,"p","center",NULL,NULL)) >= 0) {
	            const char	*t = "Copyright (c) 2015-2016 "
	            "David A.D. Morano."
	            "  All rights reserved." ;
	            rs = htm_printline(hdp,t,-1) ;
	            rs1 = htm_tagend(hdp,"p") ;
	            if (rs >= 0) rs = rs1 ;
	        }
	    } /* end if (ok) */
	    rs1 = htm_tagend(hdp,"div") ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (htm-div) */

	return rs ;
}
/* end subroutine (procdocbodyfooterleft) */


static int procdocbodyfooterext(pip,hdp)
struct proginfo	*pip ;
HTM		*hdp ;
{
	int		rs ;
	int		rs1 ;
	const char *src = 
	"http://rightcore.com/CGI/wc?db=rightcore&c=homepage" ;
	if ((rs = htm_tagbegin(hdp,"div","exticons",NULL,NULL)) >= 0) {
	    const int	w = 20 ;
	    const int	h = 22 ;
	    const char	*class = NULL ;
	    const char	*id = NULL ;
	    const char	*title = "web counter" ;
	    const char	*alt = "web counter" ;
	    rs = htm_img(hdp,class,id,src,title,alt,w,h) ;
	    rs1 = htm_tagend(hdp,"div") ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (htm-div) */

	return rs ;
}
/* end subroutine (procdocbodyfooterext) */


static int procdocbodymain_svcs(struct proginfo *pip,HTM *hdp)
{
	LOCINFO		*lip = pip->lip ;
	SVCFILE		*slp ;
	GATHER		*glp ;
	int		rs ;

	slp = &lip->s ;
	glp = &lip->g ;

	rs = procdocbodymain_svcprint(pip,hdp,glp,slp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_homepage/procdocbodymain_svcs: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdocbodymain_svcs) */


static int procdocbodymain_svcprint(pip,hdp,glp,sfp)
PROGINFO	*pip ;
HTM		*hdp ;
GATHER		*glp ;
SVCFILE		*sfp ;
{
	LOCINFO		*lip = pip->lip ;
	SVCFILE_ENT	se ;
	int		rs = SR_OK ;
	int		i ;
	const char	*snp ;

	for (i = 0 ; locinfo_svclistget(lip,i,&snp) >= 0 ; i += 1) {
	    if (snp != NULL) {
	        const int	el = SVCENTLEN ;
	        char		eb[SVCENTLEN+1] ;
	        void		*n = NULL ;
	        if ((rs = svcfile_fetch(sfp,snp,n,&se,eb,el)) > 0) {
	            rs = procdocbodymain_svcer(pip,hdp,glp,&se) ;
	        }
	    } /* end if (non-null) */
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_homepage/procdocbodymain_svcprint: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdocbodymain_svcprint) */


static int procdocbodymain_svcer(pip,hdp,glp,sep)
struct proginfo	*pip ;
HTM		*hdp ;
GATHER		*glp ;
SVCFILE_ENT	*sep ;
{
	const int	n = sep->nkeys ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		vl ;
	const char	*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procdocbodymain_svcer: svc=%s\n",sep->svc) ;
	    if ((vl = svckey(sep->keyvals,n,"h",&vp)) > 0) {
	        debugprintf("procdocbodymain_svcer: h=>%t<\n",vp,vl) ;
	    }
	}
#endif /* CF_DEBUG */

	if ((vl = svckey(sep->keyvals,n,"h",&vp)) > 0) {
	    int		cl ;
	    const char	*cp ;
	    if ((cl = sfdequote(vp,vl,&cp)) > 0) {
	        if ((rs = htm_tagbegin(hdp,"h3",NULL,NULL,NULL)) >= 0) {
	            rs = htm_printline(hdp,cp,cl) ;
	            rs1 = htm_tagend(hdp,"h3") ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (htm-h1) */
	    } /* end if (dequote) */
	    if (rs >= 0) {
	        const char	*k = "include" ;
	        if ((vl = svckey(sep->keyvals,n,k,&vp)) > 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("procdocbodymain_svcer: include\n") ;
#endif
	            rs = procdocbodymain_svcerinc(pip,hdp,vp,vl) ;
	        } else if ((rs = svckey_isfile(sep->keyvals,n,&vp)) > 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("procdocbodymain_svcer: isfile\n") ;
#endif
	            rs = procdocbodymain_svcerfile(pip,hdp,glp,sep) ;

	        } else if ((rs = svckey_isexec(sep->keyvals,n,&vp)) > 0) {
	            rs = 2 ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("procdocbodymain_svcer: isexec\n") ;
#endif
	        } /* end if */
	    } /* end if (ok) */
	} /* end if (had a text element) */

	return rs ;
}
/* end subroutine (procdocbodymain_svcer) */


static int procdocbodymain_svcerinc(pip,hdp,sp,sl)
struct proginfo	*pip ;
HTM		*hdp ;
const char	*sp ;
int		sl ;
{
	int		rs ;
	int		rs1 ;
	char		ibuf[MAXPATHLEN+1] ;

	if ((rs = mkincfname(ibuf,pip,sp,sl)) >= 0) {
	    bfile	ifile, *ifp = &ifile ;
	    if ((rs = bopen(ifp,ibuf,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        char		lbuf[LINEBUFLEN+1] ;
	        while ((rs = bread(ifp,lbuf,llen)) > 0) {
	            rs = htm_write(hdp,lbuf,rs) ;
	            if (rs < 0) break ;
	        } /* end while */
	        rs1 = bclose(ifp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else if (isNotPresent(rs))
	        rs = SR_OK ;
	} /* end if (mkincfname) */

	return rs ;
}
/* end subroutine (procdocbodymain_svcerinc) */


static int procdocbodymain_svcerfile(pip,hdp,glp,sep)
PROGINFO	*pip ;
HTM		*hdp ;
GATHER		*glp ;
SVCFILE_ENT	*sep ;
{
	int		rs ;
	int		rs1 ;
	const char	*svc = sep->svc ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("procdocbodymain_svcerfile: ent svc=%s\n",svc) ;
#endif

	if ((rs = gather_getlines(glp,svc)) >= 0) {
	    const int	n = sep->nkeys ;
	    const int	lines = rs ;
	    int		vl ;
	    const char	*vp ;
	    if ((vl = svckey_dequote(sep->keyvals,n,"h",&vp)) > 0) {
	        NULSTR		ts ;
	        const char	*t ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("procdocbodymain_svcerfile: t=>%t<\n",vp,vl) ;
#endif
	        if ((rs = nulstr_start(&ts,vp,vl,&t)) >= 0) {
	            const char	*dp ;
	            if ((rs = gather_getbuf(glp,svc,&dp)) >= 0) {
	                const int	dl = rs ;
	                const int	r = (lines+1) ;
	                const int	c = COLUMNS ;
	                const char	*class = NULL ;
	                const char	*id = NULL ;
	                if ((rs = htm_textbegin(hdp,class,id,t,r,c)) >= 0) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(5))
	                        debugprintf("procdocbodymain_svcerfile: "
	                            "htm_write()\n") ;
#endif

	                    if (dl > 0) {
	                        if ((rs = htm_write(hdp,dp,dl)) >= 0) {
	                            if (dp[dl-1] != '\n') {
	                                rs = htm_putc(hdp,CH_NL) ;
	                            }
	                        }
	                    } /* end if (positive) */

#if	CF_DEBUG
	                    if (DEBUGLEVEL(5))
	                        debugprintf("procdocbodymain_svcerfile: "
	                            "htm_write() rs=%d\n",rs) ;
#endif
	                    rs1 = htm_textend(hdp) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (htm-text) */
#if	CF_DEBUG
	                if (DEBUGLEVEL(5))
	                    debugprintf("procdocbodymain_svcerfile: "
	                        "htm-out rs=%d\n",rs) ;
#endif
	            } /* end if (gather_getbuf) */
#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("procdocbodymain_svcerfile: "
	                    "gather rs=%d\n",rs) ;
#endif
	            rs1 = nulstr_finish(&ts) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (nulstr) */
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("procdocbodymain_svcerfile: "
	                "nulstr-out rs=%d\n",rs) ;
#endif
	    } /* end if (svckey) */
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("procdocbodymain_svcerfile: "
	            "dequote-out rs=%d\n",rs) ;
#endif
	} /* end if (gather_getlines) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("procdocbodymain_svcerfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdocbodymain_svcerfile) */


static int prochdrs(struct proginfo *pip,CGI *hp,int clen)
{
	const int	tlen = TBUFLEN ;
	int		rs ;
	int		wlen = 0 ;
	char		tbuf[TBUFLEN+1] ;

	if ((rs = ctdeci(tbuf,tlen,clen)) >= 0) {
	    rs = cgi_hdr(hp,"content-length",tbuf,rs) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    time_t	t = pip->daytime ;
	    rs = cgi_hdrdate(hp,t) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    const char	*k = "content-type" ;
	    const char	*v = "text/html; charset=ISO-8859-1" ;
	    rs = cgi_hdr(hp,k,v,-1) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = cgi_eoh(hp) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (prochdrs) */


static int mkourname(rbuf,pip,inter,sp,sl)
char		rbuf[] ;
PROGINFO	*pip ;
const char	*inter ;
const char	*sp ;
int		sl ;
{
	int		rs = SR_OK ;
	const char	*pr = pip->pr ;
	const char	*sn = pip->searchname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_homepage/mkourname: ent int=%s s=%t\n",
	        inter,sp,sl) ;
#endif

	if (strnchr(sp,sl,'/') != NULL) {
	    if (sp[0] != '/') {
	        rs = mkpath2w(rbuf,pr,sp,sl) ;
	    } else {
	        rs = mkpath1w(rbuf,sp,sl) ;
	    }
	} else {
	    rs = mkpath4w(rbuf,pr,inter,sn,sp,sl) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_homepage/mkourname: ret rs=%d\n",rs) ;
	    debugprintf("b_homepage/mkourname: ret rbuf=%s\n",rbuf) ;
	}
#endif

	return rs ;
}
/* end subroutine (mkourname) */


static int mkincfname(char *ibuf,PROGINFO *pip,const char *sp,int sl)
{
	int	rs ;

	if (sp[0] != '/') {
	    const char	*pr = pip->pr ;
	    const char	*sn = pip->searchname ;
	    const char	*inter = ETCCNAME ;
	    rs = mkpath4w(ibuf,pr,inter,sn,sp,sl) ;
	} else {
	    rs = mkpath1w(ibuf,sp,sl) ;
	}

	return rs ;
}
/* end subroutine (mkincfname) */


/* calculate a file name */
static int setfname(pip,fname,ebuf,el,f_def,dname,name,suf)
struct proginfo	*pip ;
char		fname[] ;
const char	ebuf[] ;
const char	dname[], name[], suf[] ;
int		el ;
int		f_def ;
{
	int		rs = SR_OK ;
	int		ml ;
	const char	*np ;
	char		tmpname[MAXNAMELEN + 1] ;

	if ((f_def && (ebuf[0] == '\0')) ||
	    (strcmp(ebuf,"+") == 0)) {

	    np = name ;
	    if ((suf != NULL) && (suf[0] != '\0')) {
	        np = tmpname ;
	        mkfnamesuf1(tmpname,name,suf) ;
	    }

	    if (np[0] != '/') {
	        if ((dname != NULL) && (dname[0] != '\0')) {
	            rs = mkpath3(fname,pip->pr,dname,np) ;
	        } else
	            rs = mkpath2(fname, pip->pr,np) ;
	    } else
	        rs = mkpath1(fname, np) ;

	} else if (strcmp(ebuf,"-") == 0) {

	    fname[0] = '\0' ;

	} else if (ebuf[0] != '\0') {

	    np = ebuf ;
	    if (el >= 0) {
	        np = tmpname ;
	        ml = MIN(MAXPATHLEN,el) ;
	        strwcpy(tmpname,ebuf,ml) ;
	    }

	    if (ebuf[0] != '/') {
	        if (strchr(np,'/') != NULL) {
	            rs = mkpath2(fname,pip->pr,np) ;
	        } else {
	            if ((dname != NULL) && (dname[0] != '\0')) {
	                rs = mkpath3(fname,pip->pr,dname,np) ;
	            } else
	                rs = mkpath2(fname,pip->pr,np) ;
	        } /* end if */
	    } else
	        rs = mkpath1(fname,np) ;

	} /* end if */

	return rs ;
}
/* end subroutine (setfname) */


static int svckey(const char *(*kv)[2],int n,const char *np,const char **vpp)
{
	int		i ;
	int		vl = 0 ;
	if (vpp != NULL) *vpp = NULL ;
	for (i = 0 ; i < n ; i += 1) {
	    if (strcmp(kv[i][0],np) == 0) {
	        if (vpp != NULL) *vpp = kv[i][1] ;
	        vl = strlen(kv[i][1]) ;
	        break ;
	    }
	} /* end for */
	return vl ;
}
/* end subroutine (svckey) */


static int svckey_dequote(kv,n,np,vpp)
const char	*(*kv)[2] ;
int		n ;
const char	*np ;
const char	**vpp ;
{
	int		vl ;
	int		cl = 0 ;
	const char	*vp, *cp ;
	if ((vl = svckey(kv,n,np,&vp)) > 0) {
	    if ((cl = sfdequote(vp,vl,&cp)) > 0) {
	        if (vpp != NULL) *vpp = cp ;
	    }
	}
	return cl ;
}
/* end subroutine (svckey_dequote) */


static int svckey_isfile(const char *(*kv)[2],int n,const char **vpp)
{
	int		vl ;
	const char	*sp = "file" ;
	vl = svckey(kv,n,sp,vpp) ;
	return vl ;
}
/* end subroutine (svckey_isfile) */


static int svckey_isexec(const char *(*kv)[2],int n,const char **vpp)
{
	int		rs = SR_OK ;
	int		i ;
	int		vl = 0 ;
	for (i = 0 ; isexecs[i] != NULL ; i += 1) {
	    vl = svckey(kv,n,isexecs[i],vpp) ;
	    if (vl > 0) break ;
	}
	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (svckey_isexec) */


static int svckey_svcopts(const char *(*kv)[2],int n)
{
	int		vl ;
	int		ow = 0 ;
	const char	*k = "opts" ;
	const char	*vp ;
	if ((vl = svckey(kv,n,k,&vp)) > 0) {
	    int		i ;
	    int		cl ;
	    const char	*cp ;
	    const char	*tp ;
	    while ((tp = strnpbrk(vp,vl," ,")) != NULL) {
	        if ((cl = sfshrink(vp,(tp-vp),&cp)) > 0) {
	            if ((i = matstr(svcopts,cp,cl)) >= 0) {
	                ow |= (1<<i) ;
	            }
	        }
	        vl -= ((tp+1)-vp) ;
	        vp = (tp+1) ;
	    } /* end while */
	    if (vl > 0) {
	        if ((cl = sfshrink(vp,vl,&cp)) > 0) {
	            if ((i = matstr(svcopts,cp,cl)) >= 0) {
	                ow |= (1<<i) ;
	            }
	        }
	    }
	} /* end if (svckey) */
	return ow ;
}
/* end subroutine (svckey_svcopts) */


static int gather_start(GATHER *glp,const char *termtype,int cols)
{
	int		rs ;
	const char	*cp ;
	if (glp == NULL) return SR_FAULT ;
	if (termtype == NULL) return SR_FAULT ;
	if (cols <= 0) cols = COLUMNS ;
	memset(glp,0,sizeof(GATHER)) ;
	if ((rs = uc_mallocstrw(termtype,-1,&cp)) >= 0) {
	    glp->termtype = cp ;
	    glp->cols = cols ;
	    if ((rs = ptm_create(&glp->m,NULL)) >= 0) {
	        if ((rs = ptc_create(&glp->c,NULL)) >= 0) {
	            rs = vechand_start(&glp->ents,6,0) ;
	            if (rs < 0)
	                ptc_destroy(&glp->c) ;
	        }
	        if (rs < 0)
	            ptm_destroy(&glp->m) ;
	    } /* end if (ptm_create) */
	    if (rs < 0) {
	        uc_free(glp->termtype) ;
	        glp->termtype = NULL ;
	    }
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (gather_start) */


static int gather_finish(GATHER *glp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		used = 0 ;

	if (glp == NULL) return SR_FAULT ;

	rs1 = gather_entfins(glp) ;
	if (rs >= 0) rs = rs1 ;
	used = rs1 ;

#if	CF_DEBUGS
	debugprintf("gather_finish: 1 rs=%d\n",rs) ;
#endif

	rs1 = vechand_finish(&glp->ents) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("gather_finish: 2 rs=%d\n",rs) ;
#endif

	rs1 = ptc_destroy(&glp->c) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("gather_finish: 3 rs=%d\n",rs) ;
#endif

	rs1 = ptm_destroy(&glp->m) ;
	if (rs >= 0) rs = rs1 ;

	if (glp->termtype != NULL) {
	    rs1 = uc_free(glp->termtype) ;
	    if (rs >= 0) rs = rs1 ;
	    glp->termtype = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("gather_finish: ret rs=%d u=%d\n",rs,used) ;
#endif

	return (rs >= 0) ? used : rs ;
}
/* end subroutine (gather_finish) */


static int gather_file(glp,svc,f_to,fp,fl)
GATHER		*glp ;
const char	*svc ;
int		f_to ;
const char	*fp ;
int		fl ;
{
	const int	fsize = sizeof(FILER) ;
	int		rs ;
	void		*p ;

#if	CF_DEBUGS
	debugprintf("b_homepage/gather_file: ent\n") ;
	debugprintf("b_homepage/gather_file: svc=%s\n",svc) ;
	debugprintf("b_homepage/gather_file: fn=%t\n",fp,fl) ;
#endif

	if (glp == NULL) return SR_FAULT ;
	if (svc == NULL) return SR_FAULT ;
	if (fp == NULL) return SR_FAULT ;
	if (svc[0] == '\0') return SR_INVALID ;
	if (fp[0] == '\0') return SR_INVALID ;

	if ((rs = uc_malloc(fsize,&p)) >= 0) {
	    FILER	*fep = p ;
	    const int	c = glp->cols ;
	    const char	*tt = (f_to) ? glp->termtype : NULL ;
	    if ((rs = filer_start(fep,tt,c,svc,fp,fl)) >= 0) {
	        if ((rs = vechand_add(&glp->ents,fep)) >= 0) {
	            glp->nout += 1 ;
	        }
	        if (rs < 0)
	            filer_finish(fep) ;
#if	CF_DEBUGS
	        debugprintf("b_homepage/gather_file: vechand_add-out rs=%d\n",rs) ;
#endif
	    } /* end if (filer_start) */
	    if (rs < 0)
	        uc_free(p) ;
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("b_homepage/gather_file: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (gather_file) */


static int gather_entfins(GATHER *glp)
{
	FILER		*fep ;
	vechand		*elp = &glp->ents ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		used = 0 ;

	if (glp == NULL) return SR_FAULT ;

	for (i = 0 ; vechand_get(elp,i,&fep) >= 0 ; i += 1) {
	    if (fep != NULL) {
	        rs1 = filer_finish(fep) ;
	        if (rs >= 0) rs = rs1 ;
	        if ((rs1 >= 0) && (rs1 > used)) used = rs1 ;
	        rs1 = uc_free(fep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("b_homepage/gather_entfins: ret rs=%d u=%d\n",rs,used) ;
#endif

	return (rs >= 0) ? used : rs ;
}
/* end subroutine (gather_entfins) */


static int gather_getlines(GATHER *glp,const char *svc)
{
	FILER		*fep = NULL ;
	int		rs = SR_OK ;
	int		lines = 0 ;

#if	CF_DEBUGS
	debugprintf("b_homepage/gather_getlines: ent svc=%s\n",svc) ;
#endif

	if (glp == NULL) return SR_FAULT ;
	if (svc == NULL) return SR_FAULT ;

	if (svc[0] == '\0') return SR_INVALID ;

	{
	    VECHAND	*elp = &glp->ents ;
	    int		i ;
	    for (i = 0 ; (rs = vechand_get(elp,i,&fep)) >= 0 ; i += 1) {
	        if (fep != NULL) {
	            if ((rs = filer_havesvc(fep,svc)) > 0) break ;
	        }
	        if (rs < 0) break ;
	    } /* end for */
	} /* end block */

	if ((rs >= 0) && (fep != NULL)) {
	    rs = filer_getlines(fep) ;
	    lines = rs ;
	}

#if	CF_DEBUGS
	debugprintf("b_homepage/gather_getlines: ret rs=%d lns=%u\n",
	    rs,lines) ;
#endif

	return (rs >= 0) ? lines : rs ;
}
/* end subroutine (gather_getlines) */


static int gather_getbuf(GATHER *glp,const char *svc,const char **rpp)
{
	FILER		*fep = NULL ;
	int		rs = SR_OK ;
	int		rl = 0 ;

	if (glp == NULL) return SR_FAULT ;
	if (rpp != NULL) *rpp = NULL ;

	{
	    VECHAND	*elp = &glp->ents ;
	    int		i ;
	    for (i = 0 ; (rs = vechand_get(elp,i,&fep)) >= 0 ; i += 1) {
	        if (fep != NULL) {
	            if ((rs = filer_havesvc(fep,svc)) > 0) break ;
	        }
	    } /* end for */
	} /* end block */

	if ((rs >= 0) && (fep != NULL)) {
	    rs = filer_getbuf(fep,rpp) ;
	    rl = rs ;
	}

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (gather_getbuf) */


static int filer_start(fep,tt,cols,svc,fp,fl)
FILER		*fep ;
const char	*tt ;
int		cols ;
const char	*svc ;
const char	*fp ;
int		fl ;
{
	const int	to = 5 ;
	int		rs ;
	int		rs1 ;
	int		size = 0 ;
	char		*bp ;

#if	CF_DEBUGS
	debugprintf("b_homepage/filer_start: ent svc=%s\n",svc) ;
	debugprintf("b_homepage/filer_start: fn=%t\n",fp,fl) ;
#endif

	memset(fep,0,sizeof(FILER)) ;
	fep->cols = cols ;
	fep->to = to ;
	fep->f_stackcheck = CF_STACKCHECK ;

	size += (strlen(svc)+1) ;
	size += (strnlen(fp,fl)+1) ;
	if (tt != NULL) {
	    size += (strlen(tt)+1) ;
	}
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    const int	dsize = FILER_DSIZE ;
	    fep->a = bp ;
	    fep->svc = bp ;
	    bp = (strwcpy(bp,svc,-1)+1) ;
	    fep->fname = bp ;
	    bp = (strwcpy(bp,fp,fl)+1) ;
	    if (tt != NULL) {
	        fep->termtype = bp ;
	        bp = (strwcpy(bp,tt,-1)+1) ;
	    }
	    if ((rs = uc_malloc(dsize,&bp)) >= 0) {
	        fep->dbuf = bp ;
	        fep->dsize = dsize ;
	        bp[0] = '\0' ;
	        if ((rs = filer_stackbegin(fep)) >= 0) {
	            PTA		ta ;
	            tworker	tw = (tworker) filer_worker ;
	            pthread_t	tid ;
	            if ((rs = pta_create(&ta)) >= 0) {
	                const caddr_t	saddr = fep->saddr ;
	                int		ssize = fep->ssize ;
	                if ((rs = pta_setstack(&ta,saddr,ssize)) >= 0) {
	                    if ((rs = uptcreate(&tid,&ta,tw,fep)) >= 0) {
	                        fep->tid = tid ;
	                        fep->f_running = TRUE ;
	                    }
	                } /* end if (pta_setstack) */
	                rs1 = pta_destroy(&ta) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (pta) */
	            if (rs < 0)
	                filer_stackend(fep) ;
	        } /* end if (filer_stackbegin) */
	        if (rs < 0) {
	            uc_free(fep->dbuf) ;
	            fep->dbuf = NULL ;
	        }
	    } /* end if (m-a data) */
	    if (rs < 0) {
	        uc_free(fep->a) ;
	        fep->a = NULL ;
	    }
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("b_homepage/filer_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (filer_start) */


static int filer_stackbegin(FILER *fep)
{
	size_t		ms ;
	const int	ps = getpagesize() ;
	const int	ssize = MAX(STACKSIZE,PTHREAD_STACK_MIN) ;
	int		rs ;
	int		mp = (PROT_READ|PROT_WRITE) ;
	int		mf = (MAP_PRIVATE|MAP_NORESERVE|MAP_ANON) ;
	int		ss ;
	int		fd = -1 ;
	void		*md ;
	ss = iceil(ssize,ps) ;
	ms = (ss+(2*ps)) ;
	if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	    caddr_t	maddr = md ;
	    fep->maddr = md ;
	    fep->msize = ms ;
	    fep->saddr = (maddr + ps) ;
	    fep->ssize = ss ;
	    if ((rs = u_mprotect((maddr-0),ps,PROT_NONE)) >= 0) {
	        if ((rs = u_mprotect((maddr+ms-ps),ps,PROT_NONE)) >= 0) {
	            if (fep->f_stackcheck) {
	                rs = filer_stackfill(fep) ;
	            }
	        } /* end if (u_mprotect) */
	    } /* end if (u_mprotect) */
	} /* end if (u_mmap) */
	return rs ;
}
/* end subroutine (filer_stackbegin) */


static int filer_stackend(FILER *fep)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		used = 0 ;
	if (fep->saddr != NULL) {
	    if (fep->f_stackcheck) {
	        used = filer_stackused(fep) ;
	        if (rs >= 0) rs = used ;
	    }
	    rs1 = u_munmap(fep->maddr,fep->msize) ;
	    if (rs >= 0) rs = rs1 ;
	    fep->maddr = NULL ;
	    fep->saddr = NULL ;
	    fep->msize = 0 ;
	    fep->ssize = 0 ;
	}
	return (rs >= 0) ? used : rs ;
}
/* end subroutine (filer_stackend) */


static int filer_stackfill(FILER *fep)
{
	int		rs = SR_OK ;
	if ((fep->saddr != NULL) && fep->f_stackcheck) {
	    const int	shift = ffbsi(sizeof(int)) ;
	    const int	pattern = 0x5A5A5A5A ;
	    int		*ip = (int *) fep->saddr ;
	    int		il ;
	    int		i ;
	    il = (fep->ssize >> shift) ;
	    for (i = 0 ; i < il ; i += 1) *ip++ = pattern ;
	} /* end if (stackcheck) */
	return rs ;
}
/* end subroutine (filer_stackfill) */


static int filer_stackused(FILER *fep)
{
	int		rs = SR_OK ;
	int		used = 0 ;
	if (fep->saddr != NULL) {
	    const int	shift = ffbsi(sizeof(int)) ;
	    const int	pattern = 0x5A5A5A5A ;
	    int		*ip = (int *) fep->saddr ;
	    int		il ;
	    int		i ;
	    il = (fep->ssize >> shift) ;
	    for (i = 0 ; i < il ; i += 1) {
	        if (*ip++ != pattern) break ;
	    }
	    used = ((il-i) << shift) ;
	}
	return (rs >= 0) ? used : rs ;
}
/* end subroutine (filer_stackused) */


static int filer_finish(FILER *fep)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		used = 0 ;

#if	CF_DEBUGS
	debugprintf("b_homepage/filer_finish: ent svc=%s\n",fep->svc) ;
#endif

	rs1 = filer_getlines(fep) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = filer_stackend(fep) ;
	if (rs >= 0) rs = rs1 ;
	if (rs >= 0) used = rs ;

#if	CF_DEBUGS
	debugprintf("b_homepage/filer_finish: svc=%s stack used=%u\n",
	    fep->svc,used) ;
#endif

	if (fep->dbuf != NULL) {
	    rs1 = uc_free(fep->dbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    fep->dbuf = NULL ;
	}

	if (fep->a != NULL) {
	    rs1 = uc_free(fep->a) ;
	    if (rs >= 0) rs = rs1 ;
	    fep->a = NULL ;
	}

	fep->fname = NULL ;
	fep->svc = NULL ;

#if	CF_DEBUGS
	debugprintf("b_homepage/filer_finish: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? used : rs ;
}
/* end subroutine (filer_finish) */


static int filer_worker(FILER *fep)
{
	int		rs ;
	int		rl = 0 ;

#if	CF_DEBUGS
	debugprintf("filer_worker: ent svc=%s\n",fep->svc) ;
	debugprintf("filer_worker: fn=%s\n",fep->fname) ;
	debugprintf("filer_worker: uc_open() svc=%s\n",fep->svc) ;
#endif

	if ((rs = uc_open(fep->fname,O_RDONLY,0666)) >= 0) {
	    int		fd = rs ;
#if	CF_DEBUGS
	    debugprintf("filer_worker: uc_open() svc=%s fd=%d\n",fep->svc,fd) ;
#endif
	    if ((rs = filer_workread(fep,fd)) > 0) {
	        int		lines = 0 ;
	        int		sl = rs ;
	        const char	*sp = fep->dbuf ;
	        const char	*tp ;
	        rl = rs ;
	        while ((tp = strnchr(sp,sl,'\n')) != NULL) {
	            lines += 1 ;
	            sl -= ((tp+1)-sp) ;
	            sp = (tp+1) ;
	        } /* end while */
	        fep->lines = lines ;
	    } /* end if (filer_workread) */
	    u_close(fd) ;
	} /* end if (file) */

#if	CF_DEBUGS
	debugprintf("filer_worker: ret rs=%d svc=%s rl=%u lines=%u\n",
	    rs,fep->svc,rl,fep->lines) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (filer_worker) */


static int filer_workread(FILER *fep,int fd)
{
	int		rs ;
#if	CF_DEBUGS
	debugprintf("filer_workread: ent svc=%s tt=%s\n",
	    fep->svc,fep->termtype) ;
#endif
	if (fep->termtype != NULL) {
	    rs = filer_workreadterm(fep,fd) ;
	} else {
	    rs = filer_workreadreg(fep,fd) ;
	}
#if	CF_DEBUGS
	debugprintf("filer_workread: ret rs=%d svc=%s\n",rs,fep->svc) ;
#endif
	return rs ;
}
/* end subroutine (filer_workread) */


static int filer_workreadreg(FILER *fep,int fd)
{
	const int	rlen = (fep->dsize-1) ;
	const int	to = fep->to ;
	int		rs = SR_OK ;
	int		ml ;
	int		len ;
	int		rl = 0 ;
	char		*rp = fep->dbuf ;
#if	CF_DEBUGS
	debugprintf("filer_workreadreg: ent svc=%s\n",fep->svc) ;
#endif
	while ((rs >= 0) && (rl < rlen)) {
	    ml = (rlen-rl) ;
#if	CF_DEBUGS
	    debugprintf("filer_workreadreg: svc=%s uc_reade() ml=%d\n",
	        fep->svc,ml) ;
#endif
#if	CF_UCREADE
	    rs = uc_reade(fd,rp,ml,to,0) ;
#else
	    rs = u_read(fd,rp,ml) ;
#endif
#if	CF_DEBUGS
	    debugprintf("filer_workreadreg: svc=%s uc_reade() rs=%d\n",
	        fep->svc,rs) ;
#endif
	    if (rs <= 0) break ;
	    len = rs ;
	    rp += len ;
	    rl += len ;
	} /* end while */
#if	CF_DEBUGS
	debugprintf("filer_workreadreg: ret rs=%d svc=%s rl=%u\n",
	    rs,fep->svc,rl) ;
#endif
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (filer_workreadreg) */


static int filer_workreadterm(FILER *fep,int fd)
{
	TERMOUT		out ;
	const int	cols = fep->cols ;
	int		rs ;
	int		rs1 ;
	int		rl = 0 ;
	const char	*tt = fep->termtype ;
	if ((rs = termout_start(&out,tt,-1,cols)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    char	*lbuf ;
	    if ((rs = uc_malloc((llen+1),&lbuf)) >= 0) {
	        FILEBUF	b ;
	        if ((rs = filebuf_start(&b,fd,0L,512,0)) >= 0) {
	            const int	to = fep->to ;
	            int		len ;
	            void	*n = NULL ;
	            char	*dp = fep->dbuf ;
	            while ((rs = filebuf_readlines(&b,lbuf,llen,to,n)) > 0) {
	                len = rs ;
#if	CF_DEBUGS
	                debugprintf("filer_workreadterm: len=%u\n",len) ;
	                debugprintf("filer_workreadterm: termline dp{%p}\n",
	                    dp) ;
#endif
	                rs = filer_workreadtermline(fep,&out,dp,lbuf,len) ;
#if	CF_DEBUGS
	                debugprintf("filer_workreadterm: termline() rs=%d\n",
	                    rs) ;
#endif
	                if (rs > 0) {
	                    rl += rs ;
	                    dp += rs ;
	                }
	                if (rs <= 0) break ;
	            } /* end while (reading lines) */
	            rs1 = filebuf_finish(&b) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (filebuf) */
	        uc_free(lbuf) ;
	    } /* end if (m-a) */
	    rs1 = termout_finish(&out) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (termout) */
#if	CF_DEBUGS
	debugprintf("filer_workreadterm: ret rs=%d svc=%s rl=%u\n",
	    rs,fep->svc,rl) ;
#endif
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (filer_workreadterm) */


static int filer_workreadtermline(fep,top,rp,lbuf,len)
FILER		*fep ;
TERMOUT		*top ;
char		*rp ;
const char	*lbuf ;
int		len ;
{
	int		rlen = (fep->dsize-1) ;
	int		rs ;
	int		rl = 0 ;
#if	CF_DEBUGS
	debugprintf("filer_workreadtermline: ent l=>%t<\n",
	    lbuf,strlinelen(lbuf,len,40)) ;
#endif
	if ((rs = termout_load(top,lbuf,len)) >= 0) {
	    const int	ln = rs ;
	    int		lenrem = (rlen-(rp-fep->dbuf)) ;
	    int		i ;
	    int		ll ;
	    int		ml ;
	    const char	*lp ;
	    for (i = 0 ; i < ln ; i += 1) {
	        ll = termout_getline(top,i,&lp) ;
	        if (ll < 0) break ;
#if	CF_DEBUGS
	        debugprintf("filer_workreadtermline: l=>%t<\n",
	            lp,strlinelen(lp,ll,30)) ;
#endif
	        if (lenrem > 0) {
	            ml = MIN(ll,(lenrem-1)) ;
#if	CF_DEBUGS
	            debugprintf("filer_workreadtermline: ml=%u\n",ml) ;
#endif
	            rp = strwcpy(rp,lp,ml) ;
	            *rp++ = CH_NL ;
	            rl += (ml+1) ;
	            lenrem -= (ml+1) ;
	        } /* end if (positive) */
	        if (lenrem == 0) break ;
	    } /* end for */
	} /* end if (termout_load) */
#if	CF_DEBUGS
	debugprintf("filer_workreadtermline: ret rs=%d rl=%u\n",rs,rl) ;
#endif
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (filer_workreadtermline) */


static int filer_getlines(FILER *fep)
{
	int		rs ;
#if	CF_DEBUGS
	debugprintf("b_homepage/filer_getlines: ent svc=%s f_run=%u\n",
	    fep->svc,fep->f_running) ;
#endif
	if (fep->f_running) {
	    int	trs ;
	    fep->f_running = FALSE ;
	    if ((rs = uptjoin(fep->tid,&trs)) >= 0) {
#if	CF_DEBUGS
	        debugprintf("b_homepage/filer_getlines: uptjoin() trs=%d\n",
	            trs) ;
#endif
	        if (trs >= 0) {
	            fep->dl = trs ;
	            rs = fep->lines ;
	        } else
	            rs = trs ;
	    } /* end if (pthead-join) */
#if	CF_DEBUGS
	    debugprintf("b_homepage/filer_getlines: uptjoin-out rs=%d\n",rs) ;
#endif
	} else
	    rs = fep->lines ;
#if	CF_DEBUGS
	debugprintf("b_homepage/filer_getlines: ret rs=%d svc=%s lns=%u\n",
	    rs,fep->svc,fep->lines) ;
#endif
	return rs ;
}
/* end subroutine (filer_getlines) */


static int filer_getbuf(FILER *fep,const char **rpp)
{
	int		rs ;
	int		rl = 0 ;
	if (rpp != NULL) *rpp = NULL ;
	if ((rs = filer_getlines(fep)) >= 0) {
	    if (rpp != NULL) *rpp = fep->dbuf ;
	    rl = fep->dl ;
	}
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (filer_getbuf) */


static int filer_havesvc(FILER *fep,const char *svc)
{
	int		f = TRUE ;
	if (fep == NULL) return SR_FAULT ;
	if (svc == NULL) return SR_FAULT ;
	f = f && (svc[0] == fep->svc[0]) ;
	f = f && (strcmp(svc,fep->svc) == 0) ;
	return f ;
}
/* end subroutine (filer_havesvc) */


