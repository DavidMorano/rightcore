/* main (VMAIL) */
/* lang=C89 */

/* the VMAIL program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_CONFIG	1		/* call configuration */
#define	CF_LOGID	0		/* |procuserinfo_logid()| */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        This code module is completely original but is took inspiration from the
        original VMAIL program. In actually, this particular file bears no
        resemblance to the original "mail," but the idea is there. Actually not
        a single piece of this new VMAIL program bears any resemblance to any of
        the old code. The whole of that old program has been rewritten from the
        ground up.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is the front-end subroutine for the PCS program VMAIL. There is a
        good bit of setup in this subroutine before the program goes
        interactive.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/systeminfo.h>
#include	<sys/stat.h>
#include	<sys/timeb.h>
#include	<unistd.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tzfile.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<toxc.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<bfile.h>
#include	<ids.h>
#include	<userinfo.h>
#include	<pcsconf.h>
#include	<pcspoll.h>
#include	<vecstr.h>
#include	<expcook.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"kshlib.h"
#include	"config.h"
#include	"defs.h"
#include	"proglog.h"


/* local defines */


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	ctdecl(char *,int,long) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	mktmpuserdir(char *,const char *,const char *,mode_t) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	pcsgetorg(const char *,char *,int,const char *) ;
extern int	initnow(struct timeb *,char *,int) ;
extern int	gethz(int) ;
extern int	getarchitecture(char *,int) ;
extern int	getmailgid(const char *,gid_t) ;
extern int	getsystypenum(char *,char *,const char *,const char *) ;
extern int	getnprocessors(const char **,int) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	hasalldig(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isFailOpen(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_rootname(PROGINFO *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	progconf_begin(PROGINFO *) ;
extern int	progconf_end(PROGINFO *) ;

extern int	progopts(PROGINFO *,KEYOPT *) ;
extern int	progmailget(PROGINFO *,PARAMOPT *) ;
extern int	progterm(PROGINFO *) ;
extern int	defproc(vecstr *,cchar **,EXPCOOK *,cchar *) ;

extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;
extern char	*strncpylow(char *,cchar *,int) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	mainsub(int,cchar **,cchar **) ;

static int	usage(PROGINFO *) ;

static int	procmain(PROGINFO *,PARAMOPT *) ;
static int	procterm(PROGINFO *,PARAMOPT *) ;
static int	procvarload(PROGINFO *) ;
static int	procuserdir(PROGINFO *) ;
static int	procfolder(PROGINFO *) ;
static int	prochelpcmd(PROGINFO *) ;
static int	procmailcheck(PROGINFO *) ;
static int	procscanspec(PROGINFO *) ;
static int	procmaildname(PROGINFO *) ;
static int	procmbnames(PROGINFO *) ;
static int	procmaildirs_report(PROGINFO *,PARAMOPT *) ;
static int	procmaildirs(PROGINFO *,PARAMOPT *) ;
static int	procmaildir(PROGINFO *,PARAMOPT *,cchar *,int) ;
static int	procutil(PROGINFO *) ;
static int	proclog_session(PROGINFO *pip) ;

static int	procexp_begin(PROGINFO *) ;
static int	procexp_end(PROGINFO *) ;

static int	procsched_begin(PROGINFO *) ;
static int	procsched_end(PROGINFO *) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procuserinfo_org(PROGINFO *) ;

#if	CF_LOGID
static int	procuserinfo_logid(PROGINFO *) ;
#endif /* CF_LOGID */

static int	procpcsconf_begin(PROGINFO *,PCSCONF *) ;
static int	procpcsconf_end(PROGINFO *) ;

static int	procmailusers_begin(PROGINFO *,PARAMOPT *) ;
static int	procmailusers_end(PROGINFO *) ;
static int	procmailusers_load(PROGINFO *,PARAMOPT *) ;
static int	procmailusers_env(PROGINFO *,cchar *) ;
static int	procmailusers_arg(PROGINFO *,PARAMOPT *) ;
static int	procmailusers_def(PROGINFO *,cchar *) ;
static int	procmailusers_add(PROGINFO *,cchar *,int) ;

static int	procmail_begin(PROGINFO *,PARAMOPT *) ;
static int	procmail_end(PROGINFO *) ;

static int	loadschedvars(PROGINFO *) ;
static int	loadgroupname(PROGINFO *) ;
static int	loadarchitecture(PROGINFO *) ;
static int	loadhz(PROGINFO *) ;
static int	loadcooks(PROGINFO *) ;


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"VERSION",
	"HELP",
	"TERM",
	"KEYBOARD",
	"KBD",
	"LINES",
	"sn",
	"tmpdir",
	"td",
	"maildir",
	"md",
	"folderdir",
	"fd",
	"editor",
	"mailer",
	"metamail",
	"getmail",
	"tfd",
	"af",
	"ef",
	"cf",
	"lf",
	"pcf",
	"sl",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_help,
	argopt_term,
	argopt_keyboard,
	argopt_kbd,
	argopt_lines,
	argopt_sn,
	argopt_tmpdir,
	argopt_td,
	argopt_maildir,
	argopt_md,
	argopt_folderdir,
	argopt_fd,
	argopt_editor,
	argopt_mailer,
	argopt_metamail,
	argopt_getmail,
	argopt_tfd,
	argopt_af,
	argopt_ef,
	argopt_cf,
	argopt_lf,
	argopt_pcf,
	argopt_sl,
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
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static cchar	*cooks[] = {
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
	"RN",		/* program-root-name */
	"OSTYPE",	/* OS-type */
	"OSNUM",	/* OS-name */
	"S",		/* searchname */
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
	cook_ostype,
	cook_osnum,
	cook_s,
	cook_overlast
} ;

static cchar	*varmaildirs[] = {
	VARMAILDNAMEP,
	VARMAILDNAME,
	VARMAILDNAMES,
	NULL
} ;

static cchar	*varmailusers[] = {
	VARMAILUSERSP,
	VARMAILUSERS,
	NULL
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;
	if ((rs = lib_mainbegin(envv,NULL)) >= 0) {
	    ex = mainsub(argc,argv,envv) ;
	    rs1 = lib_mainend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (lib-main) */
	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;
	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


int mainsub(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	akparams ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*pcfname = NULL ;
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

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* message vaiables stuff */

	pip->prog_shell = PROG_SHELL ;
	pip->prog_getmail = PROG_GETMAIL ;
	pip->prog_editor = PROG_EDITOR ;
	pip->prog_mailer = PROG_MAILER ;
	pip->prog_metamail = PROG_METAMAIL ;
	pip->prog_pager = PROG_PAGER ;
	pip->prog_postspam = PROG_POSTSPAM ;

	pip->tfd = FD_STDIN ;
	pip->verboselevel = 1 ;

	pip->to_config = TO_CONFIG ;
	pip->to_clock = TO_CLOCK ;
	pip->to_read = TO_READ ;
	pip->to_info = TO_INFO ;

	pip->f.pcspoll = TRUE ;
	pip->f.mailget = TRUE ;
	pip->f.mailcheck = TRUE ;
	pip->f.clock = TRUE ;
	pip->f.nextdel = TRUE ;
	pip->f.nextmov = TRUE ;
	pip->f.useclen = DEFOPT_USECLEN ;
	pip->f.useclines = DEFOPT_USECLINES ;
	pip->f.winadj = DEFOPT_WINADJ ;
	pip->f.bb = FALSE ;

/* key options */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

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
	        const int ch = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ch)) {

	            argval = (argp + 1) ;

	        } else if (argp[1] == '-') {

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

	                case argopt_term:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->termtype = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->termtype = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_keyboard:
	                case argopt_kbd:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->kbdtype = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->kbdtype = argp ;
	                        } else
	                            rs = SR_INVALID ;
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

	                case argopt_tmpdir:
	                case argopt_td:
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
				PARAMOPT	*pop = &akparams ;
	                        cchar		*po = PO_MAILDIRS ;
	                        rs = paramopt_loads(pop,po,cp,cl) ;
	                    }
	                    break ;

	                case argopt_folderdir:
	                case argopt_fd:
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
	                        cchar	**vpp = &pip->folderdname ;
	                        rs = proginfo_setentry(pip,vpp,cp,cl) ;
	                    }
	                    break ;

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* getmail program */
	                case argopt_getmail:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->prog_getmail = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->prog_getmail = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* editor program */
	                case argopt_editor:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->prog_editor = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->prog_editor = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* mailer program */
	                case argopt_mailer:
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

/* metamail program */
	                case argopt_metamail:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->prog_metamail = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->prog_metamail = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* Terminal File Descriptor (TFD) */
	                case argopt_tfd:
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
	                        pip->tfd = rs ;
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

/* log file */
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

/* PCS configuration file */
	                case argopt_pcf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pcfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pcfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* scan-view lines */
	                case argopt_sl:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            pip->final.svspec = TRUE ;
	                            pip->have.svspec = TRUE ;
	                            pip->svspec = argp ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* request terminal lines to use */
	                case argopt_lines:
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
	                        pip->lines_req = rs ;
	                    }
	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

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

/* editor program */
	                    case 'E':
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->prog_editor = avp ;
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl)
	                                    pip->prog_editor = argp ;
	                            } else
	                                rs = SR_INVALID ;
	                        }
	                        break ;

	                    case 'K':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->kbdtype = argp ;
	                        } else
	                            rs = SR_INVALID ;
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

	                    case 'T':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->termtype = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* flag to automatically get new bulletins on 'bbtemp' mailbox */
	                    case 'b':
	                        pip->f.bb = TRUE ;
	                        break ;

/* number of terminal lines */
	                    case 'l':
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
	                            pip->lines_req = rs ;
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

/* mail user */
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
	                        if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
				    PARAMOPT	*pop = &akparams ;
	                            cchar	*po = PO_MAILUSERS ;
	                            rs = paramopt_loads(pop,po,cp,cl) ;
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

/* line-width (columns) */
	                    case 'w':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->have.linelen = TRUE ;
	                                pip->final.linelen = TRUE ;
	                                rs = optvalue(argp,argl) ;
	                                pip->linelen = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
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

	        } /* end if */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = getourenv(envv,VARERRORFNAME) ;
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
	    bprintf(pip->efp,"%s: version=%s\n",
	        pip->progname,VERSION) ;
	}

/* other initialization */

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
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_usage || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* argument and initialization */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if ((rs >= 0) && (pip->logsize == 0) && (argval != NULL)) {
	    int	v ;
	    rs = cfdecmfi(argval,-1,&v) ;
	    pip->logsize = v ;
	}

	if (rs >= 0) {
	    rs = initnow(&pip->now,pip->zname,ZNAMELEN) ;
	    pip->daytime = pip->now.time ;
	    pip->ti_start = pip->now.time ;
	}

	if (rs >= 0) {
	    if ((rs = proginfo_pwd(pip)) >= 0) {
	        if ((rs = proginfo_rootname(pip)) >= 0) {
		     rs = progopts(pip,&akopts) ;
		}
	    }
	}

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

	if (pip->cfname == NULL) pip->cfname = getourenv(envv,VARCONF) ;

	if (pip->tmpdname == NULL)  {
	    pip->tmpdname = getourenv(envv,VARTMPDNAMEP) ;
	}
	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if (pip->cfname != NULL) {
		fmt = "%s: cf=%s\n" ;
	        bprintf(pip->efp,fmt,pn,pip->cfname) ;
	    }
	    if (pcfname != NULL) {
		fmt = "%s: pcf=%s\n" ;
	        bprintf(pip->efp,fmt,pn,pcfname) ;
	    }
	}

	if ((rs >= 0) && (! pip->final.linelen)) {
	    if ((cp = getourenv(envv,VARCOLUMNS)) != NULL) {
	        pip->final.linelen = TRUE ;
	        rs = optvalue(cp,-1) ;
		pip->linelen = rs ;
	    }
	}

	if (pip->linelen == 0) pip->linelen = COLUMNS ;

/* terminal and keyboard types */

	if ((pip->termtype == NULL) || (pip->termtype[0] == '\0'))
	    pip->termtype = getourenv(envv,VARTERM) ;

	if ((pip->kbdtype == NULL) || (pip->kbdtype[0] == '\0'))
	    pip->kbdtype = getourenv(envv,VARKEYBOARDP) ;

	if ((pip->kbdtype == NULL) || (pip->kbdtype[0] == '\0'))
	    pip->kbdtype = getourenv(envv,VARKEYBOARD) ;

	if ((pip->kbdtype == NULL) || (pip->kbdtype[0] == '\0'))
	    pip->kbdtype = pip->termtype ;

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: termtype=%s\n", pip->progname,pip->termtype) ;
	    bprintf(pip->efp,"%s: kbdtype=%s\n", pip->progname,pip->kbdtype) ;
	} /* end if */

/* process the arguments stuff that we have */

	for (ai = 1 ; ai < argc ; ai += 1) {
	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        cp = argv[ai] ;
	        if (cp[0] != '\0') {
	            pan += 1 ;
	            pip->mbname_def = cp ;
	            break ;
	        }
	    }
	} /* end for */

	if (pip->efp != NULL) {
	    bcontrol(pip->efp,BC_NOBUF) ;
	}

/* continue */

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	            PCSCONF	pc, *pcp = &pc ;
		    cchar	*pr = pip->pr ;
	            cchar	*cfn = pcfname ;
	            if ((rs = pcsconf_start(pcp,pr,envv,cfn)) >= 0) {
	                pip->pcsconf = pcp ;
	                pip->open.pcsconf = TRUE ;
	                if ((rs = procpcsconf_begin(pip,pcp)) >= 0) {
	                    if ((rs = proglog_begin(pip,&u)) >= 0) {
	                        if ((rs = proguserlist_begin(pip)) >= 0) {
				    {
					PARAMOPT	*pop = &akparams ;
					rs = procmain(pip,pop) ;
				    }
	                            rs1 = proguserlist_end(pip) ;
				    if (rs >= 0) rs = rs1 ;
	                        } /* end if (proguserlist) */
	                        rs1 = proglog_end(pip) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (proglogfile) */
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
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,"%s: invalid query (%d)\n",
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
	        } /* end while */
	        ucmallreg_curend(&cur) ;
	    } /* end if (positive) */
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad arguments */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [-<options>] [<mailbox>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "\t<option(s)>       description of option:\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "\t-b                update 'bbtemp' with new bulletins\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "\t-L <lines>        number of terminal lines\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "\t-H <lines>        number of message header lines\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procmain(PROGINFO *pip,PARAMOPT *pop)
{
	int		rs ;
	int		rs1 ;
	if ((rs = procvarload(pip)) >= 0) {
	    if ((rs = procexp_begin(pip)) >= 0) {
		if ((rs = procsched_begin(pip)) >= 0) {
		    if ((rs = progconf_begin(pip)) >= 0) {
			if ((rs = procmail_begin(pip,pop)) >= 0) {
			    if ((rs = procutil(pip)) >= 0) {
			        rs = procterm(pip,pop) ;
			    }
			    rs1 = procmail_end(pip) ;
			    if (rs >= 0) rs = rs1 ;
			} /* end if (procmail) */
			rs1 = progconf_end(pip) ;
			if (rs >= 0) rs = rs1 ;
		    } /* end if (progconf) */
		    rs1 = procsched_end(pip) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (procsched) */
		rs1 = procexp_end(pip) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (procexp) */
	    rs1 = proclog_session(pip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (procvarload) */
	return rs ;
}
/* end subroutine (procmain) */


static int procterm(PROGINFO *pip,PARAMOPT *pop)
{
	int		rs = SR_OK ;

	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: mailget=%u\n",
	            pip->progname,pip->f.mailget) ;
	    }

	    if (pip->f.mailget) {
	        rs = progmailget(pip,pop) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: progmailget() rs=%d\n",rs) ;
#endif
	    }

/* now we are starting to execute VMAIL specific program stuff */

	    if (rs >= 0) {
	        rs = progterm(pip) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: progterm() rs=%d\n",rs) ;
#endif
	    } /* end if (progterm) */

	return rs ;
}
/* end subroutine (procterm) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

	pip->usysname = uip->sysname ;
	pip->umachine = uip->machine ;
	pip->urelease = uip->release ;
	pip->uversion = uip->version ;
	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->userhome = uip->homedname ;
	pip->gecosname = uip->gecosname ;
	pip->shell = uip->shell ;
	pip->org = uip->organization ;
	pip->realname = uip->realname ;
	pip->name = uip->name ;
	pip->fullname = uip->fullname ;
	pip->mailname = uip->mailname ;
	pip->org = uip->organization ;
	pip->logid = uip->logid ;
	pip->maildname = uip->md ;
	pip->tz = uip->tz ;

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
	}

#if	CF_LOGID
	if (rs >= 0) {
	    rs = procuserinfo_logid(pip) ;
	} /* end if (ok) */
#endif /* CF_LOGID */

	if (rs >= 0) {
	    if ((rs = procuserinfo_org(pip)) >= 0) {
	        if ((rs = ids_load(&pip->id)) >= 0) {
		    if ((rs = getmailgid(MAILGNAME,MAILGID)) >= 0) {
		        pip->gid_mail = rs ;
		    }
		    if (rs < 0) {
			ids_release(&pip->id) ;
		    }
		} /* end if (ids_load) */
	    } /* end if (procuserinfo_org) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

	rs1 = ids_release(&pip->id) ;
	if (rs >= 0) rs = rs1 ;

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


static int procuserinfo_org(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if ((pip->org == NULL) || (pip->org[0] == '\0')) {
	    const int	olen = ORGLEN ;
	    cchar	*un = pip->username ;
	    char	obuf[ORGLEN+1] ;
	    if ((rs = pcsgetorg(pip->pr,obuf,olen,un)) >= 0) {
	        int	ol = rs ;
	        cchar	**vpp = &pip->org ;
	        rs = proginfo_setentry(pip,vpp,obuf,ol) ;
	    }
	}
	return rs ;
}
/* end subroutine (procuserinfo_org) */


static int procpcsconf_begin(PROGINFO *pip,PCSCONF *pcp)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_rest/procpcsconf_begin: ent\n") ;
#endif

	if (pcp == NULL) return SR_FAULT ;

	if (pip->open.pcsconf) {

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
	                debugprintf("b_rest/procpcsconf: pair> %s=%t\n",
	                    kbuf,vbuf,vl) ;
	            } /* end while */
	            pcsconf_curend(pcp,&cur) ;
	        } /* end if (cursor) */
	    }
#endif /* CF_DEBUG */

	    if (rs >= 0) {
		const int	psize = sizeof(PCSPOLL) ;
		void		*p ;
	        if ((rs = uc_malloc(psize,&p)) >= 0) {
		    PCSPOLL	*pp = p ;
		    cchar	*sn = pip->searchname ;
		    pip->pcspoll = p ;
	            if ((rs = pcspoll_start(pp,pcp,sn)) >= 0) {
		         pip->open.pcspoll = TRUE ;
		    }
		    if (rs < 0) {
			uc_free(p) ;
			pip->pcspoll = NULL ;
		    }
		} /* end if (m-a) */
	    } /* end if (ok) */

	} /* end if (configured) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_rest/procpcsconf_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procpcsconf_begin) */


static int procpcsconf_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

	if (pip->open.pcspoll && (pip->pcspoll != NULL)) {
	    PCSPOLL	*pp = pip->pcspoll ;
	    rs1 = pcspoll_finish(pp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(pip->pcspoll) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->pcspoll = NULL ;
	} /* end if (pcspoll) */

	return rs ;
}
/* end subroutine (procpcsconf_end) */


static int procmail_begin(PROGINFO *pip,PARAMOPT *pop)
{
	int		rs ;
	if ((rs = procmaildname(pip)) >= 0) {
	    if ((rs = procmaildirs(pip,pop)) >= 0) {
		if ((rs = procmaildirs_report(pip,pop)) >= 0) {
	            if ((rs = procmailusers_begin(pip,pop)) >= 0) {
	                if ((rs = procscanspec(pip)) >= 0) {
		            rs = procmbnames(pip) ;
		        }
		    }
	        } /* end if (procmailuser) */
	    } /* end if (procmaildirs) */
	} /* end if (procmaildname) */
	return rs ;
}
/* end subroutine (procmail_begin) */


static int procmail_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip == NULL) return SR_FAULT ;
	rs1 = procmailusers_end(pip) ;
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (procmail_end) */


static int procvarload(PROGINFO *pip)
{
	int		rs ;
	if ((rs = loadgroupname(pip)) >= 0) {
	    if ((rs = loadarchitecture(pip)) >= 0) {
		if ((rs = loadhz(pip)) >= 0) {
		    if ((rs = getnprocessors(pip->envv,0)) >= 0) {
			pip->ncpu = rs ;
		    }
		}
	    }
	}
	return rs ;
}
/* end subroutine (procvarload) */


static int procuserdir(PROGINFO *pip)
{
	int		rs ;
	cchar		*pn = pip->progname ;
	cchar		*un = pip->username ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mktmpuserdir(tbuf,un,pn,VMDMODE)) >= 0) {
	    cchar	**vpp = &pip->vmdname ;
	    rs = proginfo_setentry(pip,vpp,tbuf,rs) ;
	}
	return rs ;
}
/* end subroutine (procuserdir) */


static int proclog_session(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->open.logprog) {
	    pip->daytime = time(NULL) ;
	    if (pip->ti_start != 0) {
	        time_t	sessionint = (pip->daytime - pip->ti_start) ;
	        char	tbuf[TIMEBUFLEN+1] ;
	        timestr_elapsed(sessionint,tbuf) ;
	        proglog_printf(pip,"session duration %s",tbuf) ;
	    }
	    proglog_printf(pip,"exiting (%d)",rs) ;
	} /* end if (logging) */
	return rs ;
}
/* end subroutine (proclog_session) */


static int procscanspec(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		cl = -1 ;
	cchar		*cp = pip->svspec ;
	cchar		*tp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procscanspec: S svspec=>%t<\n",cp,cl) ;
#endif

	if ((rs >= 0) && (cp != NULL) && (cp[0] != '\0')) {

	    if ((tp = strchr(cp,'%')) != NULL) {
	        cl = (tp - cp) ;
	        pip->f.svpercent = TRUE ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procscanspec: P svspec=>%t<\n",cp,cl) ;
#endif

	    rs = cfdeci(cp,cl,&pip->svlines) ;

	} /* end if */

/* scan-jump lines */

	cp = pip->sjspec ;
	cl = -1 ;
	if ((rs >= 0) && (cp != NULL) && (cp[0] != '\0')) {

	    if ((tp = strchr(cp,'%')) != NULL) {
	        cl = (tp - cp) ;
	        pip->f.sjpercent = TRUE ;
	    }

	    rs = cfdeci(cp,cl,&pip->sjlines) ;

	} /* end if */

	return rs ;
}
/* end subroutine (procscanspec) */


static int procexp_begin(PROGINFO *pip)
{
	int		rs ;
	if ((rs = expcook_start(&pip->cooks)) >= 0) {
	    rs = loadcooks(pip) ;
	}
	return rs ;
}
/* end subroutine (procexp_begin) */


static int procexp_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	rs1 = expcook_finish(&pip->cooks) ;
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (procexp_end) */


static int procsched_begin(PROGINFO *pip)
{
	int		rs ;

	if ((rs = vecstr_start(&pip->svars,6,0)) >= 0) {
	    pip->open.svars = TRUE ;

	    rs = loadschedvars(pip) ;

	    if (rs < 0) {
	        pip->open.svars = FALSE ;
	        vecstr_finish(&pip->svars) ;
	    }
	} /* end if (svars) */

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


static int procmailusers_begin(PROGINFO *pip,PARAMOPT *pop)
{
	int		rs ;
	if ((rs = vecstr_start(&pip->mailusers,5,0)) >= 0) {
	    pip->open.mailusers = TRUE ;
	    rs = procmailusers_load(pip,pop) ;
	    if (rs < 0) {
	        pip->open.mailusers = FALSE ;
		vecstr_finish(&pip->mailusers) ;
	    }
	}
	return rs ;
}
/* end subroutine (procmailusers_begin) */


static int procmailusers_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->open.mailusers) {
	    pip->open.mailusers = FALSE ;
	    rs1 = vecstr_finish(&pip->mailusers) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (procmailusers_end) */


static int procmailusers_load(PROGINFO *pip,PARAMOPT *app)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procmailusers_load: ent\n") ;
#endif

	if (rs >= 0) {
	    for (i = 0 ; (rs >= 0) && (varmailusers[i] != NULL) ; i += 1) {
	        rs = procmailusers_env(pip,varmailusers[i]) ;
	        c += rs ;
	    }
	}

	if (rs >= 0) {
	    rs = procmailusers_arg(pip,app) ;
	    c += rs ;
	}

	if (rs >= 0) {
	    rs = procmailusers_def(pip,VARMAIL) ;
	    c += rs ;
	}

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    vecstr	*mlp = &pip->mailusers ;
	    for (i = 0 ; (vecstr_get(mlp,i,&cp) >= 0) ; i += 1) {
	        if (cp != NULL) {
	            bprintf(pip->efp,"%s: mailuser=%s\n",pn,cp) ;
		}
	    } /* end for */
	} /* end if (debugging) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procmailusers_load: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers_load) */


static int procmailusers_env(PROGINFO *pip,cchar *var)
{
	VECSTR		*vlp = &pip->mailusers ;
	int		rs  = SR_OK ;
	int		sl ;
	int		c = 0 ;
	cchar		*sp ;

	if ((vlp == NULL) || (var == NULL))
	    return SR_FAULT ;

	if ((sp = getourenv(pip->envv,var)) != NULL) {
	    int		cl ;
	    const char	*tp, *cp ;

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

	} /* end if (getourenv) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers_env) */


static int procmailusers_arg(PROGINFO *pip,PARAMOPT *app)
{
	int		rs ;
	int		c = 0 ;
	cchar		*po = PO_MAILUSERS ;

	if ((rs = paramopt_havekey(app,po)) > 0) {
	    PARAMOPT_CUR	cur ;

	    if ((rs = paramopt_curbegin(app,&cur)) >= 0) {
	        int	cl ;
	        cchar	*cp ;

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
	    } /* end if */

	} /* end if (mailuser arguments) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers_arg) */


static int procmailusers_def(PROGINFO *pip,cchar *varmail)
{
	VECSTR		*vlp = &pip->mailusers ;
	int		rs ;
	int		cl ;
	int		c = 0 ;
	int		f = TRUE ;
	cchar		**envv = pip->envv ;
	cchar		*un = pip->username ;
	cchar		*cp ;

	if ((rs = vecstr_count(vlp)) > 0) {
	    int		i ;
	    c = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procmailusers_def: args rs=%d c=%d\n",rs,c) ;
#endif

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
		} /* end if (non-NULL) */
	    } /* end for */

	} /* end if (non-zero) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procmailusers_def: mid1 rs=%d c=%d f=%u\n",
		rs,c,f) ;
#endif

	if ((rs >= 0) && f && (c == 0) && (varmail != NULL)) {
	    cchar	*vp ;
	    if ((vp = getourenv(envv,varmail)) != NULL) {
	        if ((cl = sfbasename(vp,-1,&cp)) > 0) {
	            f = FALSE ;
	            rs = procmailusers_add(pip,cp,cl) ;
	            c += rs ;
	        }
	    }
	} /* end if (VARMAIL) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procmailusers_def: mid2 rs=%d c=%u f=%u u=%s\n",
		rs,c,f,un) ;
#endif

	if ((rs >= 0) && f) {
	    rs = procmailusers_add(pip,un,-1) ;
	    c += rs ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procmailusers_def: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers_def) */


static int procmailusers_add(PROGINFO *pip,cchar *dp,int dl)
{
	vecstr		*mlp = &pip->mailusers ;
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("main/procmailusers_add: ent d=%t\n",dp,dl) ;
#endif

	if (dp == NULL) return SR_FAULT ;

	if (dl < 0) dl = strlen(dp) ;

	if ((dl > 0) && (dp[0] != '\0')) {
	    if ((rs = vecstr_findn(mlp,dp,dl)) == SR_NOTFOUND) {
	        c += 1 ;
	        rs = vecstr_add(mlp,dp,dl) ;
	    } /* end if (not already) */
	} /* end if (non-zero) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("main/procmailusers_add: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmailusers_add) */


#ifdef	COMMENT

static int procprogs_begin(PROGINFO *pip)
{
	int		rs ;
	if ((rs = procprogs_mailer(pip)) >= 0) {
	    rs = 1 ;
	}
	return rs ;
}
/* end subroutine (procprogs_begin) */


static int procprogs_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	return rs ;
}
/* end subroutine (procprogs_end) */


static int procprogs_mailer(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if ((rs >= 0) && (pip->prog_mailer == NULL)) {
	    cchar	*pn = pip->progname ;
	    pip->prog_mailer = PROG_MAILER ;
	    if ((tolc(pn[0]) == 'n') || (tolc(pn[0]) == 'o')) {
	        cchar	*pm = PROG_MAILER ;
		char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = bufprintf(tbuf,MAXPATHLEN,"%c%s",pn[0],pm)) >= 0) {
	            cchar	**vpp = &pip->prog_mailer ;
	            rs = proginfo_setentry(pip,vpp,buf,rs1) ;
	        }
	    }
	} /* end if */
	return rs ;
}
/* end subroutine (procprogs_mailer) */

#endif /* COMMENT */


static int procmaildirs_report(PROGINFO *pip,PARAMOPT *pop)
{
	PARAMOPT_CUR	cur ;
	int		rs ;
	int		rs1 ;
	cchar		*po = PO_MAILDIRS ;
	if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: md=%s\n" ;
	    cchar	*cp ;
	    while ((rs1 = paramopt_fetch(pop,po,&cur,&cp)) >= 0) {
		if (pip->debuglevel > 0) {
		    bprintf(pip->efp,fmt,pn,cp) ;
		}
		proglog_printf(pip,"  md=%s",cp) ;
	    } /* end while */
	    if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	    rs1 = paramopt_curend(pop,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (paramopt-cur) */
	return rs ;
}
/* end subroutine (procmaildirs_report) */


static int procmaildirs(PROGINFO *pip,PARAMOPT *pop)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	cchar		**envv = pip->envv ;
	cchar		*dns ;
	cchar		*tp ;

	for (i = 0 ; varmaildirs[i] != NULL ; i += 1) {
	    cchar	*var = varmaildirs[i] ;
	    if ((dns = getourenv(envv,var)) != NULL) {
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

	if ((rs >= 0) && (c == 0)) {
	    dns = MAILDNAME ;
	    rs = procmaildir(pip,pop,dns,-1) ;
	    c += rs ;
	}

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
	        USTAT	sb ;
	        if ((rs = u_stat(dname,&sb)) >= 0) {
	            rs = paramopt_load(pop,po,dp,dl) ;
	            c += rs ;
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
		}
	    } /* end if (mkpath) */
	} /* end if (paramopt_haveval) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmaildir) */


static int procutil(PROGINFO *pip)
{
	int		rs ;
	if ((rs = procuserdir(pip)) >= 0) {
	    if ((rs = procfolder(pip)) >= 0) {
	        if ((rs = prochelpcmd(pip)) >= 0) {
	    	    rs = procmailcheck(pip) ;
		}
	    }
	}
	return rs ;
}
/* end subroutine (procutil) */


static int procfolder(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->folderdname == NULL) {
	    cchar	**envv = pip->envv ;
	    cchar	*cp = NULL ;
	    if (cp == NULL) cp = getourenv(envv,VARFOLDERDNAME1) ;
	    if (cp == NULL) cp = getourenv(envv,VARFOLDERDNAME2) ;
	    if (cp == NULL) cp = FOLDERDNAME ;
	    if (cp != NULL) {
	        cchar	**vpp = &pip->folderdname ;
	        rs = proginfo_setentry(pip,vpp,cp,MAXPATHLEN) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: 2 folderdname=%s\n",pip->folderdname) ;
	}
#endif

	if ((rs >= 0) && (pip->folderdname[0] != '/')) {
	    cchar	*hdn = pip->userhome ;
	    cchar	*fdn = pip->folderdname ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath2(tbuf,hdn,fdn)) >= 0) {
	        cchar	**vpp = &pip->folderdname ;
	        proginfo_setentry(pip,vpp,tbuf,rs) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 3 folderdname=%s\n",pip->folderdname) ;
#endif

	if (rs >= 0) {
	    const int	am = (X_OK | R_OK) ;
	    rs = perm(pip->folderdname,-1,-1,NULL,am) ;
	    if (isNotAccess(rs)) {
	        if ((rs = mkdirs(pip->folderdname,0755)) >= 0) {
	            u_chown(pip->folderdname,-1,pip->gid_mail) ;
	            rs = perm(pip->folderdname,-1,-1,NULL,(X_OK | R_OK)) ;
	        }
	    } /* end if */
	} /* end if (ok) */

	return rs ;
}
/* end subrouroutine (procfolder) */


static int prochelpcmd(PROGINFO *pip)
{
	int		rs ;
	cchar		*cmd = CMDHELPFNAME ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mkpath2(tbuf,pip->pr,cmd)) >= 0) {
	    cchar	**vpp = &pip->hfname ;
	    rs = proginfo_setentry(pip,vpp,tbuf,rs) ;
	}
	return rs ;
}
/* end subroutine (prochelpcmd) */


static int procmailcheck(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (! pip->have.mailcheck) {
	    cchar	**envv = pip->envv ;
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARMAILCHECK1)) == NULL) {
	        cp = getourenv(envv,VARMAILCHECK2) ;
	    }
	    if (cp != NULL) {
		int	v ;
	        pip->have.mailcheck = TRUE ;
	        rs = cfdecti(cp,-1,&v) ;
	        pip->mailcheck = v ;
	    }
	    if (pip->debuglevel > 0) {
	        cchar	*fmt = "%s: mailcheck=disabled\n" ;
	        if (pip->mailcheck >= 0) fmt = "%s: mailcheck=%u\n" ;
	        bprintf(pip->efp,fmt,pip->progname,pip->mailcheck) ;
	    }
	} /* end if (mailcheck) */
	return rs ;
}
/* end subroutine (procmailcheck) */


static int procmaildname(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->maildname == NULL) {
	    pip->maildname = MAILDNAME ;
	}
	return rs ;
}
/* end subroutine (procmaildname) */


static int procmbnames(PROGINFO *pip)
{
	int		rs = SR_OK ;
	cchar		**envv = pip->envv ;
	cchar		*cp ;

	if ((rs >= 0) && (pip->mbname_in == NULL)) {
	    cp = NULL ;
	    if (cp == NULL) cp = getourenv(envv,VARMAILBOXIN1) ;
	    if (cp == NULL) cp = getourenv(envv,VARMAILBOXIN2) ;
	    if (cp == NULL) cp = MB_INPUT ;
	    if (cp != NULL) {
	        cchar	**vpp = &pip->mbname_in ;
	        rs = proginfo_setentry(pip,vpp,cp,-1) ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: mbname_in=%s\n",pip->mbname_in) ;
#endif

	if ((rs >= 0) && (pip->mbname_def == NULL)) {
	    cp = NULL ;
	    if (cp == NULL) cp = getourenv(envv,VARMAILBOXDEF1) ;
	    if (cp == NULL) cp = getourenv(envv,VARMAILBOXDEF2) ;
	    if (cp == NULL) cp = MB_DEFAULT ;
	    if (cp != NULL) {
	        cchar	**vpp = &pip->mbname_def ;
	        rs = proginfo_setentry(pip,vpp,cp,-1) ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: mbname_def=%s\n",pip->mbname_def) ;
#endif

	if (pip->mbname_spam == NULL) pip->mbname_spam = MB_SPAM ;
	if (pip->mbname_trash == NULL) pip->mbname_trash = MB_TRASH ;

/* current (initial) mailbox */

	if (pip->mbname_cur == NULL) pip->mbname_cur = pip->mbname_def ;

	return rs ;
}
/* end subroutine (procmbnames) */


static int loadschedvars(PROGINFO *pip)
{
	VECSTR		*svp = &pip->svars ;
	int		rs = SR_OK ;
	int		i ;
	cchar		*keys = "penh" ;

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
	    case 'h':
		vp = pip->userhome ;
		break ;
	    } /* end switch */
	    if ((rs >= 0) && (vp != NULL)) {
		char	kbuf[2] ;
		kbuf[0] = kch ;
		kbuf[1] = '\0' ;
	        rs = vecstr_envset(svp,kbuf,vp,vl) ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (loadschedvars) */


static int loadgroupname(PROGINFO *pip)
{
	const int	gnlen = GROUPNAMELEN ;
	int		rs ;
	char		gnbuf[GROUPNAMELEN+1] ;

	if ((rs = getgroupname(gnbuf,gnlen,pip->gid)) >= 0) {
	    cchar	**vpp = &pip->groupname ;
	    rs = proginfo_setentry(pip,vpp,gnbuf,rs) ;
	} /* end if */

	return rs ;
}
/* end subroutine (loadgroupname) */


static int loadarchitecture(PROGINFO *pip)
{
	const int	alen = ARCHBUFLEN ;
	int		rs ;
	char		abuf[ARCHBUFLEN+1] ;

	if ((rs = getarchitecture(abuf,alen)) > 0) {
	    cchar	**vpp = &pip->architecture ;
	    rs = proginfo_setentry(pip,vpp,abuf,rs) ;
	}

	return rs ;
}
/* end subroutine (loadarchitecture) */


static int loadhz(PROGINFO *pip)
{
	int		rs ;
	if ((rs = gethz(0)) >= 0) {
	    const int	dlen = DIGBUFLEN ;
	    const int	v = rs ;
	    char	dbuf[DIGBUFLEN+1] ;
	    if ((rs = ctdeci(dbuf,dlen,v)) >= 0) {
	        cchar	**vpp = &pip->hz ;
	        rs = proginfo_setentry(pip,vpp,dbuf,rs) ;
	    }
	}
	return rs ;
}
/* end subroutine (loadhz) */


static int loadcooks(PROGINFO *pip)
{
	EXPCOOK		*cop = &pip->cooks ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ci ;
	int		cl ;
	cchar		*cp ;
	char		tbuf[USERNAMELEN+1] = { 0 } ;
	char		nbuf[USERNAMELEN+1] = { 0 } ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/loadcooks: ent\n") ;
#endif

	for (ci = 0 ; cooks[ci] != NULL ; ci += 1) {
	    cp = NULL ;
	    cl = -1 ;
	    switch (ci) {
	    case cook_machine:
	        cp = pip->umachine ;
	        break ;
	    case cook_architecture:
	        cp = pip->architecture ;
	        break ;
	    case cook_ncpu:
	        {
		    const int	dlen = DIGBUFLEN ;
	            char	dbuf[DIGBUFLEN + 1] ;
	            if (pip->ncpu >= 0) {
	                rs1 = ctdeci(dbuf,dlen,pip->ncpu) ;
	            } else {
	                strcpy(dbuf,"1") ;
	                rs1 = 1 ;
	            }
	            rs = expcook_add(cop,cooks[ci],dbuf,rs1) ;
	        } /* end block */
	        break ;
	    case cook_sysname:
	        cp = pip->usysname ;
	        break ;
	    case cook_release:
	        cp = pip->urelease ;
	        break ;
	    case cook_version:
	        cp = pip->uversion ;
	        break ;
	    case cook_hz:
	        cp = pip->hz ;
	        break ;
	    case cook_u:
	        cp = pip->username ;
	        break ;
	    case cook_g:
	        cp = pip->groupname ;
	        break ;
	    case cook_home:
	        cp = pip->userhome ;
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
	            const int	hblen = MAXHOSTNAMELEN ;
		    cchar	*nn = pip->nodename ;
		    cchar	*dn = pip->domainname ;
	            char	hnbuf[MAXHOSTNAMELEN + 1] ;
	            if ((rs = snsds(hnbuf,hblen,nn,dn)) >= 0) {
	                rs = expcook_add(cop,cooks[ci],hnbuf,rs1) ;
		    }
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
	        } /* end if */
	        break ;
	    case cook_s:
	        cp = pip->searchname ;
	        break ;
	    } /* end switch */
	    if ((rs >= 0) && (cp != NULL)) {
	        rs = expcook_add(cop,cooks[ci],cp,cl) ;
	    }
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/loadcooks: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (loadcooks) */


