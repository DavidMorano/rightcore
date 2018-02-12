/* main (DMAIL) */

/* fairly generic (PCS) front-end */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable print-outs */
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_DEBUGN	0		/* special debugging */
#define	CF_DEBUGRECIPS	0		/* debug recipients */
#define	CF_PCSTRUSTUSER	1		/* call 'pcstrustuser(3pcs)' */
#define	CF_DEFLOGSIZE	0		/* use a default log length */
#define	CF_PROGMSGS	1		/* call 'progmsgs()' */
#define	CF_SETPCS	0		/* use |procopts_setpcs()| */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */
#define	CF_LOGRECIP	0		/* use |logrecip()| */
#define	CF_LOCSETENT	0		/* use |locinfo_setentry()| */
#define	CF_LOOKADDR	1		/* use |lookaddr(3dam)| */
#define	CF_ADDRCHECK	1		/* use |procrecip_addrcheck()| */
#define	CF_SPAMBOX	1		/* use |procspambox()| */


/* revision history:

	= 1998-05-01, David A­D­ Morano
        This code module was completely rewritten to replace the previous
        mail-delivery program for PCS, written around 1990 or so.

	= 2004-02-17, David A­D­ Morano
        This was modified to add the MSGID object. That is a database that
        stores message IDs. We used it to eliminate duplicate mail deliveries
        which as of late are coming from several popular sources!

*/

/* Copyright © 1998,2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the front-end subroutine (main) for the DMAIL and DMAILBOX
	pograms.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */
#include	<grp.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<getbufsize.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<nulstr.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<userattr.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<vecitem.h>
#include	<vecobj.h>
#include	<field.h>
#include	<kvsfile.h>
#include	<dater.h>
#include	<ids.h>
#include	<logsys.h>
#include	<tmtime.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"msgid.h"
#include	"recip.h"
#include	"config.h"
#include	"defs.h"
#include	"proglog.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#define	GETPW_UID	ugetpw_uid
#else
#define	GETPW_NAME	getpw_name
#define	GETPW_UID	getpw_uid
#endif /* CF_UGETPW */

#define	BUFLEN		MAX((2 * MAXHOSTNAMELEN),MSGBUFLEN)

#define	HOSTBUFLEN	(10 * MAXHOSTNAMELEN)

#define	PSBUFLEN	60

#ifndef	VARMAIL
#define	VARMAIL		"MAIL"
#endif

#define	NUCMEMALLOC	"ucmemalloc.deb"

#define	NDF		"dmail.ndeb"


/* local typedefs */


/* external subroutines */

extern int	sntmtime(char *,int,TMTIME *,cchar *) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	headkeymat(cchar *,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	permsched(cchar **,vecstr *,char *,int,cchar *,int) ;
extern int	mktmpfile(char *,mode_t,cchar *) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	mklogid(char *,int,cchar *,int,int) ;
extern int	mkgecosname(char *,int,cchar *) ;
extern int	mkrealame(char *,int,cchar *,int) ;
extern int	mkuibang(char *,int,USERINFO *) ;
extern int	mkuiname(char *,int,USERINFO *) ;
extern int	mkbestaddr(char *,int,cchar *,int) ;
extern int	pcstrustuser(cchar *,cchar *) ;
extern int	getnodename(char *,int) ;
extern int	getusername(char *,int,uid_t) ;
extern int	getserial(cchar *) ;
extern int	getportnum(cchar *,cchar *) ;
extern int	getgid_def(cchar *,gid_t) ;
extern int	getuid_name(cchar *,int) ;
extern int	getuid_user(cchar *,int) ;
extern int	initnow(struct timeb *,cchar *,int) ;
extern int	vecstr_envadd(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

extern int	printhelp(bfile *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	prognamecache_begin(PROGINFO *,USERINFO *) ;
extern int	prognamecache_end(PROGINFO *) ;

extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;

extern int	proglogenv_begin(PROGINFO *) ;
extern int	proglogenv_end(PROGINFO *) ;

extern int	proglogzone_begin(PROGINFO *) ;
extern int	proglogzone_end(PROGINFO *) ;

static int	procmsgid_begin(PROGINFO *) ;
static int	procmsgid_end(PROGINFO *) ;
static int	procmsgid_update(PROGINFO *,MSGINFO *,cchar *) ;

extern int	progexpand(PROGINFO *,char *,int,cchar *,int) ;
extern int	progmsgs(PROGINFO *,bfile *,bfile *,vecobj *,vecobj *) ;
extern int	progcomsat(PROGINFO *,VECOBJ *) ;
extern int	progdeliver(PROGINFO *,int,RECIP *) ;
extern int	progboxer(PROGINFO *,int,RECIP *) ;
extern int	parsenodespec(PROGINFO *,int,char *,int,char *,cchar *) ;
extern int	mkrealname(char *,int,cchar *,int) ;

extern int	vecobj_recipadd(vecobj *,cchar *,int) ;
extern int	vecobj_recipfins(vecobj *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif
#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strdcpy1w(char *,int,cchar *,int) ;
extern char	*strdcpy3(char *,int,cchar *,cchar *,cchar *) ;
extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;

#if	CF_LOGRECIP
static int	logrecip(PROGINFO *,cchar *,struct passwd *) ;
#endif /* CF_LOGRECIP */

static int	mkrecipname(char *,int,cchar *) ;

static int	loadrecips(PROGINFO *,VECOBJ *,cchar *,int) ;
static int	loadrecip(PROGINFO *,VECOBJ *,cchar *,int) ;

#if	CF_SETPCS
static int	procopts_setpcs(PROGINFO *,KEYOPT *,vecstr *) ;
#endif /* CF_SETPCS */

static int	procopts(PROGINFO *,KEYOPT *,PARAMOPT *) ;
static int	procnodecluster(PROGINFO *pip) ;
static int	procuucpinfo(PROGINFO *) ;
static int	procenvfromaddr(PROGINFO *) ;
static int	procprotospec(PROGINFO *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,PARAMOPT *,
			cchar *,cchar *,cchar *) ;
static int	processing(PROGINFO *,ARGINFO *,BITS *,
			cchar *,cchar *,cchar *) ;
static int	processings(PROGINFO *,VECOBJ *,VECOBJ *,cchar *) ;
static int	procfindfiles(PROGINFO *) ;
static int	procfindsched(PROGINFO *,vecstr *) ;
static int	procfindcomsat(PROGINFO *,vecstr *) ;
static int	procfindspam(PROGINFO *,vecstr *) ;
static int	procfindmbtab(PROGINFO *,vecstr *) ;
static int	procdefs(PROGINFO *) ;
static int	procuserboxes(PROGINFO *pip) ;
static int	procloginfo(PROGINFO *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,VECOBJ *,cchar *) ;
static int	procin(PROGINFO *,vecobj *,bfile *,vecobj *,cchar *) ;
static int	procmaildirdead(PROGINFO *,cchar *,int) ;
static int	procmaildircopy(PROGINFO *,cchar *,int) ;
static int	procmaildirs(PROGINFO *,PARAMOPT *) ;
static int	procmaildir(PROGINFO *,PARAMOPT *,cchar *,int) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;

static int	procstuff_begin(PROGINFO *,cchar *) ;
static int	procstuff_end(PROGINFO *) ;

static int	procourconf_begin(PROGINFO *,PARAMOPT *) ;
static int	procourconf_end(PROGINFO *) ;

static int	proclogs_begin(PROGINFO *) ;
static int	proclogs_end(PROGINFO *) ;
static int	proclogs_log(PROGINFO *) ;

static int	procmaildname_begin(PROGINFO *,PARAMOPT *) ;
static int	procmaildname_add(PROGINFO *,cchar *,int) ;
static int	procmaildname_end(PROGINFO *) ;

static int	proctmp_begin(PROGINFO *,bfile *,char *) ;
static int	proctmp_end(PROGINFO *,bfile *,char *) ;

static int	procspamsetup(PROGINFO *,vecobj *) ;
static int	procspambox(PROGINFO *,vecobj *,vecobj *,int) ;
static int	procspamboxing(PROGINFO *,vecobj *,RECIP *) ;
static int	procspamprogboxer(PROGINFO *,RECIP *,MSGINFO *) ;
static int	procrecips(PROGINFO *,vecobj *,vecobj *,int) ;
static int	procrecip(PROGINFO *,vecobj *,RECIP *,int) ;
static int	procrecipvalid(PROGINFO *,RECIP *) ;

static int	procrecip_mailspool(PROGINFO *,RECIP *) ;
static int	procrecip_hasmailfile(PROGINFO *,RECIP *) ;
static int	procrecip_defmaildir(PROGINFO *,RECIP *) ;

static int	procrecip_addropen(PROGINFO *) ;
static int	procrecip_addrbegin(PROGINFO *,LOOKADDR_USER *,cchar *) ;
static int	procrecip_addrend(PROGINFO *,LOOKADDR_USER *) ;
static int	procrecip_addrclose(PROGINFO *) ;

#if	CF_ADDRCHECK
static int	procrecip_addrcheck(PROGINFO *,LOOKADDR_USER *,MSGINFO *,int) ;
static int	procrecip_addrchecker(PROGINFO *,LOOKADDR_USER *,cchar *,int) ;
#endif /* CF_ADDRCHECK */

static int	procmboxes(PROGINFO *,cchar *,int) ;
static int	procportcomsat(PROGINFO *pip) ;

#ifdef	COMMENT
static int	procunavail(PROGINFO *,int) ;
#endif /* COMMENT */

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_mboxadd(LOCINFO *,cchar *,int) ;
static int	locinfo_mboxcount(LOCINFO *) ;
int		locinfo_rncurbegin(LOCINFO *,LOCINFO_RNCUR *) ;
int		locinfo_rncurend(LOCINFO *,LOCINFO_RNCUR *) ;
int		locinfo_rnlook(LOCINFO *,LOCINFO_RNCUR *,cchar *,int) ;
int		locinfo_rnread(LOCINFO *,LOCINFO_RNCUR *,char *,int) ;
int		locinfo_gmcurbegin(LOCINFO *,LOCINFO_GMCUR *) ;
int		locinfo_gmcurend(LOCINFO *,LOCINFO_GMCUR *) ;
int		locinfo_gmlook(LOCINFO *,LOCINFO_GMCUR *,cchar *,int) ;
int		locinfo_gmread(LOCINFO *,LOCINFO_GMCUR *,char *,int) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif /* CF_LOCSETENT */

int		locinfo_mboxget(LOCINFO *,int,cchar **) ;

static int	mkreport(PROGINFO *,int,cchar **,int) ;
static int	mkreportfile(PROGINFO *,char *,cchar *) ;
static int	mkreportout(PROGINFO *,cchar *,cchar *,int,cchar **,int) ;
static int	mktmpreportdir(char *,cchar *,cchar *,mode_t) ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGRECIPS
static int	debugrecips(PROGINFO *,VECOBJ *) ;
#endif


/* local variables */

static cchar *argopts[] = {
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
	"cf",
	"lf",
	"ms",
	"md",
	"mr",
	"nm",
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
	argopt_cf,
	argopt_lf,
	argopt_ms,
	argopt_md,
	argopt_mr,
	argopt_nm,
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

static cchar	*progmodes[] = {
	"dmail",
	"dmailbox",
	NULL
} ;

enum progmodes {
	progmode_dmail,
	progmode_dmailbox,
	progmode_overlast
} ;

static cchar *pcsopts[] = {
	"cluster",
	"pcsadmin",
	"maildir",
	"logsize",
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
	pcsopt_pcspoll,
	pcsopt_logsys,
	pcsopt_mailhist,
	pcsopt_overlast
} ;

static cchar *locopts[] = {
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
	"tospool",
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
	locopt_tospool,
	locopt_tomsgread,
	locopt_spambox,
	locopt_mailhist,
	locopt_deliver,
	locopt_finish,
	locopt_overlast
} ;

static cchar	*varmaildirs[] = {
	VARMAILDNAMEP,
	VARMAILDNAME,
	VARMAILDNAMES,
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


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	akparams ;
	bfile		errfile ;
#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs ;
	int		rs1 = SR_OK ;
	int		v ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_version = FALSE ;
	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*pmspec = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*ifname = NULL ;
	cchar		*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */
#if	CF_DEBUGN
	nprintf(NDF,"main: starting\n") ;
#endif

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

/* get the current time-of-day */

	{
	    struct timeb	*nowp = &pip->now ;
	    const int		zlen = DATER_ZNAMESIZE ;
	    if ((rs = initnow(nowp,pip->zname,zlen)) >= 0) {
	        pip->daytime = nowp->time ;
	        rs = dater_start(&pip->tmpdate,nowp,pip->zname,-1) ;
	    } /* end if (initnow) */
	} /* end block (getting some current time stuff) */

#if	CF_DEBUGS
	{
	    struct timeb	*nowp = &pip->now ;
	    debugprintf("main: initnow() zoff=%d zname=%s\n",
		nowp->timezone,pip->zname) ;
	}
#endif

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto baddatestart ;
	}

	timestr_logz(pip->daytime,pip->stamp) ;

	pip->verboselevel = 1 ;

	pip->f.logprog = OPT_LOGPROG ;
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
	pip->f.optdeliver = OPT_DELIVER ;
	pip->f.optcopy = OPT_COPY ;
	pip->f.optspambox = OPT_SPAMBOX ;

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

	if (rs >= 0) {
	    rs = paramopt_start(&akparams) ;
	    pip->open.akparams = (rs >= 0) ;
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

/* configuration file-name */
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

/* log file name */
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

/* mail spool directory */
	                case argopt_ms:
	                case argopt_md:
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
	                        cchar	*po = PO_MAILDIRS ;
	                        rs = paramopt_loads(&akparams,po,cp,cl) ;
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

/* COMAST port */
	                case argopt_cp:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl > 0)
	                            pip->portspec = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->portspec = argp ;
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

/* envelope from address */
	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->addenvfrom = argp ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* boxname for DMAILBOX */
	                    case 'm':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = procmboxes(pip,argp,argl) ;
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

/* caller-supplied protocol specification */
	                    case 'p':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->protospec = argp ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

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
	        debugprintf("main: pm=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    } else
	        debugprintf("main: pm=NONE\n") ;
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

	if (pip->portspec == NULL) pip->portspec = getenv(VARCOMSATPORT) ;

	if (pip->csfname == NULL) {
	    pip->csfname = getenv(VARCOMSATFNAME) ;
	}

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if ((rs >= 0) && (pip->deadmaildname == NULL)) {
	    if ((cp = getenv(VARDMAILDNAMEP)) != NULL) {
		rs = procmaildirdead(pip,cp,-1) ;
	    }
	}
	if ((rs >= 0) && (pip->deadmaildname == NULL)) {
	    if ((cp = getenv(VARDMAILDNAME)) != NULL) {
		rs = procmaildirdead(pip,cp,-1) ;
	    }
	}
	if ((rs >= 0) && (pip->copymaildname == NULL)) {
	    if ((cp = getenv(VARCMAILDNAME)) != NULL) {
		rs = procmaildircopy(pip,cp,-1) ;
	    }
	}

	if (pip->defbox == NULL) pip->defbox = getenv(VARMAILBOX) ;

	if (pip->to_spool <= 1) pip->to_spool = TO_SPOOL ;

	if (pip->to_msgread <= 1) pip->to_msgread = TO_MSGREAD ;

#if	CF_DEFLOGSIZE
	if (pip->logsize < 0) pip->logsize = LOGSIZE ;
#endif

	if (rs >= 0) {
	    rs = getgid_def(MAILGNAME,MAILGID) ;
	    pip->gid_mail = rs ;
	}

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = procopts(pip,&akopts,&akparams)) >= 0) {
		if (pip->lfname == NULL) pip->lfname = getenv(VARLFNAME) ;
		if (pip->lfname == NULL) pip->lfname = getenv(VARLOGFNAME) ;
		if ((rs = procstuff_begin(pip,argval)) >= 0) {
	            USERINFO	u ;
	            if ((rs = userinfo_start(&u,NULL)) >= 0) {
	                pip->uip = &u ;
	                if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	                    if ((rs = proglog_begin(pip,&u)) >= 0) {
	                        {
	                            ARGINFO	*aip = &ainfo ;
	                            BITS	*bop = &pargs ;
	                            PARAMOPT	*aop = &akparams ;
	                            cchar	*ofn = ofname ;
	                            cchar	*afn = afname ;
	                            cchar	*ifn = ifname ;
	                            rs = process(pip,aip,bop,aop,ofn,afn,ifn) ;
	                        }
	                        rs1 = proglog_end(pip) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (proglog) */
	                    rs1 = procuserinfo_end(pip) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (procuserinfo) */
	                pip->uip = NULL ;
	                rs1 = userinfo_finish(&u) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (userinfo) */
		    rs1 = procstuff_end(pip) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (procstuff) */
	    } else {
		cchar	*pn = pip->progname ;
		cchar	*fmt ;
		fmt = "%s: usage error w/ options (%d)\n" ;
		bprintf(pip->efp,fmt,pn,rs) ;
	    } /* end if (procopts) */
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: usage or initialization error (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

/* done */
	if ((! pip->f.multirecip) && (rs < 0)) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    switch (rs) {
	    case SR_NOENT:
	        ex = EX_NOUSER ;
	        if (! pip->f.quiet) {
		    fmt = "%s: recipient not found\n" ;
	            bprintf(pip->efp,fmt,pn) ;
		}
	        break ;
	    case SR_AGAIN:
	    case SR_DEADLK:
	    case SR_NOLCK:
	    case SR_TXTBSY:
	        ex = EX_TEMPFAIL ;
	        if (! pip->f.quiet) {
		    fmt = "%s: could not capture the mail lock\n" ;
	            bprintf(pip->efp,fmt,pn) ;
	        }
	        break ;
	    case SR_ACCES:
	        ex = EX_ACCESS ;
	        if (! pip->f.quiet) {
		    fmt = "%s: could not access the mail spool-file (%d)\n" ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	        }
	        break ;
	    case SR_REMOTE:
	        ex = EX_FORWARDED ;
	        if (! pip->f.quiet) {
		    fmt = "%s: mail is being forwarded\n" ;
	            bprintf(pip->efp,fmt,pn) ;
	        }
	        break ;
	    case SR_NOSPC:
	        ex = EX_NOSPACE ;
	        if (! pip->f.quiet) {
		    fmt = "%s: local file-system is out of space\n" ;
	            bprintf(pip->efp,fmt,pn) ;
	        }
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        if (! pip->f.quiet) {
		    fmt = "%s: unknown bad thing (%d)\n" ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	        }
	        break ;
	    } /* end switch */
	} else
	    ex = EX_OK ;

/* good return from program */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: vecobj_recipfins()\n") ;
#endif

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

	if (pip->open.akparams) {
	    pip->open.akparams = FALSE ;
	    paramopt_finish(&akparams) ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	locinfo_finish(lip) ;

badlocstart:
	dater_finish(&pip->tmpdate) ;

baddatestart:
	if (rs < 0) {
	    mkreport(pip,argc,argv,rs) ;
	}
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
	        cchar		*ids = "main" ;
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


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

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


#if	CF_SETPCS
static int procopts_setpcs(PROGINFO *pip,KEYOPT *kop,vecstr *setsp)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		i ;
	int		fi, oi ;
	int		kl, vl ;
	cchar		*tp, *cp ;
	cchar		*kp, *vp ;

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

	        if ((oi >= 0) && (keyopt_fetch(kop,kp,NULL,NULL) == rsn)) {
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

	        if ((oi >= 0) && (keyopt_fetch(kop,kp,NULL,NULL) == rsn)) {
	            rs = keyopt_loadvalue(kop,kp,vp,vl) ;
	        }

	        if (rs < 0) break ;
	    } /* end for (general options) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (procopts_setpcs) */
#endif /* CF_SETPCS */


/* process program options */
static int procopts(PROGINFO *pip,KEYOPT *kop,PARAMOPT *aop)
{
	int		rs = SR_OK ;
	int		cl ;
	cchar		*cp ;

	if ((cp = getenv(VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi, v ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(locopts,3,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case locopt_deadmaildir:
	                    if (vl > 0) {
				rs = procmaildirdead(pip,vp,vl) ;
	                    }
	                    break ;
	                case locopt_mbtab:
	                    if ((vl > 0) && (pip->mbfname == NULL)) {
	                        cchar	**vpp = &pip->mbfname ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case locopt_comsat:
	                    cp = "+" ;
	                    cl = 1 ;
	                    if (vl > 0) {
	                        const int	vch = MKCHAR(vp[0]) ;
	                        if (isdigitlatin(vch)) {
	                            if (cfdeci(vp,vl,&v) >= 0) {
	                                cp = (v > 0) ? "+" : "-" ;
	                            }
	                        } else {
	                            cp = vp ;
	                            cl = vl ;
	                        }
	                    }
	                    if ((rs >= 0) && (pip->csfname == NULL)) {
	                        cchar	**vpp = &pip->csfname ;
	                        rs = proginfo_setentry(pip,vpp,cp,cl) ;
	                    }
	                    break ;
	                case locopt_spam:
	                    if ((vl > 0) && (pip->spfname == NULL)) {
	                        cchar	**vpp = &pip->spfname ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case locopt_spambox:
	                    if ((vl > 0) && (pip->spambox == NULL)) {
	                        cchar	**vpp = &pip->spambox ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case locopt_boxdir:
	                    if ((vl > 0) && (pip->boxdname == NULL)) {
	                        cchar	**vpp = &pip->boxdname ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case locopt_boxname:
	                    if ((vl > 0) && (pip->defbox == NULL)) {
	                        cchar	**vpp = &pip->defbox ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case locopt_tospool:
	                    if ((vl > 0) && (pip->to_spool == 0)) {
	                        rs = cfdecti(vp,vl,&v) ;
	                        pip->to_spool = v ;
	                    }
	                    break ;
	                case locopt_tomsgread:
	                    if ((vl > 0) && (pip->to_msgread == 0)) {
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
	                    if (vl > 0) {
	                        cchar	*po = PO_MAILDIRS ;
	                        rs = paramopt_loads(aop,po,vp,vl) ;
	                    }
	                    break ;
	                case pcsopt_cluster:
	                    if ((vl > 0) && (pip->cluster == NULL)) {
	                        cchar	**vpp = &pip->cluster ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case pcsopt_pcsadmin:
	                    if ((vl > 0) && (pip->username_pcs == NULL)) {
	                        cchar	**vpp = &pip->username_pcs ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                    break ;
	                case pcsopt_logsize:
	                    if ((vl > 0) && (pip->logsize == 0)) {
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


static int process(pip,aip,bop,aop,ofn,afn,ifn)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
PARAMOPT	*aop ;
cchar		*ofn ;
cchar		*afn ;
cchar		*ifn ;
{
	int		rs ;
	int		rs1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	if ((rs = proguserlist_begin(pip)) >= 0) {
	    if ((rs = procourconf_begin(pip,aop)) >= 0) {
	        if ((rs = procloginfo(pip)) >= 0) {
	            if ((rs = proclogs_begin(pip)) >= 0) {
	                if ((rs = procmaildname_begin(pip,aop)) >= 0) {
	                    {
	                        rs = processing(pip,aip,bop,ofn,afn,ifn) ;
	                    }
	                    rs1 = procmaildname_end(pip) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (procmaildname) */
	                rs1 = proclogs_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (proclogs) */
	        } /* end if (procloginfo) */
	        rs1 = procourconf_end(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (procourconf) */
	    rs1 = proguserlist_end(pip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (proguserlist) */
	if (pip->debuglevel > 0) {
	    pn = pip->progname ;
	    fmt = "%s: process (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	}
	fmt = "process (%d)" ;
	proglog_printf(pip,fmt,rs) ;
	return rs ;
}
/* end subroutine (process) */


static int procstuff_begin(PROGINFO *pip,cchar *argval)
{
	int		rs = SR_OK ;
	if ((pip->logsize == 0) && (argval != NULL)) {
	    int		v ;
	    if ((rs = cfdecmfi(argval,-1,&v)) >= 0) {
		pip->logsize = v ;
	    }
	}
	return rs ;
}
/* end subroutine (procstuff_begin) */


static int procstuff_end(PROGINFO *pip)
{
	if (pip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (procstuff_end) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

	pip->pid = uip->pid ;
	pip->username = uip->username ;
	pip->homedname = uip->homedname ;
	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->name = uip->name ;
	pip->fullname = uip->fullname ;
	pip->mailname = uip->mailname ;
	pip->logid = uip->logid ;

	pip->uid = uip->uid ;
	pip->euid = uip->euid ;
	pip->f.setuid = (pip->uid != pip->euid) ;

	pip->gid = uip->gid ;
	pip->egid = uip->egid ;
	pip->f.setgid = (pip->gid != pip->egid) ;

	if ((rs >= 0) && (pip->progmode == progmode_dmail)) {
	    const int	nlen = (NODENAMELEN+USERNAMELEN) ;
	    cchar	*nn = pip->nodename ;
	    cchar	*un = pip->username ;
	    char	nbuf[NODENAMELEN+USERNAMELEN+1] ;
	    if ((rs = sncpy3(nbuf,nlen,nn,"!",un)) >= 0) {
	        cchar	**vpp = &pip->lockaddr ;
	        rs = proginfo_setentry(pip,vpp,nbuf,rs) ;
	    }
	}

	if (rs >= 0) {
	    if ((rs = ids_load(&pip->id)) >= 0) {
	        if ((rs = procnodecluster(pip)) >= 0) {
	            if ((rs = procuserboxes(pip)) >= 0) {
	                cchar	*un = pip->username ;
	                if ((rs = pcstrustuser(pip->pr,un)) >= 0) {
	                    pip->f.trusted = TRUE ;
	                }
	            } /* end if (procuserboxes) */
	        } /* end if (procnodecluster) */
	        if (rs < 0)
	            ids_release(&pip->id) ;
	    } /* end if (ids_load) */
	} /* end if (ok) */

	pip->uid_divert = pip->uid ;

	if ((pip->f.optlogmsg || pip->f.optlogconf)) {
	    pip->f.logprog = TRUE ;
	}

	return rs ;
}
/* end if (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = ids_release(&pip->id) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (procuserinfo_end) */


static int procourconf_begin(PROGINFO *pip,PARAMOPT *aop)
{
	int		rs ;
	if ((rs = procmaildirdead(pip,DEADMAILDNAME,-1)) >= 0) {
	    if ((rs = procmaildircopy(pip,COPYMAILDNAME,-1)) >= 0) {
	        if ((rs = procmaildirs(pip,aop)) >= 0) {
	            if ((rs = procuucpinfo(pip)) >= 0) {
	                if ((rs = procenvfromaddr(pip)) >= 0) {
	                    if ((rs = procprotospec(pip)) >= 0) {
	                        if ((rs = procportcomsat(pip)) >= 0) {
	                            if ((rs = procfindfiles(pip)) >= 0) {
					rs = procdefs(pip) ;
				    }
				}
			    }
	                }
	            }
	        }
	    }
	}
	return rs ;
}
/* end subroutine (procourconf_begin) */


static int procourconf_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	return rs ;
}
/* end subroutine (procourconf_begin) */


static int proclogs_begin(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if ((rs = proglogenv_begin(pip)) >= 0) {
	    if ((rs = proglogzone_begin(pip)) >= 0) {
		rs = proclogs_log(pip) ;
	    }
	    if (rs < 0)
	        proglogenv_end(pip) ;
	}
	return rs ;
}
/* end subroutine (proclogs_begin) */


static int proclogs_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	rs1 = proglogenv_end(pip) ;
	if (rs >= 0) rs = rs1 ;
	rs1 = proglogzone_end(pip) ;
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (proclogs_end) */


static int proclogs_log(PROGINFO *pip)
{
	const int	dl = pip->debuglevel ;
	int		rs = SR_OK ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	cchar		*fn ;
	if (pip->zfname != NULL) {
	    fn = pip->zfname ;
	    fmt = "zl=%s"  ;
	    proglog_printf(pip,fmt,fn) ;
	}
	if (pip->csfname != NULL) {
	    fn = pip->csfname ;
	    if (dl > 0) {
	        fmt = "%s: cs=%s\n"  ;
	        bprintf(pip->efp,fmt,pn,fn) ;
	    }
	    fmt = "cs=%s"  ;
	    proglog_printf(pip,fmt,fn) ;
	}
	if (pip->mbfname != NULL) {
	    fn = pip->mbfname ;
	    if (dl > 0) {
	        fmt = "%s: mb=%s\n"  ;
	        bprintf(pip->efp,fmt,pn,fn) ;
	    }
	    fmt = "mb=%s"  ;
	    proglog_printf(pip,fmt,fn) ;
	}
	if (pip->spfname != NULL) {
	    fn = pip->spfname ;
	    if (dl > 0) {
	        fmt = "%s: sp=%s\n"  ;
	        bprintf(pip->efp,fmt,pn,fn) ;
	    }
	    fmt = "sp=%s"  ;
	    proglog_printf(pip,fmt,fn) ;
	}
	return rs ;
}
/* end subroutine (proclogs_log) */


static int proctmp_begin(PROGINFO *pip,bfile *tfp,char *tbuf)
{
	int		rs ;
	cchar		*n = "dmailXXXXXXXXX" ;
	char		template[MAXPATHLEN+1] ;
	if ((rs = mkpath2(template,pip->tmpdname,n)) >= 0) {
	    if ((rs = mktmpfile(tbuf,0600,template)) >= 0) {
	        rs = bopen(tfp,tbuf,"rwc",0600) ;
	        if (rs < 0)
	            uc_unlink(tbuf) ;
	    }
	}
	return rs ;
}
/* end subroutine (proctmp_begin) */


static int proctmp_end(PROGINFO *pip,bfile *tfp,char *tbuf)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip == NULL) return SR_FAULT ;
	rs1 = bclose(tfp) ;
	if (rs >= 0) rs = rs1 ;
	if (tbuf[0] != '\0') {
	    rs1 = uc_unlink(tbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    tbuf[0] = '\0' ;
	}
	return rs ;
}
/* end subroutine (proctmp_end) */


static int procmsgid_begin(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		f = FALSE ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/procmsgid_begin: ent f=%u\n",pip->f.optlogmsgid) ;
#endif
	if (pip->f.optlogmsgid) {
	    LOCINFO	*lip = pip->lip ;
	    cchar	*vd = VARDNAME ;
	    cchar	*sn = pip->searchname ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath3(tbuf,pip->pr,vd,sn)) >= 0) {
	        MSGID		*mip = &lip->mids ;
	        const int	rsn = SR_ACCESS ;
	        const int	n = MAXMSGID ;
#if	CF_DEBUG
		if (DEBUGLEVEL(5))
	    	    debugprintf("main/procmsgid_begin: tbuf=%s\n",tbuf) ;
#endif
	        if ((rs = msgid_open(mip,tbuf,O_RDWR,0660,n)) == rsn) {
#if	CF_DEBUG
		    if (DEBUGLEVEL(5))
	    	        debugprintf("main/procmsgid_begin: no-access\n") ;
#endif
	            rs = msgid_open(mip,tbuf,O_RDONLY,0660,n) ;
	        }
	        if (rs >= 0) {
	            lip->open.mids = TRUE ;
		    f = TRUE ;
	        } else if (isNotPresent(rs)) {
#if	CF_DEBUG
		    if (DEBUGLEVEL(5)) {
	    	        debugprintf("main/procmsgid_begin: "
				"no-present (%d)\n", rs) ;
		    }
#endif
	            rs = SR_OK ;
	        }
	    } /* end if (mkpath) */
	} /* end if (optlogmsgid) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/procmsgid_begin: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procmsgid_begin) */


static int procmsgid_end(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.mids) {
	    lip->open.mids = FALSE ;
	    rs1 = msgid_close(&lip->mids) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (procmsgid_end) */


static int procmsgid_update(PROGINFO *pip,MSGINFO *mop,cchar *r)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("main/procmsgid_update: ent open=%u\n",lip->open.mids) ;
	debugprintf("main/procmsgid_update: r=%s\n",r) ;
	}
#endif
	if (lip->open.mids) {
	    MSGID_KEY		midkey ;
	    MSGID_ENT		mident ;
	    const time_t	dt = pip->daytime ;
	    cchar		*mid = mop->h_messageid ;

	    memset(&midkey,0,sizeof(MSGID_KEY)) ;
	    midkey.recip = r ;
	    midkey.reciplen = -1 ;
	    mident.count = 0 ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("main/procmsgid_update: f_mid=%u\n",
			mop->f.messageid) ;
	        debugprintf("main/procmsgid_update: mid=%s\n",
			mop->h_messageid) ;
	    }
#endif
	        if ((mid != NULL) && (mid[0] != '\0')) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procmsgid_update: messageid=%s\n",mid) ;
#endif

	        midkey.mtime = mop->mtime ;
	        midkey.from = mop->h_from ;
	        midkey.mid = mid ;

	        rs = msgid_update(&lip->mids,dt,&midkey,&mident) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("main/procmsgid_update: "
			"msgid_update() rs=%d\n", rs) ;
		}
#endif

	        if (rs == SR_ACCESS) {
	            rs = msgid_match(&lip->mids,dt,&midkey,&mident) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                debugprintf("main/procmsgid_update: "
				"msgid_match() rs=%d\n",rs) ;
		    }
#endif
	        }

		if (rs >= 0) c = mident.count ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procmsgid_update: rs=%d c=%u\n",rs,c) ;
#endif

		} /* end if (non-NULL) */
	} /* end if (MSGID-open) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procmsgid_update: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmsgid_update) */


static int procnodecluster(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->cluster == NULL) {
	    cchar	*tp ;
	    if ((tp = strchr(pip->domainname,'.')) != NULL) {
	        int	cl = (tp - pip->domainname) ;
		cchar	**vpp = &pip->cluster ;
	        cchar	*cp = pip->domainname ;
	        rs = proginfo_setentry(pip,vpp,cp,cl) ;
	    }
	} /* end if (cluster) */
	return rs ;
}
/* end subroutine (procnodecluster) */


static int procuucpinfo(PROGINFO *pip)
{
	pip->uu_machine = getenv(VARUUMACHINE) ;
	pip->uu_user = getenv(VARUUUSER) ;
	return SR_OK ;
}
/* end subroutine (procuucpinfo) */


static int procenvfromaddr(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->addenvfrom != NULL) {
	    const int	elen = strlen(pip->addenvfrom) ;
	    char	*ebuf ;
	    if ((rs = uc_malloc((elen+1),&ebuf)) >= 0) {
	        cchar	*envfrom = pip->addenvfrom ;
	        if ((rs = mkbestaddr(ebuf,elen,envfrom,-1)) >= 0) {
	            cchar	**vpp = &pip->envfrom ;
	            rs = proginfo_setentry(pip,vpp,ebuf,rs) ;
		}
		uc_free(ebuf) ;
	    } /* end if (m-a-f) */
	} else if (pip->envfrom == NULL) {
	    if (pip->f.trusted && (pip->uu_user != NULL)) {
	        pip->envfrom = pip->uu_user ;
	    } else {
	        pip->envfrom = pip->username ;
	    } /* end if */
	} /* end if */
	return rs ;
}
/* end subroutine (procenvfromaddr) */


/* find the mail spool directory, as dynamically as possible */
static int procmaildname_begin(PROGINFO *pip,PARAMOPT *aop)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	if (pip->progmode == progmode_dmail) {
	    if ((rs = vecstr_start(&pip->maildirs,1,0)) >= 0) {
	        PARAMOPT_CUR	cur ;
	        int		vl ;
	        cchar		*vp ;
	        if ((rs = paramopt_curbegin(aop,&cur)) >= 0) {
	            cchar	*po = PO_MAILDIRS ;
	            while ((vl = paramopt_enumvalues(aop,po,&cur,&vp)) >= 0) {
	                rs = procmaildname_add(pip,vp,vl) ;
	                c += rs ;
	                if (rs < 0) break ;
	            } /* end while */
	            rs1 = paramopt_curend(aop,&cur) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (paramopt-cur) */
	        if ((rs >= 0) && (c == 0)) {
	            vl = -1 ;
	            vp = MAILDNAME ;
	            rs = procmaildname_add(pip,vp,vl) ;
	            c += rs ;
	        }
	        if (rs < 0)
	            vecstr_finish(&pip->maildirs) ;
	    } /* end if (vecstr_start) */
	} /* end if (progmode_dmail) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmaildname_begin) */


static int procmaildname_add(PROGINFO *pip,cchar *vp,int vl)
{
	NULSTR		md ;
	int		rs ;
	int		c = 0 ;
	cchar		*dname ;
	if ((rs = nulstr_start(&md,vp,vl,&dname)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = uc_stat(dname,&sb)) >= 0) {
	        if (S_ISDIR(sb.st_mode)) {
		    IDS		*idp = &pip->id ;
	            if ((rs = sperm(idp,&sb,W_OK)) >= 0) {
	                rs = vecstr_adduniq(&pip->maildirs,vp,vl) ;
	                if (rs < INT_MAX) c += 1 ;
	                if (rs > 0) {
	                    proglog_printf(pip,"maildir=%s\n",dname) ;
	                }
	            } else if (isNotAccess(rs))
	                rs = SR_OK ;
	        } /* end if (directory) */
	    } else if (isNotPresent(rs)) {
		if (pip->debuglevel > 0) {
	            cchar	*fmt = "maildir=%s not-found (%d)\n" ;
	            proglog_printf(pip,fmt,dname,rs) ;
		}
	        rs = SR_OK ;
	    }
	} /* end if (nulstr) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmaildname_add) */


static int procmaildname_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->progmode == progmode_dmail) {
	    rs1 = vecstr_finish(&pip->maildirs) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (procmaildname_end) */


static int procprotospec(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if ((pip->protospec == NULL) && (pip->uu_machine != NULL)) {
	    char	pbuf[PSBUFLEN+1] ;
	    if ((rs = snscs(pbuf,BUFLEN,"uucp",pip->uu_machine)) >= 0) {
	        cchar	**vpp = &pip->protospec ;
	        rs = proginfo_setentry(pip,vpp,pbuf,rs) ;
	    }
	}

	if (pip->protospec != NULL) {
	    cchar	*ps = pip->protospec ;
	    if (strcasecmp(ps,PROTOSPEC_POSTFIX) == 0) {
	        pip->f.trusted = TRUE ;
	    }
	}

	return rs ;
}
/* end subroutine (procprotospec) */


static int procuserboxes(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->progmode == progmode_dmailbox) {

	    if (pip->boxdname == NULL) pip->boxdname = DEFMAILFOLDER ;

	    if (pip->defbox == NULL) pip->defbox = DEFBOXNAME ;

	} /* end if (progmode-dmail) */
	return rs ;
}
/* end subroutine (procuserboxes) */


static int procloginfo(PROGINFO *pip)
{
	const int	dl = pip->debuglevel ;
	int		rs = SR_OK ;
	if (pip->f.logprog) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    pip->f.logconf = pip->f.optlogconf ;
	    pip->f.logmsg = pip->f.optlogmsg ;
	    if (pip->f.optlogconf) {
	        proglog_printf(pip,"pr=%s\n",pip->pr) ;
	    }
	    if (pip->protospec != NULL) {
		if (dl > 0) {
		    fmt = "%s: proto=%s\n" ;
		    bprintf(pip->efp,fmt,pn,pip->protospec) ;
		}
	        fmt = "proto=%s" ;
	        proglog_printf(pip,fmt,pip->protospec) ;
	    }
	    if (pip->uu_machine != NULL) {
		if (dl > 0) {
		    fmt = "%s: umachine=%s\n" ;
		    bprintf(pip->efp,fmt,pn,pip->uu_machine) ;
		}
	        fmt = "uu_machine=%s" ;
	        proglog_printf(pip,fmt,pip->uu_machine) ;
	    }
	    if (pip->uu_user != NULL) {
		if (dl > 0) {
		    fmt = "%s: uuser=%s\n" ;
		    bprintf(pip->efp,fmt,pn,pip->uu_user) ;
		}
	        fmt = "uu_user=%s" ;
	        proglog_printf(pip,fmt,pip->uu_user) ;
	    }
	    if (pip->copymaildname != NULL) {
		cchar	*sp = pip->copymaildname ;
		if (dl > 0) {
		    fmt = "%s: maildir-copy=%s\n" ;
		    bprintf(pip->efp,fmt,pn,sp) ;
		}
	        fmt = "maildir-copy=%s" ;
	        proglog_printf(pip,fmt,sp) ;
	    }
	    if (pip->deadmaildname != NULL) {
		cchar	*sp = pip->deadmaildname ;
		if (dl > 0) {
		    fmt = "%s: maildir-dead=%s\n" ;
		    bprintf(pip->efp,fmt,pn,sp) ;
		}
	        fmt = "maildir-dead=%s" ;
	        proglog_printf(pip,fmt,sp) ;
	    }
	}
	return rs ;
}
/* end subroutine (procloginfo) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *app,VECOBJ *rlp,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*cp ;

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

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	    if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

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
	                "%s: inaccessible argument-list (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",
	                pip->progname,afn) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pip->nrecips == 0)) {
	    cp = pip->username ;
	    rs = loadrecip(pip,rlp,cp,-1) ;
	    c += rs ;
	} /* end if */

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


#if	CF_LOGRECIP
static int logrecip(PROGINFO *pip,cchar recip[],struct passwd *pwp)
{
	int		rs = SR_OK ;
	if (pip->open.logmsg) {
	    const int	nlen = REALNAMELEN ;
	    cchar	*fmt = "load recip=%s" ;
	    char	nbuf[REALNAMELEN + 1] = { 0 } ;
	    if (pwp != NULL) {
	        rs = mkrecipname(nbuf,nlen,pwp->pw_gecos) ;
	        if ((rs >= 0) && (rs <= 50)) {
	            fmt = "load recip=%s (%s)" ;
	        }
	    } /* end if (was a real user) */
	    rs = proglog_printf(pip,fmt,recip,nbuf) ;
	} /* end if (logmsg) */
	return rs ;
}
/* end subroutine (logrecip) */
#endif /* CF_LOGRECIP */


/* extract the real name from a GECOS name */
static int mkrecipname(char rbuf[],int rlen,cchar gecos[])
{
	const int	glen = GNAMELEN ;
	int		rs ;
	char		gbuf[GNAMELEN + 1] ;
	rbuf[0] = '\0' ;
	if ((rs = mkgecosname(gbuf,glen,gecos)) > 0) {
	    rs = mkrealname(rbuf,rlen,gbuf,rs) ;
	}
	return rs ;
}
/* end subroutine (mkrecipname) */


static int loadrecips(PROGINFO *pip,VECOBJ *rlp,cchar sp[],int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;
	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
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


static int loadrecip(PROGINFO *pip,VECOBJ *nlp,cchar np[],int nl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
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
	} else {
	    const int	nch = MKCHAR(np[0]) ;
	    cchar	*tp ;
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
	                    rs = vecobj_recipadd(nlp,ub,rs) ;
	                    c += rs ;
	                    pip->nrecips += rs ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(5))
	                        debugprintf("main/loadrecip: "
	                            "vecobj_recipadd() rs=%d\n",rs) ;
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
	            rs1 = locinfo_rncurend(lip,&rnc) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (srncursor) */
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("main/loadrecip: rncur-out rs=%d\n",rs) ;
#endif
	    } else if (nch == MKCHAR('¡')) {
	        LOCINFO_GMCUR	gc ;
	        cchar		*gnp = (np+1) ;
	        const int	gnl = (nl-1) ;
	        if ((rs = locinfo_gmcurbegin(lip,&gc)) >= 0) {
	            if ((rs = locinfo_gmlook(lip,&gc,gnp,gnl)) >= 0) {
	                const int	ul = USERNAMELEN ;
	                char		ub[USERNAMELEN+1] ;
	                while ((rs = locinfo_gmread(lip,&gc,ub,ul)) >= 0) {
	                    rs = vecobj_recipadd(nlp,ub,rs) ;
	                    c += rs ;
	                    pip->nrecips += rs ;
	                    if (rs < 0) break ;
	                } /* end while */
	            } /* end if */
	            if (rs == SR_NOTFOUND) rs = SR_OK ;
	            rs1 = locinfo_gmcurend(lip,&gc) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (gmcursor) */
	    } else {
	        if (nch == '!') {
	            np += 1 ;
	            nl -= 1 ;
	        }
		if (nl == 0) {
	        	np = pip->username ;
			nl = -1 ;
		}
	        if ((rs >= 0) && (nl > 0)) {
	            rs = vecobj_recipadd(nlp,np,nl) ;
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


/* ARGSUSED */
static int processing(pip,aip,bop,ofn,afn,ifn)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
cchar		*ofn ;
cchar		*afn ;
cchar		*ifn ;
{
	vecobj		recips ;
	const int	size = sizeof(RECIP) ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/processing: ent\n") ;
#endif

	if ((rs = vecobj_start(&recips,size,1,0)) >= 0) {
	    if ((rs = procargs(pip,aip,bop,&recips,afn)) >= 0) {
	        bfile	tfile, *tfp = &tfile ;
	        char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = proctmp_begin(pip,tfp,tbuf)) >= 0) {
	            vecobj	info ;
	            const int	size = sizeof(MSGINFO) ;
	            if ((rs = vecobj_start(&info,size,1,0)) >= 0) {
	                if ((rs = procin(pip,&info,tfp,&recips,ifn)) >= 0) {
#if	CF_DEBUG
	                    if (DEBUGLEVEL(5))
	                        debugprintf("main/processing: "
					"procin() rs=%d\n",rs) ;
#endif
	                    if ((rs = procmsgid_begin(pip)) >= 0) {
	                        {
	                            VECOBJ	*mip = &info ;
	                            VECOBJ	*rip = &recips ;
	                            rs = processings(pip,mip,rip,tbuf) ;
	                        }
	                        rs1 = procmsgid_end(pip) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (procmsgid) */
#if	CF_DEBUG
	                    if (DEBUGLEVEL(5))
	                        debugprintf("main/processing: "
					"procmsgid-out rs=%d\n", rs) ;
#endif
	                } /* end if (procin) */
	                rs1 = vecobj_finish(&info) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (vecobj) */
#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("main/processing: vecobj-out rs=%d\n",rs) ;
#endif
	            rs1 = proctmp_end(pip,tfp,tbuf) ;
	            if (rs >= 0) rs = rs1 ;
	        } else {
	            cchar	*fmt ;
	            fmt = "%s: could not create-open tempory file (%d)\n" ;
	            bprintf(pip->efp,fmt,pip->progname,rs) ;
	        }
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("main/processing: proctmp-out rs=%d\n",rs) ;
#endif
	        rs1 = vecobj_recipfins(&recips) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (procargs) */
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("main/processing: procargs-out rs=%d\n",rs) ;
#endif
	    rs1 = vecobj_finish(&recips) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecobj) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/processing: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (processing) */


static int processings(PROGINFO *pip,VECOBJ *mip,VECOBJ *rip,cchar *tfn)
{
	int		rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/processings: ent tfn=%s\n",tfn) ;
#endif
	if ((rs = procspamsetup(pip,mip)) >= 0) {
	    const mode_t	om = 0666 ;
	    const int		of = O_RDONLY ;
	    if ((rs = uc_open(tfn,of,om)) >= 0) {
	        const int	tfd = rs ;
	        if ((rs = procrecips(pip,mip,rip,tfd)) >= 0) {
	            if (pip->progmode == progmode_dmail) {
#if	CF_SPAMBOX
	                if (pip->spambox != NULL) {
	                    rs = procspambox(pip,mip,rip,tfd) ;
	                }
#endif
			if (rs >= 0) {
			    rs = progcomsat(pip,rip) ;
#if	CF_DEBUGN
	nprintf(NDF,"main/processings: progcomsat() rs=%d\n",rs) ;
#endif
			}
	            } /* end if (progmode-dmail) */
	        } /* end if (procrecips) */
	        if (pip->debuglevel > 0) {
		    cchar	*pn = pip->progname ;
		    cchar	*fmt ;
		    fmt = "%s: recipients processed=%u delivered=%u\n" ;
	            bprintf(pip->efp,fmt,pn,pip->c_processed,pip->c_delivered) ;
	        }
	        if (pip->f.logmsg) {
		    cchar	*fmt ;
		    fmt = "recipients processed=%u delivered=%u\n" ;
	            proglog_printf(pip,fmt,pip->c_processed,pip->c_delivered) ;
	        }
	        u_close(tfd) ;
	    } /* end if (file) */
	} /* end if (procspamsetup) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/processings: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (processings) */


static int procin(PROGINFO *pip,vecobj *mip,bfile *tfp,vecobj *rip,cchar *ifn)
{
	bfile		infile, *ifp = &infile ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procin: ent ifn=%s\n",ifn) ;
#endif

	if ((ifn == NULL) || (ifn[0] == '\0') || (ifn[0] == '-'))
	    ifn = BFILE_STDIN ;

	if ((rs = bopen(ifp,ifn,"r",0666)) >= 0) {
	    USERINFO	*uip = pip->uip ;
	    if ((rs = prognamecache_begin(pip,uip)) >= 0) {

#if	CF_PROGMSGS
#if	CF_DEBUG && CF_DEBUGRECIPS
	        if (DEBUGLEVEL(4)) {
	            debugprintf("main/procin: progmsgs()\n") ;
	            debugrecips(pip,&recips) ;
	        }
#endif

	        rs = progmsgs(pip,ifp,tfp,mip,rip) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procin: progmsgs() rs=%d\n",rs) ;
#endif
#endif /* CF_PROGMSGS */

	        rs1 = prognamecache_end(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (prognamecache) */
	    rs1 = bclose(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    bprintf(pip->efp,"%s: inaccessible input (%d)\n",
	        pip->progname,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procin) */


static int procfindfiles(PROGINFO *pip)
{
	VECSTR		svars ;
	int		rs ;
	int		rs1 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procfindfiles: ent\n") ;
#endif
	if ((rs = vecstr_start(&svars,6,0)) >= 0) {
	    if ((rs = procfindsched(pip,&svars)) >= 0) {
	        if ((rs = procfindcomsat(pip,&svars)) >= 0) {
	            if ((rs = procfindspam(pip,&svars)) >= 0) {
	                rs = procfindmbtab(pip,&svars) ;
	            } /* end if (spam) */
	        } /* end if (comsat) */
	    } /* end if (sched-load) */
	    rs1 = vecstr_finish(&svars) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (svars) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procfindfiles: ret rs=%d\n",rs) ;
	    if (pip->csfname != NULL)
	    debugprintf("main/procfindfiles: ret csfn=%s\n",pip->csfname) ;
	}
#endif
	return rs ;
}
/* end subroutine (procfindfiles) */


/* do we have a COMSAT file? */
/* ARGSUSED */
static int procfindcomsat(PROGINFO *pip,vecstr *slp)
{
	int		rs = SR_OK ;
	if (pip->progmode == progmode_dmail) {
	    cchar	*fn = NULL ;
	    if ((pip->csfname == NULL) || (pip->csfname[0] == '+')) {
	        fn = COMSATFNAME ;
	    } else if (pip->csfname[0] != '-') {
	        fn = pip->csfname ;
	    }
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procfindcomsat: beginning fn=%s\n",fn) ;
#endif
	    if ((rs >= 0) && (fn != NULL)) {
	        int	flen = MAXPATHLEN ;
	        int	fl = -1 ;
	        char	fbuf[MAXPATHLEN+1] ;
	        if ((rs = progexpand(pip,fbuf,flen,fn,-1)) >= 0) {
	            fl = rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procfindcomsat: exp fl=%d fbuf=%s\n",fl,fbuf) ;
#endif
	            if (fbuf[0] != '/') {
	        	char	tbuf[MAXPATHLEN+1] ;
	                if ((rs = mkpath2(tbuf,pip->pr,fbuf)) >= 0) {
	                    rs = mkpath1w(fbuf,tbuf,rs) ;
			    fl = rs ;
	                }
	            }
	        } /* end if (progexpand) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procfindcomsat: mid fl=%d fbuf=%s\n",fl,fbuf) ;
#endif
	        if (rs >= 0) {
	            if ((rs = perm(fbuf,-1,-1,NULL,R_OK)) >= 0) {
	                cchar	**vpp = &pip->csfname ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procfindcomsat: perm() rs=%d\n",rs) ;
#endif
	                if ((rs = proginfo_setentry(pip,vpp,fbuf,fl)) >= 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("main/procfindcomsat: proginfo_setent() rs=%d\n",rs) ;
	debugprintf("main/procfindcomsat: csfn=%s\n",pip->csfname) ;
	}
#endif
	                    pip->f.comsat = TRUE ;
	                }
	            } else if (isNotAccess(rs)) {
	                rs = SR_OK ;
	            }
	        } /* end if (ok) */
	    } /* end if (non-null) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("main/procfindcomsat: ending rs=%d\n",rs) ;
	    if (pip->csfname != NULL)
	    debugprintf("main/procfindcomsat: ret csfn=%s\n",pip->csfname) ;
	}
#endif
	} /* end if (progmode-dmail) */
	return rs ;
}
/* end subroutine (procfindcomsat) */


/* do we have a spam filter file? */
/* ARGSUSED */
static int procfindspam(PROGINFO *pip,vecstr *slp)
{
	int		rs = SR_OK ;
	cchar		*fn = NULL ;
	if ((pip->spfname == NULL) || (pip->spfname[0] == '+')) {
	    fn = SPAMFNAME ;
	} else if (pip->spfname[0] != '-') {
	    fn = pip->spfname ;
	}
	if ((rs >= 0) && (fn != NULL)) {
	    int		flen = MAXPATHLEN ;
	    int		fl = -1 ;
	    char	fbuf[MAXPATHLEN+1] ;
	    if ((rs = progexpand(pip,fbuf,flen,fn,-1)) >= 0) {
	        fl = rs ;
	        if (fbuf[0] != '/') {
	            char	tbuf[MAXPATHLEN+1] ;
	            if ((rs = mkpath2(tbuf,pip->pr,fbuf)) >= 0) {
	                rs = mkpath1w(fbuf,tbuf,rs) ;
			fl = rs ;
	            }
	        }
	    } /* end if (progexpand) */
	    if (rs >= 0) {
	        if ((rs = perm(fbuf,-1,-1,NULL,R_OK)) >= 0) {
	            cchar	**vpp = &pip->spfname ;
	            if ((rs = proginfo_setentry(pip,vpp,fbuf,fl)) >= 0) {
	                pip->f.spam = TRUE ;
	            }
	        } else if (isNotAccess(rs)) {
	            rs = SR_OK ;
	        }
	    } /* end if (ok) */
	} /* end if (non-null) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("main/procfindspam: ret rs=%d\n",rs) ;
	    if (pip->spfname != NULL)
	    debugprintf("main/procfindspam: ret spamfn=%s\n",pip->spfname) ;
	}
#endif
	return rs ;
}
/* end subroutine (procfindspam) */


/* do we have a MBTAB file? */
/* ARGSUSED */
static int procfindmbtab(PROGINFO *pip,vecstr *slp)
{
	int		rs = SR_OK ;
	cchar		*fn = NULL ;
	if ((pip->mbfname == NULL) || (pip->mbfname[0] == '+')) {
	    fn = MBFNAME ;
	} else if (pip->mbfname[0] != '-') {
	    fn = pip->mbfname ;
	}
	if ((rs >= 0) && (fn != NULL)) {
	    int		flen = MAXPATHLEN ;
	    int		fl = -1 ;
	    char	fbuf[MAXPATHLEN+1] ;
	    if ((rs = progexpand(pip,fbuf,flen,fn,-1)) >= 0) {
	        fl = rs ;
	        if (fbuf[0] != '/') {
	    	    char	tbuf[MAXPATHLEN+1] ;
	            if ((rs = mkpath2(tbuf,pip->pr,fbuf)) >= 0) {
	                rs = mkpath1w(fbuf,tbuf,rs) ;
			fl = rs ;
	            }
	        }
	    } /* end if (progexpand) */
	    if (rs >= 0) {
	        if ((rs = perm(fbuf,-1,-1,NULL,R_OK)) >= 0) {
	            cchar	**vpp = &pip->mbfname ;
	            if ((rs = proginfo_setentry(pip,vpp,fbuf,fl)) >= 0) {
	                pip->f.mbtab = TRUE ;
	            }
	        } else if (isNotAccess(rs)) {
	            rs = SR_OK ;
	        }
	    } /* end if (ok) */
	} /* end if (non-null) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("main/procfindmbtab: ret rs=%d\n",rs) ;
	    if (pip->spfname != NULL)
	    debugprintf("main/procfindmbtab: ret mbfn=%s\n",pip->mbfname) ;
	}
#endif
	return rs ;
}
/* end subroutine (procfindmbtab) */


static int procdefs(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->spambox == NULL) pip->spambox = SPAMUSER ;
	if (pip->defbox != NULL) {
	    if (pip->defbox[0] != '\0') {
		LOCINFO	*lip = pip->lip ;
		if ((rs = locinfo_mboxcount(lip)) == 0) {
		    rs = procmboxes(pip,pip->defbox,-1) ;
		}
	    }
	}
	return rs ;
}
/* end subroutine (procdefs) */


static int procspamsetup(PROGINFO *pip,vecobj *ilp)
{
	MSGINFO		*mop ;
	int		rs = SR_OK ;
	int		i ;
	if (pip == NULL) return SR_FAULT ;
	for (i = 0 ; vecobj_get(ilp,i,&mop) >= 0 ; i += 1) {
	    if (mop != NULL) {
	        mop->f.spamdeliver = mop->f.spam ;
	    }
	} /* end for */
	return rs ;
}
/* end subroutine (procspamsetup) */


#if	CF_SPAMBOX

static int procspambox(PROGINFO *pip,vecobj *ilp,vecobj *rlp,int tfd)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("main/procspambox: ent\n") ;
#endif

	if (lip == NULL) return SR_FAULT ;

	if ((pip->progmode == progmode_dmail) && pip->f.optnospam) {
	    if ((pip->spambox != NULL) && (pip->spambox[0] != '\0')) {
	        int	i ;
	        int	f = FALSE ;
	        cchar	*r = NULL ;

	        {
	            RECIP		*orp ;
	            for (i = 0 ; (rs1 = vecobj_get(rlp,i,&orp)) >= 0 ; i += 1) {
	                if (orp != NULL) {
	                    r = orp->recipient ;
	                    f = recip_match(orp,pip->spambox,-1) ;
	                    if (f) break ;
	                }
	            } /* end for */
	        } /* end block */

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("main/procspambox: mid rs=%d f=%u\n",rs,f) ;
#endif

	        if ((rs >= 0) && (! f) && (r != NULL)) {
	            RECIP	recip, *rp = &recip ;
	            if ((rs = recip_start(rp,pip->spambox,-1)) >= 0) {
	                if ((rs = procrecipvalid(pip,rp)) >= 0) {
	                    if ((rs = procrecip_defmaildir(pip,rp)) >= 0) {
			        if ((rs = procspamboxing(pip,ilp,rp)) > 0) {
				    c = rs ;
	                            if (pip->f.optdeliver) {
	                                rs = progdeliver(pip,tfd,rp) ;
	                            }
	                        } /* end if (delivery) */
		            } /* end if (procrecip_defmaildir) */
		        } /* end if (procrecipvalid) */
	                rs1 = recip_finish(rp) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (recip) */
	        } /* end if (go) */

	    } /* end if (enabled) */
	} /* end if (progmode-dmail) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("main/procspambox: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspambox) */


static int procspamboxing(PROGINFO *pip,vecobj *ilp,RECIP *rp)
{
	MSGINFO		*mop ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	for (i = 0 ; vecobj_get(ilp,i,&mop) >= 0 ; i += 1) {
	    if ((mop != NULL) && mop->f.spamdeliver) {
		rs = procspamprogboxer(pip,rp,mop) ;
		c += rs ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspamboxing) */


static int procspamprogboxer(PROGINFO *pip,RECIP *rp,MSGINFO *mop)
{
	int		rs ;
	int		c = 0 ;
	cchar		*r = rp->recipient ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procspamprogboxer: moff=%u mlen=%u\n",
	                        mop->moff,mop->mlen) ;
#endif

	if ((rs = procmsgid_update(pip,mop,r)) == 0) {
	    c += 1 ;
	    rs = recip_mo(rp,mop->moff,mop->mlen) ;
	} /* end if (procmsgid_update) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspamprogboxer) */

#endif /* CF_SPAMBOX */


static int procrecips(PROGINFO *pip,vecobj *mip,vecobj *rlp,int tfd)
{
	int		rs ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrecips: ent\n") ;
#endif

	if ((rs = procrecip_addropen(pip)) >= 0) {
	    RECIP	*rp ;
	    int		i ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procrecips: for-before rs=%d\n",rs) ;
#endif
	    for (i = 0 ; vecobj_get(rlp,i,&rp) >= 0 ; i += 1) {
	        int	f_failed = FALSE ;
	        if (rp != NULL) {
	            cchar	*r = rp->recipient ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procrecips: recip=%s\n",r) ;
#endif

	            if (pip->debuglevel > 0) {
	                bprintf(pip->efp, "%s: recip=%s\n",pn,r) ;
	            }

	            if (pip->f.logmsg) {
	                proglog_printf(pip,"recip=%s",r) ;
	            }

	            if ((rs = procrecipvalid(pip,rp)) > 0) {
	                if ((rs = procrecip_defmaildir(pip,rp)) >= 0) {
	                    if ((rs = procrecip(pip,mip,rp,tfd)) >= 0) {
	                        if (rp->n > 0) pip->c_delivered += 1 ;
			    } else if (rs == 0) {
	                        f_failed = TRUE ;
			    }
	                } /* end if (procrecip_defmaildir) */
#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("main/procrecips: "
	                        "procrecip_defmaildir out rs=%d\n",rs) ;
#endif
	            } else if (rs == 0) {
	                f_failed = TRUE ;
	                recip_ds(rp,SR_NOTFOUND) ;
	                if ((pip->debuglevel > 0) && (! pip->f.quiet)) {
	                    fmt = "%s: NOTFOUND recip=%s\n" ;
	                    bprintf(pip->efp,fmt,pn,r) ;
	                } /* end if */
	                rs = proglog_printf(pip,"NOTFOUND recip=%s\n",r) ;
	            } /* end if (procrecipvalid) */

	        } /* end if (non-null) */
	        if (f_failed && (! pip->f.multirecip)) break ;
	        if (rs < 0) break ;
	    } /* end for (looping through recipients) */
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procrecips: for-after rs=%d\n",rs) ;
#endif
	    rs1 = procrecip_addrclose(pip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (procrecip) */

#if	CF_DEBUGN
	nprintf(NDF,"main/procecips: ret rs=%d\n",rs) ;
#endif
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrecips: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procrecips) */


static int procrecip(PROGINFO *pip,vecobj *mip,RECIP *rp,int tfd)
{
	LOCINFO		*lip = pip->lip ;
	LOOKADDR_USER	lcur ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*r = rp->recipient ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrecip: ent r=%s\n",r) ;
#endif
#if	CF_DEBUGN
	nprintf(NDF,"main/procecip: ent\n") ;
#endif

	if (pip->progmode == progmode_dmail) {
	    cchar	*md = rp->maildname ;
	    if (pip->debuglevel > 0) {
		bprintf(pip->efp, "%s: md=%s\n",pn,md) ;
	    }
	    proglog_printf(pip,"  md=%s",md) ;
	}

	if ((rs = procrecip_addrbegin(pip,&lcur,r)) >= 0) {
	    MSGINFO	*mop ;
	    MSGID_KEY	midkey ;
	    int		mn ;

	    memset(&midkey,0,sizeof(MSGID_KEY)) ;
	    midkey.recip = rp->recipient ;
	    midkey.reciplen = -1 ;

	    for (mn = 0 ; vecobj_get(mip,mn,&mop) >= 0 ; mn += 1) {
	        if (mop == NULL) continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procrecip: msg=%u off=%u mlen=%u\n",
	                mn,mop->moff,mop->mlen) ;
#endif

	        if ((rs = procmsgid_update(pip,mop,r)) >= 0) {
	            int	f_repeat = rs ;
	            int	f_blacklist = FALSE ;
	            int	f_deliver = (rs == 0) ;

#if	CF_DEBUGN
	nprintf(NDF,"main/procecip: update() rs=%d\n",rs) ;
#endif

/* repeat message-id? */

	            if (f_deliver && pip->f.optnorepeat && f_repeat)
	                f_deliver = FALSE ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                debugprintf("main/procrecip: repeat deliver=%u\n",
	                    f_deliver) ;
	                debugprintf("main/procrecip: optnospam=%u\n",
			    pip->f.optnospam) ;
	                debugprintf("main/procrecip: f_spam=%u\n",
			    mop->f.spam) ;
		    }
#endif /* CF_DEBUG */

/* is this address in any whitelist? */

	            if (f_deliver) {
	                const int f_spam = (pip->f.optnospam && mop->f.spam) ;
	                rs = procrecip_addrcheck(pip,&lcur,mop,f_spam) ;
			f_blacklist = ((rs > 0) && (! f_spam)) ;
	                f_deliver = (rs == 0) ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    debugprintf("main/procrecip: addrcheck() rs=%d\n",
				rs) ;
	                    debugprintf("main/procrecip: spam deliver=%u\n",
	                        f_deliver) ;
			}
#endif /* CF_DEBUG */
	            } /* end if (list-check) */

/* deliver or not based on what we know */

	            if ((rs >= 0) && f_deliver) {
	                mop->f.spamdeliver = FALSE ;
	                c += 1 ;
	                rs = recip_mo(rp,mop->moff,mop->mlen) ;
	            }

/* formulate log entry as a result */

	            if ((rs >= 0) && pip->f.logmsg) {

	                if (f_repeat) {
	                    fmt = "  %3u %c%c%c%c=%u" ;
	                } else {
	                    fmt = "  %3u %c%c%c%c" ;
			}

	                proglog_printf(pip,fmt,
	                    mn,
	                    ((f_deliver) ? 'D' : ' '),
	                    ((f_blacklist) ? 'B' : ' '),
	                    ((mop->f.spam) ? 'S' : ' '),
	                    ((f_repeat) ? 'R' : ' '),
	                    f_repeat) ;

	            } /* end if (logging enabled) */

	        } /* end if (procmsgid_update) */

	        if (rs < 0) break ;
	    } /* end for (looping through messages) */

#if	CF_DEBUGN
	nprintf(NDF,"main/procecip: mid rs=%d\n",rs) ;
#endif

	    pip->c_processed += 1 ;
	    if ((rs >= 0) && (rp->n > 0)) {

	        if (pip->progmode == progmode_dmail) {
	            if (pip->f.optdeliver) {
	                rs = progdeliver(pip,tfd,rp) ;

#if	CF_DEBUGN
		    nprintf(NDF,"main/procecip: progdeliver() rs=%d\n",rs) ;
#endif

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procrecip: "
				"deliver=%u progdeliver() rs=%d\n",
	                    pip->f.optdeliver,rs) ;
#endif

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procrecip: after progdeliver\n") ;
#endif

		    } /* end if (opt-deliver) */
	        } else if (pip->progmode == progmode_dmailbox) {

	            if (locinfo_mboxcount(lip) == 0) {
			cchar	*bn = pip->defbox ;
	                if ((bn != NULL) && (bn[0] != '\0')) {
	                    rs = locinfo_mboxadd(lip,bn,-1) ;
	                }
	            }

	            if (rs >= 0) {
	                rs = progboxer(pip,tfd,rp) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procrecip: progboxer() rs=%d\n",rs) ;
#endif
		    } /* end if (progboxer) */

	        } /* end if (progmode-dmailbox) */

	        if (rs < 0) {
	            if (pip->f.logmsg) {
	                fmt = "delivery failure r=%s (%d)\n" ;
	                proglog_printf(pip,fmt,r,rs) ;
	            }
	            if (pip->debuglevel > 0) {
			fmt = "%s: delivery failure r=%s (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,r,rs) ;
	            }
	        } /* end if */

	    } /* end if (delivery) */

	    if ((rs >= 0) && (rp->n > 0)) {
	        pip->c_delivered += 1 ;
	    }

	    if (pip->f.logmsg) {
	        if (rp->n > 0) {
	            if (rs >= 0) {
			fmt = "  delivery=%u offset=%d" ;
	                proglog_printf(pip,fmt,rp->n,rs) ;
	            } else {
			fmt = "  delivery=%u FAILED (%d)" ;
	                proglog_printf(pip,fmt,rp->n,rs) ;
		    }
	        } else {
	            proglog_printf(pip,"  delivery=0\n") ;
		}
	    } /* end if (logging) */

	    if ((pip->debuglevel > 0) && (! pip->f.quiet)) {
		cchar	*fs = ((rs >= 0) ? "ok" : "failed") ;
	        fmt = "%s: recip=%-32s %s (%d)\n" ;
	        if ((rs >= 0) || (rp->n == 0)) {
	            fmt = "%s: recip=%-32s\n" ;
	        }
	        bprintf(pip->efp,fmt,pn,r,fs,rs) ;
	    } /* end if */

	    rs1 = procrecip_addrend(pip,&lcur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (procrecip-addruser) */

#if	CF_DEBUGN
	nprintf(NDF,"main/procecip: ret rs=%d\n",rs) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrecip: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procrecip) */


static int procrecipvalid(PROGINFO *pip,RECIP *rp)
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	int		c = 0 ;
	char		*pwbuf ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrecipvalid: ent r=%s\n",
		rp->recipient) ;
#endif

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    const int	rsn = SR_NOTFOUND ;
	    cchar	*r = rp->recipient ;
	    if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,r)) >= 0) {
	        c = 1 ;
	        if ((rs = recip_setuser(rp,pw.pw_uid)) >= 0) {
	            const int	nlen = REALNAMELEN ;
	            char	nbuf[REALNAMELEN+1] ;
	            if ((rs = mkrecipname(nbuf,nlen,pw.pw_gecos)) >= 0) {
	                if ((rs = recip_setname(rp,nbuf,nlen)) >= 0) {
	                    rs = procrecip_mailspool(pip,rp) ;
	                }
	            }
	        } /* end if (recip_setuser) */
	    } else if (rs == rsn) {
		rs = SR_OK ;
		if (pip->progmode == progmode_dmail) {
	    	    rs = procrecip_hasmailfile(pip,rp) ;
	    	    c += rs ;
		}
	    } /* end if (user or not) */
	    uc_free(pwbuf) ;
	} /* end if (m-a) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrecipvalid: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procrecipvalid) */


#if	CF_LOOKADDR

static int procrecip_addropen(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	cchar		*pr = pip->pr ;
	cchar		*sn = pip->searchname ;
	if ((rs = lookaddr_start(&lip->la,pr,sn)) >= 0) {
	    lip->open.la = TRUE ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrecip_addropen: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (procrecip_addropen) */


static int procrecip_addrclose(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.la) {
	    lip->open.la = FALSE ;
	    rs1 = lookaddr_finish(&lip->la) ;
	    if (rs >= 0) rs = rs1 ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrecip_addrclose: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (procrecip_addrclose) */


static int procrecip_addrbegin(PROGINFO *pip,LOOKADDR_USER *curp,cchar *un)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrecip_addrbegin: ent un=%s\n",un) ;
#endif
	if (curp == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;
	if (lip->open.la) {
	    rs = lookaddr_userbegin(&lip->la,curp,un) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrecip_addrbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (procrecip_addrbegin) */


#if	CF_ADDRCHECK

static int procrecip_addrcheck(PROGINFO *pip,LOOKADDR_USER *curp,
		MSGINFO *mop,int f_spam)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	if (lip->open.la) {
	    cchar	*a ;
	    if ((rs >= 0) && (mop->e_from[0] != '\0')) {
		a = mop->e_from ;
	        rs = procrecip_addrchecker(pip,curp,a,f_spam) ;
		f_spam = rs ;
	    }
	    if ((rs >= 0) && (mop->h_from[0] != '\0')) {
		a = mop->h_from ;
	        rs = procrecip_addrchecker(pip,curp,a,f_spam) ;
		f_spam = rs ;
	    }
	    if ((rs >= 0) && (mop->h_replyto[0] != '\0')) {
		a = mop->h_replyto ;
	        rs = procrecip_addrchecker(pip,curp,a,f_spam) ;
		f_spam = rs ;
	    }
	    if ((rs >= 0) && (mop->h_sender[0] != '\0')) {
		a = mop->h_sender ;
	        rs = procrecip_addrchecker(pip,curp,a,f_spam) ;
		f_spam = rs ;
	    }
	} /* end if (lookup-address) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrecip_addrcheck: ret rs=%d f=%u\n",
		rs,f_spam) ;
#endif
	return (rs >= 0) ? f_spam : rs ;
}
/* end subroutine (procrecip_addrcheck) */


static int procrecip_addrchecker(PROGINFO *pip,LOOKADDR_USER *curp,
		cchar *a,int f_spam)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	if ((a[0] != '\0') && (strpbrk(a,"@!") != NULL)) {
	            LOOKADDR	*lap = &lip->la ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procrecip_addrchecker: a=%s\n",a) ;
	    debugprintf("main/procrecip_addrchecker: f_spam=%u\n",f_spam) ;
	}
#endif
	            rs = lookaddr_usercheck(lap,curp,a,f_spam) ;
	            f_spam = rs ;
	    } /* end if (qualified address) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrecip_addrchecker: ret rs=%d f=%u\n",
		rs,f_spam) ;
#endif
	return (rs >= 0) ? f_spam : rs ;
}
/* end subroutine (procrecip_addrchecker) */

#endif /* CF_ADDRCHECK */


static int procrecip_addrend(PROGINFO *pip,LOOKADDR_USER *curp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	if (curp == NULL) return SR_FAULT ;
	if (lip->open.la) {
	    rs = lookaddr_userend(&lip->la,curp) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrecip_addrend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (procrecip_addrend) */

#endif /* CF_LOOKADDR */


static int procrecip_mailspool(PROGINFO *pip,RECIP *rp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pl = 0 ;
	if (pip->progmode == progmode_dmail) {
	    USERATTR	ua ;
	    cchar	*un = rp->recipient ;
	    if ((rs = userattr_open(&ua,un)) >= 0) {
	        const int	vlen = MAXPATHLEN ;
	        int		vl ;
	        cchar		*ak = "md" ;
	        char		vbuf[MAXPATHLEN+1] ;
	        if ((vl = userattr_lookup(&ua,vbuf,vlen,ak)) >= 0) {
	            rs = recip_setmailspool(rp,vbuf,vl) ;
	            pl = rs ;
	        }
	        rs1 = userattr_close(&ua) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (userattr) */
	} /* end if (progmode-dmail) */
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (procrecip_mailspool) */


static int procrecip_hasmailfile(PROGINFO *pip,RECIP *rp)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (pip->progmode == progmode_dmail) {
	    struct ustat	sb ;
	    vecstr		*mlp = &pip->maildirs ;
	    int			i ;
	    cchar		*r = rp->recipient ;
	    cchar		*mdp ;
	    char		tbuf[MAXPATHLEN+1] ;
	    for (i = 0 ; vecstr_get(mlp,i,&mdp) >= 0 ; i += 1) {
	        if (mdp != NULL) {
	            if ((rs = mkpath2(tbuf,mdp,r)) >= 0) {
	                if ((rs = uc_stat(tbuf,&sb)) >= 0) {
	                    if (S_ISREG(sb.st_mode)) {
	                        c += 1 ;
	                        rs = recip_setmailspool(rp,mdp,-1) ;
	                    }
	                } else if (isNotPresent(rs)) {
	                    rs = SR_OK ;
			}
	            } /* end if (mkpath) */
	        } /* end if (non-null) */
	        if (c > 0) break ;
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (progmode-dmail) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procrecip_hasmailfile) */


static int procrecip_defmaildir(PROGINFO *pip,RECIP *rp)
{
	vecstr		*mlp = &pip->maildirs ;
	int		rs = SR_OK ;
	if (pip->progmode == progmode_dmail) {
	    if ((rs = recip_getmailspool(rp,NULL)) == 0) {
	        const int	rsn = SR_NOTFOUND ;
	        cchar		*mdp ;
	        if ((rs = vecstr_get(mlp,0,&mdp)) >= 0) {
	            rs = recip_setmailspool(rp,mdp,-1) ;
	        } else if (rs == rsn) {
	            mdp = MAILDNAME ;
	            rs = recip_setmailspool(rp,mdp,-1) ;
	        }
	    } /* end if (recip_getmailspool) */
	} /* end if (progmode-dmail) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrecip_defmaildir: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (procrecip_defmaildir) */


#ifdef	COMMENT
static int procunavail(PROGINFO *pip,int rstat)
{
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;
	int		f ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if (pip->open.logprog) {
	    proglog_printf(pip,"maildir=%s unavailable",pip->maildname) ;
	}

	if (pip->f.trusted && pip->f.optlogsys) {
	    LOGSYS	ls, *lsp = &ls ;
	    const int	fac = LOG_MAIL ;
	    int		opts = 0 ;
	    cchar	*logtab = pip->searchname ;
	    cchar	*logid = pip->logid ;

	    if ((rs1 = logsys_open(lsp,fac,logtab,logid,opts)) >= 0) {
	        fmt = "maildir=%s unavailable (%d)" ;
	        rs1 = logsys_printf(lsp,LOG_ERR,fmt,pip->maildname,rstat) ;
	        logsys_close(lsp) ;
	    } /* end if (logsys) */

	    if (rs1 < 0) {
	        fmt = "%s: inaccessible syslog (%d)\n" ;
	        bprintf(pip->efp,fmt,pn,rs1) ;
	        if (pip->open.logprog) {
	            fmt = "inaccessible syslog (%d)" ;
	            proglog_printf(pip,fmt,rs1) ;
		}
	    }

	} /* end if (logging to the system) */

	f = pip->f.optdivert && (pip->deadmaildname != NULL) ;
	if (f) {
	    rs = perm(pip->deadmaildname,-1,-1,NULL,W_OK) ;
	}

	if (f && (rs >= 0)) {
	    cchar	*un = DIVERTUSER ;

	    pip->f.diverting = TRUE ;
	    pip->maildname = pip->deadmaildname ;

	    if (pip->open.logprog) {
	       fmt = "diverting maildir=%s\n" ;
	        proglog_printf(pip,fmt,pip->maildname) ;
	    }

	    if ((rs = getuid_name(un,-1)) >= 0) {
	        pip->uid_divert = rs ;
	    }

	} /* end if */

	return rs ;
}
/* end subroutine (procunavail) */
#endif /* COMMENT */


static int procmboxes(PROGINFO *pip,cchar *sp,int sl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		cl ;
	int		c = 0 ;
	cchar		*tp ;
	cchar		*cp ;

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


#ifdef	COMMENT
static int procmultierror(PROGINFO *pip,cchar *mfn)
{
	int		rs = SR_OK ;
	if (pip->f.multirecip) {
	    bfile	mfile, *mfp = &mfile ;

	    if ((mfn == NULL) || (mfn[0] == '-')) mfn = BFILE_STDOUT ;

	    if ((rs = bopen(mfp,mfn,"wct",0644)) >= 0) {
	        bprintf(mfp,"hello world!\n") ;
	        bclose(mfp) ;
	    } /* end if (opened output */
	} /* end if (multi-recipient mode) */
	return rs ;
}
/* end subroutine (procmultierror) */
#endif /* COMMENT */


static int procfindsched(PROGINFO *pip,vecstr *slp)
{
	int		rs = SR_OK ;
	int		i ;
	cchar		*keys = "penmh" ;
	for (i = 0 ; keys[i] != '\0' ; i += 1) {
	    const int	kch = MKCHAR(keys[i]) ;
	    int		vl = -1 ;
	    cchar	*vp = NULL ;
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
	    case 'm':
	        vp = "mail" ;
	        break ;
	    case 'h':
	        vp = pip->homedname ;
	        break ;
	    } /* end switch */
	    if ((rs >= 0) && (vp != NULL)) {
	        char	kbuf[2] = { 0, 0 } ;
	        kbuf[0] = kch ;
	        rs = vecstr_envset(slp,kbuf,vp,vl) ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	return rs ;
}
/* end subroutine (procfindsched) */


static int procportcomsat(PROGINFO *pip)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	if (pip->portspec == NULL) pip->portspec = PORTSPEC_COMSAT ;
	if ((rs = getportnum(PROTONAME_COMSAT,pip->portspec)) == rsn) {
	    rs = SR_OK ;
	    pip->port_comsat = IPPORT_BIFFUDP ;
	} else {
	    pip->port_comsat = rs ;
	}
	return rs ;
}
/* end subroutine (procportcomsat) */


static int procmaildirdead(PROGINFO *pip,cchar *dp,int dl)
{
	int		rs = SR_OK ;
	if (pip->deadmaildname == NULL) {
	    NULSTR	ds ;
	    cchar	*dname ;
	    if ((rs = nulstr_start(&ds,dp,dl,&dname)) >= 0) {
	        struct ustat	sb ;
	        if ((rs = u_stat(dname,&sb)) >= 0) {
		    if (S_ISDIR(sb.st_mode)) {
			const int	pm = (W_OK|W_OK) ;
	                if ((rs = perm(dname,-1,-1,NULL,pm)) >= 0) {
			    cchar	**vpp = &pip->deadmaildname ;
			    rs = proginfo_setentry(pip,vpp,dname,-1) ;
		        } else if (isNotAccess(rs)) {
			    rs = SR_OK ;
		        }
		    }
	        } else if (isNotPresent(rs)) {
		    rs = SR_OK ;
	        }
		nulstr_finish(&ds) ;
	    } /* end if (nulstr) */
	}
	return rs ;
}
/* end subroutine (procmaildirdead) */


static int procmaildircopy(PROGINFO *pip,cchar *dp,int dl)
{
	int		rs = SR_OK ;
	if (pip->copymaildname == NULL) {
	    NULSTR	ds ;
	    cchar	*dname ;
	    if ((rs = nulstr_start(&ds,dp,dl,&dname)) >= 0) {
	        struct ustat	sb ;
	        if ((rs = u_stat(dname,&sb)) >= 0) {
		    if (S_ISDIR(sb.st_mode)) {
			const int	pm = (W_OK|W_OK) ;
	                if ((rs = perm(dname,-1,-1,NULL,pm)) >= 0) {
			    cchar	**vpp = &pip->copymaildname ;
			    rs = proginfo_setentry(pip,vpp,dname,-1) ;
		        } else if (isNotAccess(rs)) {
			    rs = SR_OK ;
		        }
		    }
	        } else if (isNotPresent(rs)) {
		    rs = SR_OK ;
	        }
		nulstr_finish(&ds) ;
	    } /* end if (nulstr) */
	}
	return rs ;
}
/* end subroutine (procmaildircopy) */


static int procmaildirs(PROGINFO *pip,PARAMOPT *pop)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (pip->progmode == progmode_dmail) {
	    int		i ;
	    cchar	*dns ;
	    cchar	*tp ;
	    for (i = 0 ; varmaildirs[i] != NULL ; i += 1) {
	        dns = getourenv(pip->envv,varmaildirs[i]) ;
	        if (dns != NULL) {
	            while ((tp = strpbrk(dns," :,\t\n")) != NULL) {
	                rs = procmaildir(pip,pop,dns,(tp-dns)) ;
	                if (rs < 0) break ;
	                c += rs ;
	                dns = (tp+1) ;
	            } /* end while */
	            if ((rs >= 0) && (dns[0] != '\0')) {
	                rs = procmaildir(pip,pop,dns,-1) ;
	                c += rs ;
	            } /* end if */
	        }
	        if (rs < 0) break ;
	    } /* end for */
	    if (rs >= 0) {
	        cchar	*dns = MAILDNAME ;
	        rs = procmaildir(pip,pop,dns,-1) ;
	        c += rs ;
	    }
	} /* end if (progmode-dmail) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmaildirs) */


static int procmaildir(PROGINFO *pip,PARAMOPT *pop,cchar *dp,int dl)
{
	int		rs ;
	int		c = 0 ;
	cchar		*po = PO_MAILDIRS ;

	if (pip == NULL) return SR_FAULT ;

	if (dl < 0) dl = strlen(dp) ;

	if ((rs = paramopt_haveval(pop,po,dp,dl)) == 0) {
	    char	dname[MAXPATHLEN+1] ;
	    if (mkpath1w(dname,dp,dl) > 0) {
	        struct ustat	sb ;
	        if ((rs = u_stat(dname,&sb)) >= 0) {
	            rs = paramopt_loads(pop,po,dp,dl) ;
	            c += rs ;
	        } else if (isNotPresent(rs))
	            rs = SR_OK ;
	    } /* end if */
	} /* end if (have-val?) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmaildir) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to = -1 ;

#ifdef	COMMENT
	rs = vecstr_start(&lip->stores,0,0) ;
	lip->open.stores = (rs >= 0) ;
#endif /* COMMENT */

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.la) {
	    lip->open.la = FALSE ;
	    rs1 = lookaddr_finish(&lip->la) ;
	    if (rs >= 0) rs = rs1 ;
	}

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

	if (lip->open.mboxes) {
	    lip->open.mboxes = FALSE ;
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
int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar vp[],int vl)
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
	    vecstr	*svp = &lip->stores ;
	    int		oi = -1 ;
	    if (*epp != NULL) {
	        oi = vecstr_findaddr(svp,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(svp,len,epp) ;
	    } else {
	        *epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(svp,oi) ;
	    }
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCSETENT */


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
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;

	rs = grmems_lookup(&lip->gm,&curp->gmcur,gnp,gnl) ;

	return rs ;
}
/* end subroutine (locinfo_gmlook) */


int locinfo_gmread(LOCINFO *lip,LOCINFO_GMCUR *curp,char ubuf[],int ulen)
{
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;

	rs = grmems_lookread(&lip->gm,&curp->gmcur,ubuf,ulen) ;

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
	const int	fo = 0 ;
	int		rs ;

	if (pip == NULL) return SR_FAULT ;
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


int locinfo_rnread(LOCINFO *lip,LOCINFO_RNCUR *curp,char ubuf[],int ulen)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;

	if (pip == NULL) return SR_FAULT ;
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


static int locinfo_mboxadd(LOCINFO *lip,cchar *mp,int ml)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;
	if (mp == NULL) return SR_FAULT ;

	if (! lip->open.mboxes) {
	    rs = vecstr_start(&lip->mboxes,0,0) ;
	    lip->open.mboxes = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = vecstr_adduniq(&lip->mboxes,mp,ml) ;
	}

	return rs ;
}
/* end subroutine (locinfo_mboxadd) */


static int locinfo_mboxcount(LOCINFO *lip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.mboxes) {
	    rs = vecstr_count(&lip->mboxes) ;
	}

	return rs ;
}
/* end subroutine (locinfo_mboxcount) */


int locinfo_mboxget(LOCINFO *lip,int i,cchar **rpp)
{
	int		rs = SR_OK ;
	int		rl = 0 ;
	cchar		*rp = NULL ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.mboxes) {
	    if ((rs = vecstr_get(&lip->mboxes,i,&rp)) >= 0)
	        rl = strlen(rp) ;
	} else
	    rs = SR_NOTFOUND ;

	if (rpp != NULL)
	    *rpp = rp ;

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (locinfo_mboxget) */


static int mkreport(PROGINFO *pip,int argc,cchar **argv,int rrs)
{
	const int	ulen = USERNAMELEN ;
	int		rs ;
	char		ubuf[USERNAMELEN+1] ;

	if (pip->daytime == 0) pip->daytime = time(NULL) ;

	if ((rs = getusername(ubuf,ulen,-1)) >= 0) {
	    const mode_t	m = 0777 ;
	    cchar		*dname = pip->progname ;
	    cchar		*oun = pip->username ;
	    char		rbuf[MAXPATHLEN+1] ;
	    pip->username = ubuf ;
	    if ((rs = mktmpreportdir(rbuf,ubuf,dname,m)) >= 0) {
		char	fbuf[MAXPATHLEN+1] ;
		if ((rs = mkreportfile(pip,fbuf,rbuf)) >= 0) {
		    const int	nlen = NODENAMELEN ;
		    char	nbuf[NODENAMELEN+1] ;
		    if (pip->pid == 0) pip->pid = getpid() ;
		    if ((rs = getnodename(nbuf,nlen)) >= 0) {
		        const int	llen = LOGIDLEN ;
			const int	v = pip->pid ;
			cchar		*onn = pip->nodename ;
		        char		lbuf[LOGIDLEN+1] ;
			pip->nodename = nbuf ;
		        if ((rs = mklogid(lbuf,llen,nbuf,rs,v)) >= 0) {
 			    {
		    	    rs = mkreportout(pip,fbuf,lbuf,argc,argv,rrs) ;
			    }
		        } /* end if (mklogid) */
			pip->nodename = onn ;
		    } /* end if (getnodename) */
		} /* end if (mkreportfile) */
	    } /* end if (mktmpuserdir) */
	    pip->username = oun ;
	} /* end if (getusername) */

	return rs ;
}
/* end subroutine (mkreport) */


static int mkreportfile(PROGINFO *pip,char *fbuf,cchar *rbuf)
{
	TMTIME		mt ;
	const time_t	dt = pip->daytime ;
	int		rs ;

	if ((rs = tmtime_localtime(&mt,dt)) >= 0) {
	    const int	tlen = TIMEBUFLEN ;
	    cchar	*fmt = "r%y%m%d%H%M%S" ;
	    char	tbuf[TIMEBUFLEN+1] ;
	    if ((rs = sntmtime(tbuf,tlen,&mt,fmt)) >= 0) {
	        rs = mkpath2(fbuf,rbuf,tbuf) ;
	    } /* end if (sntmtime) */
	} /* end if (localtime) */

	return rs ;
}
/* end subroutine (mkreportfile) */


static int mkreportout(PROGINFO *pip,cchar *fbuf,cchar *id,int ac,cchar **av,
		int rrs)
{
	bfile		rfile, *rfp = &rfile ;
	const time_t	dt = pip->daytime ;
	const mode_t	om = 0644 ;
	int		rs ;
	cchar		*fmt ;
	char		tbuf[TIMEBUFLEN+1] ;
	timestr_logz(dt,tbuf) ;
	if ((rs = bopen(rfp,fbuf,"wct",om)) >= 0) {
	    if ((rs = bminmod(rfp,om)) >= 0) {
	    const int	al = DISARGLEN ;
	    int		v = pip->pid ;
	    int		i ;

	    fmt = "%-15s %s junk report (%d)\n" ;
	    bprintf(rfp,fmt,id,tbuf,rrs) ;

	    fmt = "%-15s node=%s\n" ;
	    bprintf(rfp,fmt,id,pip->nodename) ;

	    fmt = "%-15s user=%s\n" ;
	    bprintf(rfp,fmt,id,pip->username) ;

	    fmt = "%-15s pid=%u\n" ;
	    bprintf(rfp,fmt,id,v) ;

	    fmt = "%-15s pwd=%s\n" ;
	    bprintf(rfp,fmt,id,pip->pwd) ;

	    if ((rs = proginfo_progdname(pip)) >= 0) {
	        fmt = "%-15s progdir=%s\n" ;
	        bprintf(rfp,fmt,id,pip->progdname) ;
	    }

	    fmt = "%-15s progname=%s\n" ;
	    bprintf(rfp,fmt,id,pip->progname) ;

	    fmt = "%-15s argc=%u args¬\n" ;
	    bprintf(rfp,fmt,id,ac) ;

	    fmt = "%-15s a%02u=>%t<\n" ;
	    for (i = 0 ; (i < ac) && (av[i] != NULL) ; i += 1) {
		cchar	*ap = av[i] ;
	        rs = bprintf(rfp,fmt,id,i,ap,al) ;
		if (rs < 0) break ;
	    } /* end if */

	    fmt = "%-15s done\n" ;
	    bprintf(rfp,fmt,id) ;

	    } /* end if (bminmod) */
	    bclose(rfp) ;
	} /* end if (file) */
	return rs ;
}
/* end subroutine (mkreportout) */


/* ARGSUSED */
static int mktmpreportdir(char *rbuf,cchar *ubuf,cchar *dname,mode_t m)
{
	cchar		*rdname = REPORTDNAME ;
	int		rs ;
	int		rl = 0 ;
	if ((rs = mkdirs(rdname,m)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = uc_stat(rdname,&sb)) >= 0) {
		const uid_t	uid = getuid() ;
		const uid_t	u = sb.st_uid ;
		uid_t		uid_admin = -1 ;
		if (u == uid) {
		    if ((rs = uc_minmod(rdname,m)) >= 0) {
			cchar	*adm = ADMINUSER ;
			if ((rs = getuid_user(adm,-1)) >= 0) {
			    uid_admin = rs ;
			    rs = uc_chown(rdname,uid_admin,-1) ;
			} else if (isNotPresent(rs)) {
			    rs = SR_OK ;
			}
		    }
		}
		if (rs >= 0) {
		    if ((rs = mkpath2(rbuf,rdname,dname)) >= 0) {
	    	        rl = rs ;
	    	        if ((rs = mkdirs(rbuf,m)) >= 0) {
	        	    if ((rs = uc_minmod(rbuf,m)) >= 0) {
				if ((uid_admin >= 0) && (uid != uid_admin)) {
				    rs = uc_chown(rbuf,uid_admin,-1) ;
				}
			    }
	    	        }
		    }
		}
	    } /* end if (stat) */
	} /* end if (mkdirs) */
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (mktmpreportdir) */


#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGRECIPS
static int debugrecips(PROGINFO *pip,VECOBJ *rlp)
{
	int		rs = SR_OK ;
	if (DEBUGLEVEL(5)) {
	    RECIP	*rp ;
	    int		i ;
	    int		cl ;
	    cchar	*cp ;
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


