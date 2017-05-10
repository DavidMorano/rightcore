/* main */

/* part of the MKMSG program */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_DEBUGENUM	0		/* enumerate MIME types */
#define	CF_DEBUGTMTIME	1		/* debug TMTIME ops */
#define	CF_FORCEINPUT	0		/* special option */
#define	CF_PROGMSGID	1		/* use 'progmsgid(3dam)' ? */
#define	CF_LOCSETENT	0		/* |locinfo_setentry()| */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This program was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	NOTES¦ invocation arguements are as:

	$ mkmsg [-c <content_type>] [-e <content_encoding>] [-of <output>] 
		[-s <subject>] [-a <attachment_spec>] [<address(s)>]


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/timeb.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sigman.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<dater.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<pcsconf.h>
#include	<pcspoll.h>
#include	<logfile.h>
#include	<mimetypes.h>
#include	<ema.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"mailmsgatt.h"
#include	"mailmsgattent.h"
#include	"ha.h"
#include	"hname.h"
#include	"config.h"
#include	"defs.h"
#include	"proglog.h"


/* local defines */

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	PROCMORE	struct procmore


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	pathadd(char *,int,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	prmktmpdir(cchar *,char *,cchar *,cchar *,mode_t) ;
extern int	pcsgetserial(cchar *) ;
extern int	pcsmsgid(const char *,char *,int) ;
extern int	pcsgetorg(const char *,char *,int,const char *) ;
extern int	pcsuserfile(cchar *,cchar *,cchar *,cchar *,cchar *) ;
extern int	pcsgetfacility(const char *,char *,int) ;
extern int	paramopt_loadone(PARAMOPT *,const char *,int) ;
extern int	initnow(struct timeb *,char *,int) ;
extern int	dater_setkey(DATER *,cchar *,int,struct timeb *,cchar *) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;

extern int	progexpcook_begin(PROGINFO *) ;
extern int	progexpcook_end(PROGINFO *) ;

extern int	progmsgid(PROGINFO *,char *,int,int) ;
extern int	progstrmailer(PROGINFO *,const char *) ;

extern int	mksenderaddr(PROGINFO *) ;
extern int	mkreplyaddr(PROGINFO *) ;
extern int	mkfromaddr(PROGINFO *) ;
extern int	progbuildmsg(PROGINFO *,EMA *,PARAMOPT *,
			MIMETYPES *,MAILMSGATT_ENT *,MAILMSGATT *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strshrink(char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* global variables */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		to:1 ;
	uint		mimetypes:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PROGINFO	*pip ;
	const char	*tmpdname ;
	const char	*pr_pcs ;
	const char	*mfname ;
	const char	*content_type ;
	const char	*content_encoding ;
	uid_t		uid_pr ;
	gid_t		gid_pr ;
	int		to ;
} ;

struct procmore {
	ARGINFO		*aip ;
	BITS		*bop ;
	EMA		*adds ;
	MAILMSGATT	*attp ;
	PARAMOPT	*hdrp ;
	MAILMSGATT_ENT	*iep ;
	const char	*ofn ;
	const char	*afn ;
	const char	*atfname ;
	const char	*datestr ;
	const char	*str_mailer ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	proclogsize(PROGINFO *,cchar *) ;
static int	process(PROGINFO *,PROCMORE *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,EMA *,cchar *) ;
static int	procatts(PROGINFO *,MAILMSGATT *,const char *) ;

static int	procdatestr_begin(PROGINFO *,const char *) ;
static int	procdatestr_end(PROGINFO *) ;

static int	procmime_begin(PROGINFO *,MIMETYPES *) ;
static int	procmime_end(PROGINFO *,MIMETYPES *) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;

static int	procpcsconf_begin(PROGINFO *) ;
static int	procpcsconf_end(PROGINFO *) ;

static int	procextras_begin(PROGINFO *,PROCMORE *) ;
static int	procextras_end(PROGINFO *) ;

static int	procmore(PROGINFO *,PROCMORE *) ;
static int	procmsg(PROGINFO *,const char *,EMA *,PARAMOPT *,
			MIMETYPES *,MAILMSGATT_ENT *,MAILMSGATT *) ;

static int	logmsginfo(PROGINFO *,EMA *,PARAMOPT *) ;
static int	logmsghead(PROGINFO *,int,const char *) ;
static int	logmsgaddr(PROGINFO *,int,EMA *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_tmpdname(LOCINFO *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif

#if	CF_LOCPRSTAT
static int	locinfo_prstat(LOCINFO *) ;
#endif /* CF_LOCPRSTAT */

static void	sighand_int(int) ;


/* local variables */

static volatile int	if_exit ;
static volatile int	if_int ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGHUP,
	SIGCHLD,
	0
} ;

static const int	sigignores[] = {
	SIGPIPE,
	SIGPOLL,
#if	defined(SIGXFSZ)
	SIGXFSZ,
#endif
	0
} ;

static const int	sigints[] = {
	SIGINT,
	SIGTERM,
	SIGQUIT,
	0
} ;

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"CONFIG",
	"LOGFILE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"lf",
	"attach",
	"attfile",
	"at",
	"atf",
	"header",
	"date",
	"disclaimer",
	"cc",
	"bcc",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_config,
	argopt_logfile,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_lf,
	argopt_attach,
	argopt_attfile,
	argopt_at,
	argopt_atf,
	argopt_header,
	argopt_date,
	argopt_disclaimer,
	argopt_cc,
	argopt_bcc,
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
	{ 0, 0 }
} ;

static const char *akonames[] = {
	"organization",
	"from",
	"addfrom",
	"addorg",
	"addface",
	"addreplyto",
	"addsender",
	"inline",
	"mime",
	"crnl",
	NULL
} ;

enum akonames {
	akoname_organization,
	akoname_from,
	akoname_addfrom,
	akoname_addorg,
	akoname_addface,
	akoname_addreplyto,
	akoname_addsender,
	akoname_inline,
	akoname_mime,
	akoname_crnl,
	akoname_overlast
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	MAILMSGATT	atts ;
	MAILMSGATT_ENT	ie ;
	SIGMAN		sm ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	headers ;
	EMA		adds[ha_overlast] ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_input = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*cfname = NULL ;
	const char	*atfname = NULL ;
	const char	*ifname = NULL ;
	const char	*datestr = NULL ;
	const char	*str_mailer = NULL ;
	const char	*cp ;


	if_exit = 0 ;
	if_int = 0 ;

	rs = sigman_start(&sm,sigblocks,sigignores,sigints,sighand_int) ;
	if (rs < 0) goto badsigman ;

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

/* early things to initialize */

	pip->verboselevel = 1 ;
	pip->f.logprog = OPT_LOGPROG ;
	pip->f.crnl = OPT_CRNL ;	/* default is "stupid" mode */
	pip->f.mime = OPT_MIME ;

	pip->lip = lip ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* get the current time-of-day */

	if ((rs = initnow(&pip->now,pip->zname,DATER_ZNAMESIZE)) >= 0) {
	    struct timeb	*nowp = &pip->now ;
	    pip->daytime = pip->now.time ;
	    if ((rs = dater_start(&pip->tmpdate,nowp,pip->zname,-1)) >= 0) {
		pip->open.dater = TRUE ;
	    }
	}
	
	if (rs >= 0) {
	    rs = paramopt_start(&headers) ;
	    pip->open.headers = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = mailmsgatt_start(&atts) ;
	    pip->open.atts = (rs >= 0) ;
	}

	if (rs >= 0) {
	    const int	size = nelements(adds) * sizeof(EMA) ;
	    int		i ;
	    memset(adds,0,size) ;
	    for (i = 0 ; i < nelements(adds) ; i += 1) {
	        rs = ema_start(adds+i) ;
	        if (rs < 0) break ;
	    } /* end for */
	    pip->open.emas = (rs >= 0) ;
	} /* end if */

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
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

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose */
	                case argopt_verbose:
	                    pip->f.verbose = TRUE ;
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
	                            lip->tmpdname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->tmpdname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* get a program root */
	                case argopt_root:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->pr = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* configuration file */
	                case argopt_config:
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

/* log file */
	                case argopt_logfile:
	                case argopt_lf:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->lfname = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* specifiy an attachment */
	                case argopt_attach:
	                case argopt_at:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            cchar	*ct = lip->content_type ;
	                            cchar	*ce = lip->content_encoding ;
	                            rs = mailmsgatt_add(&atts,ct,ce,argp,argl) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* attachments list file */
	                case argopt_attfile:
	                case argopt_atf:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            atfname = argp ;
	                    } else
	                        rs = SR_INVALID ;
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

/* input file for the main text portion of the message */
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
	                    if (rs >= 0) {
	                        if ((ifname != NULL) && (ifname[0] != '\0')) {
	                            const char	*ct ;
	                            f_input = TRUE ;
#if	CF_FORCEINPUT
	                            ct = (lip->content_type == NULL) ?
	                                "text" : lip->content_type ;
#else
	                            ct = lip->content_type ;
#endif /* CF_FORCEINPUT */
	                            rs = mailmsgattent_start(&ie,
	                                ct,lip->content_encoding,
	                                ifname,-1) ;
	                        } else
	                            pip->f.noinput = TRUE ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* specifiy an extra message header */
	                case argopt_header:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
				    PARAMOPT	*pop = &headers ;
	                            rs = paramopt_loadone(pop,argp,argl) ;
				}
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* date */
	                case argopt_date:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            datestr = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* MIME disclaimer */
	                case argopt_disclaimer:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->disclaimer = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_cc:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
				    EMA	*emp = (adds+ha_cc) ;
	                            rs = ema_parse(emp,argp,argl) ;
				}
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_bcc:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
				    EMA	*emp = (adds+ha_bcc) ;
	                            rs = ema_parse(emp,argp,argl) ;
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

/* mailer name specification */
	                    case 'M':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                str_mailer = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* header-ORGANIZATION */
	                    case 'O':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->have.h_org = TRUE ;
	                                pip->final.h_org = TRUE ;
	                                if (argp[0] != '-') {
	                                    pip->f.h_org = TRUE ;
	                                    pip->org = argp ;
	                                }
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* attachment specification */
	                    case 'a':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					MAILMSGATT	*map = &atts ;
					const int	al = argl ;
					cchar		*ap = argp ;
	                                cchar	*ct = lip->content_type ;
	                                cchar	*ce = lip->content_encoding ;
	                                rs = mailmsgatt_add(map,ct,ce,ap,al) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* blind-carbon-copy */
	                    case 'b':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					EMA	*emp = (adds+ha_bcc) ;
	                                rs = ema_parse(emp,argp,argl) ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* carbon-copy */
	                    case 'c':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					EMA	*emp = (adds+ha_cc) ;
	                                rs = ema_parse(emp,argp,argl) ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* date */
	                    case 'd':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                datestr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* content_encoding */
	                    case 'e':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            lip->content_encoding = NULL ;
	                            if (argl)
	                                lip->content_encoding = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* header-FROM */
	                    case 'f':
	                        pip->have.h_from = TRUE ;
	                        pip->final.h_from = TRUE ;
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                if (argp[0] == '+') {
	                                    pip->f.def_from = TRUE ;
	                                } else if (argp[0] != '-') {
	                                    EMA	*emap = (adds+ha_from) ;
	                                    pip->f.h_from = TRUE ;
	                                    rs = ema_parse(emap,argp,argl) ;
	                                }
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* specifiy an extra message header */
	                    case 'h':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                PARAMOPT	*pop = &headers ;
	                                rs = paramopt_loadone(pop,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* input file for the main text portion of the message */
	                    case 'i':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                const char	*ct ;
	                                f_input = TRUE ;
#if	CF_FORCEINPUT
	                                ct = (lip->content_type == NULL) ?
	                                    "text" : lip->content_type ;
#else
	                                ct = lip->content_type ;
#endif /* CF_FORCEINPUT */
	                                rs = mailmsgattent_start(&ie,
	                                    ct,lip->content_encoding,
	                                    argp,-1) ;
	                            } else
	                                pip->f.noinput = TRUE ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* MIME-types file */
	                    case 'm':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->mfname = argp ;
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

/* explicitly specify no input */
	                    case 'n':
	                        pip->f.noinput = TRUE ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* subject */
	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->hdr_subject = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* content_type */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            lip->content_type = NULL ;
	                            if (argl)
	                                lip->content_type = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->f.verbose = TRUE ;
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

	if (f_usage)
	    usage(pip) ;

/* user specified help only */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* process program options */

	if ((rs = procopts(pip,&akopts)) >= 0) {
	    rs = proclogsize(pip,argval) ;
	}

/* other initialization */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

/* what about the input?  IS IT ALWAYS ONLY TEXT? */

	if ((rs >= 0) && (! pip->f.noinput)) {

	    if (f_input) {
	        if ((ie.attfname != NULL) && 
	            (ie.attfname[0] != '\0') && (ie.attfname[0] != '-')) {
	            rs = u_access(ie.attfname,R_OK) ;
	        }
	    } else {
	        const char	*ct = lip->content_type ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf("main: raw input ct=%s ce=%s\n",
	                lip->content_type,lip->content_encoding) ;
	        }
#endif

	        f_input = TRUE ;
	        if (ct == NULL) ct = "text/plain" ;
	        rs = mailmsgattent_start(&ie,ct,NULL,"-",1) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: mailmsgattent_start() rs=%d\n",rs) ;
#endif

	    } /* end if */

	    if (rs == SR_NOTFOUND) ex = EX_NOINPUT ;
	} /* end if (we had some input) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: recipients as arguments?\n") ;
	    rs = ema_count(&adds[ha_to]) ;
	    debugprintf("main: recipients so far=%d\n",rs) ;
	}
#endif /* CF_DEBUG */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	            PCSCONF	pc, *pcp = &pc ;
	            const char	*pr_pcs = lip->pr_pcs ;
	            if (cfname != NULL) {
	                if (pip->euid != pip->uid) u_seteuid(pip->uid) ;
	                if (pip->egid != pip->gid) u_setegid(pip->gid) ;
	            }
	            if ((rs = pcsconf_start(pcp,pr_pcs,envv,cfname)) >= 0) {
	                pip->pcsconf = pcp ;
	                pip->open.pcsconf = TRUE ;
	                if ((rs = procpcsconf_begin(pip)) >= 0) {
	                    PCSPOLL	poll ;
			    cchar	*sn = pip->searchname ;
	                    if ((rs = pcspoll_start(&poll,pcp,sn)) >= 0) {
	                        if ((rs = proglog_begin(pip,&u)) >= 0) {
				    {
	                                PROCMORE	m ;
	                                memset(&m,0,sizeof(PROCMORE)) ;
	                                m.aip = &ainfo ;
	                                m.bop = &pargs ;
	                                m.adds = adds ;
	                                m.attp = &atts ;
	                                m.hdrp = &headers ;
	                                m.iep = &ie ;
	                                m.ofn = ofname ;
	                                m.afn = afname ;
	                                m.atfname = atfname ;
	                                m.datestr = datestr ;
	                                m.str_mailer = str_mailer ;
				        rs = process(pip,&m) ;
				    }
	                            rs1 = proglog_end(pip) ;
	                            if (rs >= 0) rs = rs1 ;
	                        } /* end if (proglog) */
	                        rs1 = pcspoll_finish(&poll) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (pcspoll) */
	                    rs1 = procpcsconf_end(pip) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (procpcsconf) */
	                pip->open.pcsconf = FALSE ;
	                pip->pcsconf = NULL ;
	                rs1 = pcsconf_finish(pcp) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (pcsconf) */
	            rs1 = procuserinfo_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: userinfo failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	    } /* end if (userinfo) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    if (ex == EX_OK) ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

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
	} else if (if_exit) {
	    ex = EX_TERM ;
	} else if (if_int)
	    ex = EX_INTR ;

retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: ret ex=%u (%d)\n",ex,rs) ;
#endif

	if (f_input) {
	    rs1 = mailmsgattent_finish(&ie) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    rs1 = bclose(pip->efp) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    rs1 = keyopt_finish(&akopts) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = bits_finish(&pargs) ;
	if (rs >= 0) rs = rs1 ;

badpargs:
	{
	    int	i ;
	    for (i = 0 ; i < ha_overlast ; i += 1) {
	        rs1 = ema_finish(adds+i) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end for */
	}

	if (pip->open.atts) {
	    pip->open.atts = FALSE ;
	    rs1 = mailmsgatt_finish(&atts) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pip->open.headers) {
	    pip->open.headers = FALSE ;
	    rs1 = paramopt_finish(&headers) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pip->open.dater) {
	    pip->open.dater = FALSE ;
	    rs1 = dater_finish(&pip->tmpdate) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = locinfo_finish(lip) ;
	if (rs >= 0) rs = rs1 ;

badlocstart:
	rs1 = proginfo_finish(pip) ;
	if (rs >= 0) rs = rs1 ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo_finish ;
	    uc_mallout(&mo_finish) ;
	    debugprintf("main: final mallout=%u\n",(mo_finish-mo_start)) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	sigman_finish(&sm) ;

badsigman:
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


static void sighand_int(int sn)
{
	switch (sn) {
	case SIGINT:
	    if_int = TRUE ;
	    break ;
	case SIGKILL:
	    if_exit = TRUE ;
	    break ;
	default:
	    if_exit = TRUE ;
	    break ;
	} /* end switch */
}
/* end subroutine (sighand_int) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<address(s)> ...] [-af <afile>] [-a <atspec>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-of <ofile>] [-atf <atfile>] [-o <opt(s)>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-t <content-type>] [-e <content-encoding>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-c <ccaddr>] [-b <bccaddr>] [-h <name>=<value>]\n" ;
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

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {

	                case akoname_organization:
	                    if (! pip->final.h_org) {
	                        if (vl >= 0) {
	                            const char	**vpp = &pip->org ;
	                            pip->final.h_org = TRUE ;
	                            pip->have.h_org = TRUE ;
	                            pip->f.h_org = TRUE ;
	                            rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;

	                case akoname_crnl:
	                    if (! pip->final.crnl) {
	                        pip->have.crnl = TRUE ;
	                        pip->final.crnl = TRUE ;
	                        pip->f.crnl = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.crnl = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_addfrom:
	                    if (! pip->final.add_from) {
	                        pip->have.add_from = TRUE ;
	                        pip->final.add_from = TRUE ;
	                        pip->f.add_from = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.add_from = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_addorg:
	                    if (! pip->final.add_org) {
	                        pip->have.add_org = TRUE ;
	                        pip->final.add_org = TRUE ;
	                        pip->f.add_org = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.add_org = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_addface:
	                    if (! pip->final.add_face) {
	                        pip->have.add_face = TRUE ;
	                        pip->final.add_face = TRUE ;
	                        pip->f.add_face = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.add_face = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_addreplyto:
	                    if (! pip->final.add_replyto) {
	                        pip->have.add_replyto = TRUE ;
	                        pip->final.add_replyto = TRUE ;
	                        pip->f.add_replyto = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.add_replyto = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_addsender:
	                    if (! pip->final.add_sender) {
	                        pip->have.add_sender = TRUE ;
	                        pip->final.add_sender = TRUE ;
	                        pip->f.add_sender = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.add_sender = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_inline:
	                    if (! pip->final.dis_inline) {
	                        pip->have.dis_inline = TRUE ;
	                        pip->final.dis_inline = TRUE ;
	                        pip->f.dis_inline = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.dis_inline = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_mime:
	                    if (! pip->final.mime) {
	                        pip->have.mime = TRUE ;
	                        pip->final.mime = TRUE ;
	                        pip->f.mime = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.mime = (rs > 0) ;
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
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int proclogsize(PROGINFO *pip,cchar *argval)
{
	int		rs = SR_OK ;
	if ((pip->logsize == 0) && (argval != NULL)) {
	    int		v ;
	    rs = cfdecmfi(argval,-1,&v) ;
	    pip->logsize = v ;
	}
	return rs ;
}
/* end subroutine (proclogsize) */


static int process(PROGINFO *pip,PROCMORE *pmp)
{
	int		rs ;
	int		rs1 ;
	if ((rs = proguserlist_begin(pip)) >= 0) {
	    if ((rs = procextras_begin(pip,pmp)) >= 0) {
		{
		    rs = procmore(pip,pmp) ;
		}
		rs1 = procextras_end(pip) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (procmailinf) */
	    rs1 = proguserlist_end(pip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (proguserlist) */
	return rs ;
}
/* end subroutine (process) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->gecosname = uip->gecosname ;
	pip->homedname = uip->homedname ;
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
	        cchar	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}

#ifdef	COMMENT
	if (rs >= 0) {
	    LOCINFO	*lip = pip->lip ;
	    rs = locinfo_pcspr(lip) ;
	}
#else /* COMMENT */
	lip->pr_pcs = pip->pr ;
#endif /* COMMENT */

	if (rs >= 0) {
	    rs = progexpcook_begin(pip) ;
	}

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

	rs1 = progexpcook_end(pip) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (procuserinfo_end) */


static int procpcsconf_begin(PROGINFO *pip)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procpcsconf_begin: ent\n") ;
#endif

	if (pip->open.pcsconf) {
	    PCSCONF	*pcp = pip->pcsconf ;
	    if (pcp == NULL) rs = SR_FAULT ;

#if	CF_DEBUG
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

	if ((rs >= 0) && (pip->pserial == 0)) {	
	    if ((rs = pcsgetserial(pip->pr)) >= 0) {
		pip->pserial = rs ;
	    }
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procpcsconf_begin: pcsgetserial() rs=%d\n",rs) ;
#endif
	}

	if (rs >= 0) {
	    LOCINFO	*lip = pip->lip ;
	    rs = locinfo_tmpdname(lip) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procpcsconf_begin: ret rs=%d\n",rs) ;
#endif

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


static int procmore(PROGINFO *pip,PROCMORE *mp)
{
	ARGINFO		*aip = mp->aip ;
	BITS		*bop = mp->bop ;
	EMA		*adds = mp->adds ;
	int		rs ;
	int		rs1 ;
	int		recips = 0 ;
	const char	*afn = mp->afn ;

	if ((rs = procargs(pip,aip,bop,adds,afn)) >= 0) {
	    MAILMSGATT	*attp = mp->attp ;
	    const char	*atfname = mp->atfname ;
	    const char	*datestr = mp->datestr ;
	    recips = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        EMA_ENT	*ep ;
	        int		i ;
	        debugprintf("main: addresses\n") ;
	        for (i = 0 ; ema_get(&adds[ha_to],i,&ep) >= 0 ; i += 1) {
	            debugprintf("main: a=%s\n",ep->ap) ;
	        } /* end for */
	    }
#endif /* CF_DEBUG */

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: recips=%u\n",pip->progname,recips) ;

	    if ((rs = procatts(pip,attp,atfname)) >= 0) {
	        MIMETYPES	mt ;
	        if ((rs = procmime_begin(pip,&mt)) >= 0) {
	            if ((rs = procdatestr_begin(pip,datestr)) >= 0) {
	                EMA		*adds = mp->adds ;
	                PARAMOPT	*hdrp = mp->hdrp ;
	                MAILMSGATT_ENT	*iep = mp->iep ;
	                MAILMSGATT	*attp = mp->attp ;
	                const char	*ofn = mp->ofn ;

	                if (pip->open.logprog) {
			    logmsginfo(pip,adds,hdrp) ;
			}

	                rs = procmsg(pip,ofn,adds,hdrp,&mt,iep,attp) ;
	                recips = rs ;

	                rs1 = procdatestr_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (procdatestr) */
	            rs1 = procmime_end(pip,&mt) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procmime) */
	    } /* end if (procatts) */

	} /* end if (procargs) */

	return (rs >= 0) ? recips : rs ;
}
/* end subroutine (procmore) */


static int procextras_begin(PROGINFO *pip,PROCMORE *mp)
{
	EMA		*adds = mp->adds ;
	int		rs = SR_OK ;
	int		f ;

/* header-ORGANIZATION */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: user org=»%s«\n",pip->org) ;
#endif

	if ((! pip->final.add_org) && pip->have.h_org && pip->f.h_org) {
	    pip->f.add_org = TRUE ;
	}

	if ((rs >= 0) && pip->have.h_org && pip->f.h_org) {
	    const char	*cp = pip->org ;
	    if ((cp != NULL) && (cp[0] == '+')) {
	        pip->org = NULL ;
	        pip->f.h_org = FALSE ;
	        pip->have.add_org = TRUE ;
	        pip->f.add_org = TRUE ;
	    }
	} /* end if (hdr-org) */

	if ((rs >= 0) && (! pip->final.h_org) && pip->f.add_org) {
	    const char	*un = pip->username ;
	    const char	*org = pip->org ;
	    if ((org == NULL) || (org[0] == '\0')) org = getenv(VARORG) ;
	    if ((org == NULL) || (org[0] == '\0')) org = pip->org ;
	    if ((org == NULL) || (org[0] == '\0')) {
	        const int	olen = ORGLEN ;
	        char		obuf[ORGLEN+1] ;
	        if ((rs = pcsgetorg(pip->pr,obuf,olen,un)) >= 0) {
	            const char	**vpp = &pip->org ;
	            rs = proginfo_setentry(pip,vpp,obuf,rs) ;
	            pip->have.h_org = TRUE ;
	            pip->f.h_org = TRUE ;
	            org = NULL ;
	        }
	    }
	    if ((org != NULL) && (org[0] != '\0')) {
	        pip->have.h_org = TRUE ;
	        pip->f.h_org = TRUE ;
	        pip->org = org ;
	    }
	} /* end if (hdr-org) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: org=»%s«\n",pip->org) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: org=»%s«\n",
	        pip->progname,pip->org) ;
	}

/* header-MESSAGEID */

	if (rs >= 0) {
	    const int	mlen = MAILADDRLEN ;
	    char	mbuf[MAILADDRLEN+1] ;
#if	CF_PROGMSGID
	    rs = progmsgid(pip,mbuf,mlen,0) ;
#else
	    rs = pcsmsgid(pip->pr,mbuf,mlen) ;
#endif

	    if (rs >= 0) {
	        const char	**vpp = &pip->hdr_mid ;
	        rs = proginfo_setentry(pip,vpp,mbuf,rs) ;
	    }

	} /* end if (header-MESSAGEID) */

/* header-FROM */

	f = FALSE ;
	f = f || ((! pip->have.h_from) && pip->f.add_from) ;
	f = f || (pip->have.h_from && pip->f.def_from) ;
	if ((rs >= 0) && f) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: add_from=%u have_from=%u def_from=%u\n",
	            pip->f.add_from, pip->have.h_from, pip->f.def_from) ;
#endif

	    if ((rs = mkfromaddr(pip)) >= 0) {
	        if (pip->hdr_from != NULL) {
	            pip->have.h_from = TRUE ;
	            pip->final.h_from = TRUE ;
	            pip->f.h_from = TRUE ;
	            rs = ema_parse((adds+ha_from),pip->hdr_from,-1) ;
	        }
	    } /* end if (mkfromaddr) */

	} /* end if (hdr-from) */

/* header-REPLYTO */

	if ((rs >= 0) && pip->f.add_replyto) {
	    if ((rs = mkreplyaddr(pip)) >= 0) {
	        if (pip->hdr_replyto != NULL) {
	            pip->have.h_replyto = TRUE ;
	            pip->f.h_replyto = TRUE ;
	            rs = ema_parse((adds+ha_replyto),pip->hdr_replyto,-1) ;
	        }
	    }
	} /* end if (hdr-replyto) */

/* header-SENDER */

	if ((rs >= 0) && pip->f.add_sender) {
	    if ((rs = mksenderaddr(pip)) >= 0) {
	        if (pip->hdr_sender != NULL) {
	            pip->have.h_sender = TRUE ;
	            pip->f.h_sender = TRUE ;
	            rs = ema_parse((adds+ha_sender),pip->hdr_sender,-1) ;
	        }
	    }
	} /* end if (hdr-sender) */

/* other initialization */

	if (rs >= 0) {
	    const int	rlen = LINEBUFLEN ;
	    char	rbuf[LINEBUFLEN+1] ;
	    if ((rs = pcsgetfacility(pip->pr,rbuf,rlen)) > 0) {
	        const char	**vpp = &pip->facility ;
	        rs = proginfo_setentry(pip,vpp,rbuf,rs) ;
	    }
	}

	if (pip->facility == NULL) pip->facility = STR_FACILITY ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: facility=>%s<\n",pip->facility) ;
#endif

	if (rs >= 0) {
	    rs = progstrmailer(pip,mp->str_mailer) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: progstrmailer() rs=%d\n",rs) ;
#endif
	}

	if ((rs >= 0) && (pip->disclaimer == NULL)) {
	    int		tl ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((tl = mkpath2(tbuf,pip->pr,DISCLAIMER)) > 0) {
	        if (u_access(tbuf,R_OK) >= 0) {
	            const char	**vpp = &pip->disclaimer ;
	            rs = proginfo_setentry(pip,vpp,tbuf,tl) ;
	        }
	    }
	} /* end if (disclaimer) */

	return rs ;
}
/* end subroutine (procextras_begin) */


static int procextras_end(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (procextras_end) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,EMA *adds,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;
	int		cl ;
	int		recips = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	const char	*cp ;

	if (rs >= 0) {
	    int	ai ;
	    int	f ;
	    for (ai = 1 ; ai < aip->argc ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	        if (f) {
	            cp = aip->argv[ai] ;
	            if (cp[0] != '\0') {
	                rs = ema_parse((adds+ha_to),cp,-1) ;
	                recips += rs ;
	            }
	        }

	        if (rs < 0) break ;
	    } /* end for (looping through requested circuits) */
	} /* end if (ok) */

/* process any recipients in the recipient list file */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	    if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	        BFILE_STAT	sb ;
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN+1] ;

	        if ((rs = bcontrol(afp,BC_STAT,&sb)) >= 0) {
	            if (S_ISDIR(sb.st_mode)) rs = SR_ISDIR ;
	        }

	        if (rs >= 0) {
	            while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = ema_parse((adds+ha_to),cp,cl) ;
	                        recips += rs ;
	                    }
	                }

	                if (rs < 0) break ;
	            } /* end while (reading lines) */
	        } /* end if (ok) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf("main: done processing recipient list file\n") ;
	            rs = ema_count(&adds[ha_to]) ;
	            debugprintf("main: TO recipients so far=%d\n",rs) ;
	        }
#endif /* CF_DEBUG */

	        rs1 = bclose(afp) ;
	        if (rs >= 0) rs = rs1 ;
	    }  else {
		fmt = "%s: inaccessible recipient list (%d)\n",
	        bprintf(pip->efp,fmt,pn,rs) ;
	        bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	    }

	} /* end if (recipient list file) */

	return (rs >= 0) ? recips : rs ;
}
/* end subroutine (procargs) */


static int procatts(PROGINFO *pip,MAILMSGATT *attp,cchar *atfname)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGATT_ENT	*aep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((atfname != NULL) && (atfname[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(atfname,"-") == 0) atfname = BFILE_STDIN ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main/procatts: atfname=%s\n",atfname) ;
#endif

	    if ((rs = bopen(afp,atfname,"r",0666)) >= 0) {
	        BFILE_STAT	sb ;

	        if ((rs = bcontrol(afp,BC_STAT,&sb)) >= 0) {
	            if (S_ISDIR(sb.st_mode)) rs = SR_ISDIR ;
	        }

	        if (rs >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            int		cl ;
	            const char	*cp ;
	            char	lbuf[LINEBUFLEN+1] ;
	            while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        const char	*ct = lip->content_type ;
	                        const char	*ce = lip->content_encoding ;
	                        c += 1 ;
	                        rs = mailmsgatt_add(attp,ct,ce,cp,cl) ;
	                    }
	                }

	                if (rs < 0) break ;
	            } /* end while (reading lines) */
	        } /* end if (ok) */

	        rs1 = bclose(afp) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
		fmt = "%s: inaccessible attachment list file (%d)\n" ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	        bprintf(pip->efp,"%s: atfile=%s\n",pn,atfname) ;
	    } /* end if */

	} /* end if (processing attachment list file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    int	rs1 = mailmsgatt_count(attp) ;
	    int	i ;
	    debugprintf("main: attachments n=%d\n",rs1) ;
	    for (i = 0 ; mailmsgatt_enum(attp,i,&aep) >= 0 ; i += 1) {
	        debugprintf("main: att file=%s\n",aep->attfname) ;
	        debugprintf("main: att type=%s subtype=%s\n",
	            aep->type,aep->subtype) ;
	    } /* end for */
	}
#endif /* CF_DEBUG */

/* are all of the attachment files present? */

	if (rs >= 0) {
	    for (i = 0 ; (mailmsgatt_enum(attp,i,&aep) >= 0) ; i += 1) {
	        if (aep != NULL) {
		    cchar	*atfname = aep->attfname ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: atfname=%s\n",atfname) ;
#endif

	    if ((atfname == NULL) || 
	        ((atfname[0] != '-') &&
	        ((rs = u_access(atfname,R_OK)) < 0))) {

		    fmt = "%s: could not access attachment (%d)\n" ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	            fmt = "%s: attachment file=%s\n" ;
	            bprintf(pip->efp,fmt,pn,atfname) ;

	    } /* end if */

	        }
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (ok) */

	if (rs < 0) {
	    fmt = "%s: inaccesssible attachment-list file (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procatts: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procatts) */


static int procdatestr_begin(PROGINFO *pip,cchar *datestr)
{
	int		rs = SR_OK ;

	if ((datestr != NULL) && (datestr[0] != '\0')) {
	    struct timeb	*np = &pip->now ;
	    const char		*zn = pip->zname ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: datestr=%s\n",datestr) ;
#endif

	    if ((rs = dater_setkey(&pip->tmpdate,datestr,-1,np,zn)) >= 0) {
	        pip->f.mdate = TRUE ;
	        rs = dater_startcopy(&pip->mdate,&pip->tmpdate) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: f_mdate=%d\n",pip->f.mdate) ;
#endif

	} /* end if (user specified date) */

	return rs ;
}
/* end subroutine (procdatestr_begin) */


static int procdatestr_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->f.mdate) {
	    pip->f.mdate = FALSE ;
	    rs1 = dater_finish(&pip->mdate) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (procdatestr_end) */


static int procmime_begin(PROGINFO *pip,MIMETYPES *mtp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((rs = mimetypes_start(mtp)) >= 0) {
	    const char	*mfname = lip->mfname ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((mfname == NULL) || (mfname[0] == '\0')) {
	        mfname = tbuf ;
	        rs = mkpath2(tbuf,pip->pr,TYPESFNAME) ;
	    } /* end if */
	    if (rs >= 0) {
	        rs = mimetypes_file(mtp,mfname) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: mimetypes_file() rs=%d\n",rs) ;
#endif
	    }
	    if (rs >= 0) {
	        lip->open.mimetypes = TRUE ;
	    }
	    if (rs < 0) {
	        mimetypes_finish(mtp) ;
	    }
	} /* end if (mimetypes-) */

	if (rs < 0) {
	    cchar	*mfname = lip->mfname ;
	    fmt = "%s: inaccessible MIME database file (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    bprintf(pip->efp,"%s: atfile=%s\n",pn,mfname) ;
	}

#if	CF_DEBUG && CF_DEBUGENUM
	if (pip->debuglevel > 4) {
	    MIMETYPES_CUR	cur ;
	    char	ext[MIMETYPES_TYPELEN + 1] ;
	    char	typespec[MIMETYPES_TYPELEN + 1] ;
	    debugprintf("main: mimetypes enumed\n") ;
	    mimetypes_curbegin(mtp,&cur) ;
	    while (mimetypes_enum(mtp,&cur,ext,typespec) >= 0) {
	        debugprintf("main: ext=%s typespec=%s\n",
	            ext,typespec) ;
	    } /* end while */
	    mimetypes_curend(mtp,&cur) ;
	}
#endif /* CF_DEBUG && CF_DEBUGENUM */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    const char	*cp = "gif" ;
	    char	tbuf[MAXPATHLEN+1] ;
	    debugprintf("main: mimetypes get\n") ;
	    rs = mimetypes_fetch(mtp,cp,NULL,tbuf) ;
	    debugprintf("main: mimetypes ext=%s rs=%d typespec=%s\n",
	        cp,rs,tbuf) ;
	    cp = "jpg" ;
	    mimetypes_fetch(mtp,cp,NULL,tbuf) ;
	    debugprintf("main: mimetypes ext=%s rs=%d typespec=%s\n",
	        cp,rs,tbuf) ;
	}
#endif /* CF_DEBUG */

	return rs ;
}
/* end subroutine (procmime_begin) */


static int procmime_end(PROGINFO *pip,MIMETYPES *mtp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->open.mimetypes) {
	    lip->open.mimetypes = FALSE ;
	    rs1 = mimetypes_finish(mtp) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (procmime_end) */


/* build the message! */
static int procmsg(pip,ofn,addp,hdrp,mtp,iep,attp)
PROGINFO	*pip ;
const char	*ofn ;
EMA		*addp ;
PARAMOPT	*hdrp ;
MIMETYPES	*mtp ;
MAILMSGATT_ENT	*iep ;
MAILMSGATT	*attp ;
{
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/procmsg: ent\n") ;
#endif

/* open the output file */

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0666)) >= 0) {
	    pip->ofp = ofp ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main/procmsg: opened output file\n") ;
#endif

	    rs = progbuildmsg(pip,addp,hdrp,mtp,iep,attp) ;
	    wlen += rs ;

	    pip->ofp = NULL ;
	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: output unavailable (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    bprintf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main/procmsg: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsg) */


static int logmsginfo(PROGINFO *pip,EMA *adds,PARAMOPT *hlp)
{
	int		rs = SR_OK ;

	if (hlp == NULL) return SR_FAULT ;

	if (pip->open.logprog) {
	    int	i ;

	    if ((rs >= 0) && pip->f.h_org) {
	        const char	*org = pip->org ;
	        if (org != NULL) rs = logmsghead(pip,hname_org,org) ;
	    }

	    if ((rs >= 0) && (pip->hdr_mid != NULL)) {
	        rs = logmsghead(pip,hname_msgid,pip->hdr_mid) ;
	    }

	    for (i = 0 ; (rs >= 0) && (i < ha_overlast) ; i += 1) {
	        rs = logmsgaddr(pip,i,(adds + i)) ;
	        if (rs < 0) break ;
	    } /* end for */

	    if ((rs >= 0) && (pip->hdr_subject != NULL)) {
	        rs = logmsghead(pip,hname_subject,pip->hdr_subject) ;
	    }

	} /* end if (have a log-file) */

	return rs ;
}
/* end subroutine (logmsginfo) */


static int logmsghead(PROGINFO *pip,int name,cchar *vp)
{
	const int	cmax = (80 - 16 - strlen(hname[name]) - 1 - 2) ;
	int		rs = SR_OK ;

	if (cmax > 0) {
	    if (vp != NULL) {
	        const int	vl = strnlen(vp,cmax) ;
	        const char	*fmt ;
	        fmt = (name != hname_msgid) ? "%s=>%t<\n" : "%s=%t" ;
	        rs = proglog_printf(pip,fmt,hname[name],vp,vl) ;
	    }
	} /* end if (cmax) */

	return rs ;
}
/* end subroutine (logmsghead) */


static int logmsgaddr(PROGINFO *pip,int name,EMA *ap)
{
	const int	cmax = (80 - 16 - strlen(hname[name]) - 1) ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (cmax > 0) {
	    EMA_ENT	*ep ;
	    int		i ;
	    const char	*cp ;
	    for (i = 0 ; (rs >= 0) && (ema_get(ap,i,&ep) >= 0) ; i += 1) {
	        if (ep != NULL) {
	            cp = ep->rp ;
	            if ((cp == NULL) || (cp[0] == '\0')) {
	                cp = ep->ap ;
		    }
	            if ((cp != NULL) && (cp[0] != '\0')) {
	                const int	cl = strnlen(cp,cmax) ;
		        cchar	*kn = hname[name] ;
	                c += 1 ;
	                rs = proglog_printf(pip,"%s=%t\n",kn,cp,cl) ;
	            }
		}
	    } /* end for */
	} /* end if (cmax) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (logmsgaddr) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to = -1 ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

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
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! lip->open.stores) {
	    rs = vecstr_start(&lip->stores,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int		oi = -1 ;
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
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCSETENT */


static int locinfo_tmpdname(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;

	if (lip->tmpdname == NULL) {
	    lip->tmpdname = getourenv(pip->envv,VARTMPDNAME) ;
	}
	if (lip->tmpdname == NULL) {
	    lip->tmpdname = TMPDNAME ;
	}

#ifdef	COMMENT
	if ((rs = locinfo_prstat(lip)) >= 0) {
	    int		cl ;
	    const char	*td = lip->tmpdname ;
	    const char	*sn = pip->searchname ;
	    const char	*cp ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((cl = sfbasename(pip->pr,-1,&cp)) > 0) {
	        if ((rs = mkpath1w(tbuf,td,cp,cl)) >= 0) {
	            const mode_t	dm = 0777 ;
	            const int		tlen = rs ;
	            if ((rs = mkdirs(tbuf,dm)) >= 0) {
	                if ((rs = pathadd(tbuf,tlen,sn)) >= 0) {
	                    if ((rs = mkdirs(tbuf,dm)) >= 0) {
	                        const char	**vpp = &pip->jobdname ;
	                        rs = proginfo_setentry(pip,vpp,tbuf,rs) ;
	                    }
	                }
	            }
	        }
	    }
	} /* end if (locinfo_prstat) */
#else /* COMMENT */
	{
	    const mode_t	m = 0777 ;
	    const char		*td = lip->tmpdname ;
	    const char		*sn = pip->searchname ;
	    const char		*pr = pip->pr ;
	    char		rbuf[MAXPATHLEN+1] ;
	    if ((rs = prmktmpdir(pr,rbuf,td,sn,m)) >= 0) {
	        cchar		**vpp = &pip->jobdname ;
	        rs = proginfo_setentry(pip,vpp,rbuf,rs) ;
	    }
	}
#endif /* COMMENT */

	return rs ;
}
/* end subroutine (locinfo_tmpdname) */


#if	CF_LOCPRSTAT
static int locinfo_prstat(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip->uid_pr < 0) {
	    struct ustat	sb ;
	    if ((rs = u_stat(pip->pr,&sb)) >= 0) {
	        lip->uid_pr = sb.st_uid ;
	        lip->gid_pr = sb.st_gid ;
	    }
	}

	return rs ;
}
/* end subroutine (locinfo_prstat) */
#endif /* CF_LOCPRSTAT */


