/* b_mailnew */

/* update the machine status for the current machine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_LOCSETENT	0		/* |locinfo_setentry()| */
#define	CF_LOCEPRINTF	0		/* |locinfo_eprintf()| */
#define	CF_CONFIGCHECK	0		/* |config_check()| */


/* revision history:

	= 1989-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a built-in command to the KSH shell.  It should also be able to
	be made into a stand-alone program without much (if almost any)
	difficulty, but I have not done that yet (we already have a MSU program
	out there).

	Note that special care needed to be taken with the child processes
	because we cannot let them ever return normally!  They cannot return
	since they would be returning to a KSH program that thinks it is alive
	(!) and that geneally causes some sort of problem or another.  That is
	just some weird thing asking for trouble.  So we have to take care to
	force child processes to exit explicitly.  Child processes are only
	created when run in "daemon" mode.

	Synopsis:

	$ mailnew [-u <user>]

	Notes:

	= Command options:
	Dates available right now:
	- default	hh:mm			(5 bytes)
	- long		CC-Mmm-DD hh:mm		(15 bytes)


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
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stddef.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<char.h>
#include	<tmtime.h>
#include	<sntmtime.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<field.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<userinfo.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<logfile.h>
#include	<storebuf.h>
#include	<mailbox.h>
#include	<mbcache.h>
#include	<hdrdecode.h>
#include	<prsetfname.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_mailnew.h"
#include	"defs.h"
#include	"proglog.h"


/* local defines */

#ifndef	VBUFLEN
#define	VBUFLEN		(2 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#ifndef	SUBJBUFLEN
#define	SUBJBUFLEN	(2*COLUMNS)
#endif

#ifndef	NOTEBUFLEN
#define	NOTEBUFLEN	COLUMNS
#endif

#define	DSBUFLEN	15		/* large enough for "CC-MMM-DD hh:mm" */
#define	MAXOVERLEN	22		/* standard date overhead (in bytes) */
#define	MAXFROMLEN	35

#define	DEBUGFNAME	"/tmp/mailnew.deb"

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	CONFIG		struct config

#define	MSGENTRY	struct msgentry

#define	OUTINFO		struct outinfo

#define	PO_MAILDIRS	"maildirs"
#define	PO_MAILUSERS	"mailusers"


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpycompact(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpywidehdr(char *,int,const wchar_t *,int) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkcaselower(char *,int) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	wsfnext(const int *,int,const int **) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	matpstr(cchar **,int,cchar *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	mkplogid(char *,int,const char *,int) ;
extern int	mksublogid(char *,int,const char *,int) ;
extern int	mkcleanline(char *,int,int) ;
extern int	mkdisphdr(char *,int,cchar *,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	tolc(int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpycompact(char *,cchar *,int) ;
extern char	*strdcpycompact(char *,int,cchar *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strwcpywide(char *,int *,wchar_t *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		md:1 ;
	uint		mailusers:1 ;
	uint		msgs:1 ;
	uint		sort:1 ;
	uint		sortrev:1 ;
	uint		nshow:1 ;
	uint		linelen:1 ;
	uint		datelong:1 ;
} ;

struct locinfo {
	PROGINFO	*pip ;
	LOCINFO_FL	have, f, final, changed ;
	LOCINFO_FL	open ;
	VECSTR		stores ;
	VECSTR		mailusers ;
	VECHAND		msgs ;
	int		nshow ;
	int		linelen ;
} ;

struct config {
	PROGINFO	*pip ;
	PARAMOPT	*app ;
	PARAMFILE	p ;
	EXPCOOK		cooks ;
	uint		f_p:1 ;
	uint		f_cooks:1 ;
} ;

struct msgentry {
	const char	*user ;
	const char	*date ;		/* just "HH:MM" */
	const char	*from ;
	const char	*subj ;
	const char	*a ;		/* memory-allocation */
	time_t		mdate ;
	int		lines ;
} ;

struct outinfo {
	MSGENTRY	*mep ;
	char		*fbuf ;
	char		*sbuf ;
	int		ll ;
	int		dl, ul ;
	int		flen, fl ;
	int		slen, sl ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *,PARAMOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,PARAMOPT *,cchar *) ;
static int	procnames(PROGINFO *,PARAMOPT *,cchar *,cchar *,int) ;
static int	procmaildirs(PROGINFO *,PARAMOPT *) ;
static int	procmaildir(PROGINFO *,PARAMOPT *,const char *,int) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procuserinfo_logid(PROGINFO *) ;

static int	procourconf_begin(PROGINFO *,PARAMOPT *,const char *) ;
static int	procourconf_end(PROGINFO *) ;

static int	procmailusers(PROGINFO *,PARAMOPT *) ;
static int	procmailusers_env(PROGINFO *,const char *) ;
static int	procmailusers_arg(PROGINFO *,PARAMOPT *) ;
static int	procmailusers_def(PROGINFO *) ;
static int	procmailusers_add(PROGINFO *,const char *,int) ;

static int	process(PROGINFO *pip,PARAMOPT *,const char *) ;
static int	procmailboxes(PROGINFO *,PARAMOPT *) ;
static int	procmailbox(PROGINFO *,const char *,const char *) ;
static int	procmailmsg(PROGINFO *,const char *,MBCACHE *,int) ;
static int	procout(PROGINFO *,void *) ;
static int	procouter(PROGINFO *,void *,HDRDECODE *,MSGENTRY *) ;

static int	sfnormfrom(cchar *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_nshow(LOCINFO *,const char *) ;
static int	locinfo_logprintf(LOCINFO *,const char *,...) ;
static int	locinfo_optdate(LOCINFO *,cchar *,int) ;

#if	CF_LOCEPRINTF
static int	locinfo_eprintf(LOCINFO *,const char *,...) ;
#endif /* CF_LOCEPRINTF */

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif /* CF_LOCSETENT */

static int	config_start(CONFIG *,PROGINFO *,PARAMOPT *,cchar *) ;
static int	config_findfile(CONFIG *,char *,const char *) ;
static int	config_cookbegin(CONFIG *) ;
static int	config_cookend(CONFIG *) ;
static int	config_read(CONFIG *) ;
static int	config_finish(CONFIG *) ;
static int	config_setlfname(CONFIG *,const char *,int) ;

#if	CF_CONFIGCHECK
static int	config_check(CONFIG *) ;
#endif /* CF_CONFIGCHECK */

static int	msgentry_start(MSGENTRY *,cchar *,time_t,cchar *,
			cchar **,const int *) ;
static int	msgentry_finish(MSGENTRY *) ;
static int	msgentry_from(MSGENTRY *,const char **) ;
static int	msgentry_subj(MSGENTRY *,const char **) ;

static int	outinfo_start(OUTINFO *,MSGENTRY *,int,int) ;
static int	outinfo_finish(OUTINFO *) ;
static int	outinfo_trans(OUTINFO *,HDRDECODE *) ;
static int	outinfo_cvt(OUTINFO *,HDRDECODE *,char *,int,cchar *,int,int) ;
static int	outinfo_calc(OUTINFO *) ;
static int	outinfo_total(OUTINFO *) ;
static int	outinfo_print(OUTINFO *,PROGINFO *,void *,int) ;

static int	mkmsfname(char *,const char *,int,const char *) ;

static int	vcmpfor(const void *,const void *) ;
static int	vcmprev(const void *,const void *) ;


/* local variables */

static const char	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"ROOT",
	"HELP",
	"LOGFILE",
	"md",
	"sn",
	"af",
	"ef",
	"of",
	"cf",
	"lf",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_root,
	argopt_help,
	argopt_logfile,
	argopt_md,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_cf,
	argopt_lf,
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
	"md",
	"sort",
	"nshow",
	"date",
	NULL
} ;

enum akonames {
	akoname_md,
	akoname_sort,
	akoname_nshow,
	akoname_date,
	akoname_overlast
} ;

static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

static const char	*cparams[] = {
	"maildir",
	"logsize",
	"logfile",
	NULL
} ;

enum cparams {
	cparam_maildir,
	cparam_logsize,
	cparam_logfile,
	cparam_overlast
} ;

static const char	*varmaildirs[] = {
	VARMAILDNAMESP,
	VARMAILDNAMES,
	VARMAILDNAME,
	NULL
} ;

static const char	*varmailusers[] = {
	VARMAILUSERSP,
	VARMAILUSERS,
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

static const char	*datetypes[] = {
	"long",
	NULL
} ;

enum datetypes {
	datetype_long,
	datetype_overlast
} ;


/* exported subroutines */


int b_mailnew(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    const char	**envv = (const char **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_mailnew) */


int p_mailnew(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_mailnew) */


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
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*cfname = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_mailnew: starting DFD=%d\n",rs) ;
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

	pip->f.logprog = TRUE ;

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

	if (rs >= 0) {
	    rs = paramopt_start(&aparams) ;
	    pip->open.aparams = (rs >= 0) ;
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

	                case argopt_logfile:
	                    pip->have.logprog = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            pip->lfname = avp ;
	                            pip->final.logprog = TRUE ;
	                        }
	                    }
	                    break ;

/* mail directory */
	                case argopt_md:
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
	                    if ((rs >= 0) && (cp != NULL)) {
				PARAMOPT	*pop = &aparams ;
	                        cchar		*po = PO_MAILDIRS ;
	                        rs = paramopt_loads(pop,po,cp,cl) ;
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

/* output file-name */
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
	                    cp = NULL ;
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
	                    if ((rs >= 0) && (cp != NULL)) {
	                        pip->have.cfname = TRUE ;
	                        pip->final.cfname = TRUE ;
	                        cfname = cp ;
	                    }
	                    break ;

/* log file name */
	                case argopt_lf:
	                    cp = NULL ;
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
	                    if ((rs >= 0) && (cp != NULL)) {
	                        pip->have.logprog = TRUE ;
	                        pip->lfname = cp ;
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

	                    case 'C':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->have.cfname = TRUE ;
	                                pip->final.cfname = TRUE ;
	                                cfname = argp ;
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* long dates */
	                    case 'l':
	                        lip->f.datelong = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                        	lip->f.datelong = (rs > 0) ;
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

/* alternate users */
	                    case 'u':
	                        cp = NULL ;
	                        cl = 0 ;
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
				    PARAMOPT	*pop = &aparams ;
	                            cchar	*po = PO_MAILUSERS ;
	                            rs = paramopt_loads(pop,po,cp,cl) ;
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

/* width (columns) */
	                    case 'w':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->have.linelen = TRUE ;
	                                lip->final.linelen = TRUE ;
	                                rs = optvalue(argp,argl) ;
	                                lip->linelen = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
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

#if	CF_DEBUGS
	debugprintf("main: finished parsing arguments\n") ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
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
	    shio_printf(pip->efp, "%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp, "%s: sn=%s\n",pip->progname,pip->searchname) ;
	}

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

	if (f_help || f_usage || f_version)
	    goto retearly ;


	ex = EX_OK ;

/* cotinue intialization */

	if (argval == NULL) argval = getourenv(pip->envv,VARNSHOW) ;

	if ((rs = locinfo_nshow(lip,argval)) >= 0) {
	    rs = procopts(pip,&akopts,&aparams) ;
	}

	if ((rs >= 0) && (lip->linelen == 0)) {
	    cp = NULL ;
	    if (cp == NULL) cp = getourenv(pip->envv,VARLINELEN) ;
	    if (cp == NULL) cp = getourenv(pip->envv,VARCOLUMNS) ;
	    if (cp != NULL) {
	        rs = optvalue(cp,-1) ;
	        lip->linelen = rs ;
	    }
	}

	if (lip->linelen == 0) lip->linelen = COLUMNS ;

	if (afname == NULL) afname = getourenv(pip->envv,VARAFNAME) ;

	if (cfname == NULL) cfname = getourenv(pip->envv,VARCFNAME) ;

	if (pip->lfname == NULL) pip->lfname = getourenv(pip->envv,VARLFNAME) ;

	if (cfname != NULL) pip->final.cfname = TRUE ;

/* get some mail-users */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    ARGINFO	*aip = &ainfo ;
	    if ((rs = procargs(pip,aip,&pargs,&aparams,afname)) >= 0) {
	        USERINFO	u ;
	        if ((rs = userinfo_start(&u,NULL)) >= 0) {
	            if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
			PARAMOPT	*pop = &aparams ;
		        cchar		*cfn = cfname ;
	                if (cfname != NULL) {
	                    if (pip->euid != pip->uid) u_seteuid(pip->uid) ;
	                    if (pip->egid != pip->gid) u_setegid(pip->gid) ;
	                }
	                if ((rs = procourconf_begin(pip,pop,cfn)) >= 0) {
	                    if ((rs = proglog_begin(pip,&u)) >= 0) {
	                        if ((rs = proguserlist_begin(pip)) >= 0) {

	                            if (rs >= 0)
	                                rs = procmaildirs(pip,pop) ;

	                            if (rs >= 0)
	                                rs = procmailusers(pip,pop) ;

	                            if (pip->debuglevel > 0) {
	                                cchar	*pn = pip->progname ;
	                                cchar	*s = "off" ;
				        cchar	*fmt ;
	                                if (lip->f.sort) {
					    int f_rev = lip->f.sortrev ;
	                                    s = (f_rev) ? "rev" : "for" ;
	                                }
				        fmt = "%s: sort=%s\n" ;
	                                shio_printf(pip->efp,fmt,pn,s) ;
	                            }

	                            if (rs >= 0) {
	                                rs = process(pip,pop,ofname) ;
	                            }

	                            rs1 = proguserlist_end(pip) ;
				    if (rs >= 0) rs = rs1 ;
	                        } /* end if (proguserlist) */
	                        rs1 = proglog_end(pip) ;
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
		    cchar	*pn = pip->progname ;
		    cchar	*fmt = "%s: userinfo failure (%d)\n" ;
	            ex = EX_NOUSER ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	        }
	    } /* end if (procargs) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

/* done */
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
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

/* early return thing */
retearly:
	if ((pip->debuglevel > 0) && (pip->efp != NULL)) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_mailnew: exiting ex=%u (%d)\n",ex,rs) ;
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
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
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
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<user(s)>] [-o <opt(s)>] [-md <maildir(s)>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop,PARAMOPT *app)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

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
	                case akoname_md:
	                    if (vl > 0) {
	                        const char	*po = PO_MAILDIRS ;
	                        lip->have.md = TRUE ;
	                        rs = paramopt_loads(app,po,vp,vl) ;
	                    }
	                    break ;
	                case akoname_sort:
	                    if (! lip->final.sort) {
	                        lip->final.sort = TRUE ;
	                        lip->have.sort = TRUE ;
	                        lip->f.sort = TRUE ;
	                        if (vl > 0) {
	                            int	ch = tolc(*vp & 0xff) ;
	                            switch (ch) {
	                            case '0':
	                            case 'n':
	                                lip->f.sort = FALSE ;
	                                break ;
	                            case 'r':
	                                lip->f.sortrev = TRUE ;
	                                break ;
	                            } /* end switch */
	                        }
	                    }
	                    break ;
	                case akoname_nshow:
	                    if (! lip->final.nshow) {
	                        if (vl) {
	                            lip->final.nshow = TRUE ;
	                            lip->have.nshow = TRUE ;
	                            rs = optvalue(vp,vl) ;
	                            lip->nshow = rs ;
	                        }
	                    }
	                    break ;
	                case akoname_date:
	                    if (! lip->final.datelong) {
	                        lip->final.datelong = TRUE ;
	                        lip->have.datelong = TRUE ;
	                        lip->f.datelong = TRUE ;
	                        if (vl) {
				    rs = locinfo_optdate(lip,vp,vl) ;
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

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
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
	            char	pbuf[LOGIDLEN+1] ;
	            if ((rs = mkplogid(pbuf,plen,nn,pv)) >= 0) {
	                const int	slen = LOGIDLEN ;
	                char		sbuf[LOGIDLEN+1] ;
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


static int procourconf_begin(PROGINFO *pip,PARAMOPT *app,cchar cfname[])
{
	const int	csize = sizeof(CONFIG) ;
	int		rs = SR_OK ;
	void		*p ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_mailnew/procourconf_begin: ent\n") ;
#endif

	if ((rs = uc_malloc(csize,&p)) >= 0) {
	    CONFIG	*csp = p ;
	    pip->config = csp ;
	    if ((rs = config_start(csp,pip,app,cfname)) >= 0) {
	        if ((rs = config_read(csp)) >= 0) {
	            rs = 1 ;
	        }
	        if (rs < 0)
	            config_finish(csp) ;
	    } /* end if (config) */
	    if (rs < 0) {
	        uc_free(p) ;
	        pip->config = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_mailnew/procourconf_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procourconf_begin) */


static int procourconf_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_mailnew/procourconf_end: config=%u\n",
	        (pip->config != NULL)) ;
#endif

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


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,PARAMOPT *app,
		cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	const char	*po = PO_MAILUSERS ;
	const char	*cp ;

	if (rs >= 0) {
	    int	ai ;
	    int	f ;
	    for (ai = 1 ; (rs >= 0) && (ai < aip->argc) ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	        if (f) {
	            cp = aip->argv[ai] ;
	            if (cp[0] != '\0') {
	                pan += 1 ;
	                rs = paramopt_loads(app,po,cp,-1) ;
	            }
	        }

	    } /* end for */
	} /* end if (ok) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    SHIO	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0)
	        afn = STDINFNAME ;

	    if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procnames(pip,app,po,cp,cl) ;
	                    }
	                }

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = shio_close(afp) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
		cchar	*pn = pip->progname ;
		cchar	*fmt ;
		fmt = "%s: inaccessible argument-list (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	    } /* end if */

	} /* end if (processing file argument file list) */

	return rs ;
}
/* end subroutine (procargs) */


static int procnames(PROGINFO *pip,PARAMOPT *app,cchar *po,cchar *lbuf,int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	     	    rs = paramopt_loads(app,po,fp,fl) ;
	            c += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procnames) */


static int process(PROGINFO *pip,PARAMOPT *app,const char *ofn)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {

	    rs = procmailboxes(pip,app) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("b_mailnew: procmailboxes() rs=%d\n",rs) ;
#endif

	    if (pip->debuglevel > 0) {
	        shio_printf(pip->efp,"%s: nmsgs=%d\n",pip->progname,rs) ;
	    }

	    if (rs >= 0) {
	        rs = procout(pip,ofp) ;
	        wlen += rs ;
	    }

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


/* process the mailbox */
static int procmailboxes(PROGINFO *pip,PARAMOPT *app)
{
	LOCINFO		*lip = pip->lip ;
	PARAMOPT_CUR	cur ;
	int		rs ;
	int		c = 0 ;

	if ((rs = paramopt_curbegin(app,&cur)) >= 0) {
	    vecstr	*ulp = &lip->mailusers ;
	    int		ml ;
	    int		i ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    const char	*po = PO_MAILDIRS ;
	    const char	*mp ;
	    const char	*mup ;
	    char	msfname[MAXPATHLEN+1] ;

	    while ((ml = paramopt_fetch(app,po,&cur,&mp)) >= 0) {
	        int	mc = 0 ;

	        if (pip->debuglevel > 0) {
	            fmt = "%s: maildir=%s\n" ;
	            shio_printf(pip->efp,fmt,pn,mp) ;
		}

	        if (ml > 0) {
	            for (i = 0 ; vecstr_get(ulp,i,&mup) >= 0 ; i += 1) {
	                if (mup != NULL) {
	                    if ((rs = mkmsfname(msfname,mp,ml,mup)) >= 0) {
	                        rs = procmailbox(pip,mup,msfname) ;
	                        mc += rs ;
	                    }
			}
	                if (rs < 0) break ;
	            } /* end for (mail-users) */
	        } /* end if (non-zero) */

	        if (pip->debuglevel > 0) {
	            fmt = "%s: msgs=%u\n" ;
	            shio_printf(pip->efp,fmt,pn,mc) ;
		}

	        c += mc ;
	        if (rs < 0) break ;
	    } /* end while */

	    paramopt_curend(app,&cur) ;
	} /* end if (paramopt-cur) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailboxes) */


static int procmailbox(PROGINFO *pip,cchar *un,cchar *msfname)
{
	MAILBOX		mb ;
	const int	mbopts = MAILBOX_ORDONLY ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_mailnew/procmailbox: un=%s\n",un) ;
	    debugprintf("b_mailnew/procmailbox: msfname=%s\n",msfname) ;
	}
#endif

	if ((rs = mailbox_open(&mb,msfname,mbopts)) >= 0) {
	    MBCACHE	mc ;
	    if ((rs = mbcache_start(&mc,msfname,0,&mb)) >= 0) {
	        if ((rs = mbcache_count(&mc)) > 0) {
	            int		mn = rs ;
	            int		mi ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_mailnew/procmailbox: mn=%u\n",mn) ;
#endif

	            for (mi = 0 ; mi < mn ; mi += 1) {

	                c += 1 ;
	                rs = procmailmsg(pip,un,&mc,mi) ;

	                if (rs < 0) break ;
	            } /* end for */

	        } /* end if (n-mesgs) */
	        rs1 = mbcache_finish(&mc) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (mbcache) */
	    rs1 = mailbox_close(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_mailnew/procmailbox: mailbox() rs=%d\n",rs) ;
#endif
	    if (isNotPresent(rs)) rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailnew/procmailbox: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailbox) */


static int procmailmsg(PROGINFO *pip,cchar *un,MBCACHE *mcp,int mi)
{
	LOCINFO		*lip = pip->lip ;
	MBCACHE_SCAN	*msp ;
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailnew/procmailmsg: un=%s mi=%u\n",un,mi) ;
#endif

	if ((rs = mbcache_msgscan(mcp,mi,&msp)) >= 0) {
	    MSGENTRY	*mep ;
	    const int	size = sizeof(MSGENTRY) ;
	    if ((rs = uc_malloc(size,&mep)) >= 0) {
	        time_t		t = msp->htime ;
	        const int	*vl = msp->vl ;
	        const char	**vs = msp->vs ;
	        const char	*dp ;
	        char		ds[DSBUFLEN+1] = { 0 } ;

	        dp = vs[mbcachemf_scandate] ;

	        if (t == 0) t = msp->etime ;

		if (lip->f.datelong || (dp == NULL) || (dp[0] == '\0')) {
		    TMTIME	m ;
		    const int	dl = DSBUFLEN ;
		    if ((rs = tmtime_localtime(&m,t)) >= 0) {
			cchar	*fmt = "%Y%b%d %R" ;
		        if ((rs = sntmtime(ds,dl,&m,fmt)) >= 0) {
			    ds[4] = CHAR_TOLC(ds[4]) ;
			}
		    }
	        } else {
	            if ((dp != NULL) && (dp[0] != '\0')) {
	                strwcpy(ds,(dp+7),5) ;
	            } else {
	                strwcpy(ds,"¿¿",2) ;
		    }
		}

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("b_mailnew/procmailmsg: f=>%s<\n",
	                vs[mbcachemf_scanfrom]) ;
	            debugprintf("b_mailnew/procmailmsg: s=>%s<\n",
	                vs[mbcachemf_hdrsubject]) ;
	        }
#endif /* CF_DEBUG */

		if (rs >= 0) {
	            if ((rs = msgentry_start(mep,un,t,ds,vs,vl)) >= 0) {
	                rs = vechand_add(&lip->msgs,mep) ;
	                if (rs < 0)
	                    msgentry_finish(mep) ;
		    } /* end if (msgentry) */
	        } /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailnew/procmailmsg: msgentry rs=%d\n",rs) ;
#endif

	        if (rs < 0)
	            uc_free(mep) ;
	    } /* end if (memory-allocation) */
	} /* end if (mbcache_msgscan) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailnew/procmailmsg: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmailmsg) */


static int procmaildirs(PROGINFO *pip,PARAMOPT *pop)
{
	int		rs ;
	int		c = 0 ;
	cchar		*po = PO_MAILDIRS ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("procmaildirs: ent\n") ;
#endif

	if ((rs = paramopt_countvals(pop,po)) == 0) {
	    int		i ;
	    cchar	*dns ;
	    cchar	*tp ;

	    for (i = 0 ; varmaildirs[i] != NULL ; i += 1) {
	        if ((dns = getourenv(pip->envv,varmaildirs[i])) != NULL) {
	            while ((tp = strpbrk(dns," :,\t\n")) != NULL) {
	                rs = procmaildir(pip,pop,dns,(tp-dns)) ;
	                c += rs ;
	                dns = (tp+1) ;
	                if (rs < 0) break ;
	            } /* end while */
	            if ((rs >= 0) && (dns[0] != '\0')) {
	                rs = procmaildir(pip,pop,dns,-1) ;
	                c += rs ;
	            } /* end if */
	        }
	        if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
	        const char	*dns = MAILDNAME ;
	        rs = procmaildir(pip,pop,dns,-1) ;
	        c += rs ;
	    }

	} /* end if (do not already have) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("procmaildirs: ret rs=%d c=%u\n",rs,c) ;
#endif

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
	    if ((rs = mkpath1w(dname,dp,dl)) > 0) {
	        struct ustat	sb ;
	        if ((rs = u_stat(dname,&sb)) >= 0) {
	            rs = paramopt_loads(pop,po,dp,dl) ;
	            c += rs ;
		} else if (isNotPresent(rs)) {
		    rs = SR_OK ;
	        }
	    } /* end if */
	} /* end if (have-val?) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmaildir) */


static int procmailusers(PROGINFO *pip,PARAMOPT *app)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	const char	*cp ;

	if (rs >= 0) {
	    rs = procmailusers_arg(pip,app) ;
	    c += rs ;
	}

	if ((rs >= 0) && (c == 0)) {
	    for (i = 0 ; (rs >= 0) && (varmailusers[i] != NULL) ; i += 1) {
	        rs = procmailusers_env(pip,varmailusers[i]) ;
	        c += rs ;
	    } /* end for */
	}

	if (rs >= 0) {
	    rs = procmailusers_def(pip) ;
	    c += rs ;
	}

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    LOCINFO	*lip = pip->lip ;
	    vecstr	*mlp ;
	    mlp = &lip->mailusers ;
	    for (i = 0 ; vecstr_get(mlp,i,&cp) >= 0 ; i += 1) {
	        if (cp != NULL) {
	            shio_printf(pip->efp,"%s: mailuser=%s\n",pn,cp) ;
		}
	    } /* end for */
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers) */


static int procmailusers_env(PROGINFO *pip,cchar *var)
{
	int		rs = SR_OK ;
	int		sl, cl ;
	int		c = 0 ;
	const char	*tp, *sp, *cp ;

	if (var == NULL) return SR_FAULT ;

	if ((sp = getourenv(pip->envv,var)) != NULL) {

	    sl = strlen(sp) ;

	    while ((tp = strnpbrk(sp,sl," :,\t\n")) != NULL) {

	        if ((cl = sfshrink(sp,(tp - sp),&cp)) > 0) {
	            if (cl > USERNAMELEN) cl = USERNAMELEN ;
	            rs = procmailusers_add(pip,cp,cl) ;
	            c += rs ;
	        }

	        sl -= ((tp + 1) - sp) ;
	        sp = (tp + 1) ;

	        if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (sl > 0)) {

	        if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	            rs = procmailusers_add(pip,cp,cl) ;
	            c += rs ;
	        }

	    } /* end if */

	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers_env) */


static int procmailusers_arg(PROGINFO *pip,PARAMOPT *app)
{
	int		rs ;
	int		c = 0 ;
	const char	*po = PO_MAILUSERS ;

	if ((rs = paramopt_havekey(app,po)) > 0) {
	    PARAMOPT_CUR	cur ;
	    int			cl ;
	    const char		*cp ;

	    if ((rs = paramopt_curbegin(app,&cur)) >= 0) {

	        while ((cl = paramopt_enumvalues(app,po,&cur,&cp)) >= 0) {
	            if (cp != NULL) {

	            if ((cp[0] == '-') || (cp[0] == '+')) {
	                cp = pip->username ;
	                cl = -1 ;
	            }
	            rs = procmailusers_add(pip,cp,cl) ;
	            c += rs ;

		    }
	            if (rs < 0) break ;
	        } /* end while */

	        paramopt_curend(app,&cur) ;
	    } /* end if (cursor) */

	} /* end if (mailuser arguments) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers_arg) */


static int procmailusers_def(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	vecstr		*mlp ;
	int		rs ;
	int		c = 0 ;
	int		f = TRUE ;
	const char	*username = pip->username ;

	mlp = &lip->mailusers ;
	if ((rs = vecstr_count(mlp)) > 0) {
	    int		i ;
	    cchar	*cp ;
	    c = rs ;

	    f = FALSE ;
	    for (i = 0 ; vecstr_get(mlp,i,&cp) >= 0 ; i += 1) {
	        if (cp != NULL) {
	            if (strcmp(cp,"+") == 0) {
	                f = TRUE ;
	                c -= 1 ;
	                vecstr_del(mlp,i) ;
	            } else if (strcmp(cp,"-") == 0) {
	                c -= 1 ;
	                vecstr_del(mlp,i) ;
	            } /* end if */
		}
	    } /* end for */

	} /* end if (non-zero) */

	if ((rs >= 0) && (f || (c == 0))) {
	    rs = procmailusers_add(pip,username,-1) ;
	    c += rs ;
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers_def) */


static int procmailusers_add(PROGINFO *pip,cchar *dp,int dl)
{
	LOCINFO		*lip = pip->lip ;
	vecstr		*mlp = &lip->mailusers ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (dp == NULL) return SR_FAULT ;

	if (dl < 0) dl = strlen(dp) ;

	if ((dl > 0) && (dp[0] != '\0')) {
	    if ((rs = vecstr_findn(mlp,dp,dl)) == SR_NOTFOUND) {
	        c += 1 ;
	        rs = vecstr_add(mlp,dp,dl) ;
	    } /* end if (not already) */
	} /* end if (non-zero) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers_add) */


static int procout(PROGINFO *pip,void *ofp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		nshow ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;

	nshow = lip->nshow ;
	if (lip->f.sort) {
	    int (*fn)() = (lip->f.sortrev) ? vcmprev : vcmpfor ;
	    rs = vechand_sort(&lip->msgs,fn) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailnew/procout: nshow=%u\n",nshow) ;
#endif

	if (nshow == 0) nshow = INT_MAX ;

	if (rs >= 0) {
	    HDRDECODE	d ;
	    cchar	*pr = pip->pr ;
	    if ((rs = hdrdecode_start(&d,pr)) >= 0) {
	        VECHAND		*mlp = &lip->msgs ;
	        MSGENTRY	*mep ;
	        int		i ;
	        int		c = 0 ;
	        for (i = 0 ; vechand_get(mlp,i,&mep) >= 0 ; i += 1) {
	            if (mep != NULL) {
	                c += 1 ;
	                rs = procouter(pip,ofp,&d,mep) ;
	                wlen += rs ;
   	            }
	            if (c >= nshow) break ;
	            if (rs < 0) break ;
	        } /* end for */
		rs1 = hdrdecode_finish(&d) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (hdrdecode) */
	} /* end if (ok) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int procouter(PROGINFO *pip,void *ofp,HDRDECODE *hdp,MSGENTRY *mep)
{
	LOCINFO		*lip = pip->lip ;
	OUTINFO		oi ;
	int		rs ;
	int		rs1 ;
	int		ll, dl ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mailnew/procouter: ent\n") ;
#endif

	ll = lip->linelen ;
	dl = (lip->f.datelong) ? 15 : 5 ;
	if ((rs = outinfo_start(&oi,mep,ll,dl)) >= 0) {
	    if ((rs = outinfo_trans(&oi,hdp)) >= 0) {
	        if ((rs = outinfo_calc(&oi)) >= 0) {
		    rs = outinfo_print(&oi,pip,ofp,rs) ;
		    wlen += rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_mailnew/procouter: outinfo_print() rs=%d\n",rs) ;
#endif
		} /* end if (outinfo-calc) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_mailnew/procouter: outinfo_trans() rs=%d\n",rs) ;
#endif
	    } /* end if (outinfo-trans) */
	    rs1 = outinfo_finish(&oi) ;
	    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_mailnew/procouter: outinfo_finish() rs=%d\n",rs1) ;
#endif
	} /* end if (outinfo) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_mailnew/procouter: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procouter) */


static int outinfo_start(OUTINFO *oip,MSGENTRY *mep,int ll,int dl)
{
	int		rs = SR_OK ;

	memset(oip,0,sizeof(OUTINFO)) ;
	oip->mep = mep ;
	oip->ll = ll ;
	oip->dl = dl ;

	return rs ;
}
/* end subroutine (outinfo_start) */


static int outinfo_finish(OUTINFO *oip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("b_mailnew/outinfo_finish: ent\n") ;
	debugprintf("b_mailnew/outinfo_finish: fbuf{%p}\n",oip->fbuf) ;
#endif

	if (oip->fbuf != NULL) {
	    rs1 = uc_free(oip->fbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    oip->fbuf = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("b_mailnew/outinfo_finish: mid1 rs=%d\n",rs) ;
#endif

	if (oip->sbuf != NULL) {
	    rs1 = uc_free(oip->sbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    oip->sbuf = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("b_mailnew/outinfo_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (outinfo_finish) */


static int outinfo_trans(OUTINFO *oip,HDRDECODE *hdp)
{
	MSGENTRY	*mep = oip->mep ;
	int		rs = SR_OK ;
	int		cl ;
	int		bl ;
	cchar		*cp ;
	char		*bp ;

#if	CF_DEBUGS
	debugprintf("b_mailnew/outinfo_trans: ent\n") ;
#endif

/* FROM header value */

	if (rs >= 0) {
	    if ((cl = msgentry_from(mep,&cp)) > 0) {
		const int	c = MIN(cl,MAILADDRLEN) ; /* desired */
		bl = (c*2) ;
		oip->flen = bl ;
		if ((rs = uc_malloc((bl+1),&bp)) >= 0) {
		    oip->fbuf = bp ;
		    if ((rs = outinfo_cvt(oip,hdp,bp,bl,cp,cl,c)) >= 0) {
			oip->fl = rs ;
		    }
		} /* end if (m-a) */
	    } /* end if (msgentry_from) */
	} /* end if (ok) */

#if	CF_DEBUGS
	{
	    debugprintf("b_mailnew/outinfo_trans: from rs=%d\n",rs) ;
	    if (oip->fbuf != NULL) {
		int	rs1 = uc_mallpresent(oip->fbuf) ;
	        debugprintf("b_mailnew/outinfo_trans: mp{fbuf}=%d\n",rs1) ;
	    }
	}
#endif /* CF_DEBUGS */

/* SUBJ header value */

	if (rs >= 0) {
	    if ((cl = msgentry_subj(mep,&cp)) > 0) {
	        const int	c = MIN(cl,SUBJBUFLEN) ; /* desired */
		bl = (c*2) ;
		oip->slen = bl ;
	        if ((rs = uc_malloc((bl+1),&bp)) >= 0) {
		    oip->sbuf = bp ;
	            if ((rs = outinfo_cvt(oip,hdp,bp,bl,cp,cl,c)) >= 0) {
			oip->sl = rs ;
		    }
	        } /* end if (m-a) */
	    } /* end if (msgentry_subj) */
	} /* end if (ok) */

#if	CF_DEBUGS
	{
	    if (oip->fbuf != NULL) {
		int	rs1 = uc_mallpresent(oip->fbuf) ;
	        debugprintf("b_mailnew/outinfo_trans: mp{fbuf}=%d\n",rs1) ;
	    }
	}
	debugprintf("b_mailnew/outinfo_trans: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (outinfo_trans) */


static int outinfo_cvt(OUTINFO *oip,HDRDECODE *hdp,
		char *rbuf,int rlen,cchar *sp,int sl,int c) 
{
	const int	wsize = ((sl+1) * sizeof(wchar_t)) ;
	const int	wlen = sl ;
	int		rs ;
	int		rl = 0 ;
	wchar_t		*wbuf ;
#if	CF_DEBUGS
	debugprintf("b_mailnew/outinfo_cvt: ent sl=%d c=%u\n",sl,c) ;
#endif
	if (oip == NULL) return SR_FAULT ;
	if ((rs = uc_malloc(wsize,&wbuf)) >= 0) {
	    if ((rs = hdrdecode_proc(hdp,wbuf,wlen,sp,sl)) >= 0) {
		const int	n = MIN(c,rs) ;
		int		tlen ;
		char		*tbuf ;
		tlen = (n*2) ;
		if ((rs = uc_malloc((tlen+1),&tbuf)) >= 0) {
		    if ((rs = snwcpywidehdr(tbuf,tlen,wbuf,n)) >= 0) {
			rs = mkdisphdr(rbuf,rlen,tbuf,rs) ;
		        rl = rs ;
		    }
		    uc_free(tbuf) ;
		} /* end if (m-a-f) */
	    } /* end if (hdrdecode_proc) */
	    uc_free(wbuf) ;
	} /* end if (m-a-f) */
#if	CF_DEBUGS
	debugprintf("b_mailnew/outinfo_cvt: ret rs=%d rl=%d\n",rs,rl) ;
#endif
	return (rs >= 0) ? rl : rs ;
}
/* end if (outinfo_cvt) */


static int outinfo_calc(OUTINFO *oip)
{
	MSGENTRY	*mep = oip->mep ;
	int		rs = SR_OK ;
	int		rl = 0 ;
	int		tl ;

#if	CF_DEBUGS
	debugprintf("outinfo_calc: ent\n") ;
#endif

/* setup */

	oip->ul = strlen(mep->user) ;

	if ((rl = sfnormfrom(oip->fbuf,oip->fl)) > 0) {
	    oip->fl = rl ;
	}

/* reductions */

	tl = outinfo_total(oip) ;

#if	CF_DEBUGS
	debugprintf("outinfo_calc: tl=%d\n",tl) ;
#endif

	if (tl > oip->ll) {
	    rl = (tl - oip->ll) ;
	    if ((oip->fl - rl) >= MAXFROMLEN) {
		oip->fl -= rl ;
	        tl = outinfo_total(oip) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("outinfo_calc: tl=%d\n",tl) ;
#endif

	if (tl > oip->ll) {
	    if (oip->fl > MAXFROMLEN) {
		oip->fl = MAXFROMLEN ;
	        tl = outinfo_total(oip) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("outinfo_calc: tl=%d\n",tl) ;
#endif

	if (tl > oip->ll) {
	    rl = (tl - oip->ll) ;
	    oip->sl -= rl ;
	    tl = outinfo_total(oip) ;
	}

#if	CF_DEBUGS
	debugprintf("outinfo_calc: ret rs=%d tl=%d\n",rs,tl) ;
#endif

	return (rs >= 0) ? tl : rs ;
}
/* end if (outinfo_calc) */


static int outinfo_total(OUTINFO *oip)
{
	int	len = (1+3+3) ; /* non-field columns in output string */
	len += (oip->dl+oip->ul+oip->fl+oip->sl) ;
	return len ;
}
/* end if (outinfo_total) */


static int outinfo_print(OUTINFO *oip,PROGINFO *pip,void *ofp,int olen)
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	char		*obuf ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("b_mailnew/outinfo_print: ent olen=%d\n",olen) ;
#endif

	if ((rs = uc_malloc((olen+1),&obuf)) >= 0) {
	    MSGENTRY	*mep = oip->mep ;
	    LOCINFO	*lip = pip->lip ;
	    const int	fl = oip->fl ;
	    const int	sl = oip->sl ;
	    cchar	*pn = pip->progname ;
	    cchar	*db = mep->date ;
	    cchar	*un = mep->user ;
	    cchar	*fb = oip->fbuf ;
	    cchar	*sb = oip->sbuf ;
	    cchar 	*fmt = "%s %s « %t · %t" ;
	    char	tbuf[TIMEBUFLEN+1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        debugprintf("b_mailnew/outinfo_print: db=>%t<\n",
		    db,strlinelen(db,-1,40)) ;
	        debugprintf("b_mailnew/outinfo_print: un=%t\n",
		    un,strlinelen(un,-1,40)) ;
	        debugprintf("b_mailnew/outinfo_print: fl=%d sl=%d\n",fl,sl) ;
	        debugprintf("b_mailnew/outinfo_print: sb=>%t<\n",
		    sb,strlinelen(sb,-1,40)) ;
	    }
#endif /* CF_DEBUGS */

	    if ((rs = bufprintf(obuf,olen,fmt,db,un,fb,fl,sb,sl)) >= 0) {
	        rs = shio_print(ofp,obuf,rs) ;
	        wlen += rs ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("b_mailnew/outinfo_print: "
			"shio_print() rs=%d\n",rs) ;
#endif

	        if (pip->debuglevel > 0) {
		    fmt = "%s: %s u=%s date=%s\n" ;
	            timestr_logz(pip->daytime,tbuf) ;
	            shio_printf(pip->efp,fmt,pn,tbuf,un,db) ;
	        }

	        timestr_logz(pip->daytime,tbuf) ;
	        locinfo_logprintf(lip,"%s u=%s time=%s",tbuf,un,db) ;
	        locinfo_logprintf(lip,"  from=»%t«",fb,fl) ;
	        locinfo_logprintf(lip,"  subj=»%t«",sb,sl) ;

	    rs1 = uc_free(obuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (memory-allocation) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_mailnew/outinfo_print: ret rs=%d wlen=%d\n",
		rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outinfo_print) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;

	if ((rs = vecstr_start(&lip->stores,0,0)) >= 0) {
	    lip->open.stores = TRUE ;
	    if ((rs = vecstr_start(&lip->mailusers,1,0)) >= 0) {
	        lip->open.mailusers = TRUE ;
	        if ((rs = vechand_start(&lip->msgs,5,0)) >= 0) {
	            lip->open.msgs = TRUE ;
	        }
	        if (rs < 0) {
	            lip->open.mailusers = FALSE ;
	            vecstr_finish(&lip->mailusers) ;
	        }
	    } /* end if (mailusers) */
	    if (rs < 0) {
	        lip->open.stores = FALSE ;
	        vecstr_finish(&lip->stores) ;
	    }
	} /* end if (stores) */

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	{
	    MSGENTRY	*mep ;
	    vechand	*mlp = &lip->msgs ;
	    int		i ;
	    for (i = 0 ; vechand_get(mlp,i,&mep) >= 0 ; i += 1) {
	        if (mep != NULL) {
	            rs1 = msgentry_finish(mep) ;
	            if (rs >= 0) rs = rs1 ;
	            rs1 = uc_free(mep) ;
	            if (rs >= 0) rs = rs1 ;
		}
	    } /* end for */
	} /* end block */

	if (lip->open.msgs) {
	    lip->open.msgs = FALSE ;
	    rs1 = vechand_finish(&lip->msgs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.mailusers) {
	    lip->open.mailusers = FALSE ;
	    rs1 = vecstr_finish(&lip->mailusers) ;
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


int locinfo_nshow(LOCINFO *lip,cchar *sp)
{
	int		rs = SR_OK ;

	if (! lip->final.nshow) {
	    if (sp != NULL) {
	        int	v ;
	        lip->final.nshow = TRUE ;
	        lip->have.nshow = TRUE ;
	        rs = cfdeci(sp,-1,&v) ;
	        lip->nshow = v ;
	    }
	} /* end if (needed 'nshow') */

	return rs ;
}
/* end subroutine (locinfo_nshow) */


static int locinfo_optdate(LOCINFO *lip,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	if (vl > 0) {
	    int	oi ;
	    if ((oi = matpstr(datetypes,1,vp,vl)) >= 0) {
		switch (oi) {
		case datetype_long:
		    lip->f.datelong = TRUE ;
		    break ;
		} /* end switch */
	    } /* end if (matpstr) */
	} /* end if (have option-value) */
	return rs ;
}
/* end subroutine (locinfo_optdate) */


#if	CF_LOCEPRINTF
static int locinfo_eprintf(LOCINFO *lip,const char *fmt,...)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (pip->debuglevel > 0) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = shio_vprintf(pip->efp,fmt,ap) ;
	    va_end(ap) ;
	}
	return rs ;
}
/* end subroutine (locinfo_eprintf) */
#endif /* CF_LOCEPRINTF */


static int locinfo_logprintf(LOCINFO *lip,const char *fmt,...)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (pip->open.logprog) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = logfile_vprintf(&pip->lh,fmt,ap) ;
	    va_end(ap) ;
	}
	return rs ;
}
/* end subroutine (locinfo_logprintf) */


/* configuration management */
static int config_start(CONFIG *csp,PROGINFO *pip,PARAMOPT *app,cchar *cfname)
{
	int		rs = SR_OK ;
	char		tbuf[MAXPATHLEN+1] = { 0 } ;

	if (csp == NULL) return SR_FAULT ;

	if (cfname == NULL) cfname = CONFIGFNAME ;

	memset(csp,0,sizeof(CONFIG)) ;
	csp->pip = pip ;
	csp->app = app ;

	if (strchr(cfname,'/') == NULL) {
	    rs = config_findfile(csp,tbuf,cfname) ;
	    if (rs > 0) cfname = tbuf ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: mid rs=%d cfname=%s\n",rs,cfname) ;
#endif

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    shio_printf(pip->efp,"%s: conf=%s\n",pip->progname,cfname) ;
	}

	if (rs >= 0) {
	    if ((rs = paramfile_open(&csp->p,pip->envv,cfname)) >= 0) {
	        if ((rs = config_cookbegin(csp)) >= 0) {
	            csp->f_p = (rs >= 0) ;
	        }
	        if (rs < 0)
	            paramfile_close(&csp->p) ;
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_start) */


static int config_findfile(CONFIG *op,char tbuf[],cchar cfname[])
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;
	int		tl = 0 ;

	tbuf[0] = '\0' ;
	if ((cfname != NULL) && (cfname[0] != '\0')) {
	    VECSTR	sv ;
	    if ((rs = vecstr_start(&sv,6,0)) >= 0) {
	        const int	tlen = MAXPATHLEN ;
	        int		i ;
	        int		vl ;
	        int		kch ;
	        const char	*ks = "pen" ;
	        const char	*vp ;
	        char	kbuf[2] ;
	        kbuf[1] = '\0' ;
	        for (i = 0 ; (rs >= 0) && (ks[i] != '\0') ; i += 1) {
	            kch = (ks[i] & 0xff) ;
	            kbuf[0] = kch ;
	            vp = NULL ;
	            vl = -1 ;
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
	            } /* end switch */
	            if ((rs >= 0) && (vp != NULL)) {
	                rs = vecstr_envset(&sv,kbuf,vp,vl) ;
	            }
	        } /* end for */
	        if (rs >= 0) {
	            rs = permsched(sched1,&sv,tbuf,tlen,cfname,R_OK) ;
	            tl = rs ;
	        }
	        vecstr_finish(&sv) ;
	    } /* end if (finding file) */
	} /* end if (non-null) */

	return (rs >= 0) ? tl : rs ;
}
/* end subroutine (config_findfile) */


static int config_cookbegin(CONFIG *op)
{
	PROGINFO	*pip = op->pip ;
	int		rs ;

	if ((rs = expcook_start(&op->cooks)) >= 0) {
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
	                if (rs >= 0) vp = hbuf ;
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
	            rs = expcook_add(&op->cooks,kbuf,vp,vl) ;
	        }
	    } /* end for */

	    if (rs >= 0) {
	        op->f_cooks = TRUE ;
	    } else {
	        expcook_finish(&op->cooks) ;
	    }
	} /* end if (expcook_start) */

	return rs ;
}
/* end subroutine (config_cookbegin) */


static int config_cookend(CONFIG *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f_cooks) {
	    op->f_cooks = FALSE ;
	    rs1 = expcook_finish(&op->cooks) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (config_cookend) */


static int config_finish(CONFIG *csp)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (csp == NULL) return SR_FAULT ;
	pip = csp->pip ;
	if (pip == NULL) return SR_FAULT ;

	if (csp->f_p) {

	    if (csp->f_cooks) {
	        rs1 = config_cookend(csp) ;
	        if (rs >= 0) rs = rs1 ;
	    }

	    rs1 = paramfile_close(&csp->p) ;
	    if (rs >= 0) rs = rs1 ;

	    csp->f_p = FALSE ;
	} /* end if */

	return rs ;
}
/* end subroutine (config_finish) */


#if	CF_CONFIGCHECK
static int config_check(CONFIG *op)
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;

	if (op->f_p) {
	    if ((rs = paramfile_check(&op->p,pip->daytime)) > 0) {
	        rs = config_read(op) ;
	    }
	}

	return rs ;
}
/* end subroutine (config_check) */
#endif /* CF_CONFIGCHECK */


static int config_read(CONFIG *op)
{
	PROGINFO	*pip = op->pip ;
	LOCINFO		*lip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: f_p=%u\n",op->f_p) ;
#endif

	lip = pip->lip ;
	if (lip == NULL) return SR_FAULT ;

	if (op->f_p) {
	    PARAMOPT		*app = op->app ;
	    PARAMFILE		*pfp = &op->p ;
	    PARAMFILE_CUR	cur ;
	    const int		vlen = VBUFLEN ;
	    const int		elen = EBUFLEN ;
	    int			i ;
	    int			ml, vl, el ;
	    int			v ;
	    cchar		*pr = pip->pr ;
	    char		vbuf[VBUFLEN + 1] ;
	    char		ebuf[EBUFLEN + 1] ;
	    for (i = 0 ; cparams[i] != NULL ; i += 1) {

	        if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {
	            char	tbuf[MAXPATHLEN + 1] ;

	            while (rs >= 0) {
	                vl = paramfile_fetch(pfp,cparams[i],&cur,vbuf,vlen) ;
	                if (vl == SR_NOTFOUND) break ;
	                rs = vl ;
	                if (rs < 0) break ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("config_read: vbuf=>%t<\n",vbuf,vl) ;
#endif

	                ebuf[0] = '\0' ;
	                el = 0 ;
	                if (vl > 0) {
	                    el = expcook_exp(&op->cooks,0,ebuf,elen,vbuf,vl) ;
	                    if (el >= 0) ebuf[el] = '\0' ;
	                }

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("config_read: ebuf=>%t<\n",ebuf,el) ;
#endif

	                switch (i) {
	                case cparam_logsize:
	                    if (el > 0) {
	                        if (cfdecmfi(ebuf,el,&v) >= 0) {
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
	                case cparam_maildir:
	                    ml = prsetfname(pr,tbuf,ebuf,el,TRUE,
	                        NULL,MAILDNAME,"") ;
	                    if (ml > 0) {
	                        rs = procmaildir(pip,app,tbuf,ml) ;
			    }
	                    break ;
	                case cparam_logfile:
	                    if (el > 0) {
	                        if (! pip->final.lfname) {
	                            pip->final.lfname = TRUE ;
	                            pip->have.lfname = TRUE ;
	                            rs = config_setlfname(op,ebuf,el) ;
	                        }
	                    }
	                    break ;
	                } /* end switch */

	            } /* end while (fetching) */

	            paramfile_curend(pfp,&cur) ;
	        } /* end if (cursor) */

	        if (rs < 0) break ;
	    } /* end for (parameters) */
	} /* end if (active) */

	return rs ;
}
/* end subroutine (config_read) */


static int config_setlfname(CONFIG *cfp,cchar *vp,int vl)
{
	PROGINFO	*pip = cfp->pip ;
	const char	*pr ;
	const char	*sn ;
	const char	*lfn ;
	int		rs ;
	char		tbuf[MAXPATHLEN+1] ;

	pr = pip->pr ;
	sn = pip->searchname ;
	lfn = pip->lfname ;
	if ((rs = prsetfname(pr,tbuf,vp,vl,TRUE,LOGCNAME,sn,"")) >= 0) {
	    const int	tl = rs ;
	    if ((lfn == NULL) || (strcmp(lfn,tbuf) != 0)) {
	        const char	**vpp = &pip->lfname ;
	        pip->changed.lfname = TRUE ;
	        rs = proginfo_setentry(pip,vpp,tbuf,tl) ;
	    }
	}

	return rs ;
}
/* end subroutine (config_setlfname) */


static int msgentry_start(mep,userp,t,datep,vs,vl)
MSGENTRY	*mep ;
const char	*userp ;
time_t		t ;
const char	*datep ;
const char	*vs[] ;
const int	vl[] ;
{
	int		rs = SR_OK ;
	int		ul ;
	int		dl = 0 ;
	int		vi ;
	int		size = 0 ;
	char		*bp ;

	if (mep == NULL) return SR_FAULT ;
	if (userp == NULL) return SR_FAULT ;
	if (vs == NULL) return SR_FAULT ;
	if (vl == NULL) return SR_FAULT ;

	memset(mep,0,sizeof(MSGENTRY)) ;
	mep->mdate = t ;

	size += ((ul = strlen(userp)) + 1) ;
	if (datep != NULL) size += ((dl = strlen(datep)) + 1) ;
	vi = mbcachemf_scanfrom ;
	if (vs[vi] != NULL) size += (vl[vi] + 1) ;
	vi = mbcachemf_hdrsubject ;
	if (vs[vi] != NULL) size += (vl[vi] + 1) ;

	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    mep->a = bp ;

	    if (datep != NULL) {
	        mep->date = bp ;
	        bp = (strwcpy(bp,datep,dl) + 1) ;
	    }

	    mep->user = bp ;
	    bp = (strwcpy(bp,userp,ul) + 1) ;

	    vi = mbcachemf_scanfrom ;
	    if (vs[vi] != NULL) {
	        mep->from = bp ;
	        bp = (strwcpy(bp,vs[vi],vl[vi]) + 1) ;
	    } else {
	        mep->from = (bp-1) ;
	    }

	    vi = mbcachemf_hdrsubject ;
	    if (vs[vi] != NULL) {
	        mep->subj = bp ;
	        bp = (strwcpy(bp,vs[vi],vl[vi]) + 1) ;
	    } else {
	        mep->subj = (bp-1) ;
	    }

	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (msgentry_start) */


static int msgentry_finish(MSGENTRY *mep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mep->a != NULL) {
	    rs1 = uc_free(mep->a) ;
	    if (rs >= 0) rs = rs1 ;
	    mep->a = NULL ;
	}

	return rs ;
}
/* end subroutine (msgentry_finish) */


static int msgentry_from(MSGENTRY *mep,const char **rpp)
{
	int		rs = SR_OK ;
	int		cl ;
	cchar		*s = mep->from ;
	cchar		*cp ;

	if (mep->a == NULL) rs = SR_NOTFOUND ;

	cl = sfshrink(s,-1,&cp) ;

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? cp : NULL ;
	}

	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (msgentry_from) */


static int msgentry_subj(MSGENTRY *mep,const char **rpp)
{
	int		rs = SR_OK ;
	int		cl ;
	cchar		*s = mep->subj ;
	cchar		*cp ;

	if (mep->a == NULL) rs = SR_NOTFOUND ;

	cl = sfshrink(s,-1,&cp) ;

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? cp : NULL ;
	}

	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (msgentry_subj) */


static int mkmsfname(char rbuf[],cchar mbuf[],int mlen,cchar *mup)
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,mbuf,mlen) ;
	    i += rs ;
	}

	if ((rs >= 0) && (i > 0) && (rbuf[i-1] != '/')) {
	    rs = storebuf_char(rbuf,rlen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,mup,-1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkmsfname) */


static int sfnormfrom(cchar *fp,int fl)
{
	const char	*tp ;
	if ((tp = strnchr(fp,fl,',')) != NULL) {
	    fl = (tp-fp) ;
	    while (fl && CHAR_ISWHITE(fp[fl-1])) fl -= 1 ;
	}
	return fl ;
}
/* end subroutine (sfnormfrom) */


static int vcmpfor(const void *v1pp,const void *v2pp)
{
	MSGENTRY	**e1pp = (MSGENTRY **) v1pp ;
	MSGENTRY	**e2pp = (MSGENTRY **) v2pp ;
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = ((*e1pp)->mdate - (*e2pp)->mdate) ;
	        } else
	            rc = -1 ;
	    } else
		rc = 1 ;
	}
	return rc ;
}
/* end subroutine (vcmpfor) */


static int vcmprev(const void *v1pp,const void *v2pp)
{
	MSGENTRY	**e1pp = (MSGENTRY **) v1pp ;
	MSGENTRY	**e2pp = (MSGENTRY **) v2pp ;
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = - ((*e1pp)->mdate - (*e2pp)->mdate) ;
	        } else
	            rc = -1 ;
	    } else
		rc = 1 ;
	}
	return rc ;
}
/* end subroutine (vcmprev) */


