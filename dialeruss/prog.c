/* prog */

/* SYSDIALER "prog" dialer */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-11-04, David A­D­ Morano
        This code was started by taking the corresponding code from the
        TCP-family module. In retrospect, that was a mistake. Rather I should
        have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a dialer module.

	Synopsis:

	prog <execfile>

	Arguments:

	+ dialer arguments
	+ hostname
	+ service
	+ service arguments


*******************************************************************************/


#define	PROG_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<ids.h>
#include	<userinfo.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<sbuf.h>
#include	<buffer.h>
#include	<expcook.h>
#include	<paramfile.h>
#include	<nulstr.h>
#include	<logfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"sysdialer.h"
#include	"prog.h"
#include	"envs.h"
#include	"sysvar.h"


/* local defines */

#define	PROG_MNAME	"prog"
#define	PROG_VERSION	"0"
#define	PROG_INAME	""
#define	PROG_FLAGS1	\
		(SYSDIALER_MFULL | SYSDIALER_MHALFOUT | SYSDIALER_MHALFIN)
#define	PROG_FLAGS2	(SYSDIALER_MCOR | SYSDIALER_MCO)
#define	PROG_FLAGS3	(SYSDIALER_MARGS)
#define	PROG_FLAGS	(PROG_FLAGS1 | PROG_FLAGS2 | PROG_FLAGS3)

#define	PROG_MAGIC	0x31455926
#define	PROG_SEARCHNAME	"dialerprog"
#define	PROG_VARPR	"LOCAL"
#define	PROG_PR		"/usr/add-on/local"
#define	PROG_LOGDNAME	"log"
#define	PROG_LOGFNAME	SYSDIALER_LF

#define	PROG_DEFPROG	"/usr/bin/ksh"
#define	PROG_DEFPATH	"/bin:/usr/bin"

#ifndef	NOFILE
#define	NOFILE		20
#endif

#ifndef	KBUFLEN
#define	KBUFLEN		120
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(10 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(10 * MAXPATHLEN)
#endif

#ifndef	BUFLEN
#define	BUFLEN		((2 * MAXPATHLEN) + 20)
#endif

#ifndef	ARCHBUFLEN
#define	ARCHBUFLEN	80
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#undef	DEBUGFNAME
#define	DEBUGFNAME	"/tmp/lsh.nd"

#define	ARGBUFLEN	(MAXPATHLEN + 35)
#define	PATHBUFLEN	((4 * MAXPATHLEN) + 3)

#define	MAXARGINDEX	100
#define	NARGPRESENT	(MAXARGINDEX/8 + 1)

#ifndef	VARARCHITECTURE
#define	VARARCHITECTURE	"ARCHITECTURE"
#endif

#ifndef	VARHZ
#define	VARHZ		"VARHZ"
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	VARAST
#define	VARAST		"AST"
#endif

#ifndef	VARDEFPROG
#define	VARDEFPROG	"ENVSET_DEFPROG"
#endif

#define	INITFNAME	"/etc/default/init"
#define	PATHFNAME	"etc/path"

#define	DEFSFNAME	"def"
#define	XEFNAME		"xe"
#define	PVARSFNAME	"pvar"

#define	DEFENVS		200

#define	DEFNDEFS	20
#define	DEFNXENVS	200

#define	W_OPTIONS	(WNOHANG)

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags
#define	SUBINFO_ALLOCS	struct subinfo_allocs


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	ctdeci(char *,int,int) ;
extern int	ctdecl(char *,int,long) ;
extern int	optbool(const char *,int) ;
extern int	gethz(int) ;
extern int	getpwd(char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	vecstr_loadfile(vecstr *,int,const char *) ;
extern int	vecstr_envset(VECSTR *,const char *,const char *,int) ;
extern int	vecstr_envadd(VECSTR *,const char *,const char *,int) ;
extern int	vecstr_envadds(VECSTR *,const char *,int) ;
extern int	fperm(int,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	getprogpath(IDS *,VECSTR *,char *,const char *,int) ;
extern int	msleep(uint) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,cchar *,cchar *) ;
extern int	hasalldig(const char *,int) ;

extern int	envs_procxe(ENVS *,EXPCOOK *,const char **,vecstr *,cchar *) ;
extern int	envs_subs(vecstr *,EXPCOOK *,VECSTR *,ENVS *) ;
extern int	getnprocessors(const char **,int) ;
extern int	defproc(vecstr *,const char **,EXPCOOK *,const char *) ;

#if	CF_DEBUGS
extern int	nprintf(const char *,const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */

extern const char	**environ ;


/* local structures */

struct subinfo_flags {
	uint		ids:1 ;
	uint		userinfo:1 ;
	uint		svars:1 ;
	uint		ignore:1 ;
	uint		progdash:1 ;
	uint		log:1 ;
} ;

struct subinfo_allocs {
	const char	*node ;
	const char	*svc ;
	const char	*pr ;
} ;

struct subinfo {
	const char	**argv ;
	const char	**envv ;
	const char	*pr ;
	const char	*prn ;
	const char	*searchname ;
	const char	*hostname ;
	const char	*svcname ;
	const char	*program ;
	const char	*pvfname ;
	const char	*dfname ;
	const char	*xfname ;
	const char	*efname ;
	const char	*architecture ;		/* machine architecture */
	const char	*umachine ;		/* UNAME machine name */
	const char	*usysname ;		/* UNAME OS system-name */
	const char	*urelease ;		/* UNAME OS release */
	const char	*uversion ;		/* UNAME OS version */
	const char	*hz ;			/* OS HZ */
	const char	*nodename ;		/* USERINFO */
	const char	*domainname ;		/* USERINFO */
	const char	*username ;		/* USERINFO */
	const char	*homedname ;		/* USERINFO */
	const char	*shell ;		/* USERINFO */
	const char	*organization ;		/* USERINFO */
	const char	*gecosname ; 		/* USERINFO */
	const char	*realname ;		/* USERINFO */
	const char	*name ;			/* USERINFO */
	const char	*tz ;			/* USERINFO */
	const char	*groupname ;
	const char	*tmpdname ;
	const char	*maildname ;
	const char	*helpfname ;
	const char	*logfname ;
	const char	*paramfname ;
	const char	*logid ;
	const char	*defprog ;
	PROG		*op ;
	SYSDIALER_ARGS	*ap ;
	IDS		id ;
	VECSTR		aenvs ;
	VECSTR		stores ;
	VECSTR		defs ;
	VECSTR		pvars, exports ;
	EXPCOOK		cooks ;
	vecstr		svars ;
	ENVS		xenvs ;
	USERINFO	u ;
	SUBINFO_ALLOCS	a ;
	SUBINFO_FL	f ;
	uid_t		uid ;
	gid_t		gid ;
	int		argc ;
	int		argi ;
	int		ncpu ;
	char		userinfobuf[USERINFO_LEN + 1] ;
} ;

struct intprog {
	char		fname[MAXPATHLEN + 1] ;
	char		arg[MAXPATHLEN + 1] ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,PROG *,SYSDIALER_ARGS *,
			cchar *,cchar *) ;
static int	subinfo_procargs(SUBINFO *) ;
static int	subinfo_procopts(SUBINFO *,KEYOPT *) ;
static int	subinfo_defaults(SUBINFO *) ;
static int	subinfo_userinfo(SUBINFO *) ;
static int	subinfo_findprog(SUBINFO *,char *) ;
static int	subinfo_search(SUBINFO *,VECSTR *,char *,const char *) ;
static int	subinfo_envdialer(SUBINFO *) ;
static int	subinfo_sasize(SUBINFO *) ;
static int	subinfo_sabuild(SUBINFO *,char *) ;
static int	subinfo_exec(SUBINFO *,cchar *,cchar **) ;
static int	subinfo_logfile(SUBINFO *) ;
static int	subinfo_dirok(SUBINFO *,const char *,int) ;
static int	subinfo_setentry(SUBINFO *,cchar **,cchar *,int) ;
static int	subinfo_finish(SUBINFO *) ;

static int	loadgroupname(SUBINFO *) ;
static int	loadarchitecture(SUBINFO *) ;
static int	loadhz(SUBINFO *) ;
static int	loadcooks(SUBINFO *) ;

static int	loadpathlist(SUBINFO *,VECSTR *,VECSTR *) ;
static int	loadpathcomp(SUBINFO *,VECSTR *,const char *) ;

static int	loaddefsfile(SUBINFO *,const char *) ;
static int	loaddefs(SUBINFO *,const char **) ;
static int	loadxfile(SUBINFO *,const char *) ;
static int	loadxsched(SUBINFO *,const char **) ;
static int	loadpvars(SUBINFO *,cchar **,cchar *) ;
static int	loadpvarsdef(SUBINFO *,const char **) ;

static int	pvars_begin(SUBINFO *,const char **,const char *) ;
static int	pvars_end(SUBINFO *) ;

static int	sched_begin(SUBINFO *) ;
static int	sched_end(SUBINFO *) ;

static int	procenvextra(SUBINFO *) ;
static int	procenvdef(SUBINFO *) ;
static int	procenvsys(SUBINFO *,const char *) ;
static int	procdefprog(SUBINFO *,const char **) ;

static int	xfile(IDS *,const char *) ;

#ifdef	COMMENT
static int	loadpath(vecstr *,const char *) ;
static int	intprog(struct intprog *,const char *) ;
static int	xfile(IDS *,const char *) ;
static int	setdefpath(vecstr *,const char *) ;
static int	createsearchpath(VECSTR *,const char *) ;
#endif /* COMMENT */

#ifdef	COMMENT
static int	mkpathvar(SUBINFO *,vecstr *,const char *,char **) ;
#endif

static int	isplusminus(const char *) ;
static int	isminus(const char *) ;


/* global variables (module information) */

SYSDIALER_INFO	prog = {
	PROG_MNAME,
	PROG_VERSION,
	PROG_INAME,
	sizeof(PROG),
	PROG_FLAGS
} ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"RN",
	"sn",
	"pvf",
	"pf",
	"df",
	"xf",
	"ef",
	"lf",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_rn,
	argopt_sn,
	argopt_pvf,
	argopt_pf,
	argopt_df,
	argopt_xf,
	argopt_ef,
	argopt_lf,
	argopt_overlast
} ;

static const char *procopts[] = {
	"log",
	NULL
} ;

enum procopts {
	procopt_log,
	procopt_overlast
} ;

static const char	*cparams[] = {
	"defprog",
	NULL
} ;

enum cparams {
	cparam_defprog,
	cparam_overlast
} ;

static const char	*cooks[] = {
	"MACHINE",	/* machine-name */
	"ARCHITECTURE",	/* machine-architecture */
	"NCPU",		/* number of machine CPUs */
	"SYSNAME",	/* OS system-name */
	"RELEASE",	/* OS system-release */
	"VERSION",	/* OS system-version */
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
	"RN",		/* program root-name */
	NULL
} ;

enum cooks {
	cook_machine,
	cook_architecture,
	cook_ncpu,
	cook_sysname,
	cook_release,
	cook_version,
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
	cook_overlast
} ;

static const char	*schedhconf[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	NULL
} ;

static const char	*schedpconf[] = {
	"%h/%e/%n/%n.%f",
	"%h/%e/%n/%f",
	"%h/%e/%n.%f",
	NULL
} ;

static const char	*schedpfile[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%e/%f",
	NULL
} ;

static const char	*schedhfile[] = {
	"%h/%e/%n/%n.%f",
	"%h/%e/%n/%f",
	"%h/%e/%n.%f",
	"%h/%e/%f",
	NULL
} ;

static const char	*pathvars[] = {
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

static const char	*envbad[] = {
	"_",
	"_A0",
	"_EF",
	"A__z",
	"RANDOM",
	"TMOUT",
	NULL
} ;

static const char	*envsys[] = {
	"SYSNAME",
	"RELEASE",
	"VERSION",
	"MACHINE",
	"ARCHITECTURE",
	"HZ",
	"NODE",
	"NCPU",
	"USERNAME",
	"GROUPNAME",
	"DOMAIN",
	NULL
} ;

static const char	*envdialers[] = {
	"SYSDIALER_ROOT",
	"SYSDIALER_HOST",
	"SYSDIALER_SVC",
	"SYSDIALER_SVCARGS",
	NULL
} ;

enum envdialers {
	envdialer_root,
	envdialer_host,
	envdialer_svc,
	envdialer_svcargs,
	envdialer_overlast
} ;


/* exported subroutines */


int prog_open(op,ap,hostname,svcname,av)
PROG		*op ;
SYSDIALER_ARGS	*ap ;
const char	hostname[] ;
const char	svcname[] ;
const char	*av[] ;
{
	SUBINFO		si, *sip = &si ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		opts = 0 ;
	cchar		*progfname = NULL ;
	char		progfnamebuf[MAXPATHLEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (ap == NULL) return SR_FAULT ;
	if (hostname == NULL) return SR_FAULT ;

	if (hostname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(PROG)) ;

#if	CF_DEBUGS
	debugprintf("prog_open: ent hostname=%s svcname=%s\n",
	    hostname,svcname) ;
	if (ap->argv != NULL) {
		int	i ;
	    for (i = 0 ; ap->argv[i] != NULL ; i += 1) {
		debugprintf("prog_open: a%u=>%s<\n",i,ap->argv[i]) ;
	    }
	}
#endif /* CF_DEBUGS */

	rs = subinfo_start(sip,op,ap,hostname,svcname) ;
	if (rs < 0)
	    goto bad0 ;

	rs = subinfo_userinfo(sip) ;

	if (rs >= 0)
	    rs = loadgroupname(sip) ;

	if (rs >= 0)
	    rs = loadarchitecture(sip) ;

	if (rs >= 0)
	    rs = loadhz(sip) ;

#if	CF_DEBUGS
	debugprintf("prog_open: various loads completed rs=%d\n",rs) ;
	debugprintf("prog_open: f_log=%u\n",op->f.log) ;
#endif

	if (rs < 0)
	    goto baduser ;

#if	CF_DEBUGS
	debugprintf("prog_open: subinfo_defaults()\n") ;
	debugprintf("prog_open: f_log=%u\n",op->f.log) ;
#endif

	if (rs >= 0)
	    rs = subinfo_defaults(sip) ;

#if	CF_DEBUGS
	debugprintf("prog_open: subinfo_logfile()\n") ;
	debugprintf("prog_open: f_log=%u\n",op->f.log) ;
#endif

	if (rs >= 0)
	    rs = subinfo_logfile(sip) ;

/* initialize and load up the cookies */

#if	CF_DEBUGS
	debugprintf("prog_open: cookies\n") ;
	debugprintf("prog_open: f_log=%u\n",op->f.log) ;
#endif

	rs = expcook_start(&sip->cooks) ;
	if (rs < 0)
	    goto badcookinit ;

	rs = loadcooks(sip) ;
	if (rs < 0)
	    goto badcookload ;

#if	CF_DEBUGS
	debugprintf("prog_open: sched_begin\n") ;
	debugprintf("prog_open: f_log=%u\n",op->f.log) ;
#endif

	rs = sched_begin(sip) ;

#if	CF_DEBUGS
	debugprintf("prog_open: sched_begin() rs=%d\n",rs) ;
	debugprintf("prog_open: f_log=%u\n",op->f.log) ;
#endif

	if (rs < 0)
	    goto badsched_begin ;

#if	CF_DEBUGS
	debugprintf("prog_open: init defs\n") ;
	debugprintf("prog_open: f_log=%u\n",op->f.log) ;
#endif

	rs = vecstr_start(&sip->defs,DEFNDEFS,0) ;
	if (rs < 0)
	    goto baddefsinit ;

	if ((sip->dfname != NULL) && (sip->dfname != '\0'))
	    rs = loaddefsfile(sip,sip->dfname) ;

	if (rs >= 0)
	    rs = loaddefs(sip,schedhfile) ;

	if (rs >= 0)
	    rs = loaddefs(sip,schedpfile) ;

	if (rs < 0)
	    goto baddefsload ;

	vecstr_sort(&sip->defs,vstrkeycmp) ;

#if	CF_DEBUGS
	debugprintf("prog_open: envs_start\n") ;
	debugprintf("prog_open: f_log=%u\n",op->f.log) ;
#endif

	rs = envs_start(&sip->xenvs,DEFNXENVS) ;
	if (rs < 0)
	    goto badenvsinit ;

/* load up the environment variables */

	if (rs >= 0)
	    rs = loadxsched(sip,schedpfile) ;

	if (rs >= 0)
	    rs = loadxsched(sip,schedhfile) ;

	if ((rs >= 0) && (sip->xfname != NULL) && (sip->xfname[0] != '\0'))
	    rs = loadxfile(sip,sip->xfname) ;

	if (rs < 0)
	    goto badenvsload ;

	opts = VECSTR_OCOMPACT ;
	rs = vecstr_start(&sip->exports,DEFNXENVS,opts) ;
	if (rs < 0)
	    goto badxinit ;

#if	CF_DEBUGS
	debugprintf("prog_open: pvars_begin() rs=%d\n",rs) ;
	debugprintf("prog_open: f_log=%u\n",op->f.log) ;
#endif

	rs = pvars_begin(sip,pathvars,sip->pvfname) ;
	if (rs < 0)
	    goto badpvars_begin ;

#if	CF_DEBUGS
	debugprintf("prog_open: envs_subs() rs=%d\n",rs) ;
	debugprintf("prog_open: f_log=%u\n",op->f.log) ;
#endif

	rs = envs_subs(&sip->exports,&sip->cooks,&sip->pvars,&sip->xenvs) ;
	if (rs < 0)
	    goto badenvsubs ;

	if (rs >= 0)
	    rs = procenvextra(sip) ;

	if (rs >= 0)
	    rs = procenvdef(sip) ;

	if (rs >= 0)
	    rs = procenvsys(sip,NULL) ;

	if (rs < 0)
	    goto badprocenvsys ;

/* find the program */

#if	CF_DEBUGS
	debugprintf("prog_open: program=%s\n",sip->program) ;
	debugprintf("prog_open: f_log=%u\n",op->f.log) ;
#endif

	progfname = progfnamebuf ;
	rs = subinfo_findprog(sip,progfnamebuf) ;

#if	CF_DEBUGS
	debugprintf("prog_open: subinfo_findprog() rs=%d\n",rs) ;
	debugprintf("prog_open: progfname=%s\n",progfname) ;
	debugprintf("prog_open: f_log=%u\n",op->f.log) ;
#endif

	if (rs < 0)
	    goto badfindprog ;

	if (rs >= 0) {
	    rs = subinfo_envdialer(sip) ;
#if	CF_DEBUGS
	debugprintf("prog_open: subinfo_envdialer() rs=%d\n",rs) ;
#endif
	}

	if (rs >= 0) {
	    rs = subinfo_exec(sip,progfname,av) ;
#if	CF_DEBUGS
	debugprintf("prog_open: subinfo_exec() rs=%d\n",rs) ;
#endif
	}

	if (rs >= 0)
	    op->magic = PROG_MAGIC ;

badfindprog:
badprocenvsys:
badenvsubs:
	pvars_end(sip) ;

badpvars_begin:
	vecstr_finish(&sip->exports) ;

badxinit:

badenvsload:
	envs_finish(&sip->xenvs) ;

badenvsinit:
baddefsload:
	vecstr_finish(&sip->defs) ;

baddefsinit:
	sched_end(sip) ;

badsched_begin:
badcookload:
	expcook_finish(&sip->cooks) ;

badcookinit:
ret3:
ret2:
	if ((rs < 0) && op->f.log) {

	    logfile_printf(&op->lh,"failed (%d)",rs) ;

	    logfile_close(&op->lh) ;
	    op->f.log = FALSE ;

	} /* end if */

baduser:
ret1:
	rs1 = subinfo_finish(sip) ;
	if (rs >= 0) rs = rs1 ;

	if ((rs < 0) && (op->fd >= 0)) {
		uc_close(op->fd) ;
		op->fd = -1 ;
	}

ret0:
bad0:

#if	CF_DEBUGS
	debugprintf("prog_open: ret rs=%d fd=%d\n",rs,op->fd) ;
	debugprintf("prog_open: f_log=%u\n",op->f.log) ;
#endif

	return (rs >= 0) ? op->fd : rs ;
}
/* end subroutine (prog_open) */


int prog_reade(op,buf,buflen,to,opts)
PROG		*op ;
char		buf[] ;
int		buflen ;
int		to, opts ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PROG_MAGIC) return SR_NOTOPEN ;

	rs = uc_reade(op->fd,buf,buflen,to,opts) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (prog_reade) */


int prog_recve(op,buf,buflen,flags,to,opts)
PROG		*op ;
char		buf[] ;
int		buflen ;
int		flags ;
int		to, opts ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PROG_MAGIC) return SR_NOTOPEN ;

	rs = uc_recve(op->fd,buf,buflen,flags,to,opts) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (prog_recve) */


int prog_recvfrome(op,buf,buflen,flags,sap,salenp,to,opts)
PROG		*op ;
char		buf[] ;
int		buflen ;
int		flags ;
void		*sap ;
int		*salenp ;
int		to, opts ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PROG_MAGIC) return SR_NOTOPEN ;

	rs = uc_recvfrome(op->fd,buf,buflen,flags,sap,salenp,to,opts) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (prog_recvfrom) */


int prog_recvmsge(op,msgp,flags,to,opts)
PROG		*op ;
struct msghdr	*msgp ;
int		flags ;
int		to, opts ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PROG_MAGIC) return SR_NOTOPEN ;

	rs = uc_recvmsge(op->fd,msgp,flags,to,opts) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (prog_recvmsge) */


int prog_write(op,buf,buflen)
PROG		*op ;
const char	buf[] ;
int		buflen ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PROG_MAGIC) return SR_NOTOPEN ;

	rs = uc_writen(op->fd,buf,buflen) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (prog_write) */


int prog_send(op,buf,buflen,flags)
PROG		*op ;
const char	buf[] ;
int		buflen ;
int		flags ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PROG_MAGIC) return SR_NOTOPEN ;

	rs = u_send(op->fd,buf,buflen,flags) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (prog_send) */


int prog_sendto(op,buf,buflen,flags,sap,salen)
PROG		*op ;
const char	buf[] ;
int		buflen ;
int		flags ;
void		*sap ;
int		salen ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PROG_MAGIC) return SR_NOTOPEN ;

	rs = u_sendto(op->fd,buf,buflen,flags,sap,salen) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (prog_sendto) */


int prog_sendmsg(op,msgp,flags)
PROG		*op ;
struct msghdr	*msgp ;
int		flags ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PROG_MAGIC) return SR_NOTOPEN ;

	rs = u_sendmsg(op->fd,msgp,flags) ;

	if (rs > 0)
	    op->tlen += rs ;

	return rs ;
}
/* end subroutine (prog_sendmsg) */


int prog_shutdown(PROG *op,int cmd)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PROG_MAGIC) return SR_NOTOPEN ;

	rs = u_shutdown(op->fd,cmd) ;

	return rs ;
}
/* end subroutine (prog_shutdown) */


/* close the connection */
int prog_close(PROG *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cstat ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PROG_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("prog_close: ent f_log=%u\n", op->f.log) ;
#endif

	if (op->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	}

	if (op->pid > 0) {
	    for (i = 0 ; i < 10 ; i += 1) {
	        rs1 = u_waitpid(op->pid,&cstat,W_OPTIONS) ;
	        if (rs1 >= 0) break ;
	        msleep(10) ;
	    } /* end for */
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->efname[0] != '\0') {
	    u_unlink(op->efname) ;
	    op->efname[0] = '\0' ;
	}

	if (op->f.log) {
	    logfile_printf(&op->lh,"bytes=%u",op->tlen) ;
	    rs1 = logfile_close(&op->lh) ;
	    if (rs >= 0) rs = rs1 ;
	    op->f.log = FALSE ;
	} /* end if */

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (prog_close) */


/* private subroutines */


static int subinfo_start(sip,op,ap,hostname,svcname)
SUBINFO		*sip ;
PROG		*op ;
SYSDIALER_ARGS	*ap ;
const char	hostname[] ;
const char	svcname[] ;
{
	int		rs ;

	memset(sip,0,sizeof(SUBINFO)) ;

	sip->envv = (const char **) environ ;
	sip->op = op ;
	sip->ap = ap ;
	sip->pr = ap->pr ;
	sip->prn = ap->prn ;
	sip->hostname = hostname ;
	sip->svcname = svcname ;

	if ((rs = getnprocessors(sip->envv,0)) >= 0) {
	    sip->ncpu = rs ;
	    if ((rs = vecstr_start(&sip->stores,3,0)) >= 0) {
		if ((rs = vecstr_start(&sip->aenvs,3,0)) >= 0) {
		    if (ap != NULL) {
	    		rs = subinfo_procargs(sip) ;
		    }
		    if (rs < 0)
			vecstr_finish(&sip->aenvs) ;
		} /* end if (aenvs) */
		if (rs < 0)
		    vecstr_finish(&sip->stores) ;
	    } /* end if (stores) */
	} /* end if (getnprocessors) */

#if	CF_DEBUGS
	debugprintf("prog/subinfo_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->f.ids) {
	    sip->f.ids = FALSE ;
	    rs1 = ids_release(&sip->id) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->a.node != NULL) {
	    rs1 = uc_free(sip->a.node) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->a.node = NULL ;
	}

	if (sip->a.svc != NULL) {
	    rs1 = uc_free(sip->a.svc) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->a.svc = NULL ;
	}

	if (sip->a.pr != NULL) {
	    rs1 = uc_free(sip->a.pr) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->a.pr = NULL ;
	}

	rs1 = vecstr_finish(&sip->aenvs) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecstr_finish(&sip->stores) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_procargs(SUBINFO *sip)
{
	KEYOPT		akopts ;
	SYSDIALER_ARGS	*ap = sip->ap ;
	int		rs ;
	int		argc ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_doubledash = FALSE ;
	cchar		*argval = NULL ;
	const char	**argv ;
	const char	*argp, *aop, *akp, *avp ;
	char		argpresent[NARGPRESENT] ;

#if	CF_DEBUGS
	debugprintf("prog/subinfo_procargs: arguments\n") ;
#endif

	argv = ap->argv ;

	for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;

#if	CF_DEBUGS
	{
	    debugprintf("prog/subinfo_procargs: argc=%u\n",argc) ;
	    for (ai = 0 ; argv[ai] != NULL ; ai += 1)
	        debugprintf("prog/subinfo_procargs: argv[%u]=%s\n",
		ai,argv[ai]) ;
	}
#endif /* CF_DEBUGS */

	sip->argc = argc ;
	sip->argv = argv ;
	rs = keyopt_start(&akopts) ;
	if (rs < 0)
	    goto ret0 ;

/* process program arguments */

	for (ai = 0 ; ai < NARGPRESENT ; ai += 1)
	    argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = (argc - 1) ;
	while ((rs >= 0) && (sip->program == NULL) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp+1) ;

	        } else if ((argl == 2) && (ach == '-')) {

	            f_doubledash = TRUE ;
	            ai += 1 ;
	            argr -= 1 ;

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
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->pr = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->pr = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* program-root-name */
	                case argopt_rn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->prn = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->prn = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* search-name root */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->searchname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->searchname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* logfile */
	                case argopt_lf:
			    sip->f.log = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->logfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->logfname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* path-vars file */
	                case argopt_pvf:
	                case argopt_pf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->pvfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->pvfname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_df:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->dfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->dfname = avp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_xf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->xfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->xfname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
			    break ;

	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sip->efname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sip->efname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
			    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

	                    case 'i':
	                        sip->f.ignore = TRUE ;
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

/* eXported environment */
	                    case 'x':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = vecstr_envadds(&sip->aenvs,
	                                argp,argl) ;
				}
				} else
	                            rs = SR_INVALID ;
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

	        sip->program = (const char *) argp ;

	    } /* end if (key letter-word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	sip->argi = (argc > 0) ? (ai + 1) : 0 ;

#if	CF_DEBUGS
	debugprintf("prog/subinfo_procargs: program=%s\n",sip->program) ;
	debugprintf("prog/subinfo_procargs: argi=%u\n",sip->argi) ;
#endif

	if (rs >= 0) {
	    rs = subinfo_procopts(sip,&akopts) ;
	}

ret1:
	keyopt_finish(&akopts) ;

ret0:

#if	CF_DEBUGS
	debugprintf("prog/subinfo_procargs: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_procargs) */


static int subinfo_procopts(SUBINFO *sip,KEYOPT *kop)
{
	KEYOPT_CUR	kcur ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	    int		oi ;
	    int		kl, vl ;
	    cchar	*kp, *vp ;
    
	    while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	    if ((oi = matostr(procopts,2,kp,kl)) >= 0) {

	        vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	        switch (oi) {
	        case procopt_log:
	                sip->f.log = TRUE ;
	                if ((vl > 0) && ((rs = optbool(vp,vl)) >= 0)) {
	                    sip->f.log = (rs > 0) ;
			}
	            break ;
	        } /* end switch */

	        c += 1 ;
	    } /* end if (valid option) */

	    if (rs < 0) break ;
	} /* end while (looping through key options) */

	    keyopt_curend(kop,&kcur) ;
	} /* end if (keyopt-cur) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_procopts) */


static int subinfo_setentry(SUBINFO *sip,cchar **epp,cchar v[],int vlen)
{
	int		rs ;
	int		oi, i ;
	int		vnlen = 0 ;
	const char	*cp ;

	if (sip == NULL) return SR_FAULT ;

	if (epp == NULL) return SR_INVALID ;

/* find existing entry for later deletion */

	oi = -1 ;
	if (*epp != NULL) {
	    for (i = 0 ; vecstr_get(&sip->stores,i,&cp) >= 0 ; i += 1) {
	        if (cp != NULL) {
	            if (*epp == cp) {
	                oi = i ;
	                break ;
		    }
	        }
	    } /* end for */
	} /* end if (had an existing entry) */

/* add the new entry */

	if (v != NULL) {
	    vnlen = strnlen(v,vlen) ;
	    if ((rs = vecstr_add(&sip->stores,v,vnlen)) >= 0) {
	        i = rs ;
	        if ((rs = vecstr_get(&sip->stores,i,&cp)) >= 0) {
	            *epp = cp ;
		}
	    } /* end if (added new entry) */
	} /* end if (had a new entry) */

/* delete the old entry if we had one */

	if ((rs >= 0) && (oi >= 0)) {
	    vecstr_del(&sip->stores,oi) ;
	}

	return (rs >= 0) ? vnlen : rs ;
}
/* end subroutine (subinfo_setentry) */


static int subinfo_defaults(SUBINFO *sip)
{
	SYSDIALER_ARGS	*ap = sip->ap ;
	int		rs = SR_OK ;
	int		rs1 ;
	const char	*vp ;

/* program-root */

	if ((sip->pr == NULL) && (sip->prn != NULL) && (sip->prn[0] != '\0')) {
	    char	domainname[MAXHOSTNAMELEN + 1] ;
	    char	pr[MAXPATHLEN + 1] ;

	    rs1 = getnodedomain(NULL,domainname) ;

	    if (rs1 >= 0)
	        rs1 = mkpr(pr,MAXPATHLEN,sip->prn,domainname) ;

	    if (rs1 >= 0) {

	        rs = subinfo_dirok(sip,pr,rs1) ;

	        if (rs > 0)
	            rs = uc_mallocstrw(pr,rs1,&sip->a.pr) ;

	        if (rs > 0)
	            sip->pr = sip->a.pr ;

	    } /* end if */

	} /* end if */

	if ((rs >= 0) && (sip->pr == NULL)) {

	    if ((vp = getenv(PROG_VARPR)) != NULL) {
	        rs = subinfo_dirok(sip,vp,-1) ;
	        if (rs > 0)
	            sip->pr = vp ;
	    }

	} /* end if */

	if ((rs >= 0) && (sip->pr == NULL)) {
	    vp = PROG_PR ;
	    rs = subinfo_dirok(sip,vp,-1) ;
	    if (rs > 0)
	        sip->pr = vp ;
	} /* end if */

	if (sip->pr == NULL)
	    sip->pr = ap->pr ;

/* program root-name */

	if ((rs >= 0) && (sip->prn == NULL) && (sip->pr != NULL)) {
		cchar	*cp, **epp ;
		int	cl ;
		if ((cl = sfbasename(sip->pr,-1,&cp)) > 0) {
			epp = &sip->prn ;
			rs = subinfo_setentry(sip,epp,cp,cl) ;
		}
	} /* end if */

/* search-name */

	if (sip->searchname == NULL)
	    sip->searchname = PROG_SEARCHNAME ;

/* log-file */

	if ((rs >= 0) && (sip->logfname == NULL)) {
		sip->logfname = PROG_LOGFNAME ;
	}

#if	CF_DEBUGS
	debugprintf("prog/subinfo_defaults: pr=%s\n",sip->pr) ;
	debugprintf("prog/subinfo_defaults: logfname=%s\n",sip->logfname) ;
#endif

/* out of here */

	return rs ;
}
/* end subroutine (subinfo_defaults) */


static int subinfo_userinfo(SUBINFO *sip)
{
	USERINFO	*uip = &sip->u ;
	int		rs = SR_OK ;

	if (sip->f.userinfo)
	    goto ret0 ;

	sip->f.userinfo = TRUE ;
	rs = userinfo(uip,sip->userinfobuf,USERINFO_LEN,NULL) ;
	if (rs < 0)
	    goto bad0 ;

	sip->umachine = uip->machine ;
	sip->usysname = uip->sysname ;
	sip->urelease = uip->release ;
	sip->uversion = uip->version ;
	sip->nodename = uip->nodename ;
	sip->username = uip->username ;
	sip->homedname = uip->homedname ;
	sip->shell = uip->shell ;
	sip->organization = uip->organization ;
	sip->gecosname = uip->gecosname ;
	sip->realname = uip->realname ;
	sip->name = uip->name ;
	sip->domainname = uip->domainname ;
	sip->tz = uip->tz ;
	sip->logid = uip->logid ;
	sip->uid = uip->uid ;
	sip->gid = uip->gid ;

ret0:
bad0:
	return rs ;
}
/* end subroutine (subinfo_userinfo) */


static int subinfo_findprog(SUBINFO *sip,char progfname[])
{
	vecstr		*elp ;
	int		rs = SR_OK ;
	const char	*pnp = sip->program ;

#if	CF_DEBUGS
	debugprintf("prog/subinfo_findprog: pn=>%s<\n",pnp) ;
#endif

	elp = &sip->exports ;
	progfname[0] = '\0' ;
	if (pnp == NULL) {

	    rs = procdefprog(sip,&pnp) ;

#if	CF_DEBUGS
	    debugprintf("prog/subinfo_findprog: procdefprog() rs=%d\n",rs) ;
#endif

	} /* end if */

#if	CF_DEBUGS
	if (rs >= 0)
	    debugprintf("prog/subinfo_findprog: pnp=%s\n",pnp) ;
#endif

	if ((rs >= 0) && ((pnp == NULL) || (pnp[0] == '\0')))
	    rs = SR_NOENT ;

	if (rs >= 0) {
	if (strchr(pnp,'/') == NULL) {
	    rs = subinfo_search(sip,elp,progfname,pnp) ;
	} else
	    rs = mkpath1(progfname,pnp) ;
	} /* end if (ok) */

ret0:

#if	CF_DEBUGS
	debugprintf("prog/subinfo_findprog: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_findprog) */


static int subinfo_search(SUBINFO *sip,VECSTR *elp,char progfname[],cchar pn[])
{
	VECSTR		pathlist ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("prog/subinfo_search: pn=>%s<\n",pn) ;
#endif

	if ((rs = vecstr_start(&pathlist,10,0)) >= 0) {

	    if ((rs = loadpathlist(sip,&pathlist,elp)) >= 0) {

#if	CF_DEBUGS
	{
	    int	i ;
	    char	*cp ;
	    for (i = 0 ; vecstr_get(&pathlist,i,&cp) >= 0 ; i += 1)
	        debugprintf("prog/subinfo_search: pc=%s\n",cp) ;
	}
#endif /* CF_DEBUGS */

	rs = getprogpath(&sip->id,&pathlist,progfname,pn,-1) ;

#if	CF_DEBUGS
	debugprintf("prog/subinfo_search: getprogpath() rs=%d\n",rs) ;
#endif

	if (rs == SR_NOENT) {
	    if ((rs = xfile(&sip->id,pn)) >= 0) {
	        rs = mkpath1(progfname,pn) ;
	    }
	}

	    } /* end if */

	    vecstr_finish(&pathlist) ;
	} /* end if */

	return rs ;
}
/* end subroutine (subinfo_search) */


static int subinfo_envdialer(SUBINFO *sip)
{
	vecstr		*elp = &sip->exports ;
	int		rs = SR_OK ;
	int		i ;
	int		size ;
	const char	*vp ;
	char		*sabuf = NULL ;

	for (i = 0 ; envdialers[i] != NULL ; i += 1) {
	    vp = NULL ;
	    switch (i) {
	    case envdialer_root:
		vp = sip->pr ;
		break ;
	    case envdialer_host:
		vp = sip->hostname ;
		break ;
	    case envdialer_svc:
		vp = sip->svcname ;
		break ;
	    case envdialer_svcargs:
		size = subinfo_sasize(sip) ;
#if	CF_DEBUGS
		debugprintf("subinfo_envdialer: size=%d\n",size) ;
#endif
		if ((rs = uc_malloc(size,&sabuf)) >= 0) {
		    rs = subinfo_sabuild(sip,sabuf) ;
		}
		vp = sabuf ;
		break ;
	    } /* end switch */

	    if ((rs >= 0) && (vp != NULL)) {

	        rs = vecstr_envadd(elp,envdialers[i],vp,-1) ;

#if	CF_DEBUGS
		debugprintf("subinfo_envdialer: vecstr_envadd() rs=%d\n",rs) ;
#endif

	        if (rs < 0) break ;
	    } /* end if */

	} /* end for */

	if (sabuf != NULL) {
	    uc_free(sabuf) ;
	}

	return rs ;
}
/* end subroutine (subinfo_envdialer) */


static int subinfo_sasize(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		i ;
	int		argr ;
	int		size = 1 ;
	const char	**argv ;

	argr = (sip->argc - sip->argi) ;
	argv = (const char **) (sip->argv + sip->argi) ;
	for (i = 0 ; (i < argr) && (argv[i] != NULL) ; i += 1) {
	    if (i >= 1) {
		size += (strlen(argv[i]) + 1) ;
	    }
	} /* end for */

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (subinfo_sasize) */


static int subinfo_sabuild(SUBINFO *sip,char sabuf[])
{
	int		rs = SR_OK ;
	int		i ;
	int		argr ;
	const char	**argv ;
	char		*bp = sabuf ;

	argr = (sip->argc - sip->argi) ;
	argv = (const char **) (sip->argv + sip->argi) ;
	for (i = 0 ; (i < argr) && (argv[i] != NULL) ; i += 1) {
	    if (i >= 1) {
		bp = strwcpy(bp,argv[i],-1) ;
		*bp++ = ' ' ;
	    }
	} /* end for */

	*bp = '\0' ;
	return (rs >= 0) ? (bp - sabuf) : rs ;
}
/* end subroutine (subinfo_sabuild) */


static int subinfo_exec(SUBINFO *sip,cchar *progfname,cchar **dav)
{
	VECHAND		avs ;
	BUFFER		b ;
	PROG		*op = sip->op ;
	vecstr		*elp ;

	int		rs = SR_OK ;
	int		argr ;
	int		i ;
	int		opts ;
	int		azl ;
	int		oflags = O_RDWR ;
	int		start = 10 ;
	int		f_m = FALSE ;
	int		f_sa = FALSE ;

	const char	**av ;
	const char	**ev ;
	const char	**argv ;
	const char	*azp = NULL ;
	const char	*abuf = NULL ;

	if (op->f.log) {
	    logfile_printf(&op->lh,"prog=%s",progfname) ;
	}

	argr = (sip->argc - sip->argi) ;
	argv = (sip->argv + sip->argi) ;

#if	CF_DEBUGS
	debugprintf("prog/subinfo_subexec: progfname=%s\n",progfname) ;
	debugprintf("prog/subinfo_subexec: argc=%d \n",sip->argc) ;
	for (i = 0 ; (i < sip->argc) && (sip->argv[i] != NULL) ; i += 1) {
	    debugprintf("prog/subinfo_subexec: argv%u=%s\n",
		i,sip->argv[i]) ;
	}
	debugprintf("prog/subinfo_subexec: argr=%d \n",argr) ;
	if ((argv != NULL) && (argv[0] != NULL))
	    debugprintf("prog/subinfo_subexec: argz=%s\n",argv[0]) ;
#endif /* CF_DEBUGS */

	elp = &sip->exports ;
	rs = vecstr_envadd(elp,"_",progfname,-1) ;
#if	CF_DEBUGS
	debugprintf("prog/subinfo_subexec: vecstr_envadd() rs=%d\n",rs) ;
#endif /* CF_DEBUGS */
	if (rs < 0)
	    goto ret0 ;

	opts = VECHAND_OCOMPACT ;
	rs = vechand_start(&avs,10,opts) ;
#if	CF_DEBUGS
	debugprintf("prog/subinfo_subexec: vechand_start() rs=%d\n",rs) ;
#endif /* CF_DEBUGS */
	if (rs < 0)
	    goto ret0 ;

	f_sa = ((argv == NULL) || (argv[0] == NULL)) ;
	f_sa = f_sa || ((argr >= 0) && isplusminus(argv[0])) ;
#ifdef	COMMENT
	f_sa = f_sa || (sip->progmode == 1) ;
#endif

	azp = argv[0] ;
	azl = -1 ;
	if (f_sa) {

#ifdef	COMMENT
	    si = (sip->progmode == 1) ? 0:1 ;
#endif

	    azl = sfbasename(progfname,-1,&azp) ;
	    start = (azl+2) ;
	}

	rs = buffer_start(&b,start) ;
#if	CF_DEBUGS
	debugprintf("prog/subinfo_subexec: buffer_start() rs=%d\n",rs) ;
#endif /* CF_DEBUGS */
	if (rs < 0)
	    goto ret1 ;

#if	CF_DEBUGS
	debugprintf("prog/subinfo_subexec: azl=%d azp=%s\n",azl,azp) ;
#endif

/* setup the zeroth argument */

	f_m = ((argv == NULL) || (argv[0] == NULL)) && sip->f.progdash ;
	f_m = f_m || 
	    ((argv != NULL) && (argv[0] != NULL) && isminus(argv[0])) ;
	if (f_m) {
	    rs = buffer_char(&b,'-') ;
	}

	if (rs >= 0)
	    rs = buffer_strw(&b,azp,azl) ;

	if (rs >= 0) {

	    rs = buffer_get(&b,&abuf) ;
#if	CF_DEBUGS
	debugprintf("prog/subinfo_subexec: buffer_get() rs=%d\n",rs) ;
#endif /* CF_DEBUGS */

	    if (rs >= 0)
	    rs = vechand_add(&avs,abuf) ;

#if	CF_DEBUGS
	debugprintf("prog/subinfo_subexec: abuf=%s\n",abuf) ;
	debugprintf("prog/subinfo_subexec: vechand_add() rs=%d\n",rs) ;
#endif

	} /* end if */

/* setup all remaining arguments */

	if (argv != NULL) {
	    for (i = 1 ; (rs >= 0) && (argr > 0) && (argv[i] != NULL) ; 
	        i += 1) {

	        argr -= 1 ;
	        rs = vechand_add(&avs,argv[i]) ;

	    } /* end for */
	} /* end if */

	vechand_getvec(&avs,&av) ;

	if (rs >= 0)
	    rs = vecstr_envadd(elp,"_EF",progfname,-1) ;

	if (rs >= 0)
	    rs = vecstr_envadd(elp,"_A0",av[0],-1) ;

	if (rs < 0)
	    goto ret2 ;

#if	CF_DEBUGS
	{
	    int	i ;
	    for (i = 0 ; av[i] != NULL ; i += 1)
	        debugprintf("prog/subinfo_subexec: av[%u]=>%s<\n",i,av[i]) ;
	}
#endif /* CF_DEBUGS */

	vecstr_getvec(elp,(const char ***) &ev) ;

	rs = uc_openprog(progfname,oflags,av,ev) ;
	op->fd = rs ;
#if	CF_DEBUGS
	debugprintf("prog/subinfo_subexec: uc_openprog() rs=%d\n",rs) ;
#endif /* CF_DEBUGS */

ret2:
	buffer_finish(&b) ;

ret1:
	vechand_finish(&avs) ;

ret0:
	return rs ;
}
/* end subroutine (subinfo_exec) */


static int subinfo_logfile(SUBINFO *sip)
{
	PROG		*op = sip->op ;
	int		rs = SR_OK ;
	int		rs1 ;
	const char	*lnp ;
	const char	*lidp = NULL ;
	char		logfname[MAXPATHLEN + 1] ;

	if (! sip->f.log)
	    goto ret0 ;

	sip->f.log = TRUE ;
	lnp = sip->logfname ;
	if (lnp[0] != '/') {
	    rs = mkpath3(logfname,sip->pr,PROG_LOGDNAME,lnp) ;
	    lnp = logfname ;
	}

#if	CF_DEBUGS
	debugprintf("prog/subinfo_logfile: lnp=%s\n",lnp) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	    rs1 = logfile_open(&op->lh,lnp,0,0666,lidp) ;
	    op->f.log = (rs1 >= 0) ;

#if	CF_DEBUGS
	debugprintf("prog/subinfo_logfile: logfile_open() rs=%d\n",rs1) ;
#endif

	    if (rs1 >= 0) {
		USERINFO	*uip = &sip->u ;

		logfile_userinfo(&op->lh,uip,0L,
	                PROG_MNAME,PROG_VERSION) ;

		logfile_printf(&op->lh,"pid=%d",uip->pid) ;

	        logfile_printf(&op->lh,"pr=%s",
	            sip->pr) ;

	        logfile_printf(&op->lh,"host=%s",
	            sip->hostname) ;

	        logfile_printf(&op->lh,"svc=%s",
	            sip->svcname) ;

	    } /* end if (opened logfile) */

ret0:
	return rs ;
}
/* end subroutine (subinfo_logfile) */


static int subinfo_dirok(SUBINFO *sip,cchar d[],int dlen)
{
	struct ustat	sb ;
	NULSTR		ss ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;
	const char	*dnp ;

	if (! sip->f.ids) {
	    sip->f.ids = TRUE ;
	    rs = ids_load(&sip->id) ;
	}

	if (rs >= 0) {
	    if ((rs = nulstr_start(&ss,d,dlen,&dnp)) >= 0) {

	        if ((rs1 = u_stat(dnp,&sb)) >= 0) {
	            if (S_ISDIR(sb.st_mode)) {
	                rs1 = sperm(&sip->id,&sb,(R_OK | X_OK)) ;
	                f = (rs1 >= 0) ;
	            } /* end if */
	        } /* end if (stat) */

		nulstr_finish(&ss) ;
	    } /* end if (nulstr) */
	} /* end if (ok) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_dirok) */


static int loadgroupname(SUBINFO *sip)
{
	int		rs ;
	int		gnlen = 0 ;
	char		gnbuf[GROUPNAMELEN + 1] ;

	if ((rs = getgroupname(gnbuf,GROUPNAMELEN,sip->gid)) >= 0) {
	    cchar	**vpp = &sip->groupname ;
	    gnlen = rs ;
	    rs = subinfo_setentry(sip,vpp,gnbuf,gnlen) ;
	}

	return (rs >= 0) ? gnlen : rs ;
}
/* end subroutine (loadgroupname) */


static int loadarchitecture(SUBINFO *sip)
{
	int		rs = SR_NOSYS ;
	int		cl = -1 ;
	cchar		*cp = getenv(VARARCHITECTURE) ;

#if	CF_DEBUGS
	debugprintf("prog/loadarchitecture: cp=%s\n",cp) ;
#endif

#ifdef	SI_ARCHITECTURE
	if (cp == NULL) {
	    char	archbuf[ARCHBUFLEN + 1] ;
	    rs = u_sysinfo(SI_ARCHITECTURE,archbuf,ARCHBUFLEN) ;
	    if (rs >= 0) {
	        cp = archbuf ;
	        cl = rs ;
	    }
	}
#endif /* SI_ARCHITECTURE */

#if	CF_DEBUGS
	debugprintf("prog/loadarchitecture: cp=%s\n",cp) ;
#endif

	if (cp != NULL) {
	    rs = subinfo_setentry(sip,&sip->architecture,cp,cl) ;
	}

#if	CF_DEBUGS
	debugprintf("prog/loadarchitecture: ret=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (loadarchitecture) */


static int loadhz(SUBINFO *sip)
{
	int		rs ;
	if ((rs = gethz(0)) >= 0) {
	    const int	dlen = DIGBUFLEN ;
	    char	dbuf[DIGBUFLEN+1] ;
	    if ((rs = ctdeci(dbuf,dlen,rs)) >= 0) {
		cchar	**vpp = &sip->hz ;
	        rs = subinfo_setentry(sip,vpp,dbuf,rs) ;
	    }
	}
	return rs ;
}
/* end subroutine (loadhz) */


static int loadcooks(SUBINFO *sip)
{
	EXPCOOK		*ckp = &sip->cooks ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		vl = 0 ;
	const char	*vp ;

	for (i = 0 ; cooks[i] != NULL ; i += 1) {
	    vp = NULL ;
	    vl = -1 ;
	    switch (i) {
	    case cook_machine:
	        vp = sip->umachine ;
	        break ;
	    case cook_architecture:
	        vp = sip->architecture ;
	        break ;
	    case cook_ncpu:
	        {
	            char	digbuf[DIGBUFLEN + 1] ;
	            if (sip->ncpu >= 0) {
	                rs1 = ctdeci(digbuf,DIGBUFLEN,sip->ncpu) ;
	            } else {
	                strcpy(digbuf,"1") ;
	                rs1 = 1 ;
	            }
	            rs = expcook_add(ckp,cooks[i],digbuf,rs1) ;
	        } /* end block */
	        break ;
	    case cook_sysname:
	        vp = sip->usysname ;
	        break ;
	    case cook_release:
	        vp = sip->urelease ;
	        break ;
	    case cook_version:
	        vp = sip->uversion ;
	        break ;
	    case cook_hz:
	        vp = sip->hz ;
	        break ;
	    case cook_u:
	        vp = sip->username ;
	        break ;
	    case cook_g:
	        vp = sip->groupname ;
	        break ;
	    case cook_home:
	        vp = sip->homedname ;
	        break ;
	    case cook_shell:
	        vp = sip->shell ;
	        break ;
	    case cook_organization:
	        vp = sip->organization ;
	        break ;
	    case cook_gecosname:
	        vp = sip->gecosname ;
	        break ;
	    case cook_realname:
	        vp = sip->realname ;
	        break ;
	    case cook_name:
	        vp = sip->name ;
	        break ;
	    case cook_tz:
	        vp = sip->tz ;
	        break ;
	    case cook_n:
	        vp = sip->nodename ;
	        break ;
	    case cook_d:
	        vp = sip->domainname ;
	        break ;
	    case cook_h:
	        {
		    const char	*nn = sip->nodename ;
		    const char	*dn = sip->domainname ;
	            char	hnbuf[MAXHOSTNAMELEN + 1] ;
	            rs1 = snsds(hnbuf,MAXHOSTNAMELEN,nn,dn) ;
	            if (rs1 >= 0)
	                rs = expcook_add(ckp,cooks[i],hnbuf,rs1) ;
	        } /* end block */
	        break ;
	    case cook_r:
	        vp = sip->pr ;
	        break ;

	    case cook_rn:
	        vp = sip->prn ;
	        break ;
	    } /* end switch */
	    if ((rs >= 0) && (vp != NULL)) {
	        rs = expcook_add(ckp,cooks[i],vp,vl) ;
	    }
	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (loadcooks) */


static int sched_begin(SUBINFO *sip)
{
	int		rs ;

	rs = vecstr_start(&sip->svars,6,0) ;
	if (rs < 0)
	    goto ret0 ;

	if (rs >= 0)
	    rs = vecstr_envset(&sip->svars,"p",sip->pr,-1) ;

	if (rs >= 0)
	    rs = vecstr_envset(&sip->svars,"h",sip->homedname,-1) ;

	if (rs >= 0)
	    rs = vecstr_envset(&sip->svars,"e","etc",-1) ;

	if (rs >= 0)
	    rs = vecstr_envset(&sip->svars,"n",sip->searchname,-1) ;

	if (rs < 0) {
	    vecstr_finish(&sip->svars) ;
	} else
	    sip->f.svars = TRUE ;

ret0:
	return rs ;
}
/* end subroutine (sched_begin) */


static int sched_end(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->f.svars) {
	    sip->f.svars = FALSE ;
	    rs1 = vecstr_finish(&sip->svars) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (sched_end) */


static int loadparams(SUBINFO *sip)
{
	PARAMFILE	pf ;
	PARAMFILE_CUR	cur ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		vl ;
	int		i ;
	const char	**ev = (const char **) sip->envv ;
	const char	*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		vbuf[VBUFLEN + 1] ;

	if ((rs >= 0) && (sip->defprog == NULL)) {
	    if ((cp = getenv(VARDEFPROG)) != NULL) {
	        if (cp[0] != '\0') {
	            cchar	**vpp = &sip->defprog ;
	            rs = subinfo_setentry(sip,vpp,cp,-1) ;
		}
	    }
	} /* end if (environment) */

	if (rs < 0)
	    goto ret0 ;

/* parameter file */

	if (sip->paramfname == NULL)
	    rs1 = SR_NOTFOUND ;

	if ((rs1 >= 0) && (sip->paramfname[0] == '\0'))
	    rs1 = SR_INVALID ;

	if (rs1 < 0)
	    goto ret0 ;

	rs1 = permsched(schedhconf,&sip->svars,
	    tmpfname,MAXPATHLEN, sip->paramfname,R_OK) ;

	if (rs1 == SR_NOTFOUND) {
	    rs1 = permsched(schedpconf,&sip->svars,
	        tmpfname,MAXPATHLEN, sip->paramfname,R_OK) ;

	}

#if	CF_DEBUGS
	debugprintf("prog/loadparams: permsched() rs=%d\n",rs1) ;
	debugprintf("prog/loadparams: tmpfname=%s\n",tmpfname) ;
#endif

	if ((rs1 >= 0) && (paramfile_open(&pf,ev,tmpfname) >= 0)) {
	    const int	vlen = VBUFLEN ;

	    for (i = 0 ; cparams[i] != NULL ; i += 1) {

	        if ((paramfile_curbegin(&pf,&cur)) >= 0) {

	        while (rs >= 0) {
	            vl = paramfile_fetch(&pf,cparams[i],&cur,vbuf,vlen) ;
	            if (vl == SR_NOTFOUND) break ;
		    rs = vl ;

		    if (rs >= 0) {
	            switch (i) {

	            case cparam_defprog:
	                if ((vl > 0) && (sip->defprog == NULL)) {
	                    mkpath1w(tmpfname,vbuf,vl) ;
	                    rs1 = xfile(&sip->id,tmpfname) ;
	                    if (rs1 >= 0) {
				const char	**vpp = &sip->defprog ;
	                        rs = subinfo_setentry(sip,vpp,vbuf,vl) ;
	                    }
	                } /* end if */
	                break ;

	            } /* end switch */
		    } /* end if (ok) */

	        } /* end while */

	        paramfile_curend(&pf,&cur) ;
		} /* end if (cursor) */

	        if (rs < 0) break ;
	    } /* end for */

	    paramfile_close(&pf) ;
	} /* end if (parameter file) */

	if ((rs >= 0) && (sip->defprog == NULL)) {
	    cp = PROG_DEFPROG ;
	    rs = subinfo_setentry(sip,&sip->defprog,cp,-1) ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("prog/loadparams: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (loadparams) */


static int loaddefsfile(SUBINFO *sip,cchar *dfname)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	const char	**envv = (const char **) sip->envv ;

	if ((rs1 = u_stat(dfname,&sb)) >= 0) {
	    rs1 = SR_NOENT ;
	    if (S_ISREG(sb.st_mode))
	        rs1 = SR_OK ;
	}

	if (rs1 >= 0)
	    rs1 = sperm(&sip->id,&sb,R_OK) ;

	if (rs1 >= 0)
	    rs = defproc(&sip->defs,envv,&sip->cooks,dfname) ;

	return rs ;
}
/* end subroutine (loaddefsfile) */


static int loaddefs(SUBINFO *sip,cchar **sched)
{
	int		rs = SR_OK ;
	int		rs1 ;
	const char	**envv = (const char **) sip->envv ;
	char		tmpfname[MAXPATHLEN + 1] ;

	rs1 = permsched(sched,&sip->svars,
	    tmpfname,MAXPATHLEN, DEFSFNAME,R_OK) ;

	if (rs1 >= 0) {

#if	CF_DEBUGS
	    debugprintf("prog/loaddefs: dfname=%s\n",tmpfname) ;
#endif

	    rs = defproc(&sip->defs,envv,&sip->cooks,tmpfname) ;

#if	CF_DEBUGS
	    debugprintf("prog/loaddefs: defproc() rs=%d\n",rs) ;
#endif

	}

	return rs ;
}
/* end subroutine (loaddefs) */


static int loadxfile(SUBINFO *sip,cchar *xfname)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f ;
	const char	**envv = (const char **) sip->envv ;

#if	CF_DEBUGS
	debugprintf("envset/loadxfile: fname=%s\n",xfname) ;
#endif

	rs1 = u_stat(xfname,&sb) ;

	if ((rs1 >= 0) && (! S_ISREG(sb.st_mode)))
	    rs1 = SR_NOENT ;

	if (rs1 >= 0)
	    rs1 = sperm(&sip->id,&sb,R_OK) ;

#if	CF_DEBUGS
	debugprintf("envset/loadxfile: sperm() rs=%d\n",rs1) ;
#endif

	f = (rs1 >= 0) ;
	if (rs1 >= 0) {
	    EXPCOOK	*ckp = &sip->cooks ;
	    rs = envs_procxe(&sip->xenvs,ckp,envv,&sip->defs,xfname) ;
	}

#if	CF_DEBUGS
	debugprintf("envset/loadxfile: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (loadxfile) */


static int loadxsched(sip,sched)
SUBINFO		*sip ;
const char	*sched[] ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f ;
	const char	**envv = (const char **) sip->envv ;
	char		tmpfname[MAXPATHLEN + 1] ;

	rs1 = permsched(sched,&sip->svars,tmpfname,MAXPATHLEN,XEFNAME,R_OK) ;
	f = (rs1 >= 0) ;

	if (rs1 >= 0) {
	    EXPCOOK	*ckp = &sip->cooks ;

#if	CF_DEBUGS
	    debugprintf("prog/loadxsched: xfname=%s\n",tmpfname) ;
#endif

	    rs = envs_procxe(&sip->xenvs,ckp,envv,&sip->defs,tmpfname) ;

#if	CF_DEBUGS
	    debugprintf("prog/loadxsched: envs_procxe() rs=%d\n",rs) ;
#endif

	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (loadxsched) */


static int pvars_begin(sip,pathvars,fname)
SUBINFO		*sip ;
const char	**pathvars ;
const char	fname[] ;
{
	vecstr		*pvp ;
	int		rs ;

	pvp = &sip->pvars ;
	rs = vecstr_start(pvp,10,0) ;

	if (rs >= 0)
	    rs = loadpvars(sip,schedpfile,fname) ;

	if (rs >= 0)
	    rs = loadpvars(sip,schedhfile,fname) ;

	if (rs >= 0)
	    rs = loadpvarsdef(sip,pathvars) ;

	return rs ;
}
/* end subroutine (pvars_begin) */


static int pvars_end(sip)
SUBINFO		*sip ;
{
	vecstr		*pvp ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip == NULL)
	    return SR_FAULT ;

	pvp = &sip->pvars ;
	rs1 = vecstr_finish(pvp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (pvars_end) */


static int procenvextra(sip)
SUBINFO		*sip ;
{
	NULSTR		ns ;
	vecstr		*elp = &sip->exports ;
	int		rs = SR_OK ;
	int		i ;
	int		kl, vl ;
	const char	*tp, *kp, *vp ;
	const char	*kname ;

	for (i = 0 ; sip->envv[i] != NULL ; i += 1) {

	    kp = sip->envv[i] ;
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

	            nulstr_finish(&ns) ;
	        } /* end if (nulstr) */

	    } /* end if */

	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (procenvextra) */


static int procenvdef(sip)
SUBINFO		*sip ;
{
	vecstr		*elp = &sip->exports ;
	int		rs = SR_OK ;
	int		i ;
	int		n = 0 ;
	const char	*ename ;
	const char	*tp ;

	for (i = 0 ; envsys[i] != NULL ; i += 1) {

	    ename = envsys[i] ;
	    if (vecstr_search(elp,ename,vstrkeycmp,NULL) == SR_NOTFOUND) {
	        tp = NULL ;
	        switch ((int) ename[0]) {
	        case 'S':
	            tp = sip->usysname ;
	            break ;
	        case 'R':
	            tp = sip->urelease ;
	            break ;
	        case 'V':
	            tp = sip->uversion ;
	            break ;
	        case 'M':
	            tp = sip->umachine ;
	            break ;
	        case 'N':
	            if (ename[1] == 'C') {
	                char	digbuf[DIGBUFLEN + 1] ;
	                rs = ctdeci(digbuf,DIGBUFLEN,sip->ncpu) ;
	                tp = digbuf ;
	            } else
	                tp = sip->nodename ;
	            break ;
	        case 'A':
	            tp = sip->architecture ;
	            break ;
	        case 'H':
	            tp = sip->hz ;
	            break ;
	        case 'U':
	            tp = sip->username ;
	            break ;
	        case 'G':
	            tp = sip->groupname ;
	            break ;
	        case 'D':
	            tp = sip->domainname ;
	            break ;
	        } /* end switch */
	        if (tp != NULL) {
	            n += 1 ;
	            rs = vecstr_envadd(elp,ename,tp,-1) ;
	        }
	    } /* end if (environment variable was not already present) */

	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procenvdef) */


static int procenvsys(sip,sysvardb)
SUBINFO		*sip ;
const char	sysvardb[] ;
{
	SYSVAR		sv ;
	SYSVAR_CUR	cur ;
	vecstr		*elp = &sip->exports ;
	int		rs ;

	if ((rs = sysvar_open(&sv,sip->pr,sysvardb)) >= 0) {
	    const int	vlen = VBUFLEN ;
	    int		vl ;
	    char	kbuf[KBUFLEN + 1] ;
	    char	vbuf[VBUFLEN + 1] ;
	    if ((sysvar_curbegin(&sv,&cur)) >= 0) {

	        while (rs >= 0) {
	            vl = sysvar_enum(&sv,&cur,kbuf,KBUFLEN,vbuf,vlen) ;
	            if (vl == SR_NOTFOUND) break ;
 		    rs = vl ;

		    if (rs >= 0) {
	    		rs = vecstr_envadd(elp,kbuf,vbuf,vl) ;
		    }

	        } /* end while */

	        sysvar_curend(&sv,&cur) ;
	    } /* end if (cursor) */
	    sysvar_close(&sv) ;
	} /* end if (sysvar) */

#if	CF_DEBUGS
	debugprintf("prog/procenvsys: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procenvsys) */


int procdefprog(sip,rpp)
SUBINFO		*sip ;
const char	**rpp ;
{
	int		rs = SR_OK ;

	if (rpp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("prog/procdefprog: defprog=%s\n",sip->defprog) ;
#endif

	*rpp = NULL ;
	if (sip->defprog == NULL) {

	    rs = loadparams(sip) ;

#if	CF_DEBUGS
	    debugprintf("prog/procdefprog: loadparams() rs=%d\n",rs) ;
#endif

	} /* end if */

	if ((rs >= 0) && (sip->defprog != NULL)) {
	    *rpp = sip->defprog ;
	}

	return rs ;
}
/* end subroutine (procdefprog) */


static int loadpvars(sip,sched,fname)
SUBINFO		*sip ;
const char	*sched[] ;
const char	fname[] ;
{
	VECSTR		*pvp ;
	int		rs = SR_OK ;
	int		rs1 ;
	char		tmpfname[MAXPATHLEN + 1] ;

	pvp = &sip->pvars ;
	rs1 = permsched(sched,&sip->svars,
	    tmpfname,MAXPATHLEN, fname,R_OK) ;

#if	CF_DEBUGS
	debugprintf("prog/loadpvars: permsched() rs=%d\n",rs) ;
#endif

	if (rs1 >= 0) {

#if	CF_DEBUGS
	    debugprintf("prog/loadpvars: pvarfname=%s\n",tmpfname) ;
#endif

	    rs = vecstr_loadfile(pvp,1,tmpfname) ;

#if	CF_DEBUGS
	    debugprintf("prog/loadpvars: vecstr_loadfile() rs=%d\n",rs) ;
#endif

	} /* end if */

	return (rs >= 0) ? (rs1 >= 0) : rs ;
}
/* end subroutine (loadpvars) */


static int loadpvarsdef(sip,pnames)
SUBINFO		*sip ;
const char	*pnames[] ;
{
	vecstr		*pvp = &sip->pvars ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i = 0 ;

	if ((rs1 = vecstr_count(pvp)) == 0) {

	    for (i = 0 ; pnames[i] != NULL ; i += 1) {
	        rs = vecstr_adduniq(pvp,pnames[i],-1) ;
	        if (rs < 0) break ;
	    } /* end for */

	} /* end if (loading default names) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (loadpvarsdef) */


static int loadpathlist(sip,plp,elp)
SUBINFO		*sip ;
VECSTR		*plp ;
VECSTR		*elp ;
{
	int		rs = SR_OK ;
	const char	*varpath = VARPATH ;
	const char	*pp ;

	if ((rs = vecstr_search(elp,varpath,vstrkeycmp,&pp)) >= 0) {
	    cchar	*tp ;

	    rs = SR_NOENT ;
	    if ((tp = strchr(pp,'=')) != NULL) {
	        rs = loadpathcomp(sip,plp,(tp + 1)) ;
	    }

	} /* end if */

	return rs ;
}
/* end subroutine (loadpathlist) */


static int loadpathcomp(sip,lp,pp)
SUBINFO		*sip ;
vecstr		*lp ;
const char	*pp ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c, cl ;
	const char	*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (sip == NULL)
	    return SR_FAULT ;

	c = 0 ;
	while ((cp = strpbrk(pp,":;")) != NULL) {

	    cl = pathclean(tmpfname,pp,(cp - pp)) ;

	    rs1 = vecstr_findn(lp,tmpfname,cl) ;
	    if (rs1 == SR_NOTFOUND) {
	        c += 1 ;
	        rs = vecstr_add(lp,tmpfname,cl) ;
	    }

	    pp = (cp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (pp[0] != '\0')) {

	    cl = pathclean(tmpfname,pp,-1) ;

	    rs1 = vecstr_findn(lp,tmpfname,cl) ;
	    if (rs1 == SR_NOTFOUND) {
	        c += 1 ;
	        rs = vecstr_add(lp,tmpfname,cl) ;
	    }

	} /* end if (trailing one) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpathcomp) */


#ifdef	COMMENT

static int createsearchpath(lp,pr)
VECSTR		*lp ;
const char	pr[] ;
{
	int		rs ;
	int		f_bin = FALSE ;
	int		f_usrbin = FALSE ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		*ubp = "/usr/bin" ;

	mkpath2(tmpfname,pr,PATHFNAME) ;

	if ((rs = procxpath(lp,tmpfname)) >= 0) {
	    char	*pp ;
	    if ((pp = getenv(VARPATH)) != NULL) {
	        rs = loadpath(lp,pp) ;
	    }
	}

	f_bin = (vecstr_find(lp,"/bin") >= 0) ;

	f_usrbin = (vecstr_find(lp,ubp) >= 0) ;

	if (! (f_bin || f_usrbin))
	    rs = vecstr_add(lp,ubp,-1) ;

	return rs ;
}
/* end subroutine (createsearchpath) */


static int loadpath(vecstr *lp,cchar *pp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pl ;
	int		c = 0 ;
	const char	*tp ;
	char		tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("prog/loadpath: ent\n") ;
#endif

	while ((tp = strchr(pp,':')) != NULL) {

	    pl = pathclean(tmpfname,pp,(tp - pp)) ;

#if	CF_DEBUGS
	    debugprintf("prog/loadpath: pathname=%t\n",tmpfname,pl) ;
#endif

	    rs1 = vecstr_findn(lp,tmpfname,pl) ;

	    if (rs1 == SR_NOTFOUND) {
	        rs = vecstr_add(lp,tmpfname,pl) ;
	        c += 1 ;
	    }

	    pp = (tp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (pp[0] != '\0')) {

	    pl = pathclean(tmpfname,pp,-1) ;

#if	CF_DEBUGS
	    debugprintf("prog/loadpath: pathname=%t\n",tmpfname,pl) ;
#endif

	    rs1 = vecstr_findn(lp,tmpfname,pl) ;

	    if (rs1 == SR_NOTFOUND) {
	        rs = vecstr_add(lp,tmpfname,pl) ;
	        if (rs >= 0) c += 1 ;
	    }

	} /* end if (trailing one) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpath) */


static int setdefpath(vecstr *elp,cchar *pvp)
{
	int		rs = SR_OK ;
	cchar		*vp ;

	if ((vp = getenv(pvp)) == NULL) {
	    vp = PROG_DEFPATH ;
	} /* end if (making default) */

	rs = vecstr_envadd(elp,pvp,vp,-1) ;

	return rs ;
}
/* end subroutine (setdefpath) */

#endif /* COMMENT */


static int xfile(IDS *idp,cchar *fname)
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = u_stat(fname,&sb)) >= 0) {
	    rs = SR_NOTFOUND ;
	    if (S_ISREG(sb.st_mode)) {
	        rs = sperm(idp,&sb,X_OK) ;
	    }
	}

	return rs ;
}
/* end subroutine (xfile) */


static int isplusminus(cchar *s)
{
	int	f ;
	f = (s[0] == '+') || (s[0] == '-') ;
	f = f && (s[1] == '\0') ;
	return f ;
}
/* end subroutine (isplusminus) */


static int isminus(cchar *s)
{
	int	f ;
	f = (s[0] == '-') ;
	f = f && (s[1] == '\0') ;
	return f ;
}
/* end subroutine (isminus) */


