/* main (RBBPOST) */

/* RBBPOST news spooler */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_DEBUGCONF	0		/* debug PCSCONF */


/* revision history:

	= 1995-05-01, David A­D­ Morano
	This code module was completely rewritten to replace any original
	garbage that was here before.

	= 1998-06-01, David A­D­ Morano
	I modified the code to change the effective UID to 'pcs' so that the
	program could properly access the newsgroup spool area under all
	circumstances.

	= 2008-10-07, David A­D­ Morano
	This was modified to allow for the new PCSCONF facility that used
	loadable modules for polling things.

*/

/* Copyright © 1995,1998,2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is probably a pretty-much generic main subroutine.  This is for
	the RBBPOST program.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/timeb.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	<vsystem.h>
#include	<umask.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<tmz.h>
#include	<tmtime.h>
#include	<timestr.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<userinfo.h>
#include	<pcsconf.h>
#include	<pcspoll.h>
#include	<field.h>
#include	<vecitem.h>
#include	<ema.h>
#include	<ascii.h>
#include	<buffer.h>
#include	<nulstr.h>
#include	<hdbstr.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"ng.h"
#include	"article.h"
#include	"bbhosts.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	pathadd(char *,int,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	mkuibang(char *,int,USERINFO *) ;
extern int	mkuiname(char *,int,USERINFO *) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	initnow(struct timeb *,char *,int) ;
extern int	makedirs(cchar *,int,mode_t) ;
extern int	prmktmpdir(cchar *,char *,cchar *,cchar *,mode_t) ;
extern int	ema_haveaddr(EMA *,cchar *,int) ;
extern int	emaentry_getbestaddr(EMAENTRY *,cchar **) ;
extern int	isdigitlatin(int) ;

extern int	pcsgetprog(cchar *,char *,cchar *) ;
extern int	pcsngdname(cchar *,char *,cchar *,cchar *) ;
extern int	pcsname(cchar *,char *,int,cchar *) ;
extern int	pcsfullname(cchar *,char *,int,cchar *) ;
extern int	isNotPresent(int) ;
extern int	isFailOpen(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	proglog_begin(PROGINFO *,USERINFO *) ;
extern int	proglog_end(PROGINFO *) ;
extern int	proglog_printf(PROGINFO *,cchar *,...) ;

extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;

extern int	prognamecache_begin(PROGINFO *,USERINFO *) ;
extern int	prognamecache_end(PROGINFO *) ;

extern int	progartmaint(PROGINFO *,struct tdinfo *) ;
extern int	progarts(PROGINFO *,struct tdinfo *,vechand *,bfile *,
			vecstr *) ;

#ifdef	COMMENT
extern int	broadcast(PROGINFO *,vecitem *,BBHOSTS *,BBHOSTS *) ;
#endif

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		ngmap:1 ;
	uint		altuser:1 ;
	uint		squery:1 ;
	uint		onckey:1 ;
	uint		prlocal:1 ;
	uint		clustername:1 ;
	uint		nisdomain:1 ;
	uint		pcsorg:1 ;
	uint		pcsuserorg:1 ;
	uint		org:1 ;
	uint		name:1 ;
	uint		fullname:1 ;
	uint		to:1 ;
	uint		envfrom:1 ;
	uint		hdrfroms:1 ;
	uint		def_from:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	EMA		hdrfroms ;
	HDBSTR		ngmap ;
	PROGINFO	*pip ;
	cchar		*prlocal ;
	cchar		*clustername ;
	cchar		*nisdomain ;
	cchar		*pcsorg ;
	cchar		*pcsuserorg ;
	cchar		*org ;
	cchar		*name ;
	cchar		*fullname ;
	cchar		*hdrfromname ;
	cchar		*hdrfromaddr ;
	uid_t		uid_pcs ;
	gid_t		gid_pcs ;
	int		to ;		/* time-out */
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procbase(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *,cchar *) ;
static int	procargs(PROGINFO *,struct tdinfo *,ARGINFO *,BITS *,
			cchar *,cchar *,cchar *) ;
static int	procspecs(PROGINFO *,vecstr *,cchar *,int) ;
static int	procinput(PROGINFO *,struct tdinfo *,vecstr *,cchar *) ;

static int	procartloads(PROGINFO *,struct tdinfo *,VECHAND *,vecstr *) ;
static int	procartload(PROGINFO *,struct tdinfo *,ARTICLE *,cchar *,int) ;
static int	procartloader(PROGINFO *,struct tdinfo *,char *,int,cchar *) ;

static int	procartdel(PROGINFO *,struct tdinfo *,ARTICLE *) ;

static int	procartfins(PROGINFO *,vechand *) ;
static int	procnewsdname(PROGINFO *pip) ;
static int	procartdname(PROGINFO *,struct tdinfo *) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procpcsconf_begin(PROGINFO *,PCSCONF *) ;
static int	procpcsconf_end(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_hdrfrom(LOCINFO *,cchar *,int) ;
static int	locinfo_hdrfromget(LOCINFO *,EMA **) ;
static int	locinfo_mkhdrfrom(LOCINFO *) ;
static int	locinfo_mkhdrfromname(LOCINFO *) ;
static int	locinfo_mkhdrfromaddr(LOCINFO *) ;
static int	locinfo_ngdname(LOCINFO *,char *,cchar *,int) ;

#ifdef	COMMENT
static int	pcsconf_mkdir() ;
#endif


/* global variables */


/* local variables */

static cchar *argopts[] = {
	"ROOT",
	"VERSION",
	"TMPDIR",
	"HELP",
	"MAILER",
	"RSLOW",
	"from",
	"subject",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"cf",
	"lf",
	"et",
	"expires",
	"maint",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_tmpdir,
	argopt_help,
	argopt_mailer,
	argopt_rslow,
	argopt_from,
	argopt_subject,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_cf,
	argopt_lf,
	argopt_et,
	argopt_expire,
	argopt_maint,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
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
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static cchar *akonames[] = {
	"squery",
	NULL
} ;

enum akonames {
	akoname_squery,
	akoname_overlast
} ;

#ifdef	COMMENT

static cchar	*configkeys[] = {
	"newsdir",
	"timestamp",
	"mincheck",
	NULL
} ;

enum configkeys {
	configkey_newsdir,
	configkey_timestamp,
	configkey_mincheck,
	configkey_overlast
} ;

#endif /* COMMENT */

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
	TMZ		stz ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai,ai_max, ai_pos ;
	int		rs, rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*ifname = NULL ;
	cchar		*cfname = NULL ;
	cchar		*ndname = NULL ;
	cchar		*remote_machine = NULL ;
	cchar		*remote_transport = NULL ;
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

	memset(&stz,0,sizeof(TMZ)) ;

	setumask(0002) ;

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize some stuff before command line argument processing */

	pip->verboselevel = 1 ;
	pip->f.logprog = TRUE ;
	pip->f.logenv = TRUE ;
	pip->f.logmsg = TRUE ;
	pip->f.logzone = TRUE ;

	pip->lip = lip ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

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

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* from */
	                case argopt_from:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            if (argp[0] != '-') {
	                                rs = locinfo_hdrfrom(lip,argp,argl) ;
	                            }
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* header value subject */
	                case argopt_subject:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->msgsubject = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* mailer program */
	                case argopt_mailer:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->prog_mailer = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* mailer RSLOW */
	                case argopt_rslow:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->prog_rslow = argp ;
	                    } else
	                        rs = SR_INVALID ;
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

/* argument-list file name */
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

/* input file-name */
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

/* expiration specification */
	                case argopt_et:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = tmz_toucht(&stz,argp,argl) ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* article-expire mode */
	                case argopt_expire:
	                    pip->f.artexpires = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.artexpires = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* article-maintenance mode */
	                case argopt_maint:
	                    pip->f.artmaint = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.artmaint = (rs > 0) ;
	                        }
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

/* mailer program */
	                    case 'M':
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->prog_mailer = avp ;
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl)
	                                    pip->prog_mailer = argp ;
	                            } else
	                                rs = SR_INVALID ;
	                        }
	                        break ;

/* alternate newsgroup spool area */
	                    case 'N':
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                ndname = avp ;
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl)
	                                    ndname = argp ;
	                            } else
	                                rs = SR_INVALID ;
	                        }
	                        break ;

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'E':
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

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* expiration specification */
	                    case 'e':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                rs = tmz_touch(&stz,argp,argl) ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* header value FROM address */
	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->have.hdrfroms = TRUE ;
	                                lip->final.hdrfroms = TRUE ;
	                                if (argp[0] == '+') {
	                                    lip->f.def_from = TRUE ;
	                                } else if (argp[0] != '-') {
					    const int	al = argl ;
					    cchar	*ap = argp ;
	                                    rs = locinfo_hdrfrom(lip,ap,al) ;
	                                }
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* header value SUBJECT */
	                    case 's':
	                        pip->subjectmode = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->msgsubject = avp ;
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl)
	                                    pip->msgsubject = argp ;
	                            }
	                        }
	                        break ;

/* quiet mode */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

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

	        } /* end if */

	    } else {

	        rs = bits_set(&pargs,ai) ;
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
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

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
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    rs = printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialization after argument capture */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getenv(VARAFNAME) ;
	if (ifname == NULL) ifname = getenv(VARIFNAME) ;
	if (cfname == NULL) cfname = getenv(VARCFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (rs >= 0) {
	    rs = initnow(&pip->now,pip->zname,DATER_ZNAMESIZE) ;
	    pip->daytime = pip->now.time ;
	}


/* program options */

	if (rs >= 0) {
	    if ((rs = procopts(pip,&akopts)) >= 0) {
	        if (pip->lfname == NULL) pip->lfname = getenv(VARLFNAME) ;
	        if ((rs = tmz_isset(&stz)) > 0) {
	            TMTIME	tmt ;
	            if (! stz.f.year) {
	                rs = tmtime_localtime(&tmt,pip->daytime) ;
	                stz.st.tm_year = tmt.year ;
	            } /* end if (getting the current year) */
	            if (rs >= 0) {
	                time_t	t ;
	                tmtime_insert(&tmt,&stz.st) ;
	                if ((rs = tmtime_mktime(&tmt,&t)) >= 0) {
	                    pip->ti_expires = t ;
			}
	            }
	        } /* end if (expiration specified) */
	    } /* end if (procopts) */
	} /* end if (ok) */

	pip->newsdname = ndname ;

	timestr_logz(pip->daytime,pip->stamp) ;

/* argument information */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USERINFO	u ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	            PCSCONF	pc, *pcp = &pc ;
	            cchar	*pr = pip->pr ;
	            if (cfname != NULL) {
	                if (pip->euid != pip->uid) u_seteuid(pip->uid) ;
	                if (pip->egid != pip->gid) u_setegid(pip->gid) ;
	            }
	            if ((rs = pcsconf_start(pcp,pr,envv,cfname)) >= 0) {
	                pip->pcsconf = pcp ;
	                pip->open.pcsconf = TRUE ;
	                if ((rs = procpcsconf_begin(pip,pcp)) >= 0) {
	                    PCSPOLL		poll ;
	                    cchar		*sn = pip->searchname ;
	                    if ((rs = pcspoll_start(&poll,pcp,sn)) >= 0) {
	                        if ((rs = proglog_begin(pip,&u)) >= 0) {
	                            if ((rs = proguserlist_begin(pip)) >= 0) {
	                                ARGINFO	*aip = &ainfo ;
				        BITS	*bop = &pargs ;
	                                cchar	*afn = afname ;
	                                cchar	*ofn = ofname ;
	                                cchar	*ifn = ifname ;
    
	                                rs = procbase(pip,aip,bop,afn,ofn,ifn) ;
    
	                                rs1 = proguserlist_end(pip) ;
	                                if (rs >= 0) rs = rs1 ;
	                            } /* end if (proguserlist) */
	                            rs1 = proglog_end(pip) ;
	                            if (rs >= 0) rs = rs1 ;
	                        } /* end if (proglog) */
	                        rs1 = pcspoll_finish(&poll) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (pcspoll) */
#if	CF_DEBUG
	                    if (DEBUGLEVEL(2))
	                        debugprintf("main: pcspoll_start() rs=%d\n",
				rs) ;
#endif
	                    rs1 = procpcsconf_end(pip) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (procpcsconf) */
	                pip->open.pcsconf = FALSE ;
	                pip->pcsconf = NULL ;
	                rs1 = pcsconf_finish(pcp) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (pcsconf) */
    
#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: pcsconf_start() rs=%d\n",rs) ;
#endif

	            rs1 = procuserinfo_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else if (rs < 0) {
	        fmt = "%s: userinfo failure (%d)\n" ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	        ex = EX_NOUSER ;
	    }
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    fmt = "%s: invalid argument or configuration (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    if (! pip->f.quiet) {
	        bprintf(pip->efp,"%s: could not process (%d)\n",
	            pip->progname,rs) ;
	    }
	    ex = mapex(mapexs,rs) ;
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
	    keyopt_finish(&akopts) ;
	    pip->open.akopts = FALSE ;
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
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final rs=%d\n",rs) ;
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

/* handle the bad stuff */
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

	fmt = "%s: USAGE> %s [<newsgroup(s)> ...] [-af <afile>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-s <subject>] [-f <from>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {
	                switch (oi) {

	                case akoname_squery:
	                    if (! lip->final.squery) {
	                        lip->have.squery = TRUE ;
	                        lip->final.squery = TRUE ;
	                        lip->f.squery = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.squery = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */
	                c += 1 ;
	            } /* end if (valid option) */

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopts-cursor */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procuserinfo_begin(pip,uip)
PROGINFO	*pip ;
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

	pip->uid_pcs = -1 ;
	pip->gid_pcs = -1 ;

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

	pip->envfromaddr = pip->username ;

	if (rs >= 0)
	    rs = prognamecache_begin(pip,uip) ;

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(pip)
PROGINFO	*pip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

	rs1 = prognamecache_end(pip) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (procuserinfo_end) */


static int procpcsconf_begin(PROGINFO *pip,PCSCONF *pcp)
{
	int		rs = SR_OK ;

	if (pip->open.pcsconf) {

	    if (pip->uid_pcs < 0) {
	        if ((rs = pcsconf_getpcsuid(pcp)) >= 0) {
	            pip->uid_pcs = rs ;
	            if ((rs = pcsconf_getpcsgid(pcp)) >= 0) {
	                pip->gid_pcs = rs ;
	            }
	        }
	    } /* end if (PCS IDs) */

#if	CF_DEBUG && CF_DEBUGCONF
	    if (DEBUGLEVEL(3)) {
	        PCSCONF_CUR	cur ;
	        if ((rs = pcsconf_curbegin(pcp,&cur)) >= 0) {
	            const int	klen = KBUFLEN ;
	            const int	vlen = VBUFLEN ;
	            int		vl ;
	            char	kbuf[KBUFLEN+1] ;
	            char	vbuf[VBUFLEN+1] ;
	            while (rs >= 0) {
	                vl = pcsconf_enum(pcp,&cur,kbuf,klen,vbuf,vlen) ;
	                if (vl == SR_NOTFOUND) break ;
	                debugprintf("main/procpcsconf: pair> %s=%t\n",
	                    kbuf,vbuf,vl) ;
	            } /* end while */
	            pcsconf_curend(pcp,&cur) ;
	        } /* end if (cursor) */
	    }
#endif /* CF_DEBUG */

	} /* end if (configured) */

	return rs ;
}
/* end subroutine (procpcsconf_begin) */


static int procpcsconf_end(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (procpcsconf_end) */


static int procbase(PROGINFO *pip,ARGINFO *aip,BITS *app,cchar *afn,cchar *ofn,
		cchar *ifn)
{
	int		rs ;
	cchar		*zn = pip->zname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procbase: ent\n") ;
#endif

	if ((rs = procnewsdname(pip)) >= 0) {
	    struct tdinfo	ti ;

	    if ((rs = procartdname(pip,&ti)) >= 0) {
	        DATER		*tdp = &pip->td ;
	        struct timeb	*nowp = &pip->now ;
	        if ((rs = dater_start(tdp,nowp,zn,-1)) >= 0) {

	            if (pip->f.artexpires || pip->f.artmaint) {
	                rs = progartmaint(pip,&ti) ;
	            } else
	                rs = procargs(pip,&ti,aip,app,afn,ofn,ifn) ;

	            dater_finish(tdp) ;
	        } /* end if (dater) */
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procbase: dater-out rs=%d\n",rs) ;
#endif
	    } /* end if (proc-article-dname) */

	} /* end if (proc-news-dname) */

	return rs ;
}
/* end subroutine (procbase) */


static int procargs(pip,tip,aip,app,afn,ofn,ifn)
PROGINFO	*pip ;
struct tdinfo	*tip ;
ARGINFO		*aip ;
BITS		*app ;
cchar		*afn ;
cchar		*ofn ;
cchar		*ifn ;
{
	vecstr		ngs, *nlp = &ngs ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procargs: ent\n") ;
#endif

	if ((rs = vecstr_start(nlp,1,0)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    cchar	*cp ;

	    if (rs >= 0) {
	    int		ai ;
	    int		f ;
	    for (ai = 1 ; ai < aip->argc ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(app,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	        if (f) {
	            cp = aip->argv[ai] ;
		    if (cp[0] != '\0') {
	        	pan += 1 ;
	        	rs = vecstr_adduniq(nlp,cp,-1) ;
			if (rs < INT_MAX) c += 1 ;
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
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procspecs(pip,nlp,cp,cl) ;
	                        c += rs ;
	                    }
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

	    if ((rs >= 0) && (pan == 0)) {
	        pan += 1 ;
	        rs = vecstr_adduniq(nlp,"test",-1) ;
	        if (rs < INT_MAX) c += 1 ;
	    } /* end if (environment variable) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        cchar	*cp ;
	        int	i ;
	        for (i = 0 ; vecstr_get(nlp,i,&cp) >= 0 ; i += 1)
	            debugprintf("main/procargs: ng=%s\n",cp) ;
	    }
#endif

	    if (rs >= 0) {
	        rs = procinput(pip,tip,nlp,ifn) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procargs: process() rs=%d\n",rs) ;
#endif
	    }

	    rs1 = vecstr_finish(nlp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (news-groups) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procargs: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


static int procspecs(PROGINFO *pip,vecstr *nlp,cchar *lbuf,int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;
	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl >= 0) {
	   	    rs = vecstr_adduniq(nlp,fp,fl) ;
	            if (rs < INT_MAX) c += 1 ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspecs) */


static int procinput(PROGINFO *pip,struct tdinfo *tip,vecstr *nlp,cchar *ifn)
{
	vechand		arts, *alp = &arts ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procinput: ent\n") ;
#endif

	if ((rs = vechand_start(alp,1,0)) >= 0) {
	    const mode_t	m = 0666 ;
	    bfile		ifile, *ifp = &ifile ;
	    cchar		*ifn = ifn ;

	    if ((ifn == NULL) || (ifn[0] == '\0') || (ifn[0] == '-')) 
		ifn = BFILE_STDIN ;

	    if ((rs = bopen(ifp,ifn,"r",m)) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/process: progarts()\n") ;
#endif
	        rs = progarts(pip,tip,alp,ifp,nlp) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/process: "
	                "progarts() rs=%d\n",
	                rs) ;
#endif
	        if ((rs >= 0) && pip->open.logprog) {
	            cchar	*fmt = "msgs-input=%u" ;
	            proglog_printf(pip,fmt,rs) ;
	        }

	        rs1 = bclose(ifp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (bfile-open) */
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/process: bopen-out rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        rs = procartloads(pip,tip,alp,nlp) ;
	        c += rs ;
	    }

	    rs1 = procartfins(pip,alp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = vechand_finish(alp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (arts) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/process: vechand-out rs=%d\n",
	        rs) ;
#endif

	if ((rs >= 0) && pip->open.logprog) {
	    proglog_printf(pip,"msgs-posted=%u",c) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procinput: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procinput) */


static int procartloads(pip,tip,alp,nlp)
PROGINFO	*pip ;
struct tdinfo	*tip ;
VECHAND		*alp ;
vecstr		*nlp ;
{
	ARTICLE		*aip ;
	EMA		*emap ;
	const int	at = articleaddr_newsgroups ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ai ;
	int		c = 0 ;

	for (ai = 0 ; vechand_get(alp,ai,&aip) >= 0 ; ai += 1) {
	    if (aip == NULL) continue ;

	    if ((rs = article_getaddrema(aip,at,&emap)) >= 0) {
	        EMAENTRY	*ep ;
	        int		i ;
	        int		nl ;
	        int		ac = 0 ;
	        cchar	*np ;

	        for (i = 0 ; ema_get(emap,i,&ep) >= 0 ; i += 1) {
	            if ((rs = emaentry_getbestaddr(ep,&np)) > 0) {
	                nl = rs ;
	                c += 1 ;
	                rs = procartload(pip,tip,aip,np,nl) ;
	            }
	            if (rs < 0) break ;
	        } /* end for */

	        if (rs >= 0) {
	            for (i = 0 ; vecstr_get(nlp,i,&np) >= 0 ; i += 1) {
	                if (np == NULL) continue ;
	                nl = strlen(np) ;
	                if ((rs = ema_haveaddr(emap,np,nl)) == 0) {
	                    ac += 1 ;
	                    rs = procartload(pip,tip,aip,np,nl) ;
	                }
	                if (rs < 0) break ;
	            } /* end for */
	        } /* end if */

	        if ((rs < 0) || (ac == 0)) {
	            rs1 = procartdel(pip,tip,aip) ;
	            if (rs >= 0) rs = rs1 ;
	        }

	        c += ac ;
	    } /* end if (article-get-addr) */

	    if (rs < 0) break ;
	} /* end for (articles) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procartloads) */


static int procartload(pip,tip,aip,np,nl)
PROGINFO	*pip ;
struct tdinfo	*tip ;
ARTICLE		*aip ;
cchar	*np ;
int		nl ;
{
	LOCINFO	*lip = pip->lip ;
	const int	st = articlestr_articleid ;
	int		rs ;
	int		c = 0 ;
	cchar	*pn = pip->progname ;
	cchar	*sp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procartload: ent ng=%t\n",np,nl) ;
#endif

	if (nl < 0) nl = strlen(np) ;

	if ((rs = article_getstr(aip,st,&sp)) >= 0) {
	    if ((rs = pathadd(tip->tdname,tip->tdlen,sp)) >= 0) {
	        char	ngdname[MAXPATHLEN+1] ;

	        if ((rs = locinfo_ngdname(lip,ngdname,np,nl)) > 0) {
	            const int	ngdlen = rs ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procartload: "
	                    "ngdname=%t\n",np,nl) ;
#endif

	            rs = procartloader(pip,tip,ngdname,ngdlen,sp) ;
	            if (rs >= 0) c += 1 ;

	            if (pip->debuglevel > 0) {
	                cchar	*fmt ;
	                if (rs >= 0) {
	                    fmt = "%s: posted ng=%t\n",
	                        bprintf(pip->efp,fmt,pn,np,nl) ;
	                } else {
	                    fmt = "%s: error ng=%t (%d)\n",
	                        bprintf(pip->efp,fmt,pn,np,nl,rs) ;
	                }
	            } /* end if (debugging) */
	            if (rs == SR_EXISTS) rs = SR_OK ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procartload: u_link() rs=%d\n",
	                    rs) ;
#endif

	        } else if ((rs == 0) || (rs == SR_NOTFOUND)) {
	            if (pip->debuglevel > 0)
	                bprintf(pip->efp,"%s: not-found ng=%t\n",
	                    pn,np,nl) ;
	            rs = SR_OK ;
	        } /* end if (NG-directory) */

	    } /* end if (pathadd) */
	    tip->tdname[tip->tdlen] = '\0' ;
	} /* end if (article-getstr) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procartload: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procartload) */


static int procartloader(pip,tip,ngdname,ngdlen,sp)
PROGINFO	*pip ;
struct tdinfo	*tip ;
char		ngdname[] ;
int		ngdlen ;
cchar	*sp ;
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = uc_stat(ngdname,&sb)) >= 0) {
	    if ((rs = pathadd(ngdname,ngdlen,sp)) >= 0) {
	        if (tip->dev == sb.st_dev) {
	            rs = u_link(tip->tdname,ngdname) ;
	        } else {
	            bfile	ofile, *ofp = &ofile ;
	            if ((rs = bopen(ofp,ngdname,"wct",0666)) >= 0) {
	                bfile	ifile, *ifp = &ifile ;
	                if ((rs = bopen(ifp,tip->tdname,"r",0666)) >= 0) {
	                    rs = bwriteblock(ofp,ifp,-1) ;
	                    bclose(ifp) ;
	                } /* end if (open-input-file) */
	                bclose(ofp) ;
	            } /* end if (create-file) */
	        } /* end if (same or different file-system) */
	    } /* end if (pathadd) */
	} /* end if (stat) */

	return rs ;
}
/* end subroutine (procartloader) */


static int procartdel(pip,tip,aip)
PROGINFO	*pip ;
struct tdinfo	*tip ;
ARTICLE		*aip ;
{
	const int	st = articlestr_articleid ;
	int		rs ;
	cchar	*sp ;

	if (pip == NULL) return SR_FAULT ;
	if (aip == NULL) return SR_FAULT ;

	if ((rs = article_getstr(aip,st,&sp)) >= 0) {
	    if ((rs = pathadd(tip->tdname,tip->tdlen,sp)) >= 0) {

	        uc_unlink(tip->tdname) ;

	    } /* end if (pathadd) */
	    tip->tdname[tip->tdlen] = '\0' ;
	} /* end if (article-getstr) */

	return rs ;
}
/* end subroutine (procloaddel) */


static int procartfins(PROGINFO *pip,vechand *alp)
{
	ARTICLE		*aip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procartfins: ent\n") ;
#endif

	if (pip == NULL) return SR_FAULT ;

	for (i = 0 ; vechand_get(alp,i,&aip) >= 0 ; i += 1) {
	    if (aip == NULL) continue ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procartfins: article_finish()\n") ;
#endif
	    rs1 = article_finish(aip) ;
	    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procartfins: article_finish() rs=%d\n",rs) ;
#endif
	    rs1 = uc_free(aip) ;
	    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procartfins: for-bot\n") ;
#endif
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procartfins: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procartfins) */


static int procnewsdname(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procnewsdname: ent\n") ;
#endif

	if (pip->newsdname == NULL) pip->newsdname = getenv(VARNEWSDNAME) ;

	if ((pip->newsdname == NULL) && pip->open.pcsconf) {
	    PCSCONF	*pcp = pip->pcsconf ;
	    PCSCONF_CUR	cur ;
	    const int	vlen = VBUFLEN ;
	    char	vbuf[VBUFLEN+1] ;
	    if ((rs = pcsconf_curbegin(pcp,&cur)) >= 0) {
	        cchar	*k = "bb:newsdir" ;
	        if ((rs1 = pcsconf_fetch(pcp,k,-1,&cur,vbuf,vlen)) >= 0) {
	            int	vl = rs1 ;
	            if (vl > 0) {
	                cchar	**vpp = &pip->newsdname ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("main/procnewsdname: d=>%t<\n",
	                        vbuf,vl) ;
#endif
	                rs = proginfo_setentry(pip,vpp,vbuf,vl) ;
	            }
	        } /* end if (paramconf_fetch) */
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procnewsdname: pcsconf_fetch() rs=%d\n",
	                rs1) ;
#endif
	        rs1 = pcsconf_curend(pcp,&cur) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procnewsdname: pcsconf_curend() rs=%d\n",
	                rs1) ;
#endif
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (PCS-cursor) */
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procnewsdname: cur-out rs=%d\n",rs) ;
#endif
	} /* end if (pcsconf) */

	if (pip->newsdname == NULL) pip->newsdname = NEWSDNAME ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procnewsdname: mid2 rs=%d\n",rs) ;
	    debugprintf("main/procnewsdname: newsdname=%s\n",pip->newsdname) ;
	}
#endif

	if ((rs >= 0) && (pip->newsdname[0] != '/')) {
	    char	dname[MAXPATHLEN+1] ;
	    if ((rs = mkpath2(dname,pip->pr,pip->newsdname)) >= 0) {
	        cchar	**vpp = &pip->newsdname ;
	        rs = proginfo_setentry(pip,vpp,dname,rs) ;
	    }
	} /* end if (rooting) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procnewsdname: newsdname=%s\n",pip->newsdname) ;
	    debugprintf("main/procnewsdname: ret rs=%d\n",rs) ;
	}
#endif

	return rs ;
}
/* end subroutine (procnewsdname) */


static int procartdname(PROGINFO *pip,struct tdinfo *tip)
{
	int		rs ;
	cchar	*artcname = ARTCNAME ;

	if ((rs = mkpath2(tip->tdname,pip->newsdname,artcname)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = u_stat(tip->tdname,&sb)) == SR_NOENT) {
	        const mode_t	m = (0775 | S_ISGID) ;
	        if ((rs = mkdirs(tip->tdname,m)) >= 0)
	            rs = u_stat(tip->tdname,&sb) ;
	    } /* end if (stat) */
	    if (rs >= 0) {
	        tip->dev = sb.st_dev ;
	        tip->tdlen = strlen(tip->tdname) ;
	    }
	} /* end if (mkpath) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procartdname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procartdname) */


static int locinfo_start(lip,pip)
LOCINFO	*lip ;
PROGINFO	*pip ;
{
	int		rs ;

	if (lip == NULL)
	    return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to = -1 ;

	if ((rs = vecstr_start(&lip->stores,0,0)) >= 0) {
	    rs = ema_start(&lip->hdrfroms) ;
	    if (rs < 0) 
		vecstr_finish(&lip->stores) ;
	}

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(lip)
LOCINFO	*lip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL)
	    return SR_FAULT ;

	if (lip->open.ngmap) {
	    lip->open.ngmap = FALSE ;
	    rs1 = hdbstr_finish(&lip->ngmap) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = ema_finish(&lip->hdrfroms) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecstr_finish(&lip->stores) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_finish) */


int locinfo_setentry(lip,epp,vp,vl)
LOCINFO	*lip ;
cchar	**epp ;
cchar	vp[] ;
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

	    if (*epp != NULL) oi = vecstr_findaddr(&lip->stores,*epp) ;

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


static int locinfo_hdrfrom(lip,sp,sl)
LOCINFO	*lip ;
cchar	*sp ;
int		sl ;
{
	EMA		*emap = &lip->hdrfroms ;
	int		rs ;
	lip->f.hdrfroms = TRUE ;
	rs = ema_parse(emap,sp,sl) ;
	return rs ;
}
/* end subroutine (locinfo_hdrfrom) */


static int locinfo_mkhdrfrom(lip)
LOCINFO	*lip ;
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip->have.hdrfroms && (! lip->f.def_from))
	    goto ret0 ;

	if (lip->f.hdrfroms)
	    goto ret0 ;

	if (lip->hdrfromaddr == NULL)
	    lip->hdrfromaddr = getourenv(pip->envv,VARPROGMAILFROM) ;

	if (lip->hdrfromaddr == NULL)
	    lip->hdrfromaddr = getourenv(pip->envv,VARMAILFROM) ;

	if (lip->hdrfromaddr == NULL) {
	    rs = locinfo_mkhdrfromaddr(lip) ;
	    len = rs ;
	}

	if ((rs >= 0) && (lip->hdrfromaddr != NULL)) {

	    if (len <= 0) len = strlen(lip->hdrfromaddr) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("b_imail/locinfo_mkhdrfrom: def_from=>%t<\n",
	            lip->hdrfromaddr,strlinelen(lip->hdrfromaddr,len,40)) ;
#endif

	    lip->f.hdrfroms = TRUE ;
	    rs = ema_parse(&lip->hdrfroms,lip->hdrfromaddr,len) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("b_imail/locinfo_mkhdrfrom: ema_parse() rs=%d\n",
	            rs) ;
#endif

	} /* end if */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/locinfo_mkhdrfrom: ret rs=%d len=%u\n",
	        rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_mkhdrfrom) */


static int locinfo_mkhdrfromname(lip)
LOCINFO	*lip ;
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		nbl = 0 ;
	int		len = 0 ;
	cchar	*prpcs ;
	cchar	*nbp = NULL ;
	char		namebuf[REALNAMELEN + 1] ;

	prpcs = pip->pr ;
	if (lip->hdrfromname != NULL) {
	    len = strlen(lip->hdrfromname) ;
	    goto ret0 ;
	}

/* try PCS first */

	namebuf[0] = '\0' ;
	if (prpcs != NULL) {
	    nbp = namebuf ;
	    rs = pcsfullname(prpcs,namebuf,REALNAMELEN,pip->username) ;
	    if ((rs == SR_NOENT) || (rs == SR_ACCESS)) {
	        rs = pcsname(prpcs,namebuf,REALNAMELEN,pip->username) ;
	    }
	    nbl = rs ;
	} /* end if (PCS-name help) */

	if (rs >= 0) {

/* try USERINFO-derived possibilities */

	    if ((nbp == NULL) || (nbp[0] == '\0')) {
	        nbp = lip->fullname ;
	        nbl = -1 ;
	    }

	    if ((nbp == NULL) || (nbp[0] == '\0')) {
	        nbp = lip->name ;
	        nbl = -1 ;
	    }

/* store any result */

	    if ((nbp != NULL) && (nbp[0] != '\0')) {
	        cchar	**vpp = &lip->hdrfromname ;
	        rs = locinfo_setentry(lip,vpp,nbp,nbl) ;
	        len = rs ;
	    }

	} /* end if */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/locinfo_mkhdrfromname: ret rs=%d len=%d\n",
	        rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_mkhdrfromname) */


static int locinfo_mkhdrfromaddr(lip)
LOCINFO	*lip ;
{
	PROGINFO	*pip = lip->pip ;
	BUFFER		b ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		bl = 0 ;

	if (lip->hdrfromaddr != NULL) {
	    bl = strlen(lip->hdrfromaddr) ;
	    goto ret0 ;
	}

/* cache the clustername if necessary */

	if (lip->clustername == NULL)
	    lip->clustername = getourenv(pip->envv,VARCLUSTER) ;

/* put an address together */

	if ((rs = buffer_start(&b,MAILADDRLEN)) >= 0) {
	    cchar	*cn = lip->clustername ;
	    cchar	*nn = pip->nodename ;

	    {
	        cchar	*cp = (cn != NULL) ? cn : nn ;
	        buffer_strw(&b,pip->username,-1) ;
	        buffer_char(&b,'@') ;
	        buffer_strw(&b,cp,-1) ;
	    }

/* add a name if we can find one */

	    if (lip->hdrfromname == NULL)
	        rs = locinfo_mkhdrfromname(lip) ;

	    if ((rs >= 0) && (lip->hdrfromname != NULL)) {
	        buffer_char(&b,' ') ;
	        buffer_char(&b,CH_LPAREN) ;
	        buffer_strw(&b,lip->hdrfromname,-1) ;
	        buffer_char(&b,CH_RPAREN) ;
	    } /* end if (adding name) */

	    if (rs >= 0) {
	        cchar	*bp ;
	        if ((rs = buffer_get(&b,&bp)) >= 0) {
	            cchar	**vpp = &lip->hdrfromaddr ;
	            bl = rs ;
	            rs = locinfo_setentry(lip,vpp,bp,bl) ;
	        }
	    }

	    rs1 = buffer_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (buffer) */

ret0:
	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (locinfo_mkhdrfromaddr) */


static int locinfo_hdrfromget(LOCINFO *lip,EMA **epp)
{
	int		rs ;
	if ((rs = locinfo_mkhdrfrom(lip)) >= 0) {
	    if ((rs = ema_count(&lip->hdrfroms)) >= 0) {
	        if (epp != NULL) *epp = &lip->hdrfroms ;
	    }
	}
	return rs ;
}
/* end subroutines (locinfo_hdrfromget) */


/* our little NG-directory cache (caching only positive hits) */
static int locinfo_ngdname(lip,ngdname,np,nl)
LOCINFO	*lip ;
char		ngdname[] ;
cchar	*np ;
int		nl ;
{
	PROGINFO	*pip = lip->pip ;
	HDBSTR		*mlp = &lip->ngmap ;
	int		rs = SR_OK ;
	int		vl = 0 ;
	cchar	*vp ;

	if (ngdname == NULL) return SR_FAULT ;
	if (np == NULL) return SR_FAULT ;

	if (nl < 0) nl = strlen(np) ;

	if (! lip->open.ngmap) {
	    const int	f = (pip->f.artmaint || pip->f.artexpires) ;
	    const int	n = ((f) ? 1024 : 1) ;
	    rs = hdbstr_start(mlp,n) ;
	    lip->open.ngmap = (rs >= 0) ;
	}

	if (rs >= 0) {
	    if ((rs = hdbstr_fetch(mlp,np,nl,NULL,&vp)) >= 0) {
	        vl = rs ;
	        rs = mkpath1w(ngdname,vp,vl) ;
	    } else if (rs == SR_NOTFOUND) {
	        NULSTR	n ;
	        cchar	*pr = pip->pr ;
	        cchar	*nd = pip->newsdname ;
	        cchar	*ngname ;
	        if ((rs = nulstr_start(&n,np,nl,&ngname)) >= 0) {
	            char	rbuf[MAXPATHLEN+1] ;
	            if ((rs = pcsngdname(pr,rbuf,nd,ngname)) >= 0) {
	                vl = rs ;
	                if ((rs = hdbstr_add(mlp,np,nl,rbuf,rs)) >= 0)
	                    rs = mkpath1w(ngdname,rbuf,vl) ;
	            } /* end if (pcsngdname) */
	            nulstr_finish(&n) ;
	        } /* end if (nulstr) */
	    } /* end if (lookup) */
	} /* end if */

	return (rs >= 0) ? vl : rs ;
}
/* end subroutines (locinfo_ngdname) */


int progngdname(pip,ngdname,np,nl)
PROGINFO	*pip ;
char		ngdname[] ;
cchar	*np ;
int		nl ;
{
	LOCINFO	*lip = pip->lip ;
	return locinfo_ngdname(lip,ngdname,np,nl) ;
}
/* end subroutine (progngdname) */


int progmsgfromema(PROGINFO *pip,EMA **epp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	if (pip == NULL) return SR_FAULT ;
	rs = locinfo_hdrfromget(lip,epp) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/progmsgfromema: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutines (progmsgfromget) */


int progexpiration(PROGINFO *pip,cchar **rpp)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;
	if (pip->ti_expires > 0) {
	    if (pip->expdate[0] == '\0')
	        timestr_msg(pip->ti_expires,pip->expdate) ;
	    rs = strlen(pip->expdate) ;
	} /* end if (have expiration date) */

	if (rpp != NULL)
	    *rpp = (rs > 0) ? pip->expdate : NULL ;

	return rs ;
}
/* end subroutines (progexpiration) */


#ifdef	COMMENT

static int pcsconf_mkdir(pp,name,mode)
PCSCONF		*pp ;
char		name[] ;
int		mode ;
{
	struct ustat	sb ;
	uid_t		uid_pcs ;
	gid_t		gid_pcs ;
	mode_t		um ;
	int		rs ;
	int		cl ;
	int		f_popparent = FALSE ;
	cchar		*cp ;
	char		dirbuf[MAXPATHLEN + 1] ;

	rs = pcsconf_getpcsuid(pp,&uid_pcs) ;
	if (rs < 0) goto ret0 ;

	rs = pcsconf_getpcsgid(pp,&gid_pcs) ;
	if (rs < 0) goto ret0 ;

	rs = u_access(name,(R_OK | X_OK)) ;

	if (rs >= 0)
	    goto ret0 ;

	um = umask(0) ;

/* check on the parent */

	cl = sfdirname(name,-1,&cp) ;

	rs = makedirs(cp,cl,0777) ;

	if (rs < 0)
	    goto done ;

	if ((rs > 0) && (uid_pcs >= 0)) {

	    f_popparent = TRUE ;
	    strwcpy(dirbuf,cp,cl) ;

	}

/* now do the given directory */

	rs = mkdirs(name,mode) ;

	if (rs < 0)
	    goto done ;

	if ((uid_pcs >= 0) &&
	    (u_stat(name,&sb) >= 0) && (sb.st_uid != uid_pcs))
	    u_chown(name,uid_pcs,gid_pcs) ;

	if (f_popparent &&
	    (u_stat(dirbuf,&sb) >= 0) && (sb.st_uid != uid_pcs))
	    u_chown(dirbuf,uid_pcs,gid_pcs) ;

done:
	(void) umask(um) ;

ret0:
	return rs ;
}
/* end subroutine (pcsconf_mkdir) */

#endif /* COMMENT */


