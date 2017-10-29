/* main */

/* fairly generic (PCS) front-end */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable print-outs */
#define	CF_DEBUGMALL	1		/* switchable print-outs */
#define	CF_DEBUGB	0		/* special debugging */
#define	CF_DEBUGRECIPS	0		/* debug recipients */
#define	CF_PRINTOUT	0		/* print the message out */
#define	CF_PCSPOLL	0		/* call 'pcspoll(3pcs)' */
#define	CF_PCSTRUSTUSER	1		/* call 'pcstrustuser(3pcs)' */
#define	CF_ISSAMEHOST	1		/* use 'issamehostname(3dam)' */
#define	CF_WHITEMATCH	1		/* use 'vecstr_whitematch()' */
#define	CF_RMTAB	1		/* use RMTAB file */
#define	CF_LOGFILE	1		/* perform logging */
#define	CF_DEFLOGSIZE	0		/* use a default log length */
#define	CF_PROGMSGS	1		/* call 'progmsgs()' */
#define	CF_LOCSETENT	0		/* |locinfo_setentry()| */


/* revision history:

	= 1998-05-01, David A­D­ Morano

	This code module was completely rewritten to replace the
	previous mail-delivery program for PCS, written around 1990 or
	so and which also served for a time to filter environment for
	the SENDMAIL daemon.

	= 2004-02-17, David A­D­ Morano

	This was modified to add the MSGID object.  That is a database
	that stores message IDs.  We used it to eliminate duplicate
	mail deliveries which as of late are coming from several
	popular sources!


*/

/* Copyright © 1998,2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the front-end subroutine (main) for the BB program.  It is
	similar to most other PCS programs but may be a little different since
	it originated differently from the others.

	NOTE: The big thing to note in this subroutine, and any other code that
	"pretends" to check for the existence of the mail spool directory, is
	that the directory may also be an AUTOMOUNT mount point.  This means
	that it might not be accessible when we go to check it alone.  Instead,
	we have to check something inside the directory to make sure that the
	AUTOMOUNTer first mounts the directory.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<vecitem.h>
#include	<vecobj.h>
#include	<field.h>
#include	<kvsfile.h>
#include	<dater.h>
#include	<estrings.h>
#include	<ids.h>
#include	<estrings.h>
#include	<logsys.h>
#include	<pcsconf.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"msgid.h"
#include	"whitelist.h"
#include	"recip.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	BUFLEN		MAX((2 * MAXHOSTNAMELEN),MSGBUFLEN)
#define	HOSTBUFLEN	(10 * MAXHOSTNAMELEN)

#ifndef	VARMAIL
#define	VARMAIL		"MAIL"
#endif

#define	NUCMEMALLOC	"ucmemalloc.deb"


/* external subroutines */

extern int	snscs(char *,int,const char *,const char *) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	headkeymat(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	getfname(const char *,const char *,int,char *) ;
extern int	getserial(const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	mklogid(char *,int,const char *,int,int) ;
extern int	mkgecosname(char *,int,const char *) ;
extern int	mkrealame(char *,int,const char *,int) ;
extern int	mkuibang(char *,int,USERINFO *) ;
extern int	mkuiname(char *,int,USERINFO *) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			const char *,const char *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	pcstrustuser(const char *,const char *) ;
extern int	pcsuserfile(const char *,const char *,const char *,
			const char *,const char *) ;
extern int	getportnum(const char *,const char *) ;
extern int	getmailgid(const char *,gid_t) ;
extern int	initnow(struct timeb *,const char *,int) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	proglogfname(struct proginfo *,char *,
			const char *,const char *) ;
extern int	prognamecache_begin(struct proginfo *,USERINFO *) ;
extern int	prognamecache_end(struct proginfo *) ;

extern int	progmsgs(struct proginfo *,bfile *,bfile *,vecobj *,vecobj *) ;
extern int	deliver(struct proginfo *,int,RECIP *) ;
extern int	boxer(struct proginfo *,int,RECIP *) ;
extern int	parsenodespec(struct proginfo *,int,char *,int,
			char *,const char *) ;
extern int	expander(struct proginfo *,const char *,int,char *,int) ;
extern int	mkrealname(char *,int,const char *,int) ;

extern int	vecobj_recipadd(vecobj *,const char *,int) ;
extern int	vecobj_recipfins(vecobj *) ;

extern int	prognotify(struct proginfo *,vecobj *,vecobj *) ;
extern int	progcomsat(struct proginfo *,vecobj *,vecobj *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(const char *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strdcpy3(char *,int,const char *,const char *,const char *) ;
extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* local typedefs */


/* forward references */

static int	usage(struct proginfo *) ;

static int	logrecip(struct proginfo *,const char *,int,struct passwd *) ;
static int	mkrecipname(char *,int,const char *) ;

static int	loadrecips(struct proginfo *,VECOBJ *,const char *,int) ;
static int	loadrecip(struct proginfo *,VECOBJ *,const char *,int) ;

static int	procopts_setenv(struct proginfo *,KEYOPT *) ;
static int	procopts_setpcs(struct proginfo *,KEYOPT *,vecstr *) ;
static int	procopts(struct proginfo *,KEYOPT *) ;
static int	procargs(struct proginfo *,struct arginfo *,BITS *,
			VECOBJ *,const char *,const char *) ;

static int	procspamsetup(struct proginfo *,vecobj *) ;
static int	procspambox(struct proginfo *,struct locinfo *,
			vecstr *,vecobj *,vecobj *,int) ;
static int	procrecips(struct proginfo *,struct locinfo *,
			vecstr *,vecobj *,vecobj *,int) ;
static int	procunavail(struct proginfo *,int) ;
static int	procmboxes(struct proginfo *,const char *,int) ;

static int	locinfo_start(struct locinfo *,struct proginfo *) ;
static int	locinfo_finish(struct locinfo *) ;
static int	locinfo_mboxadd(struct locinfo *,const char *,int) ;
static int	locinfo_mboxcount(struct locinfo *) ;
static int	locinfo_mboxget(struct locinfo *,int,const char **) ;
int		locinfo_rncurbegin(LOCINFO *,LOCINFO_RNCUR *) ;
int		locinfo_rncurend(LOCINFO *,LOCINFO_RNCUR *) ;
int		locinfo_rnlook(LOCINFO *,LOCINFO_RNCUR *,const char *,int) ;
int		locinfo_rnread(LOCINFO *,LOCINFO_RNCUR *,char *,int) ;
int		locinfo_gmcurbegin(LOCINFO *,LOCINFO_GMCUR *) ;
int		locinfo_gmcurend(LOCINFO *,LOCINFO_GMCUR *) ;
int		locinfo_gmlook(LOCINFO *,LOCINFO_GMCUR *,const char *,int) ;
int		locinfo_gmread(LOCINFO *,LOCINFO_GMCUR *,char *,int) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif /* CF_LOCSETENT */

static int isNotPrematch(int) ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGRECIPS
static int debugrecips(struct proginfo *,VECOBJ *) ;
#endif


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"pm",
	"sn",
	"rf",
	"af",
	"ef",
	"of",
	"if",
	"lf",
	"ms",
	"md",
	"mr",
	"nm",
	"oi",
	"cp",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_rf,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_lf,
	argopt_ms,
	argopt_md,
	argopt_mr,
	argopt_nm,
	argopt_oi,
	argopt_cp,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRPCS
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_ACCESS },
	{ SR_REMOTE, EX_FORWARDED },
	{ SR_NOSPC, EX_NOSPACE },
	{ SR_INVALID, EX_USAGE },
	{ 0, 0 }
} ;

static const char	*progmodes[] = {
	"dmail",
	"dmailbox",
	NULL
} ;

enum progmodes {
	progmode_dmail,
	progmode_dmailbox,
	progmode_overlast
} ;

static const char *pcsopts[] = {
	"cluster",
	"pcsadmin",
	"maildir",
	"logsize",
	"loglen",
	"pcspoll",
	"syslog",
	"mailhist",
	NULL
} ;

enum pcsopts {
	pcsopt_cluster,
	pcsopt_pcsadmin,
	pcsopt_maildir,
	pcsopt_logsize,
	pcsopt_loglen,
	pcsopt_pcspoll,
	pcsopt_logsys,
	pcsopt_mailhist,
	pcsopt_overlast
} ;

static const char *locopts[] = {
	"deadmaildir",
	"comsat",
	"spam",
	"logconf",
	"logmsg",
	"logzone",
	"logenv",
	"logmsgid",
	"divert",
	"forward",
	"nospam",
	"norepeat",
	"nopollmsg",
	"mbtab",
	"boxdir",
	"boxname",
	"timeout",
	"tomsgread",
	"spambox",
	"mailhist",
	"deliver",
	"finish",
	NULL
} ;

enum locopts {
	locopt_deadmaildir,
	locopt_comsat,
	locopt_spam,
	locopt_logconf,
	locopt_logmsg,
	locopt_logzone,
	locopt_logenv,
	locopt_logmsgid,
	locopt_divert,
	locopt_forward,
	locopt_nospam,
	locopt_norepeat,
	locopt_nopollmsg,
	locopt_mbtab,
	locopt_boxdir,
	locopt_boxname,
	locopt_timeout,
	locopt_tomsgread,
	locopt_spambox,
	locopt_mailhist,
	locopt_deliver,
	locopt_finish,
	locopt_overlast
} ;

static const char	*mailentries[] = {
	":saved",
	"root",
	"adm",
	"uucp",
	"staff",
	"pcs",
	NULL
} ;

/* 'conf' for most regular programs */
static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	"%p/%f",
	NULL
} ;

/* whitelist file search (for system file) */
static const char	*sched2[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%e/%m/%n.%f",
	"%p/%e/%m/%f",
	"%p/%e/%m.%f",
	"%p/%e/%f",
	NULL
} ;

/* whitelist file search (for local-user file) */
static const char	*sched3[] = {
	"%h/%e/%n/%n.%f",
	"%h/%e/%n/%f",
	"%h/%e/%n.%f",
	"%h/%e/%m/%n.%f",
	"%h/%e/%m/%f",
	"%h/%e/%m.%f",
	"%h/%e/%f",
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


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;
	struct locinfo	li, *lip = &li ;
	struct arginfo	ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	VECSTR		svars ;
	USERINFO 	u ;

	bfile		errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		tmpfile, *tfp = &tmpfile ;
	vecobj		recips ;
	vecobj		info ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs ;
	int		rs1 = SR_OK ;
	int		size ;
	int		v ;
	int		sl, cl, fl, bl ;
	int		mbtab_type = -1 ;
	int		fd  ;
	int		tfd = -1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_version = FALSE ;
	int		f_schedvar = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*logcname = LOGCNAME ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*pmspec = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*lfname = NULL ;
	const char	*envfromaddr = NULL ;
	const char	*uu_machine = NULL ;
	const char	*uu_user = NULL ;
	const char	*protospec = NULL ;
	const char	*portspec = NULL ;
	const char	*sp, *cp ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	mbfname[MAXPATHLEN + 1] ;
	char	buf[BUFLEN+ 1] ;

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

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

	tmpfname[0] = '\0' ;
	mbfname[0] = '\0' ;

/* get the current time-of-day */

	{
	    rs = initnow(&pip->now,pip->zname,DATER_ZNAMESIZE) ;

#if	CF_DEBUG
	    {
	        char	timebuf[TIMEBUFLEN+1] ;
	        debugprintf("main: now.time=%u now.time=%s\n",
	            pip->now.time,timestr_logz(pip->now.time,timebuf)) ;
	    }
#endif

	    pip->daytime = pip->now.time ;
	    if (rs >= 0)
	        rs = dater_start(&pip->tmpdate,&pip->now,pip->zname,-1) ;

	} /* end block (getting some current time stuff) */
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto baddatestart ;
	}

	timestr_logz(pip->daytime,pip->stamp) ;

	pip->verboselevel = 1 ;
	pip->logsize = -1 ;
	pip->to_spool = -1 ;
	pip->to_msgread = -1 ;

	pip->f.optlogmsg = OPT_LOGMSG ;
	pip->f.optlogzone = OPT_LOGZONE ;
	pip->f.optlogenv = OPT_LOGENV ;
	pip->f.optlogmsgid = OPT_LOGMSGID ;
	pip->f.optlogsys = OPT_LOGSYS ;
	pip->f.optdivert = OPT_DIVERT ;
	pip->f.optforward = OPT_FORWARD ;
	pip->f.optnorepeat = OPT_NOREPEAT ;
	pip->f.optnospam = OPT_NOSPAM ;
	pip->f.optmailhist = OPT_MAILHIST ;
	pip->f.optdeliver = OPT_DELIBER ;

/* local information */

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

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badstart ;
	}

	ai = 0 ;
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

	        if (isdigit(ach)) {

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

/* do we have a keyword match or should we assume only key letters? */

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

	                case argopt_pm:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl > 0)
	                            pmspec = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pmspec = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl > 0)
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
	                case argopt_rf:
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

/* log file name */
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

/* mail spool directory */
	                case argopt_ms:
	                case argopt_md:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->maildname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->maildname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    break ;

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

	                case argopt_mr:
	                    pip->f.multirecip = TRUE ;
	                    break ;

	                case argopt_nm:
	                    pip->f.nopollmsg = TRUE ;
	                    break ;

/* ignore dots on input (default anyway!) */
	                case argopt_oi:
	                    pip->f.optin = TRUE ;
	                    break ;

/* COMAST port */
	                case argopt_cp:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl > 0)
	                            portspec = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            portspec = argp ;
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

	                    case 'V':
	                        f_version = TRUE ;
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

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'R':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
				        pr = argp ;
				    }
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* ignore the "deliver" flag for compatibility */
	                    case 'd':
	                        break ;

/* from email address */
	                    case 'f':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            envfromaddr = argp ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* ignore dots on input (default anyway!) */
	                    case 'i':
	                        pip->f.optin = TRUE ;
	                        break ;

/* boxname for DMAILBOX */
	                    case 'm':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = procmboxes(pip,argp,argl) ;
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

/* caller-supplied protocol specification */
	                    case 'p':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            protospec = argp ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* quiet */
	                    case 'q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* subject */
	                    case 's':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->msgsubject = argp ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* timeout */
	                    case 't':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdecti(argp,argl,&v) ;
	                            pip->to_spool = v ;
	                        }
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* verbose (level) */
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

	        } /* end if (digit or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter-word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n", pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,pip->version) ;
	}

/* get some program information */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

/* continue */

	if (pmspec == NULL) pmspec = pip->progname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;
	if (pip->progmode < 0) pip->progmode = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0) {
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    } else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif /* CF_DEBUG */

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: progmode=%s(%u)\n",
	        pip->progname,progmodes[pip->progmode],pip->progmode) ;
	}

/* help file */

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_usage || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* other initialization */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (lfname == NULL) lfname = getenv(VARLFNAME) ;
	if (lfname == NULL) lfname = getenv(VARLOGFNAME) ;

	if (portspec == NULL) portspec = getenv(VARCOMSATPORT) ;

	if (pip->comsatfname == NULL)
	    pip->comsatfname = getenv(VARCOMSATFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* arguments */

/* some UUCP stuff */

	uu_machine = getenv("UU_MACHINE") ;

	uu_user = getenv("UU_USER") ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: uu_machine=%s uu_user=%s\n",
	        uu_machine,uu_user) ;
#endif

/* get the group information that we need */

	pip->gid_mail = getmailgid(MAILGNAME,MAILGID) ;

/* get user profile information */

	rs = ids_load(&pip->id) ;

	if (rs >= 0)
	    rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	if (rs < 0) {
	    ex = EX_NOUSER ;
	    bprintf(pip->efp,"%s: userinfo failure (%d)\n",
	        pip->progname,rs) ;
	    goto baduser ;
	}

	pip->uip = &u ;
	pip->pid = u.pid ;
	pip->username = u.username ;
	pip->homedname = u.homedname ;
	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->name = u.name ;
	pip->fullname = u.fullname ;
	pip->mailname = u.mailname ;

	pip->uid = u.uid ;
	pip->euid = u.euid ;
	pip->f.setuid = (pip->uid != pip->euid) ;

	pip->gid = u.gid ;
	pip->egid = u.egid ;
	pip->f.setgid = (pip->gid != pip->egid) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: rs=%d user=%s homedir=%s euid=%d egid=%d\n",
	        rs,u.username,u.homedname,u.euid,u.egid) ;
#endif

	if (pip->boxname == NULL) pip->boxname = getenv(VARMAILBOX) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: boxname=%s\n", pip->boxname) ;
#endif

	if (rs >= 0)
	    rs = procopts_setenv(pip,&akopts) ;

/* get the system PCS configuration information */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pcsconf()\n") ;
#endif

	if (rs >= 0)
	    rs = procopts(pip,&akopts) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badinit1 ;
	}

#if	CF_PCSTRUSTUSER
	rs = pcstrustuser(pip->pr,pip->username) ;
	pip->f.trusted = (rs > 0) ;
	if (rs < 0) goto badinit1 ;
#endif

/* establish what cluster we are on */

	if (pip->cluster == NULL) {
	    const char	*tp ;

	    if ((tp = strchr(pip->domainname,'.')) != NULL) {
	        cl = (tp - pip->domainname) ;
	        cp = pip->domainname ;

	        rs = proginfo_setentry(pip,&pip->cluster,cp,cl) ;

	    }

	} /* end if (cluster) */
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badinit1 ;
	}

/* boxdname and boxname */

	if (pip->progmode == progmode_dmailbox) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: 1 boxdname=%s\n",pip->boxdname) ;
	    debugprintf("main: 1 boxname=%s\n",pip->boxname) ;
	}
#endif

	if (pip->boxdname == NULL) pip->boxdname = DEFBOXDNAME ;

	if (pip->boxname == NULL) pip->boxname = DEFBOXNAME ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: 2 boxdname=%s\n",pip->boxdname) ;
	    debugprintf("main: 2 boxname=%s\n",pip->boxname) ;
	}
#endif

	} /* end if (promode-dmailbox) */

/* validate the timeout */

	if (pip->to_spool <= 1)
	    pip->to_spool = TO_LOCK ;

	if (pip->to_msgread <= 1)
	    pip->to_msgread = TO_MSGREAD ;

/* should we check the logfile length */

#if	CF_DEFLOGSIZE
	if (pip->logsize < 0)
	    pip->logsize = LOGSIZE ;
#endif

/* the protocol specification */

	if ((protospec == NULL) && (uu_machine != NULL)) {
	    protospec = buf ;
	    snscs(buf,BUFLEN,"uucp",uu_machine) ;
	}

	if (protospec != NULL)
	    proginfo_setentry(pip,&pip->protospec,protospec,-1) ;

	if ((protospec != NULL) &&
	    (strcasecmp(pip->protospec,PROTOSPEC_POSTFIX) == 0))
	    pip->f.trusted = TRUE ;

/* log ID */

	sp = SERIALFNAME ;
	if (sp[0] != '/') {
	    if (strchr(sp,'/') != NULL) {
	        rs1 = mkpath2(tmpfname,pip->pr,sp) ;
	    } else
	        rs1 = mkpath3(tmpfname,pip->pr,VARDNAME,sp) ;
	    sp = tmpfname ;
	}
	if (rs1 >= 0)
	    rs1 = getserial(sp) ;
	if (rs1 >= 0) {
	    sp = buf ;
	    rs = mklogid(buf,LOGIDLEN,pip->nodename,5,rs1) ;
	    sl = rs ;
	} else
	    sp = u.logid ;

	if (rs >= 0)
	    rs = proginfo_setentry(pip,&pip->logid,sp,-1) ;

/* log file */

	if ((rs >= 0) && ((pip->f.optlogmsg || pip->f.optlogconf))) {

	    rs = proglogfname(pip,tmpfname,logcname,lfname) ;
	    if (rs > 0) lfname = tmpfname ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: lfname=%s\n",lfname) ;
#endif

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: lf=%s\n",pip->progname,lfname) ;

	    if ((rs >= 0) && (lfname != NULL)) {
	        pip->f.logprog = TRUE ;

	        rs1 = SR_NOENT ;
	        f = (lfname[0] != '\0') && (lfname[0] != '-') ;
	        if ((rs >= 0) && f)
	            rs = proginfo_setentry(pip,&pip->lfname,lfname,-1) ;

	        if ((rs >= 0) && f)
	            rs1 = logfile_open(&pip->lh,lfname,0,0666,pip->logid) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: LOG logfile_open() rs=%d\n",rs1) ;
#endif

	        if ((rs >= 0) && (rs1 >= 0)) {
	            pip->open.logprog = TRUE ;
	            pip->f.logconf = pip->f.optlogconf ;
	            pip->f.logmsg = pip->f.optlogmsg ;
	            if (pip->logsize > 0)
	                logfile_checksize(&pip->lh,pip->logsize) ;
	            rs1 = logfile_userinfo(&pip->lh,&u,pip->daytime,
	                pip->progname,pip->version) ;
	            if (pip->f.optlogconf)
	                logfile_printf(&pip->lh,"pr=%s\n", pip->pr) ;
	            if (protospec != NULL)
	                logfile_printf(&pip->lh,"proto=%s\n",protospec) ;
	            if (uu_machine != NULL)
	                logfile_printf(&pip->lh,"uu_machine=%s\n",uu_machine) ;
	            if (uu_user != NULL)
	                logfile_printf(&pip->lh,"uu_user=%s\n",uu_user) ;
	        } /* end if */

	    } /* end if */

	} /* end if */
	if (rs < 0) goto retearly ;

/* user-list file */

#if	CF_DEBUGN
	nprintf(NUCMEMALLOC,"main: user-list file\n") ;
#endif

	if (rs >= 0) {
	    if ((rs1 = mkuiname(buf,BUFLEN,&u)) >= 0) {
	        const char	*nn = pip->nodename ;
	        const char	*un = pip->username ;
	        char	ucname[MAXNAMELEN+1] ;

	        rs1 = snsds(ucname,MAXNAMELEN,pip->searchname,USERFSUF) ;

	        if (rs1 >= 0)
	            rs1 = mkpath3(tmpfname,pip->pr,logcname,ucname) ;

	        if (rs1 >= 0)
	            rs1 = pcsuserfile(pip->pr,tmpfname,nn,un,buf) ;

	        if (pip->f.logmsg) {
	            if (rs1 == 1) {
	                logfile_printf(&pip->lh,
	                    "created user-list file") ;
	            } else if (rs1 < 0)
	                logfile_printf(&pip->lh,
	                    "inaccessible user-list file (%d)", rs1) ;
	        }
	    } /* end if */
	} /* end if */

/* try to open an environment summary log also */

#if	CF_DEBUGN
	nprintf(NUCMEMALLOC,"main: env-sum log file\n") ;
#endif

	if ((rs >= 0) && pip->f.optlogenv) {
	    const char	*env = LOGENVFNAME ;
	    char	cname[MAXNAMELEN+1] ;
	    char	envfname[MAXPATHLEN+1] ;

	    rs = snsds(cname,MAXNAMELEN,pip->searchname,env) ;
	    if (rs >= 0)
	        rs1 = mkpath3(envfname,pip->pr,logcname,cname) ;

	    if ((rs >= 0) && (rs1 >= 0) && (envfname[0] != '/')) {
	        rs1 = getfname(pip->pr,envfname,1,tmpfname) ;
	        fl = rs1 ;
	        if (fl > 0) mkpath1(envfname,tmpfname) ;
	    }

	    if ((rs >= 0) && (rs1 >= 0)) {
	        rs1 = logfile_open(&pip->envsum,envfname,0,0666,pip->logid) ;
	        pip->open.logenv = (rs1 >= 0) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: ENV logfile_open() rs=%d\n",rs1) ;
#endif

	} /* end if (option to log envelope information) */

/* find the time-zone name log file */

#if	CF_DEBUGN
	nprintf(NUCMEMALLOC,"main: log-zone file\n") ;
#endif

	if ((rs >= 0) && pip->f.optlogzone) {
	    const char	*zone = LOGZONEFNAME ;
	    char	cname[MAXNAMELEN+1] ;
	    char	zfname[MAXPATHLEN+1] ;

	    rs = snsds(cname,MAXNAMELEN,pip->searchname,zone) ;
	    if (rs >= 0)
	        rs1 = mkpath3(zfname,pip->pr,logcname,cname) ;

#ifdef	COMMENT
	    if ((rs1 >= 0) && (zonefname[0] != '/')) {
	        rs1 = getfname(pip->pr,zfname,1,tmpfname) ;
	        fl = rs1 ;
	        if (fl > 0) mkpath1(zfname,tmpfname) ;
	    }
#endif /* COMMENT */

	    if (rs1 >= 0) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: zfname=%s\n",zfname) ;
#endif
	        rs1 = logzones_open(&pip->lz,zfname,O_RDWR,0666) ;
	        pip->open.logzone = (rs1 >= 0) ;
	    }

	    if (rs1 >= 0) {
	        rs = proginfo_setentry(pip,&pip->zfname,zfname,rs1) ;
	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: zf=%s\n",
	                pip->progname,pip->zfname) ;
	    }

	} /* end if (option for logging time-zone information) */

/* other stuff */

#if	CF_DEBUGN
	nprintf(NUCMEMALLOC,"main: other stuff\n") ;
#endif

	if (rs >= 0) {

/* make the maillock address */

	    bl = sncpy3(buf,BUFLEN,pip->nodename,"!",pip->username) ;

	    if (bl > 0)
	        rs = proginfo_setentry(pip,&pip->lockaddr,buf,bl) ;

/* the envelope-from address */

	    if ((rs >= 0) && (envfromaddr == NULL)) {

	        if (pip->f.trusted && (uu_user != NULL)) {
	            envfromaddr = uu_user ;

	        } else if (bl > 0) {
	            envfromaddr = buf ;

	        } else
	            envfromaddr = pip->username ;

	    } /* end if */

	    if (rs >= 0)
	        rs = proginfo_setentry(pip,&pip->envfromaddr,envfromaddr,-1) ;

	} /* end block */

/* find the mail spool directory, as dynamically as possible */

	pip->uid_divert = pip->uid ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: maildname=%s\n",pip->maildname) ;
#endif

	if (pip->progmode == progmode_dmail) {
	    struct ustat	sb ;

	    if ((pip->maildname == NULL) || (pip->maildname[0] == '\0'))
	        pip->maildname = SPOOLDNAME ;

	    if ((rs >= 0) && (pip->maildname != NULL)) {
	        int	i ;

/* pop something inside the directory because of automounting */

	        rs1 = SR_NOENT ;
	        for (i = 0 ; mailentries[i] != NULL ; i += 1) {

	            rs1 = mkpath2(tmpfname,pip->maildname,mailentries[i]) ;
	            if (rs1 >= 0) rs1 = u_stat(tmpfname,&sb) ;
	            if (rs1 >= 0) break ;

	        } /* end for */

	        if (rs1 >= 0)
	            rs1 = perm(pip->maildname,-1,-1,NULL,W_OK) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: perm() rs=%d \n",rs) ;
#endif

	        if (rs1 < 0) procunavail(pip,rs1) ;

	    } /* end if (finding mail-dir) */

/* get and save some information on the mail spool directory */

	    if (pip->open.logprog)
	        logfile_printf(&pip->lh, 
	            "maildir=%s\n", pip->maildname) ;

	    if ((rs >= 0) && (pip->maildname != NULL)) {
	        rs = u_stat(pip->maildname,&sb) ;
	        pip->uid_maildir = sb.st_uid ;
	        pip->gid_maildir = sb.st_gid ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: maildir uid=%d gid=%d\n",
	            sb.st_uid, sb.st_gid) ;
#endif

#if	CF_DEBUGN
	    nprintf(NUCMEMALLOC,"main: getportnum()\n") ;
#endif

	    if ((rs >= 0) && (portspec != NULL)) {
	        rs = getportnum(PROTONAME_COMSAT,portspec) ;
	        if (rs == SR_NOTFOUND) rs = IPPORT_BIFFUDP ;
	        pip->port_comsat = rs ;
	    }

	} /* end block */

	if (rs < 0) {
	    ex = EX_OSERR ;
	    bprintf(pip->efp,"%s: inaccessible maildir=%s (%d)\n",
	        pip->progname,pip->maildname,rs) ;
	    goto badspooldir ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: uid_maildir=%d\n",pip->uid_maildir) ;
#endif

/* process the usernames given at invocation or in the argfile */

	size = sizeof(RECIP) ;
	rs = vecobj_start(&recips,size,10,0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: vecobj_start() rs=%d\n",rs) ;
#endif

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badinit2 ;
	}

#if	CF_DEBUGS
	rs1 = vecobj_audit(&recips) ;
	debugprintf("main: 1 vecobj_audit() rs=%d\n",rs1) ;
#endif /* CF_DEBUGS */

	if (rs >= 0) {
	    memset(&ainfo,0,sizeof(struct arginfo)) ;
	    ainfo.argc = argc ;
	    ainfo.argv = argv ;
	    ainfo.ai_max = ai_max ;
	    ainfo.ai_pos = ai_pos ;

	    rs = procargs(pip,&ainfo,&pargs,&recips,afname,ofname) ;

	} /* end if (args) */

	if (rs < 0)
	    goto badinit3 ;

	pip->c_processed = 0 ;
	pip->c_delivered = 0 ;

/* some file initialization */

	rs1 = vecstr_start(&svars,6,0) ;
	f_schedvar = (rs1 >= 0) ;

	vecstr_envset(&svars,"p",pip->pr,-1) ;

	vecstr_envset(&svars,"e","etc",-1) ;

	vecstr_envset(&svars,"n",pip->searchname,-1) ;

	vecstr_envset(&svars,"m","mail",-1) ;

/* do we have a COMSAT file? */

	if (pip->progmode == progmode_dmail) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: DMAIL COMSAT\n") ;
#endif

	if ((pip->comsatfname == NULL) || (pip->comsatfname[0] == '+')) {
	    char	csfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: setting default COMSAT filename\n") ;
#endif

	    rs1 = expander(pip,COMSATFNAME,-1,csfname,MAXPATHLEN) ;

	    if (csfname[0] != '/') {
	        cl = mkpath2(tmpfname,pip->pr,csfname) ;
	        if (cl > 0)
	            mkpath1w(csfname,tmpfname,cl) ;
	    }

	    if (rs1 >= 0)
	        rs1 = perm(csfname,-1,-1,NULL,R_OK) ;

#if	CF_RMTAB
	    if ((rs1 == SR_NOTFOUND) && (pip->comsatfname == NULL)) {
	        rs1 = SR_OK ;
	        cl = 1 ;
	        mkpath1(csfname,"+") ;
	    }
#endif /* CF_RMTAB */

	    if (rs1 >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: 1 csfname=%s\n",csfname) ;
#endif

	        rs = proginfo_setentry(pip,&pip->comsatfname,csfname,-1) ;
	    }

	} else if (pip->comsatfname[0] != '-') {
	    char	csfname[MAXPATHLEN + 1] ;

	    rs1 = expander(pip,pip->comsatfname,-1,csfname,MAXPATHLEN) ;

	    if (csfname[0] != '/') {
	        cl = mkpath2(tmpfname,pip->pr,csfname) ;
	        if (cl > 0)
	            mkpath1w(csfname,tmpfname,cl) ;
	    } /* end if */

	    if (rs1 >= 0)
	        rs1 = perm(csfname,-1,-1,NULL,R_OK) ;

	    if (rs1 >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: 2 csfname=%s\n",csfname) ;
#endif

	        rs = proginfo_setentry(pip,&pip->comsatfname,csfname,-1) ;
	    }

	} /* end if */

	pip->f.comsat = 
	    (pip->comsatfname != NULL) && (pip->comsatfname[0] != '-') ;

	} /* end if (progmode-dmail) */

/* do we have a spam filter file? */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: filter file?\n") ;
#endif

	if ((rs >= 0) && 
	    ((pip->spamfname == NULL) || (pip->spamfname[0] == '+'))) {

	    char	spamfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: setting default SPAM filename\n") ;
#endif

	    expander(pip,SPAMFNAME,-1,spamfname,MAXPATHLEN) ;

	    cl = -1 ;
	    if (spamfname[0] != '/') {

	        cl = mkpath2(tmpfname,pip->pr,spamfname) ;

	        if (cl > 0)
	            mkpath1w(spamfname,tmpfname,cl) ;

	    } /* end if */

	    rs1 = perm(spamfname,-1,-1,NULL,R_OK) ;

	    if (rs1 >= 0)
	        proginfo_setentry(pip,&pip->spamfname,spamfname,cl) ;

	} /* end if (default spam file) */

	pip->f.spam = 
	    (pip->spamfname != NULL) && (pip->spamfname[0] != '-') ;

	if (pip->progmode == progmode_dmailbox) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: DMAIBOX MBTAB\n") ;
#endif

/* find a MBTAB file if we can */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 0 mbfname=%s\n",mbfname) ;
#endif

	rs = SR_NOEXIST ;
	if (mbfname[0] == '\0') {

	    mbtab_type = 1 ;
	    rs = mkpath1(mbfname,MBFNAME) ;
	    if (rs >= 0) {
	        rs = permsched(sched1,&svars,tmpfname,MAXPATHLEN,mbfname,R_OK) ;
	        sl = rs ;
	    }

#ifdef	COMMENT
	    if (rs == SR_NOENT) {
	        rs = permsched(sched3,&svars,tmpfname,MAXPATHLEN,mbfname,R_OK) ;
	        sl = rs ;
	    }
#endif /* COMMENT */

	    if (sl > 0)
	        mkpath1(mbfname,tmpfname) ;

	} else {

	    if (mbtab_type < 0) mbtab_type = 0 ;
	    rs = getfname(pip->pr,mbfname,mbtab_type,tmpfname) ;
	    sl = rs ;
	    if (sl > 0) mkpath1w(mbfname,tmpfname,sl) ;

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: 1 mbfname=%s\n",mbfname) ;
	    debugprintf("main: rs=%d sl=%d\n",rs,sl) ;
	}
#endif

	if ((rs >= 0) || (perm(mbfname,-1,-1,NULL,R_OK) >= 0)) {

	    pip->f.mbtab = TRUE ;
	    proginfo_setentry(pip,&pip->mbfname,mbfname,-1) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: 2 mbfname=%s\n",pip->mbfname) ;
#endif

	}

	} /* end if (progmode-dmailbox) */

/* do we have a system whitelist? */

	if ((rs >= 0) && pip->f.optnospam) {
	    sl = permsched(sched2,&svars,tmpfname,MAXPATHLEN,WLFNAME,R_OK) ;
	    if (sl > 0) {
	        rs1 = whitelist_open(&lip->wl1,tmpfname) ;
	        lip->f.wl_system = (rs1 >= 0) ;
	        if (rs1 > 0)
	            pip->nwl_system += rs1 ;
	    }

	} /* end if (requesting spam recognition) */

/* do we have a system blacklist? */

	sl = permsched(sched2,&svars,
	    tmpfname,MAXPATHLEN, BLFNAME,R_OK) ;

	if (sl > 0) {
	    rs1 = whitelist_open(&lip->bl1,tmpfname) ;
	    lip->f.bl_system = (rs1 >= 0) ;
	    if (rs1 > 0)
	        pip->nbl_system += rs1 ;
	}

/* process the input message */

	if ((ifname == NULL) || (ifname[0] == '-')) ifname = BFILE_STDIN ;

	rs = bopen(ifp,ifname,"r",0666) ;
	if (rs < 0) {
	    ex = EX_NOINPUT ;
	    bprintf(pip->efp,"%s: inaccessible input file (%d)\n",
	        pip->progname,rs) ;
	    goto badinopen ;
	}

/* create a temporary file to hold the processed input */

	{
	    char	template[MAXPATHLEN+1] ;
	    rs = mkpath2(template,pip->tmpdname,"dmailXXXXXXXXX") ;
	    if (rs >= 0)
	        rs = mktmpfile(tmpfname,0600,template) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: mktmpfile() rs=%d tmpfname=%s\n",rs,tmpfname) ;
#endif

	if (rs < 0) {
	    ex = EX_TEMPFAIL ;
	    bprintf(pip->efp,"%s: could create-open tempory file (%d)\n",
	        pip->progname,rs) ;
	    goto badtmpfile ;
	}

	rs = bopen(tfp,tmpfname,"rwct",0600) ;
	u_unlink(tmpfname) ;
	if (rs < 0) {
	    ex = EX_TEMPFAIL ;
	    bprintf(pip->efp,"%s: could create-open tempory file (%d)\n",
	        pip->progname,rs) ;
	    goto badtmpopen ;
	}

/* initialze the container for message information */

	size = sizeof(struct msginfo) ;
	rs = vecobj_start(&info,size,5,0) ;
	if (rs < 0) {
	    ex = EX_TEMPFAIL ;
	    goto badinfoinit ;
	}

	if ((rs = prognamecache_begin(pip,&u)) >= 0) {

/* process the message on the input */

#if	CF_PROGMSGS
#if	CF_DEBUG && CF_DEBUGRECIPS
	    if (DEBUGLEVEL(4)) {
	        debugprintf("main: progmsgs()\n") ;
	        debugrecips(pip,&recips) ;
	    }
#endif

	    rs = progmsgs(pip,ifp,tfp,&info,&recips) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: progmsgs() rs=%d\n",rs) ;
#endif
#endif /* CF_PROGMSGS */

	    rs1 = prognamecache_end(pip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (prognamecache) */

	if (rs < 0) {

	    bclose(tfp) ;
	    tfp = NULL ;

	    switch (rs) {
	    case SR_NOMEM:
	    case SR_NOSPC:
	    case SR_NOLCK:
	    case SR_DQUOT:
	    case SR_NFILE:
	        ex = EX_TEMPFAIL ;
	        break ;
	    case SR_FBIG:
	        ex = EX_IOERR ;
	        break ;
	    default:
	        ex = EX_UNKNOWN ;
	        break ;
	    } /* end switch */

	    goto badproc ;

	} /* end if (processing error) */

/* convert the BIO file pointer to a file descriptor */

	rs = bcontrol(tfp,BC_FD,&fd) ;
	if (rs < 0) {
	    ex = EX_TEMPFAIL ;
	    goto badcontrol ;
	}

	rs = u_dup(fd) ;
	tfd = rs ;
	if (tfp != NULL) {
	    bclose(tfp) ;
	    tfp = NULL ;
	}

	if (rs < 0) {
	    ex = EX_TEMPFAIL ;
	    goto baddup ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    struct ustat	sb ;
	    rs = u_fstat(tfd,&sb) ;
	    debugprintf("main: u_fstat() rs=%d tfd=%d size=%lu\n",
	        rs,tfd,sb.st_size) ;
	}
#endif /* CF_DEBUG */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    struct msginfo	*mop ;
	    int	i ;
	    for (i = 0 ; vecobj_get(&info,i,&mop) >= 0 ; i += 1) {
	        if (mop == NULL) continue ;
	        debugprintf("main: msg=%u moff=%u mlen=%u\n",
	            i,mop->moff,mop->mlen) ;
	    } /* end for */
	}
#endif /* CF_DEBUG */

/* deliver the message to the recipients */

	if (pip->f.optlogmsgid) {

	    mkpath2(tmpfname,pip->pr,MSGIDDBNAME) ;

	    rs1 = msgid_open(&lip->mids,tmpfname,O_RDWR,0660,MAXMSGID) ;

	    if (rs1 < 0)
	        rs1 = msgid_open(&lip->mids,tmpfname,O_RDONLY,0660,MAXMSGID) ;

	    lip->f.mids = (rs1 >= 0) ;

	} /* end if */

	if (rs >= 0)
	    rs = procspamsetup(pip,&info) ;

	if (rs >= 0)
	    rs = procrecips(pip,lip,&svars,&info,&recips,tfd) ;

	if (pip->progmode == progmode_dmail) {
	    if ((rs >= 0) && (pip->spambox != NULL)) {
	        rs = procspambox(pip,lip,&svars,&info,&recips,tfd) ;
	    }
	}

	if (lip->f.mids) {
	    lip->f.mids = FALSE ;
	    msgid_close(&lip->mids) ;
	}

/* do we want to issue notices to COMSAT or something else? */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: extra services c_deliver=%u\n",
	        pip->c_delivered) ;
#endif

	bflush(pip->efp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: extra services 1\n") ;
#endif

	if (pip->open.logprog)
	    logfile_flush(&pip->lh) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: extra services 2\n") ;
#endif

	if (pip->progmode == progmode_dmail) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    const char	*pn = pip->progname ;
	    debugprintf("main: comsatfname=%s\n",pip->comsatfname) ;
	    debugprintf("main: c_delivered=%u\n",pip->c_delivered) ;
	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: comsat (%d)\n",pn,rs) ;
	}
#endif

	if ((pip->comsatfname != NULL) && (pip->comsatfname[0] != '-') &&
	    (pip->c_delivered > 0) &&
	    (uc_fork() == 0)) {

	    int	ex1 ;


	    if (tfd >= 0)
	        u_close(tfd) ;

	    rs1 = progcomsat(pip,&info,&recips) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: progcomsat() rs=%d\n",rs1) ;
#endif

	    if (pip->f.logmsg || (pip->debuglevel > 0)) {

	        cl = (rs1 >= 0) ? rs1 : 0 ;
	        if (pip->f.logmsg)
	            logfile_printf(&pip->lh,
	                "COMSAT messages sent=%d\n",cl) ;

	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: COMSAT messages sent=%d\n",
	                pip->progname,cl) ;

	    } /* end if */

	    if (pip->open.logprog) {
	        pip->open.logprog = FALSE ;
	        logfile_close(&pip->lh) ;
	    }

	    bclose(pip->efp) ;

	    ex1 = (rs1 >= 0) ? EX_OK : EX_DATAERR ;
	    uc_exit(ex1) ;

	} /* end if (COMSAT) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: dmail-handle rs=%d\n",rs) ;
	    if (pip->debuglevel > 0)
	        bprintf(pip->efp, 
	            "%s: dmail-handle (%d)\n",
	            pip->progname,rs) ;
	}
#endif

	} /* end if (progmode-dmail) */

	if (pip->progmode == progmode_dmailbox) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: prognotify() mbtab=%s f_mbtab=%u\n",
	        pip->mbfname,pip->f.mbtab) ;
#endif

	if ((! pip->f.nopollmsg) && pip->f.mbtab && 
	    (pip->mbfname != NULL) && (pip->c_delivered > 0) &&
	    (uc_fork() == 0)) {

	    int	ex1 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: mbtab=%s\n",pip->mbfname) ;
#endif

	    if (tfd >= 0)
	        u_close(tfd) ;

	    rs1 = prognotify(pip,&info,&recips) ;

	    if (rs1 > 0) {

	        if (pip->f.logmsg)
	            logfile_printf(&pip->lh,
	                "MAILPOLL messages sent=%d\n",rs1) ;

	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: MAILPOLL messages sent=%d\n",
	                pip->progname,rs1) ;

	    } /* end if */

	    if (pip->open.logprog) {
	        pip->open.logprog = FALSE ;
	        logfile_close(&pip->lh) ;
	    }

	    bclose(pip->efp) ;

	    ex1 = (rs1 >= 0) ? EX_OK : EX_DATAERR ;
	    uc_exit(ex1) ;

	} /* end if (MAILPOLL) */

	} /* end if (progmode-dmailbox) */

/* free up the message information container */

	vecobj_finish(&info) ;

/* do any optional reporting for multi-recipient mode */

#ifdef	COMMENT
	if (pip->f.multirecip) {
	    bfile	outfile, *ofp = &outfile ;

	    rs1 = SR_NOENT ;
	    if ((ofname != NULL) && (ofname[0] != '-')) {
	        rs1 = bopen(ofp,ofname,"wct",0644) ;
	    } else
	        rs1 = bopen(ofp,BFILE_STDOUT,"dwct",0644) ;

	    if (rs1 >= 0) {



	        bclose(ofp) ;

	    } /* end if (opened output */

	    rs = SR_OK ;
	    ex = EX_OK ;

	} /* end if (multi-recipient mode) */
#endif /* COMMENT */

/* print a regular error message to STDERR if we are NOT in multi-recip mode */
done:
	if ((! pip->f.multirecip) && (rs < 0)) {
	    switch (rs) {
	    case SR_NOENT:
	        ex = EX_NOUSER ;
	        if (pip->f.logmsg) {
	            logfile_printf(&pip->lh,
	                "ex=%u recipient not found (%d)\n",
	                ex,rs) ;
	        }
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: recipient not found\n",
	                pip->progname) ;
	        }
	        break ;
	    case SR_AGAIN:
	    case SR_DEADLK:
	    case SR_NOLCK:
	    case SR_TXTBSY:
	        ex = EX_TEMPFAIL ;
	        if (pip->f.logmsg) {
	            logfile_printf(&pip->lh,
	                "ex=%u could not capture mail lock (%d)\n",
	                ex,rs) ;
	        }
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: could not capture the mail lock\n",
	                pip->progname) ;
	        }
	        break ;
	    case SR_ACCES:
	        ex = EX_ACCESS ;
	        if (pip->f.logmsg) {
	            logfile_printf(&pip->lh,
	                "ex=%u could not access mail spool-file (%d)\n",
	                ex,rs) ;
	        }
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: could not access the mail spool-file (%d)\n",
	                pip->progname,rs) ;
	        }
	        break ;
	    case SR_REMOTE:
	        ex = EX_FORWARDED ;
	        if (pip->f.logmsg) {
	            logfile_printf(&pip->lh,
	                "ex=%u mail is being forwarded (%d)\n",
	                ex,rs) ;
	        }
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,"%s: mail is being forwarded\n",
	                pip->progname) ;
	        }
	        break ;
	    case SR_NOSPC:
	        ex = EX_NOSPACE ;
	        if (pip->f.logmsg) {
	            logfile_printf(&pip->lh,
	                "ex=%u file-system is out of space (%d)\n",
	                ex,rs) ;
	        }
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: local file-system is out of space\n",
	                pip->progname) ;
	        }
	        break ;
	    default:
	        ex = EX_UNKNOWN ;
	        if (pip->f.logmsg) {
	            logfile_printf(&pip->lh,
	                "ex=%u unknown error (%d)\n",
	                ex,rs) ;
	        }
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,"%s: unknown bad thing (%d)\n",
	                pip->progname,rs) ;
	        }
	        break ;
	    } /* end switch */
	} else
	    ex = EX_OK ;

	if (pip->f.logmsg)
	    logfile_printf(&pip->lh,
	        "recipients processed=%u delivered=%u\n",
	        pip->c_processed,pip->c_delivered) ;

ret9:
	if (tfd >= 0) {
	    u_close(tfd) ;
	    tfd = -1 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: TFD close rs=%d\n",rs) ;
	    if (pip->debuglevel > 0)
	        bprintf(pip->efp, 
	            "%s: TFD-close (%d)\n",
	            pip->progname,rs) ;
	}
#endif

ret8:
badproc:
baddup:
badcontrol:
	vecobj_finish(&info) ;

badtmpfile:
badtmpopen:
badinfoinit:
ret7:
	bclose(ifp) ;

/* good return from program */
retgood:
ret6:
badinopen:
ret5:
	if (lip->f.bl_system) {
	    lip->f.bl_system = FALSE ;
	    whitelist_close(&lip->bl1) ;
	}

	if (lip->f.wl_system) {
	    lip->f.wl_system = FALSE ;
	    whitelist_close(&lip->wl1) ;
	}

	if (f_schedvar) {
	    f_schedvar = FALSE ;
	    vecstr_finish(&svars) ;
	}

#if	CF_DEBUGS
	rs = vecobj_audit(&recips) ;
	debugprintf("main: 5 vecobj_audit() rs=%d\n",rs) ;
#endif

badinit3:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: vecobj_reciprems()\n") ;
#endif

	vecobj_recipfins(&recips) ;
	vecobj_finish(&recips) ;

badportnum:
badspooldir:
badinit2:
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: badinit2\n") ;
#endif

	if (pip->open.logzone) {
	    pip->open.logzone = FALSE ;
	    logzones_close(&pip->lz) ;
	}

	if (pip->open.logenv) {
	    pip->open.logenv = FALSE ;
	    logfile_close(&pip->envsum) ;
	}

	if (pip->open.logprog) {
	    pip->open.logprog = FALSE ;
	    logfile_close(&pip->lh) ;
	}

badinit1:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: badinit1\n") ;
#endif

baduser:
	ids_release(&pip->id) ;

retearly:

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: retearly pn=%s\n",pip->progname) ;
	    debugprintf("main: dl=%d efp{%p}\n",pip->debuglevel,pip->efp) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    rs1 = bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
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

	bits_finish(&pargs) ;

badpargs:
	locinfo_finish(lip) ;

badlocstart:
badstart:
	dater_finish(&pip->tmpdate) ;

baddatestart:
badinitnow:
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


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-d] [-f <fromaddr>] <recip(s)> ...]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	if (pip->progmode == progmode_dmailbox) {
	    fmt = "%s:  [-m <mailbox>]\n" ;
	    if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	    wlen += rs ;
	}

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts_setenv(pip,kop)
struct proginfo	*pip ;
KEYOPT		*kop ;
{
	int		rs = SR_OK ;
	const char	*cp ;

	if ((cp = getenv(VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	return rs ;
}
/* end subroutine (procopts_setenv) */


static int procopts_setpcs(pip,kop,setsp)
struct proginfo	*pip ;
KEYOPT		*kop ;
vecstr		*setsp ;
{
	const int	nrs = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		i ;
	int		fi, oi ;
	int		kl, vl ;
	const char	*tp, *cp ;
	const char	*kp, *vp ;

/* all PCS options *with* our component name prefix */

	if (rs >= 0) {
	    for (i = 0 ; vecstr_get(setsp,i,&cp) >= 0 ; i += 1) {
	        if (cp == NULL) continue ;

	        fi = headkeymat(pip->searchname,cp,-1) ;
	        if (fi <= 0) continue ;

	        cp += fi ;

	        kp = cp ;
	        kl = -1 ;
	        vp = NULL ;
	        vl = 0 ;
	        if ((tp = strchr(cp,'=')) != NULL) {
	            kl = (tp - cp) ;
	            vp = tp + 1 ;
	            vl = -1 ;
	        }

	        oi = matostr(locopts,3,kp,kl) ;
	        if (oi >= 0) kp = locopts[oi] ;

	        if ((oi >= 0) && (keyopt_fetch(kop,kp,NULL,NULL) == nrs)) {
	            rs = keyopt_loadvalue(kop,kp,vp,vl) ;
		}

	        if (rs < 0) break ;
	    } /* end for (local options) */
	} /* end if */

/* all PCS options *without* any component name prefix */

	if (rs >= 0) {
	    for (i = 0 ; vecstr_get(setsp,i,&cp) >= 0 ; i += 1) {
	        if (cp == NULL) continue ;

	        if (strchr(cp,':') != NULL)
	            continue ;

	        kp = cp ;
	        kl = -1 ;
	        vp = NULL ;
	        vl = 0 ;
	        if ((tp = strchr(cp,'=')) != NULL) {
	            kl = (tp - cp) ;
	            vp = tp + 1 ;
	            vl = -1 ;
	        }

	        oi = matostr(pcsopts,3,kp,kl) ;
	        if (oi >= 0) kp = pcsopts[oi] ;

	        if ((oi >= 0) && (keyopt_fetch(kop,kp,NULL,NULL) == nrs)) {
	            rs = keyopt_loadvalue(kop,kp,vp,vl) ;
		}

	        if (rs < 0) break ;
	    } /* end for (general options) */
	} /* end if */

	return rs ;
}
/* end subroutine (procopts_setpcs) */


/* process program options */
static int procopts(pip,kop)
struct proginfo	*pip ;
KEYOPT		*kop ;
{
	int		rs = SR_OK ;
	int		cl ;
	const char	*cp ;

	if ((cp = getenv(VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int		oi, v ;
	        int		kl, vl ;
	        const char	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	            if ((oi = matostr(locopts,3,kp,kl)) >= 0) {

	                switch (oi) {
	                case locopt_deadmaildir:
	                    if ((vl > 0) && (pip->deadmaildname == NULL)) {
	                        const char	**vpp = &pip->deadmaildname ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case locopt_mbtab:
	                    if ((vl > 0) && (pip->mbfname == NULL)) {
	                        const char	**vpp = &pip->mbfname ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case locopt_comsat:
	                    cp = "+" ;
	                    cl = 1 ;
	                    if (vl > 0) {
				const int	vch = MKCHAR(vp[0]) ;
	                        if (isdigit(vch)) {
	                            if (cfdeci(vp,vl,&v) >= 0) {
	                                cp = (v > 0) ? "+" : "-" ;
	                            }
	                        } else {
	                            cp = vp ;
	                            cl = vl ;
	                        }
	                    }
	                    if (pip->comsatfname == NULL) {
	                        const char	**vpp = &pip->comsatfname ;
	                        rs = proginfo_setentry(pip,vpp,cp,cl) ;
	                    }
	                    break ;
	                case locopt_spam:
	                    if ((vl > 0) && (pip->spamfname == NULL)) {
	                        const char	**vpp = &pip->spamfname ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case locopt_spambox:
	                    if ((vl > 0) && (pip->spambox == NULL)) {
	                        const char	**vpp = &pip->spambox ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case locopt_boxdir:
	                    if ((vl > 0) && (pip->boxdname == NULL)) {
	                        const char	**vpp = &pip->boxdname ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case locopt_boxname:
	                    if ((vl > 0) && (pip->boxname == NULL)) {
	                        const char	**vpp = &pip->boxname ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case locopt_timeout:
	                    if ((vl > 0) && (pip->to_spool < 0)) {
	                        rs = cfdecti(vp,vl,&v) ;
	                        pip->to_spool = v ;
	                    }
	                    break ;
	                case locopt_tomsgread:
	                    if ((vl > 0) && (pip->to_msgread < 0)) {
	                        rs = cfdecti(vp,vl,&v) ;
	                        pip->to_msgread = v ;
	                    }
	                    break ;
	                case locopt_logconf:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.optlogconf = (rs > 0) ;
	                    }
	                    break ;
	                case locopt_logmsg:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.optlogmsg = (rs > 0) ;
	                    }
	                    break ;
	                case locopt_logzone:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.optlogzone = (rs > 0) ;
	                    }
	                    break ;
	                case locopt_logenv:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.optlogenv = (rs > 0) ;
	                    }
	                    break ;
	                case locopt_logmsgid:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.optlogmsgid = (rs > 0) ;
	                    }
	                    break ;
	                case locopt_divert:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.optdivert = (rs > 0) ;
	                    }
	                    break ;
	                case locopt_forward:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.optforward = (rs > 0) ;
	                    }
	                    break ;
	                case locopt_nospam:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.optnospam = (rs > 0) ;
	                    }
	                    break ;

	                case locopt_norepeat:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.optnorepeat = (rs > 0) ;
	                    }
	                    break ;
	                case locopt_nopollmsg:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.nopollmsg = (rs > 0) ;
	                    }
	                    break ;
	                case locopt_mailhist:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.optmailhist = (rs > 0) ;
	                    }
	                    break ;
	                case locopt_deliver:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.optdeliver = (rs > 0) ;
	                    }
	                    break ;
	                case locopt_finish:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.optfinish = (rs > 0) ;
	                    }
	                    break ;
	                } /* end switch */

	            } else if ((oi = matostr(pcsopts,3,kp,kl)) >= 0) {

	                switch (oi) {
	                case pcsopt_maildir:
	                    if ((vl > 0) && (pip->maildname == NULL)) {
	                        const char	**vpp = &pip->maildname ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case pcsopt_cluster:
	                    if ((vl > 0) && (pip->cluster == NULL)) {
	                        const char	**vpp = &pip->cluster ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case pcsopt_pcsadmin:
	                    if ((vl > 0) && (pip->username_pcs == NULL)) {
	                        const char	**vpp = &pip->username_pcs ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case pcsopt_logsize:
	                case pcsopt_loglen:
	                    if ((vl > 0) && (pip->logsize < 0)) {
	                        rs = cfdecmfi(vp,vl,&v) ;
	                        pip->logsize = v ;
	                    }
	                    break ;
	                case pcsopt_pcspoll:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.pcspoll = (rs > 0) ;
	                    }
	                    break ;
	                case pcsopt_logsys:
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.optlogsys = (rs > 0) ;
	                    }
	                    break ;
	                } /* end switch */

	            } /* end if */

	            if (rs < 0) break ;
	        } /* end while */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (procopts) */


static int procargs(pip,aip,app,rlp,afname,ofname)
struct proginfo	*pip ;
struct arginfo	*aip ;
BITS		*app ;
VECOBJ		*rlp ;
const char	*afname ;
const char	*ofname ;
{
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0){
	    const char	*cp ;

	    if (rs >= 0) {
	        int	ai ;
	        int	f ;
	        for (ai = 1 ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(app,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
	                if (cp[0] != '\0') {
	                    rs = loadrecip(pip,rlp,cp,-1) ;
	                    c += rs ;
	                }
	            }
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	        bfile	afile, *afp = &afile ;

	        if (strcmp(afname,"-") == 0) afname = BFILE_STDIN ;

	        if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((len > 0) && (lbuf[0] != '#')) {
	                    rs = loadrecips(pip,rlp,lbuf,len) ;
	                    c += rs ;
	                }

	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = bclose(afp) ;
		    if (rs >= 0) rs = rs1 ;
	        } else {
	            if (! pip->f.quiet) {
	                bprintf(pip->efp,
	                    "%s: inaccessible argument list file (%d)\n",
	                    pip->progname,rs) ;
	                bprintf(pip->efp,"%s: afile=%s\n",
	                    pip->progname,afname) ;
	            }
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    if ((rs >= 0) && (pip->nrecips == 0)) {
	        cp = pip->username ;
	        rs = loadrecip(pip,rlp,cp,-1) ;
	        c += rs ;
	    } /* end if */

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (open-file) */

#if	CF_DEBUG && CF_DEBUGRECIPS
	if (DEBUGLEVEL(3))
	    debugrecips(pip,rlp) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procargs: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


static int logrecip(pip,recip,rs1,pwp)
struct proginfo	*pip ;
const char	recip[] ;
int		rs1 ;
struct passwd	*pwp ;
{
	int		rs = SR_OK ;

	if (pip->f.logmsg) {
	    char	namebuf[REALNAMELEN + 1] = { 0 } ;
	    const char	*fmt = "recip=%s" ;

	    if ((rs1 >= 0) && (pwp != NULL)) {

	        rs1 = mkrecipname(namebuf,REALNAMELEN,pwp->pw_gecos) ;

	        if ((rs1 >= 0) && (rs1 <= 50))
	            fmt = "recip=%s (%s)" ;

	    } /* end if (was a real user) */

	    rs = logfile_printf(&pip->lh,fmt, recip,namebuf) ;

	} /* end if (logmsg) */

	return rs ;
}
/* end subroutine (logrecip) */


/* extract the real name from a GECOS name */
static int mkrecipname(rname,rnamelen,gecos)
char		rname[] ;
int		rnamelen ;
const char	gecos[] ;
{
	int		rs1 ;
	int		rl = 0 ;
	char		gname[GNAMELEN + 1] ;

	if ((rs1 = mkgecosname(gname,GNAMELEN,gecos)) >= 0) {
	    int	gl = rs1 ;
	    rs1 = mkrealname(rname,rnamelen,gname,gl) ;
	    rl = rs1 ;
	}

	return (rs1 >= 0) ? rl : 0 ;
}
/* end subroutine (mkrecipname) */


static int loadrecips(pip,rlp,sp,sl)
struct proginfo	*pip ;
VECOBJ		*rlp ;
const char	sp[] ;
int		sl ;
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		fl ;
	    const char	*fp ;

	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = loadrecip(pip,rlp,fp,fl) ;
	            c += rs ;
		}
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */

	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadrecips) */


static int loadrecip(pip,rlp,np,nl)
struct proginfo	*pip ;
VECOBJ		*rlp ;
const char	np[] ;
int		nl ;
{
	struct locinfo	*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (np == NULL) return SR_FAULT ;

	if (np[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/loadrecip: rn=%t\n",np,nl) ;
#endif

	if (nl < 0) nl = strlen(np) ;

	if ((np[0] == '\0') || (strcmp(np,"-") == 0)) {
	    np = pip->username ;
	    nl = -1 ;
	} else if (strcmp(np,"+") == 0) {
	    np = pip->username_pcs ;
	    nl = -1 ;
	}

	if ((np != NULL) && (np[0] != '\0')) {
	    const int	nch = MKCHAR(np[0]) ;
	    const char	*tp ;

	    if ((tp = strnchr(np,nl,'+')) != NULL) {
	        nl = (tp-np) ;
	    }

	    if (strnchr(np,nl,'.') != NULL) {
	        LOCINFO_RNCUR	rnc ;
	        if ((rs = locinfo_rncurbegin(lip,&rnc)) >= 0) {
	            if ((rs = locinfo_rnlook(lip,&rnc,np,nl)) >= 0) {
	                const int	ul = USERNAMELEN ;
	                char		ub[USERNAMELEN+1] ;
	                while ((rs = locinfo_rnread(lip,&rnc,ub,ul)) >= 0) {
	                    rs = vecobj_recipadd(rlp,ub,rs) ;
	                    c += rs ;
	                    pip->nrecips += rs ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(5))
	                        debugprintf("main/loadrecip: "
					"vecobj_recipadd() rs=%d\n",
	                            rs) ;
#endif
	                    if (rs < 0) break ;
	                } /* end while (reading entries) */
#if	CF_DEBUG
	                if (DEBUGLEVEL(5))
	                    debugprintf("main/loadrecip: "
				"while-out rs=%d\n",rs) ;
#endif
	            } /* end if */
#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("main/loadrecip: rnlook-out rs=%d\n",rs) ;
#endif
	            if (rs == SR_NOTFOUND) rs = SR_OK ;
	            locinfo_rncurend(lip,&rnc) ;
	        } /* end if (srncursor) */
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("main/loadrecip: rncur-out rs=%d\n",rs) ;
#endif
	    } else if (nch == MKCHAR('¡')) {
	        LOCINFO_GMCUR	gc ;
	        const char	*gnp = (np+1) ;
	        const int	gnl = (nl-1) ;
	        if ((rs = locinfo_gmcurbegin(lip,&gc)) >= 0) {
	            if ((rs = locinfo_gmlook(lip,&gc,gnp,gnl)) >= 0) {
	                const int	ul = USERNAMELEN ;
	                char		ub[USERNAMELEN+1] ;
	                while ((rs = locinfo_gmread(lip,&gc,ub,ul)) >= 0) {
	                    rs = vecobj_recipadd(rlp,ub,rs) ;
	                    c += rs ;
	                    pip->nrecips += rs ;
	                    if (rs < 0) break ;
	                } /* end while */
	            } /* end if */
	            if (rs == SR_NOTFOUND) rs = SR_OK ;
	            locinfo_gmcurend(lip,&gc) ;
	        } /* end if (gmcursor) */
	    } else {
	        if (nch == '!') {
	            np += 1 ;
	            nl -= 1 ;
	        }
	        if (nl > 0) {
	            rs = vecobj_recipadd(rlp,np,nl) ;
	            c += rs ;
	            pip->nrecips += rs ;
	        }
	    } /* end if */

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/loadrecip: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadrecip) */


static int procspamsetup(pip,ilp)
struct proginfo	*pip ;
vecobj		*ilp ;
{
	struct msginfo	*mop ;
	int		rs = SR_OK ;
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; vecobj_get(ilp,i,&mop) >= 0 ; i += 1) {
	    if (mop == NULL) continue ;

	    mop->f.spamdeliver = mop->f.spam ;
	    f = TRUE ;
	    break ;

	} /* end for */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procspamsetup) */


static int procspambox(pip,lip,svp,ilp,rlp,tfd)
struct proginfo	*pip ;
struct locinfo	*lip ;
vecstr		*svp ;
vecobj		*ilp ;
vecobj		*rlp ;
int		tfd ;
{
	struct msginfo	*mop ;
	RECIP		r, *rp = &r ;
	RECIP		*orp ;
	MSGID_KEY	midkey ;
	MSGID_ENT	mid ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;
	int		f_repeat = FALSE ;
	int		f_deliver ;
	int		f ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procspambox: ent\n") ;
#endif

	if ((pip->spambox == NULL) || (pip->spambox[0] == '\0'))
	    goto ret0 ;

/* did we already deliver to this "recipient" mailbox? */

	for (i = 0 ; (rs1 = vecobj_get(rlp,i,&orp)) >= 0 ; i += 1) {
	    if (orp == NULL) continue ;

	    f = recip_match(orp,pip->spambox,-1) ;
	    if (f)
	        break ;

	} /* end for */

	if (rs1 >= 0)
	    goto ret0 ;

/* proceed */

	if ((rs = recip_start(rp,pip->spambox,-1)) >= 0) {
	    time_t	dt ;

	    pip->daytime = time(NULL) ;
	    dt = pip->daytime ;
	    memset(&midkey,0,sizeof(MSGID_KEY)) ;

	    midkey.recip = rp->recipient ;
	    midkey.reciplen = -1 ;
	    for (i = 0 ; vecobj_get(ilp,i,&mop) >= 0 ; i += 1) {
	        if (mop == NULL) continue ;

	        if (! mop->f.spamdeliver) continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: msg=%u moff=%u mlen=%u\n",
	                i,mop->moff,mop->mlen) ;
#endif

	        mid.count = 0 ;
	        if (lip->f.mids && mop->f.messageid && 
	            (mop->h_messageid != NULL) && 
	            (mop->h_messageid[0] != '\0')) {

	            midkey.mtime = mop->mtime ;
	            midkey.from = mop->h_from ;
	            midkey.mid = mop->h_messageid ;

	            rs1 = msgid_update(&lip->mids,dt,&midkey,&mid) ;

	            if (rs1 == SR_ACCESS)
	                rs1 = msgid_match(&lip->mids,dt,&midkey,&mid) ;

	            f_repeat = ((rs1 >= 0) && (mid.count > 0)) ;

	        } else
	            rs1 = SR_NOTFOUND ;

	        f_deliver = TRUE ;

/* repeat message-id? */

	        if (f_deliver && pip->f.optnorepeat && f_repeat)
	            f_deliver = FALSE ;

	        if (f_deliver) {
	            c += 1 ;
	            rs = recip_mo(rp,mop->moff,mop->mlen) ;
	        }

	        if (rs < 0) break ;
	    } /* end for (looping through messages) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procspambox: nrecips=%u\n",rp->n) ;
#endif

	    if ((rs >= 0) && (rp->n > 0)) {

		if (pip->progmode == progmode_dmail) {
	            if (pip->f.optdeliver) {
	                rs = deliver(pip,tfd,rp) ;
		    }
		}

	        if (rs < 0) {

	            if (pip->f.logmsg)
	                logfile_printf(&pip->lh, 
	                    "delivery failure u=%s (%d)\n",
	                    rp->recipient,rs) ;

	            if (pip->debuglevel > 0)
	                bprintf(pip->efp, 
	                    "%s: delivery failure (%d)\n",
	                    pip->progname,rs) ;

	        } /* end if */

	    } /* end if (delivery) */

	    recip_finish(rp) ;
	} /* end if (recip) */

ret0:
	return rs ;
}
/* end subroutine (procspambox) */


static int procrecips(pip,lip,svp,ilp,rlp,tfd)
struct proginfo	*pip ;
struct locinfo	*lip ;
vecstr		*svp ;
vecobj		*ilp ;
vecobj		*rlp ;
int		tfd ;
{
	struct msginfo	*mop ;
	struct passwd	pw ;
	RECIP		*rp ;
	MSGID_KEY	midkey ;
	MSGID_ENT	mid ;
	time_t		dt ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		mn ;
	int		sl ;
	int		c = 0 ;
	const char	*fmt ;
	char		pwbuf[PWBUFLEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procrecips: ent\n") ;
#endif

	pip->daytime = time(NULL) ;
	dt = pip->daytime ;

	for (i = 0 ; vecobj_get(rlp,i,&rp) >= 0 ; i += 1) {
	    if (rp == NULL) continue ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp, "%s: recipient=%s\n",
	            pip->progname,rp->recipient) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procrecips: recipient=%s\n",rp->recipient) ;
#endif

#ifdef	COMMENT
	    if (pip->f.logmsg)
	        logfile_printf(&pip->lh,"recip=%s\n",rp->recipient) ;
#endif

/* prepare for white and black list lookup */

	    rs1 = uc_getpwnam(rp->recipient,&pw,pwbuf,PWBUFLEN) ;

	    if (pip->f.logmsg)
	        logrecip(pip,rp->recipient,rs1,&pw) ;

	    if (rs1 >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procrecips: home=%s\n",pw.pw_dir) ;
#endif

	        vecstr_envset(svp,"h",pw.pw_dir,-1) ;

/* user-specific whitelist? */

	        lip->f.wl_system = FALSE ;
	        if (pip->f.optnospam) {

	            sl = permsched(sched3,svp,
	                tmpfname,MAXPATHLEN,WLFNAME,R_OK) ;

	            if (sl > 0) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("main/procrecips: whitelist fname=%s\n",
	                        tmpfname) ;
#endif

	                rs1 = whitelist_open(&lip->wl2,tmpfname) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("main/procrecips: wl2 "
	                        "whitelist_open() rs=%d\n",
	                        rs1) ;
#endif

	                if (rs1 >= 0) {
	                    lip->f.wl_local = TRUE ;
	                    if (rs1 > 0)
	                        pip->nwl_local += rs1 ;
	                }

	            } /* end if (got a file) */

	        } /* end if (whitelist check) */

/* user-specific blacklist? */

	        sl = permsched(sched3,svp,tmpfname,MAXPATHLEN,BLFNAME,R_OK) ;

	        if (sl > 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(3)) {
	                debugprintf("main/procrecips: BL2 (local)\n") ;
	                debugprintf("main/procrecips: fn=%s\n",tmpfname) ;
		    }
#endif

	            rs1 = whitelist_open(&lip->bl2,tmpfname) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("main/procrecips: "
			"blacklist_open() rs=%d\n",rs1) ;
#endif

	            if (rs1 >= 0) {
	                lip->f.bl_local = TRUE ;
	                if (rs1 > 0)
	                    pip->nbl_local += rs1 ;
	            }

	        } /* end if (got a file) */

	    } /* end if (looked up username) */

	    memset(&midkey,0,sizeof(MSGID_KEY)) ;

	    midkey.recip = rp->recipient ;
	    midkey.reciplen = -1 ;
	    for (mn = 0 ; vecobj_get(ilp,mn,&mop) >= 0 ; mn += 1) {
	        int	f_repeat = FALSE ;
	        int	f_blacklist = FALSE ;
	        int	f_deliver ;

	        if (mop == NULL) continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procrecips: msg=%u off=%u mlen=%u\n",
	                mn,mop->moff,mop->mlen) ;
#endif

	        mid.count = 0 ;
	        if (lip->f.mids && mop->f.messageid && 
	            (mop->h_messageid != NULL) && 
	            (mop->h_messageid[0] != '\0')) {

	            if (((c & 255) == 255) || ((mn & 15) == 15)) {
	                pip->daytime = time(NULL) ;
		    }
	            dt = pip->daytime ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procrecips: messageid=%s\n",
	                    mop->h_messageid) ;
#endif

	            midkey.mtime = mop->mtime ;
	            midkey.from = mop->h_from ;
	            midkey.mid = mop->h_messageid ;

	            rs1 = msgid_update(&lip->mids,dt,&midkey,&mid) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procrecips: msgid_update() rs=%d\n",
			rs1) ;
#endif

	            if (rs1 == SR_ACCESS) {

	                rs1 = msgid_match(&lip->mids,dt,&midkey,&mid) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("main/procrecips: "
				"msgid_match() rs=%d\n",rs1) ;
#endif

	            }

	            f_repeat = ((rs1 >= 0) && (mid.count > 0)) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                debugprintf("main/procrecips: f_repeat=%u\n",
				f_repeat) ;
	                if (rs1 >= 0)
	                    debugprintf("main/procrecips: "
				"MID count=%u\n",mid.count) ;
	            }
#endif

	        } else
	            rs1 = SR_NOTFOUND ;

	        f_deliver = TRUE ;

/* repeat message-id? */

	        if (f_deliver && pip->f.optnorepeat && f_repeat)
	            f_deliver = FALSE ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("main/procrecips: repeat deliver=%u\n",
	                f_deliver) ;
	            debugprintf("main/procrecips: f_optnospam=%u\n",
			pip->f.optnospam) ;
	            debugprintf("main/procrecips: f_spam=%u\n",
			mop->f.spam) ;
		}
#endif

/* is this address in any whitelist? */

	        if (f_deliver && pip->f.optnospam && mop->f.spam) {

	            f_deliver = FALSE ;
	            rs1 = SR_NOTFOUND ;
	            if (lip->f.wl_local && (pip->nwl_local > 0)) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("main/procrecips: "
				"local wl2 prematch?\n") ;
#endif

	                rs1 = whitelist_prematch(&lip->wl2,mop->h_from) ;

	                if (isNotPrematch(rs1) && 
	                    (mop->h_replyto[0] != '\0') &&
	                    (strcmp(mop->h_from,mop->h_replyto) != 0))
	                    rs1 = whitelist_prematch(&lip->wl2,
	                        mop->h_replyto) ;

	                if (isNotPrematch(rs1) && 
	                    (mop->h_sender[0] != '\0') &&
	                    (strcmp(mop->h_from,mop->h_sender) != 0))
	                    rs1 = whitelist_prematch(&lip->wl2,
	                        mop->h_sender) ;

	                if (isNotPrematch(rs1) && 
	                    (mop->h_returnpath[0] != '\0') && 
	                    (strcmp(mop->h_from,mop->h_returnpath) != 0))
	                    rs1 = whitelist_prematch(&lip->wl2,
	                        mop->h_returnpath) ;

	            } /* end if (local whitelist check) */

	                if (isNotPrematch(rs1) && 
	                lip->f.wl_system && (pip->nwl_system > 0)) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("main/procrecips: "
				"system wl1 prematch?\n") ;
#endif

	                rs1 = whitelist_prematch(&lip->wl1,mop->h_from) ;

	                if (isNotPrematch(rs1) && 
	                    (mop->h_replyto[0] != '\0') &&
	                    (strcmp(mop->h_from,mop->h_replyto) != 0))
	                    rs1 = whitelist_prematch(&lip->wl1,
	                        mop->h_replyto) ;

	                if (isNotPrematch(rs1) && 
	                    (mop->h_sender[0] != '\0') &&
	                    (strcmp(mop->h_from,mop->h_sender) != 0))
	                    rs1 = whitelist_prematch(&lip->wl1,
	                        mop->h_sender) ;

	                if (isNotPrematch(rs1) && 
	                    (mop->h_returnpath[0] != '\0') && 
	                    (strcmp(mop->h_from,mop->h_returnpath) != 0))
	                    rs1 = whitelist_prematch(&lip->wl1,
	                        mop->h_returnpath) ;

	            } /* end if (system whitelist check) */

	            if (rs1 >= 0)
	                f_deliver = TRUE ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procrecips: whitelist deliver=%u\n",
	                    f_deliver) ;
#endif

	        } /* end if (spam delivery determination) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procrecips: spam deliver=%u\n",
	                f_deliver) ;
#endif

/* is this address in any blacklist? */

	        if (f_deliver) {

	            rs1 = SR_NOTFOUND ;
	            if (lip->f.bl_local && (pip->nbl_local > 0)) {

	                rs1 = whitelist_prematch(&lip->bl2,mop->h_from) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procrecips: BL2 prematch From rs1=%u\n",rs1) ;
#endif

	                if (isNotPrematch(rs1) && 
	                    (mop->h_replyto[0] != '\0') &&
	                    (strcmp(mop->h_from,mop->h_replyto) != 0))
	                    rs1 = whitelist_prematch(&lip->bl2,
	                        mop->h_replyto) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procrecips: BL2 prematch Replyto rs1=%u\n",rs1) ;
#endif

	                if (isNotPrematch(rs1) && 
	                    (mop->h_sender[0] != '\0') &&
	                    (strcmp(mop->h_from,mop->h_sender) != 0))
	                    rs1 = whitelist_prematch(&lip->bl2,
	                        mop->h_sender) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procrecips: BL2 prematch Sender rs1=%u\n",rs1) ;
#endif

	                if (isNotPrematch(rs1) && 
	                    (mop->h_returnpath[0] != '\0') && 
	                    (strcmp(mop->h_from,mop->h_returnpath) != 0))
	                    rs1 = whitelist_prematch(&lip->bl2,
	                        mop->h_returnpath) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procrecips: BL2 rs1=%u\n",rs1) ;
#endif

	            } /* end if (local blacklist check) */

	                if (isNotPrematch(rs1) && 
	                lip->f.bl_system && (pip->nbl_system > 0)) {

	                rs1 = whitelist_prematch(&lip->bl1,mop->h_from) ;

	                if (isNotPrematch(rs1) && 
	                    (mop->h_replyto[0] != '\0') &&
	                    (strcmp(mop->h_from,mop->h_replyto) != 0))
	                    rs1 = whitelist_prematch(&lip->bl1,
	                        mop->h_replyto) ;

	                if (isNotPrematch(rs1) && 
	                    (mop->h_sender[0] != '\0') &&
	                    (strcmp(mop->h_from,mop->h_sender) != 0))
	                    rs1 = whitelist_prematch(&lip->bl1,
	                        mop->h_sender) ;

	                if (isNotPrematch(rs1) && 
	                    (mop->h_returnpath[0] != '\0') && 
	                    (strcmp(mop->h_from,mop->h_returnpath) != 0))
	                    rs1 = whitelist_prematch(&lip->bl1,
	                        mop->h_returnpath) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procrecips: BL1 rs1=%u\n",rs1) ;
#endif

	            } /* end if (system blacklist check) */

	            if (rs1 > 0) {
	                f_blacklist = TRUE ;
	                f_deliver = FALSE ;
	            }

	        } /* end if (blacklist check) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procrecips: final f_deliver=%u\n",f_deliver) ;
#endif

/* deliver or not based on what we know */

	        if (f_deliver) {
	            mop->f.spamdeliver = FALSE ;
	            c += 1 ;
	            rs = recip_mo(rp,mop->moff,mop->mlen) ;
	        }

/* formulate log entry as a result */

	        if ((rs >= 0) && pip->f.logmsg) {

	            if (f_repeat) {
	                fmt = "  %3u %c%c%c%c=%u" ;
	            } else
	                fmt = "  %3u %c%c%c%c" ;

	            logfile_printf(&pip->lh,fmt,
	                mn,
	                ((f_deliver) ? 'D' : ' '),
	                ((f_blacklist) ? 'B' : ' '),
	                ((mop->f.spam) ? 'S' : ' '),
	                ((f_repeat) ? 'R' : ' '),
	                mid.count) ;

	        } /* end if (logging enabled) */

	        if (rs < 0) break ;
	    } /* end for (looping through messages) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procrecips: about n=%u\n",rp->n) ;
#endif

	    pip->c_processed += 1 ;
	    if ((rs >= 0) && (rp->n > 0)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procrecips: go n=u\n",rp->n) ;
#endif


		if (pip->progmode == progmode_dmail) {

	        if (pip->f.optdeliver)
	            rs = deliver(pip,tfd,rp) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procrecips: deliver=%u deliver() rs=%d\n",
	                pip->f.optdeliver,rs) ;
#endif

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf("main/procrecips: after deliver\n") ;
	            if (pip->debuglevel > 0)
	                bprintf(pip->efp, 
	                    "%s: deliver (%d)\n",
	                    pip->progname,rs) ;
	        }
#endif

		} /* end if (progmode-dmail) */

		if (pip->progmode == progmode_dmailbox) {

	        if (locinfo_mboxcount(lip) == 0) {
	            if ((pip->boxname != NULL) && (pip->boxname[0] != '\0')) {
	                rs = locinfo_mboxadd(lip,pip->boxname,-1) ;
	            }
	        }

	        if (rs >= 0)
	            rs = boxer(pip,tfd,rp) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procrecips: boxer() rs=%d\n",rs) ;
#endif

		} /* end if (progmode-dmailbox) */

	        if (rs < 0) {

	            if (pip->f.logmsg)
	                logfile_printf(&pip->lh, 
	                    "delivery failure u=%s (%d)\n",
	                    rp->recipient,rs) ;

	            if (pip->debuglevel > 0)
	                bprintf(pip->efp, 
	                    "%s: delivery failure (%d)\n",
	                    pip->progname,rs) ;

	        } /* end if */

	    } /* end if (delivery) */

	    if ((rs >= 0) && (rp->n > 0))
	        pip->c_delivered += 1 ;

	    if (pip->f.logmsg) {

	        if (rp->n > 0) {
	            if (rs >= 0) {
	                logfile_printf(&pip->lh, 
	                    "  delivery=%u offset=%d",
	                    rp->n,rs) ;
	            } else {
	                logfile_printf(&pip->lh, 
	                    "  delivery=%u FAILED (%d)",
	                    rp->n,rs) ;
		    }
	        } else {
	            logfile_printf(&pip->lh, 
	                "  delivery=0\n") ;
		}
	    } /* end if (logging) */

	    if ((pip->debuglevel > 0) && (! pip->f.quiet)) {

	        fmt = "%s: recip=%-32s %s (%d)\n" ;
	        if ((rs >= 0) || (rp->n == 0))
	            fmt = "%s: recip=%-32s\n" ;

	        bprintf(pip->efp,fmt,
	            pip->progname,
	            rp->recipient,((rs >= 0) ? "" : "FAILED"),rs) ;

	    } /* end if */

	    if (lip->f.bl_local) {
	        lip->f.bl_local = FALSE ;
	        whitelist_close(&lip->bl2) ;
	    }

	    if (lip->f.wl_local) {
	        lip->f.wl_local = FALSE ;
	        whitelist_close(&lip->wl2) ;
	    }

	    if ((rs < 0) && (! pip->f.multirecip))
	        break ;

	} /* end for (looping through recipients) */

#if	CF_DEBUGS
	rs1 = vecobj_audit(rlp) ;
	debugprintf("main/procrecips: 4 vecobj_audit() rs=%d\n",rs1) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    const char	*pn = pip->progname ;
	    debugprintf("main/procrecips: ret rs=%d\n",rs) ;
	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: procrecips-ret (%d)\n",pn,rs) ;
	}
#endif

	return rs ;
}
/* end subroutine (procrecips) */


static int procunavail(pip,rstat)
struct proginfo	*pip ;
int		rstat ;
{
	int		rs1 = SR_OK ;
	int		f ;

	if (pip->open.logprog)
	    logfile_printf(&pip->lh, 
	        "maildir=%s unavailable",
	        pip->maildname) ;

	if (pip->f.trusted && pip->f.optlogsys) {
	    LOGSYS	ls, *lsp = &ls ;
	    const int	fac = LOG_MAIL ;
	    int		opts = 0 ;
	    const char	*logtab = pip->searchname ;
	    const char	*logid = pip->logid ;

	    if ((rs1 = logsys_open(lsp,fac,logtab,logid,opts)) >= 0) {
	        const char	*fmt = "maildir=%s unavailable (%d)" ;

	        rs1 = logsys_printf(lsp,LOG_ERR,fmt,pip->maildname,rstat) ;

	        logsys_close(lsp) ;
	    } /* end if (logsys) */

	    if (rs1 < 0) {
	        bprintf(pip->efp,"%s: inaccessible syslog (%d)\n",
	            pip->progname,rs1) ;
	        if (pip->open.logprog)
	            logfile_printf(&pip->lh,"inaccessible syslog (%d)",rs1) ;
	    }

	} /* end if (logging to the system) */

	f = pip->f.optdivert && (pip->deadmaildname != NULL) ;
	if (f)
	    rs1 = perm(pip->deadmaildname,-1,-1,NULL,W_OK) ;

	if (f && (rs1 >= 0)) {
	    struct passwd	pw ;
	    const int	pwlen = PWBUFLEN ;
	    char	pwbuf[PWBUFLEN + 1] ;

	    pip->f.diverting = TRUE ;
	    pip->maildname = pip->deadmaildname ;

	    if (pip->open.logprog) {
	        logfile_printf(&pip->lh, 
	            "diverting maildir=%s\n",
	            pip->maildname) ;
	    }

	    rs1 = uc_getpwnam(DIVERTUSER,&pw,pwbuf,pwlen) ;
	    if (rs1 >= 0) pip->uid_divert = pw.pw_uid ;

	} /* end if */

	return rs1 ;
}
/* end subroutine (procunavail) */


static int procmboxes(struct proginfo *pip,const char *sp,int sl)
{
	struct locinfo	*lip = pip->lip ;
	int		rs = SR_OK ;
	int		cl ;
	int		c = 0 ;
	const char	*tp ;
	const char	*cp ;

	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	while ((tp = strnpbrk(sp,sl,", :")) != NULL) {
	    if ((cl = sfshrink(sp,(tp-sp),&cp)) > 0) {
	        c += 1 ;
	        rs = locinfo_mboxadd(lip,cp,cl) ;
	    }
	    sl -= ((tp+1)-sp) ;
	    sp = (tp+1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	        c += 1 ;
	        rs = locinfo_mboxadd(lip,cp,cl) ;
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmboxes) */


static int locinfo_start(lip,pip)
struct locinfo	*lip ;
struct proginfo	*pip ;
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(struct locinfo)) ;
	lip->pip = pip ;
	lip->to = -1 ;

#ifdef	COMMENT
	rs = vecstr_start(&lip->stores,0,0) ;
	lip->open.stores = (rs >= 0) ;
#endif /* COMMENT */

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(lip)
struct locinfo	*lip ;
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

	if (lip->f.mboxes) {
	    lip->f.mboxes = FALSE ;
	    rs1 = vecstr_finish(&lip->mboxes) ;
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


#if	CF_LOCSETENT
int locinfo_setentry(lip,epp,vp,vl)
struct locinfo	*lip ;
const char	**epp ;
const char	vp[] ;
int		vl ;
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

	    if (*epp != NULL)
	        oi = vecstr_findaddr(&lip->stores,*epp) ;

	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(&lip->stores,vp,len,epp) ;
	    } else
	        *epp = NULL ;

	    if ((rs >= 0) && (oi >= 0))
	        vecstr_del(&lip->stores,oi) ;

	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCSETENT */


int locinfo_gmcurbegin(lip,curp)
LOCINFO		*lip ;
LOCINFO_GMCUR	*curp ;
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


int locinfo_gmcurend(lip,curp)
LOCINFO		*lip ;
LOCINFO_GMCUR	*curp ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (curp == NULL) return SR_FAULT ;

	rs1 = grmems_curend(&lip->gm,&curp->gmcur) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_gmcurend) */


int locinfo_gmlook(lip,curp,gnp,gnl)
LOCINFO		*lip ;
LOCINFO_GMCUR	*curp ;
const char	*gnp ;
int		gnl ;
{
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;

	rs = grmems_lookup(&lip->gm,&curp->gmcur,gnp,gnl) ;

	return rs ;
}
/* end subroutine (locinfo_gmlook) */


int locinfo_gmread(lip,curp,ubuf,ulen)
LOCINFO		*lip ;
LOCINFO_GMCUR	*curp ;
char		ubuf[] ;
int		ulen ;
{
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;

	rs = grmems_lookread(&lip->gm,&curp->gmcur,ubuf,ulen) ;

	return rs ;
}
/* end subroutine (locinfo_gmread) */


int locinfo_rncurbegin(lip,curp)
LOCINFO		*lip ;
LOCINFO_RNCUR	*curp ;
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


int locinfo_rncurend(lip,curp)
LOCINFO		*lip ;
LOCINFO_RNCUR	*curp ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (curp == NULL) return SR_FAULT ;

	rs1 = sysrealname_curend(&lip->rn,&curp->rncur) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_rncurend) */


int locinfo_rnlook(lip,curp,gnp,gnl)
LOCINFO		*lip ;
LOCINFO_RNCUR	*curp ;
const char	*gnp ;
int		gnl ;
{
	struct proginfo	*pip = lip->pip ;
	const int	fo = 0 ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;

	rs = sysrealname_look(&lip->rn,&curp->rncur,fo,gnp,gnl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rnlook: sysrealname_look() rs=%d\n",
	        rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_rnlook) */


int locinfo_rnread(lip,curp,ubuf,ulen)
LOCINFO		*lip ;
LOCINFO_RNCUR	*curp ;
char		ubuf[] ;
int		ulen ;
{
	struct proginfo	*pip = lip->pip ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;

	if ((ulen >= 0) && (ulen < USERNAMELEN)) return SR_OVERFLOW ;

	rs = sysrealname_lookread(&lip->rn,&curp->rncur,ubuf) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rnread: sysrealname_lookread() rs=%d\n",
	        rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_rnread) */


static int locinfo_mboxadd(lip,mp,ml)
struct locinfo	*lip ;
const char	*mp ;
int		ml ;
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;
	if (mp == NULL) return SR_FAULT ;

	if (! lip->f.mboxes) {
	    rs = vecstr_start(&lip->mboxes,0,0) ;
	    lip->f.mboxes = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = vecstr_adduniq(&lip->mboxes,mp,ml) ;
	}

	return rs ;
}
/* end subroutine (locinfo_mboxadd) */


static int locinfo_mboxcount(lip)
struct locinfo	*lip ;
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->f.mboxes) {
	    rs = vecstr_count(&lip->mboxes) ;
	}

	return rs ;
}
/* end subroutine (locinfo_mboxcount) */


static int locinfo_mboxget(lip,i,rpp)
struct locinfo	*lip ;
int		i ;
const char	**rpp ;
{
	int		rs = SR_OK ;
	int		rl = 0 ;
	const char	*rp = NULL ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->f.mboxes) {
	    if ((rs = vecstr_get(&lip->mboxes,i,&rp)) >= 0)
	        rl = strlen(rp) ;
	} else
	    rs = SR_NOTFOUND ;

	if (rpp != NULL)
	    *rpp = rp ;

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (locinfo_mboxget) */


#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGRECIPS
static int debugrecips(struct proginfo *pip,VECOBJ *rlp)
{
	int		rs = SR_OK ;
	if (DEBUGLEVEL(5)) {
	    RECIP	*rp ;
	    int	i ;
	    int	cl ;
	    const char	*cp ;
	    debugprintf("main/debugrecips: rlp={%p} ¬\n",rlp) ;
	    for (i = 0 ; vecobj_get(rlp,i,&rp) >= 0 ; i += 1) {
	        if ((rs = recip_get(rp,&cp)) > 0) {
	            cl = rs ;
	            debugprintf("main/debugrecips: r=%t\n",cp,cl) ;
	        }
	        if (rs < 0) break ;
	    } /* end for */
	    debugprintf("main/debugrecips: ret rs=%d\n",rs) ;
	}
	return rs ;
}
/* end subroutine (debugrecips) */
#endif /* CF_DEBUGS */


static int isNotPrematch(int rs)
{
	return ((rs == SR_NOTFOUND) || (rs == 0)) ;
}


