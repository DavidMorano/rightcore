/* main (envset) */

/* set environment for a user (usually at login) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */
#define	CF_DEBUG	0		/* debug print-outs switchable */
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_DEBUGN	0		/* debug w/ 'nprintf(3b)' */
#define	CF_DEBUGLEVEL	0		/* set default debug-level */
#define	CF_ENVSORT	1		/* sort environment */
#define	CF_NPROCESSORS	1		/* use 'sysconf(3c)' */
#define	CF_SYSVAR	0		/* use 'sysvar' */
#define	CF_LOGID	0		/* use |procuserinfo_logid()| */


/* revision history:

	= 2001-04-11, David A­D­ Morano
        This old dog program has been enhanced to serve as the environment wiper
        for executing MIPS programs.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ envset [-V] <prog> <arg0> <arg1> ...

	Implementation notes:

	= NCPU

        We get the number of CPUs from the kernel. On Sun-Solaris, this is
        (gag!) not thread-safe. This is not a problem with the present code
        since this "program" is (so far) never embedded within other
        (multi-threaded) software. But it is something to note if this software
        ever does get embeeded into some multi-threaded code. Unlikely you say?
        More and more "programs" in the old sense (separate independent
        processes) are increasingly becoming embedded within larger
        multi-threaded programs that create the illusion of programs being
        separate processes (but which really are not). So just keep this in
        mind.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<bfile.h>
#include	<ids.h>
#include	<uinfo.h>
#include	<userinfo.h>
#include	<userattr.h>
#include	<vecstr.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<nulstr.h>
#include	<tmtime.h>		/* is NOT thread-safe */
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"progreport.h"
#include	"envs.h"
#include	"sysvar.h"


/* local defines */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	VARAST
#define	VARAST		"AST"
#endif

#ifndef	VARNCPU
#define	VARNCPU		"NCPU"
#endif

#ifndef	VARLOCALDOMAIN
#define	VARLOCALDOMAIN	"LOCALDOMAIN"
#endif

#ifndef	VARNAME
#define	VARNAME		"NAME"
#endif

#ifndef	VARFULLNAME
#define	VARFYLLNAME	"FULLNAME"
#endif

#ifndef	UA_TZ
#define	UA_TZ		"tz"
#endif

#undef	NAMECNAME
#define	NAMECNAME	".name"

#undef	FULLNAMECNAME
#define	FULLNAMECNAME	".fullname"

#undef	NDFN
#define	NDFN		"/tmp/envset.deb"

#define	OURCONF		struct ourconf
#define	OURCONF_FL	struct ourconf_flags


/* external subroutines */

extern int	sntmtime(char *,int,TMTIME *,cchar *) ;
extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	pathclean(char *,cchar *,int) ;
extern int	nleadstr(cchar *,cchar *,int) ;
extern int	vstrkeycmp(cchar **,cchar **) ;
extern int	ctdeci(char *,int,int) ;
extern int	ctdecl(char *,int,long) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	strlinelen(cchar *,int,int) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	vecstr_envadd(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_envget(vecstr *,cchar *,cchar **) ;
extern int	vecstr_loadfile(vecstr *,int,cchar *) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	fperm(int,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	permsched(cchar **,vecstr *,char *,int,cchar *,int) ;
extern int	xfile(IDS *,cchar *) ;
extern int	getnodename(char *,int) ;
extern int	getusername(char *,int,uid_t) ;
extern int	gethz(int) ;
extern int	getnodeinfo(cchar *,char *,char *,vecstr *,cchar *) ;
extern int	getsysdomain(char *,int) ;
extern int	getsystypenum(char *,char *,cchar *,cchar *) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	getnprocessors(cchar **,int) ;
extern int	getuid_user(cchar *,int) ;
extern int	getuserorg(char *,int,cchar *) ;
extern int	localgetorg(cchar *,char *,int,cchar *) ;
extern int	inittimezone(char *,int,cchar *) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	mktmpuserdir(char *,cchar *,cchar *,mode_t) ;
extern int	mklogid(char *,int,cchar *,int,int) ;
extern int	mksublogid(char *,int,cchar *,int) ;
extern int	readfileline(char *,int,cchar *) ;
extern int	hasalldig(cchar *,int) ;
extern int	hasallplusminus(cchar *,int) ;
extern int	hasallminus(cchar *,int) ;
extern int	hasallwhite(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isNotValid(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	proginfo_rootname(PROGINFO *) ;
extern int	progfindprog(PROGINFO *,char *,cchar *) ;
extern int	progexec(PROGINFO *,cchar *,cchar **,int) ;

extern int	defproc(vecstr *,cchar **,EXPCOOK *,cchar *) ;

extern int	envs_procxe(ENVS *,EXPCOOK *,cchar **,VECSTR *,cchar *) ;
extern int	envs_subs(ENVS *,EXPCOOK *,VECSTR *,VECSTR *) ;

#if	CF_DEBUGS || CF_DEBUG || CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strnnlen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct ourconf_flags {
	uint		stores:1 ;
} ;

struct ourconf {
	vecstr		stores ;
	OURCONF_FL	open ;
	PROGINFO	*pip ;
	PARAMOPT	*pop ;
	cchar		*defprog ;
} ;

struct xfile {
	cchar		*fname ;
	cchar		*vname ;
} ;

struct execmap {
	cchar		*fname ;
	char		*interpreter ;
} ;

struct intprog {
	char		fname[MAXPATHLEN + 1] ;
	char		arg[MAXPATHLEN + 1] ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;

#if	CF_LOGID
static int	procuserinfo_logid(PROGINFO *) ;
#endif /* CF_LOGID */

static int	procexpcooks_begin(PROGINFO *) ;
static int	procexpcooks_end(PROGINFO *) ;

static int	procourconf_begin(PROGINFO *,PARAMOPT *) ;
static int	procourconf_end(PROGINFO *) ;
static int	procourconf_cfname(PROGINFO *) ;
static int	procourconf_getdefprog(PROGINFO *,cchar **) ;

static int	procdefs_begin(PROGINFO *) ;
static int	procdefs_end(PROGINFO *) ;

static int	procenvs_begin(PROGINFO *) ;
static int	procenvs_end(PROGINFO *) ;

static int	procexps_begin(PROGINFO *,cchar *) ;
static int	procexps_end(PROGINFO *) ;

static int	procloadinfo(PROGINFO *) ;
static int	process(PROGINFO *,ARGINFO *,PARAMOPT *,cchar *,cchar *) ;
static int	processor(PROGINFO *,ARGINFO *,cchar *) ;
static int	procnopreload(PROGINFO *) ;

static int	procenvextra(PROGINFO *) ;
static int	procenvdef(PROGINFO *) ;
static int	procenvuser(PROGINFO *) ;
static int	procenvsort(PROGINFO *) ;
static int	procenvname(PROGINFO *,cchar *,cchar *,cchar *) ;

#if	CF_SYSVAR
static int	procenvsysvar(PROGINFO *,cchar *) ;
#endif

static int	procsched_begin(PROGINFO *) ;
static int	procsched_end(PROGINFO *) ;

static int	pvarstart(PROGINFO *,cchar **,cchar *) ;
static int	pvarfinish(PROGINFO *) ;

static int	loadtz(PROGINFO *) ;
static int	loadgroupname(PROGINFO *) ;
static int	loadaux(PROGINFO *) ;
static int	loadhz(PROGINFO *) ;
static int	loadncpu(PROGINFO *) ; /* may not be thread-safe */
static int	loadnodeinfo(PROGINFO *) ;
static int	loadorganization(PROGINFO *) ;
static int	loadcooks(PROGINFO *) ;
static int	loaddefsfile(PROGINFO *,cchar *) ;
static int	loaddefs(PROGINFO *,cchar **) ;
static int	loadxfile(PROGINFO *,cchar *) ;
static int	loadxsched(PROGINFO *,cchar **) ;
static int	loadpvars(PROGINFO *,cchar **,cchar *) ;
static int	loadpvarsdef(PROGINFO *,cchar **) ;

static int	ourconf_start(OURCONF *,PROGINFO *,PARAMOPT *) ;
static int	ourconf_finish(OURCONF *) ;
static int	ourconf_setentry(OURCONF *,cchar **,cchar *,int) ;
static int	ourconf_read(OURCONF *) ;
static int	ourconf_defprog(OURCONF *,cchar *,int) ;
static int	ourconf_nopreload(OURCONF *,cchar *,int) ;
static int	ourconf_getdefprog(OURCONF *,cchar **) ;
static int	ourconf_ifnopreload(OURCONF *) ;


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"VERSION",
	"HELP",
	"pm",
	"sn",
	"caf",
	"ef",
	"of",
	"if",
	"cf",
	"df",
	"xf",
	"pf",
	"svdb",
	"nice",
	"nopreload",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_caf,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_cf,
	argopt_df,
	argopt_xf,
	argopt_pf,
	argopt_svdb,
	argopt_nice,
	argopt_nopreload,
	argopt_overlast
} ;

static cchar	*akonames[] = {
	"quiet",
	"xeall",
	"xextra",
	NULL
} ;

enum akonames {
	akoname_quiet,
	akoname_xeall,
	akoname_xextra,
	akoname_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOMEM, EX_OSERR },
	{ SR_NOENT, EX_NOTFOUND },
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

static cchar	*progmodes[] = {
	"envset",
	"lsh",
	"ksh",
	NULL
} ;

enum progmodes {
	progmode_envset,
	progmode_lsh,
	progmode_ksh,
	progmode_overlast
} ;

/* configuration parameter key-names */
static cchar	*cparams[] = {
	"defprog",
	"organization",
	"shell",
	"nopreload",
	NULL
} ;

enum cparams {
	cparam_defprog,
	cparam_org,
	cparam_shell,
	cparam_nopreload,
	cparam_overlast
} ;

/* cookie-names */
static cchar	*cooks[] = {
	"SYSNAME",	/* OS system-name */
	"RELEASE",	/* OS system-release */
	"VERSION",	/* OS system-version */
	"MACHINE",	/* machine-name */
	"ARCHITECTURE",	/* UAUX machine-architecture */
	"PLATFORM",	/* UAUX machine platform */
	"PROVIDER",	/* UAUX machine provider */
	"HWSERIAL",	/* UAUX HW-serial */
	"NISDOMAIN",
	"NCPU",		/* number of machine CPUs */
	"HZ",		/* OS clock tics */
	"CLUSTER",	/* cluster-name */
	"SYSTEM",	/* system-name */
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
	"RN",		/* program root-name */
	"OSTYPE",	/* OS-type */
	"OSNUM",	/* OS-name */
	"S",		/* search-name */
	NULL
} ;

enum cooks {
	cook_sysname,
	cook_release,
	cook_version,
	cook_machine,
	cook_architecture,
	cook_platform,
	cook_provider,
	cook_hwserial,
	cook_nisdomain,
	cook_ncpu,
	cook_hz,
	cook_cluster,
	cook_system,
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
	cook_overlast
} ;

static cchar	*schedhconf[] = {
	"%h/etc/%n/%n.%f",
	"%h/etc/%n/%f",
	"%h/etc/%n.%f",
	NULL
} ;

static cchar	*schedpconf[] = {
	"%p/etc/%n/%n.%f",
	"%p/etc/%n/%f",
	"%p/etc/%n.%f",
	NULL
} ;

static cchar	*schedaconf[] = {
	"%a/etc/%n/%n.%f",
	"%a/etc/%n/%f",
	"%a/etc/%n.%f",
	NULL
} ;

static cchar	*schedhfile[] = {
	"%h/etc/%n/%n.%f",
	"%h/etc/%n/%f",
	"%h/etc/%n.%f",
	"%h/etc/%f",
	NULL
} ;

static cchar	*schedpfile[] = {
	"%p/etc/%n/%n.%f",
	"%p/etc/%n/%f",
	"%p/etc/%n.%f",
	"%p/etc/%f",
	NULL
} ;

static cchar	*altprs[] = {
	"/usr/extra",
	"/usr/preroot",
	"/",
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

static cchar	*envbad[] = {
	"_",
	"_A0",
	"_EF",
	"A__z",
	"RANDOM",
	"TMOUT",
	"TZ",				/* we do not want this in default */
	NULL
} ;

enum envdefs {
	envdef_architecture,
	envdef_platform,
	envdef_provider,
	envdef_hwserial,
	envdef_nisdomain,
	envdef_overlast
} ;

static cchar	*envdefs[] = {
	"ARCHITECTURE",
	"PLATFORM",
	"PROVIDER",
	"HWSERIAL",
	"NISDOMAIN",
	"SYSNAME",
	"RELEASE",
	"VERSION",
	"MACHINE",
	"HZ",
	"NCPU",
	"CLUSTER",
	"SYSTEM",
	"USERNAME",
	"HOME",
	"GROUPNAME",
	"NODE",
	"DOMAIN",
	"ORGANIZATION",
	"PRINTER",
	NULL
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argi, argl, aol, akl, avl, kwi ;
	int		ai, ai_pos ;
	int		rs, rs1 ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optplus, f_optminus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_envset = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*argz = NULL ;
	cchar		*pr = NULL ;
	cchar		*pm = NULL ;
	cchar		*sn = NULL ;
	cchar		*program = NULL ;
	cchar		*efname = NULL ;
	cchar		*pvfname = NULL ;
	cchar		*svdb = NULL ;
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

	argz = argv[0] ;
	rs = proginfo_start(pip,envv,argz,VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGLEVEL
	pip->debuglevel = DEFDEBUGLEVEL ;
#endif

/* program mode */

#if	CF_DEBUGS
	debugprintf("main: progname=%s\n",pip->progname) ;
#endif

	rs1 = matstr(progmodes,pip->progname,-1) ;
	if (rs1 >= 0) pip->progmode = rs1 ;

	pip->verboselevel = 1 ;
	pip->f.xeall = TRUE ;
	pip->f.xextra = TRUE ;

#if	CF_DEBUGS
	rs1 = pip->progmode ;
	debugprintf("main: progmode=%s(%u)\n",progmodes[rs1],rs1) ;
#endif

	switch (pip->progmode) {
	case progmode_lsh:
	case progmode_ksh:
	    pip->f.shell = TRUE ;
	    break ;
	} /* end switch */

	f_envset = (! pip->f.shell) ;

/* gather the arguments */

	if (rs >= 0) {
	    rs = keyopt_start(&akopts) ;
	    pip->open.akopts = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = paramopt_start(&aparams) ;
	    pip->open.aparams = (rs >= 0) ;
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badkeyoptstart ;
	}

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("main: argc=%u\n",argc) ;
	    for (i = 0 ; argv[i] != NULL ; i += 1)
	        debugprintf("main: arg[%u]=>%s<\n",i,argv[i]) ;
	}
#endif /* CF_DEBUGS */

	ai = 0 ;
	ai_pos = 0 ;
	argr = (argc - 1) ;
	argi = ai ;
	while ((rs >= 0) && (program == NULL) && (argr > 0) && f_envset) {

#if	CF_DEBUGS
	    debugprintf("main: T ai=%d argr=%d\n",ai,argr) ;
#endif

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optplus || f_optminus)) {
	        const int ach = MKCHAR(argp[1]) ;

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

/* help */
	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program mode */
	                case argopt_pm:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pm = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pm = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* search name */
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

/* configuration file */
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

/* nice-value */
	                case argopt_df:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->dfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->dfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* export file */
	                case argopt_xf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->xfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->xfname = argp ;
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

/* sysvar-DB name */
	                case argopt_svdb:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            svdb = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                svdb = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* nice-value */
	                case argopt_nice:
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
	                        rs = optvalue(cp,cl) ;
	                        pip->niceval = rs ;
	                    }
	                    break ;

/* no-preload (users) */
	                case argopt_nopreload:
			    pip->have.nopreload = TRUE ;
	                    cp = NULL ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            if ((rs = optbool(avp,avl)) >= 0) {
					pip->f.nopreload = (rs > 0) ;
				    } else if (isNotValid(rs)) {
					rs = SR_OK ;
	                                cl = avl ;
					cp = avp ;
				    }
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
				cchar		*po = PO_NOPRELOAD ;
	                        rs = paramopt_loads(pop,po,cp,cl) ;
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

/* configuration file */
	                    case 'C':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->have.cfname = TRUE ;
	                                pip->final.cfname = TRUE ;
	                                pip->cfname = argp ;
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

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'b':
	                        pip->f.background = TRUE ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = keyopt_loads(&akopts,argp,argl) ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
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
	                        f_usage = TRUE ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        program = argp ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

#if	CF_DEBUGS
	    debugprintf("main: B ai=%d argr=%d\n",ai,argr) ;
#endif

	} /* end while (all command line argument processing) */

#if	CF_DEBUGS
	debugprintf("main: done args rs=%d\n",rs) ;
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

/* continue */

	argi = (argc > 0) ? (ai + 1) : 0 ;
	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUGS
	debugprintf("main: usage ok so far\n") ;
#endif

	if ((rs >= 0) && (pip->debuglevel == 0)) {
	    if ((cp = getourenv(envv,VARDEBUGLEVEL)) != NULL) {
	        rs = optvalue(cp,-1) ;
	        pip->debuglevel = rs ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif /* CF_DEBUG */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: progname=%s\n",pip->progname) ;
	    debugprintf("main: program=%s\n",program) ;
	    debugprintf("main: argi=%u argr=%d\n",argi,argr) ;
	    debugprintf("main: next ai=%u arg=>%s<\n",
	        ai,
	        (((ai < argc) && (argv[ai] != NULL)) ? argv[ai] : "*NA*")) ;
	    debugprintf("main: remaining args>\n") ;
	    {
	        int	i ;
	        for (i = argi ; argv[i] != NULL ; i += 1) {
	            debugprintf("main: arg[%u]=>%s<\n",i,argv[i]) ;
	        }
	    }
	}
#endif /* CF_DEBUG */

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* get the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

	if (rs < 0) {
	    bprintf(pip->efp,"%s: program information initization (%d)\n",
	        rs) ;
	    ex = EX_OSERR ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

#if	CF_DEBUGN
	nprintf(NDFN,"main: pr=%s\n",pip->pr) ;
	nprintf(NDFN,"main: sn=%s\n",pip->searchname) ;
#endif

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialization */

	if (pip->cfname == NULL) pip->cfname = getenv(VARCFNAME) ;
	if (pip->dfname == NULL) pip->dfname = getenv(VARDFNAME) ;
	if (pip->xfname == NULL) pip->xfname = getenv(VARXFNAME) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: cf=%s\n",pip->cfname) ;
	    debugprintf("main: df=%s\n",pip->dfname) ;
	    debugprintf("main: xf=%s\n",pip->xfname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: cf=%s\n",pn,pip->cfname) ;
	    bprintf(pip->efp,"%s: df=%s\n",pn,pip->dfname) ;
	    bprintf(pip->efp,"%s: xf=%s\n",pn,pip->xfname) ;
	}

	if (rs >= 0)  {
	    if ((rs = proginfo_rootname(pip)) >= 0) {
	        if ((rs = procopts(pip,&akopts)) >= 0) {
		    if ((pip->niceval == 0) && (argval != NULL)) {
			rs = optvalue(argval,-1) ;
			pip->niceval = rs ;
		    }
		}
	    }
	}

/* are we in some sort of program mode? */

	if (pm != NULL) {
	    int	v = matstr(progmodes,pm,-1) ;
	    if (v >= 0) pip->progmode = v ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    if (pip->progmode >= 0) {
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    } else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif /* CF_DEBUG */

/* defaults */

	if (pvfname == NULL)
	    pvfname = PVARSFNAME ;

#if	CF_DEBUGN
	nprintf(NDFN,"main: env-tz=%s\n",getourenv(envv,VARTZ)) ;
#endif

/* start processing stuff */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.argr = argr ;
	ainfo.argi = argi ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USERINFO	u ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
		    if ((rs = procexpcooks_begin(pip)) >= 0) {
			PARAMOPT	*pop = &aparams ;
			if ((rs = procourconf_begin(pip,pop)) >= 0) {
		            if ((rs = procdefs_begin(pip)) >= 0) {
		                if ((rs = procenvs_begin(pip)) >= 0) {
				    cchar	*pfn = pvfname ;
				    if ((rs = procexps_begin(pip,pfn)) >= 0) {
				        {
					    ARGINFO	*aip = &ainfo ;
					    cchar	*pgn = program ;
					    rs = process(pip,aip,pop,svdb,pgn) ;
				        }
				        rs1 = procexps_end(pip) ;
				        if (rs >= 0) rs = rs1 ;
				    } /* end if (procexps) */
		                    rs1 = procenvs_end(pip) ;
				    if (rs >= 0) rs = rs1 ;
			        } /* end if (procenvs) */
		                rs1 = procdefs_end(pip) ;
			        if (rs >= 0) rs = rs1 ;
		            } /* end if (procdefs) */
			    rs1 = procourconf_end(pip) ;
	                    if (rs >= 0) rs = rs1 ;
		        } /* end if (procourconf) */
			rs1 = procexpcooks_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
		    } /* end if (procexpcooks) */
	            rs1 = procuserinfo_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
		fmt = "%s: credentials failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	    } /* end if */
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    cchar	*pn = pip->progname ;
	    mkreport(pip,argc,argv,rs) ;
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        break ;
	    case SR_NOENT:
	        ex = EX_NOPROG ;
	        bprintf(pip->efp, "%s: program not found\n",pn) ;
	        break ;
	    case SR_NOEXEC:
	        ex = EX_NOEXEC ;
	        bprintf(pip->efp, "%s: program would not execute\n",pn) ;
	        break ;
	    case SR_2BIG:
	        bprintf(pip->efp, "%s: program arguments too big\n",pn) ;
	        break ;
	    case SR_TXTBSY:
	        ex = EX_TEMPFAIL ;
	        bprintf(pip->efp, "%s: program text is busy\n",pn) ;
	        break ;
	    case SR_AGAIN:
	        bprintf(pip->efp, "%s: temporary fail (again)\n",pn) ;
	        ex = EX_TEMPFAIL ;
	        break ;
	    default:
	        bprintf(pip->efp, "%s: unknown error (%d)\n",pn,rs) ;
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} /* end if (exitcode) */

/* early return thing */
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp, "%s: exiting ex=%u (%d)\n",
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

	if (rs >= 0) {
	    rs = paramopt_start(&aparams) ;
	    pip->open.aparams = (rs >= 0) ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

badkeyoptstart:
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

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,
	    "%s: invalid argument was specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	mkreport(pip,argc,argv,rs) ;
	goto retearly ;

}
/* end subroutine (main) */


int progdefprog(PROGINFO *pip,cchar **rpp)
{
	int		rs = SR_OK ;
	cchar		*vp = NULL ;

	if (rpp == NULL) return SR_FAULT ;

	*rpp = NULL ;

	if ((rs >= 0) && (pip->defprog == NULL)) {
	    if ((vp = getenv(VARDEFPROG)) != NULL) {
	        if (vp[0] != '\0') {
	            cchar	**vpp = &pip->defprog ;
	            rs = proginfo_setentry(pip,vpp,vp,-1) ;
	        }
	    }
	} /* end if (environment) */

#if	CF_DEBUGN
	nprintf(NDFN,"main/progdefprog: rs=%d dp=%s\n",
	    rs,pip->defprog) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/progdefprog: defprog=%s\n",pip->defprog) ;
#endif

	if ((rs >= 0) && (pip->defprog == NULL)) {
	   if ((rs = procourconf_getdefprog(pip,&vp)) >= 0) {
	        if (vp[0] != '\0') {
	            cchar	**vpp = &pip->defprog ;
	            rs = proginfo_setentry(pip,vpp,vp,-1) ;
		}
	   }
	}

#if	CF_DEBUGN
	nprintf(NDFN,"main/progdefprog: defprog=%s\n",pip->defprog) ;
#endif

/* default value */

	if ((rs >= 0) && (pip->defprog == NULL)) {
	    cchar	**vpp = &pip->defprog ;
	    vp = DEFPROGNAME ;
	    rs = proginfo_setentry(pip,vpp,vp,-1) ;
	}

/* return result */

	if ((rs >= 0) && (pip->defprog != NULL)) {
	    *rpp = pip->defprog ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("main/progdefprog: ret rs=%d\n",rs) ;
	    debugprintf("main/progdefprog: defprog=%s\n",pip->defprog) ;
	}
#endif

	return rs ;
}
/* end subroutine (progdefprog) */


/* local subroutines */


/* print out (standard error) some short usage */
static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [-df <df>] [-xf <ef>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  <program> [<arg0> [<arg(s)> ...]]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*cp ;

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
	                case akoname_quiet:
	                    pip->verboselevel = 0 ;
	                    if (vl > 0) {
	                        rs = optvalue(vp,vl) ;
	                        pip->verboselevel = rs ;
	                    }
	                    c += 1 ;
	                    break ;
	                case akoname_xeall:
	                    pip->f.xeall = TRUE ;
	                    if (vl > 0) {
	                        rs = optvalue(vp,vl) ;
	                        pip->f.xeall = (rs > 0) ;
	                    }
	                    c += 1 ;
	                    break ;
	                case akoname_xextra:
	                    pip->f.xextra = TRUE ;
	                    if (vl > 0) {
	                        rs = optvalue(vp,vl) ;
	                        pip->f.xextra = (rs > 0) ;
	                    }
	                    c += 1 ;
	                    break ;
	                } /* end switch */

	            } /* end if (valid option) */

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procenvextra(PROGINFO *pip)
{
	NULSTR		ns ;
	vecstr		*elp = &pip->exports ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		kl, vl ;
	int		c = 0 ;
	cchar		*tp, *kp, *vp ;
	cchar		*kname ;

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

	    if ((vp != NULL) && (matstr(envbad,kp,kl) < 0)) {
	        if ((rs = nulstr_start(&ns,kp,kl,&kname)) >= 0) {

	            rs = vecstr_envadd(elp,kname,vp,vl) ;
	            if (rs != INT_MAX) c += 1 ;

	            rs1 = nulstr_finish(&ns) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (nulstr) */
	        if (rs < 0) break ;
	    } /* end if (match) */

	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procenvextra) */


static int procenvdef(PROGINFO *pip)
{
	vecstr		*elp = &pip->exports ;
	const int	nrs = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		i ;
	int		n = 0 ;
	cchar		*ename ;
	cchar		*vp ;
	char		digbuf[DIGBUFLEN + 1] ;

	for (i = 0 ; envdefs[i] != NULL ; i += 1) {
	    ename = envdefs[i] ;
	    if ((rs = vecstr_search(elp,ename,vstrkeycmp,NULL)) == nrs) {
	        int	sc = MKCHAR(ename[0]) ;
	        int	sc1 = ((ename[0] != '\0') ? MKCHAR(ename[1]) : 0) ;
	        rs = SR_OK ;
	        vp = NULL ;
	        switch (sc) {
	        case 'S':
	            if (strcmp(ename,VARSYSNAME) == 0) {
	                vp = pip->usysname ;
	            } else {
	                vp = pip->systemname ;
	            }
	            break ;
	        case 'C':
	            vp = pip->clustername ;
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
	            if (sc1 == 'C') {
	                rs = ctdeci(digbuf,DIGBUFLEN,pip->ncpu) ;
	                vp = digbuf ;
	            } else if (sc1 == 'I') {
	                vp = pip->nisdomainname ;
	            } else {
	                vp = pip->nodename ;
	            }
	            break ;
	        case 'O':
	            vp = pip->org ;
	            break ;
	        case 'P':
	            if (sc1 == 'L') {
	                vp = pip->platform ;
	            } else if (sc1 == 'R') {
	                int sc2 = MKCHAR(ename[2]) ;
	                if (sc2 == 'O') {
	                    vp = pip->provider ;
	                } else {
	                    vp = pip->printer ;
	                }
	            }
	            break ;
	        case 'A':
	            vp = pip->architecture ;
	            break ;
	        case 'H':
	            switch (sc1) {
	            case 'W':
	                vp = pip->hwserial ;
	                break ;
	            case 'Z':
	                vp = pip->hz ;
	                break ;
	            case 'O':
	                vp = pip->homedname ;
	                break ;
	            } /* end switch */
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
	        if ((rs >= 0) && (vp != NULL) && (vp[0] != '\0')) {
	            rs = vecstr_envadd(elp,ename,vp,-1) ;
	            if (rs != INT_MAX) n += 1 ;
	        }
	    } /* end if (environment variable was not already present) */
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procenvdef) */


#if	CF_SYSVAR
static int procenvsysvar(PROGINFO *pip,cchar *sysvardb)
{
	SYSVAR		sv ;
	SYSVAR_CUR	cur ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = sysvar_open(&sv,pip->pr,sysvardb)) >= 0) {
	    if ((rs = sysvar_curbegin(&sv,&cur)) >= 0) {
	        vecstr		*elp = &pip->exports ;
	        const int	klen = KBUFLEN ;
	        const int	vlen = VBUFLEN ;
	        int		vl ;
	        char		kbuf[KBUFLEN + 1] ;
	        char		vbuf[VBUFLEN + 1] ;
	        while (rs >= 0) {
	            vl = sysvar_enum(&sv,&cur,kbuf,klen,vbuf,vlen) ;
	            if (vl == SR_NOTFOUND) break ;
	            rs = vl ;

	            if (rs >= 0) {
	                rs = vecstr_envadd(elp,kbuf,vbuf,vl) ;
	                if (rs != INT_MAX) c += 1 ;
	            }

	        } /* end while */
	        rs1 = sysvar_curend(&sv,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */
	    rs1 = sysvar_close(&sv) ;
	    if (rs >= 0) rs = rs1 ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procenvsysvar: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procenvsysvar) */
#endif /* CF_SYSVAR */


static int procenvuser(PROGINFO *pip)
{
	vecstr		*elp = &pip->exports ;
	const int	nrs = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procenvuser: ent\n",rs) ;
#endif

#if	CF_DEBUGN
	nprintf(NDFN,"main/procenvuser: mid tz=%s\n",pip->tz) ;
#endif

/* environment LOCALDOMAIN */

	if (rs >= 0) {
	    cchar	*varlocaldomain = VARLOCALDOMAIN ;
	    if ((rs = vecstr_envget(elp,varlocaldomain,NULL)) == nrs) {
	        const int	dl = MAXHOSTNAMELEN ;
	        char		dn[MAXHOSTNAMELEN+1] ;
	        if ((rs = getsysdomain(dn,dl)) >= 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(3)) {
	                cchar	*udn = pip->domainname ;
	                debugprintf("main/procenvuser: sdomain=%s\n",dn) ;
	                debugprintf("main/procenvuser: udomain=%s\n",udn) ;
	            }
#endif
	            vp = pip->domainname ;
	            if (strcmp(vp,dn) != 0) {
	                rs = vecstr_envadd(elp,varlocaldomain,vp,-1) ;
	                if (rs != INT_MAX) c += 1 ;
	            }
	        } /* end if (getsysdomain) */
	    } /* end if (environment variable not found) */
	} /* end if (environment LOCALDOMAIN) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procenvuser: LOCALDOMAIN rs=%d\n",rs) ;
	    debugprintf("main/procenvuser: mid tz=%s\n",pip->tz) ;
	}
#endif

/* environment TZ */

	if (rs >= 0) {
	    cchar	*vartz = VARTZ ;
	    if ((rs = vecstr_envget(elp,vartz,NULL)) == nrs) {
	        rs = SR_OK ;
	        vp = pip->tz ;
	        if ((vp != NULL) && (vp[0] != '\0')) {
	            rs = vecstr_envadd(elp,vartz,vp,-1) ;
	            if (rs != INT_MAX) c += 1 ;
	        }
	    }
	} /* end if (environment TZ) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procenvuser: TZ rs=%d\n",rs) ;
#endif

/* environment NAME */

	if (rs >= 0) {
	    rs = procenvname(pip,VARNAME,NAMECNAME,NULL) ;
	    c += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procenvuser: 1 procenvname() rs=%d\n",rs) ;
#endif

/* environment FULLNAME */

	if (rs >= 0) {
	    rs = procenvname(pip,VARFULLNAME,FULLNAMECNAME,pip->fullname) ;
	    c += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procenvuser: 2 procenvname() rs=%d\n",rs) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procenvuser: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procenvuser) */


static int procenvname(pip,varname,fname,prename)
PROGINFO	*pip ;
cchar		*varname ;
cchar		fname[] ;
cchar		*prename ;
{
	vecstr		*elp = &pip->exports ;
	const int	nrs = SR_NOTFOUND ;
	int		rs ;
	int		c = 0 ;

	if (varname == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procenvname: ent prename=>%s<\n",prename) ;
#endif

	if ((rs = vecstr_envget(elp,varname,NULL)) == nrs) {
	    cchar	*np = NULL ;
	    int		nl = -1 ;
	    if ((prename != NULL) && (prename[0] != '\0')) {
	        rs = SR_OK ;
	        np = prename ;
	    } else {
	        const int	nlen = REALNAMELEN ;
	        char		nbuf[REALNAMELEN+1] ;
	        char		nfname[MAXPATHLEN+1] ;
	        if ((rs = mkpath2(nfname,pip->homedname,fname)) >= 0) {
	            rs = readfileline(nbuf,nlen,nfname) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                debugprintf("main/procenvname: nfname=%s\n",nfname) ;
	                debugprintf("main/procenvname: readfileline() rs=%d\n",
	                    rs) ;
	            }
#endif
	            if (rs > 0) {
	                np = nbuf ;
	                nl = rs ;
	            } else if (isNotPresent(rs))
	                rs = SR_OK ;
	        }
	    } /* end if (alternatives to find the name) */
	    if ((rs >= 0) && (np != NULL)) {
	        rs = vecstr_envadd(elp,varname,np,nl) ;
	        if (rs < INT_MAX) c += 1 ;
	    }
	} /* end if (not already found) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procenvname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procenvname) */


static int procenvsort(PROGINFO *pip)
{
	int		rs ;

	rs = vecstr_sort(&pip->exports,NULL) ;

	return rs ;
}
/* end subroutine (procenvsort) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

	pip->umachine = uip->machine ;
	pip->usysname = uip->sysname ;
	pip->urelease = uip->release ;
	pip->uversion = uip->version ;

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->gecosname = uip->gecosname ;
	pip->homedname = uip->homedname ;
	pip->gecosname = uip->gecosname ;
	pip->realname = uip->realname ;
	pip->shell = uip->shell ;
	pip->org = uip->organization ;
	pip->name = uip->name ;
	pip->fullname = uip->fullname ;
	pip->logid = uip->logid ;
	pip->tz = uip->tz ;
	pip->groupname = uip->groupname ;

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
	} /* end if (ok) */

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*un = pip->username ;
	    bprintf(pip->efp,"%s: user=%s\n",pn,un) ;
	}

#if	CF_LOGID
	if (rs >= 0) {
	    rs = procuserinfo_logid(pip) ;
	} /* end if (ok) */
#endif /* CF_LOGID */

	if (rs >= 0) {
	    if ((rs = ids_load(&pip->id)) >= 0) {
		pip->open.id = TRUE ;
		rs = procloadinfo(pip) ;
		if (rs < 0) {
		    pip->open.id = FALSE ;
		    ids_release(&pip->id) ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

	if (pip->open.id) {
	    pip->open.id = FALSE ;
	    rs1 = ids_release(&pip->id) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (procuserinfo_end) */


#if	CF_LOGID
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
#endif /* CF_LOGID */


static int procexpcooks_begin(PROGINFO *pip)
{
	int		rs ;
	if ((rs = expcook_start(&pip->cooks)) >= 0) {
	    pip->open.cooks = TRUE ;
	    if ((rs = loadcooks(pip)) >= 0) {
		rs = procsched_begin(pip) ;
	    }
	    if (rs < 0) {
	        pip->open.cooks = FALSE ;
		expcook_finish(&pip->cooks) ;
	    }
	}
	return rs ;
}
/* end subroutine (procexpcooks_begin) */


static int procexpcooks_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = procsched_end(pip) ;
	if (rs >= 0) rs = rs1 ;

	if (pip->open.cooks) {
	    pip->open.cooks = FALSE ;
	    rs1 = expcook_finish(&pip->cooks) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (procexpcooks_end) */


static int procourconf_begin(PROGINFO *pip,PARAMOPT *pop)
{
	int		rs = SR_OK ;
	if (pip->config == NULL) {
	    const int	csize = sizeof(OURCONF) ;
	    void	*p ;
	    if ((rs = uc_malloc(csize,&p)) >= 0) {
		pip->config = p ;
		if ((rs = procourconf_cfname(pip)) >= 0) {
		    OURCONF	*ocp = pip->config ;
		    if ((rs = ourconf_start(ocp,pip,pop)) >= 0) {
			rs = ourconf_read(ocp) ;
			if (rs < 0) {
			    ourconf_finish(ocp) ;
			}
		    }
		}
		if (rs < 0) {
		    uc_free(p) ;
		    pip->config = NULL ;
		}
	    } /* end if (m-a) */
	}
	return rs ;
}
/* end subroutine (procourconf_begin) */


static int procourconf_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->config != NULL) {
	    OURCONF	*ocp = pip->config ;
	    rs1 = ourconf_finish(ocp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(pip->config) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->config = NULL ;
	}
	return rs ;
}
/* end subroutine (procourconf_end) */


static int procourconf_cfname(PROGINFO *pip)
{
	vecstr		*slp = &pip->svars ;
	int		rs = SR_OK ;
	int		rs1 ;
	cchar		*cfn = pip->cfname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("main/loadparams: conf=%s\n",pip->cfname) ;
	    debugprintf("main/loadparams: defprog=%s\n",pip->defprog) ;
	}
#endif

/* configuration file */

	if ((cfn == NULL) || ((cfn[0] != '\0') && (cfn[0] != '-'))) {
	    const int	nrs = SR_NOTFOUND ;
	    const int	tlen = MAXPATHLEN ;
	    cchar	*defcfn = CONFIGFNAME ;
	    char	tbuf[MAXPATHLEN + 1] = { 0 } ;

	    if ((cfn != NULL) && (cfn[0] == '+')) cfn = NULL ;

	    rs1 = SR_OK ;
	    if (cfn == NULL) {
	        cchar		**cs = schedhconf ;
	        if ((rs1 = permsched(cs,slp,tbuf,tlen,defcfn,R_OK)) == nrs) {
	            cchar	**cs = schedpconf ;
	            rs1 = permsched(cs,slp,tbuf,tlen,defcfn,R_OK) ;
	        }
	    }
	    if (rs1 == nrs) {
	        int	i ;
	        cchar	*apr ;
	        for (i = 0 ; (rs >= 0) && (altprs[i] != NULL) ; i += 1) {
	            apr = altprs[i] ;
	            if ((rs = vecstr_envset(slp,"a",apr,-1)) >= 0) {
	                rs1 = permsched(schedaconf,slp,tbuf,tlen,cfn,R_OK) ;
	                if (rs1 > 0) break ;
	            }
	        } /* end for */
	    } /* end if */

	    if ((rs >= 0) && (rs1 >= 0)) {
	        cchar	**vpp = &pip->cfname ;
	        rs = proginfo_setentry(pip,vpp,tbuf,rs1) ;
	    }

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/loadparams: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procourconf_cfname) */


static int procourconf_getdefprog(PROGINFO *pip,cchar **rpp)
{
	int		rs = SR_OK ;
	if (pip->config != NULL) {
	   OURCONF	*ocp = pip->config ;
	   rs = ourconf_getdefprog(ocp,rpp) ;
	}
	return rs ;
}
/* end subroutine (procourconf_getdefprog) */


static int procdefs_begin(PROGINFO *pip)
{
	vecstr		*dlp = &pip->defs ;
	int		rs ;
	if ((rs = vecstr_start(dlp,DEFNDEFS,0)) >= 0) {
	    pip->open.defs = TRUE ;
	    if ((pip->dfname != NULL) && (pip->dfname != '\0')) {
	        rs = loaddefsfile(pip,pip->dfname) ;
	    }
	    if (rs >= 0) {
	        if ((rs = loaddefs(pip,schedhfile)) >= 0) {
	    	    if ((rs = loaddefs(pip,schedpfile)) >= 0) {
			rs = vecstr_sort(dlp,vstrkeycmp) ;
		    }
		}
	    }
	    if (rs < 0) {
	        pip->open.defs = FALSE ;
		vecstr_finish(dlp) ;
	    }
	} /* end if (vecstr) */
	return rs ;
}
/* end subroutine (procdefs_begin) */


static int procdefs_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->open.defs) {
	    pip->open.defs = FALSE ;
	    rs1 = vecstr_finish(&pip->defs) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (procdefs_end) */


static int procenvs_begin(PROGINFO *pip)
{
	ENVS		*xlp = &pip->xenvs ;
	int		rs ;
	if ((rs = envs_start(xlp,DEFNXENVS)) >= 0) {
	    pip->open.envs = TRUE ;
	    if ((pip->xfname != NULL) && (pip->xfname[0] != '\0')) {
	        rs = loadxfile(pip,pip->xfname) ;
	    }
	    if (rs >= 0) {
		if (pip->f.xeall) {
	    	    if ((rs = loadxsched(pip,schedpfile)) >= 0) {
	        	rs = loadxsched(pip,schedhfile) ;
		    }
		}
	    }
	    if (rs < 0) {
	        pip->open.envs = FALSE ;
		envs_finish(xlp) ;
	    }
	}
	return rs ;
}
/* end subroutine (procenvs_begin) */


static int procenvs_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->open.envs) {
	    pip->open.envs = FALSE ;
	    rs1 = envs_finish(&pip->xenvs) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (procenvs_end) */


static int procexps_begin(PROGINFO *pip,cchar *pvfname)
{
	const int	vo = VECSTR_OCOMPACT ;
	int		rs ;
	if ((rs = vecstr_start(&pip->exports,DEFNXENVS,vo)) >= 0) {
	    pip->open.exports = TRUE ;
	    rs = pvarstart(pip,pathvars,pvfname) ;
	    if (rs < 0) {
	        pip->open.exports = FALSE ;
	        vecstr_finish(&pip->exports) ;
	    }
	}
	return rs ;
}
/* end subroutine (procexps_begin) */


static int procexps_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	rs1 = pvarfinish(pip) ;
	if (rs >= 0) rs = rs1 ;
	if (pip->open.exports) {
	    pip->open.exports = FALSE ;
	    rs1 = vecstr_finish(&pip->exports) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (procexps_end) */


static int procloadinfo(PROGINFO *pip)
{
	int		rs ;
	if ((rs = loadtz(pip)) >= 0) {
	    if ((rs = loadgroupname(pip)) >= 0) {
	        if ((rs = loadaux(pip)) >= 0) {
	    	    if ((rs = loadhz(pip)) >= 0) {
	    		if ((rs = loadncpu(pip)) >= 0) {
	    		    if ((rs = loadnodeinfo(pip)) >= 0) {
	    			rs = loadorganization(pip) ;
			    }
			}
		    }
		}
	    }
	}
	return rs ;
}
/* end subroutine (procloadinfo) */


/* ARGSUSED */
static int process(PROGINFO *pip,ARGINFO *aip,PARAMOPT *pop,
		cchar *svdb,cchar *prog)
{
	ENVS		*enp = &pip->xenvs ;
	vecstr		*exp = &pip->exports ;
	int		rs ;
	if ((rs = envs_subs(enp,&pip->cooks,&pip->pvars,exp)) >= 0) {
	    if ((rs = procenvuser(pip)) >= 0) {
		if (pip->f.xextra) {
	    	    rs = procenvextra(pip) ;
		}
		if (rs >= 0) {
	    	    if ((rs = procenvdef(pip)) >= 0) {
#if	CF_SYSVAR
	    		rs = procenvsysvar(pip,svdb) ;
#else
			rs = 3 ;
#endif
#if	CF_ENVSORT
			if (rs >= 0) {
	    		    rs = procenvsort(pip) ;
			}
#endif /* CF_ENVSORT */
			if (rs >= 0) {
			    rs = processor(pip,aip,prog) ;
			}
		    }
		}
	    }
	} /* end if (envs_subs) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/process: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (process) */


static int processor(PROGINFO *pip,ARGINFO *aip,cchar *prog)
{
	int		rs ;
	cchar		*progfname ;
	char		pbuf[MAXPATHLEN+1] ;
	if ((prog != NULL) && hasallplusminus(prog,-1)) {
	    if ((! pip->f.progdash) && (hasallminus(prog,-1))) {
	        pip->f.progdash = TRUE ;
	    }
	    prog = NULL ;
	}
	progfname = prog ;
	if ((rs = progfindprog(pip,pbuf,prog)) >= 0) {
	    if (rs > 0) progfname = pbuf ;
	    if ((rs = procnopreload(pip)) >= 0) {
	        if (pip->efp != NULL) {
		    rs = bflush(pip->efp) ;
	        }
	        if ((rs >= 0) && (pip->niceval > 0)) {
	            u_nice(pip->niceval) ;
	        }
	        if (rs >= 0) {
		    const int	argr = aip->argr ;
		    const int	argi = aip->argi ;
		    cchar	**argv = aip->argv ;
	            rs = progexec(pip,progfname,(argv + argi),argr) ;
		    if (rs < 0) {
			cchar	*pn = pip->progname ;
			cchar	*fmt ;
			fmt = "%s: exec failed (%d)\n" ;
			bprintf(pip->efp,fmt,pn,rs) ;
			fmt = "%s: prog=%s\n" ;
			bprintf(pip->efp,fmt,pn,progfname) ;
		    }
	        }
	    } /* end if (procnopreload) */
	} /* end if (progfindprog) */
	return rs ;
}
/* end subroutine (processor) */


static int procnopreload(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		f = FALSE ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("main/procnopreload: ent\n") ;
	    debugprintf("main/procnopreload: nopreload=%u\n",pip->f.nopreload) ;
	    debugprintf("main/procnopreload: init-cf=%u\n",
		(pip->config != NULL)) ;
	}
#endif
	if (pip->have.nopreload) {
	    int	f_elide = pip->f.nopreload ;
	    if (! f_elide) {
		if (pip->config != NULL) {
	            OURCONF	*ocp = pip->config ;
	            if ((rs = ourconf_ifnopreload(ocp)) > 0) {
			f_elide = TRUE ;
		    }
		}
	    }
	    if ((rs >= 0) && f_elide) {
		VECSTR		*elp = &pip->exports ;
		const int	rsn = SR_NOTFOUND ;
		cchar		*k = "LD_PRELOAD" ;
	    	if ((rs = vecstr_search(elp,k,vstrkeycmp,NULL)) >= 0) {
#if	CF_DEBUG
		    if (DEBUGLEVEL(5))
		        debugprintf("main/procnopreload: del LD_RELOAD\n") ;
#endif
		    f = TRUE ;
		    rs = vecstr_del(elp,rs) ;
		} else if (rs == rsn) {
		    rs = SR_OK ;
		}
	    } /* end if (active) */
	} /* end if (checking required) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procnopreload) */


static int procsched_begin(PROGINFO *pip)
{
	VECSTR		*svp = &pip->svars ;
	int		rs ;

	if ((rs = vecstr_start(svp,6,0)) >= 0) {
	    cchar	*keys = "phsn" ;
	    int		i ;
	    pip->open.svars = TRUE ;
	    for (i = 0 ; keys[i] != '\0' ; i += 1) {
	        const int	kc = MKCHAR(keys[i]) ;
	        cchar		*vp = NULL ;
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
	            char	kbuf[2] = "×" ;
	            kbuf[0] = kc ;
	            rs = vecstr_envadd(svp,kbuf,vp,vl) ;
	        }
	        if (rs < 0) break ;
	    } /* end for */
	    if (rs < 0) {
	        pip->open.svars = FALSE ;
	        vecstr_finish(svp) ;
	    }
	} /* end if (vecstr_start) */

	return rs ;
}
/* end subroutine (procsched_begin) */


static int procsched_end(PROGINFO *pip)
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
/* end subroutine (procsched_end) */


static int pvarstart(PROGINFO *pip,cchar **pathvars,cchar *fname)
{
	vecstr		*pvp = &pip->pvars ;
	int		rs ;
	if ((rs = vecstr_start(pvp,10,0)) >= 0) {
	    pip->open.pvars = TRUE ;
	    if (rs >= 0) rs = loadpvars(pip,schedpfile,fname) ;
	    if (rs >= 0) rs = loadpvars(pip,schedhfile,fname) ;
	    if (rs >= 0) rs = loadpvarsdef(pip,pathvars) ;
	    if (rs < 0) {
	        pip->open.pvars = FALSE ;
		vecstr_finish(pvp) ;
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/pvarstart: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (pvarstart) */


static int pvarfinish(PROGINFO *pip)
{
	vecstr		*pvp = &pip->pvars ;
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->open.pvars) {
	    pip->open.pvars = FALSE ;
	    rs1 = vecstr_finish(pvp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (pvarfinish) */


static int loadpvars(PROGINFO *pip,cchar **sched,cchar *fname)
{
	VECSTR		*pvp = &pip->pvars ;
	const int	tlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	char		tbuf[MAXPATHLEN + 1] ;

	if ((rs1 = permsched(sched,&pip->svars,tbuf,tlen,fname,R_OK)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/loadpvars: tbuf=%s\n",tbuf) ;
#endif

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: pvar=%s\n",pip->progname,tbuf) ;

	    rs = vecstr_loadfile(pvp,1,tbuf) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/loadpvars: vecstr_loadfile() rs=%d\n",rs) ;
#endif

	}

	return (rs >= 0) ? (rs1 >= 0) : rs ;
}
/* end subroutine (loadpvars) */


static int loadpvarsdef(PROGINFO *pip,cchar **pnames)
{
	vecstr		*pvp = &pip->pvars ;
	int		rs ;
	int		i = 0 ;

	if ((rs = vecstr_count(pvp)) == 0) {
	    for (i = 0 ; pnames[i] != NULL ; i += 1) {
	        rs = vecstr_adduniq(pvp,pnames[i],-1) ;
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (loading default names) */

#if	CF_DEBUGS
	if (DEBUGLEVEL(3))
	debugprintf("main/loadpvarsdef: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (loadpvarsdef) */


static int loadtz(PROGINFO *pip)
{
	const pid_t	sid = getsid(0) ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("loadtz: ent tz=%s\n",pip->tz) ;
#endif

	if ((pip->tz == NULL) || (pip->tz[0] == '\0') || (sid == pip->pid)) {
	    USERATTR	ua ;
	    const int	alen = TZLEN ;
	    int		al = 0 ;
	    char	abuf[TZLEN+1] = { 0 } ;

	    if ((rs = userattr_open(&ua,pip->username)) >= 0) {
	        if ((rs = userattr_lookup(&ua,abuf,alen,UA_TZ)) >= 0) {
	            al = rs ;
	        } else if (isNotPresent(rs))
	            rs = SR_OK ;
	        userattr_close(&ua) ;
	    } /* end if (userattr) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("loadtz: UA al=%d abuf=%s\n",al,abuf) ;
#endif

	    if (rs >= 0) {

	        if ((pip->tz == NULL) || (pip->tz[0] == '\0')) {
	            if (abuf[0] == '\0') {
	                rs = inittimezone(abuf,alen,NULL) ;
	                al = rs ;
	            }
	        } /* end if */

	        if ((rs >= 0) && (abuf[0] != '\0')) {
	            cchar	**vpp = &pip->tz ;
	            rs = proginfo_setentry(pip,vpp,abuf,al) ;
	        }

	    } /* end if (ok) */

	} /* end if (need TZ) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("loadtz: ret rs=%d\n",rs) ;
	    debugprintf("loadtz: ret tz=%s\n",pip->tz) ;
	}
#endif

	return rs ;
}
/* end subroutine (loadtz) */


static int loadgroupname(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		gl = 0 ;

	if ((pip->groupname == NULL) || (pip->groupname[0] == '\0')) {
	    const int	gnlen = GROUPNAMELEN ;
	    char	gnbuf[GROUPNAMELEN + 1] ;
	    if ((rs = getgroupname(gnbuf,gnlen,pip->gid)) >= 0) {
	        cchar	**vpp = &pip->groupname ;
	        gl = rs ;
	        rs = proginfo_setentry(pip,vpp,gnbuf,gl) ;
	    }
	} else
	    gl = strlen(pip->groupname) ;

	return (rs >= 0) ? gl : rs ;
}
/* end subroutine (loadgroupname) */


static int loadaux(PROGINFO *pip)
{
	UINFO_AUX	ua ;
	int		rs ;

	if ((rs = uinfo_aux(&ua)) >= 0) {
	    int		i ;
	    cchar	*cp ;
	    cchar	*vn ;
	    cchar	*vp ;
	    cchar	**vpp ;
	    for (i = 0 ; (rs >= 0) && (i < 5) ; i += 1) {
	        vp = NULL ;
	        vn = envdefs[i] ;
	        switch (i) {
	        case envdef_architecture:
	            vpp = &pip->architecture ;
	            vp = ua.architecture ;
	            break ;
	        case envdef_platform:
	            vpp = &pip->platform ;
	            vp = ua.platform ;
	            break ;
	        case envdef_provider:
	            vpp = &pip->provider ;
	            vp = ua.provider ;
	            break ;
	        case envdef_hwserial:
	            vpp = &pip->hwserial ;
	            vp = ua.hwserial ;
	            break ;
	        case envdef_nisdomain:
	            vpp = &pip->nisdomainname ;
	            vp = ua.nisdomain ;
	            break ;
	        } /* end switch */
	        if ((cp = getourenv(pip->envv,vn)) != NULL) vp = cp ;
	        if (vp != NULL) {
	            rs = proginfo_setentry(pip,vpp,vp,-1) ;
	        }
	    } /* end for */
	} /* end if (uinfo_aux) */

	return rs ;
}
/* end subroutine (loadaux) */


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
	    } else
	        cp = NULL ;
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
	    } else
	        rs = SR_NOSYS ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/loadncpu: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (loadncpu) */


static int loadnodeinfo(PROGINFO *pip)
{
	int		rs = SR_OK ;
	cchar		*cluster = getourenv(pip->envv,VARCLUSTER) ;
	cchar		*system = getourenv(pip->envv,VARSYSTEM) ;
	char		cbuf[NODENAMELEN+1] = { 0 	} ;
	char		sbuf[NODENAMELEN+1] = { 0 	} ;

	if ((cluster == NULL) || (system == NULL)) {
	    cchar	*nn = pip->nodename ;
	    if ((rs = getnodeinfo(pip->pr,cbuf,sbuf,NULL,nn)) >= 0) {
	        if (cluster == NULL) cluster = cbuf ;
	        if (system == NULL) system = sbuf ;
	    } else if (isNotPresent(rs) || (rs == SR_OVERFLOW))
	        rs = SR_OK ;
	} /* end if (getnodeinfo) */

	if ((rs >= 0) && (cluster != NULL)) {
	    cchar	**vpp = &pip->clustername ;
	    rs = proginfo_setentry(pip,vpp,cluster,-1) ;
	}

	if ((rs >= 0) && (system != NULL)) {
	    cchar	**vpp = &pip->systemname ;
	    rs = proginfo_setentry(pip,vpp,system,-1) ;
	}

	return rs ;
}
/* end subroutine (loadnodeinfo) */


static int loadorganization(PROGINFO *pip)
{
	const int	orglen = MAXNAMELEN ;
	int		rs = SR_OK ;
	cchar		*orgp = pip->org ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/loadorg: entered org=>%s<\n",pip->org) ;
#endif

	if ((orgp == NULL) || (orgp[0] == '\0')) {
	    char	orgbuf[MAXNAMELEN+1] ;
	    cchar	*cp ;
	    int		cl = 0 ;
	    cp = orgbuf ;
#ifdef	COMMENT /* USERINFO already does this function completely */
	    rs = getuserorg(orgbuf,orglen,pip->username) ;
	    cl = rs ;
	    if ((rs == SR_NOENT) || (rs == SR_ACCESS)) {
	        rs = SR_OK ;
	        cl = 0 ;
	    }
#endif /* COMMENT */
	    if ((rs >= 0) && (cl == 0)) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/loadorg: localgetorg()\n") ;
#endif
	        rs = localgetorg(pip->pr,orgbuf,orglen,pip->username) ;
	        cl = rs ;
	        if (rs >= 0) cp = orgbuf ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/loadorg: localgetorg() rs=%d org=>%s<\n",
	                rs,orgbuf) ;
#endif
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

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/loadorg: rs=%d\n",rs) ;
	    debugprintf("main/loadorg: org=>%s<\n",pip->org) ;
	}
#endif

	return rs ;
}
/* end subroutine (loadorganization) */


static int loadcooks(PROGINFO *pip)
{
	EXPCOOK		*ckp = &pip->cooks ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ci ;
	int		cl ;
	cchar		*cp ;
	char		tbuf[USERNAMELEN+1] = { 0 	} ;
	char		nbuf[USERNAMELEN+1] = { 0 	} ;

	for (ci = 0 ; cooks[ci] != NULL ; ci += 1) {
	    cp = NULL ;
	    cl = -1 ;
	    switch (ci) {
	    case cook_sysname:
	        cp = pip->usysname ;
	        break ;
	    case cook_release:
	        cp = pip->urelease ;
	        break ;
	    case cook_version:
	        cp = pip->uversion ;
	        break ;
	    case cook_machine:
	        cp = pip->umachine ;
	        break ;
	    case cook_architecture:
	        cp = pip->architecture ;
	        break ;
	    case cook_platform:
	        cp = pip->platform ;
	        break ;
	    case cook_provider:
	        cp = pip->provider ;
	        break ;
	    case cook_hwserial:
	        cp = pip->hwserial ;
	        break ;
	    case cook_nisdomain:
	        cp = pip->nisdomainname ;
	        break ;
	    case cook_ncpu:
	        {
	            char	digbuf[DIGBUFLEN + 1] ;
	            if (pip->ncpu >= 0) {
	                rs1 = ctdeci(digbuf,DIGBUFLEN,pip->ncpu) ;
	            } else {
	                strcpy(digbuf,"1") ;
	                rs1 = 1 ;
	            }
	            rs = expcook_add(ckp,cooks[ci],digbuf,rs1) ;
	        } /* end block */
	        break ;
	    case cook_hz:
	        cp = pip->hz ;
	        break ;
	    case cook_cluster:
	        cp = pip->clustername ;
	        break ;
	    case cook_system:
	        cp = pip->systemname ;
	        break ;
	    case cook_u:
	        cp = pip->username ;
	        break ;
	    case cook_g:
	        cp = pip->groupname ;
	        break ;
	    case cook_home:
	        cp = pip->homedname ;
	        break ;
	    case cook_shell:
	        cp = pip->shell ;
	        break ;
	    case cook_organization:
	        cp = pip->org ;
	        break ;
	    case cook_gecosname:
	        cp = pip->gecosname ;
	        break ;
	    case cook_realname:
	        cp = pip->realname ;
	        break ;
	    case cook_name:
	        cp = pip->name ;
	        break ;
	    case cook_tz:
	        cp = pip->tz ;
	        break ;
	    case cook_n:
	        cp = pip->nodename ;
	        break ;
	    case cook_d:
	        cp = pip->domainname ;
	        break ;
	    case cook_h:
	        {
	            cchar	*nn = pip->nodename ;
	            cchar	*dn = pip->domainname ;
	            char	hnbuf[MAXHOSTNAMELEN + 1] ;
	            if ((rs = snsds(hnbuf,MAXHOSTNAMELEN,nn,dn)) >= 0)
	                rs = expcook_add(ckp,cooks[ci],hnbuf,rs) ;
	        } /* end block */
	        break ;
	    case cook_r:
	        cp = pip->pr ;
	        break ;
	    case cook_rn:
	        cp = pip->rootname ;
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
	                cp = tbuf ;
	                break ;
	            case cook_osnum:
	                cp = nbuf ;
	                break ;
	            } /* end switch */
	        } /* end if (ok) */
	        break ;
	    case cook_s:
	        cp = pip->searchname ;
	        break ;
	    } /* end switch */
	    if ((rs >= 0) && (cp != NULL)) {
	        rs = expcook_add(ckp,cooks[ci],cp,cl) ;
	    }
	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (loadcooks) */


static int loaddefsfile(PROGINFO *pip,cchar *dfname)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	cchar		**envv = pip->envv ;

	if ((rs1 = uc_stat(dfname,&sb)) >= 0) {
	    rs1 = SR_NOENT ;
	    if (S_ISREG(sb.st_mode)) rs1 = SR_OK ;
	}

	if (rs1 >= 0) {
	    if ((rs1 = sperm(&pip->id,&sb,R_OK)) >= 0) {
	        if (pip->debuglevel > 0) {
	            cchar	*pn = pip->progname ;
	            bprintf(pip->efp,"%s: def=%s\n",pn,dfname) ;
	        }
	        rs = defproc(&pip->defs,envv,&pip->cooks,dfname) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (loaddefsfile) */


static int loaddefs(PROGINFO *pip,cchar *sched[])
{
	const int	tlen = MAXPATHLEN ;
	int		rs ;
	int		f = FALSE ;
	cchar		*de = DEFSFNAME ;
	char		tbuf[MAXPATHLEN + 1] ;

	if ((rs = permsched(sched,&pip->svars,tbuf,tlen,de,R_OK)) >= 0) {
	    EXPCOOK	*clp = &pip->cooks ;
	    cchar	**envv = (cchar **) pip->envv ;
	    f = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/loaddefs: dfname=%s\n",tbuf) ;
#endif

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: def=%s\n",pip->progname,tbuf) ;

	    rs = defproc(&pip->defs,envv,clp,tbuf) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/loaddefs: defproc() rs=%d\n",rs) ;
#endif

	} else if (isNotPresent(rs))
	    rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/loaddefs: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (loaddefs) */


static int loadxfile(PROGINFO *pip,cchar xfname[])
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;
	cchar		**envv = (cchar **) pip->envv ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("envset/loadxfile: ent xfname=%s\n",xfname) ;
#endif

	rs1 = uc_stat(xfname,&sb) ;

	if ((rs1 >= 0) && (! S_ISREG(sb.st_mode)))
	    rs1 = SR_NOENT ;

	if (rs1 >= 0)
	    rs1 = sperm(&pip->id,&sb,R_OK) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("envset/loadxfile: sperm() rs=%d\n",rs1) ;
#endif

	f = (rs1 >= 0) ;
	if (rs1 >= 0) {
	    EXPCOOK	*clp = &pip->cooks ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: xe=%s\n",pip->progname,xfname) ;

	    rs = envs_procxe(&pip->xenvs,clp,envv,&pip->defs,xfname) ;

	} /* end if */

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
	cchar		*xe = XEFNAME ;
	char		tbuf[MAXPATHLEN + 1] ;

	if ((rs = permsched(sched,&pip->svars,tbuf,tlen,xe,R_OK)) >= 0) {
	    EXPCOOK	*clp = &pip->cooks ;
	    cchar	**envv = (cchar **) pip->envv ;
	    f = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/loadxsched: xfname=%s\n",tbuf) ;
#endif

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: xe=%s\n",pip->progname,tbuf) ;

	    rs = envs_procxe(&pip->xenvs,clp,envv,&pip->defs,tbuf) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/loadxsched: envs_procxe() rs=%d\n",rs) ;
#endif

	} else if (isNotPresent(rs))
	    rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/loadxsched: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (loadxsched) */


static int ourconf_start(OURCONF *ocp,PROGINFO *pip,PARAMOPT *pop)
{
	memset(ocp,0,sizeof(OURCONF)) ;
	ocp->pip = pip ;
	ocp->pop = pop ;
	return SR_OK ;
}
/* end subroutine (ourconf_start) */


static int ourconf_finish(OURCONF *ocp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ocp->open.stores) {
	    ocp->open.stores = FALSE ;
	    rs1 = vecstr_finish(&ocp->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (ourconf_finish) */


static int ourconf_setentry(OURCONF *lip,cchar **epp,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! lip->open.stores) {
	    rs = vecstr_start(&lip->stores,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
		oi = vecstr_findaddr(&lip->stores,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(&lip->stores,vp,len,epp) ;
	    } else {
	        *epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(&lip->stores,oi) ;
	    }
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (ourconf_setentry) */


static int ourconf_read(OURCONF *ocp)
{
	PROGINFO	*pip = ocp->pip ;
	PARAMFILE	pf ;
	PARAMFILE_CUR	cur ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	cchar		**ev ;
	cchar		*cfname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("main/ourconf_read: ent\n") ;
#endif

	ev = (cchar **) pip->envv ;
	cfname = pip->cfname ;
	if ((rs = paramfile_open(&pf,ev,cfname)) >= 0) {
	    IDS		*idp = &pip->id ;
	    const int	elen = EBUFLEN ;
	    const int	vlen = VBUFLEN ;
	    int		oi ;
	    int		vl, el ;
	    cchar	*ccp ;
	    char	vbuf[VBUFLEN + 1] ;
	    char	ebuf[EBUFLEN + 1] ;

	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: conf=%s\n",pip->progname,cfname) ;	
	    }

	    for (oi = 0 ; cparams[oi] != NULL ; oi += 1) {
	        if ((rs = paramfile_curbegin(&pf,&cur)) >= 0) {
	            while (rs >= 0) {

	                ccp = cparams[oi] ;
	                vl = paramfile_fetch(&pf,ccp,&cur,vbuf,vlen) ;
	                if (vl == SR_NOTFOUND) break ;
	                rs = vl ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("main/ourconf_read: k=%s v=%t\n",cparams[oi],vbuf,vl) ;
#endif

	                ebuf[0] = '\0' ;
	                el = 0 ;
	                if ((rs >= 0) && (vl > 0)) {
	                    el = expcook_exp(&pip->cooks,0,ebuf,elen,vbuf,vl) ;
	                }

	                if ((rs >= 0) && (el > 0)) {
	                    switch (oi) {
	                    case cparam_defprog:
	                        if (el > 0) {
	                            c += 1 ;
				    rs = ourconf_defprog(ocp,ebuf,el) ;
	                        } /* end if */
	                        break ;
	                    case cparam_org:
	                        if (el > 0) {
	                            cchar	*op = pip->org ;
	                            if ((op == NULL) || (op[0] == '\0')) {
	                                cchar	**vpp = &pip->org ;
	                                cchar	*eb = ebuf ;
	                                c += 1 ;
	                                rs = proginfo_setentry(pip,vpp,eb,el) ;
	                            }
	                        }
	                        break ;
	                    case cparam_shell:
	                        if ((el > 0) && (pip->defprog == NULL)) {
	                            char	tbuf[MAXPATHLEN+1] ;
	                            mkpath1w(tbuf,ebuf,el) ;
	                            if ((rs1 = xfile(idp,tbuf)) >= 0) {
	                                cchar	**vpp = &pip->shprog ;
	                                cchar	*eb = ebuf ;
	                                c += 1 ;
	                                rs = proginfo_setentry(pip,vpp,eb,el) ;
	                            }
	                        }
	                        break ;
	                    case cparam_nopreload:
	                        if (el > 0) {
				    cchar	*eb = ebuf ;
	                            c += 1 ;
				    pip->have.nopreload = TRUE ;
	                            rs = ourconf_nopreload(ocp,eb,el) ;
	                        }
	                        break ;
	                    } /* end switch */
	                } /* end if */

	            } /* end while */
	            rs1 = paramfile_curend(&pf,&cur) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (paramfile-cur) */
	        if (rs < 0) break ;
	    } /* end for */
	    rs1 = paramfile_close(&pf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (parameter file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/loadparamread: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? rs : c ;
}
/* end subroutine (ourconf_read) */


static int ourconf_defprog(OURCONF *ocp,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	if ((vp != NULL) && (ocp->defprog == NULL)) {
	    PROGINFO	*pip = ocp->pip ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath1w(tbuf,vp,vl)) >= 0) {
		IDS		*idp = &pip->id ;
		const int	tl = rs ;
	        if ((rs = xfile(idp,tbuf)) >= 0) {
	    	    cchar	**vpp = &ocp->defprog ;
	    	    rs = ourconf_setentry(ocp,vpp,tbuf,tl) ;
		} else if (isNotPresent(rs)) {
		    rs = SR_OK ;
		}
	    }
	}
	return rs ;
}
/* end suroutine (ourconf_defprog) */


static int ourconf_nopreload(OURCONF *ocp,cchar *sp,int sl)
{
	PROGINFO	*pip = ocp->pip ;
	PARAMOPT	*pop = ocp->pop ;
	int		rs ;
	cchar		*po = PO_NOPRELOAD ;
	if (pip == NULL) return SR_FAULT ; /* lint */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("main/ourconf_nopreload: s=%t\n",sp,sl) ;
#endif
	rs = paramopt_loads(pop,po,sp,sl) ;
	return rs ;
}
/* end subroutine (ourconf_nopreload) */


static int ourconf_getdefprog(OURCONF *ocp,cchar **rpp)
{
	int		rs = SR_OK ;
	if (rpp != NULL) {
	    *rpp = ocp->defprog ;
	}
	return rs ;
}
/* end subroutine (ourconf_getdefprog) */


static int ourconf_ifnopreload(OURCONF *ocp)
{
	PROGINFO	*pip = ocp->pip ;
	PARAMOPT	*pop = ocp->pop ;
	PARAMOPT_CUR	cur ;
	int		rs  ;
	int		rs1 ;
	int		f = FALSE ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("main/ifnopreload: ent\n") ;
#endif
	if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {
	    const uid_t	uid = pip->uid ;
	    const int	rsn = SR_NOTFOUND ;
	    int		vl ;
	    cchar	*po = PO_NOPRELOAD ;
	    cchar	*vp ;
	    while ((rs1 = paramopt_enumvalues(pop,po,&cur,&vp)) >= 0) {
		vl = rs1 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("main/ifnopreload: v=%t",vp,vl) ;
#endif
		if ((vl > 0) && (vp != NULL)) {
		    if ((rs = getuid_user(vp,vl)) >= 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("main/ifnopreload: getuid_user() rs=%d\n",rs) ;
#endif
			uid_t	u = rs ;
			f = (u == uid) ;
		    } else if (rs == rsn) {
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("main/ifnopreload: getuid_user() rs=%d\n",rs) ;
#endif
			rs = SR_OK ;
		    }
		}
		if (f) break ;
		if (rs < 0) break ;
	    } /* end while */
	    if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	    rs1 = paramopt_curend(pop,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (paramopt-cur) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("main/ifnopreload: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (ourconf_ifnopreload) */


