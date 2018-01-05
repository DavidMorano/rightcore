/* b_userinfo */

/* SHELL built-in: return various user information */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_DEFGROUP	0		/* for compatibility w/ 'id(1)' */
#define	CF_AUID		0		/* can handle Audit-UID */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  It was inspired by many
	programs that performs various subset functions of this program.  This
	can be either a KSH builtin or a stand-alone program.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This command queries and returns numerous values associated with a
	particular system user.

	Synopsis:

	$ userinfo [[<username>|-] <qkey(s)>] [-af <qkeyfile>]


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
#include	<sys/task.h>
#include	<sys/statvfs.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>
#include	<project.h>
#include	<netdb.h>

#if	SYSHAS_AUDIT
#include	<bsm/audit.h>
#endif

#include	<vsystem.h>
#include	<ugetpid.h>
#include	<getbufsize.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<field.h>
#include	<vecstr.h>
#include	<sbuf.h>
#include	<realname.h>
#include	<uinfo.h>
#include	<ugetpw.h>
#include	<getax.h>
#include	<getxusername.h>
#include	<pwfile.h>
#include	<getpwentry.h>
#include	<getutmpent.h>
#include	<pcsns.h>
#include	<lastlogfile.h>
#include	<userattr.h>
#include	<ctdec.h>
#include	<filebuf.h>
#include	<sysgroup.h>
#include	<sysproject.h>
#include	<tmpx.h>
#include	<pwentry.h>
#include	<estrings.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_userinfo.h"
#include	"defs.h"


/* local defines */

#ifndef	VARTZ
#define	VARTZ		"TZ"
#endif

#ifndef	VARNAME
#define	VARNAME		"NAME"
#endif

#ifndef	VARFULLNAME
#define	VARFULLNAME	"FULLNAME"
#endif

#ifndef	VARMAILNAME
#define	VARMAILNAME	"MAILNAME"
#endif

#ifndef	VARPRINTER
#define	VARPRINTER	"PRINTER"
#endif

#ifndef	ORGCODELEN
#define	ORGCODELEN	USERNAMELEN
#endif

#ifndef	ORGLOCLEN
#define	ORGLOCLEN	USERNAMELEN
#endif

#ifndef	CBUFLEN
#define	CBUFLEN		MAX(MAXPATHLEN,100)
#endif

#ifndef	DEVDNAME
#define	DEVDNAME	"/dev"
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	PROGDATA	struct progdata
#define	PROGDATA_FL	struct progdata_flags

#define	DATASYS		struct datasys
#define	DATASYS_FL	struct datasys_flags

#define	DATAUSER	struct datauser
#define	DATAUSER_FL	struct datauser_flags


/* external subroutines */

extern int	mkfmtphone(char *,int,cchar *,int) ;
extern int	pathadd(char *,int,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	udomain(cchar *,char *,int,cchar *) ;
extern int	getdomainname(char *,int,cchar *) ;
extern int	getnodedomain(char *,char *) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	getlogname(char *,int) ;
extern int	getloghost(char *,int,pid_t) ;
extern int	gethomeorg(char *,int,cchar *) ;
extern int	localgetorg(cchar *,char *,int,cchar *) ;
extern int	localgetorgcode(cchar *,char *,int,cchar *) ;
extern int	localgetorgloc(cchar *,char *,int,cchar *) ;
extern int	lastlogin(char *,uid_t,time_t *,char *,char *) ;
extern int	mkpr(char *,int,cchar *,cchar *) ;
extern int	statvfsdir(cchar *,struct statvfs *) ;
extern int	mkgecosname(char *,int,cchar *) ;
extern int	mkrealname(char *,int,cchar *,int) ;
extern int	mkmailname(char *,int,cchar *,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	getnodeinfo(cchar *,char *,char *,vecstr *,cchar *) ;
extern int	nisdomainname(char *,int) ;
extern int	inittimezone(char *,int,cchar *) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	tmpx_getuserlines(TMPX *,VECSTR *,cchar *) ;
extern int	hasalldig(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	pwilookup(cchar *,cchar *,char *,int,cchar *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strdcpy1(char *,int,cchar *) ;
extern char	*strdcpy1w(char *,int,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		ns:1 ;
	uint		phone:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	PROGINFO	*pip ;
	vecstr		stores ;
	PCSNS		ns ;
	cchar		*pr_pcs ;
	int		phone ;
} ;

struct datauser_flags {
	uint		pent:1 ;
	uint		gr:1 ;
	uint		pj:1 ;
	uint		groups:1 ;
	uint		projects:1 ;
	uint		ua:1 ;
	uint		ruid:1 ;
	uint		euid:1 ;
	uint		rgid:1 ;
	uint		egid:1 ;
	uint		projid:1 ;
	uint		eprojid:1 ;
	uint		domain:1 ;
	uint		logname:1 ;
	uint		netname:1 ;
	uint		tz:1 ;
	uint		dn:1 ;
	uint		statvfs:1 ;
	uint		lastlog:1 ;
	uint		utmpent:1 ;
} ;

struct datauser {
	PROGINFO	*pip ;
	cchar		*pr ;
	cchar		*un ;
	cchar		*netname ;
	cchar		*pwfname ;
	cchar		*a ;		/* aggregate memory-allocation */
	char		*pentbuf ;
	char		*grbuf ;
	char		*pjbuf ;
	DATAUSER_FL	init ;
	DATAUSER_FL	have ;
	struct statvfs	fss ;
	struct group	gr ;
	struct project	pj ;
	PWENTRY		pent ;
	GETUTMPENT	ue ;
	vecstr		groups ;
	vecstr		projects ;
	time_t		lasttime ;
	uid_t		ruid, euid ;
	gid_t		rgid, egid ;
	projid_t	projid ;
	projid_t	eprojid ;
	int		pentlen ;
	int		grlen ;
	int		pjlen ;
	char		unbuf[USERNAMELEN+1] ;
	char		domainname[MAXHOSTNAMELEN + 1] ;
	char		logname[LOGNAMELEN + 1] ;
	char		tz[TZLEN + 1] ;
	char		dn[MAXHOSTNAMELEN + 1] ;
	char		lasthost[LASTLOGFILE_LHOST + 1] ;
	char		lastline[LASTLOGFILE_LLINE + 1] ;
	char		lastseen[LASTLOGFILE_LLINE + 1] ;
	char		orgcode[ORGCODELEN+1] ;
	char		orgloc[ORGLOCLEN+1] ;
} ;

struct datasys_flags {
	uint		stores:1 ;
	uint		node:1 ;
	uint		system:1 ;
	uint		cluster:1 ;
	uint		domain:1 ;
	uint		nisdomain:1 ;
	uint		uname:1 ;
	uint		uaux:1 ;
} ;

struct datasys {
	PROGINFO	*pip ;
	vecstr		stores ;
	DATASYS_FL	f ;
	UINFO_NAME	uname ;
	UINFO_AUX	uaux ;
	cchar		*nodename ;
	cchar		*systemname ;
	cchar		*clustername ;
	cchar		*domainname ;
	cchar		*nisdomainname ;
} ;

struct progdata_flags {
	uint		self:1 ;
	uint		name:1 ;
	uint		domain:1 ;
	uint		host:1 ;
	uint		prpcs:1 ;
} ;

struct progdata {
	PROGINFO	*pip ;
	cchar		*pr ;
	cchar		*un ;		/* specified user-name */
	cchar		*domainname ;
	cchar		*pwfname ;
	PROGDATA_FL	f ;
	DATAUSER	du ;
	struct datasys	ds ;
	char		hostname[MAXHOSTNAMELEN + 1] ;
	char		prpcs[MAXPATHLEN + 1] ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,PROGDATA *,
			cchar *,cchar *) ;
static int	procqueries(PROGINFO *,PROGDATA *,void *,cchar *,int) ;
static int	procquery(PROGINFO *,PROGDATA *,void *,cchar *,int) ;
static int	procquery_name(PROGINFO *,PROGDATA *,char *,int) ;
static int	procquery_fullname(PROGINFO *,PROGDATA *,char *,int) ;
static int	procquery_netname(PROGINFO *,PROGDATA *,char *,int) ;
static int	procquery_projinfo(PROGINFO *,PROGDATA *,char *,int) ;
static int	procquery_projects(PROGINFO *,PROGDATA *,char *,int) ;
static int	procout(PROGINFO *,void *,cchar *,int) ;
static int	procgetns(PROGINFO *,char *,int,cchar *,int) ;

static int	progdata_start(PROGDATA *,PROGINFO *,cchar *,int, cchar *) ;
static int	progdata_host(PROGDATA *) ;
static int	progdata_domain(PROGDATA *) ;
static int	progdata_haveuser(PROGDATA *) ;
static int	progdata_finish(PROGDATA *) ;

static int	datasys_start(struct datasys *,PROGINFO *) ;
static int	datasys_finish(struct datasys *) ;
static int	datasys_setentry(DATASYS *,cchar **,cchar *,int) ;
static int	datasys_uname(struct datasys *) ;
static int	datasys_uaux(struct datasys *) ;
static int	datasys_node(struct datasys *) ;
static int	datasys_system(struct datasys *) ;
static int	datasys_cluster(struct datasys *) ;
static int	datasys_nodeinfo(struct datasys *) ;
static int	datasys_domain(struct datasys *) ;
static int	datasys_nisdomain(struct datasys *) ;

static int	datauser_start(DATAUSER *,PROGINFO *,cchar *,cchar *) ;
static int	datauser_ua(DATAUSER *) ;
static int	datauser_domain(DATAUSER *) ;
static int	datauser_pw(DATAUSER *) ;
static int	datauser_gr(DATAUSER *) ;
static int	datauser_pj(DATAUSER *) ;
static int	datauser_groups(DATAUSER *) ;
#if	CF_DEFGROUP
static int	datauser_groupdef(DATAUSER *) ;
#endif /* CF_DEFGROUP */
static int	datauser_groupsfind(DATAUSER *) ;
static int	datauser_projects(DATAUSER *) ;
static int	datauser_projectsfind(DATAUSER *) ;
static int	datauser_tz(DATAUSER *) ;
static int	datauser_lastlog(DATAUSER *) ;
static int	datauser_statvfs(DATAUSER *) ;
static int	datauser_utmpent(DATAUSER *) ;
static int	datauser_orgcode(DATAUSER *) ;
static int	datauser_orgloc(DATAUSER *) ;
static int	datauser_lastseen(DATAUSER *) ;
static int	datauser_lastseener(DATAUSER *,char *,int,vecstr *) ;
static int	datauser_username(DATAUSER *,char *,int) ;
static int	datauser_realname(DATAUSER *,char *,int) ;
static int	datauser_netname(DATAUSER *,cchar *) ;
static int	datauser_finish(DATAUSER *) ;
static int	datauser_mkgids(DATAUSER *,char *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_prpcs(LOCINFO *) ;
static int	locinfo_pcsns(LOCINFO *) ;
static int	locinfo_pcsnsget(LOCINFO *,char *,int,cchar *,int) ;
static int	locinfo_setphone(LOCINFO *,cchar *,int) ;

static int	mkstrlist(char *,int,vecstr *) ;
static int	mkgid(char *,int,cchar *) ;
static int	getuser(char *,int,uid_t) ;


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
	"if",
	"pwfile",
	"pwidb",
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
	argopt_if,
	argopt_pwfile,
	argopt_pwidb,
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
	{ SR_NOTUNIQ, EX_TEMPFAIL },
	{ 0, 0 }
} ;

static const char	*progopts[] = {
	"phone",
	NULL
} ;

enum progopts {
	progopt_phone,
	progopt_overlast
} ;

/* define the query keywords */
static const char	*qopts[] = {
	"sysname",
	"nodename",
	"release",
	"version",
	"machine",
	"architecture",
	"platform",
	"provider",
	"hwserial",
	"domainname",
	"hostname",
	"clustername",
	"systemname",
	"nisdomain",
	"inetdomain",
	"username",
	"uid",
	"gid",
	"gids",
	"gecos",
	"homedir",
	"shell",
	"lstchg",
	"gecosname",
	"mailname",
	"name",
	"pcsname",
	"fullname",
	"pcsfullname",
	"netname",
	"organization",
	"password",
	"passwd",
	"realname",
	"account",
	"bin",
	"office",
	"wphone",
	"hphone",
	"printer",
	"lastlog",
	"logid",
	"logline",
	"logname",
	"loghost",
	"logsession",
	"logdate",
	"groupname",
	"groups",
	"projectname",
	"projectinfo",
	"projects",
	"projname",
	"projinfo",
	"projid",
	"pjid",
	"tz",
	"udomain",
	"ruid",
	"rgid",
	"euid",
	"egid",
	"egids",
	"eprojid",
	"pid",
	"ppid",
	"pgid",
	"psid",
	"ptid",
	"sid",
	"tid",
	"eusername",
	"rgroupname",
	"egroupname",
	"egroups",
	"wstation",
	"fsbs",
	"fspbs",
	"fstotal",
	"fsavail",
	"fsfree",
	"fsused",
	"fsutil",
	"fstype",
	"fsstr",
	"fsid",
	"fsflags",
	"auid",
	"orgcode",
	"orgloc",
	"lastseen",
	"execname",
	NULL
} ;

enum qopts {
	qopt_sysname,
	qopt_nodename,
	qopt_release,
	qopt_version,
	qopt_machine,
	qopt_architecture,
	qopt_platform,
	qopt_provider,
	qopt_hwserial,
	qopt_domain,
	qopt_hostname,
	qopt_cluster,
	qopt_system,
	qopt_nisdomain,
	qopt_inetdomain,
	qopt_username,
	qopt_uid,
	qopt_gid,
	qopt_gids,
	qopt_gecos,
	qopt_homedir,
	qopt_shell,
	qopt_lstchg,
	qopt_gecosname,
	qopt_mailname,
	qopt_name,
	qopt_pcsname,
	qopt_fullname,
	qopt_pcsfullname,
	qopt_netname,
	qopt_organization,
	qopt_password,
	qopt_passwd,
	qopt_realname,
	qopt_account,
	qopt_bin,
	qopt_office,
	qopt_wphone,
	qopt_hphone,
	qopt_printer,
	qopt_lastlog,
	qopt_logid,
	qopt_logline,
	qopt_logname,
	qopt_loghost,
	qopt_logsession,
	qopt_logdate,
	qopt_group,
	qopt_groups,
	qopt_projectname,
	qopt_projectinfo,
	qopt_projects,
	qopt_projname,
	qopt_projinfo,
	qopt_projid,
	qopt_pjid,
	qopt_tz,
	qopt_udomain,
	qopt_ruid,
	qopt_rgid,
	qopt_euid,
	qopt_egid,
	qopt_egids,
	qopt_eprojid,
	qopt_pid,
	qopt_ppid,
	qopt_pgid,
	qopt_psid,
	qopt_ptid,
	qopt_sid,
	qopt_tid,
	qopt_eusername,
	qopt_rgroupname,
	qopt_egroupname,
	qopt_egroups,
	qopt_wstation,
	qopt_fsbs,
	qopt_fspbs,
	qopt_fstotal,
	qopt_fsavail,
	qopt_fsfree,
	qopt_fsused,
	qopt_fsutil,
	qopt_fstype,
	qopt_fsstr,
	qopt_fsid,
	qopt_fsflags,
	qopt_auid,
	qopt_orgcode,
	qopt_orgloc,
	qopt_lastseen,
	qopt_execname,
	qopt_overlast
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

static const char	*uakeys[] = {
	"tz",
	"dn",
	NULL
} ;

enum uakeys {
	uakey_tz,
	uakey_dn,
	uakey_overlast
} ;

static const char	*phonetypes[] = {
	"fancy",
	"plain",
	NULL
} ;

enum phonetypes {
	phonetype_fancy,
	phonetype_plain,
	phonetype_overlast
} ;


/* exported subroutines */


int b_userinfo(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_userinfo) */


int p_userinfo(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_userinfo) */


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
	uid_t		uid = getuid() ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	const int	unlen = USERNAMELEN ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos, ai_continue ;
	int		rs, rs1 ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_name = FALSE ;
	int		f_self = FALSE ;
	int		f ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*pwfname = NULL ;
	cchar		*pwidb = NULL ;
	cchar		*un = NULL ;
	cchar		*cp ;
	char		unbuf[USERNAMELEN + 1] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_userinfo: starting DFD=%d\n",rs) ;
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

/* password file */
	                case argopt_pwfile:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pwfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pwfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* inverse password ('ipasswd') file */
	                case argopt_pwidb:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pwidb = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pwidb = argp ;
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

	                    case 'n':
	                        f_name = TRUE ;
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
	    debugprintf("b_userinfo: debuglevel=%u\n",pip->debuglevel) ;
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
	    printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some preliminary initialization */

	if ((rs >= 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (ofname == NULL) ofname = getourenv(envv,VAROFNAME) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

	un = NULL ;
	ai_continue = 1 ;
	for (ai = ai_continue ; ai < argc ; ai += 1) {
	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        un = argv[ai] ;
	        ai_continue = (ai + 1) ;
	        break ;
	    }
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_userinfo: un=%s\n",un) ;
#endif

	if (rs >= 0) {
	if ((un != NULL) && (un[0] != '\0') && (strcmp(un,"-") != 0)) {

	    if (f_name) {
	        cchar	*pr = pip->pr ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_userinfo: pwidb=%s un=%s\n",pwidb,un) ;
#endif
	        rs = pwilookup(pr,pwidb,unbuf,unlen,un) ;
	        un = unbuf ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_userinfo: pwilookup() rs=%d un=%s\n",
	                rs,un) ;
#endif

	        if ((rs < 0) && (! pip->f.quiet)) {
	            cchar	*pn = pip->progname ;
	            cchar	*fmt ;
	            if (rs == SR_NOTUNIQ) {
	                fmt = "%s: multiple name matches\n" ;
	                shio_printf(pip->efp,fmt,pn) ;
	                ex = EX_TEMPFAIL ;
	            } else {
	                fmt = "%s: name not found (%d)\n" ;
	                shio_printf(pip->efp,fmt,pn,rs) ;
	                ex = EX_NOUSER ;
	            }
	        } /* end if */

	    } else if (hasalldig(un,-1)) {
	        if ((rs = cfdeci(un,-1,&v)) >= 0) {
	            uid_t	tuid = v ;
	            un = unbuf ;
	            rs = getusername(unbuf,USERNAMELEN,tuid) ;
	            if ((rs >= 0) && (tuid == uid)) f_self = TRUE ;
	        } /* end if (cfdec) */
	    } /* end if */

	} else {
	    f_self = TRUE ;
	    un = unbuf ;
	    rs = getusername(unbuf,USERNAMELEN,uid) ;
	}
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_userinfo: get rs=%d un=%s f_name=%u f_self=%u\n",
	        rs,un,f_name,f_self) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: username=%s\n",pip->progname,un) ;
	}

/* more initialization */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;
	ainfo.ai_continue = ai_continue ;

	if (rs >= 0) {
	    const int	size = sizeof(PROGDATA) ;
	    void	*p ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        PROGDATA	*pdp = p ;
	        cchar		*pfn = pwfname ;
	        if ((rs = progdata_start(pdp,pip,un,f_self,pfn)) >= 0) {
	            ARGINFO	*aip = &ainfo ;
	            BITS	*bop = &pargs ;
	            cchar	*afn = afname ;
	            cchar	*ofn = ofname ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("b_userinfo: un=%s f_name=%u "
	                    "f_self=%u\n", un,f_name,f_self) ;
#endif

	            if ((rs = procargs(pip,aip,bop,pdp,ofn,afn)) >= 0) {
	                rs = progdata_haveuser(pdp) ;
	            }

	            rs1 = progdata_finish(pdp) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (progdata) */
	        uc_free(pdp) ;
	    } /* end if (m-a) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_userinfo: progdata-out rs=%d\n",rs) ;
#endif

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_NOANODE:
	        ex = EX_OLDER ;
	        break ;
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_SEARCH:
	        ex = EX_NOUSER ;
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
	    debugprintf("b_userinfo: exiting ex=%u rs=%d\n",ex,rs) ;
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
	    debugprintf("b_userinfo: mallout=%u\n",mo-mo_start) ;
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
	int		i ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if (rs >= 0) {
	    fmt = "%s: USAGE> %s [<username>|- [<keyword(s)> ...]]\n" ;
	    rs = shio_printf(pip->efp,fmt,pn,pn) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	    rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    fmt = "%s:   possible specifications are: \n",
	        rs = shio_printf(pip->efp,fmt,pn) ;
	    wlen += rs ;
	}

	for (i = 0 ; (rs >= 0) && (qopts[i] != NULL) ; i += 1) {

	    if ((rs >= 0) && ((i % USAGECOLS) == 0)) {
	        rs = shio_printf(pip->efp,"%s: \t",pn) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = shio_printf(pip->efp,"%-16s",qopts[i]) ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && ((i % USAGECOLS) == 3)) {
	        rs = shio_printf(pip->efp,"\n") ;
	        wlen += rs ;
	    }

	} /* end for */

	if ((rs >= 0) && ((i % USAGECOLS) != 0)) {
	    rs = shio_printf(pip->efp,"\n") ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


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

	            if ((oi = matostr(progopts,3,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case progopt_phone:
	                    if (! lip->final.phone) {
	                        lip->have.phone = TRUE ;
	                        lip->final.phone = TRUE ;
	                        if (vl > 0) {
	                            rs = locinfo_setphone(lip,vp,vl) ;
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


static int procargs(pip,aip,bop,pdp,ofn,afn)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
PROGDATA	*pdp ;
cchar		*ofn ;
cchar		*afn ;
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("userinfo/procargs: ent\n") ;
#endif

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    cchar	*cp ;

	    if (rs >= 0) {
	        int	ai ;
	        int	f ;
	        for (ai = aip->ai_continue ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
	                if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = procquery(pip,pdp,ofp,cp,-1) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end for (handling positional arguments) */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        SHIO	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0) afn = STDINFNAME ;

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
	                        rs = procqueries(pip,pdp,ofp,cp,cl) ;
	                        wlen += rs ;
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

	    if ((rs >= 0) && (pan == 0)) {

	        cp = "username" ;
	        pan += 1 ;
	        rs = procquery(pip,pdp,ofp,cp,-1) ;
	        wlen += rs ;

	    } /* end if */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("userinfo/procargs: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procqueries(pip,pdp,ofp,lbuf,len)
PROGINFO	*pip ;
PROGDATA	*pdp ;
void		*ofp ;
cchar		*lbuf ;
int		len ;
{
	FIELD		fsb ;
	int		rs ;
	int		wlen = 0 ;
	if ((rs = field_start(&fsb,lbuf,len)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procquery(pip,pdp,ofp,fp,fl) ;
	            wlen += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procqueries) */


/* process a query specification */
static int procquery(PROGINFO *pip,PROGDATA *pdp,void *ofp,cchar rp[],int rl)
{
	DATAUSER	*dup = &pdp->du ;
	LOCINFO		*lip = pip->lip ;
	const int	clen = CBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 = SR_NOENT ;
	int		qi ;
	int		sl = -1 ;
	int		vl = 0 ;
	int		wlen = 0 ;
	cchar		*tp ;
	cchar		*sp = NULL ;
	cchar		*vp = NULL ;
	cchar		*cp = NULL ;
	char		cbuf[CBUFLEN + 1] ;

	if (rl < 0) rl = strlen(rp) ;

	if ((tp = strnchr(rp,rl,'=')) != NULL) {
	    vl = ((rp+rl)-(tp+1)) ;
	    if (vl) vp = (tp+1) ;
	    rl = (tp-rp) ;
	}

	if (vp == NULL) vl = 0 ; /* for GCC-warning */

	cbuf[0] = '\0' ;
	qi = matostr(qopts,1,rp,rl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_userinfo/procquery: query=%t qi=%d\n",
	        rp,rl,qi) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: query=%t(%d)\n",
	        pip->progname,rp,rl,qi) ;
	}

	switch (qi) {
	case qopt_sysname:
	case qopt_release:
	case qopt_version:
	case qopt_machine:
	    {
	        struct datasys	*dsp = &pdp->ds ;
	        if ((rs = datasys_uname(dsp)) >= 0) {
	            switch (qi) {
	            case qopt_sysname:
	                sp = dsp->uname.sysname ;
	                break ;
	            case qopt_release:
	                sp = dsp->uname.release ;
	                break ;
	            case qopt_version:
	                sp = dsp->uname.version ;
	                break ;
	            case qopt_machine:
	                sp = dsp->uname.machine ;
	                break ;
	            } /* end switch */
	        } /* end if (uname) */
	    } /* end block */
	    break ;
	case qopt_architecture:
	case qopt_platform:
	case qopt_provider:
	case qopt_hwserial:
	    {
	        struct datasys	*dsp = &pdp->ds ;
	        if ((rs = datasys_uaux(dsp)) >= 0) {
	            switch (qi) {
	            case qopt_architecture:
	                sp = dsp->uaux.architecture ;
	                break ;
	            case qopt_platform:
	                sp = dsp->uaux.platform ;
	                break ;
	            case qopt_provider:
	                sp = dsp->uaux.provider ;
	                break ;
	            case qopt_hwserial:
	                sp = dsp->uaux.hwserial ;
	                break ;
	            } /* end switch */
	        } /* end if (uaux) */
	    } /* end block */
	    break ;
	case qopt_nodename:
	case qopt_cluster:
	case qopt_system:
	case qopt_nisdomain:
	    {
	        struct datasys	*dsp = &pdp->ds ;
	        switch (qi) {
	        case qopt_nodename:
	            if (! dsp->f.node) rs = datasys_node(dsp) ;
	            if ((rs >= 0) && (dsp->nodename[0] != '\0'))
	                sp = dsp->nodename ;
	            break ;
	        case qopt_cluster:
	            if (! dsp->f.cluster) rs = datasys_cluster(dsp) ;
	            if ((rs >= 0) && (pdp->ds.clustername[0] != '\0'))
	                sp = dsp->clustername ;
	            break ;
	        case qopt_system:
	            if (! dsp->f.system) rs = datasys_system(dsp) ;
	            if ((rs >= 0) && (dsp->systemname[0] != '\0'))
	                sp = dsp->systemname ;
	            break ;
	        case qopt_nisdomain:
	            if (! dsp->f.nisdomain) rs = datasys_nisdomain(dsp) ;
	            if ((rs >= 0) && (dsp->nisdomainname[0] != '\0'))
	                sp = dsp->nisdomainname ;
	            break ;
	        } /* end switch */
	    } /* end block */
	    break ;
	case qopt_domain:
	case qopt_udomain:
	case qopt_inetdomain:
	    {
	        if (! pdp->f.domain) rs = progdata_domain(pdp) ;
	        if ((rs >= 0) && (pdp->domainname != NULL))
	            sp = pdp->domainname ;
	    }
	    break ;
	case qopt_hostname:
	    {
	        if (! pdp->f.host) rs = progdata_host(pdp) ;
	        if ((rs >= 0) && (pdp->hostname[0] != '\0'))
	            sp = pdp->hostname ;
	    }
	    break ;
/* primary user entries */
	case qopt_uid:
	case qopt_gid:
	case qopt_password:
	case qopt_passwd:
	case qopt_gecos:
	case qopt_gecosname:
	case qopt_homedir:
	case qopt_shell:
	case qopt_lstchg:
	case qopt_username:
	case qopt_organization:
	case qopt_realname:
	case qopt_account:
	case qopt_bin:
	case qopt_office:
	case qopt_wphone:
	case qopt_hphone:
	case qopt_printer:
	    if (! pdp->du.init.pent) rs = datauser_pw(&pdp->du) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_userinfo/procquery: q=uid rs=%d have.pent=%u\n",
	            rs,pdp->du.have.pent) ;
#endif
	    if ((rs >= 0) && pdp->du.have.pent) {
	        switch (qi) {
	        case qopt_uid:
	            sp = cbuf ;
	            rs = ctdecui(cbuf,clen,(uint) pdp->du.pent.uid) ;
	            sl = rs ;
	            break ;
	        case qopt_gid:
	            sp = cbuf ;
	            rs = ctdecui(cbuf,clen,(uint) pdp->du.pent.gid) ;
	            sl = rs ;
	            break ;
	        case qopt_username:
	            sp = pdp->du.pent.username ;
	            break ;
	        case qopt_gecos:
	            sp = pdp->du.pent.gecos ;
	            break ;
	        case qopt_homedir:
	            sp = pdp->du.pent.dir ;
	            break ;
	        case qopt_shell:
	            sp = pdp->du.pent.shell ;
	            break ;
	        case qopt_gecosname:
	            {
	                cchar	*gecos = pdp->du.pent.gecos ;
	                if ((rs = mkgecosname(cbuf,clen,gecos)) >= 0) {
	                    sp = cbuf ;
	                    sl = rs ;
	                }
	            }
	            break ;
	        case qopt_organization:
	            if (pdp->f.self) {
	                sp = getourenv(pip->envv,VARORGANIZATION) ;
	            }
	            if ((rs >= 0) && (sp == NULL)) {
	                cchar	*homedname = pdp->du.pent.dir ;
	                rs = gethomeorg(cbuf,clen,homedname) ;
	                sl = rs ;
	                if (rs > 0) sp = cbuf ;
	                if (isNotAccess(rs)) {
	                    rs = SR_OK ;
	                }
	            }
	            if ((rs >= 0) && (sp == NULL)) {
	                cp = pdp->du.pent.organization ;
	                if ((cp != NULL) && (cp[0] != '\0')) {
	                    sp = cp ;
	                    sl = -1 ;
	                }
	            }
	            if ((rs >= 0) && (sp == NULL)) {
	                cchar	*un = pdp->du.pent.username ;
	                rs = localgetorg(pip->pr,cbuf,clen,un) ;
	                sl = rs ;
	                if (rs > 0) sp = cbuf ;
	                if (isNotAccess(rs)) {
	                    rs = SR_OK ;
	                }
	            }
	            break ;
	        case qopt_realname:
	            rs1 = mkrealname(cbuf,clen,pdp->du.pent.realname,-1) ;
	            if (rs1 >= 0) sp = cbuf ;
	            break ;
	        case qopt_account:
	            sp = pdp->du.pent.account ;
	            if (sp == NULL)
	                sp = cbuf ;
	            break ;
	        case qopt_bin:
	            sp = pdp->du.pent.bin ;
	            if (sp == NULL)
	                sp = cbuf ;
	            break ;
	        case qopt_office:
	            sp = pdp->du.pent.office ;
	            if (sp == NULL)
	                sp = cbuf ;
	            break ;
	        case qopt_wphone:
	        case qopt_hphone:
	            {
	                switch (qi) {
	                case qopt_wphone:
	                    sp = pdp->du.pent.wphone ;
	                    break ;
	                case qopt_hphone:
	                    sp = pdp->du.pent.hphone ;
	                    break ;
	                } /* end switch */
	                if (sp != NULL) {
	                    switch (lip->phone) {
	                    case phonetype_fancy:
	                        if ((rs = mkfmtphone(cbuf,clen,sp,sl)) >= 0) {
	                            sp = cbuf ;
	                            sl = rs ;
	                        }
	                        break ;
	                    } /* end switch */
	                } else {
	                    sp = cbuf ;
	                }
	            } /* end block */
	            break ;
	        case qopt_printer:
	            if (pdp->f.self) sp = getourenv(pip->envv,VARPRINTER) ;
	            if (sp == NULL) sp = pdp->du.pent.printer ;
	            if (sp == NULL) sp = cbuf ;
	            break ;
	        case qopt_password:
	        case qopt_passwd:
	            sp = pdp->du.pent.password ;
	            break ;
	        case qopt_lstchg:
	            {
	                const long	lv = pdp->du.pent.lstchg ;
	                sp = cbuf ;
	                rs = ctdecl(cbuf,clen,lv) ;
	                sl = rs ;
	            }
	            break ;
	        } /* end switch */
	    } /* end if (have.pent) */
	    break ;
/* group query */
	case qopt_group:
	    if (! pdp->du.init.gr) {
	        rs = datauser_gr(&pdp->du) ;
	    }
	    if ((rs >= 0) && pdp->du.have.gr) {
	        sp = pdp->du.gr.gr_name ;
	    }
	    break ;
/* project ID queries */
	case qopt_projid:
	case qopt_pjid:
	case qopt_projectname:
	case qopt_projname:
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_userinfo/procquery: f_initpj=%u\n",
	            pdp->du.init.pj) ;
#endif
	    if (! pdp->du.init.pj) {
	        rs = datauser_pj(&pdp->du) ;
	    }
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_userinfo/procquery: "
	            "datauser_pj() rs=%d\n",rs) ;
#endif
	    if ((rs < 0) || (! pdp->du.have.pj)) break ;
	    switch (qi) {
	    case qopt_projid:
	    case qopt_pjid:
	        if (pdp->du.have.pj) {
	            const int	v = (int) pdp->du.pj.pj_projid ;
	            sp = cbuf ;
	            rs = ctdeci(cbuf,clen,v) ;
	            sl = rs ;
	        }
	        break ;
	    case qopt_projectname:
	    case qopt_projname:
	        if (pdp->du.have.pj) {
	            sp = pdp->du.pj.pj_name ;
	        }
	        break ;
	    } /* end switch */
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_userinfo/procquery: pj rs=%d sp=%s\n",
	            rs,sp) ;
#endif
	    break ;
/* the easy self queries */
	case qopt_ruid:
	    if (pdp->f.self) {
	        uid_t	v = getuid() ;
	        sp = cbuf ;
	        rs = ctdecui(cbuf,clen,(uint) v) ;
	        sl = rs ;
	    }
	    break ;
	case qopt_rgid:
	    if (pdp->f.self) {
	        gid_t	v = getgid() ;
	        sp = cbuf ;
	        rs = ctdecui(cbuf,clen,(uint) v) ;
	        sl = rs ;
	    }
	    break ;
	case qopt_eprojid:
	    if (pdp->f.self) {
	        projid_t	v = getprojid() ;
	        sp = cbuf ;
	        rs = ctdecui(cbuf,clen,(uint) v) ;
	        sl = rs ;
	    }
	    break ;
	case qopt_euid:
	    if (pdp->f.self) {
	        uid_t	v = geteuid() ;
	        sp = cbuf ;
	        rs = ctdecui(cbuf,clen,(uint) v) ;
	        sl = rs ;
	    }
	    break ;
	case qopt_egid:
	    if (pdp->f.self) {
	        gid_t	v = getegid() ;
	        sp = cbuf ;
	        rs = ctdecui(cbuf,clen,(uint) v) ;
	        sl = rs ;
	    }
	    break ;
	case qopt_egids:
	    if (pdp->f.self) {
	        SBUF	b ;
	        sp = cbuf ;
	        if ((rs = sbuf_start(&b,cbuf,clen)) >= 0) {
	            gid_t	egids[NGROUPS_MAX + 1] ;
	            if ((rs1 = u_getgroups(NGROUPS_MAX,egids)) >= 0) {
	                const int	n = rs1 ;
	                int		i ;
	                for (i = 0 ; i < n ; i += 1) {
	                    if (i > 0) sbuf_char(&b,' ') ;
	                    sbuf_decui(&b,((uint) egids[i])) ;
	                } /* end for */
	            } /* end if (getgroups) */
	            sl = sbuf_finish(&b) ;
	            if (rs >= 0) rs = sl ;
	        } /* end if (initialization) */
	    } /* end if (self) */
	    break ;
	case qopt_pid:
	    if (pdp->f.self) {
	        const pid_t	v = ugetpid() ;
	        sp = cbuf ;
	        rs = ctdecui(cbuf,clen,(uint) v) ;
	        sl = rs ;
	    }
	    break ;
	case qopt_ppid:
	    if (pdp->f.self) {
	        const pid_t	v = getppid() ;
	        sp = cbuf ;
	        rs = ctdecui(cbuf,clen,(uint) v) ;
	        sl = rs ;
	    }
	    break ;
	case qopt_pgid:
	    if (pdp->f.self) {
	        const pid_t	v = getpgrp() ;
	        sp = cbuf ;
	        rs = ctdecui(cbuf,clen,(uint) v) ;
	        sl = rs ;
	    }
	    break ;
	case qopt_sid:
	case qopt_psid:
	    if (pdp->f.self) {
	        if ((rs1 = u_getsid(0)) >= 0) {
	            sp = cbuf ;
	            rs = ctdecui(cbuf,clen,(uint) rs1) ;
	            sl = rs ;
	        }
	    }
	    break ;
	case qopt_tid:
	case qopt_ptid:
	    if (pdp->f.self) {
	        const taskid_t	v = gettaskid() ;
	        sp = cbuf ;
	        rs = ctdecui(cbuf,clen,(uint) v) ;
	        sl = rs ;
	    }
	    break ;
	case qopt_gids:
	    if ((rs = datauser_groups(&pdp->du)) >= 0) {
	        DATAUSER	*dup = &pdp->du ;
	        sp = cbuf ;
	        sl = 0 ;
	        if (dup->have.groups) {
	            if ((rs = datauser_mkgids(dup,cbuf,clen)) >= 0) {
	                sp = cbuf ;
	                sl = rs ;
	            }
	        } /* end if (have groups) */
	    } /* end if (datauser_groups) */
	    break ;
	case qopt_groups:
	    if ((rs = datauser_groups(&pdp->du)) >= 0) {
	        DATAUSER	*dup = &pdp->du ;
	        sp = cbuf ;
	        sl = 0 ;
	        if (dup->have.groups) {
	            vecstr	*glp = &dup->groups ;
	            if ((rs = mkstrlist(cbuf,clen,glp)) >= 0) {
	                sp = cbuf ;
	                sl = rs ;
	            } /* end if (mkstrlist) */
	        } /* end if (have groups) */
	    } /* end if (datauser_groups) */
	    break ;
	case qopt_projects:
	    if ((rs = procquery_projects(pip,pdp,cbuf,clen)) >= 0) {
	        sp = cbuf ;
	        sl = rs ;
	    }
	    break ;
	case qopt_projectinfo:
	case qopt_projinfo:
	    if ((rs = procquery_projinfo(pip,pdp,cbuf,clen)) >= 0) {
	        sp = cbuf ;
	        sl = rs ;
	    }
	    break ;
	case qopt_logid:
	case qopt_logline:
	case qopt_loghost:
	case qopt_logname:
	case qopt_logsession:
	case qopt_logdate:
	    if (pdp->f.self) {
	        if (! pdp->du.init.utmpent) {
	            rs = datauser_utmpent(&pdp->du) ;
	        }
	        if ((rs >= 0) && pdp->du.have.utmpent) {
	            cp = NULL ;
	            switch (qi) {
	            case qopt_logid:
	                cp = pdp->du.ue.id ;
	                break ;
	            case qopt_logline:
	                cp = pdp->du.ue.line ;
	                break ;
	            case qopt_loghost:
	                cp = pdp->du.ue.host ;
	                break ;
	            case qopt_logname:
	                cp = pdp->du.ue.user ;
	                break ;
	            case qopt_logsession:
	                sp = cbuf ;
	                sl = ctdeci(cbuf,clen,pdp->du.ue.session) ;
	                break ;
	            case qopt_logdate:
	                sp = cbuf ;
	                timestr_log(pdp->du.ue.date,cbuf) ;
	                break ;
	            } /* end switch */
	            if (cp != NULL) {
	                sp = cbuf ;
	                sl = sncpy1(cbuf,clen,cp) ;
	            }
	        } /* end if (utmp) */
	    } /* end if (self) */
	    break ;
	case qopt_tz:
	    if (pdp->f.self) {
	        sp = getourenv(pip->envv,VARTZ) ;
	    }
	    if (sp == NULL) {
	        if (! pdp->du.init.tz) {
	            rs = datauser_tz(&pdp->du) ;
	        }
	        if ((rs >= 0) && pdp->du.have.tz) {
	            sp = pdp->du.tz ;
	        }
	        if (sp == NULL) {
	            sp = cbuf ;
		}
	    } /* end if */
	    break ;
	case qopt_lastlog:
	    if (! pdp->du.init.lastlog) {
	        rs = datauser_lastlog(&pdp->du) ;
	    }
	    if ((rs >= 0) && (pdp->du.have.lastlog)) {
	        time_t	t = dup->lasttime ;
	        cchar	*lh = dup->lasthost ;
	        cchar	*ll = dup->lastline ;
	        cchar	*fmt ;
	        char	timebuf1[TIMEBUFLEN + 1] ;
	        char	timebuf2[TIMEBUFLEN + 1] ;
	        if (pip->verboselevel >= 1) {
	            fmt = "%-23s %-8s %16s (%17s)" ;
	        } else {
	            fmt = "%-23s %-8s %16s" ;
	        }
	        sp = cbuf ;
	        timestr_logz(t,timebuf1) ;
	        timestr_elapsed((pip->daytime - t),timebuf2) ;
	        rs = bufprintf(cbuf,clen,fmt,timebuf1,ll,lh,timebuf2) ;
	        sl = rs ;
	    }
	    break ;
	case qopt_name:
	case qopt_pcsname:
	    if ((rs = procquery_name(pip,pdp,cbuf,clen)) >= 0) {
	        sp = cbuf ;
	        sl = rs ;
	    }
	    break ;
	case qopt_fullname:
	case qopt_pcsfullname:
	    if ((rs = procquery_fullname(pip,pdp,cbuf,clen)) >= 0) {
	        sp = cbuf ;
	        sl = rs ;
	    }
	    break ;
	case qopt_netname:
	    if ((rs = procquery_netname(pip,pdp,cbuf,clen)) >= 0) {
	        sp = cbuf ;
	        sl = rs ;
	    }
	    break ;
	case qopt_mailname:
	    if ((sp == NULL) && pdp->f.self) {
	        sp = getourenv(pip->envv,VARMAILNAME) ;
	    }
	    if (sp == NULL) {
	        if ((rs = datauser_pw(&pdp->du)) >= 0) {
	            if (pdp->du.have.pent) {
	                cchar	*rn = pdp->du.pent.realname ;
	                if ((rs = mkmailname(cbuf,clen,rn,-1)) > 0) {
	                    sp = cbuf ;
	                    sl = rs ;
	                }
	            }
	        }
	    } /* end if */
	    if ((rs >= 0) && (sp == NULL)) {
	        sp = cbuf ;
	        sl = sncpylc(cbuf,clen,pdp->un) ;
	    }
	    break ;
	case qopt_eusername:
	    if (pdp->f.self) {
	        if (! pdp->du.init.pent) {
	            rs = datauser_pw(&pdp->du) ;
	        }
	        if ((rs >= 0) && pdp->du.have.pent) {
	            sp = pdp->du.pent.realname ;
	        }
	        if (rs >= 0) {
	            const uid_t		uid = geteuid() ;
	            if (uid == pdp->du.pent.uid) {
	                sp = pdp->du.pent.username ;
	            } else {
	                sp = cbuf ;
	                rs = getuser(cbuf,clen,uid) ;
	                sl = rs ;
	            }
	        }
	    } /* end if (self eusername) */
	    break ;
	case qopt_rgroupname:
	    if (pdp->f.self) {
	        if (! pdp->du.init.gr) {
	            rs = datauser_gr(&pdp->du) ;
	        }
	        if (rs >= 0) {
	            const gid_t		gid = getgid() ;
	            if (gid == pdp->du.gr.gr_gid) {
	                sp = pdp->du.gr.gr_name ;
	            } else {
	                sp = cbuf ;
	                rs = getgroupname(cbuf,clen,gid) ;
	                sl = rs ;
	            }
	        }
	    } /* end if (self rgroupname) */
	    break ;
	case qopt_egroupname:
	    if (pdp->f.self) {
	        if (! pdp->du.init.gr) {
	            rs = datauser_gr(&pdp->du) ;
	        }
	        if (rs >= 0) {
	            const gid_t		gid = getegid() ;
	            if (gid == pdp->du.gr.gr_gid) {
	                sp = pdp->du.gr.gr_name ;
	            } else {
	                sp = cbuf ;
	                rs = getgroupname(cbuf,clen,gid) ;
	                sl = rs ;
	            }
	        }
	    } /* end if (self egroupname) */
	    break ;
	case qopt_egroups:
	    if (pdp->f.self) {
	        SBUF	b ;
	        gid_t	egids[NGROUPS_MAX + 1] ;
	        gid_t	v ;
	        int	n, i ;
	        char	gnbuf[GROUPNAMELEN + 1] ;
	        if ((rs = sbuf_start(&b,cbuf,clen)) >= 0) {
	            if ((rs1 = u_getgroups(NGROUPS_MAX,egids)) >= 0) {
	                const int	gnlen = GROUPNAMELEN ;
	                n = rs1 ;
	                for (i = 0 ; i < n ; i += 1) {
	                    if (i > 0) sbuf_char(&b,' ') ;
	                    v = egids[i] ;
	                    if ((rs = getgroupname(gnbuf,gnlen,v)) >= 0) {
	                        sbuf_strw(&b,gnbuf,rs) ;
	                    }
	                    if (rs < 0) break ;
	                } /* end for */
	            } /* end if (getgroups) */
	            sp = cbuf ;
	            sl = sbuf_finish(&b) ;
	            if (rs >= 0) rs = sl ;
	        } /* end if (initialization) */
	    } /* end if (self egroups) */
	    break ;
	case qopt_wstation:
	    sp = cbuf ;
	    break ;
	case qopt_fsbs:
	case qopt_fspbs:
	case qopt_fstotal:
	case qopt_fsavail:
	case qopt_fsfree:
	case qopt_fsused:
	case qopt_fsutil:
	case qopt_fstype:
	case qopt_fsstr:
	case qopt_fsid:
	case qopt_fsflags:
	    if (! pdp->du.init.statvfs) {
	        rs = datauser_statvfs(&pdp->du) ;
	    }
	    if ((rs >= 0) && pdp->du.have.statvfs) {
	        struct statvfs	*fssp = &pdp->du.fss ;
	        LONG		vt ;
	        LONG		v = -1 ;
	        sp = cbuf ;
	        switch (qi) {
	        case qopt_fsbs:
	            v = fssp->f_frsize ;
	            break ;
	        case qopt_fspbs:
	            v = fssp->f_bsize ;
	            break ;
	        case qopt_fstotal:
	            vt = fssp->f_blocks * fssp->f_frsize ;
	            v = vt / 1024 ;
	            break ;
	        case qopt_fsavail:
	            vt = fssp->f_bavail * fssp->f_frsize ;
	            v = vt / 1024 ;
	            break ;
	        case qopt_fsused:
	            vt = (fssp->f_blocks - fssp->f_bfree) * fssp->f_frsize ;
	            v = vt / 1024 ;
	            break ;
	        case qopt_fsfree:
	            vt = fssp->f_bfree * fssp->f_frsize ;
	            v = vt / 1024 ;
	            break ;
	        case qopt_fsutil:
	            {
	                LONG f_bused = fssp->f_blocks - fssp->f_bavail ;
	                if (fssp->f_blocks > 0) {
	                    int		per ;
	                    vt = (f_bused * 100)  ;
	                    per = (vt / fssp->f_blocks) ;
	                    rs = bufprintf(cbuf,clen,"%u%%",per) ;
	                } else {
	                    rs = sncpy1(cbuf,clen,"na") ;
			}
	            }
	            break ;
	        case qopt_fstype:
	            rs = snwcpy(cbuf,clen,fssp->f_basetype,FSTYPSZ) ;
	            break ;
	        case qopt_fsstr:
	            rs = snwcpy(cbuf,clen,fssp->f_fstr,32) ;
	            break ;
	        case qopt_fsid:
	            rs = ctdecul(cbuf,clen,fssp->f_fsid) ;
	            break ;
	        case qopt_fsflags:
	            rs = snfsflags(cbuf,clen,fssp->f_flag) ;
	            break ;
	        } /* end switch */
	        if ((rs >= 0) && (v >= 0)) {
	            rs = bufprintf(cbuf,clen,"%llu",v) ;
	        }
	    } /* end if (statvfs) */
	    break ;
	case qopt_auid:
#if	CF_AUID
#if	defined(SYSHAS_AUDIT) && (SYSHAS_AUDIT > 0)
	    if (pdp->f.self) {
	        const int	v = uc_getauid() ;
	        if (v >= 0) {
	            sp = cbuf ;
	            rs = ctdeci(cbuf,clen,v) ;
	            sl = rs ;
	        }
	    }
#endif /* SYSHAS_AUDIT */
#endif /* CF_AUID */
	    break ;
	case qopt_orgcode:
	    rs = datauser_orgcode(dup) ;
	    sl = rs ;
	    sp = dup->orgcode ;
	    break ;
	case qopt_orgloc:
	    rs = datauser_orgloc(dup) ;
	    sl = rs ;
	    sp = dup->orgloc ;
	    break ;
	case qopt_lastseen:
	    rs = datauser_lastseen(dup) ;
	    sl = rs ;
	    sp = dup->lastseen ;
	    break ;
	case qopt_execname:
	    if ((rs = proginfo_getename(pip,cbuf,clen)) >= 0) {
	        sl = rs ;
	        sp = cbuf ;
	    }
	    break ;
	} /* end switch */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("procquery: fin rs=%d sl=%d\n",rs,sl) ;
	    if (sp != NULL)
	        debugprintf("procquery: c=>%t<\n",
	            sp,strlinelen(sp,sl,50)) ;
	}
#endif /* CF_DEBUG */

/* print out */

	if ((rs >= 0) && (pip->verboselevel > 0)) {
	    rs = procout(pip,ofp,sp,sl) ;
	    wlen += rs ;
	} /* end if (printing out) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procquery) */


static int procquery_name(PROGINFO *pip,PROGDATA *pdp,char *cbuf,int clen)
{
	int		rs = SR_OK ;
	int		sl = -1 ;
	cchar		*sp = NULL ;
	if ((sp == NULL) && pdp->f.self) {
	    cchar	*cp ;
	    cchar	*var = VARNAME ;
	    if ((cp = getourenv(pip->envv,var)) != NULL) {
	        if (cp[0] != '\0') {
	            rs = sncpy1(cbuf,clen,cp) ;
	            sp = cbuf ;
	        }
	    } /* end if (VARNAME) */
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_userinfo/procquery: pcsname -> %s\n",sp) ;
#endif
	if ((rs >= 0) && (sp == NULL)) {
	    DATAUSER	*dup = &pdp->du ;
	    if ((rs = datauser_pw(dup)) >= 0) {
	        if (dup->have.pent) {
	            const int	w = pcsnsreq_pcsname ;
	            cchar	*pun = dup->pent.username ;
	            if ((rs = procgetns(pip,cbuf,clen,pun,w)) > 0) {
	                sp = cbuf ;
	                sl = rs ;
	            } else if (isNotAccess(rs)) {
	                rs = SR_OK ;
	            }
	        } /* end if (have-entry) */
	    } /* end if (datauser_pw) */
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_userinfo/procquery: pcsname -> %s\n",sp) ;
#endif
	if ((rs >= 0) && (sp == NULL)) {
	    DATAUSER	*dup = &pdp->du ;
	    if ((rs = datauser_realname(dup,cbuf,clen)) > 0) {
	        sp = cbuf ;
	        sl = rs ;
	    }
	} /* end if (real-name from GECOS) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_userinfo/procquery: pcsname -> %s\n",sp) ;
#endif
	if ((rs >= 0) && (sp == NULL)) {
	    DATAUSER	*dup = &pdp->du ;
	    if ((rs = datauser_username(dup,cbuf,clen)) > 0) {
	        sp = cbuf ;
	        sl = rs ;
	    }
	} /* end if (username) */
	if (rs >= 0) { /* finishing */
	    if (sp != NULL) {
	        if (sl < 0) sl = strlen(sp) ;
	    } else {
	        sl = 0 ;
	    }
	} /* end if (finishing) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("b_userinfo/procquery_name: ret rs=%d sl=%u\n",rs,sl) ;
	    debugprintf("b_userinfo/procquery_name: ret r=>%t<\n",sp,sl) ;
	}
#endif
	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (procquery_name) */


static int procquery_fullname(PROGINFO *pip,PROGDATA *pdp,char *cbuf,int clen)
{
	int		rs = SR_OK ;
	int		sl = -1 ;
	cchar		*sp = NULL ;
	cchar		*cp ;
	if ((sp == NULL) && pdp->f.self) {
	    cchar	*var = VARFULLNAME ;
	    if ((cp = getourenv(pip->envv,var)) != NULL) {
	        if (cp[0] != '\0') {
	            rs = sncpy1(cbuf,clen,cp) ;
	            sp = cbuf ;
	        }
	    } /* end if (VARFULLNAME) */
	}
	if ((rs >= 0) && (sp == NULL)) {
	    DATAUSER	*dup = &pdp->du ;
	    if ((rs = datauser_pw(dup)) >= 0) {
	        if (dup->have.pent) {
	            const int	w = pcsnsreq_fullname ;
	            cchar		*pun = dup->pent.username ;
	            if ((rs = procgetns(pip,cbuf,clen,pun,w)) > 0) {
	                sp = cbuf ;
	                sl = rs ;
	            } else if (isNotAccess(rs)) {
	                rs = SR_OK ;
	            }
	        } /* end if (have-entry) */
	    } /* end if (datauser_pw) */
	} /* end if */
	if ((rs >= 0) && (sp == NULL) && pdp->f.self) {
	    if ((cp = getourenv(pip->envv,VARNAME)) != NULL) {
	        REALNAME	rn ;
	        if ((rs = realname_startparse(&rn,cp,-1)) >= 0) {
	            if ((rs = realname_fullname(&rn,cbuf,clen)) > 0) {
	                sp = cbuf ;
	                sl = rs ;
	            }
	            realname_finish(&rn) ;
	        } /* end if (realname) */
	    } /* end if (non-null) */
	} /* end if */
	if ((rs >= 0) && (sp == NULL)) {
	    DATAUSER	*dup = &pdp->du ;
	    if ((rs = datauser_realname(dup,cbuf,clen)) > 0) {
	        sp = cbuf ;
	        sl = rs ;
	    }
	}
	if (rs >= 0) { /* finishing */
	    if (sp != NULL) {
	        if (sl < 0) sl = strlen(sp) ;
	    } else {
	        sl = 0 ;
	    }
	} /* end if (finishing) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("b_userinfo/procquery_fname: ret rs=%d sl=%u\n",rs,sl) ;
	    debugprintf("b_userinfo/procquery_fname: ret r=>%t<\n",sp,sl) ;
	}
#endif
	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (procquery_fullname) */


static int procquery_netname(PROGINFO *pip,PROGDATA *pdp,char *cbuf,int clen)
{
	int		rs ;
	int		sl = 0 ;
	if (pdp->f.self) {
	    char	nbuf[MAXNETNAMELEN+1] ;
	    if ((rs = uc_getnetname(nbuf)) >= 0) {
	            sl = (strdcpy1w(cbuf,clen,nbuf,rs) - cbuf) ;
	    } else if ((rs == SR_NOTFOUND) || (rs == SR_UNAVAIL)) {
	            rs = SR_OK ;
	            cbuf[0] = '\0' ;
	    }
	} /* end if (self) */
	if ((rs >= 0) && (sl == 0)) {
	        DATASYS		*dsp = &pdp->ds ;
	        if ((rs = datasys_nisdomain(dsp)) >= 0) {
	    	    DATAUSER	*dup = &pdp->du ;
	            cchar	*nd = dsp->nisdomainname ;
	            if ((rs = datauser_netname(dup,nd)) >= 0) {
	                cchar	*nn = dup->netname ;
	                sl = (strdcpy1w(cbuf,clen,nn,rs) - cbuf) ;
	            }
	        }
	} /* end if */
	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (procquery_netname) */


static int procquery_projects(PROGINFO *pip,PROGDATA *pdp,char *cbuf,int clen)
{
	DATAUSER	*dup = &pdp->du ;
	int		rs ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = datauser_projects(dup)) > 0) {
	    vecstr	*lp = &dup->projects ;
	    rs = mkstrlist(cbuf,clen,lp) ;
	} /* end if (datauser_projects) */
	return rs ;
}
/* end subroutine (procquery_projects) */


static int procquery_projinfo(PROGINFO *pip,PROGDATA *pdp,char *cbuf,int clen)
{
	DATAUSER	*dup = &pdp->du ;
	int		rs ;
	int		sl = 0 ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = datauser_pw(dup)) > 0) {
	    const int	w = pcsnsreq_projinfo ;
	    cchar	*pun = dup->pent.username ;
	    if ((rs = procgetns(pip,cbuf,clen,pun,w)) > 0) {
	        sl = rs ;
	    } else if (isNotAccess(rs)) {
	        rs = SR_OK ;
	    }
	} /* end if (datauser_pw) */
	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (procquery_projinfo) */


static int procout(PROGINFO *pip,void *ofp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	if (pip->verboselevel > 0) {
	    if (sp == NULL) {
	        sp = "*" ;
	        sl = 1 ;
	    }
	    rs = shio_print(ofp,sp,sl) ;
	    wlen += rs ;
	} /* end if (printing out) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int procgetns(PROGINFO *pip,char *nbuf,int nlen,cchar *un,int w)
{
	LOCINFO		*lip = pip->lip ;
	return locinfo_pcsnsget(lip,nbuf,nlen,un,w) ;
}
/* end subroutine (procgetns) */


static int progdata_start(PROGDATA *pdp,PROGINFO *pip,cchar *un,int f_self,
		cchar *pwfname)
{
	int		rs ;

	memset(pdp,0,sizeof(PROGDATA)) ;
	pdp->pip = pip ;
	pdp->un = un ;
	pdp->pwfname = pwfname ;
	pdp->domainname = NULL ;
	pdp->hostname[0] = '\0' ;
	pdp->f.self = f_self ;

#if	CF_DEBUGS
	debugprintf("userinfo/progdata_start: ent un=%s\n",un) ;
	debugprintf("userinfo/progdata_start: f_self=%u\n",pdp->f.self) ;
#endif

	if ((rs = datasys_start(&pdp->ds,pip)) >= 0) {
	    rs = datauser_start(&pdp->du,pip,un,pwfname) ;
	    if (rs < 0)
	        datasys_finish(&pdp->ds) ;
	} /* end if (datasys-start) */

#if	CF_DEBUGS
	debugprintf("userinfo/progdata_start: datauser_start() rs=%d\n",rs) ;
	debugprintf("userinfo/progdata_start: f_self=%u\n",pdp->f.self) ;
#endif

	if (rs == SR_NOTFOUND) rs = SR_SEARCH ;

#if	CF_DEBUGS
	debugprintf("userinfo/progdata_start: ret rs=%d f_self=%u\n",
	    rs,pdp->f.self) ;
#endif

	return rs ;
}
/* end subroutine (progdata_start) */


static int progdata_finish(PROGDATA *pdp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = datauser_finish(&pdp->du) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = datasys_finish(&pdp->ds) ;
	if (rs >= 0) rs = rs1 ;

#ifdef	OPTIONAL
	memset(pdp,0,sizeof(PROGDATA)) ;
#endif

	return rs ;
}
/* end subroutine (progdata_finish) */


static int progdata_domain(PROGDATA *pdp)
{
	int		rs = SR_OK ;

	if (! pdp->f.domain) {
	    cchar	*sp = NULL ;

	    if (! pdp->du.init.domain) rs = datauser_domain(&pdp->du) ;

	    if ((rs >= 0) && (pdp->du.domainname[0] != '\0'))
	        sp = pdp->du.domainname ;

	    if (sp == NULL) {
	        if (! pdp->ds.f.domain) rs = datasys_domain(&pdp->ds) ;
	        if ((rs >= 0) && (pdp->ds.domainname[0] != '\0'))
	            sp = pdp->ds.domainname ;
	    } /* end if */

	    pdp->f.domain = TRUE ;
	    if ((rs >= 0) && (sp != NULL)) {
	        pdp->domainname = sp ;
	    }

	} /* end if (initialization needed) */

	return rs ;
}
/* end subroutine (progdata_domain) */


static int progdata_host(PROGDATA *pdp)
{
	int		rs = SR_OK ;

	if ((! pdp->f.host) && (pdp->hostname[0] == '\0')) {
	    pdp->f.host = TRUE ;
	    if (! pdp->ds.f.node) {
	        rs = datasys_node(&pdp->ds) ;
	    }
	    if ((rs >= 0) && (! pdp->f.domain)) {
	        rs = progdata_domain(pdp) ;
	    }
	    if ((rs >= 0) && (pdp->ds.nodename[0] != '\0') &&
	        (pdp->domainname != NULL)) {
	        rs = sncpy3(pdp->hostname,MAXHOSTNAMELEN,
	            pdp->ds.nodename,".",pdp->domainname) ;
	    }
	}

	return rs ;
}
/* end subroutine (progdata_host) */


static int progdata_haveuser(PROGDATA *pdp)
{
	int		rs = SR_OK ;

	if (pdp->du.init.pent && (! pdp->du.have.pent)) {
	    rs = SR_SEARCH ;
	}

	return rs ;
}
/* end subroutine (progdata_haveuser) */


static int datasys_start(DATASYS *dsp,PROGINFO *pip)
{
	int		rs ;

	memset(dsp,0,sizeof(struct datasys)) ;
	dsp->pip = pip ;

	rs = vecstr_start(&dsp->stores,0,0) ;

	return rs ;
}
/* end subroutine (datasys_start) */


static int datasys_finish(DATASYS *dsp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecstr_finish(&dsp->stores) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (datasys_finish) */


int datasys_setentry(DATASYS *dsp,cchar **epp,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		oi = -1 ;
	int		len = 0 ;

	if (dsp == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (*epp != NULL) {
	    oi = vecstr_findaddr(&dsp->stores,*epp) ;
	}

	if (vp != NULL) {
	    len = strnlen(vp,vl) ;
	    rs = vecstr_store(&dsp->stores,vp,len,epp) ;
	} else if (epp != NULL) {
	    *epp = NULL ;
	}

	if ((rs >= 0) && (oi >= 0)) {
	    vecstr_del(&dsp->stores,oi) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (datasys_setentry) */


static int datasys_uname(DATASYS *dsp)
{
	int		rs = SR_OK ;

	if (! dsp->f.uname) {
	    dsp->f.uname = TRUE ;
	    rs = uinfo_name(&dsp->uname) ;
	} /* end if (uname) */

	return rs ;
}
/* end subroutine (datasys_uname) */


static int datasys_uaux(DATASYS *dsp)
{
	int		rs = SR_OK ;

	if (! dsp->f.uaux) {
	    dsp->f.uaux = TRUE ;
	    rs = uinfo_aux(&dsp->uaux) ;
	} /* end if (uaux) */

	return rs ;
}
/* end subroutine (datasys_uaux) */


#ifdef	COMMENT
static int datasys_si(DATASYS *dsp)
{
	int		rs = SR_OK ;

	if (! dsp->f.si) {
	    rs = u_uname(&dsp->si) ;
	    if (rs >= 0) dsp->f.si = TRUE ;
	}

	return rs ;
}
/* end subroutine (datasys_si) */
#endif /* COMMENT */


static int datasys_node(DATASYS *dsp)
{
	PROGINFO	*pip = dsp->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	if (! dsp->f.node) {
	    dsp->f.node = TRUE ;
	    if (dsp->nodename == NULL) {
	        cchar	*cp = getourenv(pip->envv,VARNODE) ;
	        if ((cp != NULL) && (cp[0] != '\0'))
	            dsp->nodename = cp ;
	    }
	    if ((dsp->nodename == NULL) || (dsp->nodename[0] == '\0')) {
	        if (! dsp->f.uname) rs = datasys_uname(dsp) ;
	        if (rs >= 0) {
	            int	nl = -1 ;
	            cchar	*np = dsp->uname.nodename ;
	            cchar	**vpp = &dsp->nodename ;
	            char	*tp ;
	            if ((tp = strchr(np,'.')) != NULL) {
	                nl = (tp-np) ;
	            }
	            rs = datasys_setentry(dsp,vpp,np,nl) ;
	        }
	    }
	} /* end if (initializing) */

	return rs ;
}
/* end subroutine (datasys_node) */


static int datasys_system(DATASYS *dsp)
{
	PROGINFO	*pip = dsp->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	if (! dsp->f.system) {
	    dsp->f.system = TRUE ;
	    if (dsp->systemname == NULL) {
	        cchar	*cp = getourenv(pip->envv,VARSYSTEM) ;
	        if ((cp != NULL) && (cp[0] != '\0')) {
	            dsp->systemname = cp ;
	        }
	    }
	    if (dsp->systemname == NULL) {
	        rs = datasys_nodeinfo(dsp) ;
	    } /* end if (needed) */
	} /* end if (initializing) */

	return rs ;
}
/* end subroutine (datasys_system) */


static int datasys_cluster(DATASYS *dsp)
{
	PROGINFO	*pip = dsp->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	if (! dsp->f.cluster) {
	    dsp->f.cluster = TRUE ;
	    if (dsp->clustername == NULL) {
	        cchar	*cp = getourenv(pip->envv,VARCLUSTER) ;
	        if ((cp != NULL) && (cp[0] != '\0')) {
	            dsp->clustername = cp ;
	        }
	    }
	    if (dsp->clustername == NULL) {
	        rs = datasys_nodeinfo(dsp) ;
	    } /* end if (needed) */
	} /* end if (initializing) */

	return rs ;
}
/* end subroutine (datasys_cluster) */


static int datasys_nodeinfo(DATASYS *dsp)
{
	PROGINFO	*pip = dsp->pip ;
	int		rs = SR_OK ;
	cchar		*cn = dsp->clustername ;
	cchar		*sn = dsp->systemname ;
	if ((cn == NULL) || (sn == NULL)) {
	    if ((rs = datasys_node(dsp)) >= 0) {
	        cchar	*pr = pip->pr ;
	        cchar	*nn = dsp->nodename ;
	        char	cbuf[NODENAMELEN+1] ;
	        char	sbuf[NODENAMELEN+1] ;
	        if ((rs = getnodeinfo(pr,cbuf,sbuf,NULL,nn)) >= 0) {
	            if ((rs >= 0) && (cn == NULL)) {
	                cchar	**vpp = &dsp->clustername ;
	                rs = datasys_setentry(dsp,vpp,cbuf,-1) ;
	            }
	            if ((rs >= 0) && (sn == NULL)) {
	                cchar	**vpp = &dsp->systemname ;
	                rs = datasys_setentry(dsp,vpp,sbuf,-1) ;
	            }
	        } /* end if (getnodeinfo) */
	    } /* end if (datasys_node) */
	}
	return rs ;
}
/* end subroutine (datasys_nodeinfo) */


/* this is a "user" domain-name (not a "system" domain-name) */
static int datasys_domain(DATASYS *dsp)
{
	PROGINFO	*pip = dsp->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	if (! dsp->f.domain) {
	    dsp->f.domain = TRUE ;
	    if (dsp->domainname == NULL) {
	        cchar	*cp = getourenv(pip->envv,VARDOMAIN) ;
	        if ((cp != NULL) && (cp[0] != '\0'))
	            dsp->domainname = cp ;
	    }
	    if ((dsp->domainname == NULL) || (dsp->domainname[0] == '\0')) {
	        if (dsp->nodename == NULL) {
	            if (! dsp->f.node) rs = datasys_node(dsp) ;
	        }
	        if (rs >= 0) {
	            const int	dlen = NODENAMELEN ;
	            cchar	*nn = dsp->nodename ;
	            char	dbuf[NODENAMELEN + 1] ;
	            if ((rs = getdomainname(dbuf,dlen,nn)) >= 0) {
	                cchar	**vpp = &dsp->domainname ;
	                rs = datasys_setentry(dsp,vpp,dbuf,rs) ;
	            }
	        }
	    } /* end if */
	} /* end if (needed initialization) */

	return rs ;
}
/* end subroutine (datasys_domain) */


static int datasys_nisdomain(DATASYS *dsp)
{
	PROGINFO	*pip = dsp->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	if (! dsp->f.nisdomain) {
	    cchar	*ndp ;
	    dsp->f.nisdomain = TRUE ;
	    if (dsp->nisdomainname == NULL) {
	        cchar	*cp = getourenv(pip->envv,VARNISDOMAIN) ;
	        if ((cp != NULL) && (cp[0] != '\0')) {
	            dsp->nisdomainname = cp ;
	            rs = strlen(cp) ;
	        }
	    }
	    ndp = dsp->nisdomainname ;
	    if ((ndp == NULL) || (ndp[0] == '\0')) {
	        const int	nlen = NODENAMELEN ;
	        char		nbuf[NODENAMELEN+1] ;
	        if ((rs = nisdomainname(nbuf,nlen)) >= 0) {
	            cchar	**vpp = &dsp->nisdomainname ;
	            rs = datasys_setentry(dsp,vpp,nbuf,rs) ;
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	    }
	} else {
	    if (dsp->nisdomainname == NULL) {
	        rs = strlen(dsp->nisdomainname) ;
	    }
	} /* end if (initialization needed) */

	return rs ;
}
/* end subroutine (datasys_nisdomain) */


static int datauser_start(DATAUSER *dup,PROGINFO *pip,cchar *un,cchar *pwfname)
{
	const int	pentlen = pwentry_bufsize() ;
	const int	grlen = getbufsize(getbufsize_gr) ;
	const int	pjlen = getbufsize(getbufsize_pj) ;
	int		rs ;
	int		size = 0 ;
	char		*bp ;

#if	CF_DEBUGS
	debugprintf("userinfo/datauser_start: ent un=%s\n",un) ;
#endif

	memset(dup,0,sizeof(DATAUSER)) ;
	dup->pip = pip ;
	dup->ruid = -1 ;
	dup->euid = -1 ;
	dup->rgid = -1 ;
	dup->egid = -1 ;
	dup->projid = -1 ;
	dup->eprojid = -1 ;
	dup->un = un ;
	dup->pwfname = pwfname ;

	size += (pentlen+1) ;
	size += (grlen+1) ;
	size += (pjlen+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    dup->a = bp ;
	    dup->pentbuf = bp ;
	    dup->pentlen = pentlen ;
	    bp += (pentlen+1) ;
	    dup->grbuf = bp ;
	    dup->grlen = grlen ;
	    bp += (grlen+1) ;
	    dup->pjbuf = bp ;
	    dup->pjlen = pjlen ;
	    {
	        rs = datauser_pw(dup) ;
	    }
	    if (rs < 0) {
	        uc_free(dup->a) ;
	        dup->a = NULL ;
	        dup->pentbuf = NULL ;
	        dup->pentlen = 0 ;
	        dup->grbuf = NULL ;
	        dup->grlen = 0 ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("userinfo/datauser_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (datauser_start) */


static int datauser_finish(DATAUSER *dup)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (dup == NULL)
	    return SR_FAULT ;

	if (dup->netname != NULL) {
	    rs1 = uc_free(dup->netname) ;
	    if (rs >= 0) rs = rs1 ;
	    dup->netname = NULL ;
	}

	if (dup->have.groups) {
	    dup->have.groups = FALSE ;
	    rs1 = vecstr_finish(&dup->groups) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (dup->have.projects) {
	    dup->have.projects = FALSE ;
	    rs1 = vecstr_finish(&dup->projects) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (dup->a != NULL) {
	    rs1 = uc_free(dup->a) ;
	    if (rs >= 0) rs = rs1 ;
	    dup->a = NULL ;
	    dup->pentbuf = NULL ;
	    dup->grbuf = NULL ;
	    dup->pjbuf = NULL ;
	}

	return rs ;
}
/* end subroutine (datauser_finish) */


static int datauser_ua(DATAUSER *dup)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (dup == NULL) return SR_FAULT ;

	pip = dup->pip ;
	if (pip == NULL) return SR_FAULT ;

	if (! dup->init.ua) {
	    USERATTR	ua ;
	    dup->init.ua = TRUE ;
	    dup->tz[0] = '\0' ;
	    dup->dn[0] = '\0' ;
	    if ((rs = userattr_open(&ua,dup->un)) >= 0) {
	        const int	vlen = VBUFLEN ;
	        int		i ;
	        int		vl ;
	        char		vbuf[VBUFLEN + 1] ;

	        for (i = 0 ; uakeys[i] != NULL ; i += 1) {
	            if ((vl = userattr_lookup(&ua,vbuf,vlen,uakeys[i])) >= 0) {
	                switch (i) {
	                case uakey_tz:
	                    rs = snwcpy(dup->tz,TZLEN,vbuf,vl) ;
	                    dup->have.tz = (rs > 0) ;
	                    break ;
	                case uakey_dn:
	                    rs = snwcpy(dup->dn,MAXHOSTNAMELEN,vbuf,vl) ;
	                    dup->have.dn = (rs > 0) ;
	                    break ;
	                } /* end switch */
	            } /* end if (userattr_lookup) */
	            if (rs < 0) break ;
	        } /* end for */

	        rs1 = userattr_close(&ua) ;
	        if (rs >= 0) rs = rs1 ;
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	} /* end if (not initialized) */

	return rs ;
}
/* end subroutine (datauser_ua) */


static int datauser_domain(DATAUSER *dup)
{
	int		rs = SR_OK ;

	if (dup == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("datauser_domain: un=%s\n",dup->un) ;
#endif

	if (! dup->init.domain) {
	    dup->init.domain = TRUE ;

	    if (! dup->init.ua) {
	        rs = datauser_ua(dup) ;
	    }

	    if ((rs >= 0) && dup->have.dn) {
	        rs = sncpy1(dup->domainname,MAXHOSTNAMELEN,dup->dn) ;
	        dup->have.domain = (rs >= 0) ;
	    }

#ifdef	COMMENT
	    if ((rs >= 0) && (! dup->have.domain)) {
	        const int	dlen = MAXHOSTNAMELEN ;
	        rs1 = udomain(NULL,dup->domainname,dlen,dup->un) ;
	        if (rs1 <= 0) {
	            dup->domainname[0] = '\0' ;
	        } else {
	            dup->have.domain = (dup->domainname[0] != '\0') ;
	        }
	    } /* end if */
#endif /* COMMENT */

	} /* end if (needed) */

#if	CF_DEBUGS
	debugprintf("datauser_domain: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (datauser_domain) */


static int datauser_pw(DATAUSER *dup)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_ent = FALSE ;

	if (dup == NULL) return SR_FAULT ;
	pip = dup->pip ;
	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("userinfo/datauser_pw: ent un=%s\n",dup->un) ;
#endif

	if (! dup->init.pent) {
	    PWENTRY	*pp = &dup->pent ;
	    uint	uiw ;
	    const int	plen = dup->pentlen ;
	    const int	rsn = SR_NOTFOUND ;
	    cchar	*un = dup->un ;
	    char	*pbuf = dup->pentbuf ;
	    dup->init.pent = TRUE ;
	    if (dup->pwfname == NULL) {
	        if ((rs = getpwentry_name(pp,pbuf,plen,un)) >= 0) {
	            dup->have.pent = TRUE ;
	            f_ent = TRUE ;
	        } else if ((rs == rsn) && hasalldig(un,-1)) {
	            if ((rs = cfdecui(un,-1,&uiw)) >= 0) {
	                const uid_t	tuid = uiw ;
	                if ((rs = getpwentry_uid(pp,pbuf,plen,tuid)) >= 0) {
	                    const int	unlen = USERNAMELEN ;
	                    char	*unbuf = dup->unbuf ;
	                    dup->have.pent = TRUE ;
	                    f_ent = TRUE ;
	                    dup->un = dup->unbuf ;
	                    strdcpy1(unbuf,unlen,pp->username) ;
	                } /* end if (getpwentry_uid) */
	            } /* end if (cfdec) */
	        } /* end if (hasalldig) */
	    } else {
	        PWFILE	pf ;
	        void	*n = NULL ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("datauser_pw: using PWFILE\n") ;
#endif

	        if ((rs = pwfile_open(&pf,dup->pwfname)) >= 0) {
	            if ((rs = pwfile_fetchuser(&pf,un,n,pp,pbuf,plen)) >= 0) {
	                dup->have.pent = TRUE ;
	                f_ent = TRUE ;
	            } else if ((rs == rsn) && hasalldig(un,-1)) {
	                const int	ulen = USERNAMELEN ;
	                if ((rs = cfdecui(un,-1,&uiw)) >= 0) {
	                    const uid_t		uid = uiw ;
	                    if ((rs = getusername(dup->unbuf,ulen,uid)) >= 0) {
	                        cchar	*un ;
	                        dup->un = dup->unbuf ;
	                        un = dup->un ;
	                        rs = pwfile_fetchuser(&pf,un,n,pp,pbuf,plen) ;
	                        if (rs >= 0) {
	                            dup->have.pent = TRUE ;
	                            f_ent = TRUE ;
	                        }
	                    } /* end if (getusername) */
	                } /* end if (cfdec) */
	            } /* end if (digits) */
	            rs1 = pwfile_close(&pf) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (opened PWFILE DB) */

	    } /* end if (system or file) */
	} else {
	    f_ent = dup->have.pent ;
	} /* end if (initialization) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("userinfo/datauser_pw: ret rs=%d f_ent=%u\n",rs,f_ent) ;
	    debugprintf("userinfo/datauser_pw: un=%s\n",dup->un) ;
	}
#endif

	return (rs >= 0) ? f_ent : rs ;
}
/* end subroutine (datauser_pw) */


/* find group entry for given username */
static int datauser_gr(DATAUSER *dup)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (dup == NULL) return SR_FAULT ;
	pip = dup->pip ;
	if (pip == NULL) return SR_FAULT ;

	if (! dup->init.gr) {
	    dup->init.gr = TRUE ;
	    if (! dup->init.pent) rs = datauser_pw(dup) ;
	    if ((rs >= 0) && dup->have.pent) {
	        struct group	*grp = &dup->gr ;
	        const gid_t	gid = dup->pent.gid ;
	        const int	grlen = dup->grlen ;
	        char		*grbuf = dup->grbuf ;
	        if ((rs = getgr_gid(grp,grbuf,grlen,gid)) >= 0) {
	            dup->have.gr = TRUE ;
	            f = TRUE ;
	        } else if (isNotPresent(rs)) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("userinfo/datauser_gr: getgr_gid() rs=%d\n",rs) ;
#endif
	            rs = SR_OK ;
	        }
	    } /* end if */
	} else {
	    f = dup->have.gr ;
	} /* end if (needed) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("userinfo/datauser_gr: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (datauser_gr) */


static int datauser_pj(DATAUSER *dup)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (dup == NULL) return SR_FAULT ;
	pip = dup->pip ;
	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("datauser_pj: f_initpj=%u\n",
	        dup->init.pj) ;
#endif

	if (! dup->init.pj) {
	    struct project	*pjp = &dup->pj ;
	    const int		pjlen = dup->pjlen ;
	    cchar		*un = dup->un ;
	    char		*pjbuf = dup->pjbuf ;
	    dup->init.pj = TRUE ;
	    if ((rs = uc_getdefaultproj(un,pjp,pjbuf,pjlen)) >= 0) {
	        dup->have.pj = TRUE ;
	        f = TRUE ;
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	} else {
	    f = dup->have.pj ;
	} /* end if (needed initialization) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("datauser_pj: have=%u projname=%s\n",
	        dup->have.pj,dup->pj.pj_name) ;
	    debugprintf("datauser_pj: ret rs=%d f=%u\n",rs,f) ;
	}
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (datauser_pj) */


/* find supplemental group names for a given username */
static int datauser_groups(DATAUSER *dup)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (dup == NULL) return SR_FAULT ;

	if (! dup->init.groups) {
	    dup->init.groups = TRUE ;
	    if ((rs = datauser_pw(dup)) >= 0) {
	        if ((rs = vecstr_start(&dup->groups,10,0)) >= 0) {
	            dup->have.groups = TRUE ;

#if	CF_DEFGROUP
	            rs = datauser_groupdef(dup) ;
	            c += 1 ;
#endif

	            if (rs >= 0) {
	                rs = datauser_groupsfind(dup) ;
	                c += rs ;
	            }

	        } /* end if (vecstr-start) */
	    } /* end if */
	} else {
	    if (dup->have.groups) {
	        rs = vecstr_count(&dup->groups) ;
	        c = rs ;
	    }
	} /* end if (initialization needed) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (datauser_groups) */


#if	CF_DEFGROUP
static int datauser_groupdef(DAYAUSER *dup)
{
	int		rs ;
	int		c = 0 ;

	if ((rs = datauser_gr(dup)) >= 0) {
	    cchar	*gn = dup->gr.gr_name ;
	    if ((gn != NULL) && (gn[0] != '\0')) {
	        if (vecstr_find(&dup->groups,gn) == SR_NOTFOUND) {
	            c += 1 ;
	            rs = vecstr_add(&dup->groups,gn,-1) ;
	        }
	    } /* end if (have a group-name) */
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (datauser_groupdef) */
#endif /* CF_DEFGROUP */


static int datauser_groupsfind(DATAUSER *dup)
{
	struct group	gr ;
	const int	grlen = getbufsize(getbufsize_gr) ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	char		*grbuf ;

	if ((rs = uc_malloc((grlen+1),&grbuf)) >= 0) {
	    SYSGROUP	sgr ;
	    if ((rs = sysgroup_open(&sgr,NULL)) >= 0) {
	        vecstr		*glp = &dup->groups ;
	        const int	rsn = SR_NOTFOUND ;
	        cchar		**groups ;
	        while ((rs = sysgroup_readent(&sgr,&gr,grbuf,grlen)) > 0) {
	            if (gr.gr_mem != NULL) {
	                cchar	*un = dup->un ;
	                cchar	*gn = gr.gr_name ;
	                groups = (cchar **) gr.gr_mem ;
	                if (matstr(groups,un,-1) >= 0) {
	                    if ((rs = vecstr_find(glp,gn)) == rsn) {
	                        c += 1 ;
	                        rs = vecstr_add(glp,gn,-1) ;
	                    }
	                } /* end if (match) */
	            } /* end if (non-null) */
	            if (rs < 0) break ;
	        } /* end while */
	        rs1 = sysgroup_close(&sgr) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sysgroup) */
	    uc_free(grbuf) ;
	} /* end if (m-a-f) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (datauser_groupsfind) */


/* find supplemental project names for a given username */
static int datauser_projects(DATAUSER *dup)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (dup == NULL) return SR_FAULT ;
	pip = dup->pip ;
	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("userinfo/datauser_projects: ent init=%u\n",
	        dup->init.projects) ;
	}
#endif

	if (! dup->init.projects) {
	    dup->init.projects = TRUE ;
	    if ((rs = datauser_pw(dup)) >= 0) {
	        if ((rs = datauser_gr(dup)) >= 0) {
	            if ((rs = datauser_groups(dup)) >= 0) {
	                vecstr	*pjp = &dup->projects ;
	                if ((rs = vecstr_start(pjp,10,0)) >= 0) {
	                    dup->have.projects = TRUE ;
	                    if ((rs = datauser_pj(dup)) >= 0) {
	                        cchar	*pn = dup->pj.pj_name ;
	                        if (dup->have.pj && (pn[0] != '\0')) {
	                            const int	rsn = SR_NOTFOUND ;
	                            if ((rs = vecstr_find(pjp,pn)) == rsn) {
	                                c += 1 ;
	                                rs = vecstr_add(pjp,pn,-1) ;
	                            }
	                        }
	                        if (rs >= 0) {
	                            rs = datauser_projectsfind(dup) ;
	                            c += rs ;
	                        }
	                    } /* end if (datauser_pj) */
	                    if (rs < 0) {
	                        dup->have.projects = FALSE ;
	                        vecstr_finish(pjp) ;
	                    }
	                } /* end if (vecstr_start) */
	            } /* end if (datauser_groups) */
	        } /* end if (datauser_gr) */
	    } /* end if (datauser_pw) */
	} else {
	    if (dup->have.projects) {
	        vecstr	*pjp = &dup->projects ;
	        rs = vecstr_count(pjp) ;
	        c = rs ;
	    }
	} /* end if (initialization needed) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("userinfo/datauser_projects: ret have-projects=%u\n",
	        dup->have.projects) ;
	    debugprintf("userinfo/datauser_projects: ret rs=%d c=%u\n",rs,c) ;
	}
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (datauser_projects) */


static int datauser_projectsfind(DATAUSER *dup)
{
	struct project	pj ;
	const int	pjlen = getbufsize(getbufsize_pj) ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	char		*pjbuf ;

	if ((rs = uc_malloc((pjlen+1),&pjbuf)) >= 0) {
	    SYSPROJECT	spj ;
	    if ((rs = sysproject_open(&spj,NULL)) >= 0) {
	        vecstr		*glp = &dup->groups ;
	        vecstr		*plp = &dup->projects ;
	        const int	rsn = SR_NOTFOUND ;
	        int		f ;
	        cchar		*un = dup->un ;
	        cchar		*gn = dup->gr.gr_name ;
	        while ((rs = sysproject_readent(&spj,&pj,pjbuf,pjlen)) > 0) {
	            f = FALSE ;
	            if ((! f) && (pj.pj_users != NULL)) {
	                cchar	**users = (cchar **) pj.pj_users ;
	                f = (matstr(users,un,-1) >= 0) ;
	            } /* end if */
	            if ((! f) && (pj.pj_groups != NULL)) {
	                int	i ;
	                cchar	**groups = (cchar **) pj.pj_groups ;
	                for (i = 0 ; groups[i] != NULL ; i += 1) {
	                    if (dup->have.gr) {
	                        if (strcmp(gn,groups[i]) == 0)
	                            break ;
	                    }
	                    if (dup->have.groups) {
	                        if (vecstr_find(glp,groups[i]) >= 0)
	                            break ;
	                    }
	                } /* end for */
	                f = (groups[i] != NULL) ;
	            } /* end if */
	            if (f && (vecstr_find(plp,pj.pj_name) == rsn)) {
	                c += 1 ;
	                rs = vecstr_add(plp,pj.pj_name,-1) ;
	            }
	            if (rs < 0) break ;
	        } /* end while (reading entries) */
	        rs1 = sysproject_close(&spj) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sysproject) */
	    uc_free(pjbuf) ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (datauser_projectsfind) */


/* timezone */
static int datauser_tz(DATAUSER *dup)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (dup == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("datauser_tz: ent\n") ;
#endif

	if (! dup->init.tz) {
	    dup->init.tz = TRUE ;
	    if (! dup->init.ua) rs = datauser_ua(dup) ;
	    if ((rs >= 0) && (! dup->have.tz)) {
	        if ((rs = inittimezone(dup->tz,TZLEN,DEFINITFNAME)) >= 0) {
	            dup->have.tz = TRUE ;
	            f = TRUE ;
	        }
	    } /* end if */
	} else {
	    f = dup->have.tz ;
	} /* end if (needed initialization) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (datauser_tz) */


static int datauser_lastlog(DATAUSER *dup)
{
	int		rs = SR_OK ;

	if (dup == NULL) return SR_FAULT ;

	if (! dup->init.lastlog) {
	    dup->init.lastlog = TRUE ;
	    if ((rs >= 0) && (! dup->have.lastlog)) {
	        if (! dup->init.pent) rs = datauser_pw(dup) ;
	        if ((rs >= 0) && (dup->have.pent)) {
	            const uid_t	uid = dup->pent.uid ;
	            time_t	t ;
	            rs = lastlogin(NULL,uid,&t,dup->lasthost,dup->lastline) ;
	            dup->lasttime = t ;
	            dup->have.lastlog = (rs >= 0) ;
	        }
	    }
	} /* end if (needed initialization) */

	return rs ;
}
/* end subroutine (datauser_lastlog) */


static int datauser_statvfs(DATAUSER *dup)
{
	int		rs = SR_OK ;

	if (dup == NULL) return SR_FAULT ;

	if (! dup->init.statvfs) {
	    dup->init.statvfs = TRUE ;
	    if ((rs >= 0) && (! dup->have.statvfs)) {
	        if (! dup->init.pent) {
	            rs = datauser_pw(dup) ;
	        }
	        if ((rs >= 0) && (dup->have.pent)) {
	            rs = statvfsdir(dup->pent.dir,&dup->fss) ;
	            dup->have.statvfs = (rs >= 0) ;
	        }
	    } /* end if */
	}

	return rs ;
}
/* end subroutine (datauser_statvfs) */


static int datauser_utmpent(DATAUSER *dup)
{
	PROGINFO	*pip = dup->pip ;
	int		rs = SR_OK ;

	if (dup == NULL) return SR_FAULT ;
	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_userinfo: datauser_utmpent init=%u\n",
	        dup->init.utmpent) ;
#endif
	if (! dup->init.utmpent) {
	    dup->init.utmpent = TRUE ;
	    if (! dup->have.utmpent) {
	        int	rs1 = getutmpent(&dup->ue,0) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(5)) {
	            debugprintf("b_userinfo: datauser_utmpent rs1=%d\n",rs1) ;
	            debugprintf("b_userinfo/datauser_utmpent session=%d\n",
	                dup->ue.session) ;
	        }
#endif
	        dup->have.utmpent = (rs1 >= 0) ;
	        if (rs1 != SR_NOTFOUND) rs = rs1 ;
	    } /* end if */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_userinfo/datauser_utmpent: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (datauser_utmpent) */


static int datauser_orgcode(DATAUSER *dup)
{
	PROGINFO	*pip = dup->pip ;
	const int	bl = ORGCODELEN ;
	int		rs = SR_OK ;
	int		len = 0 ;
	char		*bp = dup->orgcode ;

	if (bp[0] == '\0') {
	    rs = localgetorgcode(pip->pr,bp,bl,dup->un) ;
	    len = rs ;
	} else {
	    len = strlen(bp) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (datauser_orgcode) */


static int datauser_orgloc(DATAUSER *dup)
{
	PROGINFO	*pip = dup->pip ;
	const int	bl = ORGLOCLEN ;
	int		rs = SR_OK ;
	int		len = 0 ;
	char		*bp = dup->orgloc ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_userinfo/_orgloc: un=%s\n",dup->un) ;
#endif

	if (bp[0] == '\0') {
	    rs = localgetorgloc(pip->pr,bp,bl,dup->un) ;
	    len = rs ;
	} else {
	    len = strlen(bp) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_userinfo/_orgloc: ret rs=%d orgloc=%t\n",rs,bp,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (datauser_orgloc) */


static int datauser_lastseen(DATAUSER *dup)
{
	const int	bl = LASTLOGFILE_LLINE ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		len = 0 ;
	char		*bp = dup->lastseen ;

	if (bp[0] == '\0') {
	    if ((rs = datauser_pw(dup)) >= 0) {
	        vecstr	tl ;
	        if ((rs = vecstr_start(&tl,4,0)) >= 0) {
	            rs = datauser_lastseener(dup,bp,bl,&tl) ;
	            len = rs ;
	            rs1 = vecstr_finish(&tl) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (vecstr) */
	    } /* end if (datauser_pw) */
	} else {
	    len = strlen(bp) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (datauser_lastseen) */


static int datauser_lastseener(DATAUSER *dup,char *bp,int bl,vecstr *tlp)
{
	TMPX		ut ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	if ((rs = tmpx_open(&ut,NULL,O_RDONLY)) >= 0) {
	    cchar	*un = dup->un ;
	    if ((rs = tmpx_getuserlines(&ut,tlp,un)) > 0) {
	        char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = mkpath1(tbuf,DEVDNAME)) >= 0) {
	            USTAT	sb ;
	            time_t	max = 0 ;
	            const int	tlen = rs ;
	            int		i ;
	            cchar	*lp ;
	            for (i = 0 ; vecstr_get(tlp,i,&lp) >= 0 ; i += 1) {
	                if (lp != NULL) {
	                    if ((rs = pathadd(tbuf,tlen,lp)) >= 0) {
	                        if (u_stat(tbuf,&sb) >= 0) {
	                            if (sb.st_mode & S_IWGRP) {
	                                if (sb.st_mtime > max) {
	                                    max = sb.st_mtime ;
	                                    len = strdcpy1(bp,bl,lp) - bp
	                                        ;
	                                }
	                            } /* end if (group-writable) */
	                        } /* end if (stat) */
	                    } /* end if (pathadd) */
	                }
	                if (rs < 0) break ;
	            } /* end for (looping over lines) */
	        } /* end if (mkpath) */
	    } /* end if (tmpx_getuserlines) */
	    rs1 = tmpx_close(&ut) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (tmpx) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (datauser_lastseener) */


static int datauser_username(DATAUSER *dup,char *cbuf,int clen)
{
	int		rs ;
	if ((rs = datauser_pw(dup)) >= 0) {
	    if (dup->have.pent) {
	        cchar	*un = dup->pent.username ;
	        rs = sncpy1(cbuf,clen,un) ;
	    }
	}
	return rs ;
}
/* end subroutine (datauser_username) */


static int datauser_realname(DATAUSER *dup,char *cbuf,int clen)
{
	int		rs ;
	if ((rs = datauser_pw(dup)) >= 0) {
	    if (dup->have.pent) {
	        cchar	*rn = dup->pent.realname ;
	        rs = mkrealname(cbuf,clen,rn,-1) ;
	    }
	}
	return rs ;
}
/* end subroutine (datauser_realname) */


static int datauser_netname(DATAUSER *dup,cchar *nis)
{
	int		rs ;
	if (dup->netname == NULL) {
	    if ((rs = datauser_pw(dup)) >= 0) {
	        if (dup->have.pent) {
	            const int	dlen = DIGBUFLEN ;
	            const int	v = dup->pent.uid ;
	            char	dbuf[DIGBUFLEN+1] ;
	            if ((rs = ctdeci(dbuf,dlen,v)) >= 0) {
	                const int	nlen = MAXNETNAMELEN ;
	                char		nbuf[MAXNETNAMELEN+1] ;
	                cchar		*u = "unix" ;
			cchar		*d = "." ;
	                if ((rs = sncpy5(nbuf,nlen,u,d,dbuf,"@",nis)) >= 0) {
	                    cchar	*np ;
	                    if ((rs = uc_mallocstrw(nbuf,rs,&np)) >= 0) {
	                        dup->netname = np ;
	                    }
	                }
	            }
	        }
	    }
	} else {
	    rs = strlen(dup->netname) ;
	}
	return rs ;
}
/* end subroutine (datauser_netname) */


static int datauser_mkgids(DATAUSER *dup,char *rbuf,int rlen)
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;
	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    const int	grlen = getbufsize(getbufsize_gr) ;
	    char	*grbuf ;
	    if ((rs = uc_malloc((grlen+1),&grbuf)) >= 0) {
	        vecstr	*glp = &dup->groups ;
	        int	i ;
	        int	c = 0 ;
	        cchar	*cp ;
	        for (i = 0 ; vecstr_get(glp,i,&cp) >= 0 ; i += 1) {
	            if (cp != NULL) {
	                if (c++ > 0) sbuf_char(&b,' ') ;
	                if ((rs = mkgid(grbuf,grlen,cp)) >= 0) {
	                    sbuf_deci(&b,rs) ;
	                } else if (rs == SR_NOTFOUND) {
	                    rs = sbuf_strw(&b,"?",1) ;
	                }
	            }
	        } /* end for */
	        rs1 = uc_free(grbuf) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (m-a) */
	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */
	return rs ;
}
/* end subroutine (datauser_mkgids) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;

	return SR_OK ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.ns) {
	    lip->open.ns = FALSE ;
	    rs1 = pcsns_close(&lip->ns) ;
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


static int locinfo_prpcs(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;

	if (lip->pr_pcs == NULL) {
	    const int	plen = MAXPATHLEN ;
	    cchar	*dn = pip->domainname ;
	    char	pbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpr(pbuf,plen,VARPRPCS,dn)) >= 0) {
	        cchar	**vpp = &lip->pr_pcs ;
	        rs = locinfo_setentry(lip,vpp,pbuf,rs) ;
	    }
	} else {
	    rs = strlen(lip->pr_pcs) ;
	}

	return rs ;
}
/* end subroutine (locinfo_prpcs) */


static int locinfo_pcsns(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (! lip->open.ns) {
	    if ((rs = locinfo_prpcs(lip)) >= 0) {
	        cchar	*pr_pcs = lip->pr_pcs ;
	        if ((rs = pcsns_open(&lip->ns,pr_pcs)) >= 0) {
	            lip->open.ns = TRUE ;
	        }
	    }
	} /* end if (needed initialization) */
	return rs ;
}
/* end subroutine (locinfo_pcsns) */


static int locinfo_pcsnsget(LOCINFO *lip,char *rbuf,int rlen,cchar *un,int w)
{
	int		rs ;
	if ((rs = locinfo_pcsns(lip)) >= 0) {
	    rs = pcsns_get(&lip->ns,rbuf,rlen,un,w) ;
	}
	return rs ;
}
/* end subroutine (locinfo_pcsnsget) */


static int locinfo_setphone(LOCINFO *lip,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		oi ;
	if ((oi = matostr(phonetypes,2,vp,vl)) >= 0) {
	    lip->phone = oi ;
	} /* end if */
	return rs ;
}
/* end subroutine (locinfo_setphone) */


static int mkstrlist(char *cbuf,int clen,vecstr *lp)
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;
	if ((rs = sbuf_start(&b,cbuf,clen)) >= 0) {
	    int		i ;
	    int		c = 0 ;
	    cchar	*cp ;
	    for (i = 0 ; vecstr_get(lp,i,&cp) >= 0 ; i += 1) {
	        if (cp != NULL) {
	            if (c++ > 0) sbuf_char(&b,' ') ;
	            rs = sbuf_strw(&b,cp,-1) ;
	        }
	        if (rs < 0) break ;
	    } /* end for */
	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkstrlist) */


static int mkgid(char *grbuf,int grlen,cchar *gname)
{
	struct group	gr ;
	int		rs ;
	if ((rs = getgr_name(&gr,grbuf,grlen,gname)) >= 0) {
	    rs = gr.gr_gid ;
	}
	return rs ;
}
/* end subroutine (mkgid) */


/* yes, this subroutine degenerates into an existing interface */
static int getuser(char *ubuf,int ulen,uid_t uid)
{
	return getusername(ubuf,ulen,uid) ;
}
/* end subroutine (getuser) */


