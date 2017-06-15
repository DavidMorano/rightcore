/* main (PCSGETMAIL) */
/* lang=C99 */

/* the PCSGETMAIL program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGN	0		/* 'nprintf(3dam)' */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_LOCSETENT	0		/* |locinfo_setentry()| */


/* revision history:

	= 2000-03-01, David A­D­ Morano
	This code module was originally written.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the front-end subroutine for the GETMAIL program.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<storebuf.h>
#include	<nulstr.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<pcsconf.h>
#include	<vecstr.h>
#include	<field.h>
#include	<char.h>
#include	<ascii.h>
#include	<localmisc.h>
#include	<exitcodes.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	VARFOLDER
#define	VARFOLDER	"folder"
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags

#define	DEBFNAME	"/tmp/getmail.deb"


/* external routines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkmaildirtest(char *,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	mkuibang(char *,int,USERINFO *) ;
extern int	mkuiname(char *,int,USERINFO *) ;
extern int	getmailgid(const char *,gid_t) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;

extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	proginfo_realbegin(PROGINFO *) ;
extern int	proginfo_realend(PROGINFO *) ;

extern int	proglog_begin(PROGINFO *,USERINFO *) ;
extern int	proglog_end(PROGINFO *) ;
extern int	proglog_print(PROGINFO *,cchar *,int) ;
extern int	proglog_printf(PROGINFO *,cchar *,...) ;
extern int	proglog_flush(PROGINFO *) ;
extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;

extern int	progmailbox(PROGINFO *,int,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern const char	makedate[] ;


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		poll:1 ;
	uint		lock:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PROGINFO	*pip ;
	const char	*pr_pcs ;
	const char	*homedname ;
	uint		sum ;
	int		mfd ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;
static int	makedate_get(const char *,const char **) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procsetup(PROGINFO *,ARGINFO *,BITS *,PARAMOPT *,
			cchar *,cchar *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,PARAMOPT *,cchar *) ;
static int	procnames(PROGINFO *,PARAMOPT *,cchar *,cchar *,int) ;
static int	procmailfname(PROGINFO *) ;
static int	procfoldercheck(PROGINFO *,const char *) ;
static int	procthem(PROGINFO *) ;
static int	proclog_maildirbad(PROGINFO *,cchar *,int,int) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;

static int	procpcsconf_begin(PROGINFO *,PCSCONF *) ;
static int	procpcsconf_end(PROGINFO *) ;

static int	maildirs_begin(PROGINFO *,PARAMOPT *) ;
static int	maildirs_end(PROGINFO *) ;
static int	maildirs_env(PROGINFO *,const char *) ;
static int	maildirs_arg(PROGINFO *,PARAMOPT *) ;
static int	maildirs_def(PROGINFO *,const char *) ;
static int	maildirs_add(PROGINFO *,const char *,int) ;
static int	maildirs_accessible(PROGINFO *,cchar *,int,int) ;

static int	mailusers_begin(PROGINFO *,PARAMOPT *) ;
static int	mailusers_end(PROGINFO *) ;
static int	mailusers_env(PROGINFO *,const char *) ;
static int	mailusers_arg(PROGINFO *,PARAMOPT *) ;
static int	mailusers_def(PROGINFO *,const char *) ;
static int	mailusers_add(PROGINFO *,const char *,int) ;

#ifdef	COMMENT
static int	forwarded(PROGINFO *,char *,int) ;
#endif

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif /* CF_LOCSETENT */


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"LOGFILE",
	"HELP",
	"maildir",
	"folder",
	"md",
	"sn",
	"af",
	"ef",
	"of",
	"lf",
	"cf",
	"fd",
	"tomb",
	"toms",
	"nodel",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_logfile,
	argopt_help,
	argopt_maildir,
	argopt_folder,
	argopt_md,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_lf,
	argopt_cf,
	argopt_fd,
	argopt_tomb,
	argopt_toms,
	argopt_nodel,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRPCS
} ;

static const char *akonames[] = {
	"nodel",
	NULL
} ;

enum akonames {
	akoname_nodel,
	akoname_overlast
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

static const char *varmaildirs[] = {
	VARMAILDNAME,
	VARMAILDNAMES,
	VARMAILDNAMESP,
	NULL
} ;

static const char *varmailusers[] = {
	VARMAILUSERS,
	VARMAILUSERSP,
	NULL
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		bytes = 0 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_makedate = FALSE ;
	int		f_help = FALSE ;
	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*cfname = NULL ;
	const char	*reportfname = NULL ;
	const char	*jobid = NULL ;
	const char	*cp ;

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

	pip->verboselevel = 1 ;
	pip->timeout = DEFTIMEOUT ;
	pip->to_mailbox = TO_MAILBOX ;
	pip->to_mailspool = TO_MAILSPOOL ;

	pip->f.logprog = OPT_LOGPROG ;

	pip->lip = &li ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* parse the command line arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	if (rs >= 0) {
	    rs = keyopt_start(&akopts) ;
	    pip->open.akopts = (rs >= 0) ;
	}

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

	                case argopt_fd:
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
	                        lip->mfd = rs ;
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

	                case argopt_logfile:
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

	                case argopt_maildir:
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
	                    if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
	                        const char	*po = PO_MAILDIRS ;
	                        rs = paramopt_loads(&aparams,po,cp,cl) ;
	                    }
	                    break ;

	                case argopt_folder:
	                    cp = NULL ;
	                    cl = -1 ;
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
	                    if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
	                        pip->folderdname = cp ;
	                    }
	                    break ;

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

/* argument-list file-name */
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

/* PCS configuration file-name */
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

/* mailbox timeout */
	                case argopt_tomb:
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
	                        pip->to_mailbox = rs ;
	                    }
	                    break ;

/* mailspool timeout */
	                case argopt_toms:
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
	                        pip->to_mailspool = rs ;
	                    }
	                    break ;

/* version */
	                case argopt_version:
	                    f_makedate = f_version ;
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
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

/* no-delete mode */
	                case argopt_nodel:
	                    pip->f.nodel = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.nodel = (rs > 0) ;
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

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'V':
	                        f_makedate = f_version ;
	                        f_version = TRUE ;
	                        break ;

/* job ID passed to us */
	                    case 'j':
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                jobid = avp ;
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl)
	                                    jobid = argp ;
	                            } else
	                                rs = SR_INVALID ;
	                        }
	                        break ;

	                    case 'l':
	                        lip->f.lock = TRUE ;
	                        break ;

/* mailbox file? */
	                    case 'm':
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
	                        if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
	                            pip->mbox = cp ;
	                        }
	                        break ;

/* mail-spool directory */
	                    case 'f':
	                    case 's':
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
	                        if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
	                            const char	*po = PO_MAILDIRS ;
	                            rs = paramopt_loads(&aparams,po,cp,cl) ;
	                        }
	                        break ;

	                    case 'n':
	                        lip->f.lock = FALSE ;
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

	                    case 'r':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                reportfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* timeout (general) */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                pip->timeout = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* mail user name */
	                    case 'u':
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
	                        if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
	                            const char	*po = PO_MAILUSERS ;
	                            rs = paramopt_loads(&aparams,po,cp,cl) ;
	                        }
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

	        } /* end if (number or option) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

#if	CF_DEBUGN
	nprintf(DEBFNAME,"main: args rs=%d\n",rs) ;
#endif

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
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
	    cchar	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: version %s/%s\n",pn,
	        VERSION,(pip->f.sysv_ct) ? "SYSV" : "BSD") ;
	    if (f_makedate) {
	        cl = makedate_get(makedate,&cp) ;
	        bprintf(pip->efp,"%s: makedate %t\n",pn,cp,cl) ;
	    }
	} /* end if */

/* try to get a program root */

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

/* help */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialize */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: procopts() rs=%d\n",rs) ;
#endif

	if (afname == NULL) afname = getourenv(pip->envv,VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* mail-box */

	if (pip->mbox == NULL) pip->mbox = getenv(VARMB) ;

#ifdef	COMMENT
	if ((rs >= 0) && (pip->mbox == NULL)) {
	    if ((cp = getenv(VARMBOX)) != NULL) {
	        int		vl ;
	        const char	*vp ;
	        if ((vl = sfbasename(cp,-1,&vp)) > 0) {
	            const char	**vpp = &pip->mbox ;
	            rs = proginfo_setentry(pip,vpp,vp,vl) ;
	        }
	    }
	}
	if ((rs >= 0) && (pip->mbox == NULL)) {
	    pip->mbox = MAILMBOX ;
	}
#endif /* COMMENT */

/* initialize some other common stuff only needed for mail operations */

/* get the current time-of-day */

	pip->daytime = time(NULL) ;

/* if the folder does not start from root, it must start from HOME */

	if (pip->folderdname == NULL) pip->folderdname = getenv(VARFOLDER) ;
	if (pip->folderdname == NULL) pip->folderdname = MAILFOLDER ;

/* the MAILGROUP is really only valid on SYSV systems! */

	pip->gid_mail = getmailgid(MAILGNAME,MAILGID) ;

/* continue */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USERINFO	u ;
	    cchar	*pn = pip->progname ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	            PCSCONF	pc, *pcp = &pc ;
		    cchar	**envv = pip->envv ;
	            cchar	*pr_pcs = lip->pr_pcs ;
		    cchar	*cfn = cfname ;
	            if (cfname != NULL) {
	                if (pip->euid != pip->uid) u_seteuid(pip->uid) ;
	                if (pip->egid != pip->gid) u_setegid(pip->gid) ;
	            }
	            if ((rs = pcsconf_start(pcp,pr_pcs,envv,cfn)) >= 0) {
	                pip->pcsconf = pcp ;
	                pip->open.pcsconf = TRUE ;
	                if ((rs = procpcsconf_begin(pip,pcp)) >= 0) {
	                    if ((rs = proglog_begin(pip,&u)) >= 0) {
	                        if ((rs = proguserlist_begin(pip)) >= 0) {
	                            ARGINFO	*aip = &ainfo ;
	                            BITS	*bop = &pargs ;
	                            PARAMOPT	*app = &aparams ;
	                            const char	*afn = afname ;
	                            const char	*ofn = ofname ;
    
	                            rs = procsetup(pip,aip,bop,app,ofn,afn) ;
				    bytes = rs ;

	                            rs1 = proguserlist_end(pip) ;
	                            if (rs >= 0) rs = rs1 ;
	                        } /* end if (proguserlist) */
	                        rs1 = proglog_end(pip) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (proglog) */
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
	        cchar	*fmt = "%s: userinfo failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	    } /* end if (userinfo) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

/* report file */

	if (reportfname != NULL) {
	    bfile	rfile ;
	    if (bopen(&rfile,reportfname,"wct",0666) >= 0) {
		{
	            rs = bprintf(&rfile,"%u\n",bytes) ;
		}
	        rs1 = bclose(&rfile) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (file) */
	} /* end if (report file) */

/* STDERR report */

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: mail read bytes=%u\n",
	        pip->progname,bytes) ;
	}

/* done: */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_TXTBSY:
	        ex = EX_TEMPFAIL ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: could not capture the mail lock\n",
	                pip->progname) ;
	        }
	        break ;
	    case SR_ACCES:
	        ex = EX_ACCESS ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: could not access the mail spool-file\n",
	                pip->progname) ;
	        }
	        break ;
	    case SR_REMOTE:
	        ex = EX_FORWARDED ;
	        if (! pip->f.quiet) {
#ifdef	COMMENT
	            if (forwarded(pip,buf,BUFLEN)) {
	                bprintf(pip->efp,
	                    "%s: mail is being forwarded to \"%s\"\n",
	                    pip->progname,buf) ;
	            } else
	                bprintf(pip->efp,"%s: mail is being forwarded\n",
	                    pip->progname) ;
#else /* COMMENT */
	            bprintf(pip->efp,"%s: mail is being forwarded\n",
	                pip->progname) ;
#endif /* COMMENT */
	        }
	        break ;
	    case SR_NOSPC:
	        ex = EX_NOSPACE ;
	        proglog_printf(pip,
	            "file-system is out of space\n") ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: local file-system is out of space\n",
	                pip->progname) ;
	        }
	        break ;
	    default:
	        ex = EX_UNKNOWN ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,"%s: unknown bad thing (%d)\n",
	                pip->progname,rs) ;
	        }
	        break ;
	    } /* end switch */
	} else
	    ex = EX_OK ;

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
	    uint	mo_finish ;
	    uc_mallout(&mo_finish) ;
	    debugprintf("main: final mallout=%u\n",(mo_finish-mo_start)) ;
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
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-u <mailuser>] [-vp] [-l | -n] [-m <mailbox>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-md <maildir>] [-r <reportfile>] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s: \n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s: options:\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  -md <maildir>     system mail spool directory\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  -u <user>        alternate user (get user's mail)\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  -m <mailbox>     mailbox-file for output\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  -p               print the mailspool filepath\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  -j <jobid>       supply a JOBID from caller\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  -r <reportfile>  file to report transferred bytes\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


#ifdef	COMMENT
static int forwarded(pip,buf,buflen)
PROGINFO	*pip ;
char		buf[] ;
int		buflen ;
{
	bfile		sfile, *sfp = &sfile ;
	int		len ;
	int		forlen = strlen(FORWARDED) ;
	int		rlen = 0 ;
	char		lbuf[LINEBUFLEN + 1] ;

	buf[0] = '\0' ;
	if (bopen(sfp,pip->spoolfname,"r",0666) >= 0) {

	    len = breadline(sfp,lbuf,LINEBUFLEN) ;

	    if (lbuf[len - 1] == '\n')
	        len -= 1 ;

	    if (len > forlen) {

	        rlen = MIN((len - forlen),buflen) ;
	        strwcpy(buf,(lbuf + forlen),rlen) ;

	    }

	    bclose(sfp) ;
	} /* end if (opened the mail spoolfile) */

	return (buf[0] != '\0') ? rlen : FALSE ;
}
/* end subroutine (forwarded) */
#endif /* COMMENT */


/* get the date out of the ID string */
static int makedate_get(cchar md[],cchar **rpp)
{
	int		ch ;
	const char	*sp ;
	const char	*cp ;

	if (rpp != NULL)
	    *rpp = NULL ;

	if ((cp = strchr(md,CH_RPAREN)) == NULL)
	    return SR_NOENT ;

	while (CHAR_ISWHITE(*cp))
	    cp += 1 ;

	ch = MKCHAR(cp[0]) ;
	if (! isdigitlatin(ch)) {
	    while (*cp && (! CHAR_ISWHITE(*cp))) cp += 1 ;
	    while (CHAR_ISWHITE(*cp)) cp += 1 ;
	} /* end if (skip over the name) */

	sp = cp ;
	if (rpp != NULL)
	    *rpp = cp ;

	while (*cp && (! CHAR_ISWHITE(*cp)))
	    cp += 1 ;

	return (cp - sp) ;
}
/* end subroutine (makedate_get) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
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

	            vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                switch (oi) {
	                case akoname_nodel:
	                    if (! pip->final.nodel) {
	                        pip->have.nodel = TRUE ;
	                        pip->final.nodel = TRUE ;
	                        pip->f.nodel = TRUE ;
	                        if ((vp != NULL) && (vl > 0)) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.nodel = (rs > 0) ;
	                        }
	                    }
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


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procuserinfo_begin: ent\n") ;
#endif

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->homedname = uip->homedname ;
	pip->gecosname = uip->gecosname ;
	pip->realname = uip->realname ;
	pip->org = uip->organization ;
	pip->mailname = uip->mailname ;
	pip->name = uip->name ;
	pip->fullname = uip->fullname ;
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

	if (pip->euid != pip->uid) pip->f.setuid = TRUE ;
	if (pip->egid != pip->gid) pip->f.setgid = TRUE ;

#ifdef	COMMENT
	if (rs >= 0) {
	    LOCINFO	*lip = pip->lip ;
	    rs = locinfo_pcspr(lip) ;
	}
#else
	{
	    LOCINFO	*lip = pip->lip ;
	    lip->pr_pcs = pip->pr ;
	}
#endif /* COMMENT */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procuserinfo_begin: ret rs=%d\n",rs) ;
#endif

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


static int procpcsconf_begin(PROGINFO *pip,PCSCONF *pcp)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procpcsconf_begin: ent\n") ;
#endif

	if (pip->open.pcsconf) {
	    rs = 1 ;

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


static int procsetup(pip,aip,bop,app,ofn,afn)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
PARAMOPT	*app ;
const char	*ofn ;
const char	*afn ;
{
	int		rs ;
	int		rs1 ;
	int		bytes = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsetup: ent\n") ;
#endif
	if ((rs = procargs(pip,aip,bop,app,afn)) >= 0) {
	    if ((rs = ids_load(&pip->id)) >= 0) {
	        const int	fac = LOG_MAIL ;
	        const char	*lid = pip->logid ;
	        const char	*pn = pip->progname ;
	        if ((rs = logsys_open(&pip->ls,fac,pn,lid,0)) >= 0) {
	            pip->open.logsys = TRUE ;
	            if ((rs = maildirs_begin(pip,app)) >= 0) {
	                if ((rs = mailusers_begin(pip,app)) >= 0) {
	                    if ((rs = procmailfname(pip)) >= 0) {

	                        rs = procthem(pip) ;
	                        bytes = rs ;

	                    } /* end if (procmailfname) */
	                    rs1 = mailusers_end(pip) ;
		            if (rs >= 0) rs = rs1 ;
	                } /* end if (mailusers) */
	                rs1 = maildirs_end(pip) ;
		        if (rs >= 0) rs = rs1 ;
	            } /* end if (maildirs) */
	            pip->open.logsys = FALSE ;
	            rs1 = logsys_close(&pip->ls) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (logsys) */
	        rs1 = ids_release(&pip->id) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (id) */
	} /* end if (procargs) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsetup: ret rs=%d bytes=%u\n",rs,bytes) ;
#endif
	return (rs >= 0) ? bytes :rs ;
}
/* end subroutine (procsetup) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,PARAMOPT *app,
		cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	int		c = 0 ;
	const char	*po = PO_MAILUSERS ;
	const char	*cp ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if (rs >= 0) {
	    int		ai ;
	    int		f ;
	    const char	**argv = aip->argv ;
	    for (ai = 1 ; (rs >= 0) && (ai < aip->argc) ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            cp = argv[ai] ;
	            if (cp[0] != '\0') {
	                pan += 1 ;
	                rs = paramopt_loads(app,po,cp,-1) ;
			c += rs ;
	            }
	        }

	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (ok) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0)
	        afn = BFILE_STDIN ;

	    if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procnames(pip,app,po,cp,cl) ;
	                        c += rs ;
	                    }
	                }

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(afp) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
	        if (! pip->f.quiet) {
		    fmt = "%s: inaccessible argument-list (%d)\n" ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


static int procnames(PROGINFO *pip,PARAMOPT *app,cchar *po,cchar *lbuf,int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;
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


static int procmailfname(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		f_store = TRUE ;
	const char	*mfname = NULL ;
	char		tbuf[MAXPATHLEN+1] ;

	if (pip->folderdname[0] != '/') {
	    if ((rs = mkpath2(tbuf,pip->homedname,pip->folderdname)) >= 0) {
	        const char	**vpp = &pip->folderdname ;
	        rs = proginfo_setentry(pip,vpp,tbuf,rs) ;
	    }
	}

	if (pip->mbox != NULL) {
	    const char	*folder = pip->folderdname ;
	    if ((rs = procfoldercheck(pip,folder)) >= 0) {
	        rs = mkpath2(tbuf,pip->folderdname,pip->mbox) ;
	    }
	} else if ((mfname = getenv(VARMBOX)) != NULL) {
	    const char	*cp ;
	    int		cl ;
	    f_store = FALSE ;
	    pip->mfname = mfname ;
	    if (mfname[0] == '/') {
	        if ((cl = sfdirname(mfname,-1,&cp)) > 0) {
	            if ((rs = mkpath1w(tbuf,cp,cl)) >= 0) {
	                rs = procfoldercheck(pip,tbuf) ;
	            }
	        }
	    } else
	        rs = SR_INVALID ;
	} else {
	    const char	*folder = pip->folderdname ;
	    pip->mbox = MAILBOX ;
	    if ((rs = procfoldercheck(pip,folder)) >= 0) {
	        rs = mkpath2(tbuf,pip->folderdname,pip->mbox) ;
	    }
	}
	if ((rs >= 0) && f_store) {
	    const char	**vpp = &pip->mfname ;
	    rs = proginfo_setentry(pip,vpp,tbuf,rs) ;
	}

#ifdef	COMMENT
	if ((rs >= 0) && (pip->mbfname != NULL)) {
	    const int	am = (W_OK | R_OK) ;
	    mbfname = pip->mbfname ;
	    if ((rs = u_access(mbfname,am)) == SR_NOENT) {
	    }
	}
#endif /* COMMENT */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procmailfname: ret mfname=%s\n",pip->mfname) ;
	    debugprintf("main/procmailfname: ret rs=%d\n",rs) ;
	}
#endif

	return rs ;
}
/* end subroutine (procmailfname) */


static int procfoldercheck(PROGINFO *pip,const char *folder)
{
	const int	am = (X_OK | R_OK) ;
	int		rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/procfoldercheck: ent folder=%s\n",folder) ;
#endif
	if ((rs = u_access(folder,am)) == SR_NOENT) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("main/procfoldercheck: access NOENT\n") ;
#endif
	    if ((rs = proginfo_realbegin(pip)) >= 0) {
	        rs = mkdirs(folder,0755) ;
	        proginfo_realend(pip) ;
	    } /* end if (proginfo_real) */
	}
	if ((rs < 0) && (! pip->f.quiet)) {
	    const char	*pn = pip->progname ;
	    bprintf(pip->efp,
	        "%s: inaccessible folder directory (%d)\n",pn,rs) ;
	    bprintf(pip->efp,
	        "%s: folder=%s\n",pn,folder) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/procfoldercheck: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (procfoldercheck) */


static int procthem(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		bytes = 0 ;
	int		f_mfd = FALSE ;
	const char	*mailfname = pip->mfname ;

/* get the mail if any */

	if ((rs >= 0) && (lip->mfd < 0)) {
	    int	of = 0 ;

	    f_mfd = TRUE ;
	    if (mailfname[0] != '\0') {

	        if (pip->debuglevel > 0) {
	            bprintf(pip->efp,"%s: target mailbox file=%s\n",
	                pip->progname,
	                (mailfname[0] != '\0') ? mailfname: "STDOUT") ;
	        }

	        of = (O_CREAT | O_APPEND | O_WRONLY) ;
	        rs = uc_open(mailfname,of,0666) ;
	        lip->mfd = rs ;

	    } else {

	        lip->mfd = FD_STDOUT ;
	        of = (O_APPEND | O_WRONLY) ;
	        rs = u_fcntl(lip->mfd,F_SETFL,of) ;

	    } /* end if */

	} else
	    lip->f.lock = FALSE ;

/* do the deed */

	if (rs >= 0) {
	    const int	mfd = lip->mfd ;
	    const int	f_lock = lip->f.lock ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: mfd=%d\n",mfd) ;
#endif

#if	CF_DEBUGN
	    nprintf(DEBFNAME,"main: mfd=%d\n",mfd) ;
#endif

	    rs = progmailbox(pip,mfd,f_lock) ;
	    bytes = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: process() rs=%d\n",rs) ;
#endif

	    if (f_mfd && (mfd >= 0)) {
	        u_close(mfd) ;
	    }
	} /* end if (ok) */

	return (rs >= 0) ? bytes : rs ;
}
/* end subroutine (procthem) */


static int maildirs_begin(PROGINFO *pip,PARAMOPT *app)
{
	VECSTR		*mdp = &pip->maildirs ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    uid_t	euid = geteuid() ;
	    uid_t	egid = getegid() ;
	    debugprintf("main/maildirs_begin: ent\n") ;
	    debugprintf("main/maildirs_begin: euid=%d egid=%d\n",euid,egid) ;
	}
#endif

	if ((rs = vecstr_start(mdp,5,0)) >= 0) {
	    int		i ;
	    cchar	*pn = pip->progname ;
	    const char	*cp ;

	    if (rs >= 0) {
	        rs = maildirs_arg(pip,app) ;
	        c += rs ;
	    }

	    for (i = 0 ; (rs >= 0) && (varmaildirs[i] != NULL) ; i += 1) {
	        rs = maildirs_env(pip,varmaildirs[i]) ;
	        c += rs ;
	    } /* end for */

	    if (rs >= 0) {
	        rs = maildirs_def(pip,VARMAIL) ;
	        c += rs ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        for (i = 0 ; vecstr_get(mdp,i,&cp) >= 0 ; i += 1) {
	            if (cp == NULL) continue ;
	            debugprintf("main/maildirs_begin: maildir=%s\n",cp) ;
	        } /* end for */
	    }
#endif

	    if (pip->debuglevel > 0) {
	        for (i = 0 ; vecstr_get(mdp,i,&cp) >= 0 ; i += 1) {
	            if (cp != NULL) {
	                bprintf(pip->efp,"%s: maildir=%s\n",pn,cp) ;
		    }
	        } /* end for */
	    }

	} /* end if (vecstr_start) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/maildirs_begin: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (maildirs_begin) */


static int maildirs_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecstr_finish(&pip->maildirs) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (maildirs_end) */


static int maildirs_env(PROGINFO *pip,cchar *var)
{
	VECSTR		*vlp = &pip->maildirs ;
	int		rs = SR_OK ;
	int		sl, cl ;
	int		c = 0 ;
	const char	*sp, *cp ;

	if ((vlp == NULL) || (var == NULL))
	    return SR_FAULT ;

	if ((sp = getourenv(pip->envv,var)) != NULL) {
	    cchar	*tp ;
	    sl = strlen(sp) ;

	    while ((tp = strnpbrk(sp,sl," ,:\t\n")) != NULL) {
	        if ((cl = sfshrink(sp,(tp - sp),&cp)) > 0) {
	            rs = maildirs_add(pip,cp,cl) ;
	            c += rs ;
	        }
	        sl -= ((tp + 1) - sp) ;
	        sp = (tp + 1) ;
	        if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (sl > 0)) {
	        if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	            rs = maildirs_add(pip,cp,cl) ;
	            c += rs ;
	        }
	    } /* end if */

	} /* end if (got var) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (maildirs_env) */


static int maildirs_arg(PROGINFO *pip,PARAMOPT *app)
{
	int		rs ;
	int		c = 0 ;
	const char	*qn = PO_MAILDIRS ;

	if ((rs = paramopt_havekey(app,qn)) >= 0) {
	    PARAMOPT_CUR	cur ;
	    int			vl ;
	    const char		*vp ;
	    if ((rs = paramopt_curbegin(app,&cur)) >= 0) {
	        while ((vl = paramopt_enumvalues(app,qn,&cur,&vp)) >= 0) {
	            if (vp != NULL) {
	                rs = maildirs_add(pip,vp,vl) ;
	                c += rs ;
		    }
	            if (rs < 0) break ;
	        } /* end while */
	        paramopt_curend(app,&cur) ;
	    } /* end if (paramopt-cur) */
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	} /* end if (maildir arguments) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (maildirs_arg) */


static int maildirs_def(PROGINFO *pip,cchar varmail[])
{
	VECSTR		*vlp = &pip->maildirs ;
	int		rs ;
	int		cl ;
	int		c = 0 ;
	int		f = TRUE ;
	const char	*cp ;

	if ((rs = vecstr_count(vlp)) > 0) {
	    int		i ;
	    c = rs ;

	    f = FALSE ;
	    for (i = 0 ; vecstr_get(vlp,i,&cp) >= 0 ; i += 1) {
	        if (cp != NULL) {
	            if (strcmp(cp,"+") == 0) {
	                f = TRUE ;
	                c -= 1 ;
	                vecstr_del(vlp,i) ;
	            } else if (strcmp(cp,"-") == 0) {
	                c -= 1 ;
	                vecstr_del(vlp,i) ;
	            } /* end if */
		}
	    } /* end for */

	} /* end if (non-zero) */

	if ((rs >= 0) && f && (c == 0) && (varmail != NULL)) {
	    const char	*vp ;
	    if ((vp = getenv(varmail)) != NULL) {
	        if ((cl = sfdirname(vp,-1,&cp)) > 0) {
	            rs = maildirs_add(pip,cp,cl) ;
	            c += rs ;
	        }
	    } /* end if (getenv) */
	} /* end if */

	if ((rs >= 0) && (c == 0)) {

	    cp = MAILDNAME ;
	    cl = -1 ;
	    rs = maildirs_add(pip,cp,cl) ;
	    c += rs ;

	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (maildirs_def) */


static int maildirs_add(PROGINFO *pip,cchar *dp,int dl)
{
	vecstr		*mlp = &pip->maildirs ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (dp == NULL) return SR_FAULT ;

	if (dl < 0) dl = strlen(dp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/maildirs_add: ent md=%t\n",dp,dl) ;
#endif

	if ((dl > 0) && (dp[0] != '\0')) {
	    if ((rs = vecstr_findn(mlp,dp,dl)) == SR_NOTFOUND) {
	        const int	am = (W_OK|X_OK|R_OK) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/maildirs_add: NF md=%t\n",dp,dl) ;
#endif
	        if ((rs = maildirs_accessible(pip,dp,dl,am)) > 0) {
	            c += 1 ;
	            rs = vecstr_add(mlp,dp,dl) ;
	        } else {
	            rs = proclog_maildirbad(pip,dp,dl,rs) ;
	        }
	    } /* end if (not already) */
	} /* end if (non-zero) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/maildirs_add: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (maildirs_add) */


static int maildirs_accessible(PROGINFO *pip,cchar *dp,int dl,int am)
{
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;
	char		mbuf[MAXPATHLEN+1] ;
	if (pip == NULL) return SR_FAULT ; /* ¥ GCC and LINT */
#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	debugprintf("main/maildirs_accessible: ent md=%t\n",dp,dl) ;
	debugprintf("main/maildirs_accessible: egid=%d\n",getegid()) ;
	}
#endif
	if ((rs = mkmaildirtest(mbuf,dp,dl)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = uc_stat(mbuf,&sb)) >= 0) {
	        NULSTR		n ;
	        const char	*mdname ;
	        if ((rs = nulstr_start(&n,dp,dl,&mdname)) >= 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("main/maildirs_accessible: md=%s\n",
			mdname) ;
#endif
	            if ((rs = uc_stat(mdname,&sb)) >= 0) {
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("main/maildirs_accessible: "
				"u_stat() rs=%d\n", rs) ;
#endif
	                if (S_ISDIR(sb.st_mode)) {
	                    if ((rs = sperm(&pip->id,&sb,am)) >= 0) {
	                        f = TRUE ;
	                    } else if (isNotPresent(rs)) {
#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("main/maildirs_accessible: "
	                            "sperm() rs=%d\n",rs) ;
#endif
	                        rs = SR_OK ;
			    }
	                } /* end if (is-directory) */
	            } else if (isNotPresent(rs)) {
#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("main/maildirs_accessible: "
	                            "u_stat() md rs=%d\n",rs) ;
#endif
	                rs = SR_OK ;
		    }
	            rs1 = nulstr_finish(&n) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (nulstr) */
	    } else if (isNotPresent(rs)) {
#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("main/maildirs_accessible: "
	                            "u_stat() ms rs=%d\n",rs) ;
#endif
	        rs = SR_OK ;
	    }
	} /* end if (mkmaildirtest) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/maildirs_accessible: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (maildirs_accessible) */


static int mailusers_begin(PROGINFO *pip,PARAMOPT *app)
{
	VECSTR		*ulp = &pip->mailusers ;
	int		rs ;
	int		c = 0 ;

	if ((rs = vecstr_start(ulp,5,0)) >= 0) {
	    int		i ;
	    const char	*cp ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/mailusers_begin: 1 rs=%d\n",rs) ;
#endif

	    for (i = 0 ; (rs >= 0) && (varmailusers[i] != NULL) ; i += 1) {
	        rs = mailusers_env(pip,varmailusers[i]) ;
	        c += rs ;
	    } /* end for */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/mailusers_begin: 2 rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        rs = mailusers_arg(pip,app) ;
	        c += rs ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/mailusers_begin: 3 rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        rs = mailusers_def(pip,VARMAIL) ;
	        c += rs ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/mailusers_begin: 4 rs=%d\n",rs) ;
#endif

	    if (pip->debuglevel > 0) {
	        for (i = 0 ; vecstr_get(ulp,i,&cp) >= 0 ; i += 1) {
	            if (cp == NULL) continue ;
	            bprintf(pip->efp,"%s: mailuser=%s\n",pip->progname,cp) ;
	        } /* end for */
	    } /* end if */

	} /* end if (vecstr_start) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/mailusers_begin: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mailusers_begin) */


static int mailusers_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecstr_finish(&pip->mailusers) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (mailusers_end) */


static int mailusers_env(PROGINFO *pip,cchar *var)
{
	VECSTR		*vlp = &pip->mailusers ;
	int		rs = SR_OK ;
	int		sl, cl ;
	int		c = 0 ;
	const char	*sp, *cp ;

	if ((vlp == NULL) || (var == NULL))
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/mailusers_env: var=%s\n",var) ;
#endif

	if ((sp = getourenv(pip->envv,var)) != NULL) {
	    const char	*tp ;

	    sl = strlen(sp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/mailusers_env: mu=%t\n",
	            sp,strlinelen(sp,sl,50)) ;
#endif

	    while ((tp = strnpbrk(sp,sl," ,:\t\n")) != NULL) {

	        if ((cl = sfshrink(sp,(tp - sp),&cp)) > 0) {
	            rs = mailusers_add(pip,cp,cl) ;
	            c += rs ;
	        }

	        sl -= ((tp + 1) - sp) ;
	        sp = (tp + 1) ;

	        if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (sl > 0)) {

	        if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	            rs = mailusers_add(pip,cp,cl) ;
	            c += rs ;
	        }

	    } /* end if */

	} /* end if (got var) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/mailusers_env: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mailusers_env) */


static int mailusers_arg(PROGINFO *pip,PARAMOPT *app)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*qn = PO_MAILUSERS ;

	if (paramopt_havekey(app,qn) >= 0) {
	    PARAMOPT_CUR	cur ;
	    int			vl ;
	    const char		*vp ;

	    if ((rs = paramopt_curbegin(app,&cur)) >= 0) {

	        while ((vl = paramopt_enumvalues(app,qn,&cur,&vp)) >= 0) {
	            if (vp == NULL) continue ;

	            rs = mailusers_add(pip,vp,vl) ;
	            c += rs ;

	            if (rs < 0) break ;
	        } /* end while */

	        paramopt_curend(app,&cur) ;
	    } /* end if (cursor) */

	} /* end if (mailuser arguments) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mailuserarg) */


static int mailusers_def(PROGINFO *pip,cchar *varmail)
{
	VECSTR		*vlp = &pip->mailusers ;
	int		rs = SR_OK ;
	int		cl ;
	int		c = 0 ;
	int		f = TRUE ;
	const char	*username = pip->username ;
	const char	*cp ;

	if ((rs = vecstr_count(vlp)) > 0) {
	    int	i ;
	    c = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/mailusers_def: 1 rs=%d c=%u\n",rs,c) ;
#endif

	    f = FALSE ;
	    for (i = 0 ; vecstr_get(vlp,i,&cp) >= 0 ; i += 1) {
	        if (cp == NULL) continue ;

	        if (strcmp(cp,"+") == 0) {
	            f = TRUE ;
	            c -= 1 ;
	            vecstr_del(vlp,i) ;
	        } else if (strcmp(cp,"-") == 0) {
	            c -= 1 ;
	            vecstr_del(vlp,i) ;
	        } /* end if */

	    } /* end for */

	} /* end if (non-zero) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/mailusers_def: 3 rs=%d c=%u\n",rs,c) ;
#endif

	if ((rs >= 0) && f && (c == 0) && (varmail != NULL)) {
	    const char	*vp ;
	    if ((vp = getenv(varmail)) != NULL) {
	        if ((cl = sfbasename(vp,-1,&cp)) > 0) {
	            f = FALSE ;
	            rs = mailusers_add(pip,cp,cl) ;
	            c += rs ;
	        }
	    }
	} /* end if (VARMAIL) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/mailusers_def: 4 rs=%d c=%u\n",rs,c) ;
#endif

	if ((rs >= 0) && f) {
	    rs = mailusers_add(pip,username,-1) ;
	    c += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/mailusers_def: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mailusers_def) */


static int mailusers_add(PROGINFO *pip,cchar *dp,int dl)
{
	vecstr		*mlp = &pip->mailusers ;
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
/* end subroutine (mailusers_add) */


static int proclog_maildirbad(PROGINFO *pip,cchar *dp,int dl,int mrs)
{
	int		rs = SR_OK ;
	const char	*pn = pip->progname ;

	if (pip->efp != NULL) {
	    cchar	*fmt = "%s: maildir=%t inaccessible (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,dp,dl,mrs) ;
	}

	if (pip->open.logprog) {
	    const char	*fmt = "maildir=%t inaccessible (%d)" ;
	    proglog_printf(pip,fmt,dp,dl,mrs) ;
	}

	if (pip->open.logsys) {
	    const char	*fmt = "maildir=%t inaccessible (%d)" ;
	    const int	pri = LOG_NOTICE ;
	    logsys_printf(&pip->ls,pri,fmt,dp,dl,mrs) ;
	}

	return rs ;
}
/* end subroutine (proclog_maildirbad) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->mfd = -1 ;

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
#endif /* CF_LOCSETENT */


