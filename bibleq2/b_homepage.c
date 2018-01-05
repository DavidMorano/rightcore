/* b_homepage (HOMEPAGE) */

/* this is a generic "main" module */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_DEBUGMOUT	0		/* debug memory-allocations */
#define	CF_PROCARGS	0		/* |procargs()| */
#define	CF_STACKCHECK	1		/* check stacks afterwards */
#define	CF_UCREADE	1		/* use |uc_reade(3uc)| */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */
#define	CF_TMPGROUP	1		/* set group on TMP dir */
#define	CF_MNTCHECK	0		/* perform mount-check */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a simple program (of some sort!).

	This program appends either specified files or the standard input to a
	specified file.

	Synopsis:

	$ homepage [-V]

	Arguments:

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
#include	<unistd.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<estrings.h>
#include	<getbufsize.h>
#include	<intceil.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<userinfo.h>
#include	<vecobj.h>
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
#include	<ugetpw.h>
#include	<spawner.h>
#include	<lfm.h>
#include	<tmtime.h>
#include	<querystring.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_homepage.h"
#include	"defs.h"
#include	"proglog.h"
#include	"cgi.h"
#include	"htm.h"
#include	"svckv.h"
#include	"svcent.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#define	STACKSIZE	(1*1024*1024)
#define	STACKGUARD	(2*1024)

#define	HBUFLEN		120		/* header buffer length */

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	CONFIG		struct config
#define	CONFIG_FILE	struct config_file
#define	CONFIG_MAGIC	0x23FFEEDD

#define	GATHER		struct gather

#define	FILER		struct filer
#define	FILER_DSIZE	(3*1024) ;

#define	PO_OPTION	"option"

#define	SVCLEN		(MAXNAMELEN)
#define	SVCENTLEN	(2*LINEBUFLEN)

#ifndef	ENVBUFLEN
#define	ENVBUFLEN	2048
#endif

#define	MEMPATTERN	0x5A5A5A5A


/* typedefs */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef cchar	cchar ;
#endif

typedef	int (*tworker)(void *) ;


/* external subroutines */

extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfdequote(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	nextfieldterm(cchar *,int,cchar *,cchar **) ;
extern int	nchr(cchar *,int,int) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	cfdecmful(cchar *,int,ulong *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	mkplogid(char *,int,cchar *,int) ;
extern int	mksublogid(char *,int,cchar *,int) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	mktmpfile(char *,mode_t,cchar *) ;
extern int	prmktmpdir(cchar *,char *,cchar *,cchar *,mode_t) ;
extern int	prgetprogpath(cchar *,char *,cchar *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	sperm(IDS *,USTAT *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(cchar **,vecstr *,char *,int,cchar *,int) ;
extern int	rmdirfiles(cchar *,cchar *,int) ;
extern int prsetfname(cchar *,char *,cchar *,int,int,cchar *,cchar *,cchar *) ;
extern int	msleep(int) ;
extern int	tolc(int) ;
extern int	hasnonwhite(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isStrEmpty(cchar *,int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	proginfo_rootname(PROGINFO *) ;

extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;
extern char	*strnrpbrk(cchar *,int,cchar *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct gather {
	vechand		ents ;
	PTM		m ;
	PTC		c ;
	PROGINFO	*pip ;
	cchar		*termtype ;
	int		nout ;
	int		cols ;
} ;

struct filer {
	PROGINFO	*pip ;
	cchar		*a ;
	cchar		*fname ;
	cchar		*svc ;
	cchar		*termtype ;
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
	uint		reqfname:1 ;
	uint		mntfname:1 ;
	uint		msfname:1 ;
	uint		svcs:1 ;
	uint		pidlock:1 ;
	uint		intcache:1 ;
	uint		intspeed:1 ;
	uint		intconf:1 ;
	uint		intsvcs:1 ;
	uint		intwait:1 ;
	uint		intmaint:1 ;
	uint		defpage:1 ;
	uint		defsvcs:1 ;
	uint		quietlock:1 ;
	uint		force:1 ;
	uint		maint:1 ;
	uint		s:1 ;			/* SVCFILE */
	uint		cooks:1 ;
} ;

struct locinfo {
	VECSTR		stores ;
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	init, open ;
	SVCFILE		s ;
	GATHER		g ;
	LFM		pidlock ;
	EXPCOOK		cooks ;
	PROGINFO	*pip ;
	void		*svcs ;
	cchar		*tmpourdname ;		/* work-dir */
	cchar		*basedname ;		/* base-dir (for DB) */
	cchar		*dbfname ;
	cchar		*wfname ;		/* log-welcome file-name */
	cchar		*hfname ;		/* HEAD file-name */
	cchar		*sfname ;		/* SVC file-name */
	cchar		*reqfname ;
	cchar		*mntfname ;
	cchar		*msfname ;
	cchar		*termtype ;
	cchar		*copyright ;
	cchar		*webmaster ;
	gid_t		gid_rootname ;
	int		start ;
	int		nproc ;
	int		cols ;
	int		year ;
	int		intcache ;		/* interval cache */
	int		intspeed ;
	int		intconf ;
	int		intsvcs ;
	int		intwait ;
	int		intmaint ;
	int		to_cache ;
	int		to_lock ;
} ;

struct config_file {
	cchar		*fname ;
	time_t		mtime ;
} ;

struct config {
	uint		magic ;
	PROGINFO	*pip ;
	PARAMOPT	*app ;
	vecobj		files ;
	uint		f_p:1 ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_basedir(LOCINFO *,cchar *,int) ;
static int	locinfo_tmpourdname(LOCINFO *) ;
static int	locinfo_tmpgroup(LOCINFO *,cchar *) ;
static int	locinfo_gidrootname(LOCINFO *,struct passwd *,char *,int) ;
static int	locinfo_defreg(LOCINFO *) ;
static int	locinfo_defdaemon(LOCINFO *) ;
static int	locinfo_dbinfo(LOCINFO *,cchar *,cchar *) ;
static int	locinfo_wfname(LOCINFO *,cchar *) ;
static int	locinfo_sethead(LOCINFO *,cchar *,int) ;
static int	locinfo_svclistbegin(LOCINFO *) ;
static int	locinfo_svclistadds(LOCINFO *,cchar *,int) ;
static int	locinfo_svclistadd(LOCINFO *,cchar *,int) ;
static int	locinfo_svclistend(LOCINFO *) ;
static int	locinfo_svclistget(LOCINFO *,int,cchar **) ;
static int	locinfo_svclistdel(LOCINFO *,int) ;
static int	locinfo_svclistdelall(LOCINFO *) ;
static int	locinfo_svclistcount(LOCINFO *) ;
static int	locinfo_svclistfinal(LOCINFO *) ;
static int	locinfo_copyright(LOCINFO *,cchar *,int) ;
static int	locinfo_defs(LOCINFO *) ;
static int	locinfo_defsvc(LOCINFO *) ;
static int	locinfo_defpfname(LOCINFO *) ;
static int	locinfo_qs(LOCINFO *,cchar *) ;
static int	locinfo_finalize(LOCINFO *) ;
static int	locinfo_svcsbegin(LOCINFO *) ;
static int	locinfo_svcsend(LOCINFO *) ;
static int	locinfo_svcscheck(LOCINFO *,time_t) ;
static int	locinfo_gatherbegin(LOCINFO *) ;
static int	locinfo_gatherbeginall(LOCINFO *) ;
static int	locinfo_gatherbeginsome(LOCINFO *) ;
static int	locinfo_gatherbeginer(LOCINFO *,SVCFILE_ENT *) ;
static int	locinfo_gatherend(LOCINFO *) ;

static int	locinfo_lockbegin(LOCINFO *) ;
static int	locinfo_lockend(LOCINFO *) ;
static int	locinfo_lockbeginone(LOCINFO *,LFM *,cchar *) ;
static int	locinfo_lockcheck(LOCINFO *,LFM_CHECK *) ;

static int	locinfo_cookbegin(LOCINFO *) ;
static int	locinfo_cookend(LOCINFO *) ;
static int	locinfo_cookexp(LOCINFO *,int,char *,int,cchar *,int) ;

static int	config_start(CONFIG *,PROGINFO *,PARAMOPT *,cchar *) ;
static int	config_addfile(CONFIG *,cchar *) ;
static int	config_addfins(CONFIG *) ;
static int	config_check(CONFIG *) ;
static int	config_load(CONFIG *,cchar *) ;
static int	config_havefile(CONFIG *,vecstr *,char *,cchar *,cchar *) ;
static int	config_read(CONFIG *,cchar *) ;
static int	config_reader(CONFIG *,PARAMFILE *) ;
static int	config_finish(CONFIG *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procuserinfo_logid(PROGINFO *) ;

static int	procourconf_begin(PROGINFO *,PARAMOPT *,cchar *) ;
static int	procourconf_end(PROGINFO *) ;

static int	process(PROGINFO *,cchar *,cchar *) ;
static int	procregular(PROGINFO *,cchar *,cchar *) ;
static int	procbackinfo(PROGINFO *) ;
static int	procback(PROGINFO *) ;
static int	procbacks(PROGINFO *) ;
static int	procbackcheck(PROGINFO *) ;
static int	procbackmaint(PROGINFO *) ;
static int	procbacker(PROGINFO *,cchar *,cchar **) ;
static int	procbackenv(PROGINFO *,SPAWNER *) ;
static int	procmntcheck(PROGINFO *) ;
static int	procdaemon(PROGINFO *) ;
static int	procdaemons(PROGINFO *) ;
static int	procdaemoner(PROGINFO *) ;
static int	procdaemoncheck(PROGINFO *) ;

static int	procpidfname(PROGINFO *) ;
static int	procbackdefs(PROGINFO *) ;
static int	procdaemondefs(PROGINFO *) ;
static int	procdaemoninfo(PROGINFO *) ;

#if	CF_PROCARGS
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *,cchar *) ;
#endif

static int	procresp(PROGINFO *,cchar *,cchar *) ;
static int	procexecname(PROGINFO *,char *,int) ;

static int	procdoc(PROGINFO *,cchar *) ;
static int	procdocbody(PROGINFO *,HTM *) ;
static int	procdocbodyhdr(PROGINFO *,HTM *) ;
static int	procdocbodyhdrfile(PROGINFO *,char *,int,cchar *) ;
static int	procdocbodymain(PROGINFO *,HTM *) ;
static int	procdocbodyfooter(PROGINFO *,HTM *) ;
static int	procdocbodyfooterleft(PROGINFO *,HTM *) ;
static int	procdocbodyfooterext(PROGINFO *,HTM *) ;
static int	procdocbodymain_svcs(PROGINFO *,HTM *) ;
static int	procdocbodymain_svcer(PROGINFO *,HTM *,GATHER *,SVCFILE_ENT *) ;
static int	procdocbodymain_svcerhdr(PROGINFO *,HTM *,SVCFILE_ENT *) ;
static int	procdocbodymain_svcerinc(PROGINFO *,HTM *,SVCFILE_ENT *) ;
static int	procdocbodymain_svcerincer(PROGINFO *,HTM *,
			SVCFILE_ENT *,cchar *) ;
static int	procdocbodymain_svcerfile(PROGINFO *,HTM *,GATHER *,
			SVCFILE_ENT *) ;
static int	procdocbodymain_svcerfiler(PROGINFO *,HTM *,GATHER *,
			SVCFILE_ENT *,int) ;
static int	proclockcheck(PROGINFO *) ;
static int	proclockprint(PROGINFO *,cchar *,LFM_CHECK *) ;
static int	proclocklog(PROGINFO *,cchar *,LFM_CHECK *,cchar *) ;

static int	procdocbodymain_svcprint(PROGINFO *,HTM *) ;
static int	prochdrs(PROGINFO *,CGI *,int) ;

static int	procpage_begin(PROGINFO *,char *) ;
static int	procpage_end(PROGINFO *,char *) ;
static int	procpage_make(PROGINFO *,char *) ;

static int	mkourname(PROGINFO *,char *,cchar *,cchar *,int) ;
static int	mkincfname(PROGINFO *,char *,cchar *,int) ;

static int gather_start(GATHER *,PROGINFO *,cchar *,int) ;
static int gather_finish(GATHER *) ;
static int gather_file(GATHER *,cchar *,int,cchar *,int) ;
static int gather_entfins(GATHER *) ;
static int gather_getlines(GATHER *,cchar *) ;
static int gather_getbuf(GATHER *,cchar *,cchar **) ;

static int filer_start(FILER *,PROGINFO *,cchar *,int,cchar *,cchar *,int) ;
static int filer_worker(FILER *) ;
static int filer_workread(FILER *,int) ;
static int filer_workreadreg(FILER *,int) ;
static int filer_workreadterm(FILER *,int) ;
static int filer_workreadtermline(FILER *,TERMOUT *,char *,cchar *,int) ;
static int filer_getlines(FILER *) ;
static int filer_getbuf(FILER *,cchar **) ;
static int filer_havesvc(FILER *,cchar *) ;
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
	"daemon",
	"svcs",
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
	argopt_daemon,
	argopt_svcs,
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
	{ 0, 0 }
} ;

static const char	*akonames[] = {
	"print",
	"add",
	"inc",
	"base",
	"cf",
	"lf",
	"wf",
	"log",
	"head",
	"svcs",
	"welcome",
	"daemon",
	"pidfile",
	"mntfile",
	"msfile",
	"to",
	"intrun",
	"intidle",
	"intpoll",
	"intconf",
	"intsvcs",
	"intcache",
	"intwait",
	"intmaint",
	"quiet",
	"quietlock",
	"force",
	"maint",
	NULL
} ;

enum akonames {
	akoname_print,
	akoname_add,
	akoname_inc,
	akoname_base,
	akoname_cf,
	akoname_lf,
	akoname_wf,
	akoname_log,
	akoname_head,
	akoname_svcs,
	akoname_welcome,
	akoname_daemon,
	akoname_pidfile,
	akoname_mntfile,
	akoname_msfile,
	akoname_to,
	akoname_intrun,
	akoname_intidle,
	akoname_intpoll,
	akoname_intconf,
	akoname_intsvcs,
	akoname_intcache,
	akoname_intwait,
	akoname_intmaint,
	akoname_quiet,
	akoname_quietlock,
	akoname_force,
	akoname_maint,
	akoname_overlast
} ;

static const char	*csched[] = {
	"%p/etc/%n/%n.%f",
	"%p/etc/%n/%f",
	"%p/etc/%n.%f",
	NULL
} ;

static const char	*cparams[] = {
	"workdir",
	"basedir",
	"basedb",
	"logfile",
	"logsize",
	"head",
	"svcs",
	"copyright",
	"webmaster",
	"intrun",
	"intidle",
	"intpoll",
	"intconf",
	"intlock",
	"intmark",
	"intcache",
	"intwait",
	"intmaint",
	NULL
} ;

enum cparams {
	cparam_workdir,
	cparam_basedir,
	cparam_basedb,
	cparam_logfile,
	cparam_logsize,
	cparam_head,
	cparam_svcs,
	cparam_copyright,
	cparam_webmaster,
	cparam_intrun,
	cparam_intidle,
	cparam_intpoll,
	cparam_intconf,
	cparam_intlock,
	cparam_intmark,
	cparam_intcache,
	cparam_intwait,
	cparam_intmaint,
	cparam_overlast
} ;

static const char	*isexecs[] = {
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

static const int	sigignores[] = {
	SIGPIPE,
	SIGPOLL,
	SIGHUP,
	0
} ;


/* exported subroutines */


int b_homepage(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_homepage) */


int p_homepage(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_homepage) */


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
	int		ai, ai_max, ai_pos, ai_continue ;
	int		rs, rs1 ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*cfname = NULL ;
	cchar		*wfname = NULL ;
	cchar		*dbfname = NULL ;
	cchar		*qs = NULL ;
	cchar		*un = NULL ;
	cchar		*cp ;


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
	rs = proginfo_setbanner(pip,cp) ;

/* early things to initialize */

	pip->verboselevel = 1 ;
	pip->daytime = time(NULL) ;
	pip->to_open = -1 ;
	pip->f.logprog = OPT_LOGPROG ;

	pip->lip = lip ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* process program arguments */

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
	                            pip->lfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->lfname = argp ;
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
	                    if ((rs >= 0) && (cp != NULL)) {
	                        lip->final.dbfname = TRUE ;
	                        lip->have.dbfname = TRUE ;
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

/* daemon mode */
	                case argopt_daemon:
	                    pip->have.daemon = TRUE ;
	                    pip->final.daemon = TRUE ;
	                    pip->f.daemon = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = cfdecti(avp,avl,&v) ;
	                            pip->intrun = v ;
	                        }
	                    }
	                    break ;

/* services */
	                case argopt_svcs:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = locinfo_svclistadds(lip,argp,argl) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
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

/* daemon mode */
	                    case 'd':
	                        pip->final.background = TRUE ;
	                        pip->f.background = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                cchar	ch = MKCHAR(*avp) ;
	                                if (isdigitlatin(ch)) {
	                                    pip->final.intrun = TRUE ;
	                                    pip->have.intrun = TRUE ;
	                                    rs = cfdecti(avp,avl,&v) ;
	                                    pip->intrun = v ;
	                                } else if (tolc(ch) == 'i') {
	                                    pip->intrun = INT_MAX ;
	                                } else
	                                    rs = SR_INVALID ;
	                            }
	                        }
	                        break ;

	                    case 'f':
	                        lip->final.force = TRUE ;
	                        lip->have.force = TRUE ;
	                        lip->f.force = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.force = (rs > 0) ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_homepage: debuglevel=%u\n",pip->debuglevel) ;
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

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (rs >= 0) {
	    if ((rs = locinfo_dbinfo(lip,NULL,dbfname)) >= 0) {
	        if ((rs = locinfo_finalize(lip)) >= 0) {
	            if ((rs = locinfo_qs(lip,qs)) >= 0) {
	                if ((rs = locinfo_finalize(lip)) >= 0) {
	                    if ((rs = procopts(pip,&akopts)) >= 0) {
	                        rs = locinfo_finalize(lip) ;
	                    }
	                }
	            }
	        }
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_homepage: 2 rs=%d\n",rs) ;
#endif

	if (cfname == NULL) cfname = getourenv(envv,VARCFNAME) ;

	if (pip->lfname != NULL) pip->lfname = getourenv(envv,VARLFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (lip->tmpourdname == NULL) {
	    lip->tmpourdname = getourenv(envv,VARWORKDNAME) ;
	}

	if (wfname == NULL) wfname = getourenv(envv,VARWFNAME) ;
	if (wfname == NULL) wfname = LWFNAME ;

	if (rs >= 0) {
	    rs = locinfo_wfname(lip,wfname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_homepage: 3 rs=%d\n",rs) ;
#endif

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: cf=%s\n" ;
	    shio_printf(pip->efp,fmt,pn,cfname) ;
	    fmt = "%s: lf=%s\n" ;
	    shio_printf(pip->efp,fmt,pn,pip->lfname) ;
	    fmt = "%s: wf=%s\n" ;
	    shio_printf(pip->efp,fmt,pn,lip->wfname) ;
	    fmt = "%s: force=%u\n" ;
	    shio_printf(pip->efp,fmt,pn,lip->f.force) ;
	    fmt = "%s: maint=%u\n" ;
	    shio_printf(pip->efp,fmt,pn,lip->f.maint) ;
	}

/* go */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;
	ainfo.ai_continue = ai_continue ;

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,un)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	            if (cfname != NULL) {
	                if (pip->euid != pip->uid) u_seteuid(pip->uid) ;
	                if (pip->egid != pip->gid) u_setegid(pip->gid) ;
	            }
	            if ((rs = procourconf_begin(pip,&aparams,cfname)) >= 0) {
	                if ((rs = proglog_begin(pip,&u)) >= 0) {
	                    if ((rs = proguserlist_begin(pip)) >= 0) {
	                        cchar	*ofn = ofname ;
	                        cchar	*afn = afname ;
	                        if ((rs = process(pip,ofn,afn)) >= 0) {
	                            if (pip->debuglevel > 0) {
	                                cchar	*pn = pip->progname ;
	                                cchar	*fmt = "%s: updates=%u\n" ;
	                                shio_printf(pip->efp,fmt,pn,rs) ;
	                            }
	                        }
	                        rs1 = proguserlist_end(pip) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (proguserlist) */
	                    rs1 = proglog_end(pip) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (progloger) */
	                rs1 = procourconf_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (procourconf) */
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
	    }
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

	if ((rs < 0) && (! pip->f.quiet)) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if (rs == SR_TIMEDOUT) {
	        fmt = "%s: timed-out (%d)\n" ;
	    } else if (rs == SR_EXIT) {
	        fmt = "%s: program terminated (%d)\n" ;
	    } else {
	        fmt = "%s: could not process (%d)\n" ;
	    }
	    shio_printf(pip->efp,fmt,pn,rs) ;
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
	        cchar		*ids = "main" ;
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
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	int		rs ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [-of <outfile>] [-d[=<runint>]]\n" ;
	rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-af <afile>] [-svcs <svc(s)>]\n" ;
	rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
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
	        cchar	*pr = pip->progname ;
	        cchar	*sn = pip->searchname ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {
	                int	v = 0 ;

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case akoname_base:
	                    if (! lip->final.basedname) {
	                        if (vl > 0) {
	                            lip->final.basedname = TRUE ;
	                            lip->have.basedname = TRUE ;
	                            rs = locinfo_basedir(lip,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_cf:
	                    if ((pip->cfname == NULL) && (vl > 0)) {
	                        cchar	*lc = LOGCNAME ;
	                        char	tbuf[MAXPATHLEN+1] ;
	                        rs = prsetfname(pr,tbuf,vp,vl,TRUE,lc,sn,"") ;
	                        if (rs >= 0) {
	                            cchar	**vpp = &pip->cfname ;
	                            rs = proginfo_setentry(pip,vpp,tbuf,rs) ;
	                        }
	                    }
	                    break ;
	                case akoname_lf:
	                    if ((pip->lfname == NULL) && (vl > 0)) {
	                        cchar	*lc = LOGCNAME ;
	                        char	tbuf[MAXPATHLEN+1] ;
	                        rs = prsetfname(pr,tbuf,vp,vl,TRUE,lc,sn,"") ;
	                        if (rs >= 0) {
	                            cchar	**vpp = &pip->lfname ;
	                            rs = proginfo_setentry(pip,vpp,tbuf,rs) ;
	                        }
	                    }
	                    break ;
	                case akoname_wf:
	                    if ((lip->wfname == NULL) && (vl > 0)) {
	                        cchar	*lc = LOGCNAME ;
	                        char	tbuf[MAXPATHLEN+1] ;
	                        rs = prsetfname(pr,tbuf,vp,vl,TRUE,lc,sn,"") ;
	                        if (rs >= 0) {
	                            cchar	**vpp = &lip->wfname ;
	                            rs = locinfo_setentry(lip,vpp,tbuf,rs) ;
	                        }
	                    }
	                    break ;
	                case akoname_log:
	                    if (! pip->final.logprog) {
	                        pip->f.logprog = TRUE ;
	                        if (vl > 0) {
	                            pip->final.logprog = TRUE ;
	                            pip->have.logprog = TRUE ;
	                            rs = optbool(vp,vl) ;
	                            pip->f.logprog = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_quiet:
	                    if (! pip->final.quiet) {
	                        pip->f.quiet = TRUE ;
	                        if (vl > 0) {
	                            pip->final.quiet = TRUE ;
	                            pip->have.quiet = TRUE ;
	                            rs = optbool(vp,vl) ;
	                            pip->f.quiet = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_quietlock:
	                    if (! lip->final.quietlock) {
	                        lip->f.quietlock = TRUE ;
	                        if (vl > 0) {
	                            lip->final.quietlock = TRUE ;
	                            lip->have.quietlock = TRUE ;
	                            rs = optbool(vp,vl) ;
	                            lip->f.quietlock = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_force:
	                    if (! lip->final.force) {
	                        lip->f.force = TRUE ;
	                        if (vl > 0) {
	                            lip->final.force = TRUE ;
	                            lip->have.force = TRUE ;
	                            rs = optbool(vp,vl) ;
	                            lip->f.force = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_maint:
	                    if (! lip->final.maint) {
	                        lip->have.maint = TRUE ;
	                        lip->f.maint = TRUE ;
	                        if (vl > 0) {
	                            lip->final.maint = TRUE ;
	                            rs = optbool(vp,vl) ;
	                            lip->f.maint = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_head:
	                    if (! lip->final.hfname) {
	                        if (vl > 0) {
	                            lip->final.hfname = TRUE ;
	                            lip->have.hfname = TRUE ;
	                            rs = locinfo_sethead(lip,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_svcs:
	                    if (! lip->final.svcs) {
	                        if (vl > 0) {
	                            lip->have.svcs = TRUE ;
	                            rs = locinfo_svclistadds(lip,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_daemon:
	                    if (! pip->final.daemon) {
	                        c += 1 ;
	                        pip->final.daemon = TRUE ;
	                        pip->have.daemon = TRUE ;
	                        pip->f.daemon = TRUE ;
	                        if (vl > 0) {
	                            rs = cfdecti(vp,vl,&v) ;
	                            pip->intrun = v ;
	                            if (v == 0) {
	                                pip->f.daemon = FALSE ;
	                            }
	                        }
	                    } /* end if */
	                    break ;
	                case akoname_pidfile:
	                    if (! pip->final.pidfname) {
	                        if (vl > 0) {
	                            cchar	**vpp = &pip->pidfname ;
	                            pip->final.pidfname = TRUE ;
	                            pip->have.pidfname = TRUE ;
	                            rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_to:
	                case akoname_intrun:
	                case akoname_intidle:
	                case akoname_intpoll:
	                case akoname_intconf:
	                case akoname_intsvcs:
	                case akoname_intcache:
	                case akoname_intwait:
	                case akoname_intmaint:
	                    {
	                        if (vl > 0) {
	                            rs = cfdecti(vp,vl,&v) ;
	                        }
	                        if ((rs >= 0) && (vl > 0)) {
	                            switch(oi) {
	                            case akoname_to:
	                                if (! pip->final.to) {
	                                    c += 1 ;
	                                    pip->final.to = TRUE ;
	                                    pip->have.to = TRUE ;
	                                    pip->to = v ;
	                                }
	                                break ;
	                            case akoname_intrun:
	                                if (! pip->final.intrun) {
	                                    c += 1 ;
	                                    pip->final.intrun = TRUE ;
	                                    pip->have.intrun = TRUE ;
	                                    pip->intrun = v ;
	                                }
	                                break ;
	                            case akoname_intidle:
	                                if (! pip->final.intidle) {
	                                    c += 1 ;
	                                    pip->final.intidle = TRUE ;
	                                    pip->have.intidle = TRUE ;
	                                    pip->intidle = v ;
	                                }
	                                break ;
	                            case akoname_intpoll:
	                                if (! pip->final.intpoll) {
	                                    c += 1 ;
	                                    pip->final.intpoll = TRUE ;
	                                    pip->have.intpoll = TRUE ;
	                                    pip->intpoll = v ;
	                                }
	                                break ;
	                            case akoname_intconf:
	                                if (! lip->final.intconf) {
	                                    c += 1 ;
	                                    lip->final.intconf = TRUE ;
	                                    lip->have.intconf = TRUE ;
	                                    lip->intconf = v ;
	                                }
	                                break ;
	                            case akoname_intsvcs:
	                                if (! lip->final.intsvcs) {
	                                    c += 1 ;
	                                    lip->final.intsvcs = TRUE ;
	                                    lip->have.intsvcs = TRUE ;
	                                    lip->intsvcs = v ;
	                                }
	                                break ;
	                            case akoname_intcache:
	                                if (! lip->final.intcache) {
	                                    c += 1 ;
	                                    lip->final.intcache = TRUE ;
	                                    lip->have.intcache = TRUE ;
	                                    lip->intcache = v ;
	                                }
	                                break ;
	                            case akoname_intwait:
	                                if (! lip->final.intwait) {
	                                    c += 1 ;
	                                    lip->final.intwait = TRUE ;
	                                    lip->have.intwait = TRUE ;
	                                    lip->intwait = v ;
	                                }
	                                break ;
	                            case akoname_intmaint:
	                                if (! lip->final.intmaint) {
	                                    c += 1 ;
	                                    lip->final.intmaint = TRUE ;
	                                    lip->have.intmaint = TRUE ;
	                                    lip->intmaint = v ;
	                                }
	                                break ;
	                            } /* end switch */
	                        } /* end if (have) */
	                    } /* end block */
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } else {
	                cchar	*pn = pip->progname ;
	                cchar	*fmt = "%s: invalid key=>%t<\n" ;
	                shio_printf(pip->efp,fmt,pn,kp,kl) ;
	                rs = SR_INVALID ;
	            }

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        rs1 = keyopt_curend(kop,&kcur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */
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
	    cchar	*nn = pip->nodename ;
	    cchar	*dn = pip->domainname ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        cchar	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}

	if (rs >= 0) {
	    rs = procuserinfo_logid(pip) ;
	} /* end if (ok) */

	if (rs >= 0) {
	    LOCINFO	*lip = pip->lip ;
	    TMTIME	t ;
	    if ((rs = tmtime_gmtime(&t,pip->daytime)) >= 0) {
	        lip->year = (t.year + TM_YEAR_BASE) ;
	        rs = locinfo_cookbegin(lip) ;
	    }
	}

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: username=%s\n",
	        pip->progname,pip->username) ;
	}

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

	{
	    LOCINFO	*lip = pip->lip ;
	    rs1 = locinfo_cookend(lip) ;
	    if (rs >= 0) rs = rs1 ;
	}

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


static int procourconf_begin(PROGINFO *pip,PARAMOPT *app,cchar *cfname)
{
	const int	csize = sizeof(CONFIG) ;
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
	        if (lip->copyright == NULL) {
	            cchar	*c = COPYRIGHT ;
	            if ((rs = locinfo_copyright(lip,c,-1)) >= 0) {
	                if (lip->webmaster == NULL) {
	                    lip->webmaster = WEBMASTER ;
	                }
	            }
	        }
	        if (rs < 0) {
	            config_finish(csp) ;
	        }
	    } /* end if (config_start) */
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


static int procourconf_end(PROGINFO *pip)
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


static int process(PROGINFO *pip,cchar *ofn,cchar *afn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = ids_load(&pip->id)) >= 0) {
	    if ((rs = locinfo_tmpourdname(lip)) >= 0) {
	        if (pip->f.background || pip->f.daemon) {
	            if ((rs = procbackdefs(pip)) >= 0) {
	                if ((rs = procpidfname(pip)) >= 0) {
	                    if ((rs = procbackinfo(pip)) >= 0) {
	                        if (pip->f.background) {
	                            rs = procback(pip) ;
	                        } else if (pip->f.daemon) {
	                            rs = procdaemon(pip) ;
	                            c = rs ;
	                        }
	                    } /* end if (procbackinfo) */
	                } /* end if (procpidfname) */
	            } /* end if (procbackdefs) */
	        } else {
	            rs = procregular(pip,ofn,afn) ;
	            c = rs ;
	        }
	    } /* end if (locinfo_tmpourdname) */
	    rs1 = ids_release(&pip->id) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ids) */

#ifdef	COMMENT
	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp, "%s: updates=%u\n",pip->progname,c) ;
	} /* end if */
#endif /* COMMENT */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("msumain/process: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (process) */


static int procbackdefs(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if (rs >= 0){
	    rs = locinfo_defpfname(lip) ;
	}

	return rs ;
}
/* end subroutine (procbackdefs) */


static int procdaemondefs(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if (rs >= 0) {
	    rs = locinfo_defpfname(lip) ;
	}

	if (lip->reqfname == NULL) {
	    cchar	**envv = pip->envv ;
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARREQFNAME)) != NULL) {
	        lip->final.reqfname = TRUE ;
	        lip->have.reqfname = TRUE ;
	        lip->reqfname = cp ;
	    }
	}

	if (pip->intidle == 0) pip->intidle = TO_IDLE ;

	if (pip->intpoll == 0) pip->intpoll = TO_POLL ;

	if (pip->intmark == 0) pip->intmark = TO_MARK ;

	if (pip->intlock == 0) pip->intlock = TO_LOCK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("msumain: daemon=%u logging=%u\n",
	        pip->f.daemon,pip->have.logprog) ;
#endif
#if	CF_DEBUGS
	debugprintf("msumain: daemon=%u logging=%u\n",
	    pip->f.daemon,pip->have.logprog) ;
#endif

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if (pip->intpoll >= 0) {
	        fmt = "%s: mspoll=%u\n" ;
	    } else {
	        fmt = "%s: mspoll=Inf\n" ;
	    }
	    shio_printf(pip->efp,fmt,pn,pip->intpoll) ;
	    shio_printf(pip->efp,"%s: intspeed=%u\n",pn,lip->intspeed) ;
	}

	return rs ;
}
/* end subroutine (procdaemondefs) */


static int procpidfname(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		pfl = -1 ;
	int		f_changed = FALSE ;
	cchar		*sn = pip->searchname ;
	cchar		*pfp = pip->pidfname ;
	char		rundname[MAXPATHLEN+1] ;
	char		cname[MAXNAMELEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if ((pfp == NULL) || (pfp[0] == '+')) {

	    f_changed = TRUE ;
	    if ((rs = mkpath2(rundname,pip->pr,RUNDNAME)) >= 0) {
	        USTAT	sb ;
	        if ((rs = uc_stat(rundname,&sb)) >= 0) {
	            if (! S_ISDIR(sb.st_mode)) {
	                rs = SR_NOTDIR ;
	            }
	        } else if (rs == SR_NOENT) {
	            rs = mkdirs(rundname,0777) ;
	        }
	    }

	    if (rs >= 0) {
	        if ((rs = snsds(cname,MAXNAMELEN,pip->nodename,sn)) >= 0) {
	            pfp = tmpfname ;
	            rs = mkpath2(tmpfname,rundname,cname) ;
	            pfl = rs ;
	        }
	    }

	} /* end if (creating a default PID file-name) */

	if ((rs >= 0) && (pfp != NULL) && (pfp[0] == '-')) {
	    pfp = NULL ;
	    pip->have.pidfname = FALSE ;
	    pip->f.pidfname = FALSE ;
	    f_changed = FALSE ;
	}

	if ((rs >= 0) && (pfp != NULL) && f_changed) {
	    cchar	**vpp = &pip->pidfname ;
	    rs = proginfo_setentry(pip,vpp,pfp,pfl) ;
	}

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (procpidfname) */


static int procbackinfo(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    cchar	*pn = pip->progname ;
	    cchar	*mntfname = lip->mntfname ;

	    if (pip->pidfname != NULL) {
	        shio_printf(pip->efp,"%s: pidfile=%s\n",pn,pip->pidfname) ;
	    }

	    if (mntfname != NULL) {
	        shio_printf(pip->efp,"%s: mntfile=%s\n",pn,mntfname) ;
	    }

	    shio_flush(pip->efp) ;
	} /* end if (debugging information) */

	return rs ;
}
/* end subroutine (procbackinfo) */


static int procdaemoninfo(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    cchar	*pn = pip->progname ;
	    if (pip->f.daemon) {
	        shio_printf(pip->efp,"%s: pidfile=%u\n",pn,pip->pidfname) ;
	        shio_printf(pip->efp,"%s: intrun=%u\n",pn,pip->intrun) ;
	        shio_printf(pip->efp,"%s: intidle=%u\n",pn,pip->intidle) ;
	        shio_printf(pip->efp,"%s: intpoll=%u\n",pn,pip->intpoll) ;
	        shio_printf(pip->efp,"%s: intmark=%u\n",pn,pip->intmark) ;
	        shio_printf(pip->efp,"%s: intwait=%u\n",pn,lip->intwait) ;
	    }
	    shio_printf(pip->efp,"%s: intcache=%u\n",pn,lip->intcache) ;
	    shio_flush(pip->efp) ;
	} /* end if (debugging information) */

	if (pip->f.daemon) {
	    proglog_printf(pip,"pidfile=%d",pip->pidfname) ;
	    proglog_printf(pip,"intrun=%d",pip->intrun) ;
	    proglog_printf(pip,"intidle=%d",pip->intidle) ;
	    proglog_printf(pip,"intpoll=%d",pip->intpoll) ;
	    proglog_printf(pip,"intlock=%d",pip->intlock) ;
	    proglog_printf(pip,"intwait=%d",lip->intwait) ;
	}
	proglog_printf(pip,"intcache=%d",lip->intcache) ;
	proglog_flush(pip) ;

	return rs ;
}
/* end subroutine (procdaemoninfo) */


#if	CF_PROCARGS
static int procargs(pip,aip,bop,ofn,ifn,afn)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
cchar		*ofn ;
cchar		*ifn ;
cchar		*afn ;
{
	SHIO		ofile, *ofp = &ofile ;
	const int	to_open = pip->to_open ;
	int		rs ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procargs: ofn=%s\n",ofn) ;
#endif

	if ((rs = shio_opene(ofp,ofn,"wc",0666,to_open)) >= 0) {
	    LOCINFO	*lip = pip->lip ;
	    int		pan = 0 ;
	    int		cl ;
	    cchar	*cp ;

	    if (lip->f.trunc) lip->start = 0L ;

	    if ((rs = shio_isseekable(ofp)) > 0) {
	        if (lip->start >= 0) {
	            offset_t	o = lip->start ;
	            rs = shio_seek(ofp,o,SEEK_SET) ;
	        } else {
	            rs = shio_seek(ofp,0L,SEEK_END) ;
	        }
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
	        cchar	**argv = aip->argv ;
	        for (ai = aip->ai_continue ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
	                if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = procfile(pip,ofp,cp) ;
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
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	                int	len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                cp = lbuf ;
	                cl = len ;

	                if (cl > 0) {
	                    pan += 1 ;
	                    rs = procfile(pip,ofp,cp) ;
	                    wlen += rs ;
	                }

	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            shio_close(afp) ;
	        } else {
	            fmt = "%s: inaccessible argument-list (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	            shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        } /* end if */

	    } /* end if (procesing file argument file list) */

	    if ((rs >= 0) && (pan == 0)) {

	        cp = (ifn != NULL) ? ifn : "-" ;
	        pan += 1 ;
	        rs = procfile(pip,ofp,cp) ;
	        wlen += rs ;

	    } /* end if (standard-input) */

	    lip->nproc = pan ;
	    shio_close(ofp) ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procargs: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */
#endif /* CF_PROCARGS */


static int procback(PROGINFO *pip)
{
	int		rs ;

	if (pip->open.logprog) {
	    proglog_printf(pip,"mode=background") ;
	    proglog_flush(pip) ;
	}

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: mode=background\n",pip->progname) ;
	    shio_flush(pip->efp) ;
	}

	if ((rs = procbackcheck(pip)) >= 0) {
	    if ((rs = procbackmaint(pip)) >= 0) {
	        rs = procbacks(pip) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_homepage/procback: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procback) */


static int procbackmaint(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	const int	to = TO_TMPFILES ;
	int		rs = SR_OK ;
	if (lip->f.maint || (lip->intmaint > 0)) {
	    cchar	*dir = lip->tmpourdname ;
	    cchar	*prefix = "homepage" ;
	    if ((rs = rmdirfiles(dir,prefix,to)) > 0) {
	        if (pip->debuglevel > 0) {
	            cchar	*pn = pip->progname ;
	            cchar	*fmt ;
	            fmt = "%s: files-cleaned=%u\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	        }
	        proglog_printf(pip,"files-cleaned=%u",rs) ;
	    }
	} /* end if (maintenance) */
	return rs ;
}
/* end subroutine (procbackmaint) */


static int procbackcheck(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_homepage/procbackcheck: ent\n") ;
#endif

	if ((rs = locinfo_lockbegin(lip)) >= 0) {
	    {
	        rs = procmntcheck(pip) ;
	    }
	    rs1 = locinfo_lockend(lip) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    const int	f = ((pip->debuglevel > 0) || (! lip->f.quietlock)) ;
	    if ((! pip->f.quiet) && f) {
	        cchar	*lfn = pip->pidfname ;
	        fmt = "%s: could not acquire PID lock (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        fmt = "%s: lock=%s\n" ;
	        shio_printf(pip->efp,fmt,pn,lfn) ;
	    }
	    proglog_printf(pip,"could not capture PID lock (%d)",rs) ;
	}

	return rs ;
}
/* end subroutine (procbackcheck) */


static int procmntcheck(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	if (lip == NULL) return SR_FAULT ;
#if	CF_MNTCHECK
	if (lip->mntfname != NULL) {
	    USTAT	usb ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if ((rs = u_stat(lip->mntfname,&usb)) >= 0) {
	        if (S_ISREG(usb.st_mode)) {
	            rs = sperm(&pip->id,&usb,W_OK) ;
	        } else {
	            rs = SR_BUSY ;
	        }
	        if (rs < 0) {
	            if (! pip->f.quiet) {
	                fmt = "%s: inaccessible mount point (%d)\n" ;
	                shio_printf(pip->efp,fmt,pn,rs) ;
	            }
	        }
	    }
	} /* end if (mntfname) */
#endif /* CF_MNTCHECK */
	return rs ;
}
/* end subroutine (procmntcheck) */


static int procbacks(PROGINFO *pip)
{
	const int	elen = MAXPATHLEN ;
	int		rs ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	char		ebuf[MAXPATHLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_homepage/procbacks: ent\n") ;
#endif

	if ((rs = procexecname(pip,ebuf,elen)) >= 0) {
	    int		el = rs ;
	    cchar	*pf = ebuf ;
	    cchar	*tp ;
	    char	pbuf[MAXPATHLEN+1] ;

	    if (pip->debuglevel > 0) {
	        fmt = "%s: execname=%t\n" ;
	        shio_printf(pip->efp,fmt,pn,ebuf,el) ;
	    }

	    if ((tp = strnrpbrk(ebuf,el,"/.")) != NULL) {
	        if (tp[0] == '.') {
	            el = (tp-ebuf) ;
	            ebuf[el] = '\0' ;
	        }
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_homepage/procbacks: ebuf=%t\n",ebuf,el) ;
#endif

	    if ((rs = prgetprogpath(pip->pr,pbuf,ebuf,el)) > 0) {
	        pf = pbuf ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("b_homepage/procbacks: mid rs=%d\n",rs) ;
	        debugprintf("b_homepage/procbacks: mid pf=%s\n",pf) ;
	    }
#endif

	    if (rs >= 0) {
	        int	i = 0 ;
	        cchar	*av[5] ;
	        char	dbuf[10+1] ;
	        if (pip->debuglevel > 0) {
	            fmt = "%s: pf=%s\n" ;
	            shio_printf(pip->efp,fmt,pn,pf) ;
	        }
	        av[i++] = pip->progname ;
	        av[i++] = "-daemon" ;
	        if (pip->debuglevel > 0) {
	            bufprintf(dbuf,10,"-D=%u",pip->debuglevel) ;
	            av[i++] = dbuf ;
	        }
	        av[i++] = NULL ;
	        rs = procbacker(pip,pf,av) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("b_homepage/procbacks: procbacker() rs=%d\n",rs) ;
#endif
	    } /* end if (ok) */
	} /* end if (procexecname) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_homepage/procbacks: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procbacks) */


static int procbacker(PROGINFO *pip,cchar *pf,cchar **av)
{
	SPAWNER		s ;
	int		rs ;
	int		rs1 ;
	int		pid = 0 ;
	cchar		**ev = pip->envv ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procbacker: ent\n") ;
#endif
	if ((rs = spawner_start(&s,pf,av,ev)) >= 0) {
	    if ((rs = procbackenv(pip,&s)) >= 0) {
	        int	i ;
	        for (i = 0 ; sigignores[i] > 0 ; i += 1) {
	            spawner_sigignore(&s,sigignores[i]) ;
	        }
	        spawner_setsid(&s) ;
	        if (pip->uid != pip->euid) {
	            spawner_seteuid(&s,pip->uid) ;
	        }
	        if (pip->gid != pip->egid) {
	            spawner_setegid(&s,pip->gid) ;
	        }
	        for (i = 0 ; i < 2 ; i += 1) {
	            spawner_fdclose(&s,i) ;
	        }
	        if ((rs = spawner_run(&s)) >= 0) {
	            cchar	*fmt ;
	            pid = rs ;
	            if (pip->open.logprog) {
	                fmt = "backgrounded (%u)" ;
	                proglog_printf(pip,fmt,pid) ;
	            }
	        }
	    } /* end if (procbackenv) */
	    rs1 = spawner_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (spawner) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procbacker: ret rs=%d pid=%u\n",rs,pid) ;
#endif
	return (rs >= 0) ? pid : rs ;
}
/* end subroutine (procbacker) */


static int procbackenv(PROGINFO *pip,SPAWNER *srp)
{
	LOCINFO		*lip = pip->lip ;
	BUFFER		b ;
	int		rs ;
	int		rs1 ;
	cchar		*varopts = VAROPTS ;
	if ((rs = buffer_start(&b,ENVBUFLEN)) >= 0) {
	    cchar	*np ;
	    int		v ;
	    int		i ;
	    int		c = 0 ;

	    for (i = 0 ; i < 9 ; i += 1) {
	        np = NULL ;
	        v = 0 ;
	        switch (i) {
	        case 0:
	            v = pip->intrun ;
	            if (v > 0) np = "intrun" ;
	            break ;
	        case 1:
	            v = pip->intidle ;
	            if (v > 0) np = "intidle" ;
	            break ;
	        case 2:
	            v = pip->intpoll ;
	            if (v > 0) np = "intpoll" ;
	            break ;
	        case 3:
	            v = lip->intconf ;
	            if (v > 0) np = "intconf" ;
	            break ;
	        case 4:
	            v = lip->intsvcs ;
	            if (v > 0) np = "intsvcs" ;
	            break ;
	        case 5:
	            v = lip->intspeed ;
	            if (v > 0) np = "intspeed" ;
	            break ;
	        case 6:
	            v = (pip->f.reuseaddr&1) ;
	            if (v > 0) np = "resueaddr" ;
	            break ;
	        case 7:
	            v = (pip->f.quiet&1) ;
	            if (v > 0) np = "quiet" ;
	            break ;
	        case 8:
	            v = (lip->f.quietlock&1) ;
	            if (v > 0) np = "quietlock" ;
	            break ;
	        } /* end switch */
	        if ((np != NULL) && (v > 0)) {
	            if (c++ > 0) {
	                buffer_char(&b,CH_COMMA) ;
	            }
	            rs = buffer_printf(&b,"%s=%d",np,v) ;
	        }
	        if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
	        cchar	*vp ;
	        for (i = 0 ; i < 3 ; i += 1) {
	            np = NULL ;
	            switch (i) {
	            case 0:
	                if (pip->pidfname != NULL) {
	                    np = "pidfile" ;
	                    vp = pip->pidfname ;
	                }
	                break ;
	            case 1:
	                if (lip->mntfname != NULL) {
	                    np = "mntfile" ;
	                    vp = lip->mntfname ;
	                }
	                break ;
	            case 2:
	                if (lip->msfname != NULL) {
	                    np = "msfile" ;
	                    vp = lip->msfname ;
	                }
	                break ;
	            } /* end switch */
	            if (np != NULL) {
	                if (c++ > 0) {
	                    buffer_char(&b,CH_COMMA) ;
	                }
	                rs = buffer_printf(&b,"%s=%s",np,vp) ;
	            } /* end if (non-null) */
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

	    if ((rs >= 0) && (c > 0)) {
	        if ((rs = buffer_get(&b,&np)) >= 0) {
	            rs = spawner_envset(srp,varopts,np,rs) ;
	        }
	    }

	    rs1 = buffer_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (buffer) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procbackenv: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procbackenv) */


static int procdaemon(PROGINFO *pip)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procdaemon: ent\n") ;
#endif

	if (pip->open.logprog) {
	    proglog_printf(pip,"mode=daemon pid=%d",pip->pid) ;
	    proglog_flush(pip) ;
	}
	if (pip->debuglevel > 0) {
	    pid_t	pid = pip->pid ;
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp,"%s: mode=daemon pid=%d\n",pn,pid) ;
	}

	if ((rs = procdaemondefs(pip)) >= 0) {
	    if ((rs = procdaemoncheck(pip)) >= 0) {
	        LOCINFO	*lip = pip->lip ;
	        if ((rs = locinfo_defs(lip)) >= 0) {
	            if ((rs = locinfo_defdaemon(lip)) >= 0) {
	                if ((rs = procdaemoninfo(pip)) >= 0) {
	                    if ((rs = locinfo_lockbegin(lip)) >= 0) {
	                        if ((rs = locinfo_svcsbegin(lip)) >= 0) {
	                            {
	                                rs = procdaemons(pip) ;
	                                c = rs ;
	                            }
	                            rs1 = locinfo_svcsend(lip) ;
	                            if (rs >= 0) rs = rs1 ;
	                        } /* end if (locinfo-svcs) */
	                        rs1 = locinfo_lockend(lip) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (lock) */
	                } /* end if (procdaemoninfo) */
	            } /* end if (locinfo_defdaemon) */
	        } /* end if (locinfo_defs) */
	    } /* end if (procdaemoncheck) */
	} /* end if (procdaemondefs) */

	if ((pip->debuglevel > 0) && (pip->efp != NULL)) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if (rs == SR_TIMEDOUT) {
	        fmt = "%s: daemon run-time expired (%d)\n" ;
	    } else if (rs == SR_EXIT) {
	        fmt = "%s: daemon terminated (%d)\n" ;
	    } else {
	        fmt = "%s: daemon exiting (%d)\n" ;
	    }
	    shio_printf(pip->efp,fmt,pn,rs) ;
	}
	if (pip->open.logprog) {
	    char	tbuf[TIMEBUFLEN+1] ;
	    timestr_logz(pip->daytime,tbuf) ;
	    proglog_printf(pip,"%s exiting (%d)",tbuf,rs) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdaemon) */


static int procdaemons(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	const time_t	ti_start = pip->daytime ;
	time_t		ti_last = pip->daytime ;
	time_t		ti_wait = pip->daytime ;
	time_t		ti_pid = pip->daytime ;
	time_t		ti_logflush = pip->daytime ;
	time_t		ti_config = pip->daytime ;
	time_t		ti_svcs = pip->daytime ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	int		u = 0 ;
	int		nhandle = 0 ;

	if (lip == NULL) return SR_FAULT ;

	while (rs >= 0) {
	    int	f ;
	    if (c++ > 0) {
	        rs = uc_safesleep(pip->intidle) ;
	        pip->daytime = time(NULL) ;
	    }
	    if ((rs >= 0) && ((pip->daytime-ti_last) >= pip->intpoll)) {
	        ti_last = pip->daytime ;
	        u += 1 ;
	        nhandle += 1 ;
	        rs = procdaemoner(pip) ;
	    }
	    if (rs >= 0) rs = lib_sigterm() ;
	    if (rs >= 0) rs = lib_sigintr() ;

	    f = ((pip->daytime - ti_wait) > lip->intwait) ;
	    if ((rs >= 0) && (nhandle || f)) {
	        ti_wait = pip->daytime ;
	        rs1 = SR_OK ;
	        while ((nhandle > 0) || f) {
	            rs1 = u_waitpid(-1,NULL,WNOHANG) ;
	            if (rs1 <= 0) break ;
	            if (nhandle > 0) nhandle -= 1 ;
	        }
	        if ((pip->daytime & 3) == 0) {
	            if ((rs1 == SR_CHILD) && (nhandle > 0)) {
	                nhandle -= 1 ;
	            }
	        }
	    }

	    if ((rs >= 0) && (pip->config != NULL)) {
	        f = ((pip->daytime - ti_config) >= lip->intconf) ;
	        if (f) {
	            CONFIG	*csp = (CONFIG *) pip->config ;
	            ti_config = pip->daytime ;
	            rs = config_check(csp) ;
	        }
	    } /* end if (have) */

	    if (rs >= 0) {
	        f = ((pip->daytime - ti_svcs) >= lip->intsvcs) ;
	        if (f) {
	            ti_svcs = pip->daytime ;
	            rs = locinfo_svcscheck(lip,pip->daytime) ;
	        }
	    } /* end if (have) */

	    f = ((pip->daytime - ti_pid) >= pip->intlock) ;
	    if ((rs >= 0) && f && lip->open.pidlock) {
	        ti_pid = pip->daytime ;
	        rs = proclockcheck(pip) ;
	    }

	    f = ((pip->daytime - ti_logflush) >= TO_LOGFLUSH) ;
	    if ((rs >= 0) && f) {
	        ti_logflush = pip->daytime ;
	        rs = proglog_flush(pip) ;
	    }

	    if (pip->intrun > 0) {
	        if ((pip->daytime-ti_start) >= pip->intrun) break ;
	    }
	} /* end while */

	return (rs >= 0) ? u : rs ;
}
/* end subroutine (procdaemons) */


static int procdaemoner(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		clen = 0 ;
	cchar		*dname ;
	cchar		*dcn = "default.htm" ;
	char		dbuf[MAXPATHLEN+1] ;
	dname = lip->tmpourdname ;
	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: making\n" ;
	    shio_printf(pip->efp,fmt,pn) ;
	}
	{
	    char	tbuf[TIMEBUFLEN+1] ;
	    timestr_logz(pip->daytime,tbuf) ;
	    proglog_printf(pip,"%s making",tbuf) ;
	}
	if ((rs = mkpath2(dbuf,dname,dcn)) >= 0) {
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = procpage_make(pip,tbuf)) >= 0) {
	        clen = rs ;
	        rs = u_rename(tbuf,dbuf) ;
	        if (rs < 0) {
	            uc_unlink(tbuf) ;
	        }
	    } /* end if (procpage_make) */
	} /* end if (mkpath) */
	return (rs >= 0) ? clen : rs ;
}
/* end subroutine (procdaemoner) */


static int procdaemoncheck(PROGINFO *pip)
{
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("msumain/procdaemoncheck: ent\n") ;
#endif

	rs = procmntcheck(pip) ;

	return rs ;
}
/* end subroutine (procdaemoncheck) */


static int procregular(PROGINFO *pip,cchar *ofn,cchar *afn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		c = 1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("msumain/procregular: ent\n") ;
#endif

	if ((rs = locinfo_defs(lip)) >= 0) {
	    if ((rs = locinfo_defreg(lip)) >= 0) {
	        if ((rs = procbackmaint(pip)) >= 0) {
	            rs = procresp(pip,ofn,afn) ;
	        }
	    } /* end if (locinfo_defreg) */
	} /* end if (locinfo_defs) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("msumain/procregular: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procregular) */


/* ARGSUSED */
static int procresp(PROGINFO *pip,cchar *ofn,cchar *afn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procresp: ent ofn=%s\n",ofn) ;
#endif

	if ((rs = locinfo_svcsbegin(lip)) >= 0) {
	    SHIO		ofile, *ofp = &ofile ;
	    const mode_t	om = 0666 ;
	    if ((rs = shio_open(ofp,ofn,"wct",om)) >= 0) {
	        char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = procpage_begin(pip,tbuf)) >= 0) {
	            if (pip->verboselevel > 0) {
	                CGI		hdrs, *hp = &hdrs ;
	                const int	clen = rs ;
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
	            rs1 = procpage_end(pip,tbuf) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (mkpath-template) */
	        rs1 = shio_close(ofp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: inaccessible output (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    } /* end if (file-output) */
	    rs1 = locinfo_svcsend(lip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (locinfo-svcs) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procresp: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procresp) */


static int procexecname(PROGINFO *pip,char *rbuf,int rlen)
{
	int		rs ;
	if ((rs = proginfo_progdname(pip)) >= 0) {
	    cchar	*dn = pip->progdname ;
	    cchar	*pn = pip->progname ;
	    rs = mknpath2(rbuf,rlen,dn,pn) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procexecname: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (procexecname) */


static int procpage_begin(PROGINFO *pip,char *dbuf)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		clen = 0 ;
	cchar		*dcn = DEFPAGE ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procpage_begin: ent\n") ;
	    debugprintf("procpage_begin: f_havesvcs=%u\n",lip->have.svcs) ;
	    debugprintf("procpage_begin: svcs{%p}\n",lip->svcs) ;
	}
#endif
	if ((rs = locinfo_tmpourdname(lip)) >= 0) {
	    cchar	*dname = lip->tmpourdname ;
	    if ((rs = locinfo_svclistfinal(lip)) == 0) {
	        if ((rs = mkpath2(dbuf,dname,dcn)) >= 0) {
	            USTAT		sb ;
	            const time_t	dt = pip->daytime ;
	            const int		nrs = SR_NOENT ;
	            const int		am = R_OK ;
	            const int		f_force = lip->f.force ;
	            char		tbuf[MAXPATHLEN+1] ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("procpage_begin: d=%s\n",dbuf) ;
#endif
	            if ((rs = u_stat(dbuf,&sb)) == nrs) {
#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("procpage_begin: NOENT\n") ;
#endif
	                if ((rs = procpage_make(pip,tbuf)) >= 0) {
	                    clen = rs ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(4)) {
	                        debugprintf("procpage_begin: "
	                            "procpage_make() rs=%d\n", rs) ;
	                        debugprintf("procpage_begin: o=%s\n",tbuf) ;
	                        debugprintf("procpage_begin: n=%s\n",dbuf) ;
	                    }
#endif
	                    lip->f.defpage = TRUE ;
	                    rs = u_rename(tbuf,dbuf) ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("procpage_begin: "
	                            "u_rename() rs=%d\n",rs) ;
#endif
	                }
	            } else if ((rs = sperm(&pip->id,&sb,am)) == SR_ACCESS) {
#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("procpage_begin: ACCESS\n") ;
#endif
	                if ((rs = procpage_make(pip,tbuf)) >= 0) {
	                    clen = rs ;
	                    rs = mkpath1(dbuf,tbuf) ;
	                }
	            } else if (((dt-sb.st_mtime) >= lip->intcache) || f_force) {
#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("procpage_begin: TIMEOUT\n") ;
#endif
	                if ((rs = procpage_make(pip,tbuf)) >= 0) {
	                    clen = rs ;
	                    lip->f.defpage = TRUE ;
	                    rs = u_rename(tbuf,dbuf) ;
	                }
	            } else {
	                clen = sb.st_size ;
	                lip->f.defpage = TRUE ;
	            } /* end if (stat) */
	        } /* end if (mkpath) */
	    } else {
	        rs = procpage_make(pip,dbuf) ;
	        clen = rs ;
	    }
	} /* end if (locinfo_tmpourdname) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procpage_begin: ret rs=%d clen=%u\n",rs,clen) ;
#endif
	return (rs >= 0) ? clen : rs ;
}
/* end subroutine (procpage_begin) */


static int procpage_end(PROGINFO *pip,char *tbuf)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	if ((! lip->f.defpage) && (tbuf[0] != '\0')) {
	    uc_unlink(tbuf) ;
	    tbuf[0] = '\0' ;
	}
	return rs ;
}
/* end subroutine (procpage_end) */


static int procpage_make(PROGINFO *pip,char *tbuf)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	int		clen = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procpage_make: ent\n") ;
#endif
	tbuf[0] = '\0' ;
	if ((rs = locinfo_gatherbegin(lip)) >= 0) {
	    cchar	*tcn = "homepageXXXXXX" ;
	    cchar	*dname = lip->tmpourdname ;
	    char	template[MAXPATHLEN+1] ;
	    if ((rs = mkpath2(template,dname,tcn)) >= 0) {
	        const mode_t	om = 0664 ;
	        if ((rs = mktmpfile(tbuf,om,template)) >= 0) {
	            rs = procdoc(pip,tbuf) ;
	            clen = rs ;
	            if (rs < 0) {
	                uc_unlink(tbuf) ;
	                tbuf[0] = '\0' ;
	            }
	        } /* end if (mktmpfile) */
	    } /* end if (mkpath) */
	    rs1 = locinfo_gatherend(lip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (locinfo-gather) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procpage_make: ret rs=%d clen=%u\n",rs,clen) ;
#endif
	return (rs >= 0) ? clen : rs ;
}
/* end subroutine (procpage_make) */


static int procdoc(PROGINFO *pip,cchar *ofn)
{
	LOCINFO		*lip = pip->lip ;
	SHIO		ofile, *ofp = &ofile ;
	HTM		h ;
	const mode_t	om = 0666 ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*lang = "en" ;

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
	        cchar	*hfname = lip->hfname ;
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
	                {
	                    rs = procdocbody(pip,&h) ;
	                }
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
	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (file-output) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procdoc: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procdoc) */


static int procdocbody(PROGINFO *pip,HTM *hdp)
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


static int procdocbodyhdr(PROGINFO *pip,HTM *hdp)
{
	int		rs ;
	int		rs1 ;

	if ((rs = htm_tagbegin(hdp,"header",NULL,NULL,NULL)) >= 0) {
	    LOCINFO		*lip = pip->lip ;
	    if ((rs = htm_tagbegin(hdp,"h1",NULL,NULL,NULL)) >= 0) {
	        const int	hlen = HBUFLEN ;
	        cchar		*fn = lip->wfname ;
	        char		hbuf[HBUFLEN+1] ;
	        if ((rs = procdocbodyhdrfile(pip,hbuf,hlen,fn)) >= 0) {
	            rs = htm_printline(hdp,hbuf,rs) ;
	        } else if (isNotPresent(rs)) {
	            cchar	*hp = pip->org ;
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


static int procdocbodyhdrfile(PROGINFO *pip,char *hbuf,int hlen,cchar *fn)
{
	bfile		hfile, *hfp = &hfile ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	if (pip == NULL) return SR_FAULT ;
	if ((fn == NULL) || (fn[0] == '\0')) return SR_NOENT ; /* special */
	if ((rs = bopen(hfp,fn,"r",0666)) >= 0) {
	    int		cl ;
	    cchar	*cp ;
	    while ((rs = breadline(hfp,hbuf,hlen)) > 0) {
	        len = rs ;
	        if (hbuf[len-1] == '\n') len -= 1 ;
	        if ((cl = sfshrink(hbuf,len,&cp)) > 0) {
	            if (cp != hbuf) {
	                memmove(hbuf,cp,cl) ;
	                hbuf[cl] = '\0' ;
	            }
	            len = cl ;
	        } else {
	            len = 0 ;
	        }
	        if (len > 0) break ;
	    } /* end while */
	    rs1 = bclose(hfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* enbd if (bfile) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procdocbodyhdrfile) */


static int procdocbodymain(PROGINFO *pip,HTM *hdp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_homepage/procdocbodyb_homepage: ent\n") ;
#endif

	if (lip->sfname != NULL) {
	    cchar	*tag = "main" ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        debugprintf("b_homepage/procdocbodyb_homepage: svcs¬\n") ;
	        if ((rs = locinfo_svclistcount(lip)) > 0) {
	            VECPSTR	*slp = lip->svcs ;
	            int		i ;
	            cchar	*cp ;
	            for (i = 0 ; vecpstr_get(slp,i,&cp) >= 0 ; i += 1) {
	                if (cp == NULL) continue ;
	                debugprintf("b_homepage/procdocbodyb_homepage: "
	                    "svc=%s\n",cp) ;
	            }
	        }
	    }
#endif /* CF_DEBUG */
	    if ((rs = htm_tagbegin(hdp,tag,NULL,NULL,NULL)) >= 0) {
	        {
	            rs = procdocbodymain_svcs(pip,hdp) ;
	        }
	        rs1 = htm_tagend(hdp,tag) ;
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


static int procdocbodyfooter(PROGINFO *pip,HTM *hdp)
{
	int		rs ;
	int		rs1 ;
	cchar		*tag = "footer" ;

	if ((rs = htm_tagbegin(hdp,tag,NULL,NULL,NULL)) >= 0) {
	    if (rs >= 0) {
	        rs = htm_hr(hdp,NULL,NULL) ;
	    } /* end if (ok) */
	    if (rs >= 0) {
	        rs = procdocbodyfooterleft(pip,hdp) ;
	    } /* end if (ok) */
	    if (rs >= 0) {
	        rs = procdocbodyfooterext(pip,hdp) ;
	    } /* end if (ok) */
	    rs1 = htm_tagend(hdp,tag) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (htm-footer) */

	return rs ;
}
/* end subroutine (procdocbodyfooter) */


static int procdocbodyfooterleft(PROGINFO *pip,HTM *hdp)
{
	int		rs ;
	int		rs1 ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = htm_tagbegin(hdp,"div","left",NULL,NULL)) >= 0) {
	    LOCINFO	*lip = pip->lip ;
	    if ((rs = htm_tagbegin(hdp,"p","center",NULL,NULL)) >= 0) {
	        const int	hlen = (strlen(lip->webmaster)+10) ;
	        cchar		*n = NULL ;
	        cchar		*class = "center" ;
	        cchar		*title = lip->webmaster ;
	        char		*hbuf ;
	        if ((rs = uc_malloc((hlen+1),&hbuf)) >= 0) {
	            cchar	*href = hbuf ;
	            strdcpy2(hbuf,hlen,"mailto:",title) ;
	            if ((rs = htm_abegin(hdp,class,n,href,title)) >= 0) {
	                {
	                    rs = htm_printline(hdp,"webmaster",-1) ;
	                }
	                rs1 = htm_aend(hdp) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (htm-a) */
	            uc_free(hbuf) ;
	        } /* end if (m-a-f) */
	        rs1 = htm_tagend(hdp,"p") ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (htm-p) */
	    if (rs >= 0) {
	        if ((rs = htm_tagbegin(hdp,"p","center",NULL,NULL)) >= 0) {
	            LOCINFO	*lip = pip->lip ;
	            {
	                cchar	*c = lip->copyright ;
	                rs = htm_printline(hdp,c,-1) ;
	            }
	            rs1 = htm_tagend(hdp,"p") ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (htm-tag) */
	    } /* end if (ok) */
	    rs1 = htm_tagend(hdp,"div") ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (htm-tag) */
	return rs ;
}
/* end subroutine (procdocbodyfooterleft) */


static int procdocbodyfooterext(PROGINFO *pip,HTM *hdp)
{
	int		rs ;
	int		rs1 ;
	cchar *src = "http://rightcore.com/CGI/wc?db=rightcore&c=homepage" ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = htm_tagbegin(hdp,"div","exticons",NULL,NULL)) >= 0) {
	    {
	        const int	w = 20 ;
	        const int	h = 22 ;
	        cchar		*class = NULL ;
	        cchar		*id = NULL ;
	        cchar		*title = "web counter" ;
	        cchar		*alt = "web counter" ;
	        rs = htm_img(hdp,class,id,src,title,alt,w,h) ;
	    }
	    rs1 = htm_tagend(hdp,"div") ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (htm-div) */
	return rs ;
}
/* end subroutine (procdocbodyfooterext) */


static int procdocbodymain_svcs(PROGINFO *pip,HTM *hdp)
{
	int		rs ;

	rs = procdocbodymain_svcprint(pip,hdp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_homepage/procdocbodymain_svcs: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdocbodymain_svcs) */


static int procdocbodymain_svcprint(PROGINFO *pip,HTM *hdp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	{
	    SVCFILE	*slp = &lip->s ;
	    GATHER	*glp = &lip->g ;
	    SVCFILE_ENT	se ;
	    int		i ;
	    cchar	*snp ;
	    for (i = 0 ; locinfo_svclistget(lip,i,&snp) >= 0 ; i += 1) {
	        if (snp != NULL) {
	            const int	el = SVCENTLEN ;
	            char	eb[SVCENTLEN+1] ;
	            void	*n = NULL ;
	            if ((rs = svcfile_fetch(slp,snp,n,&se,eb,el)) > 0) {
	                rs = procdocbodymain_svcer(pip,hdp,glp,&se) ;
	            }
	        } /* end if (non-null) */
	        if (rs < 0) break ;
	    } /* end for */
	} /* end block */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_homepage/procdocbodymain_svcprint: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdocbodymain_svcprint) */


static int procdocbodymain_svcer(PROGINFO *pip,HTM *hdp,GATHER *glp,
SVCFILE_ENT *sep)
{
	int		rs ;
	cchar		*hp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    const int	n = sep->nkeys ;
	    int		hl ;
	    debugprintf("procdocbodymain_svcer: svc=%s\n",sep->svc) ;
	    if ((hl = svckv_val(sep->keyvals,n,"h",&hp)) > 0) {
	        debugprintf("procdocbodymain_svcer: h=>%t<\n",hp,hl) ;
	    }
	}
#endif /* CF_DEBUG */

	if ((rs = svcent_getval(sep,"h",&hp)) > 0) {
	    const int	n = sep->nkeys ;
	    cchar	*(*kv)[2] = sep->keyvals ;
	    cchar	*k = "include" ;
	    cchar	*vp ;
	    if ((rs = svcent_getval(sep,k,&vp)) > 0) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("procdocbodymain_svcer: include\n") ;
#endif
	        rs = procdocbodymain_svcerinc(pip,hdp,sep) ;
	    } else if ((rs = svckv_isfile(kv,n,&vp)) > 0) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("procdocbodymain_svcer: isfile\n") ;
#endif
	        rs = procdocbodymain_svcerfile(pip,hdp,glp,sep) ;
	    } else if ((rs = svckv_isprog(kv,n,&vp)) > 0) {
	        rs = 2 ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("procdocbodymain_svcer: isexec\n") ;
#endif
	    } /* end if (alternatives) */
	} /* end if (svckv) */

	return rs ;
}
/* end subroutine (procdocbodymain_svcer) */


static int procdocbodymain_svcerhdr(PROGINFO *pip,HTM *hdp,
SVCFILE_ENT *sep)
{
	int		rs ;
	int		rs1 ;
	int		vl ;
	cchar		*vp ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = svcent_getdeval(sep,"h",&vp)) > 0) {
	    vl = rs ;
	    if ((rs = htm_tagbegin(hdp,"h3",NULL,NULL,NULL)) >= 0) {
	        {
	            rs = htm_printline(hdp,vp,vl) ;
	        }
	        rs1 = htm_tagend(hdp,"h3") ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (htm-h3) */
	} /* end if (svcent_getdeval) */
	return rs ;
}
/* end subroutine (procdocbodymain_svcerhdr) */


static int procdocbodymain_svcerinc(PROGINFO *pip,HTM *hdp,
SVCFILE_ENT *sep)
{
	int		rs ;
	cchar		*k = "include" ;
	cchar		*vp ;
	if ((rs = svcent_getval(sep,k,&vp)) > 0) {
	    char	ibuf[MAXPATHLEN+1] ;
	    if ((rs = mkincfname(pip,ibuf,vp,rs)) >= 0) {
	        rs = procdocbodymain_svcerincer(pip,hdp,sep,ibuf) ;
	    } /* end if (mkincfname) */
	} /* end if (svcent_getval) */
	return rs ;
}
/* end subroutine (procdocbodymain_svcerinc) */


static int procdocbodymain_svcerincer(PROGINFO *pip,HTM *hdp,
SVCFILE_ENT *sep,cchar *ibuf)
{
	bfile		ifile, *ifp = &ifile ;
	int		rs ;
	int		rs1 ;
	if ((rs = bopen(ifp,ibuf,"r",0666)) >= 0) {
	    if ((rs = bsize(ifp)) > 0) {
	        if ((rs = procdocbodymain_svcerhdr(pip,hdp,sep)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            char		lbuf[LINEBUFLEN+1] ;
	            while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	                rs = htm_write(hdp,lbuf,rs) ;
	                if (rs < 0) break ;
	            } /* end while */
	        } /* end if (procdocbodymain_svcerhdr) */
	    } /* end if (bsize) */
	    rs1 = bclose(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
	return rs ;
}
/* end subroutine (procdocbodymain_svcerincer) */


static int procdocbodymain_svcerfile(PROGINFO *pip,HTM *hdp,GATHER *glp,
SVCFILE_ENT *sep)
{
	int		rs ;
	cchar		*svc = sep->svc ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("procdocbodymain_svcerfile: ent svc=%s\n",svc) ;
#endif

	if ((rs = gather_getlines(glp,svc)) > 0) {
	    const int	nl = rs ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("procdocbodymain_svcerfile: nl=%u\n",nl) ;
#endif
	    if ((rs = procdocbodymain_svcerhdr(pip,hdp,sep)) >= 0) {
	        rs = procdocbodymain_svcerfiler(pip,hdp,glp,sep,nl) ;
	    }
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("procdocbodymain_svcerfile: "
	            "dequote-out rs=%d\n",rs) ;
#endif
	} /* end if (gather_getlines) */

	if (rs < 0) {
	    cchar	*fmt = "FAILED svc=%s (%d)" ;
	    proglog_printf(pip,fmt,svc,rs) ;
	}


#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("procdocbodymain_svcerfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdocbodymain_svcerfile) */


static int procdocbodymain_svcerfiler(PROGINFO *pip,HTM *hdp,GATHER *glp,
SVCFILE_ENT *sep,int nl)
{
	const int	n = sep->nkeys ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		vl ;
	cchar	*vp ;
	if (pip == NULL) return SR_FAULT ;
	if ((vl = svckv_dequote(sep->keyvals,n,"h",&vp)) > 0) {
	    NULSTR	ts ;
	    cchar	*t ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("procdocbodymain_svcerfile: t=>%t<\n",vp,vl) ;
#endif
	    if ((rs = nulstr_start(&ts,vp,vl,&t)) >= 0) {
	        cchar	*svc = sep->svc ;
	        cchar	*dp ;
	        if ((rs = gather_getbuf(glp,svc,&dp)) >= 0) {
	            const int	dl = rs ;
	            const int	r = (nl+1) ;
	            const int	c = COLUMNS ;
	            int		i = 0 ;
	            cchar		*class = NULL ;
	            cchar		*id = NULL ;
	            cchar		*kv[2][2] ;
	            kv[i][0] = "readonly" ;
	            kv[i][1] = NULL ;
	            i += 1 ;
	            kv[i][0] = NULL ;
	            kv[i][1] = NULL ;
	            if ((rs = htm_textbegin(hdp,class,id,t,r,c,kv)) >= 0) {

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
	} /* end if (svckv) */
	return rs ;
}
/* end subroutine (procdocbodymain_svcerfiler) */


static int prochdrs(PROGINFO *pip,CGI *hp,int clen)
{
	const int	hlen = HBUFLEN ;
	int		rs ;
	int		wlen = 0 ;
	char		hbuf[HBUFLEN+1] ;

	if ((rs = ctdeci(hbuf,hlen,clen)) >= 0) {
	    rs = cgi_hdr(hp,"content-length",hbuf,rs) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    time_t	t = pip->daytime ;
	    rs = cgi_hdrdate(hp,t) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    cchar	*k = "content-type" ;
	    cchar	*v = "text/html; charset=ISO-8859-1" ;
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


static int proclockcheck(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	LFM_CHECK	lc ;
	int		rs ;
	if ((rs = locinfo_lockcheck(lip,&lc)) < 0) {
	    if ((rs == SR_LOCKLOST) || (rs == SR_AGAIN)) {
	        proclockprint(pip,pip->pidfname,&lc) ;
	    }
	}

	return rs ;
}
/* end subroutine (proclockcheck) */


/* print out lock-check information */
static int proclockprint(PROGINFO *pip,cchar *lfname,LFM_CHECK *lcp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	cchar		*np ;

	switch (lcp->stat) {
	case SR_AGAIN:
	    np = "busy" ;
	    break ;
	case SR_LOCKLOST:
	    np = "lost" ;
	    break ;
	default:
	    np = "unknown" ;
	    break ;
	} /* end switch */

	if (pip->open.logprog) {
	    if (! lip->f.quietlock) {
	        proclocklog(pip,lfname,lcp,np) ;
	    } else {
	        proglog_printf(pip,"lock=%s",lfname) ;
	        proglog_printf(pip,"pid=%s",np) ;
	    }
	}

	if (! lip->f.quietlock) {
	    if ((pip->debuglevel > 0) && (pip->efp != NULL)) {
	        cchar	*pn = pip->progname ;
	        char	tbuf[TIMEBUFLEN + 1] ;

	        timestr_logz(pip->daytime,tbuf) ;
	        shio_printf(pip->efp,"%s: %s lock %s\n",pn,tbuf,np) ;

	        shio_printf(pip->efp,"%s: other_pid=%d\n",pn,lcp->pid) ;

	        if (lcp->nodename != NULL) {
	            cchar	*nn = lcp->nodename ;
	            shio_printf(pip->efp,"%s: other_node=%s\n",pn,nn) ;
	        }

	        if (lcp->username != NULL) {
	            cchar	*un = lcp->username ;
	            rs = shio_printf(pip->efp,"%s: other_user=%s\n",pn,un) ;
	        }

	        if (lcp->banner != NULL) {
	            cchar	*bn = lcp->banner ;
	            shio_printf(pip->efp,"%s: other_banner=>%s<\n",pn,bn) ;
	        }

	    } /* end if (standard-error) */
	} /* end if (quiet-lock) */

	return rs ;
}
/* end subroutine (proclockprint) */


static int proclocklog(PROGINFO *pip,cchar *lfname,LFM_CHECK *lcp,cchar *np)
{
	int		rs = SR_OK ;
	char		tbuf[TIMEBUFLEN + 1] ;

	timestr_logz(pip->daytime,tbuf) ;
	proglog_printf(pip, "%s lock %s\n",tbuf, np) ;

	proglog_printf(pip, "pid=%s",lfname) ;

	proglog_printf(pip, "other_pid=%d\n", lcp->pid) ;

	if (lcp->nodename != NULL) {
	    proglog_printf(pip, "other_node=%s\n", lcp->nodename) ;
	}

	if (lcp->username != NULL) {
	    proglog_printf(pip, "other_user=%s\n", lcp->username) ;
	}

	if (lcp->banner != NULL) {
	    proglog_printf(pip, "other_banner=>%s<\n", lcp->banner) ;
	}

	return rs ;
}
/* end subroutine (proclocklog) */


static int gather_start(GATHER *glp,PROGINFO *pip,cchar *termtype,int cols)
{
	int		rs ;
	cchar		*cp ;
	if (glp == NULL) return SR_FAULT ;
	if (termtype == NULL) return SR_FAULT ;
	if (cols <= 0) cols = COLUMNS ;
	memset(glp,0,sizeof(GATHER)) ;
	glp->pip = pip ;
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


static int gather_file(GATHER *glp,cchar *svc,int f_to,cchar *fp,int fl)
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
	    cchar	*tt = (f_to) ? glp->termtype : NULL ;
	    if ((rs = filer_start(fep,glp->pip,tt,c,svc,fp,fl)) >= 0) {
	        if ((rs = vechand_add(&glp->ents,fep)) >= 0) {
	            glp->nout += 1 ;
	        }
	        if (rs < 0)
	            filer_finish(fep) ;
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


static int gather_getlines(GATHER *glp,cchar *svc)
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


static int gather_getbuf(GATHER *glp,cchar *svc,cchar **rpp)
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


static int filer_start(FILER *fep,PROGINFO *pip,cchar *tt,int cols,cchar *svc,
cchar *fp,int fl)
{
	const int	to = 30 ;
	int		rs ;
	int		rs1 ;
	int		size = 0 ;
	char		*bp ;

#if	CF_DEBUGS
	debugprintf("b_homepage/filer_start: ent svc=%s\n",svc) ;
	debugprintf("b_homepage/filer_start: fn=%t\n",fp,fl) ;
#endif

	memset(fep,0,sizeof(FILER)) ;
	fep->pip = pip ;
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
	            if ((rs = pta_create(&ta)) >= 0) {
	                const caddr_t	saddr = fep->saddr ;
	                int		ssize = fep->ssize ;
	                if ((rs = pta_setstack(&ta,saddr,ssize)) >= 0) {
	                    tworker	tw = (tworker) filer_worker ;
	                    pthread_t	tid ;
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
	    const int	pattern = MEMPATTERN ;
	    int		*ip = (int *) fep->saddr ;
	    int		il ;
	    int		i ;
	    il = (fep->ssize >> shift) ;
	    for (i = 0 ; i < il ; i += 1) {
	        *ip++ = pattern ;
	    }
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
	    const int	pattern = MEMPATTERN ;
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


/* this is an independent thread */
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
	    const int	fd = rs ;
#if	CF_DEBUGS
	    debugprintf("filer_worker: uc_open() svc=%s fd=%d\n",fep->svc,fd) ;
#endif
	    if ((rs = filer_workread(fep,fd)) > 0) {
	        int	lines = 0 ;
	        int	sl = rs ;
	        cchar	*sp = fep->dbuf ;
	        cchar	*tp ;
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
	cchar		*tt = fep->termtype ;
	if ((rs = termout_start(&out,tt,-1,cols)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    char	*lbuf ;
	    if ((rs = uc_malloc((llen+1),&lbuf)) >= 0) {
	        FILEBUF		b ;
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
	    } /* end if (m-a-f) */
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


static int filer_workreadtermline(FILER *fep,TERMOUT *top,char *rp,
cchar *lbuf,int len)
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
	    cchar	*lp ;
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
	PROGINFO	*pip = fep->pip ;
	int		rs ;
#if	CF_DEBUGS
	debugprintf("b_homepage/filer_getlines: ent svc=%s f_run=%u\n",
	    fep->svc,fep->f_running) ;
#endif
	if (fep->f_running) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    int		trs ;
	    fep->f_running = FALSE ;
	    if ((rs = uptjoin(fep->tid,&trs)) >= 0) {
#if	CF_DEBUGS
	        debugprintf("b_homepage/filer_getlines: uptjoin() trs=%d\n",
	            trs) ;
#endif
	        if (trs >= 0) {
	            fep->dl = trs ;
	            rs = fep->lines ;
	        } else {
	            rs = trs ;
	            fmt = "%s: worker failure svc=%s (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,fep->svc,rs) ;
	        }
	    } else {
	        fmt = "%s: join failure svc=%s (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,fep->svc,rs) ;
	    } /* end if (pthead-join) */
#if	CF_DEBUGS
	    debugprintf("b_homepage/filer_getlines: uptjoin-out rs=%d\n",rs) ;
#endif
	} else {
	    rs = fep->lines ;
	}
#if	CF_DEBUGS
	debugprintf("b_homepage/filer_getlines: ret rs=%d svc=%s lns=%u\n",
	    rs,fep->svc,fep->lines) ;
#endif
	return rs ;
}
/* end subroutine (filer_getlines) */


static int filer_getbuf(FILER *fep,cchar **rpp)
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


static int filer_havesvc(FILER *fep,cchar *svc)
{
	int		f = TRUE ;
	if (fep == NULL) return SR_FAULT ;
	if (svc == NULL) return SR_FAULT ;
	f = f && (svc[0] == fep->svc[0]) ;
	f = f && (strcmp(svc,fep->svc) == 0) ;
	return f ;
}
/* end subroutine (filer_havesvc) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to_cache = -1 ;
	lip->to_lock = -1 ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	rs1 = locinfo_svclistend(lip) ;
	if (rs >= 0) rs = rs1 ;

	if (lip->open.cooks) {
	    lip->open.cooks = FALSE ;
	    rs1 = locinfo_cookend(lip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

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


static int locinfo_dbinfo(LOCINFO *lip,cchar *basedname,cchar *dbfname)
{
	PROGINFO	*pip = lip->pip ;
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
	        cchar	**vpp = &lip->dbfname ;
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


static int locinfo_basedir(LOCINFO *lip,cchar *vp,int vl)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip->basedname == NULL) {
	    cchar	*inter = ETCCNAME ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkourname(pip,tbuf,inter,vp,vl)) >= 0) {
	        cchar	**vpp = &lip->basedname ;
	        rs = locinfo_setentry(lip,vpp,tbuf,rs) ;
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_basedir) */


static int locinfo_wfname(LOCINFO *lip,cchar *wfname)
{
	int		rs = SR_OK ;
	if ((lip->wfname == NULL) && (wfname != NULL)) {
	    cchar	**vpp = &lip->wfname ;
	    rs = locinfo_setentry(lip,vpp,wfname,-1) ;
	}
	return rs ;
}
/* end subroutine (locinfo_wfname) */


static int locinfo_sethead(LOCINFO *lip,cchar *vp,int vl)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/locinfo_sethead: v=%t\n",vp,vl) ;
#endif

	if (lip->hfname == NULL) {
	    cchar	*inter = ETCCNAME ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkourname(pip,tbuf,inter,vp,vl)) >= 0) {
	        cchar	**vpp = &lip->hfname ;
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
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (pip == NULL) return SR_FAULT ;
	if (lip->svcs == NULL) {
	    const int	size = sizeof(VECPSTR) ;
	    void	*p ;
	    f = TRUE ;
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
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/locinfo_svclistbegin: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
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


static int locinfo_svclistadds(LOCINFO *lip,cchar *sp,int sl)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		cl ;
	int		c = 0 ;
	cchar		*tp ;
	cchar		*cp ;
	if (pip == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_homepage/locinfo_svclistadds: s=>%t<\n",sp,sl) ;
#endif
	while ((tp = strnchr(sp,sl,',')) != NULL) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_homepage/locinfo_svclistadds: svc=>%t<\n",
	            sp,(tp-sp)) ;
#endif
	    if ((cl = sfshrink(sp,(tp-sp),&cp)) > 0) {
	        rs = locinfo_svclistadd(lip,cp,cl) ;
	        c += rs ;
	    }
	    sl -= ((tp+1)-sp) ;
	    sp = (tp+1) ;
	    if (rs < 0) break ;
	} /* end while */
	if ((rs >= 0) && sl) {
	    if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	        rs = locinfo_svclistadd(lip,cp,cl) ;
	        c += rs ;
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_homepage/locinfo_svclistadds: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_svclistadds) */


static int locinfo_svclistadd(LOCINFO *lip,cchar *vp,int vl)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (pip == NULL) return SR_FAULT ;

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
	    cchar	st[] = { 
	        CH_FS, 0 		} ;
	    cchar	*sp ;
	    if (vl < 0) vl = strlen(vp) ;
	    while (vl > 0) {
	        if ((sl = nextfieldterm(vp,vl,st,&sp)) < 0) break ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/locinfo_svclistadd: s=>%t<\n",sp,sl) ;
#endif
	        if (sl > 0) {
	            lip->have.svcs = TRUE ;
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


static int locinfo_svclistget(LOCINFO *lip,int i,cchar **rpp)
{
	int		rs = SR_NOTFOUND ;
	if (lip->svcs != NULL) {
	    VECPSTR	*slp = lip->svcs ;
	    rs = vecpstr_get(slp,i,rpp) ;
	}
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


static int locinfo_svclistdelall(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if ((lip->svcs != NULL) && (! lip->final.svcs)) {
	    VECPSTR	*slp = lip->svcs ;
	    rs = vecpstr_delall(slp) ;
	}
	return rs ;
}
/* end subroutine (locinfo_svclistdelall) */


static int locinfo_svclistcount(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (lip->svcs != NULL) {
	    VECPSTR	*slp = lip->svcs ;
	    rs = vecpstr_count(slp) ;
	}
	return rs ;
}
/* end subroutine (locinfo_svclistcount) */


static int locinfo_svclistfinal(LOCINFO *lip)
{
	const int	f = lip->final.svcs ;
	return f ;
}
/* end subroutine (locinfo_svclistfinal) */


static int locinfo_copyright(LOCINFO *lip,cchar *vp,int vl)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	if (lip->copyright == NULL) {
	    const int	elen = (vl+MAXNAMELEN) ;
	    char	*ebuf ;
	    if ((rs = uc_malloc((elen+1),&ebuf)) >= 0) {
	        const int	w = FALSE ;
	        if ((rs = locinfo_cookexp(lip,w,ebuf,elen,vp,vl)) >= 0) {
	            cchar	**vpp = &lip->copyright ;
	            rs = locinfo_setentry(lip,vpp,ebuf,rs) ;
	        }
	        uc_free(ebuf) ;
	    } /* end if (m-a-f) */
	} /* end if (needed) */

	return rs ;
}
/* end subroutine (locinfo_copyright) */


static int locinfo_defs(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if ((rs >= 0) && (lip->hfname == NULL)) {
	    cchar	*inter = ETCCNAME ;
	    cchar	*fn = HEADFNAME ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkourname(pip,tbuf,inter,fn,-1)) >= 0) {
	        const int	tl = rs ;
	        if ((rs = perm(tbuf,-1,-1,NULL,R_OK)) >= 0) {
	            cchar	**vpp = &lip->hfname ;
	            rs = locinfo_setentry(lip,vpp,tbuf,tl) ;
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	    } /* end if (mkourname) */
	}

	if (rs >= 0) {
	    rs = locinfo_defsvc(lip) ;
	}

	if (lip->termtype == NULL) lip->termtype = DEFTERMTYPE ;

	if ((rs >= 0) && (lip->cols == 0)) {
	    cchar	*vp = getourenv(pip->envv,VARCOLUMNS) ;
	    if ((vp != NULL) && (vp[0] != '\0')) {
	        rs = optvalue(vp,-1) ;
	        lip->cols = rs ;
	    }
	    if (lip->cols == 0) lip->cols = COLUMNS ;
	} /* end if (columns) */

	if (lip->intcache == 0) lip->intcache = TO_CACHE ;

	if (lip->intwait == 0) lip->intwait = TO_WAIT ;

	if (lip->intconf == 0) lip->intconf = TO_CONFIG ;

	if (lip->intsvcs == 0) lip->intsvcs = TO_SVCS ;

	return rs ;
}
/* end subroutine (locinfo_defs) */


static int locinfo_defsvc(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if ((rs >= 0) && (lip->sfname == NULL)) {
	    char	tbuf[MAXPATHLEN+1] ;
	    cchar	*inter = ETCCNAME ;
	    cchar	*fn = SVCFNAME ;
	    if ((rs = mkourname(pip,tbuf,inter,fn,-1)) >= 0) {
	        const int	tl = rs ;
	        if ((rs = perm(tbuf,-1,-1,NULL,R_OK)) >= 0) {
	            cchar	**vpp = &lip->sfname ;
	            rs = locinfo_setentry(lip,vpp,tbuf,tl) ;
	        } else if (isNotAccess(rs)) {
	            rs = SR_OK ;
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
	    lip->open.s = TRUE ;
	}
	return rs ;
}
/* end subroutine (locinfo_svcsbegin) */


static int locinfo_svcsend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.s) {
	    rs1 = svcfile_close(&lip->s) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (locinfo_svcsend) */


static int locinfo_svcscheck(LOCINFO *lip,time_t dt)
{
	int		rs = SR_OK ;
	if (lip->open.s) {
	    if (dt < 0) dt = time(NULL) ;
	    rs = svcfile_check(&lip->s,dt) ;
	}
	return rs ;
}
/* end subroutine (locinfo_svcsend) */


static int locinfo_gatherbegin(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	GATHER		*glp = &lip->g ;
	int		rs ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = gather_start(glp,pip,lip->termtype,lip->cols)) >= 0) {
	    if ((rs = locinfo_svclistcount(lip)) > 0) {
	        rs = locinfo_gatherbeginsome(lip) ;
	    } else {
	        rs = locinfo_gatherbeginall(lip) ;
	    }
	    if (rs < 0) {
	        gather_finish(&lip->g) ;
	    }
	} /* end if (gather_start) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_homepage/locinfo_gatherbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_gatherbegin) */


static int locinfo_gatherbeginall(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	SVCFILE		*sfp = &lip->s ;
	SVCFILE_CUR	c ;
	SVCFILE_ENT	se ;
	int		rs ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_gatherbeginall: ent\n") ;
#endif

	lip->f.defsvcs = TRUE ;
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
	        debugprintf("locinfo_gatherbeginall: while-out "
	            "rs=%d rs1=%d\n",rs,rs1) ;
#endif
	    if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	    rs1 = svcfile_curend(sfp,&c) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (svcfile-cur) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_gatherbeginall: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_gatherbeginall) */


static int locinfo_gatherbeginsome(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	SVCFILE		*sfp = &lip->s ;
	const int	nrs = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		i ;
	cchar		*snp ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_gatherbeginsome: ent\n") ;
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
	    debugprintf("locinfo_gatherbeginsome: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_gatherbeginsome) */


static int locinfo_gatherend(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		used = 0 ;
	cchar		*pn ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_homepage/locinfo_gatherend: ent\n") ;
#endif

	pn = pip->progname ;
	rs1 = gather_finish(&lip->g) ;
	if (rs >= 0) rs = rs1 ;
	used = rs1 ;

#if	CF_STACKCHECK
	if ((rs >= 0) && (used >= (STACKSIZE-STACKGUARD))) {
	    cchar	*fmt = "%s: stack-guard violated used=%d\n" ;
	    shio_printf(pip->efp,fmt,pn,used) ;
	}
#endif /* CF_STACKCHECK */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: max stack used=%u\n",pn,used) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_homepage/locinfo_gatherend: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? used : rs ;
}
/* end subroutine (locinfo_gatherend) */


static int locinfo_gatherbeginer(LOCINFO *lip,SVCFILE_ENT *sep)
{
	PROGINFO	*pip = lip->pip ;
	GATHER		*glp = &lip->g ;
	const int	n = sep->nkeys ;
	int		rs ;
	cchar		*(*kv)[2] = sep->keyvals ;
	cchar		*vp ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_gatherbeginer: ent\n") ;
#endif

	if ((rs = svckv_dequote(kv,n,"h",&vp)) > 0) {
	    cchar	*fp ;
	    if ((rs = svckv_isfile(kv,n,&fp)) > 0) {
	        int	fl = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("locinfo_gatherbeginer: "
	                "svckv_svcopts()\n") ;
#endif

	        if ((rs = svckv_svcopts(kv,n)) >= 0) {
	            const int	f_to = bwtst(rs,svcopt_termout) ;
	            cchar	*svc = sep->svc ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("locinfo_gatherbeginer: rs=\\b%04ß\n",rs) ;
#endif
	            rs = gather_file(glp,svc,f_to,fp,fl) ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("locinfo_gatherbeginer: "
	                "gather_file() rs=%d\n",rs) ;
#endif

	    } /* end if (svckv_isfile) */
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("locinfo_gatherbeginer: "
	            "svckv-isfile-out rs=%d\n",rs) ;
#endif
	} /* end if (svckv) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_gatherbeginer: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_gatherbeginer) */


int locinfo_defreg(LOCINFO *lip)
{

	if (lip->intspeed == 0) lip->intspeed = TO_SPEED ;

	return SR_OK ;
}
/* end subroutine (locinfo_defreg) */


int locinfo_defdaemon(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;

	if (lip->intspeed == 0) lip->intspeed = TO_SPEED ;

	if (pip->uid != pip->euid)
	    u_setreuid(pip->euid,-1) ;

	if (pip->gid != pip->egid)
	    u_setregid(pip->egid,-1) ;

	return SR_OK ;
}
/* end subroutine (locinfo_defdaemon) */


int locinfo_lockbegin(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	LFM		*lfp = &lip->pidlock ;
	int		rs ;
	cchar		*lockfname = pip->pidfname ;
	if ((rs = locinfo_lockbeginone(lip,lfp,lockfname)) >= 0) {
	    lip->open.pidlock = (rs > 0) ;
	}
	return rs ;
}
/* end subroutine (locinfo_lockbegin) */


int locinfo_lockcheck(LOCINFO *lip,LFM_CHECK *lcp)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (lip->open.pidlock) {
	    rs = lfm_check(&lip->pidlock,lcp,pip->daytime) ;
	} /* end if (pidlock) */
	return rs ;
}
/* end subroutine (locinfo_lockcheck) */


int locinfo_lockend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.pidlock) {
	    lip->open.pidlock = FALSE ;
	    rs1 = lfm_finish(&lip->pidlock) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (locinfo_lockend) */


static int locinfo_lockbeginone(LOCINFO *lip,LFM *lfp,cchar *lockfname)
{
	PROGINFO	*pip = lip->pip ;
	const mode_t	dmode = 0777 ;
	int		rs = SR_OK ;
	int		f_opened = FALSE ;
	cchar		*ccp = lockfname ;

	if ((rs >= 0) && (ccp != NULL) && (ccp[0] != '\0') && (ccp[0] != '-')) {
	    int		cl ;
	    cchar	*cp ;
	    char	tmpfname[MAXPATHLEN+1] ;

	    cl = sfdirname(lockfname,-1,&cp) ;
	    if ((rs = mkpath1w(tmpfname,cp,cl)) >= 0) {
	        USTAT	usb ;
	        if ((rs = u_stat(tmpfname,&usb)) >= 0) {
	            if (! S_ISDIR(usb.st_mode)) {
	                rs = SR_NOTDIR ;
	            }
	        } else if (rs == SR_NOENT) {
	            rs = mkdirs(tmpfname,dmode) ;
	        }
	    }

	    if (rs >= 0) {
	        LFM_CHECK	lc ;
	        const int	ltype = LFM_TRECORD ;
	        const int	to_lock = lip->to_lock ;
	        cchar		*nn = pip->nodename ;
	        cchar		*un = pip->username ;
	        cchar		*bn = pip->banner ;
	        rs = lfm_start(lfp,ccp,ltype,to_lock,&lc,nn,un,bn) ;
	        f_opened = (rs >= 0) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("locinfo_lockbeginone: lfm_start() rs=%d\n",
	                rs) ;
	        }
#endif

#ifdef	COMMENT
	        if ((pip->debuglevel > 0) && (rs < 0))
	            shio_printf(pip->efp,
	                "%s: inaccessible PID lock (%d)\n",
	                pip->progname,rs) ;
#endif /* COMMENT */

	        if ((rs == SR_LOCKLOST) || (rs == SR_AGAIN)) {
	            proclockprint(pip,ccp,&lc) ;
	        }
	    } /* end if */

	} /* end if (establish lock) */

	return (rs >= 0) ? f_opened : rs ;
}
/* end subroutine (locinfo_lockbeginone) */


int locinfo_tmpourdname(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		pl = 0 ;

	if (lip->tmpourdname == NULL) {
	    if ((rs = proginfo_rootname(pip)) >= 0) {
	        mode_t	dm = 0777 ;
	        int	f_needmode = FALSE ;
	        int	f_created = FALSE ;
	        int	f_runasprn ;
	        cchar	*tn = pip->tmpdname ;
	        cchar	*sn = pip->searchname ;
	        cchar	*un = pip->username ;
	        cchar	*rn = pip->rootname ;
	        char	tmpourdname[MAXPATHLEN + 1] = { 
	            0 			} ;
	        f_runasprn = (strcmp(un,rn) == 0) ;
	        if (! f_runasprn) dm = 0777 ;
	        rs1 = SR_OK ;
	        if ((rs = mkpath3(tmpourdname,tn,rn,sn)) >= 0) {
	            USTAT	usb ;
	            pl = rs ;
	            if ((rs1 = u_stat(tmpourdname,&usb)) >= 0) {
	                if (S_ISDIR(usb.st_mode)) {
	                    const int	am = (R_OK|W_OK|X_OK) ;
	                    f_needmode = ((usb.st_mode & dm) != dm) ;
	                    rs = u_access(tmpourdname,am) ;
	                } else {
	                    rs = SR_NOTDIR ;
	                }
	            }
	            if (rs >= 0) {
	                if (rs1 == SR_NOENT) {
	                    f_needmode = TRUE ;
	                    f_created = TRUE ;
	                    rs = mkdirs(tmpourdname,dm) ;
	                }
	                if ((rs >= 0) && f_needmode) {
	                    rs = uc_minmod(tmpourdname,dm) ;
	                }

#if	CF_TMPGROUP
	                if ((rs >= 0) && f_created) {
	                    rs = locinfo_tmpgroup(lip,tmpourdname) ;
	                }
#endif /* CF_TMPGROUP */

	                if (rs >= 0) {
	                    cchar	**vpp = &lip->tmpourdname ;
	                    rs = locinfo_setentry(lip,vpp,tmpourdname,pl) ;
	                }

	            } /* end if (ok) */
	        } /* end if (mkpath) */
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt ;
	        fmt = "%s: TMPDIR access problem (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        fmt = "%s: tmpdir=%s\n" ;
	        shio_printf(pip->efp,fmt,pn,lip->tmpourdname) ;
	        if (pip->open.logprog) {
	            proglog_printf(pip,"TMPDIR access problem (%d)\n",rs) ;
	            proglog_printf(pip,"tmpdir=%s",lip->tmpourdname) ;
	        }
	    } /* end if */
	} else {
	    pl = strlen(lip->tmpourdname) ;
	}

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (locinfo_tmpourdname) */


#if	CF_TMPGROUP
static int locinfo_tmpgroup(LOCINFO *lip,cchar *tmpourdname)
{
	PROGINFO	*pip = lip->pip ;
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	char		*pwbuf ;
	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    cchar	*rn = pip->rootname ;
	    cchar	*un = pip->username ;
	    if ((rs = locinfo_gidrootname(lip,&pw,pwbuf,pwlen)) >= 0) {
	        const gid_t	gid_prn = lip->gid_rootname ;
	        const int	f_runasprn = (strcmp(un,rn) == 0) ;
	        if (! f_runasprn) {
	            uid_t	uid_prn = pip->euid ;
	            if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,rn)) >= 0) {
	                uid_prn = pw.pw_uid ;
	            } else if (isNotPresent(rs)) {
	                rs = SR_OK ;
	            }
	            if (rs >= 0) {
	                rs = u_chown(tmpourdname,uid_prn,gid_prn) ;
	            }
	        } else {
	            rs = u_chown(tmpourdname,-1,gid_prn) ;
	        }
	    } /* end if (locinfo_gidrootname) */
	    uc_free(pwbuf) ;
	} /* end if (m-a-f) */
	return rs ;
}
/* end subroutine (locinfo_tmpgroup) */
#endif /* CF_TMPGROUP */


#ifdef	COMMENT
int locinfo_ipcpid(LOCINFO *lip,int f)
{
	PROGINFO	*pip = lip->pip ;
	const int	oflags = (O_CREAT | O_WRONLY | O_TRUNC) ;
	int		rs = SR_OK ;
	cchar		*pidcname = PIDCNAME ;
	cchar		*pf ;
	char		pidfname[MAXPATHLEN + 1] ;

	if (pip->pidfname == NULL) {
	    int		pl ;

	    rs = locinfo_tmpourdname(lip) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("msulocinfo/_ipcpid: _tmpourdname() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        rs = mkpath2(pidfname,lip->tmpourdname,pidcname) ;
	        pl = rs ;
	    }

	    if (rs >= 0) {
	        cchar	**vpp = &pip->pidfname ;
	        rs = proginfo_setentry(pip,vpp,pidfname,pl) ;
	    }

	    if ((rs < 0) && (pip->efp != NULL)) {
	        shio_printf(pip->efp,"%s: TMPDIR access problem (%d)\n",
	            pip->progname,rs) ;
	    }

	} /* end if (creating PID file) */

	if (rs >= 0) {
	    pf = pip->pidfname ;
	    if (f) { /* activate */

	        pip->f.pidfname = FALSE ;
	        if ((rs = u_open(pf,oflags,0664)) >= 0) {
	            int 	fd = rs ;
	            int		wl ;
	            char	*pidbuf = pidfname ;
	            if ((rs = ctdeci(pidbuf,MAXPATHLEN,pip->pid)) >= 0) {
	                wl = rs ;
	                pidbuf[wl++] = '\n' ;
	                rs = u_write(fd,pidbuf,wl) ;
	                pip->f.pidfname = (rs >= 0) ;
	            }
	            u_close(fd) ;
	        } /* end if (file) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("msulocinfo/_ipcpid: activate rs=%d\n",rs) ;
#endif

	    } else { /* de-activate */

	        if ((pf != NULL) && pip->f.pidfname) {
	            pip->f.pidfname = FALSE ;
	            if (pf[0] != '\0') u_unlink(pf) ;
	        }

	    } /* end if (invocation mode) */
	} /* end if (ok) */

	if ((rs < 0) && (pip->efp != NULL)) {
	    shio_printf(pip->efp,"%s: PID access problem (%d)\n",
	        pip->progname,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("msulocinfo/_ipcpid: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_ipcpid) */
#endif /* COMMENT */


int locinfo_gidrootname(LOCINFO *lip,struct passwd *pwp,char *pwbuf,int pwlen)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->gid_rootname == 0) {
	    if ((rs = proginfo_rootname(pip)) >= 0) {
	        rs1 = GETPW_NAME(pwp,pwbuf,pwlen,pip->rootname) ;
	        if (rs1 >= 0) lip->gid_rootname = pwp->pw_gid ;
	        if (lip->gid_rootname <= 0) {
	            cchar	*tmpdname = pip->tmpdname ;
	            cchar	*rn = pip->rootname ;
	            char	dname[MAXPATHLEN+1] ;
	            if ((rs = mkpath2(dname,tmpdname,rn)) >= 0) {
	                USTAT	sb ;
	                if ((rs = u_stat(dname,&sb)) >= 0) {
	                    if (S_ISDIR(sb.st_mode)) {
	                        lip->gid_rootname = sb.st_gid ;
	                    } else 
	                        rs = SR_NOTDIR ;
	                }
	            } /* end if (mkpath) */
	        } /* end if (needed) */
	    } /* end if (rootname) */
	} /* end if (needed) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("msulocinfo_gidrootname: ret rs=%d gid=%d\n",
	        rs,lip->gid_rootname) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_gidrootname) */


static int locinfo_defpfname(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (pip->pidfname == NULL) {
	    cchar	**envv = pip->envv ;
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARPIDFNAME)) != NULL) {
	        pip->final.pidfname = TRUE ;
	        pip->have.pidfname = TRUE ;
	        pip->pidfname = cp ;
	    }
	}
	return rs ;
}
/* end subroutine (locinfo_defpfname) */


static int locinfo_qs(LOCINFO *lip,cchar *qs)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	if ((! lip->final.svcs) && (qs != NULL)) {
	    QUERYSTRING		ps ;
	    QUERYSTRING_CUR	cur ;
	    if ((rs = querystring_start(&ps,qs,-1)) >= 0) {
	        if ((rs = querystring_curbegin(&ps,&cur)) >= 0) {
	            cchar	*kp, *vp ;
	            while ((rs1 = querystring_enum(&ps,&cur,&kp,&vp)) >= 0) {
	                if (vp == NULL) break ; /* lint */
	                rs = locinfo_svclistadd(lip,kp,-1) ;
	                c += rs ;
	                if (rs < 0) break ;
	            } /* end while */
	            if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	        } /* end if (querystring-cur) */
	        rs1 = querystring_finish(&ps) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (querystring) */
	    lip->final.svcs = (c > 0) ;
	} /* end if (allowed) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_qs) */


/* finalize argument objects */
static int locinfo_finalize(LOCINFO *lip)
{
	int		rs ;
	if ((rs = locinfo_svclistcount(lip)) > 0) {
	    lip->final.svcs = TRUE ;
	}
	return rs ;
}
/* end subroutine (finalize) */


static int locinfo_cookbegin(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	EXPCOOK		*clp = &lip->cooks ;
	int		rs ;

	if ((rs = expcook_start(clp)) >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    int		i ;
	    int		kch ;
	    int		vl ;
	    cchar	*ks = "PSNDHRUY" ;
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
	                vp = hbuf ;
	            }
	            break ;
	        case 'R':
	            vp = pip->pr ;
	            break ;
	        case 'U':
	            vp = pip->username ;
	            break ;
	        case 'Y':
	            {
	                const int	ylen = USERNAMELEN ;
	                char		ybuf[USERNAMELEN+1] ;
	                rs = ctdeci(ybuf,ylen,lip->year) ;
	                vl = rs ;
	                vp = ybuf ;
	            }
	            break ;
	        } /* end switch */
	        if ((rs >= 0) && (vp != NULL)) {
	            kbuf[0] = kch ;
	            rs = expcook_add(clp,kbuf,vp,vl) ;
	        }
	    } /* end for */

	    if (rs >= 0) {
	        lip->open.cooks = TRUE ;
	    } else {
	        expcook_finish(clp) ;
	    }
	} /* end if (expcook_start) */

	return rs ;
}
/* end subroutine (locinfo_cookbegin) */


static int locinfo_cookend(LOCINFO *lip)
{
	EXPCOOK		*clp = &lip->cooks ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->open.cooks) {
	    lip->open.cooks = FALSE ;
	    rs1 = expcook_finish(clp) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_cookend) */


static int locinfo_cookexp(LOCINFO *lip,int w,char *ebuf,int elen,
cchar *vbuf,int vl)
{
	EXPCOOK		*clp = &lip->cooks ;
	int		rs ;

	rs = expcook_exp(clp,w,ebuf,elen,vbuf,vl) ;

	return rs ;
}
/* end subroutine (locinfo_cookexp) */


/* configuration */
static int config_start(CONFIG *csp,PROGINFO *pip,PARAMOPT *app,cchar *cfname)
{
	const int	esize = sizeof(CONFIG_FILE) ;
	int		rs ;

	if (csp == NULL) return SR_FAULT ;
	if (app == NULL) return SR_FAULT ;

	memset(csp,0,sizeof(CONFIG)) ;
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

	if ((rs = vecobj_start(&csp->files,esize,2,0)) >= 0) {
	    if ((cfname != NULL) && (cfname[0] != '\0')) {
	        if ((rs = config_addfile(csp,cfname)) >= 0) {
	            rs = config_read(csp,cfname) ;
	        }
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
	    }
	    if (rs < 0)
	        vecobj_finish(&csp->files) ;
	} /* end if (vecobj_start) */

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

	rs1 = config_addfins(csp) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&csp->files) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (config_finish) */


static int config_addfile(CONFIG *csp,cchar *cfn)
{
	USTAT		sb ;
	int		rs ;
	if ((rs = u_stat(cfn,&sb)) >= 0) {
	    cchar	*cp ;
	    if ((rs = uc_mallocstrw(cfn,-1,&cp)) >= 0) {
	        CONFIG_FILE	f ;
	        f.fname = cp ;
	        f.mtime = sb.st_mtime ;
	        rs = vecobj_add(&csp->files,&f) ;
	        if (rs < 0)
	            uc_free(cp) ;
	    } /* end if (m-a) */
	} /* end if (stat) */
	return rs ;
}
/* end subroutine (config_addfile) */


static int config_addfins(CONFIG *csp)
{
	VECOBJ		*flp = &csp->files ;
	CONFIG_FILE	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	for (i = 0 ; (rs1 = vecobj_get(flp,i,&ep)) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs1 = uc_free(ep->fname) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */
	if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	return rs ;
}
/* end subroutine (config_addfins) */


static int config_check(CONFIG *csp)
{
	PROGINFO	*pip ;
	LOCINFO		*lip ;
	int		rs = SR_OK ;

	if (csp == NULL) return SR_FAULT ;

	if (csp->magic != CONFIG_MAGIC) return SR_NOTOPEN ;

	pip = csp->pip ;
	lip = pip->lip ;
	if (lip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("config_check: ent intconf=%u\n",lip->intconf) ;
#endif

	if (csp->f_p) {
	    USTAT		sb ;
	    VECOBJ		*flp = &csp->files ;
	    CONFIG_FILE	*ep ;
	    int		i ;
	    int		f = FALSE ;
	    for (i = 0 ; vecobj_get(flp,i,&ep) >= 0 ; i += 1) {
	        if (ep != NULL) {
	            if ((rs = u_stat(ep->fname,&sb)) >= 0) {
	                if (sb.st_mtime > ep->mtime) {
	                    ep->mtime = sb.st_mtime ;
	                    f = TRUE ;
	                }
	            } else if (isNotPresent(rs)) {
	                rs = SR_OK ;
	            }
	        } /* end if */
	        if (f) break ;
	        if (rs < 0) break ;
	    } /* end for */
	    if ((rs >= 0) && f) {
	        if ((rs = locinfo_svclistdelall(lip)) >= 0) {
	            cchar	*fn = ep->fname ;
	            for (i = 0 ; vecobj_get(flp,i,&ep) >= 0 ; i += 1) {
	                if ((rs = u_stat(fn,&sb)) >= 0) {
	                    rs = config_read(csp,fn) ;
	                } else if (isNotPresent(rs)) {
	                    rs = SR_OK ;
	                }
	                if (rs < 0) break ;
	            } /* end for */
	        } /* end if (locinfo_svclistdelall) */
	    } /* end if */
	} /* end if (active) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("config_check: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_check) */


static int config_load(CONFIG *csp,cchar *cfn)
{
	PROGINFO	*pip = csp->pip ;
	vecstr		sv ;
	int		rs ;
	int		rs1 ;

	if ((rs = vecstr_start(&sv,3,0)) >= 0) {
	    int		i ;
	    cchar	*pn = pip->progname ;
	    cchar	*sn = pip->searchname ;
	    cchar	*pr ;
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

	                if ((rs = config_addfile(csp,cbuf)) >= 0) {
	                    rs = config_read(csp,cbuf) ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("config_load: _read() rs=%d\n",rs) ;
#endif
	                }
	            }
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (vecstr_envset) */
	    rs1 = vecstr_finish(&sv) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (finding file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_load: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_load) */


static int config_havefile(CONFIG *csp,vecstr *svp,char cbuf[],cchar *pr,
cchar *cfname)
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


static int config_read(CONFIG *csp,cchar *cfname)
{
	PROGINFO	*pip = csp->pip ;
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
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (config_read) */


static int config_reader(CONFIG *csp,PARAMFILE *pfp)
{
	PROGINFO	*pip = csp->pip ;
	LOCINFO		*lip ;
	PARAMFILE_CUR	cur ;
	const int	vlen = VBUFLEN ;
	const int	elen = EBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		ml, vl, el ;
	int		v ;
	cchar		*pr = pip->pr ;
	char		vbuf[VBUFLEN + 1] ;
	char		ebuf[EBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_reader: ent f_p=%u\n",csp->f_p) ;
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
	                rs = locinfo_cookexp(lip,0,ebuf,elen,vbuf,vl) ;
	                el = rs ;
	            }

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("config_reader: ebuf=>%t<\n",ebuf,el) ;
#endif

	            if ((rs >= 0) && (el > 0)) {
	                cchar	*sn = pip->searchname ;
	                char	tbuf[MAXPATHLEN + 1] ;
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
	                        cchar	*lc = LOGCNAME ;
	                        cchar	*lfn = pip->lfname ;
	                        cchar	*tfn = tbuf ;
	                        pip->final.lfname = TRUE ;
	                        pip->have.lfname = TRUE ;
	                        ml = prsetfname(pr,tbuf,ebuf,el,TRUE,lc,sn,"") ;
	                        if ((lfn == NULL) || 
	                            (strcmp(lfn,tfn) != 0)) {
	                            cchar	**vpp = &pip->lfname ;
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
	                case cparam_copyright:
	                    if (lip->copyright == NULL) {
	                        if (el > 0) {
	                            rs = locinfo_copyright(lip,ebuf,el) ;
	                        }
	                    }
	                    break ;
	                case cparam_webmaster:
	                    if (lip->webmaster == NULL) {
	                        if (el > 0) {
	                            cchar	**vpp = &lip->webmaster ;
	                            rs = locinfo_setentry(lip,vpp,ebuf,el) ;
	                        }
	                    }
	                    break ;
	                case cparam_intrun:
	                    if (! pip->final.intrun) {
	                        if (el > 0) {
	                            rs = cfdecti(ebuf,el,&v) ;
	                            pip->intrun = v ;
	                        }
	                    }
	                    break ;
	                case cparam_intpoll:
	                    if (! pip->final.intpoll) {
	                        if (el > 0) {
	                            rs = cfdecti(ebuf,el,&v) ;
	                            pip->intpoll = v ;
	                        }
	                    }
	                    break ;
	                case cparam_intconf:
	                    if (! lip->final.intconf) {
	                        if (el > 0) {
	                            rs = cfdecti(ebuf,el,&v) ;
	                            lip->intconf = v ;
	                        }
	                    }
	                    break ;
	                case cparam_intidle:
	                    if (! pip->final.intidle) {
	                        if (el > 0) {
	                            rs = cfdecti(ebuf,el,&v) ;
	                            pip->intidle = v ;
	                        }
	                    }
	                    break ;
	                case cparam_intlock:
	                    if (! pip->final.intlock) {
	                        if (el > 0) {
	                            rs = cfdecti(ebuf,el,&v) ;
	                            pip->intlock = v ;
	                        }
	                    }
	                    break ;
	                case cparam_intmark:
	                    if (! pip->final.intmark) {
	                        if (el > 0) {
	                            rs = cfdecti(ebuf,el,&v) ;
	                            pip->intmark = v ;
	                        }
	                    }
	                    break ;
	                case cparam_intcache:
	                    if (! lip->final.intcache) {
	                        if (el > 0) {
	                            rs = cfdecti(ebuf,el,&v) ;
	                            lip->intcache = v ;
	                        }
	                    }
	                    break ;
	                case cparam_intwait:
	                    if (! lip->final.intwait) {
	                        if (el > 0) {
	                            rs = cfdecti(ebuf,el,&v) ;
	                            lip->intwait = v ;
	                        }
	                    }
	                    break ;
	                case cparam_intmaint:
	                    if (! lip->final.intmaint) {
	                        if (el > 0) {
	                            rs = cfdecti(ebuf,el,&v) ;
	                            lip->intmaint = v ;
	                        }
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


static int mkourname(PROGINFO *pip,char *rbuf,cchar *inter,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	cchar		*pr = pip->pr ;
	cchar		*sn = pip->searchname ;

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


static int mkincfname(PROGINFO *pip,char *ibuf,cchar *sp,int sl)
{
	int		rs ;
	if (sp[0] != '/') {
	    cchar	*pr = pip->pr ;
	    cchar	*sn = pip->searchname ;
	    cchar	*inter = ETCCNAME ;
	    rs = mkpath4w(ibuf,pr,inter,sn,sp,sl) ;
	} else {
	    rs = mkpath1w(ibuf,sp,sl) ;
	}
	return rs ;
}
/* end subroutine (mkincfname) */


