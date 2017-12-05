/* main */

/* generic (pretty much) front end program subroutine */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUG	0		/* switchable print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_DEBUGMAX	0		/* debug at maximum */
#define	CF_DEBUGARGZ	0		/* ? */
#define	CF_DEBUGENV	0		/* debug environment stuff */
#define	CF_DEBUGCOOKS	0		/* debug-cooks */
#define	CF_FIXENV	0		/* fix environment */
#define	CF_FIXARGZ	0		/* fix argz */
#define	CF_STAMPFNAME	1		/* observe a stamp file!? */
#define	CF_CHECKONC	0		/* check ONC */
#define	CF_PROGCONFIG	1		/* call 'progconfigxx()' */
#define	CF_PROCENVSYS	1		/* call 'procenvsys()' */
#define	CF_SYSVARENUM	1		/* enumerate sys-vars */


/* revision history:

	= 2008-09-01, David A­D­ Morano
	This subroutine was borrowed and modified from previous generic
	front-end 'main' subroutines!

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is the front-end for several a super-server programs.
	These server programs have widely differing functions, but they start
	by initializing in similar ways.  Hence this subroutine.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/systeminfo.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<grp.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<nulstr.h>
#include	<bfile.h>
#include	<ids.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<expcook.h>
#include	<varsub.h>
#include	<storebuf.h>
#include	<getax.h>
#include	<char.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#if	defined(P_PCSPOLL) && (P_PCSPOLL > 0)
#include	<pcsconf.h>
#endif

#include	"config.h"
#include	"defs.h"
#include	"sysvar.h"
#include	"envs.h"
#include	"listenspec.h"


/* local defines */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + REALNAMELEN)
#endif

#ifndef	ARCHBUFLEN
#define	ARCHBUFLEN	80
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	DEFNXENVS
#define	DEFNXENVS	40
#endif

#undef	DEBUGFNAME
#define	DEBUGFNAME	"/tmp/pcspoll.nd"


/* external subroutines */

extern int	snabbr(char *,int,const char *,int) ;
extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecl(const char *,int,long *) ;
extern int	ctdeci(char *,int,int) ;
extern int	ctdecl(char *,int,long) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	gethz(int) ;
extern int	getarchitecture(char *,int) ;
extern int	getnprocessors(const char **,int) ;
extern int	getproviderid(const char *,int) ;
extern int	getsystypenum(char *,char *,cchar *,cchar *) ;
extern int	getnodedomain(char *,char *) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	getserial(const char *) ;
extern int	mkuiname(char *,int,USERINFO *) ;
extern int	localgetorg(const char *,char *,int,const char *) ;
extern int	isasocket(int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	opendefstds(int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	vecstr_loadfile(vecstr *,int,const char *) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_rootname(PROGINFO *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	proginfo_nameidbegin(PROGINFO *) ;
extern int	proginfo_nameidend(PROGINFO *) ;

extern int	progpidbegin(PROGINFO *,int) ;
extern int	progpidcheck(PROGINFO *) ;
extern int	progpidend(PROGINFO *) ;

extern int	progstampcheck(PROGINFO *) ;
extern int	progpcsconf(PROGINFO *) ;
extern int	progcmd(PROGINFO *,ARGINFO *) ;
extern int	progpass(PROGINFO *,ARGINFO *) ;
extern int	progprocess(PROGINFO *,ARGINFO *,USERINFO *) ;

extern int	progconfigstart(PROGINFO *,const char **,const char *) ;
extern int	progconfigread(PROGINFO *) ;
extern int	progconfigcheck(PROGINFO *) ;
extern int	progconfigfinish(PROGINFO *) ;

extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;

extern int	defproc(vecstr *,const char **,EXPCOOK *,const char *) ;

extern int	envs_procxe(ENVS *,EXPCOOK *,cchar **,VECSTR *,cchar *) ;
extern int	envs_subs(ENVS *,EXPCOOK *,VECSTR *,VECSTR *) ;

extern int	securefile(const char *,uid_t,gid_t) ;
extern int	hasalldig(const char *,int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strnnlen(const char *,int,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern char	**environ ;


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;

static int	procschedbegin(PROGINFO *) ;
static int	procschedend(PROGINFO *) ;

static int	loadschedvars(PROGINFO *) ;
static int	loadgroupname(PROGINFO *) ;
static int	loadplatform(PROGINFO *) ;
static int	loadarchitecture(PROGINFO *) ;
static int	loadhz(PROGINFO *) ;
static int	loadncpu(PROGINFO *) ;
static int	loadorg(PROGINFO *) ;
static int	loadprovider(PROGINFO *) ;
static int	loadcooks(PROGINFO *) ;

static int	loaddefs(PROGINFO *,cchar *,cchar **,cchar **) ;
static int	loaddefsfile(PROGINFO *,cchar *) ;
static int	loaddefsfind(PROGINFO *,cchar **) ;
static int	loadxfile(PROGINFO *,cchar *) ;
static int	loadxsched(PROGINFO *,cchar **) ;
static int	loadpvars(PROGINFO *,cchar **,cchar *) ;
static int	loadpvarsdef(PROGINFO *,cchar **) ;
static int	loadsysinfo(PROGINFO *) ;

static int	procenvextra(PROGINFO *) ;
static int	procenvdef(PROGINFO *) ;
static int	procenvsysvar(PROGINFO *,const char *) ;
static int	procenvsort(PROGINFO *) ;

static int	pvarsbegin(PROGINFO *,const char **,const char *) ;
static int	pvarsend(PROGINFO *) ;

static int	isenvok(const char **,const char *,int) ;

#if	CF_DEBUG && CF_DEBUGCOOKS
static int	debugcooks(PROGINFO *,const char *) ;
#endif


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"LOGFILE",
	"CONFIG",
	"HELP",
	"sn",
	"df",
	"af",
	"ef",
	"cf",
	"lf",
	"xf",
	"pf",
	"passfd",
	"ra",
	"log",
	"cmd",
	"caf",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_tmpdir,
	argopt_version,
	argopt_verbose,
	argopt_logfile,
	argopt_config,
	argopt_help,
	argopt_sn,
	argopt_df,
	argopt_af,
	argopt_ef,
	argopt_cf,
	argopt_lf,
	argopt_xf,
	argopt_pf,
	argopt_passfd,
	argopt_ra,
	argopt_log,
	argopt_cmd,
	argopt_caf,
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
	{ SR_NOEXEC, EX_NOEXEC },
	{ SR_LIBACC, EX_NOEXEC },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static cchar	*akonames[] = {
	"cf",
	"lf",
	"log",
	"sd",
	"pollint",
	"ra",
	"reuseaddr",
	"showsysbanner",
	"uniq",
	"quiet",
	"tmptype",
	NULL
} ;

enum akonames {
	akoname_cf,
	akoname_lf,
	akoname_log,
	akoname_sd,
	akoname_intpoll,
	akoname_ra,
	akoname_reuseaddr,
	akoname_showsysbanner,
	akoname_uniq,
	akoname_quiet,
	akoname_tmptype,
	akoname_overlast
} ;

static cchar	*cooks[] = {
	"SYSNAME",	/* OS system-name */
	"RELEASE",	/* OS system-release */
	"VERSION",	/* OS system-version */
	"MACHINE",	/* machine-name */
	"PLATFORM",	/* pathform */
	"ARCHITECTURE",	/* machine-architecture */
	"NCPU",		/* number of machine CPUs */
	"HZ",		/* OS clock tics */
	"U",		/* current user username */
	"G",		/* current user groupname */
	"HOME",		/* current user home directory */
	"SHELL",	/* current user shell */
	"ORGANIZATION",	/* current user organization name */
	"GECOSNAME",	/* current user gecos-name */
	"REALNAME",	/* current user real-name */
	"NAME",		/* current user name */
	"TZ",		/* current user time-zone */
	"N",		/* nodename */
	"D",		/* INET domainname (for current user) */
	"H",		/* INET hostname */
	"R",		/* program-root */
	"RN",		/* program-root-name */
	"OSTYPE",
	"OSNUM",
	"S",		/* searchname */
	"O",		/* organization */
	"OO",		/* organization w/ hyphens */
	"OC",		/* org-code */
	"V",		/* program version */
	NULL
} ;

enum cooks {
	cook_sysname,
	cook_release,
	cook_version,
	cook_machine,
	cook_platform,
	cook_architecture,
	cook_ncpu,
	cook_hz,
	cook_u,
	cook_g,
	cook_home,
	cook_shell,
	cook_organization,
	cook_gecosname,
	cook_realname,
	cook_name,
	cook_tz,
	cook_n,
	cook_d,
	cook_h,
	cook_r,
	cook_rn,
	cook_ostype,
	cook_osnum,
	cook_s,
	cook_o,
	cook_oo,
	cook_oc,
	cook_v,
	cook_overlast
} ;

static cchar	*schedpconf[] = {
	"%p/etc/%n/%n.%f",
	"%p/etc/%n/%f",
	"%p/etc/%n.%f",
	NULL
} ;

static cchar	*schedpfile[] = {
	"%p/etc/%n/%n.%f",
	"%p/etc/%n/%f",
	"%p/etc/%n.%f",
	"%p/etc/%f",
	"%p/%n.%f",
	NULL
} ;

static cchar	*schedhfile[] = {
	"%h/etc/%n/%n.%f",
	"%h/etc/%n/%f",
	"%h/etc/%n.%f",
	"%h/etc/%f",
	"%h/%n.%f",
	NULL
} ;

static cchar	*pathvars[] = {
	"PATH",
	"LD_LIBRARY_PATH",
	"MANPATH",
	"FPATH",
	"CDPATH",
	"XFILESEARCHPATH",
	"XUSERFILESEARCHPATH",
	"MAILDIR",
	"MAILDIRS",
	NULL
} ;

static cchar	*envbads[] = {
	"_",
	"_A0",
	"_EF",
	"A__z",
	"RANDOM",
	"TMOUT",
	NULL
} ;

static cchar	*envdefs[] = {
	"PLATFORM",
	"ARCHITECTURE",
	"SYSNAME",
	"RELEASE",
	"VERSION",
	"MACHINE",
	"HZ",
	"NODE",
	"NCPU",
	"USERNAME",
	"GROUPNAME",
	"DOMAIN",
	NULL
} ;

enum envdefs {
	envdef_platform,
	envdef_architecture,
	envdef_overlast
} ;

static cchar	*tmptypes[] = {
	"system",
	"user",
	NULL
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	KEYOPT		akopts ;
	USERINFO	u ;
	bfile		errfile ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		v ;
	int		cl ;
	int		opts ;
	int		size ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*dfname = NULL ;
	const char	*xfname = NULL ;
	const char	*pvfname = NULL ;
	const char	*cp ;

	char		buf[BUFLEN + 2] ;
	char		tmpfname[MAXPATHLEN + 2] ;

#if	CF_DEBUGS && CF_DEBUGARGZ
	nprintf(DEBUGFNAME,"argz(%p)\n",argv[0]) ;
	nprintf(DEBUGFNAME,"argz=>%t<\n",
	    argv[0],strlinelen(argv[0],-1,40)) ;
#endif

#if	CF_FIXENV
	{
	    uint	a ;
	    int		ps = getpagesize() ;
	    for (i = 0 ; envv[i] != NULL ; i += 1) {
	        a = (uint) envv[i] ;
	        if (a < (2 * ps)) {
		    envv[i] = "*DELETED*=" ;
		}
	    } /* end for */
	}
#endif /* CF_FIXENV */

	opendefstds(3) ;

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

	cp = argv[0] ;
#if	CF_FIXARGZ
	if (cp != NULL) {
	    uint	uch = cp[0] && 0xff ;
	    if (uch >= 128) cp = SEARCHNAME ;
	}
#endif /* CF_FIXARGZ */

	memset(&ainfo,0,sizeof(ARGINFO)) ;

	rs = proginfo_start(pip,envv,cp,VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->debuglevel = CF_DEBUGMAX ;
	pip->verboselevel = 1 ;
	pip->intmin = -1 ;
	pip->intpoll = -1 ;	/* program check interval */
	pip->intrun = 0 ;	/* regular mode requires '0' here */
	pip->maxjobs = MAXJOBS ;
	pip->fd_pass = -1 ;
	pip->gid_rootname = -1 ;
	pip->daytime = time(NULL) ;
	pip->hostid = (uint) gethostid() ;

	pip->f.defsvc = TRUE ;
	pip->f.logprog = TRUE ;		/* default logging */

#if	defined(P_PCSPOLL) && (P_PCSPOLL > 0)
	pip->f.defpidlock = TRUE ;
	pip->f.stampfname = TRUE ;
	pip->f.rundname = FALSE ;
#endif

#if	defined(P_FINGERS) && (P_FINGERS > 0)
	pip->f.loginsvc = TRUE ;
	pip->f.useracct = TRUE ;	/* match on user-accounts */
#endif

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&ainfo.pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	rs = keyopt_start(&pip->cmds) ;
	pip->open.cmds = (rs >= 0) ;

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
	        const int ach = MKCHAR(argp[1]) ;

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

/* configuration file */
	                case argopt_config:
	                case argopt_cf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->cfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* help */
	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

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

	                case argopt_logfile:
	                case argopt_lf:
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
	                            if (argl) {
	                                cp = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    } /* end if */
	                    if ((rs >= 0) && (cp != NULL)) {
	                        pip->have.lfname = TRUE ;
	                        pip->final.lfname = TRUE ;
	                        pip->f.lfname = TRUE ;
	                        pip->lfname = cp ;
	                    }
	                    break ;

	                case argopt_df:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            dfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                dfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* argument-list file */
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

/* export file */
	                case argopt_xf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            xfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                xfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* path-vars file */
	                case argopt_pf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pvfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pvfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* pass-FD */
	                case argopt_passfd:
	                    pip->f.passfd = TRUE ;
	                    pip->fd_pass = FD_STDIN ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
	                            pip->fd_pass = rs ;
	                        }
	                    }
	                    break ;

/* reuse-address */
	                case argopt_ra:
	                    pip->have.reuseaddr = TRUE ;
	                    pip->final.reuseaddr = TRUE ;
	                    pip->f.reuseaddr = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.reuseaddr = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* logging? */
	                case argopt_log:
	                    pip->have.logprog = TRUE ;
	                    pip->final.logprog = TRUE ;
	                    pip->f.logprog = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.logprog = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* command */
	                case argopt_cmd:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            KEYOPT	*kop = &pip->cmds ;
	                            pip->f.cmd = TRUE ;
	                            rs = keyopt_loads(kop,argp,argl) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* close-all-files */
	                case argopt_caf:
	                    pip->f.caf = TRUE ;
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

/* configuration file */
	                    case 'C':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
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

/* mutex lock PID file */
	                    case 'P':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->have.pidfname = TRUE ;
	                                pip->final.pidfname = TRUE ;
	                                pip->pidfname = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* background mode */
	                    case 'b':
	                        pip->f.background = TRUE ;
	                        break ;

/* daemon mode */
	                    case 'd':
	                        pip->f.daemon = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                pip->final.intrun = TRUE ;
	                                pip->have.intrun = TRUE ;
	                                pip->intrun = -1 ;
	                                if (avp[0] != '-') {
	                                    rs = cfdecti(avp,avl,&v) ;
	                                    pip->intrun = v ;
	                                }
	                            }
	                        }
	                        break ;

/* force a run */
	                    case 'f':
	                        pip->f.force = TRUE ;
	                        pip->final.intmin = TRUE ;
	                        pip->have.intmin = TRUE ;
	                        pip->intmin = 0 ;
	                        break ;

/* minimum interval */
	                    case 'm':
	                        pip->have.intmin = TRUE ;
	                        pip->final.intmin = TRUE ;
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
	                                pip->intmin = v ;
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

/* poll interval */
	                    case 'p':
	                        pip->have.intpoll = TRUE ;
	                        pip->final.intpoll = TRUE ;
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
	                                pip->intpoll = v ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* verbose mode off */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* uniq mode (check our PID mutex w/o error message) */
	                    case 'u':
	                        pip->f.uniq = TRUE ;
	                        pip->have.uniq = TRUE ;
	                        pip->final.uniq = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.uniq = (rs > 0) ;
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

	        rs = bits_set(&ainfo.pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

/* get our program root */

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
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* go */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(2)) {
	    for (i = 0 ; i < 3 ; i += 1) {
	        debugprintf("main: isasocket(%u)=%u\n",i,isasocket(i)) ;
	        debugprintf("main: fd=%u u_fstat() rs=%d s_issock=%u\n",
	            i, u_fstat(i,&sb),S_ISSOCK(sb.st_mode)) ;
	    }
	}
#endif /* CF_DEBUG */

	if ((rs >= 0) && (pip->intpoll <= 0) && (argval != NULL)) {
	    pip->final.intpoll = TRUE ;
	    pip->have.intpoll = TRUE ;
	    rs = cfdecti(argval,-1,&v) ;
	    pip->intpoll = v ;
	}

	if (rs >= 0) {
	    if ((rs = proginfo_pwd(pip)) >= 0) {
	        if ((rs = proginfo_rootname(pip)) >= 0) {
	            rs = procopts(pip,&akopts) ;
	        }
	    }
	}

#if	CF_CHECKONC
	if (rs >= 0) {
	    if ((rs = checkonc(pip->pr,NULL,NULL,NULL)) >= 0) {
		pip->f.onckey = TRUE ;
	    }
	}
#endif /* CF_CHECKONC */

/* get some host-user information (offensive to ONC secure operations!) */

	if ((rs >= 0) && (pip->linelen < 7)) {
	    cp = getenv(VARLINELEN) ;
	    if (cp == NULL) cp = getenv(VARCOLUMNS) ;
	    if (cp != NULL) {
	        if ((rs = optvalue(cp,-1)) >= 0) {
		    if (rs > 7) {
	                pip->have.linelen = TRUE ;
	                pip->final.linelen = TRUE ;
	                pip->linelen = rs ;
		    }
	        }
	    }
	}

	if (pip->linelen < rs1) pip->linelen = COLUMNS ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badopts ;
	}

/* continue w/ primary initialization */

	rs = proginfo_nameidbegin(pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badidbegin ;
	}

	rs = userinfo_start(&u,NULL) ;
	if (rs < 0) {
	    ex = EX_NOUSER ;
	    bprintf(pip->efp,
	        "%s: userinfo failure (%d)\n",
	        pip->progname,rs) ;
	    goto baduserstart ;
	}
	pip->uip = &u ;

	pip->usysname = u.sysname ;
	pip->urelease = u.release ;
	pip->uversion = u.version ;
	pip->umachine = u.machine ;
	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;
	pip->homedname = u.homedname ;
	pip->shell = u.shell ;
	pip->org = u.organization ;
	pip->gecosname = u.gecosname ;
	pip->realname = u.realname ;
	pip->name = u.name ;
	pip->fullname = u.fullname ;
	pip->mailname = u.mailname ;
	pip->maildname = u.md ;
	pip->tz = u.tz ;
	pip->logid = u.logid ;

	pip->pid = u.pid ;
	pip->ppid = u.pid ;
	pip->spid = u.pid ;

	pip->uid = u.uid ;
	pip->euid = u.euid ;

	pip->gid = u.gid ;
	pip->egid = u.egid ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: version=%s\n",pip->uversion) ;
#endif

	if (rs >= 0) {
	    rs = loadsysinfo(pip) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: mid7\n") ;
#endif

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto baduserstart ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: org=%s\n",pip->org) ;
#endif

	if ((pip->orgcode == NULL) || (pip->orgcode[0] == '\0')) {
	    const int	oclen = ORGCODELEN ;
	    const char	*org = pip->org ;
	    rs1 = -1 ;
	    if ((org != NULL) && (org[0] != '\0')) {
	        rs1 = snabbr(pip->orgcode,oclen,org,-1) ;
	    }
#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: snabbr() rs1=%d orgcode=%s\n",
	            rs1,pip->orgcode) ;
#endif
	    if (rs1 < 0)
	        strwcpy(pip->orgcode,ORGCODE,oclen) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: basic init done rs=%d\n",rs) ;
#endif

/* initialize and load up the cookies */

	rs = expcook_start(&pip->cooks) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: expcook_start() rs=%d\n",rs) ;
#endif

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badcookstart ;
	}

	rs = loadcooks(pip) ;

#if	CF_DEBUG && CF_DEBUGCOOKS
	if (DEBUGLEVEL(2))
	    debugcooks(pip,"lookcooks") ;
#endif

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badcookload ;
	}

/* reset our effect UID if we are: a) SUID and b) sn=poll */

#if	defined(P_PCSPOLL) && (P_PCSPOLL > 0)
	pip->f.proglocal = (pip->searchname != NULL) && 
	    (strcmp(pip->searchname,"poll") == 0) ;

	if (pip->f.proglocal && (pip->uid != pip->euid))
	    u_seteuid(pip->uid) ;

	if (pip->f.proglocal && (pip->gid != pip->egid))
	    u_setegid(pip->gid) ;
#endif /* P_PCSPOLL */

/* make a best name */

#ifdef	COMMENT
	if (rs >= 0) {
	    char	rnamebuf[REALNAMELEN + 1] ;
	    rs1 = mkuiname(rnamebuf,REALNAMELEN,&u) ;
	    if (rs1 >= 0) {
	        cchar	**vpp = &pip->bestname ;
	        rs = proginfo_setentry(pip,vpp,rnamebuf,rs1) ;
	    }
	} /* end block */
#endif /* COMMENT */

/* root secure? */

	if (rs >= 0) {

	    rs1 = securefile(pip->pr,pip->euid,pip->egid) ;
	    pip->f.secure_root = (rs1 > 0) ;

	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: secure_root=%u\n",
	            pip->progname,pip->f.secure_root) ;
	    }

/* make the hostname */

	    if ((rs1 = snsds(buf,BUFLEN,pip->nodename,pip->domainname)) >= 0) {
	        cchar	**vpp = &pip->hostname ;
	        proginfo_setentry(pip,vpp,buf,rs1) ;
	    }

	} /* end if */

/* create the values for the schedule-file searching */

	if (rs >= 0)
	rs = procschedbegin(pip) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: procschedbegin() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badschedinit ;

/* listener specifications */

	size = sizeof(LISTENSPEC) ;
	rs = vecobj_start(&pip->listens,size,5,0) ;
	if (rs < 0)
	    goto badlisteninit ;

/* configuration file */

	if (pip->cfname == NULL) pip->cfname = getenv(VARCONF) ;
	if (pip->cfname == NULL) pip->cfname = CONFFNAME ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: progconfigstart() cf=%s\n",pip->cfname) ;
#endif

#if	CF_PROGCONFIG
	rs = progconfigstart(pip,schedpconf,pip->cfname) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: progconfigstart() rs=%d\n",rs) ;
	    debugprintf("main: svcfname=%s\n",pip->svcfname) ;
	}
#endif

	if ((pip->debuglevel > 0) && (pip->cfname != NULL)) {
	    bprintf(pip->efp,"%s: cf=%s (%d)\n",
	        pip->progname,pip->cfname,rs) ;
	}

	if (rs < 0)
	    goto badconfiginit ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: 1 progconfigstart() \n") ;
	    debugprintf("main: pidfname=%s\n",pip->pidfname) ;
	}
#endif

/* check the system PCS configuration */

#if	defined(P_PCSPOLL) && (P_PCSPOLL > 0)
	rs = progpcsconf(pip) ;
	if (rs < 0)
	    goto badpcsconf ;
#endif /* P_PCSPOLL */

/* end of accessing the configuration file */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* poll interval */

	if (pip->intpoll <= 0) pip->intpoll = TO_POLLSVC ;

	if (pip->intmin < 0) pip->intmin = TO_MINCHECK ;

	if (pip->intrun < 0) pip->intrun = TO_RUN ;

#ifdef	COMMENT
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: poll interval=%d\n",
	        pip->progname,pip->intpoll) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: intpoll=%u\n",pip->intpoll) ;
#endif

	if (pip->to_recvfd < 1) pip->to_recvfd = TO_RECVFD ;
	if (pip->to_sendfd < 1) pip->to_sendfd = TO_SENDFD ;

/* maximum number of jobs */

	if (pip->maxjobs <= 0) pip->maxjobs = MAXJOBS ;

/* time-stamp directory */

#if	defined(P_PCSPOLL) && (P_PCSPOLL > 0)
	if (rs >= 0) {

	    cp = pip->stampdname ;
	    cl = -1 ;
	    if ((cp == NULL) || (strcmp(cp,"+") == 0)) {
	        cp = STAMPDNAME ;
	        cl = -1 ;
	    }

	    if (cp[0] != '/') {
	        if (strchr(cp,'/') != NULL) {
	            cl = mkpath2(tmpfname,pip->pr,cp) ;
	        } else {
	            cl = mkpath3(tmpfname,pip->pr,VARDNAME,cp) ;
	        }
	        cp = tmpfname ;
	    }

	    if (cp != NULL) {
	        cchar	**vpp = &pip->stampdname ;
	        rs = proginfo_setentry(pip,vpp,cp,cl) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: stampdname=%s cl=%d rs=%d\n",
	            pip->stampdname,cl,rs) ;
#endif

	} /* end if (stampdname) */
#endif /* P_PCSPOLL */

/* working directory */

	if (rs >= 0) {

	    cp = pip->workdname ;
	    cl = -1 ;
	    if ((cp == NULL) || (strcmp(cp,"+") == 0)) {
	        cp = WORKDNAME ;
	        cl = -1 ;
	    }

	    if (cp[0] != '/') {
	        if (strchr(cp,'/') != NULL) {
	            cl = mkpath2(tmpfname,pip->pr,cp) ;
	        } else {
	            cl = mkpath3(tmpfname,pip->pr,VARDNAME,cp) ;
	        }
	        cp = tmpfname ;
	    }

	    if (cp != NULL) {
	        cchar	**vpp = &pip->workdname ;
	        rs = proginfo_setentry(pip,vpp,cp,cl) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: 1 workdname=%s cl=%d rs=%d\n",
	            pip->workdname,cl,rs) ;
#endif

	} /* end if (workdname) */

/* run directory */

	if ((rs >= 0) && pip->f.rundname) {

	    cp = pip->rundname ;
	    cl = -1 ;
	    if ((cp == NULL) || (strcmp(cp,"+") == 0)) {
	        cp = RUNDNAME ;
	        cl = -1 ;
	    }

	    if (cp[0] != '/') {
	        if (strchr(cp,'/') != NULL) {
	            cl = mkpath2(tmpfname,pip->pr,cp) ;
	        } else {
	            cl = mkpath3(tmpfname,pip->pr,VARDNAME,cp) ;
	        }
	        cp = tmpfname ;
	    }

	    if (cp != NULL) {
	        cchar	**vpp = &pip->rundname ;
	        rs = proginfo_setentry(pip,vpp,cp,cl) ;
	    }

	} /* end if (rundname) */

/* check the directories */

#if	defined(P_PCSPOLL) && (P_PCSPOLL > 0)
	if (rs >= 0) {
	    cp = pip->stampdname ;
	    if (rs >= 0) {
	        const int	am = (R_OK | W_OK | X_OK) ;
	        rs = perm(cp,-1,-1,NULL,am) ;
	        if (rs == SR_NOENT) {
	            mode_t	oldmask ;
	            oldmask = umask(0000) ;
	            rs = mkdirs(cp,0777) ;
	            umask(oldmask) ;
	        } /* end if (making directory) */
	    } /* end if */
#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: stampdname=%s rs=%d\n",
	            pip->stampdname,rs) ;
#endif
	} /* end if (stmpdname) */
#endif /* P_PCSPOLL */

	if (rs >= 0) {

	    cp = pip->workdname ;
	    if (rs >= 0) {
	        const int	am = (R_OK | W_OK | X_OK) ;
	        rs = perm(cp,-1,-1,NULL,am) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: 2 workdname=%s rs=%d\n",
	            pip->workdname,rs) ;
#endif

	} /* end if (workdname) */

	if ((rs >= 0) && (pip->rundname != NULL)) {

	    cp = pip->rundname ;
	    if (rs >= 0) {
	        const int	am = (R_OK | W_OK | X_OK) ;

	        rs = perm(cp,-1,-1,NULL,am) ;

	        if (rs == SR_NOENT) {
	            mode_t	oldmask ;
	            oldmask = umask(0000) ;
	            rs = mkdirs(cp,0777) ;
	            umask(oldmask) ;
	        } /* end if (making directory) */

	    } /* end if */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: rundname=%s rs=%d\n",
	            pip->rundname,rs) ;
#endif

	} /* end if (rundname) */

	if (rs < 0)
	    goto baddirs ;

/* do we have named services */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&ainfo.pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        cp = argv[ai] ;
	        if ((cp != NULL) && (cp[0] != '\0')) {
	            pip->svcpass = cp ;
	            pip->f.named = TRUE ;
	            break ;
		}
	    }

	} /* end for */

	if (pip->f.daemon)
	    pip->f.named = FALSE ;

/* do we have a stamp-file for ourselves? */

#if	defined(P_PCSPOLL) && (P_PCSPOLL > 0)
	cp = pip->stampfname ;
	cl = -1 ;
	if ((cp == NULL) || (cp[0] == '+')) {
	    cp = pip->progname ;
	    cl = -1 ;
	}

	if (cp[0] != '/') {
	    if (strchr(cp,'/') != NULL) {
	        cl = mkpath2(tmpfname,pip->pr,cp) ;
	    } else {
	        cl = mkpath2(tmpfname,pip->stampdname,cp) ;
	    }
	    cp = tmpfname ;
	}

	if (cp != NULL) {
	    cchar	**vpp = &pip->stampfname ;
	    rs = proginfo_setentry(pip,vpp,cp,cl) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: stampfname=%s rs=%d\n",
	        pip->stampfname,rs) ;
#endif

	if (rs < 0)
	    goto badstamp ;

/* now check if we need any servicing (only for unnamed services) */

#if	CF_STAMPFNAME
	if ((rs >= 0) && pip->f.stampfname) {

#ifdef	COMMENT
	    if (pip->open.log)
	        logfile_printf(&pip->lh,"sd=%s",pip->stampfname) ;
	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: sf=%s\n",pip->progname,pip->stampfname) ;
#endif /* COMMENT */

	    f = FALSE ;
	    if (! pip->f.named) {
	        rs = progstampcheck(pip) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: progstampcheck() rs=%d\n",rs) ;
#endif
	        f = (rs > 0) ;
	    }

	    if (rs < 0)
	        goto badcheck ;	/* error */

	    if ((! pip->f.named) && (! pip->f.daemon) && (! f)) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: no processing needed\n") ;
#endif
	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: no processing needed\n",
	                pip->progname) ;
	        goto retcheck ;	/* return OK, no processing needed */
	    }

	} /* end if (stampfname) */
#endif /* CF_STAMPFNAME */

#endif /* P_PCSPOLL */

/* before we go too far, are we the only one on this PID mutex? */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: pid-lock check\n") ;
#endif

	if ((pip->f.defpidlock && (! pip->f.named)) || pip->f.daemon) {
	    if ((rs = progpidbegin(pip,0)) >= 0) {

	        if ((pip->debuglevel > 0) && (pip->pidfname != NULL)) {
	            bprintf(pip->efp,"%s: pid=%s\n",
	                pip->progname,pip->pidfname) ;
		}

	        progpidend(pip) ;		/* release it for the child */
	    } /* end if */
	} /* end if (PID mutex) */
	if (rs < 0) {
	    if ((! pip->f.quiet) || (pip->debuglevel > 0)) {
	        bprintf(pip->efp,"%s: (1) PID mutex busy (%d)\n",
	            pip->progname,rs) ;
	    }
	    goto badpidcapture ;
	}

/* load up definitions and environment */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: load defs\n") ;
#endif

	rs = vecstr_start(&pip->defs,DEFNDEFS,0) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto baddefsinit ;
	}

	rs = loaddefs(pip,dfname,schedhfile,schedpfile) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: loaddefs() rs=%d\n",rs) ;
#endif

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto baddefsload ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: defs loaded\n",pip->progname) ;
	}

	vecstr_sort(&pip->defs,vstrkeycmp) ;

	pip->f.secure_path = pip->f.secure_root ;
	rs = envs_start(&pip->xenvs,DEFNXENVS) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badenvsinit ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: envs_start() rs=%d\n",rs) ;
#endif

/* load up the environment variables */

	if (rs >= 0) {
	    rs = loadxsched(pip,schedpfile) ;
	    if (pip->debuglevel > 0) {
		cchar	*pn = pip->progname ;
		bprintf(pip->efp,"%s: p-sched loaded (%d)\n",pn,rs) ;
	    }
	}

#ifdef	COMMENT
	if (rs >= 0) {
	    rs = loadxsched(pip,schedhfile) ;
	}
#endif

	if ((rs >= 0) && (xfname != NULL) && (xfname[0] != '\0')) {
	    rs = loadxfile(pip,xfname) ;
	    if (pip->debuglevel > 0) {
		cchar	*pn = pip->progname ;
		bprintf(pip->efp,"%s: xfile loaded (%d)\n",pn,rs) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badenvsload ;
	}

	opts = VECSTR_OCOMPACT ;
	rs = vecstr_start(&pip->exports,DEFNXENVS,opts) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badxinit ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: pvarsbegin() \n") ;
#endif

	rs = pvarsbegin(pip,pathvars,pvfname) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badpvarsbegin ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: envsubs() \n") ;
#endif

	rs = envs_subs(&pip->xenvs,&pip->cooks,&pip->pvars,&pip->exports) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badenvsubs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: procenvextra() \n") ;
#endif

	    if (pip->debuglevel > 0) {
		cchar	*pn = pip->progname ;
		bprintf(pip->efp,"%s: envs substituted (%d)\n",pn,rs) ;
	    }

	if (rs >= 0) {
	    rs = procenvextra(pip) ;
	    if (pip->debuglevel > 0) {
		cchar	*pn = pip->progname ;
		bprintf(pip->efp,"%s: extra envs processed (%d)\n",pn,rs) ;
	    }
	}

	if (rs >= 0)
	    rs = procenvdef(pip) ;

#if	CF_PROCENVSYS
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: procenvsysvar() \n") ;
#endif
	if (rs >= 0)
	    rs = procenvsysvar(pip,NULL) ;
#endif /* CF_PROCENVSYS */

	if (rs >= 0) {
	    rs = procenvsort(pip) ;
	    if (pip->debuglevel > 0) {
		cchar	*pn = pip->progname ;
		bprintf(pip->efp,"%s: envs sorted (%d)\n",pn,rs) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprocenvsysvar ;
	}

/* prepare to spawn the child (much work left to do) */

	ainfo.argc = argc ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;
	if (rs >= 0) {
	    if ((rs = proguserlist_begin(pip)) >= 0) {
	        if (pip->f.cmd) {
	            rs = progcmd(pip,&ainfo) ;
	        } else if (pip->f.passfd) {
	            rs = progpass(pip,&ainfo) ;
	        } else {
	            rs = progprocess(pip,&ainfo,&u) ;
	        } /* end if */
	        rs1 = proguserlist_end(pip) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (proguserlist) */
	} /* end if (ok) */

	    if (pip->debuglevel > 0) {
		cchar	*pn = pip->progname ;
		bprintf(pip->efp,"%s: completed (%d)\n",pn,rs) ;
	    }

/* get out */
badprocenvsysvar:
badenvsubs:
	pvarsend(pip) ;

badpvarsbegin:
	vecstr_finish(&pip->exports) ;

badxinit:
badenvsload:
	envs_finish(&pip->xenvs) ;

badenvsinit:
baddefsload:
	vecstr_finish(&pip->defs) ;

baddefsinit:
badpidcapture:
retcheck:
badcheck:
badstamp:
baddirs:
badpcsconf:
	progconfigfinish(pip) ;

badconfiginit:
	vecobj_finish(&pip->listens) ;

badlisteninit:
	procschedend(pip) ;

badschedinit:
badcookload:
	expcook_finish(&pip->cooks) ;

badcookstart:
	pip->uip = NULL ;
	userinfo_finish(&u) ;

baduserstart:
	proginfo_nameidend(pip) ;

badidbegin:
badopts:
/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,"%s: invalid argument (%d)\n",
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
#ifdef	COMMENT
	} else if (if_exit) {
	    ex = EX_TERM ;
	} else if (if_int) {
	    ex = EX_INTR ;
#endif /* COMMENT */
	} /* end if */

retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	if (pip->open.cmds) {
	    pip->open.cmds = FALSE ;
	    keyopt_finish(&pip->cmds) ;
	}

	bits_finish(&ainfo.pargs) ;

badpargs:
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
	        const char	*ids = "main" ;
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
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


#if	CF_DEBUGS || CF_DEBUG
int progexports(PROGINFO *pip,cchar *s)
{
	int		i ;
	const char	*cp ;
	debugprintf("main/progexports: elp={%p}\n",&pip->exports) ;
	for (i = 0 ; vecstr_get(&pip->exports,i,&cp) >= 0 ; i += 1) {
	    if (cp == NULL) continue ;
	    debugprintf("main/progexports: %s e=>%t<\n",
	        s,cp,strlinelen(cp,-1,40)) ;
	}
	return SR_OK ;
}
#endif /* CF_DEBUGS */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<svc(s)> ...]] [-C <conf>] [-d[=<runtime>]]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-f] [-v] [-<interval>] [-m <mincheck>] [-o <opt(s)>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getenv(VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {
	            if ((oi = matostr(akonames,3,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case akoname_cf:
			    if (! pip->final.cfname) {
			        pip->final.cfname = TRUE ;
				if (vl > 0) {
				    cchar	**vpp = &pip->cfname ;
				    rs = proginfo_setentry(pip,vpp,vp,vl) ;
				}
			    }
			    break ;
	                case akoname_lf:
			    if (! pip->final.lfname) {
			        pip->final.lfname = TRUE ;
				if (vl > 0) {
				    cchar	**vpp = &pip->lfname ;
				    rs = proginfo_setentry(pip,vpp,vp,vl) ;
				}
			    }
			    break ;
	                case akoname_log:
	                    if (! pip->final.logprog) {
	                        pip->have.logprog = TRUE ;
	                        pip->f.logprog = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.logprog = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_intpoll:
	                    if (! pip->final.intpoll) {
	                        if (vl > 0) {
	                            pip->have.intpoll = TRUE ;
	                            rs = optvalue(vp,vl) ;
	                            pip->intpoll = rs ;
	                        }
	                    }
	                    break ;
	                case akoname_sd:
	                    break ;
	                case akoname_ra:
	                case akoname_reuseaddr:
	                    if (! pip->final.reuseaddr) {
	                        pip->have.reuseaddr = TRUE ;
	                        pip->f.reuseaddr = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.reuseaddr = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_showsysbanner:
	                    if (! pip->final.showsysbanner) {
	                        pip->have.showsysbanner = TRUE ;
	                        pip->f.showsysbanner = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.showsysbanner = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_uniq:
	                    if (! pip->final.uniq) {
	                        pip->have.uniq = TRUE ;
	                        pip->f.uniq = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.uniq = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_quiet:
	                    if (! pip->final.quiet) {
	                        pip->have.quiet = TRUE ;
	                        pip->f.quiet = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.quiet = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_tmptype:
	                    if (! pip->final.tmptype) {
	                        pip->have.tmptype = TRUE ;
	                        pip->final.tmptype = TRUE ;
	                        if (vl > 0) {
	                            int	tt = matpstr(tmptypes,1,vp,vl) ;
	                            if (tt >= 0) pip->tmptype = tt ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procopts: out-loop rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procschedbegin(PROGINFO *pip)
{
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procschedbegin: ent\n") ;
#endif

	if ((rs = vecstr_start(&pip->svars,6,0)) >= 0) {
	    pip->open.svars = TRUE ;
	    rs = loadschedvars(pip) ;
	    c = rs ;
	    if (rs < 0) {
	        pip->open.svars = FALSE ;
	        vecstr_finish(&pip->svars) ;
	    }
	} /* end if (svars) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procschedbegin: ret rs=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procschedbegin) */


static int procschedend(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->open.svars) {
	    pip->open.svars = FALSE ;
	    rs1 = vecstr_finish(&pip->svars) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (procschedend) */


static int procenvextra(PROGINFO *pip)
{
	NULSTR		ns ;
	vecstr		*elp = &pip->exports ;
	int		rs = SR_OK ;
	int		i ;
	int		kl, vl ;
	int		c = 0 ;
	const char	*tp, *kp, *vp ;
	const char	*kname ;

	for (i = 0 ; pip->envv[i] != NULL ; i += 1) {

	    kp = pip->envv[i] ;
	    kl = -1 ;
	    vp = NULL ;
	    vl = 0 ;
	    if ((tp = strchr(kp,'=')) != NULL) {
	        kl = (tp - kp) ;
	        vp = (tp + 1) ;
	        vl = -1 ;
	    }

#if	CF_DEBUG && CF_DEBUGENV
	    if (DEBUGLEVEL(4)) {
	        debugprintf("main/procenvextra: env can=%t\n",kp,kl) ;
	        debugprintf("main/procenvextra: env v=>%t<\n",
	            vp,strlinelen(vp,vl,40)) ;
	    }
#endif

	    if ((vp != NULL) && isenvok(envbads,kp,kl)) {

	        if ((rs = nulstr_start(&ns,kp,kl,&kname)) >= 0) {

#if	CF_DEBUG && CF_DEBUGENV
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procenvextra: env add=%s\n",kname) ;
#endif

	            rs = vecstr_envadd(elp,kname,vp,vl) ;
	            if (rs != INT_MAX) c += 1 ;

#if	CF_DEBUG && CF_DEBUGENV
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procenvextra: env "
	                    "vecstr_envadd() rs=%d\n",rs) ;
#endif

	            nulstr_finish(&ns) ;
	        } /* end if (nulstr) */

	        if (rs < 0) break ;
	    } /* end if */

	} /* end for */

#if	CF_DEBUG && CF_DEBUGENV
	if (DEBUGLEVEL(4))
	    debugprintf("main/procenvextra: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procenvextra) */


static int procenvdef(PROGINFO *pip)
{
	vecstr		*elp = &pip->exports ;
	const int	rsn = SR_NOTFOUND ;
	const int	dlen = DIGBUFLEN ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	char		dbuf[DIGBUFLEN + 1] ;

	for (i = 0 ; envdefs[i] != NULL ; i += 1) {
	    cchar	*ename = envdefs[i] ;
	    if ((rs = vecstr_search(elp,ename,vstrkeycmp,NULL)) == rsn) {
	        const int	sc = MKCHAR(ename[0]) ;
		int		vl = -1 ;
		cchar		*vp = NULL ;
	        switch (sc) {
	        case 'S':
	            vp = pip->usysname ;
	            break ;
	        case 'R':
	            vp = pip->urelease ;
	            break ;
	        case 'V':
	            vp = pip->uversion ;
	            break ;
	        case 'M':
	            vp = pip->umachine ;
	            break ;
	        case 'N':
	            if (ename[1] == 'C') {
	                rs = ctdeci(dbuf,dlen,pip->ncpu) ;
	                vp = dbuf ;
			vl = rs ;
	            } else {
	                vp = pip->nodename ;
		    }
	            break ;
	        case 'A':
	            vp = pip->architecture ;
	            break ;
	        case 'H':
	            vp = pip->hz ;
	            break ;
	        case 'U':
	            vp = pip->username ;
	            break ;
	        case 'G':
	            vp = pip->groupname ;
	            break ;
	        case 'D':
	            vp = pip->domainname ;
	            break ;
	        } /* end switch */
	        if ((rs >= 0) && (vp != NULL)) {
	            rs = vecstr_envadd(elp,ename,vp,vl) ;
	            if (rs < INT_MAX) c += 1 ;
	        }
	    } /* end if (environment variable was not already present) */
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procenvdef) */


static int procenvsysvar(PROGINFO *pip,cchar sysvardb[])
{
	SYSVAR		sv ;
	SYSVAR_CUR	cur ;
	vecstr		*elp ;
	const int	vlen = VBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		vl ;
	int		c = 0 ;
	const char	*varpath = VARPATH ;
	char		kbuf[KBUFLEN + 1] ;
	char		vbuf[VBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procenvsysvar: ent sysvardb=%s\n",sysvardb) ;
#endif

	elp = &pip->exports ;
	if ((rs = sysvar_open(&sv,pip->pr,sysvardb)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("procenvsysvar: sysvar_open() rs=%d\n",rs) ;
#endif

#if	CF_SYSVARENUM
	    if ((rs = sysvar_curbegin(&sv,&cur)) >= 0) {

	        while (rs >= 0) {
	            vl = sysvar_enum(&sv,&cur,kbuf,KBUFLEN,vbuf,vlen) ;
	            if (vl == SR_NOTFOUND) break ;
	            rs = vl ;
	            if (rs < 0) break ;

#if	CF_DEBUG && CF_DEBUGENV
	            if (DEBUGLEVEL(3)) {
	                debugprintf("procenvsysvar: sysvar_enum() rs=%d\n",rs) ;
	                debugprintf("procenvsysvar: k=%s v=>%t<\n",kbuf,
	                    vbuf,strnnlen(vbuf,vl,40)) ;
	            }
#endif

	            rs = vecstr_envadd(elp,kbuf,vbuf,vl) ;
	            if (rs != INT_MAX) c += 1 ;

#if	CF_DEBUG && CF_DEBUGENV
	            if (DEBUGLEVEL(3))
	                debugprintf("procenvsysvar: "
	                    "vecstr_envadd() rs=%d\n",rs) ;
#endif

	            if ((rs >= 0) && (pip->defpath == NULL) &&
	                (strcmp(varpath,kbuf) == 0)) {
	                const char	**vpp = &pip->defpath ;

#if	CF_DEBUG && CF_DEBUGENV
	                if (DEBUGLEVEL(3))
	                    debugprintf("procenvsysvar: defpath=>%t<\n",
	                        vbuf,strnnlen(vbuf,vl,40)) ;
#endif

	                rs = proginfo_setentry(pip,vpp,vbuf,vl) ;
	            }

	        } /* end while */

	        sysvar_curend(&sv,&cur) ;
	    } /* end if (sysvar-cursor) */
#endif /* CF_SYSVARENUM */

	    rs1 = sysvar_close(&sv) ;
	    if (rs >= 0) rs = rs1 ;
	} else if (isNotPresent(rs)) {
#if	CF_DEBUG 
	    if (DEBUGLEVEL(3))
	        debugprintf("procenvsysvar: sysvar_open() rs=%d\n",rs) ;
#endif
	    rs = SR_OK ;
	} /* end if (sysvar) */

#if	CF_DEBUG 
	if (DEBUGLEVEL(3))
	    debugprintf("procenvsysvar: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procenvsysvar) */


static int procenvsort(PROGINFO *pip)
{
	int		rs ;

	rs = vecstr_sort(&pip->exports,NULL) ;

	return rs ;
}
/* end subroutine (procenvsort) */


static int loadschedvars(PROGINFO *pip)
{
	VECSTR		*svp = &pip->svars ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	const char	*keys = "phen" ;
	for (i = 0 ; keys[i] != '\0' ; i += 1) {
	    const int	kc = MKCHAR(keys[i]) ;
	    const char	*vp = NULL ;
	    int		vl = -1 ;
	    switch (kc) {
	    case 'p':
	        vp = pip->pr ;
	        break ;
	    case 'h':
	        vp = pip->homedname ;
	        break ;
	    case 'e':
	        vp = "etc" ;
	        break ;
	    case 'n':
	        vp = pip->searchname ;
	        break ;
	    } /* end switch */
	    if (vp != NULL) {
	        char	kbuf[2] = "x" ;
	        kbuf[0] = kc ;
	        rs = vecstr_envadd(svp,kbuf,vp,vl) ;
	        if (rs < INT_MAX) c += 1 ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadschedvars) */


static int loadgroupname(PROGINFO *pip)
{
	int		rs ;
	int		gnlen = GROUPNAMELEN ;
	char		gnbuf[GROUPNAMELEN + 1] ;

	if ((rs = getgroupname(gnbuf,gnlen,pip->gid)) >= 0) {
	    cchar	**vpp = &pip->groupname ;
	    rs = proginfo_setentry(pip,vpp,gnbuf,rs) ;
	}

	return rs ;
}
/* end subroutine (loadgroupname) */


static int loadplatform(PROGINFO *pip)
{
	int		rs = SR_NOSYS ;
	int		cl ;
	const char	*vn = envdefs[envdef_platform] ;
	const char	*cp ;
	char		archbuf[ARCHBUFLEN + 1] ;

	cl = -1 ;
	cp = getenv(vn) ;

#ifdef	SI_PLATFORM
	if (cp == NULL) {
	    rs = u_sysinfo(SI_PLATFORM,archbuf,ARCHBUFLEN) ;
	    if (rs >= 0) {
	        cp = archbuf ;
	        cl = rs ;
	    }
	}
#endif /* SI_PLATFORM */

	if (cp != NULL) {
	    cchar	**vpp = &pip->platform ;
	    rs = proginfo_setentry(pip,vpp,cp,cl) ;
	}

	return rs ;
}
/* end subroutine (loadplatform) */


static int loadarchitecture(PROGINFO *pip)
{
	const int	alen = ARCHBUFLEN ;
	int		rs ;
	char		abuf[ARCHBUFLEN + 1] ;
	if ((rs = getarchitecture(abuf,alen)) >= 0) {
	    cchar	**vpp = &pip->architecture ;
	    rs = proginfo_setentry(pip,vpp,abuf,rs) ;
	}
	return rs ;
}
/* end subroutine (loadarchitecture) */


static int loadhz(PROGINFO *pip)
{
	const int	dlen = DIGBUFLEN ;
	int		rs = SR_OK ;
	int		cl = -1 ;
	cchar		*cp ;
	char		dbuf[DIGBUFLEN + 1] ;

	if ((cp = getenv(VARHZ)) != NULL) {
	    if (hasalldig(cp,-1)) {
	        rs = SR_OK ;
	    } else {
	        cp = NULL ;
	    }
	}

	if (cp == NULL) {
	    if ((rs = gethz(0)) >= 0) {
	        const int	v = rs ;
	        if ((rs = ctdeci(dbuf,dlen,v)) >= 0) {
	            cp = dbuf ;
	            cl = rs ;
	        }
	    } /* end if (gethz) */
	} /* end if (still needed) */

	if (rs >= 0) {
	    if (cp != NULL) {
		cchar	**vpp = &pip->hz ;
	        rs = proginfo_setentry(pip,vpp,cp,cl) ;
	    } else {
	        rs = SR_NOSYS ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/loadhz: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (loadhz) */


static int loadncpu(PROGINFO *pip)
{
	int		rs ;
	rs = getnprocessors(pip->envv,0) ;
	pip->ncpu = rs ;
	return rs ;
}
/* end subroutine (loadncpu) */


static int loadorg(PROGINFO *pip)
{
	const int	orglen = MAXNAMELEN ;
	int		rs = SR_OK ;
	const char	*orgp = pip->org ;

	if ((orgp == NULL) || (orgp[0] == '\0')) {
	    char	orgbuf[MAXNAMELEN+1] ;
	    const char	*cp ;
	    int		cl = 0 ;
	    cp = orgbuf ;
	    if ((rs >= 0) && (cl == 0)) {
	        rs = localgetorg(pip->pr,orgbuf,orglen,pip->username) ;
	        cl = rs ;
	        if (rs >= 0) cp = orgbuf ;
	        if ((rs == SR_NOENT) || (rs == SR_ACCESS)) {
	            rs = SR_OK ;
	            cl = 0 ;
	        }
	    }
	    if ((rs >= 0) && (cl > 0)) {
		cchar	**vpp = &pip->org ;
	        rs = proginfo_setentry(pip,vpp,cp,cl) ;
	    }
	} /* end if (organization) */

	return rs ;
}
/* end subroutine (loadorg) */


static int loadprovider(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	char		provname[PROVIDERLEN + 1] ;

#ifdef	SI_HW_PROVIDER
	rs = u_sysinfo(SI_HW_PROVIDER,provname,PROVIDERLEN) ;
	len = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/loadprovider: u_sysinfo() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    if ((rs = getproviderid(provname,len)) >= 0) {
		cchar	**vpp = &pip->provider ;
	        pip->providerid = rs ;
	        rs = proginfo_setentry(pip,vpp,provname,len) ;
	    }
	}
#endif /* SI_HW_PROVIDER */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/loadprovider: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (loadprovider) */


static int loadcooks(PROGINFO *pip)
{
	EXPCOOK		*cop = &pip->cooks ;
	const int	dlen = DIGBUFLEN ;
	int		rs = SR_OK ;
	int		ci ;
	char		tbuf[USERNAMELEN+1] = { 0 } ;
	char		nbuf[USERNAMELEN+1] = { 0 } ;
	char		dbuf[DIGBUFLEN + 1] ;

	for (ci = 0 ; cooks[ci] != NULL ; ci += 1) {
	    cchar	*vp = NULL ;
	    int		vl = -1 ;
	    switch (ci) {
	    case cook_sysname:
	        vp = pip->usysname ;
	        break ;
	    case cook_release:
	        vp = pip->urelease ;
	        break ;
	    case cook_version:
	        vp = pip->uversion ;
	        break ;
	    case cook_machine:
	        vp = pip->umachine ;
	        break ;
	    case cook_platform:
	        vp = pip->platform ;
	        break ;
	    case cook_architecture:
	        vp = pip->architecture ;
	        break ;
	    case cook_ncpu:
			vp = dbuf ;
	            if (pip->ncpu >= 0) {
	                rs = ctdeci(dbuf,dlen,pip->ncpu) ;
			vl = rs ;
	            } else {
	                strcpy(dbuf,"1") ;
	                vl = 1 ;
	            }
	        break ;
	    case cook_hz:
	        vp = pip->hz ;
	        break ;
	    case cook_u:
	        vp = pip->username ;
	        break ;
	    case cook_g:
	        vp = pip->groupname ;
	        break ;
	    case cook_home:
	        vp = pip->homedname ;
	        break ;
	    case cook_shell:
	        vp = pip->shell ;
	        break ;
	    case cook_organization:
	    case cook_o:
	        vp = pip->org ;
	        break ;
	    case cook_gecosname:
	        vp = pip->gecosname ;
	        break ;
	    case cook_realname:
	        vp = pip->realname ;
	        break ;
	    case cook_name:
	        vp = pip->name ;
	        break ;
	    case cook_tz:
	        vp = pip->tz ;
	        break ;
	    case cook_n:
	        vp = pip->nodename ;
	        break ;
	    case cook_d:
	        vp = pip->domainname ;
	        break ;
	    case cook_h:
	        {
		    const int	hlen = MAXHOSTNAMELEN ;
	            cchar	*nn = pip->nodename ;
	            cchar	*dn = pip->domainname ;
	            char	hbuf[MAXHOSTNAMELEN + 1] ;
	            if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	                rs = expcook_add(cop,cooks[ci],hbuf,rs) ;
		    }
	        } /* end block */
	        break ;
	    case cook_r:
	        vp = pip->pr ;
	        break ;
	    case cook_rn:
	        vp = pip->rootname ;
	        break ;
	    case cook_ostype:
	    case cook_osnum:
	        if (tbuf[0] == '\0') {
	            cchar	*sysname = pip->usysname ;
	            cchar	*release = pip->urelease ;
	            rs = getsystypenum(tbuf,nbuf,sysname,release) ;
	        }
	        if (rs >= 0) {
	            switch (ci) {
	            case cook_ostype:
	                vp = tbuf ;
	                break ;
	            case cook_osnum:
	                vp = nbuf ;
	                break ;
	            } /* end switch */
	        } /* end if */
	        break ;
	    case cook_s:
	        vp = pip->searchname ;
	        break ;
	    case cook_oo:
	        {
	            const int	oolen = ORGLEN ;
	            int		i ;
	            int		ch ;
	            cchar	*o = pip->org ;
	            char	oobuf[ORGLEN + 1] ;
	            for (i = 0 ; (i < oolen) && *o ; i += 1) {
	                ch = (o[i] & 0xff) ;
	                oobuf[i] = (char) ((CHAR_ISWHITE(ch)) ? '-' : ch) ;
	            }
	            oobuf[i] = '\0' ;
	            rs = expcook_add(cop,cooks[ci],oobuf,i) ;
	        } /* end block */
	        break ;
	    case cook_oc:
	        vp = pip->orgcode ;
	        break ;
	    case cook_v:
		vp = pip->version ;
		break ;
	    } /* end switch */
	    if ((rs >= 0) && (vp != NULL)) {
#if	CF_DEBUG
		if (DEBUGLEVEL(3))
		debugprintf("main/loadcooks: k=%s v=>%t<\n",cooks[ci],vp,vl) ;
#endif
	        rs = expcook_add(cop,cooks[ci],vp,vl) ;
	    }
	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (loadcooks) */


static int loaddefs(PROGINFO *pip,cchar *dfname,cchar **s1,cchar **s2)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	debugprintf("main/loaddefs: ent\n") ;
	debugprintf("main/loaddefs: dfname=%s\n",dfname) ;
	}
#endif

	if ((dfname != NULL) && (dfname != '\0')) {
	    rs = loaddefsfile(pip,dfname) ;
	}

	if (rs >= 0) {
	    if ((rs = loaddefsfind(pip,s1)) >= 0) {
	        rs = loaddefsfind(pip,s2) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/loaddefs: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (loaddefs) */


static int loaddefsfile(PROGINFO *pip,cchar *dfname)
{
	struct ustat	sb ;
	int		rs ;
	int		f = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/loaddefsfile: dfname=%s\n",dfname) ;
#endif

	if ((rs = u_stat(dfname,&sb)) >= 0) {
	    if (S_ISREG(sb.st_mode)) {
	        if ((rs = sperm(&pip->id,&sb,R_OK)) >= 0) {
		    VECSTR	*defp = &pip->defs ;
		    EXPCOOK	*ecp = &pip->cooks ;
		    cchar	**envv = pip->envv ;
		    f = TRUE ;
	    	    rs = defproc(defp,envv,ecp,dfname) ;
		} else if (isNotAccess(rs)) {
		    rs = SR_OK ;
		}
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	} /* end if (stat) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/loaddefsfile: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (loaddefsfile) */


static int loaddefsfind(PROGINFO *pip,cchar *sched[])
{
	const int	tlen = MAXPATHLEN ;
	int		rs ;
	int		f = FALSE ;
	const char	*df = DEFSFNAME ;
	char		tbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/loaddefsfind: ent\n") ;
#endif

	if ((rs = permsched(sched,&pip->svars,tbuf,tlen,df,R_OK)) >= 0) {
	    VECSTR	*defp = &pip->defs ;
	    EXPCOOK	*ecp = &pip->cooks ;
	    cchar	**envv = pip->envv ;
	    f = TRUE ;
	    rs = defproc(defp,envv,ecp,tbuf) ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/loaddefsfind: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (loaddefsfind) */


static int loadxfile(PROGINFO *pip,cchar *xfname)
{
	struct ustat	sb ;
	int		rs ;
	int		f = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("envset/loadxfile: fname=%s\n",xfname) ;
#endif

	    if (pip->debuglevel > 0) {
		cchar	*pn = pip->progname ;
		bprintf(pip->efp,"%s: xfile=%s\n",pn,xfname) ;
	    }

	if ((rs = u_stat(xfname,&sb)) >= 0) {
	    if (S_ISREG(sb.st_mode)) {
	        if ((rs = sperm(&pip->id,&sb,R_OK)) >= 0) {
		    if ((rs = securefile(xfname,pip->euid,pip->egid)) >= 0) {
	    		pip->f.secure_path = TRUE ;
		    } else if (isNotPresent(rs)) {
			rs = SR_OK ;
		    }
		    if (rs >= 0) {
			ENVS		*envp = &pip->xenvs ;
	    	        EXPCOOK		*clp = &pip->cooks ;
			VECSTR		*defp = &pip->defs ;
			cchar		**envv = pip->envv ;
			f = TRUE ;
	    	        rs = envs_procxe(envp,clp,envv,defp,xfname) ;
		    }
		} else if (isNotAccess(rs)) {
		    rs = SR_OK ;
		}
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

	    if (pip->debuglevel > 0) {
		cchar	*pn = pip->progname ;
		bprintf(pip->efp,"%s: xfile %s loaded (%d)\n",
			((f)?"":"not"),pn,rs) ;
	    }

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("envset/loadxfile: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (loadxfile) */


static int loadxsched(PROGINFO *pip,cchar *sched[])
{
	const int	tlen = MAXPATHLEN ;
	int		rs ;
	int		f = FALSE ;
	const char	*xf = XEFNAME ;
	char		tbuf[MAXPATHLEN + 1] ;

	if ((rs = permsched(sched,&pip->svars,tbuf,tlen,xf,R_OK)) >= 0) {
	    ENVS	*envp = &pip->xenvs ;
	    EXPCOOK	*clp = &pip->cooks ;
	    VECSTR	*defp = &pip->defs ;
	    cchar	**envv = pip->envv ;
	    f = TRUE ;
	    rs = envs_procxe(envp,clp,envv,defp,tbuf) ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (loadxsched) */


static int pvarsbegin(PROGINFO *pip,cchar **pathvars,cchar *fname)
{
	vecstr		*pvp = &pip->pvars ;
	int		rs ;

	if ((rs = vecstr_start(pvp,10,0)) >= 0) {
	    if ((rs = loadpvars(pip,schedpfile,fname)) >= 0) {
	        if ((rs = loadpvars(pip,schedhfile,fname)) >= 0) {
	    	    rs = loadpvarsdef(pip,pathvars) ;
		}
	    }
	    if (rs < 0) {
		vecstr_finish(pvp) ;
	    }
	}

	return rs ;
}
/* end subroutine (pvarsbegin) */


static int pvarsend(PROGINFO *pip)
{
	vecstr		*pvp ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

	pvp = &pip->pvars ;
	rs1 = vecstr_finish(pvp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (pvarsend) */


static int loadpvars(PROGINFO *pip,cchar *sched[],cchar *fname)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if ((fname != NULL) && (fname[0] != '\0')) {
	    VECSTR	*svp = &pip->svars ;
	    const int	tlen = MAXPATHLEN ;
	    char	tbuf[MAXPATHLEN + 1] ;
	    if ((rs = permsched(sched,svp,tbuf,tlen,fname,R_OK)) >= 0) {
	        VECSTR	*pvp = &pip->pvars ;
	        f = TRUE ;
	        rs = vecstr_loadfile(pvp,TRUE,tbuf) ;
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	} /* end if (non-empty) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (loadpvars) */


static int loadpvarsdef(PROGINFO *pip,cchar *pnames[])
{
	vecstr		*pvp = &pip->pvars ;
	int		rs ;
	int		i = 0 ;

	if ((rs = vecstr_count(pvp)) == 0) {
	    for (i = 0 ; pnames[i] != NULL ; i += 1) {
	        rs = vecstr_adduniq(pvp,pnames[i],-1) ;
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (zero) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (loadpvarsdef) */


static int loadsysinfo(PROGINFO *pip)
{
	int		rs ;
	if ((rs = loadgroupname(pip)) >= 0) {
	    if ((rs = loadplatform(pip)) >= 0) {
	        if ((rs = loadarchitecture(pip)) >= 0) {
	    	    if ((rs = loadhz(pip)) >= 0) {
	    		if ((rs = loadncpu(pip)) >= 0) {
	    		    if ((rs = loadorg(pip)) >= 0) {
	    			rs = loadprovider(pip) ;
			    }
			}
		    }
		}
	    }
	}
	return rs ;
}
/* end subroutine (loadsysinfo) */


static int isenvok(cchar *envbads[],cchar *kp,int kl)
{
	int		f = TRUE ;

	if (kl < 0) kl = strlen(kp) ;

	f = f && (kl > 0) ;
#ifdef	COMMENT
	f = f && (kp[0] != '_') ;
#endif
	f = f && (matstr(envbads,kp,kl) < 0) ;
	f = f && (nleadstr("_INIT",kp,kl) < 5) ;

	return f ;
}
/* end subroutine (isenvok) */


#if	(CF_DEBUG || CF_DEBUGS) && CF_DEBUGCOOKS

static int debugcooks(PROGINFO *pip,cchar s[])
{
	EXPCOOK_CUR	cur ;
	int		rs1 ;
	char		buf[BUFLEN + 1] ;
	char		*cp ;
	if (s != NULL)
	    debugprintf("main/debugcooks: s=%s\n",s) ;
	expcook_curbegin(&pip->cooks,&cur) ;
	while (expcook_enum(&pip->cooks,&cur,buf,BUFLEN) >= 0) {
	    debugprintf("main: cook=>%s<\n",buf) ;
	}
	expcook_curend(&pip->cooks,&cur) ;
	rs1 = expcook_findkey(&pip->cooks,VARHOME,-1,&cp) ;
	debugprintf("main: _findkey() rs=%d HOME=>%s<\n",rs1,cp) ;
}
/* end subroutine (debugcooks) */

#endif /* CF_DEBUG */


