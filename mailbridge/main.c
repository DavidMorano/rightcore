/* main */

/* small (rather generic) front-end subroutine */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_SIGNAL	1		/* catch signals */
#define	CF_PCSPOLL	1		/* run 'pcspoll(1pcs)' */
#define	CF_SHUTDOWN	1		/* use 'shutdown(3socket)' */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	Synopsis:

	$ mailbridge [recipient(s) ...]


*************************************************************************/


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
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<schedvar.h>
#include	<userinfo.h>
#include	<pcsconf.h>
#include	<logfile.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"systems.h"
#include	"dialer.h"
#include	"cm.h"



/* local defines */

#define	NPARG		2	/* number of positional arguments */
#define	MAXARGINDEX	100

#define	NARGPRESENT	(MAXARGINDEX/8 + 1)

#define	HOSTPARTLEN	1024

#ifndef	TO_CONNECT
#define	TO_CONNECT	5
#endif

#ifndef	TO_READ
#define	TO_READ		10
#endif



/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			const char *,const char *) ;
extern int	pcspoll(const char *,const char *,PCSCONF *,VECSTR *) ;
extern int	pcstrustuser(const char *,const char *) ;
extern int	headkeymat(const char *,const char *,int) ;
extern int	isindomain(const char *,const char *) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	proginfo_rootname(struct proginfo *) ;
extern int	procflow(struct proginfo *,CM *,bfile *,int *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	makedate[] ;


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;
static int	procopts(struct proginfo *,KEYOPT *,vecstr *) ;

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
	"oi",
	"af",
	"if",
	"mh",
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
	argopt_oi,
	argopt_af,
	argopt_if,
	argopt_mh,
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
	"mh",
	"mailsvc",
	"ms",
	NULL
} ;

enum progopts {
	progopt_pcspoll,
	progopt_loglen,
	progopt_cluster,
	progopt_timeout,
	progopt_mailhost,
	progopt_mh,
	progopt_mailsvc,
	progopt_ms,
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


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	struct sigaction	sigs ;

	struct ustat	sb ;

	PCSCONF		p ;

	USERINFO	u ;

	SYSTEMS		sysdb ;

	DIALER		d ;

	CM		con ;

	KEYOPT		akopts ;

	sigset_t	signalmask ;

	bfile		errfile ;

	vecstr		sets, addrs ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, rs1 ;
	int	i, j ;
	int	sl, cl ;
	int	c_recips ;
	int	tlen = 0 ;
	int	timeout = -1 ;
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
	int	f ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[NARGPRESENT] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	char	pcsconfbuf[PCSCONF_LEN + 1] ;
	char	*pr = NULL ;
	char	*afname = NULL ;
	char	*ifname = NULL ;
	char	*logfname = NULL ;
	char	*fromaddr = NULL ;
	char	*sp, *cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* initialization */

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

/* do we have a keyword match or only key letters? */

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
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->helpfname = avp ;

	                    }

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

/* mailhost */
	                case argopt_mh:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }

	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        pip->mailhost = argp ;

	                    break ;

	                case argopt_oi:
	                    pip->f.optin = TRUE ;
	                    break ;

/* display the time this program was last "made" */
	                case argopt_makedate:
	                    f_makedate = TRUE ;
	                    break ;

	                default:
	                    rs = SR_INVALID ;
	                    bprintf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((uint) *akp) {

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

	                        if (argl)
	                            rs = keyopt_loads(&akopts,argp,argl) ;

	                        break ;

	                    case 'q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* timeout */
	                    case 't':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            rs = cfdeci(argp,argl,&timeout) ;

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

/* set program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    bprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	}

	if (f_usage)
	    usage(pip) ;

/* help */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage || f_makedate)
	    goto retearly ;


	ex = EX_OK ;

/* who are we? */

	proginfo_rootname(pip) ;

	ids_load(&pip->id) ;

	rs = userinfo(&u,userinfobuf,USERINFO_LEN,NULL) ;

	if (rs < 0) {
	    ex = EX_NOUSER ;
	    if (! pip->f.quiet)
	        bprintf(pip->efp,
	            "%s: could not get user information (%d)\n",
	            pip->progname,rs) ;
	    goto baduser ;
	}

	pip->pid = u.pid ;
	pip->username = u.username ;
	pip->groupname = u.groupname ;
	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->logid = u.logid ;

/* get the system PCS configuration information */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pcsconf()\n") ;
#endif

	rs = vecstr_start(&sets,10,0) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    if (! pip->f.quiet)
	        bprintf(pip->efp,
	            "%s: initialization 1 problem (%d)\n",
	            pip->progname,rs) ;
	    goto badinit ;
	}

	rs1 = pcsconf(pip->pr,NULL,&p,&sets,NULL,
	    pcsconfbuf,PCSCONF_LEN) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pcsconf() rs=%d\n",rs) ;
#endif

	f_pcsconf = (rs1 >= 0) ;

/* process any program options */

	rs = procopts(pip,&akopts,&sets) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: procopts() rs=%d\n",rs) ;
#endif

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badinit ;
	}

/* check arguments */

	if ((pip->mailsvc == NULL) || (pip->mailsvc[0] == '\0'))
	    pip->mailsvc = getenv(VARMAILSVC) ;

	if ((pip->mailsvc == NULL) || (pip->mailsvc[0] == '\0'))
	    pip->mailsvc = SVCSPEC_MAILBRIDGE ;

	if (pip->mailhost == NULL) 
	    pip->mailhost = getenv(VARMAILHOST) ;

	if (f_pcsconf && (pip->mailhost == NULL)) {

	    if ((p.mailhost != NULL) && (p.mailhost[0] != '\0')) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: PCS mailhost=%s\n",p.mailhost) ;
#endif

	        sp = p.mailhost ;
	        sl = MAXHOSTNAMELEN ;
	        if (isindomain(p.mailhost,pip->domainname)) {

	            sl = MAXHOSTNAMELEN ;
	            if ((cp = strchr(p.mailhost,'.')) != NULL)
	                sl = (cp - p.mailhost) ;

	        }

		rs = proginfo_setentry(pip,&pip->mailhost,sp,sl) ;

	    }
	}

	if (pip->mailhost == NULL)
	    pip->mailhost = MAILHOST ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badinit ;
	}

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: mailhost=%s\n",
		pip->progname,pip->mailhost) ;

#if	CF_PCSPOLL
	if ((rs >= 0) && pip->f.pcspoll) {

	    pip->pp = &p ;
	    pcspoll(pip->pr,NULL,&p,&sets) ;

	}
#endif /* CF_PCSPOLL */

/* we are delivering mail, but are we a trusted user? */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: f_trusted=%d\n",pip->f.trusted) ;
#endif

	vecstr_finish(&sets) ;

	rs1 = pcstrustuser(pip->pr,pip->username) ;
	pip->f.trusted = (rs1 > 0) ;

/* other initialization */

	if ((fromaddr == NULL) || (fromaddr[0] == '\0')) {
	    char	hostname[MAXHOSTNAMELEN + 1] ;

	    snsds(hostname,MAXHOSTNAMELEN,pip->nodename,pip->domainname) ;

	    sncpy3(tmpfname,MAXPATHLEN,hostname,"@",pip->username) ;
	    fromaddr = pip->username ;
	}

	proginfo_setentry(pip,&pip->fromaddr,fromaddr,-1) ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: fromaddr=%s\n",
	        pip->progname,pip->fromaddr) ;

/* prepapre for other initialization */

	pip->daytime = time(NULL) ;

/* do we have a log file? */

	if (logfname == NULL)
	    logfname = LOGFNAME ;

	if (logfname[0] != '/') {
	    mkpath2(tmpfname,pip->pr,logfname) ;
	    logfname = tmpfname ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: logfile=%s\n", logfname) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: logfile=%s\n",
	        pip->progname,logfname) ;

	rs1 = logfile_open(&pip->lh,logfname,0,0666,pip->logid) ;

	if (rs1 >= 0) {

	    pip->open.log = TRUE ;
	    if (pip->daytime == 0)
	        pip->daytime = time(NULL) ;

	    logfile_userinfo(&pip->lh,&u,
	        pip->daytime,pip->progname,pip->version) ;

	    logfile_printf(&pip->lh,"mailhost=%s\n",pip->mailhost) ;

	} /* end if (opened a log) */

/* loop through the arguments */

	c_recips = 0 ;
	rs = vecstr_start(&addrs,20,0) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    if (! pip->f.quiet)
	        bprintf(pip->efp,
	            "%s: initialization 2 problem (%d)\n",
	            pip->progname,rs) ;
	    goto badaddrinit ;
	}

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs1 = vecstr_find(&addrs,cp) ;

	    if (rs1 == SR_NOTFOUND) {

	        rs = vecstr_add(&addrs,cp,-1) ;
	        c_recips += 1 ;
	        if (rs < 0)
	            break ;

	    }

	} /* end for */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {

	    bfile	argfile ;


	    if (strcmp(afname,"-") != 0)
	        rs = bopen(&argfile,afname,"r",0666) ;

	    else
	        rs = bopen(&argfile,BFILE_STDIN,"dr",0666) ;

	    if (rs >= 0) {

	        int	len ;

	        char	linebuf[LINEBUFLEN + 1] ;


	        while ((rs = breadline(&argfile,linebuf,LINEBUFLEN)) > 0) {

	            len = rs ;
	            if (linebuf[len - 1] == '\n')
	                len -= 1 ;

	            linebuf[len] = '\0' ;

#if	F_ARGSHRINK
	            cp = strshrink(linebuf) ;
#else
	            cp = linebuf ;
#endif

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            pan += 1 ;
	            rs1 = vecstr_find(&addrs,cp) ;

	            if (rs1 == SR_NOTFOUND) {

	                rs = vecstr_add(&addrs,cp,-1) ;
	                c_recips += 1 ;
	                if (rs < 0) {
	                    bprintf(pip->efp,
	                        "%s: processing error (%d) in file=%s\n",
	                        pip->progname,rs,cp) ;
	                    break ;
	                }

	            }

	        } /* end while (reading lines) */

	        bclose(&argfile) ;

	    } else {

	        if (! pip->f.quiet) {

	            bprintf(pip->efp,
	                "%s: could not open the argument list file (%d)\n",
	                pip->progname,rs) ;

	            bprintf(pip->efp,"%s: \targfile=%s\n",
	                pip->progname,afname) ;

	        }

	    }

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pan == 0)) {

	    cp = tmpfname ;
	    cl = sncpy3(tmpfname,MAXPATHLEN,pip->username,"@",pip->domainname) ;

	    if (cl > 0) {

	        pan += 1 ;
	        rs1 = vecstr_find(&addrs,cp) ;

	        if (rs1 == SR_NOTFOUND) {

	            rs = vecstr_add(&addrs,cp,cl) ;
	            if (rs >= 0)
	                c_recips += 1 ;

	        }

	    }

	} /* end if (default revipient) */

/* log recipients? */

	if ((rs >= 0) && (pip->open.log || (pip->debuglevel > 0))) {

	    for (i = 0 ; vecstr_get(&addrs,i,&cp) >= 0 ; i += 1) {

	        if (cp == NULL) continue ;

	        if (pip->open.log)
	            logfile_printf(&pip->lh,"  to=%s\n",cp) ;

	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: to=%s\n",pip->progname,cp) ;

	    }

	} /* end if (logging recipients) */

/* do the main thing */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: performing main processing\n") ;
#endif

	pip->signal_num = -1 ;

	sigemptyset(&signalmask) ;

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


	    if ((rs = schedvar_start(&sf)) >= 0) {

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

#if	CF_DEBUG
	                if (DEBUGLEVEL(5)) {
	                    debugprintf("main: systems_fileadd() rs=%d\n",rs) ;
	                    debugprintf("main: fname=%s\n",tmpfname) ;
	                }
#endif

	            }

	            if (rs == SR_EXIST)
	                rs = SR_OK ;

	            if (rs < 0)
	                break ;

	        } /* end for */

	    } /* end for */

	    schedvar_finish(&sf) ;

	    } /* end if */

	} /* end block (loading 'systems' files) */

#if	CF_DEBUG 
	if (DEBUGLEVEL(6)) {
	    SYSTEMS_CUR	cur ;
	    SYSTEMS_ENT	*sep ;
	    debugprintf("main: sysnames: \n") ;
	    systems_curbegin(&sysdb,&cur) ;
	    while (systems_enum(&sysdb,&cur,&sep) >= 0)
	        debugprintf("main: sysname=%s\n",sep->sysname) ;
	    systems_curend(&sysdb,&cur) ;
	}
#endif /* CF_DEBUG */

/* can we initialize the DIALER subsystem? */

	if (rs >= 0) {

	    rs = dialer_init(&d,pip->pr,NULL,NULL) ;
	    f_dialer = (rs >= 0) ;

	}

/* try to open a connection (with connection-manager) */

	if (rs >= 0) {

	    CM_ARGS	ca ;

	    const char	**av ;


	    memset(&ca,0,sizeof(CM_ARGS)) ;

	ca.pr = pip->pr ;
	ca.prn = pip->rootname ;
	ca.searchname = pip->searchname ;
	ca.nodename = pip->nodename ;
	ca.domainname = pip->domainname ;
	ca.username = pip->username ;

	    ca.sp = &sysdb ;
	    ca.dp = &d ;
	    ca.timeout = TO_CONNECT ;
	    ca.options = (DIALER_MHALFOUT | DIALER_MCO | DIALER_MARGS) ;

	    vecstr_getvec(&addrs,&av) ;

	    rs = cm_open(&con,&ca,pip->mailhost,pip->mailsvc,av) ;
	    f_cm = (rs >= 0) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("main: cm_open() rs=%d\n",rs) ;
#endif /* CF_DEBUG */

	    if ((pip->debuglevel > 0) && (rs < 0))
		bprintf(pip->efp,"%s: open failed (%d)\n",
		    pip->progname,rs) ;

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

	        rs = procflow(pip,&con,&infile,&f_signal) ;
	        tlen += rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("main: procflow() rs=%d\n",rs) ;
#endif /* CF_DEBUG */

	        bclose(&infile) ;

	    } /* end if (opened input) */

	    rs1 = cm_close(&con) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end if (opened connection) */

	vecstr_finish(&addrs) ;

	if (f_dialer)
	    dialer_free(&d) ;

	if (f_systems)
	    systems_close(&sysdb) ;

	if (f_signal)
	    rs = SR_INTR ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: recips=%u msglen=%u sent (%d)\n",
	        pip->progname,c_recips,tlen,rs) ;

	if (pip->open.log) {

	    if (rs >= 0)
	        logfile_printf(&pip->lh,"recips=%u msglen=%u sent (%d)\n",
	            c_recips,tlen,rs) ;

	    else
	        logfile_printf(&pip->lh,"recips=%u operation failed (%d)\n",
	            c_recips,rs) ;

	} /* end if (logging) */

done:
	if ((rs < 0) && (ex == EX_OK)) {

	    switch (rs) {

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

	vecstr_finish(&addrs) ;

badaddrinit:
	if (pip->open.log)
	    logfile_close(&pip->lh) ;

baduser:
badinit:
	ids_release(&pip->id) ;

retearly:
ret3:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

ret2:
	if (pip->open.akopts)
	    keyopt_finish(&akopts) ;

	if (pip->efp != NULL)
	    bclose(pip->efp) ;

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

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [<recipient(s)> ...]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s: \t[-oi] [-i] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	return rs ;
}
/* end subroutine (usage) */


void int_all(signum)
int	signum ;
{


	f_signal = TRUE ;
	signal_num = signum ;

}


/* process program options */
static int procopts(pip,kop,setsp)
struct proginfo	*pip ;
KEYOPT		*kop ;
vecstr		*setsp ;
{
	KEYOPT_CUR	kcur ;

	int	rs = SR_OK ;
	int	i, oi, val ;
	int	nlen ;
	int	kl, vl ;

	const char	*kp, *vp ;

	char	*cp ;


	nlen = strlen(pip->searchname) ;

/* load up the environment options */

	if ((cp = getenv(VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs < 0)
	    goto ret0 ;

/* system-wide options? */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procopts: scanning system options\n") ;
#endif

	for (i = 0 ; vecstr_get(setsp,i,&cp) >= 0 ; i += 1) {

	    if (cp == NULL)
	        continue ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	        debugprintf("procopts: conf >%s<\n",cp) ;
#endif

	    if (! headkeymat(pip->searchname,cp,-1))
	        continue ;

/* we have one of ours, separate the keyname from the value */

	    cp += (nlen + 1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	        debugprintf("procopts: key=%s\n",cp) ;
#endif

#ifdef	COMMENT
	    keyopt_loads(kop,cp,-1) ;
#else
	    kp = cp ;
	    vp = NULL ;
	    vl = 0 ;
	    if ((cp = strchr(cp,'=')) != NULL) {
	        vp = cp + 1 ;
	        vl = -1 ;
	    }

	    keyopt_loadvalue(kop,kp,vp,vl) ;

#endif /* COMMENT */

	} /* end for (system options) */

/* process all of the options that we have so far */

	keyopt_curbegin(kop,&kcur) ;

	while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	    KEYOPT_CUR	vcur ;


/* get the first (non-zero length) value for this key */

	    vl = -1 ;
	    keyopt_curbegin(kop,&vcur) ;

/* use only the first of any option with the same key */

	    while ((vl = keyopt_enumvalues(kop,kp,&vcur,&vp)) >= 0) {

	        if (vl > 0)
	            break ;

	    } /* end while */

	    keyopt_curend(kop,&vcur) ;

/* do we support this option? */

	    if ((oi = matostr(progopts,2,kp,kl)) >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	            debugprintf("procopts: system valid option=%s(%d)\n",
	                progopts[oi],oi) ;
#endif

	        switch (oi) {

	        case progopt_cluster:
	            if ((vl > 0) && (pip->cluster == NULL))
	                rs = proginfo_setentry(pip,&pip->cluster,vp,vl) ;
	                strwcpy(pip->cluster,vp,MIN(vl, NODENAMELEN)) ;

	            break ;

	        case progopt_mailhost:
	        case progopt_mh:
	            if ((vl > 0) && (pip->mailhost == NULL))
	                rs = proginfo_setentry(pip,&pip->mailhost,vp,vl) ;

	            break ;

	        case progopt_mailsvc:
	        case progopt_ms:
	            if ((vl > 0) && (pip->mailsvc == NULL))
	                rs = proginfo_setentry(pip,&pip->mailsvc,vp,vl) ;

	            break ;

	        case progopt_loglen:
	            if ((vl > 0) && (pip->loglen < 0)) {

	                if (cfdeci(vp,vl,&val) >= 0)
	                    pip->loglen = val ;

	            }

	            break ;

	        case progopt_timeout:
	            if ((vl > 0) && (pip->timeout < 0)) {

	                if (cfdeci(vp,vl,&val) >= 0)
	                    pip->timeout = val ;

	            }

	            break ;

	        case progopt_pcspoll:
	            if (vl > 0) {
			rs = optbool(vp,vl) :
	                pip->f.pcspoll = (rs > 0) ;
		    }
	            break ;

	        } /* end switch */

	    } /* end if (got a match) */

	    if (rs < 0) break ;
	} /* end while */

	keyopt_curend(kop,&kcur) ;

ret0:
	return rs ;
}
/* end subroutine (procopts) */



