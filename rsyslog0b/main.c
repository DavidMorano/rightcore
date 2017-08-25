/* main */

/* small (rather generic) front-end subroutine */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_SIGNAL	1
#define	CF_PCSPOLL	1
#define	CF_PCS		0
#define	CF_SHUTDOWN	0


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ rsyslog [recipient(s) ...] [-p priority] [-t tag]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<schedvar.h>
#include	<pcsconf.h>
#include	<buffer.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"systems.h"
#include	"dialer.h"
#include	"cm.h"


/* local defines */

#define	NPARG		2	/* number of positional arguments */
#define	MAXARGINDEX	100
#define	NARGPRESENT	(MAXARGINDEX/8 + 1)

#define	NDEFADDRS	20

#define	HOSTPARTLEN	1024

#ifndef	TO_CONNECT
#define	TO_CONNECT	20
#endif

#ifndef	TO_READ
#define	TO_READ		10
#endif


/* external subroutines */

extern int	snscs(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,
			const char *,const char *) ;
extern int	sncpy5(char *,int,const char *,const char *,
			const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecfi(const char *,int,int *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			const char *,const char *) ;
extern int	pcstrustuser(const char *,const char *) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	proginfo_rootname(struct proginfo *) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_hdate(time_t,char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	makedate[] ;


/* local structures */

struct hparams {
	const char	*svcspec ;
	const char	*fromaddr ;
	const char	*logtag ;
	const char	*logpriority ;
} ;


/* forward references */

static int	usage(struct proginfo *) ;
static int	getprogopts(struct proginfo *,KEYOPT *,vecstr *) ;
static int	prochdr(struct proginfo *,USERINFO *,struct hparams *,
			CM *,vecstr *) ;

static void	int_all() ;


/* global variables */

static int	f_signal = FALSE ;
static int	signal_num = 0 ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"DEBUG",
	"VERSION",
	"VERBOSE",
	"HELP",
	"LOG",
	"MAKEDATE",
	"af",
	"if",
	"to",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_debug,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_log,
	argopt_makedate,
	argopt_af,
	argopt_if,
	argopt_to,
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
	{ 0, 0 }
} ;

static const char *progopts[] = {
	"pcspoll",
	"loglen",
	"cluster",
	"timeout",
	"mailhost",
	"mailsvc",
	NULL
} ;

enum progopts {
	progopt_pcspoll,
	progopt_loglen,
	progopt_cluster,
	progopt_timeout,
	progopt_mailhost,
	progopt_mailsvc,
	progopt_overlast
} ;

static const char	*sysfiles[] = {
	"%p/etc/%n/%n.%f",
	"%p/etc/%n/%f",
	"%p/etc/%n.%f",
	"%p/%n.%f",
	"%n.%f",
	NULL
} ;

static const char	*headers[] = {
	"message-id",
	"date",
	"host",
	"logname",
	"from",
	"tag",
	"priority",
	"to",
	"x-service",
	NULL
} ;

enum headers {
	header_msgid,
	header_date,
	header_host,
	header_logname,
	header_from,
	header_tag,
	header_priority,
	header_to,
	header_xservice,
	header_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct sigaction	sigs ;

	struct ustat	sb ;

	struct proginfo	pi, *pip = &pi ;

	struct hparams	hdr ;

	USERINFO	u ;

	PCSCONF		p ;

	SYSTEMS		sysdb ;

	DIALER		d ;

	CM		con ;

	KEYOPT		akopts ;

	sigset_t	signalmask ;

	bfile		errfile ;
	bfile		pidfile ;

	vecstr		sets, addrs ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, rs1 ;
	int	len, i, j ;
	int	sl, cl ;
	int	tlen = 0 ;
	int	timeout = -1 ;
	int	c_recips ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_systems = FALSE ;
	int	f_dialer = FALSE ;
	int	f_cm = FALSE ;
	int	f_pcsconf = FALSE ;
	int	f_addrs ;
	int	f ;

	cchar	*argp, *aop, *akp, *avp ;
	cchar	*pr = NULL ;
	cchar	*afname = NULL ;
	cchar	*ifname = NULL ;
	cchar	*logfname = NULL ;
	cchar	*hostname ;
	cchar	*loghost = NULL ;
	cchar	*svcspec = NULL ;
	cchar	*fromaddr = NULL ;
	cchar	*logpriority = NULL ;
	cchar	*logtag = NULL ;
	cchar	*tospec = NULL ;
	cchar	*sp, *cp, *cp2 ;
	char	argpresent[NARGPRESENT] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	fromaddrbuf[MAILADDRLEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	char	pcsconfbuf[PCSCONF_LEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto ret0 ;
	}

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* more initialization */

	pip->daytime = time(NULL) ;

	pip->verboselevel = 1 ;

/* process program arguments */

	rs = keyopt_start(&akopts) ;

	pip->open.akopts = (rs >= 0) ;
	for (ai = 0 ; ai < NARGPRESENT ; ai += 1)
	    argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {

	        if (isdigit(argp[1])) {

	            rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

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

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pr = argp ;

	                    }

	                    break ;

/* debug level */
	                case argopt_debug:
	                    pip->debuglevel = 1 ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            rs = cfdeci(avp,avl,&pip->debuglevel) ;

	                    }

	                    break ;

	                case argopt_version:
	                    f_version = TRUE ;
	                    break ;

	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

	                    }

	                    break ;

/* help file */
	                case argopt_help:
	                    f_help  = TRUE ;
	                    break ;

/* argument file */
	                case argopt_af:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            afname = argp ;

	                    }

	                    break ;

	                case argopt_if:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            ifname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            ifname = argp ;

	                    }

	                    break ;

/* log file */
	                case argopt_log:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            logfname = avp ;

	                    }

	                    break ;

	                case argopt_to:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            tospec = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            tospec = argp ;

	                    }

	                    break ;

/* display the time this program was last "made" */
	                case argopt_makedate:
	                    f_makedate = TRUE ;
	                    break ;

	                default:
	                    rs = SR_INVALID ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl, 
	                                    &pip->debuglevel) ;

	                        }

	                        break ;

	                    case 'R':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pr = argp ;

	                        break ;

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* from address */
	                    case 'f':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            fromaddr = argp ;

	                        break ;

	                    case 'i':
	                        pip->f.optin = TRUE ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) {

	                            rs = keyopt_loads(&akopts,argp,argl) ;

	                        }

	                        break ;

/* log priority */
	                    case 'p':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            logpriority = argp ;

	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* service */
	                    case 's':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            svcspec = argp ;

	                        break ;

/* tag */
	                    case 't':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            logtag = argp ;

	                        break ;

	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                        }

	                        break ;

/* fall through from above */
	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        bprintf(pip->efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                    } /* end switch */

	                    akp += 1 ;
	                    if (rs < 0)
	                        break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n", pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_makedate)
	    bprintf(pip->efp,"%s: built %s\n",
	        pip->progname,makedate) ;

/* program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialization */

	proginfo_rootname(pip) ;

/* check arguments */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	        BACLR(argpresent,ai) ;
	    if (cp[0] != '\0') {

	        if (strcmp(cp,"-") != 0)
	            loghost = cp ;

	    }

	    break ;

	} /* end for */

	if ((loghost == NULL) || (loghost[0] == '\0')) {

	    if ((cp = getenv(VARLOGHOST)) != NULL)
	        loghost = cp ;

	}

	if ((loghost == NULL) || (loghost[0] == '\0'))
	    loghost = LOGHOST ;

	if ((svcspec == NULL) || (svcspec[0] == '\0')) {

	    if ((cp = getenv(VARSVC)) != NULL)
	        svcspec = cp ;

	}

	if ((svcspec == NULL) || (svcspec[0] == '\0'))
	    svcspec = SVCSPEC_RSYSLOG ;

/* connection timeout */

	if ((tospec != NULL) && (tospec[0] != '\0')) {

	    rs = cfdecti(tospec,-1,&timeout) ;
	    if (rs < 0)
	        goto badarg ;

	}

	if (timeout < 0)
	    timeout = TO_CONNECT ;

	if ((logpriority == NULL) || (logpriority[0] == '\0'))
	    logpriority = LOGPRIORITY ;

	if ((logtag == NULL) || (logtag[0] == '\0'))
	    logtag = LOGTAG ;

/* who are we? */

	ids_load(&pip->id) ;

	rs = userinfo(&u,userinfobuf,USERINFO_LEN,NULL) ;
	if (rs < 0) {
	    ex = EX_NOUSER ;
	    bprintf(pip->efp,
	        "%s: could not get user information (%d)\n",
	        pip->progname,rs) ;
	    goto baduser ;
	}

	pip->pid = u.pid ;
	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;

/* loghost */

	if ((loghost != NULL) && (loghost[0] != '\0')) {
	    sncpy1(pip->mailhost,MAXHOSTNAMELEN,loghost) ;
	}

#if	CF_PCS

/* get the system PCS configuration information */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pcsconf()\n") ;
#endif

	if ((rs = vecstr_start(&sets,10,0)) {

	    rs = pcsconf(pip->pr,NULL,&p,&sets,NULL,
	        pcsconfbuf,PCSCONF_LEN) ;
	        f_pcsconf = (rs >= 0) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	        debugprintf("main: pcsconf() rs=%d\n",rs) ;
#endif


	        /* process any program options */

	    if (rs >= 0)
	        rs = getprogopts(pip,&akopts,&sets) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	        debugprintf("main: getprogopts() rs=%d\n",rs) ;
#endif

	        if (pip->mailhost[0] == '\0') {

	        if ((cp = getenv(VARMAILHOST)) != NULL)
	            sncpy1(pip->mailhost, MAXHOSTNAMELEN, cp) ;

	    }

	    if (f_pcsconf && (pip->mailhost[0] == '\0')) {

	        if ((p.mailhost != NULL) && (p.mailhost[0] != '\0')) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main: PCS mailhost=%s\n",p.mailhost) ;
#endif

	                sp = p.mailhost ;
	                sl = MAXHOSTNAMELEN ;
	                if (isindomain(p.mailhost,u.domainname)) {

	                sl = MAXHOSTNAMELEN ;
	                    if ((cp = strchr(p.mailhost,'.')) != NULL)
	                    sl = (cp - p.mailhost) ;

	            }

	            cl = MIN(sl,MAXHOSTNAMELEN) ;
	                strwcpy(pip->mailhost, sp,cl) ;

	        }
	    }

#if	CF_PCSPOLL
	    if ((rs >= 0) && pip->f.pcspoll) {
	        pip->pp = &p ;
	            (void) pcspoll(pip->pr,&p,&sets) ;
	    }
#endif /* CF_PCSPOLL */

/* we are delivering mail, but are we a trusted user? */

#ifdef	COMMENT
	    i = matstr(trustedusers,u.username,-1) ;
	        pip->f.trusted = (i >= 0) ;
#else

#endif /* COMMENT */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	        debugprintf("main: f_trusted=%d\n",pip->f.trusted) ;
#endif

	        vecstr_finish(&sets) ;

	} /* end if (pcsconf) */

#endif /* CF_PCS */

	        rs = pcstrustuser(pip->pr,u.username) ;
	        pip->f.trusted = (rs > 0) ;

/* other initialization */

	if (pip->mailhost[0] == '\0')
	    sncpy1(pip->mailhost, MAXHOSTNAMELEN, MAILHOST) ;

	    if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: mailhost=%s\n",
	    pip->progname,pip->mailhost) ;

	    /* do we have a log file? */

	if (logfname == NULL)
	    logfname = LOGFNAME ;

	    if (logfname[0] != '/') {

	    len = mkpath2(buf, pip->pr,logfname) ;

	        logfname = mallocstrw(buf,len) ;

	}

/* make a log entry */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: logfile=%s\n",
	    logfname) ;
#endif

	    rs1 = logfile_open(&pip->lh,logfname,0,0666,u.logid) ;

	    if (rs1 >= 0) {

	    pip->f.log = TRUE ;
	        logfile_userinfo(&pip->lh,&u,
	        pip->daytime,pip->progname,pip->version) ;

	} /* end if (opened a log) */

/* do the main thing */

	pip->signal_num = -1 ;

	    uc_sigsetempty(&signalmask) ;

#if	CF_SIGNAL
	    sigs.sa_handler = int_all ;
	    sigs.sa_mask = signalmask ;
	    sigs.sa_flags = 0 ;
	    u_sigaction(SIGHUP,&sigs,NULL) ;

	    sigs.sa_handler = int_all ;
	    sigs.sa_mask = signalmask ;
	    sigs.sa_flags = 0 ;
	    u_sigaction(SIGPIPE,&sigs,NULL) ;
#endif /* CF_SIGNAL */

	    /* setup for looping */

	rs = systems_open(&sysdb,NULL) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	    debugprintf("main: systems_open() rs=%d\n",rs) ;
#endif

	    f_systems = (rs >= 0) ;
	    if (rs >= 0) {

	    SCHEDVAR	sf ;

	        char	sysfname[MAXPATHLEN + 1] ;


	        schedvar_start(&sf) ;

	        schedvar_add(&sf,"p",pip->pr,-1) ;

	        schedvar_add(&sf,"n",pip->searchname,-1) ;

	        for (j = 0 ; j < 2 ; j += 1) {

	        if (j == 0)
	            schedvar_add(&sf,"f",SYSFNAME1,-1) ;

	            else
	            schedvar_add(&sf,"f",SYSFNAME2,-1) ;

	                for (i = 0 ; sysfiles[i] != NULL ; i += 1) {

	                rs1 = schedvar_expand(&sf,
	                    tmpfname,MAXPATHLEN,
				sysfiles[i],-1) ;

	                    if (rs1 >= 0)
	                    rs1 = u_stat(tmpfname,&sb) ;

	                    if (rs1 >= 0) {

	                    rs1 = SR_NOENT ;
	                        if (! S_ISDIR(sb.st_mode))
	                        rs1 = sperm(&pip->id,&sb,R_OK) ;

	                }

	                if (rs1 >= 0) {
	                    rs = systems_fileadd(&sysdb,tmpfname) ;
	                }

	                if (rs == SR_EXIST)
	                    rs = SR_OK ;

	                    if (rs < 0)
	                    break ;

	            } /* end for */

	    } /* end for */

	    schedvar_finish(&sf) ;

	        if (rs == SR_EXIST)
	        rs = SR_OK ;

	} /* end block (loading 'systems' files) */

#if	CF_DEBUG 
	if (DEBUGLEVEL(6)) {
	    SYSTEMS_CUR	cur ;
	        SYSTEMS_ENT	*sep ;
	        debugprintf("main: sysnames: \n") ;
	        systems_curbegin(&sysdb,&cur) ;
	        while (systems_enum(&sysdb,&cur,&sep) >= 0) {
	        debugprintf("main: sysname=%s\n",sep->sysname) ;
	    }
	    systems_curend(&sysdb,&cur) ;
	}
#endif /* CF_DEBUG */

/* can we initialize the DIALER subsystem? */

	if (rs >= 0) {
	    rs = dialer_init(&d,pip->pr,NULL,NULL) ;
	        f_dialer = (rs >= 0) ;
	}

/* loop through the arguments */

	c_recips = 0 ;
	    if (rs >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: getting recipients\n") ;
#endif

	        pan = 0 ;
	        rs = vecstr_start(&addrs,NDEFADDRS,0) ;
	        f_addrs = (rs >= 0) ;

	        if (rs >= 0) {

	        for (ai = 1 ; ai < argc ; ai += 1) {

	            f = (ai <= ai_max) && BATST(argpresent,ai) ;
	                f = f || (ai > ai_pos) ;
	                if (! f) continue ;

	                cp = argv[ai] ;
	                c_recips += 1 ;
	                pan += 1 ;
	                rs = vecstr_add(&addrs,cp,-1) ;
	                if (rs < 0)
	                break ;

	        } /* end for */

	    } /* end if */

#ifdef	COMMENT
	    if ((rs >= 0) && (pan == 0)) {

	        cl = sncpy3(tmpfname,MAXPATHLEN,
	            u.username,"@",u.domainname) ;

	            if (cl > 0) {
	            c_recips += 1 ;
	                rs = vecstr_add(&addrs,tmpfname,cl) ;
	        }

	    } /* end if (default revipient) */
#endif /* COMMENT */

	} /* end if (getting recipients) */

/* log recipients? */

	if ((rs >= 0) && (pip->f.log || (pip->debuglevel > 0))) {

	    for (i = 0 ; vecstr_get(&addrs,i,&cp) >= 0 ; i += 1) {

	        if (cp == NULL) continue ;

	            if (pip->f.log)
	            logfile_printf(&pip->lh,"  to=%s\n",cp) ;

	            if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: to=%s\n",pip->progname,cp) ;

	    }

	} /* end if (logging recipients) */

/* try to open a connection (with connection-manager) */

	if (rs >= 0) {

	    CM_ARGS	ca ;

	        const char	**av ;


#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	        debugprintf("main: cm_open()\n") ;
#endif /* CF_DEBUG */

	        memset(&ca,0,sizeof(CM_ARGS)) ;

	ca.pr = pip->pr ;
	ca.prn = pip->rootname ;
	ca.searchname = pip->searchname ;
	ca.nodename = pip->nodename ;
	ca.domainname = pip->domainname ;
	ca.username = pip->username ;
	        ca.sp = &sysdb ;
	        ca.dp = &d ;
	        ca.timeout = timeout ;
	        ca.options = (DIALER_MHALFOUT | DIALER_MCO | DIALER_MARGS) ;

	        vecstr_getvec(&addrs,&av) ;

	        rs = cm_open(&con,&ca,pip->mailhost,svcspec,av) ;
	        f_cm = (rs >= 0) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	        debugprintf("main: cm_open() rs=%d\n",rs) ;
#endif /* CF_DEBUG */

	} /* end if (connection-open attempt) */

/* if we have an open connection => do the deed */

	if (rs >= 0) {

	    bfile	infile ;


#if	CF_SHUTDOWN
	        cm_shutdown(&con,SHUT_RD) ;
#endif

	        tlen = 0 ;
	        if ((ifname != NULL) && (ifname[0] != '\0'))
	        rs = bopen(&infile,ifname,"r",0666) ;

	        else
	        rs = bopen(&infile,BFILE_STDIN,"dr",0666) ;

	            if (rs >= 0) {

			hdr.logtag = logtag ;
			hdr.logpriority = logpriority ;
			hdr.svcspec = svcspec ;
			hdr.fromaddr = fromaddr ;
			rs = prochdr(pip,&u,&hdr,&con,&addrs) ;

/* copy over the message */

	            while ((rs >= 0) && (! f_signal) &&
	                ((rs = bread(&infile,buf,BUFLEN)) > 0)) {

	                len = rs ;
			rs = cm_write(&con,buf,len) ;
			if (rs < 0)
	                    break ;

			tlen += len ;

	            } /* end while */

	            bclose(&infile) ;

	        } /* end if (opened input) */

	    rs1 = cm_close(&con) ;
	        if ((rs >= 0) && (rs1 < 0))
	        rs = rs1 ;

	} /* end if (opened connection) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: recips=%d\n",c_recips) ;
#endif

	    if (f_addrs)
	    vecstr_finish(&addrs) ;

	    if (f_dialer)
	    dialer_free(&d) ;

	    if (f_systems)
	    systems_close(&sysdb) ;

baduser:
	    ids_release(&pip->id) ;

	    if (f_signal)
	    rs = SR_INTR ;

	    if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: recips=%u msglen=%u sent (%d)\n",
	    pip->progname,c_recips,tlen,rs) ;

	    if (pip->f.log)
	    logfile_printf(&pip->lh,"recips=%u msglen=%u sent (%d)\n",
	    c_recips,tlen,rs) ;

done:
	    if ((rs < 0) && (ex == EX_OK)) {

	    switch (rs) {

	    case SR_INVALID:
	        ex = EX_USAGE ;
	            if (! pip->f.quiet)
	            bprintf(pip->efp,"%s: invalid query (%d)\n",
	            pip->progname,rs) ;

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

/* close off and get out! */
ret2:
	if (pip->f.log)
	    logfile_close(&pip->lh) ;

retearly:
	    if (pip->open.akopts)
	    keyopt_finish(&akopts) ;

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

ret1:
	    proginfo_finish(pip) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	    return ex ;

	    /* bad argument stuff */
badarg:
	ex = EX_USAGE ;
	    bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	    usage(pip) ;

	    goto retearly ;

badinit1:
	    ex = EX_OSERR ;
	    if (! pip->f.quiet)
	    bprintf(pip->efp,
	    "%s: initialization problem (%d)\n",
	    pip->progname,rs) ;

	    goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [loghost] [<recipient(s)> ...]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s: \t[-oi] [-i] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


void int_all(signum)
int	signum ;
{


	f_signal = TRUE ;
	signal_num = signum ;

}


/* process program options */
static int getprogopts(pip,kop,setsp)
struct proginfo	*pip ;
KEYOPT		*kop ;
vecstr		*setsp ;
{
	KEYOPT_CUR	kcur ;

	    int	rs ;
	    int	i, oi, val, cl ;
	    int	nlen, klen, vlen ;

	    char	*kp, *vp ;
	    char	*cp ;


	    nlen = strlen(pip->searchname) ;

	    /* load up the environment options */

	if ((cp = getenv(VAROPTS)) != NULL)
	    keyopt_loads(kop,cp,-1) ;

	    /* system-wide options? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("getprogopts: scanning system options\n") ;
#endif

	    for (i = 0 ; vecstr_get(setsp,i,&cp) >= 0 ; i += 1) {

	    char	*cp2 ;


	        if (cp == NULL)
	        continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	        debugprintf("getprogopts: conf >%s<\n",cp) ;
#endif

	        if (! headkeymat(pip->searchname,cp,-1))
	        continue ;

	        /* we have one of ours, separate the keyname from the value */

	    cp += (nlen + 1) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	        debugprintf("getprogopts: key=%s\n",cp) ;
#endif

#ifdef	COMMENT
	        keyopt_loads(kop,cp,-1) ;
#else
	        kp = cp ;
	        vp = NULL ;
	        vlen = 0 ;
	        if ((cp = strchr(cp,'=')) != NULL) {

	        vp = cp + 1 ;
	            vlen = -1 ;
	    }

	    keyopt_loadvalue(kop,kp,vp,vlen) ;

#endif /* COMMENT */

	} /* end for (system options) */

/* process all of the options that we have so far */

	keyopt_curbegin(kop,&kcur) ;

	    while ((rs = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	    KEYOPT_CUR	vcur ;

	        int	f_value = FALSE ;


	        klen = rs ;

	        /* get the first (non-zero length) value for this key */

	    vlen = -1 ;
	        keyopt_curbegin(kop,&vcur) ;

	        /* use only the first of any option with the same key */

	    while ((rs = keyopt_enumvalues(kop,kp,&vcur,&vp)) >= 0) {

	        f_value = TRUE ;
	            vlen = rs ;
	            if (vlen > 0)
	            break ;

	    } /* end while */

	    keyopt_curend(kop,&vcur) ;

	        /* do we support this option? */

	    if ((oi = matostr(progopts,3,kp,klen)) >= 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("getprogopts: system valid option=%s(%d)\n",
	            progopts[oi],oi) ;
#endif

	            switch (oi) {

	        case progopt_cluster:
	            if ((vlen > 0) && (pip->cluster[0] == '\0'))
	                strwcpy(pip->cluster,vp,MIN(vlen, NODENAMELEN)) ;

	                break ;

	            case progopt_mailhost:
	            if ((vlen > 0) && (pip->mailhost[0] == '\0'))
	                strwcpy(pip->mailhost,vp,MIN(vlen, MAXHOSTNAMELEN)) ;

	                break ;

	            case progopt_mailsvc:
	            if ((vlen > 0) && (pip->mailsvc[0] == '\0'))
	                strwcpy(pip->mailsvc,vp,MIN(vlen, SVCNAMELEN)) ;

	                break ;

	            case progopt_loglen:
	            if ((vlen > 0) && (pip->loglen < 0)) {

	                if (cfdeci(vp,vlen,&val) >= 0)
	                    pip->loglen = val ;

	            }

	            break ;

	            case progopt_timeout:
	            if ((vlen > 0) && (pip->timeout < 0)) {

	                if (cfdecti(vp,vlen,&val) >= 0)
	                    pip->timeout = val ;

	            }

	            break ;

	            case progopt_pcspoll:
	            if ((vlen > 0) && (cfdeci(vp,vlen,&val) >= 0))
	                pip->f.pcspoll = (val > 0) ;

	                break ;

	        } /* end switch */

	    } /* end if (got a match) */

	} /* end while */

	keyopt_curend(kop,&kcur) ;

	    return OK ;
}
/* end subroutine (getprogopts) */


static int prochdr(pip,uip,hdrp,conp,addrp)
struct proginfo	*pip ;
USERINFO	*uip ;
struct hparams	*hdrp ;
CM		*conp ;
vecstr		*addrp ;
{

	BUFFER	b ;

	int	rs ;
	int	rs1 ;
	int	len ;
	int	i, j ;
	int	bl ;
	int	wlen = 0 ;

	const char	*ccp ;

	char	fromaddrbuf[BUFLEN + 1] ;
	char	tmpfname[BUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*bp ;
	char	*cp, *cp2 ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	debugprintf("main/prochdr: logtag=%s\n", hdrp->logtag) ;
	debugprintf("main/prochdr: logpriority=%s\n", hdrp->logpriority ) ;
	debugprintf("main/prochdr: svcspec=%s\n", hdrp->svcspec ) ;
	debugprintf("main/prochdr: fromaddr=%s\n", hdrp->fromaddr ) ;
	}
#endif /* CF_DEBUG */


	rs = buffer_start(&b,BUFLEN) ;
	if (rs < 0)
	    goto ret0 ;

	j = 0 ;
	for (i = 0 ; i < header_overlast ; i += 1) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("main: switch i=%s(%u)\n",
	            headers[i],i) ;
#endif /* CF_DEBUG */

	    switch (i) {

	    case header_msgid:
	        len = sncpy5(tmpfname,MAXPATHLEN,
	            headers[header_msgid],": ",
	            uip->logid,"@",uip->domainname) ;

	        break ;

	    case header_date:
	        len = sncpy3(tmpfname,MAXPATHLEN,
	            headers[header_date],": ",
	            timestr_hdate(pip->daytime,timebuf)) ;

	        break ;

	    case header_host:
	        len = sncpy5(tmpfname,MAXPATHLEN,
	            headers[header_host],": ",
	            uip->nodename,".",uip->domainname) ;

	        break ;

	    case header_logname:
	        len = snscs(tmpfname,MAXPATHLEN,
	            headers[header_logname], uip->username) ;

	        break ;

	    case header_from:
		ccp = hdrp->fromaddr ;
	        if ((ccp == NULL) || (ccp[0] == '\0')) {

	            ccp = fromaddrbuf ;
	            sncpy3(fromaddrbuf,MAILADDRLEN,
	                uip->username,"@",uip->domainname) ;

	        }

	        if ((ccp != NULL) && (ccp[0] != '\0')) {

	            len = snscs(tmpfname,MAXPATHLEN,
	                headers[header_from],ccp) ;

	        } else
	            len = -1 ;

	        break ;

	    case header_tag:
		ccp = hdrp->logtag ;
	        if ((ccp != NULL) && (ccp[0] != '\0')) {

	            len = snscs(tmpfname,MAXPATHLEN,
	                headers[header_tag],ccp) ;

	        } else
	            len = -1 ;

	        break ;

	    case header_priority:
		ccp = hdrp->logpriority ;
	        len = snscs(tmpfname,MAXPATHLEN,
	            headers[header_priority],ccp) ;

	        break ;

	    case header_to:
	        rs1 = vecstr_get(addrp,j,&cp) ;

	        if ((rs1 >= 0) && (cp != NULL)) {

	            cp2 = (j == 0) ? 
	                (char *) headers[header_to] :
	                " , " ;
	            len = snscs(tmpfname,MAXPATHLEN,
	                cp2,cp) ;

	            j += 1 ;
	            i -= 1 ;

	        } else
	            len = -1 ;

	        break ;

	    case header_xservice:
		ccp = hdrp->svcspec ;
	        len = snscs(tmpfname,MAXPATHLEN,
	            headers[header_xservice],ccp) ;

	        break ;

	    } /* end switch */

	    if ((rs >= 0) && (len > 0)) {

	        rs1 = buffer_strw(&b,tmpfname,len) ;

	        if (rs1 >= 0)
	            buffer_char(&b,'\n') ;

	    } /* end if */

	    if (rs < 0)
	        break ;

	} /* end for (preparing headers) */

	if (rs >= 0) {

/* write the end-of-header */

	    buffer_char(&b,'\n') ;

/* prepapre the write everything out */

	    bl = buffer_get(&b,&bp) ;
	    wlen = bl ;
	    if (bl > 0)
	        rs = cm_write(conp,bp,bl) ;

	} /* end if */

	buffer_finish(&b) ;

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (prochdr) */



