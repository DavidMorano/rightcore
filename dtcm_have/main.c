/* main */

/* small (rather generic) front-end subroutine */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */
#define	CF_SIGNAL	1
#define	CF_SHUTDOWN	0


/* revision history:

	= 1988-02-01, David A­D­ Morano
	This subroutine was originally written.

	= 1988-02-01, David A­D­ Morano
        This subroutine was modified to not write out anything to standard
        output if the access time of the associated terminal has not been
        changed in 10 minutes.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ dtcm_have [-c calendar]


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
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
#include	<userinfo.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<schedvar.h>
#include	<serialbuf.h>
#include	<hostaddr.h>
#include	<sockaddress.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"systems.h"
#include	"dialer.h"
#include	"cm.h"
#include	"havemsg.h"
#include	"address.h"
#include	"recipient.h"
#include	"nifinfo.h"


/* local defines */

#define	NPARG		2	/* number of positional arguments */
#define	MAXARGINDEX	100

#define	NARGPRESENT	(MAXARGINDEX/8 + 1)

#ifndef	LOCALHOST
#define	LOCALHOST	"localhost"
#endif

#define	HOSTPARTLEN	1024

#ifndef	ADDRBUFLEN
#define	ADDRBUFLEN	128
#endif

#ifndef	TO_CONNECT
#define	TO_CONNECT	20
#endif

#ifndef	TO_READ
#define	TO_READ		5
#endif

#define	RETRIES		5


/* external subroutines */

extern int	snscs(char *,int,const char *,const char *) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,
			const char *,const char *) ;
extern int	sncpy5(char *,int,const char *,const char *,
			const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	issamehostname(const char *,const char *,const char *) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_hdate(time_t,char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	makedate[] ;


/* local structures */

struct conninfo {
	uint	co : 1 ;		/* connection oriented */
} ;


/* forward references */

static int	usage(PROGINFO *) ;
static int	getprogopts(PROGINFO *,KEYOPT *,vecstr *) ;
static int	prochost(PROGINFO *, RECIPIENT *, SYSTEMS *,DIALER *,
			const char *,const char *) ;
static int	process(PROGINFO *,CM *,struct conninfo *,
			const char *) ;

static void	int_all() ;


/* module-scope (as bad as global?) variables */

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
	"svc",
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
	argopt_svc,
	argopt_overlast
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
	{ SR_ACCESS, EX_ACCESS },
	{ SR_REMOTE, EX_FORWARDED },
	{ SR_NOSPC, EX_NOSPACE },
	{ SR_INTR, EX_INTR },
	{ 0, 0 }
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
	header_overlast
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	struct sigaction	sigs ;
	PROGINFO	g, *pip = &g ;
	struct conninfo	cf ;
	struct ustat	sb ;
	USERINFO	u ;
	RECIPIENT	recips ;
	SYSTEMS		sysdb ;
	DIALER		d ;
	KEYOPT		akopts ;
	sigset_t	signalmask ;
	bfile		errfile ;
	vecstr		sets, addrs ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		argvalue = -1 ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		v ;
	int		len, n, i, j ;
	int		sl, cl ;
	int		tlen = 0 ;
	int		timeout = -1 ;
	int		c_recips = 0 ;
	int		fd_debug = -1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_makedate = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_systems = FALSE ;
	int		f_dialer = FALSE ;
	int		f_addrs = FALSE ;
	int		f_recips = FALSE ;
	int		f_have = TRUE ;
	int		f ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*pr = NULL ;
	cchar		*afname = NULL ;
	cchar		*ifname = NULL ;
	cchar		*logfname = NULL ;
	cchar		*hostname ;
	cchar		*loghost = NULL ;
	cchar		*svcspec = NULL ;
	cchar		*fromaddr = NULL ;
	cchar		*logpriority = NULL ;
	cchar		*logtag = NULL ;
	cchar		*tospec = NULL ;
	cchar		*calendar = NULL ;
	cchar		*sp, *cp, *cp2 ;
	char		argpresent[NARGPRESENT] ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		buf[BUFLEN + 1] ;
	char		userinfobuf[USERINFO_LEN + 1] ;
	char		hostpart[MAILADDRLEN + 1] ;
	char		localpart[MAILADDRLEN + 1] ;
	char		timebuf[TIMEBUFLEN + 1] ;


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
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

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
	                        if (avl) {
	                                    rs = optvalue(avp,avl) ;
	                                    pip->debuglevel = rs ;
	                        }
	                    }
	                    break ;

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

/* service specification */
	                case argopt_svc:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            svcspec = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            svcspec = argp ;

	                    }

	                    break ;

/* display the time this program was last "made" */
	                case argopt_makedate:
	                    f_makedate = TRUE ;
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
	                                rs = optbalue(avp,avl) ;
	                                pip->debuglevel = rs ;
	                            }
	                        }
	                        break ;

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
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

/* calendar */
	                    case 'c':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            calendar = argp ;

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
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->verboselevel = rs ;
	                            }
	                        }
	                        break ;

/* fall through from above */
	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        bprintf(pip->efp,
	                            "%s : unknown option - %c\n",
	                            pip->progname,*aop) ;

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
	    debugprintf("main: debuglevel=%u\n",
	        pip->debuglevel) ;
#endif

/* continue w/ the trivia argument processing stuff */

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_makedate)
	    bprintf(pip->efp,"%s: built %s\n",
	        pip->progname,makedate) ;

	if (f_usage)
	    usage(pip) ;

	if (f_version + f_makedate)
	    goto done ;

	rs = proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

/* help */

	if (f_help) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: printhelp() pr=%s sn=%s\n",
	            pip->pr,pip->searchname) ;
#endif

	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	    goto retearly ;

	} /* end if */

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	} /* end if */


	ex = EX_OK ;

/* check arguments */

	if ((svcspec == NULL) || (svcspec[0] == '\0')) {

	    if ((cp = getenv(VARSVC)) != NULL)
	        svcspec = cp ;

	}

	if ((svcspec == NULL) || (svcspec[0] == '\0'))
	    svcspec = SVCSPEC_DTCMHAVE ;

/* connection timeout */

	if ((tospec != NULL) && (tospec[0] != '\0')) {

	    rs = cfdecti(tospec,-1,&timeout) ;
	    if (rs < 0)
	        goto badarg ;

	}

	if ((timeout < 0) && (argvalue >= 0))
	    timeout = argvalue ;

	if (timeout < 0)
	    timeout = TO_CONNECT ;

	pip->timeout = timeout ;

	if ((logpriority == NULL) || (logpriority[0] == '\0'))
	    logpriority = LOGPRIORITY ;

	if ((logtag == NULL) || (logtag[0] == '\0'))
	    logtag = LOGTAG ;

/* who are we? */

	rs = userinfo(&u,userinfobuf,USERINFO_LEN,NULL) ;
	if (rs < 0) {
	    ex = EX_NOUSER ;
	    goto baduser ;
	}

	pip->pid = u.pid ;
	pip->domainname = u.domainname ;
	pip->nodename = u.nodename ;
	pip->username = u.username ;

	pip->hostid = (uint) gethostid() ;

/* option initialization */

	getprogopts(pip,&akopts,NULL) ;

/* other initialization */

	if (pip->mailhost[0] == '\0')
	    sncpy1(pip->mailhost, MAXHOSTNAMELEN, MAILHOST) ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: mailhost=%s\n",
	        pip->progname,pip->mailhost) ;

/* prepapre for other initialization */

	pip->daytime = time(NULL) ;

	ids_load(&pip->id) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    NIFINFO		info ;
	    NIFINFO_ENT	*nip ;
	    ULONG	*ulwp ;
	    nifinfo_start(&info,NULL) ;
	    for (i = 0 ; nifinfo_get(&info,i,&nip) >= 0 ; i += 1) {
	        ulwp = (ULONG *) &nip->addr ;
	        debugprintf("main: IF af=%u\n", nip->af) ;
	        debugprintf("main: IF i=%u flags=%016llx dev=%s addr=%016llx\n",
	            nip->index,nip->flags,nip->device,*ulwp) ;
	    } /* end for */
	    nifinfo_finish(&info) ;
	} /* end block */
#endif /* CF_DEBUG */

	nifinfo_start(&pip->ifs,NULL) ;

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

	rs = logfile_open(&pip->lh,logfname,0,0666,u.logid) ;

	if (rs >= 0) {

	    pip->f.log = TRUE ;

	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

	    else if (u.mailname != NULL)
	        sprintf(buf,"(%s)",u.mailname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 2)
	        debugprintf("main: about to do 'logfile_printf'\n") ;
#endif

	    logfile_printf(&pip->lh,"%s %-14s %s/%s\n",
	        timestr_log(pip->daytime,timebuf),
	        pip->progname,
	        VERSION,(u.f.sysv_ct ? "SYSV" : "BSD")) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s) d=%s\n",
	        (u.f.sysv_rt ? "SYSV" : "BSD"),
	        u.sysname,u.release,
	        u.domainname) ;

	    logfile_printf(&pip->lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;

	    logfile_printf("mailhost=%s\n",pip->mailhost) ;

	} /* end if (opened a log) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking for positional arguments\n") ;
#endif

/* do the main thing */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: performing main processing\n") ;
#endif

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

	ex = EX_OSERR ;
	rs = vecstr_start(&addrs,20,0) ;
	f_addrs = (rs >= 0) ;
	if (rs < 0)
	     goto badaddrinit ;

/* do we have a SYSTEMS file? */

	rs1 = systems_open(&sysdb,NULL) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main: systems_open() rs=%d\n",rs1) ;
#endif

	f_systems = (rs1 >= 0) ;
	if (f_systems) {

	    SCHEDVAR	sf ;

	    char	sysfname[MAXPATHLEN + 1] ;


	    if ((rs = schedvar_start(&sf)) >= 0) {

	    schedvar_add(&sf,"p",pip->pr,-1) ;

	    schedvar_add(&sf,"n",pip->searchname,-1) ;

	    for (j = 0 ; j < 2 ; j += 1) {

	        if (j == 0) {
	            schedvar_add(&sf,"f",SYSFNAME1,-1) ;
	        } else
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
	                if (DEBUGLEVEL(5))
	                    debugprintf("main: systems_fileadd() "
				"rs=%d fname=%s\n",
	                        rs,tmpfname) ;
#endif

	            }

	            if (rs == SR_EXIST)
	                rs = SR_OK ;

	            if (rs < 0)
	                break ;

		if (rs >= 0) {

			pip->f.systems += 1 ;
			if (pip->f.systems == 0)
				pip->f.systems = 1 ;

		}

	        } /* end for (inner) */

	    } /* end for (outer) */

	    schedvar_finish(&sf) ;

	    } /* end if */

	    if (rs == SR_EXIST)
	        rs = SR_OK ;

	} /* end block (loading 'systems' files) */

#if	CF_DEBUG 
	if (f_systems && DEBUGLEVEL(6)) {
	    SYSTEMS_CUR	cur ;
	    SYSTEMS_ENT	*sep ;

	    debugprintf("main: systems=%u sysnames: \n",pip->f.systems) ;

	    systems_curbegin(&sysdb,&cur) ;

	    while (systems_enum(&sysdb,&cur,&sep) >= 0) {

	        debugprintf("main: sysname=%s\n",sep->sysname) ;

	    }

	    systems_curend(&sysdb,&cur) ;

	}
#endif /* CF_DEBUG */

/* can we initialize the DIALER subsystem? */

	if (f_systems && (rs >= 0)) {
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

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	                rs = vecstr_add(&addrs,cp,-1) ;

	                if (rs >= 0)
	                    c_recips += 1 ;

	        } /* end for */

	    if ((rs >= 0) && (calendar != NULL)) {

	        pan += 1 ;
	        rs = vecstr_add(&addrs,calendar,-1) ;

	        if (rs >= 0)
	            c_recips += 1 ;

	    }

	    if ((rs >= 0) && (pan == 0)) {

	        if (u.domainname[0] != '\0') {
	            cl = sncpy3(tmpfname,MAXPATHLEN,
	                u.username,"@",u.domainname) ;
	        } else
	            cl = sncpy3(tmpfname,MAXPATHLEN,
	                u.username,"@",u.nodename) ;

	        if (cl > 0) {

	            pan += 1 ;
	            rs = vecstr_add(&addrs,tmpfname,cl) ;

	            if (rs >= 0)
	                c_recips += 1 ;

	        }

	    } /* end if (default revipient) */

	} /* end if (getting recipients) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: recipients=%u\n",c_recips) ;
#endif

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
	    rs = recipient_start(&recips,20) ;
	    f_recips = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	type ;

	    for (i = 0 ; vecstr_get(&addrs,i,&sp) >= 0 ; i += 1) {
	        if (sp == NULL) continue ;

	        type = addressparse(sp,-1,hostpart,localpart) ;

	        if (type == ADDRESSTYPE_LOCAL)
	            sncpy1(hostpart,MAILADDRLEN,u.nodename) ;

	        rs = recipient_add(&recips,hostpart,localpart,type) ;

	        if (rs < 0)
	            break ;

	    } /* end for */

	} /* end if */

	if (rs >= 0) {
	    RECIPIENT_HCUR	hcur ;

	    recipient_hcurbegin(&recips,&hcur) ;

	    while (recipient_enumhost(&recips,&hcur,&cp) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: prochost() hostname=%s\n",cp) ;
#endif

	        rs = prochost(pip,&recips,&sysdb,&d,cp,svcspec) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: prochost() rs=%d\n",rs) ;
#endif

	        if (rs < 0)
	            break ;

	        f_have = f_have && (rs > 0) ;

	    } /* end while */

	    recipient_hcurend(&recips,&hcur) ;

	} /* end if */

	if (f_recips)
	    recipient_finish(&recips) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: recips=%d\n",c_recips) ;
#endif

	if (f_dialer)
	    dialer_free(&d) ;

	if (f_systems)
	    systems_close(&sysdb) ;

	nifinfo_finish(&pip->ifs) ;

	ids_release(&pip->id) ;

	if (f_signal)
	    rs = SR_INTR ;

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

	} else {

	    if (pip->debuglevel > 0)
		bprintf(pip->efp,"%s: have=%u\n",
			pip->progname,f_have) ;

	    ex = (f_have) ? EX_OK : EX_NOUSER ;

	} /* end if */

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
			pip->progname,ex,rs) ;

	if (f_addrs)
	    vecstr_finish(&addrs) ;

	if (pip->f.log)
	    logfile_close(&pip->lh) ;

badaddrinit:
baduser:
retearly:
ret3:
	if (pip->open.akopts)
	    keyopt_finish(&akopts) ;

ret2:
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
PROGINFO	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [recipient(s) ...]\n",
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
PROGINFO	*pip ;
KEYOPT		*kop ;
vecstr		*setsp ;
{
	KEYOPT_CUR	kcur ;
	int		rs ;
	int		i, oi, val, cl ;
	int		nlen, klen, vlen ;
	const char	*kp, *vp ;
	cchar		*cp ;

	nlen = strlen(pip->searchname) ;

/* load up the environment options */

	if ((cp = getenv(VAROPTS)) != NULL)
	    keyopt_loads(kop,cp,-1) ;

/* system-wide options? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("getprogopts: scanning system options\n") ;
#endif

	if (setsp != NULL) {

	    for (i = 0 ; vecstr_get(setsp,i,&cp) >= 0 ; i += 1) {
	        char	*cp2 ;
	        if (cp == NULL) continue ;

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

	} /* end if (have set-options) */

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


static int prochost(pip,recipp,sysdbp,dp,hostname,svcspec)
PROGINFO	*pip ;
RECIPIENT	*recipp ;
SYSTEMS		*sysdbp ;
DIALER		*dp ;
const char	hostname[] ;
const char	svcspec[] ;
{
	struct conninfo	cf ;
	struct addrinfo	hint, *aip ;
	HOSTADDR	ha ;
	HOSTADDR_CUR	cur ;
	SOCKADDRESS	*sap ;
	CM		con ;
	CM_ARGS		ca ;
	RECIPIENT_VCUR	lcur ;
	RECIPIENT_VALUE	*rvp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c ;
	int		sl, cl ;
	int		af ;
	int		f_local = FALSE ;
	int		f = FALSE ;
	cchar		*dargs[2] ;
	cchar		*sp ;
	char		calendar[MAILADDRLEN + 1] ;

/* set the restrictions */

	memset(&hint,0,sizeof(struct addrinfo)) ;
	hint.ai_flags = AI_CANONNAME ;

/* do the lookup */

	rs1 = SR_NOTFOUND ;
	if (strcmp(hostname,pip->nodename) == 0)
	    rs1 = SR_OK ;

	if ((rs1 < 0) &&
	    issamehostname(hostname,pip->nodename,pip->domainname))
	    rs1 = SR_OK ;

	if ((rs1 < 0) &&
	    (strcmp(hostname,LOCALHOST) == 0))
	    rs1 = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("prochost: initial rs1=%d\n",rs1) ;
#endif

	if (rs1 < 0) {

	    if ((rs = hostaddr_start(&ha,hostname,NULL,&hint)) >= 0) {
	        int	alen ;
	        char	a[ADDRBUFLEN + 1] ;

	        hostaddr_curbegin(&ha,&cur) ;

	        c = 0 ;
	        while (hostaddr_enum(&ha,&cur,&aip) >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("prochost: hostaddr_enum() pf=%u \n",
	                    aip->ai_family) ;
#endif

	            if (c == 0) {

	                f = issamehostname(aip->ai_canonname,
	                    pip->nodename,pip->domainname) ;

	                if (f) {
	                    rs1 = SR_OK ;
	                    break ;
	                }

	            } /* end if (first entry) */

	            sap = (SOCKADDRESS *) aip->ai_addr ;
	            af = sockaddress_getaf(sap) ;

	            if ((alen = sockaddress_getaddr(sap,a,ADDRBUFLEN) > 0) {
	                rs1 = nifinfo_match(&pip->ifs,af,a,alen) ;
	                if (rs1 >= 0) break ;
	            }

	            c += 1 ;

	        } /* end while */

	        hostaddr_curend(&ha,&cur) ;

	        hostaddr_finish(&ha) ;
	    } /* end if (got something) */

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("prochost: mid rs=%d rs1=%d\n",rs,rs1) ;
#endif

	f = TRUE ;
	if ((rs1 >= 0) && (rs >= 0)) {

	    char	tmpfname[MAXPATHLEN + 1] ;
	    char	fname[MAXNAMELEN + 1] ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("prochost: LOCAL\n") ;
#endif /* CF_DEBUG */

	    recipient_vcurbegin(recipp,&lcur) ;

	    while (TRUE) {

	        sl = recipient_fetchvalue(recipp,hostname,&lcur,&rvp) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("prochost: recipient_fetchvalue() rs=%d\n",sl) ;
#endif /* CF_DEBUG */

	        if (sl < 0)
	            break ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("prochost: localpart=%s\n",rvp->localpart) ;
#endif /* CF_DEBUG */

	        snsds(fname,MAXNAMELEN,"callog",rvp->localpart) ;

	        mkpath2(tmpfname,CALDNAME,fname) ;

	        rs1 = u_access(tmpfname,R_OK) ;

	        f = f && (rs1 >= 0) ;
	        if (! f)
	            break ;

	    } /* end while */

	    recipient_vcurend(recipp,&lcur) ;

	} else if (pip->f.systems && (rs >= 0)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("prochost: REMOTE\n") ;
#endif /* CF_DEBUG */

	    memset(&ca,0,sizeof(CM_ARGS)) ;

	    ca.searchname = pip->searchname ;
	    ca.pr = pip->pr ;
	    ca.searchname = pip->nodename ;
	    ca.domainname = pip->domainname ;
	    ca.username = pip->username ;
	    ca.sp = sysdbp ;
	    ca.dp = dp ;
	    ca.timeout = pip->timeout ;
	    ca.options = (DIALER_MFULL ) ;

	    dargs[0] = NULL ;

	    rs = cm_open(&con,&ca,hostname,svcspec,NULL) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("prochost: cm_open() rs=%d\n",rs) ;
#endif /* CF_DEBUG */

/* if we have an open connection => do the deed */

	    if (rs >= 0) {

	        CM_INFO		ci ;


	        rs1 = cm_info(&con,&ci) ;

	        if (rs1 >= 0) {
	            cf.co = (ci.dflags & DIALER_MCO) ? 1 : 0 ;
	        }

	        recipient_vcurbegin(recipp,&lcur) ;

	        while (TRUE) {

	            sl = recipient_fetchvalue(recipp,hostname,&lcur,&rvp) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("prochost: recipient_fetchvalue() rs=%d\n",
				sl) ;
#endif /* CF_DEBUG */

	            if (sl < 0)
	                break ;

	            cl = addressarpa(calendar,MAILADDRLEN,
	                hostname,rvp->localpart, ADDRESSTYPE_LOCAL) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("prochost: process() calendar=%s\n",
				calendar) ;
#endif /* CF_DEBUG */

	            rs = process(pip,&con,&cf,calendar) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("prochost: process() rs=%d\n",rs) ;
#endif /* CF_DEBUG */

	            if (rs < 0)
	                break ;

	            f = f && (rs > 0) ;
	            if (! f)
	                break ;

	        } /* end while */

	        recipient_vcurend(recipp,&lcur) ;

	        rs1 = cm_close(&con) ;

	        if ((rs >= 0) && (rs1 < 0))
	            rs = rs1 ;

	    } /* end if (opened connection) */

	} else if (rs >= 0)
	    rs = SR_NOTFOUND ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("prochost: ret rs=%d f=%u\n",rs,f) ;
#endif /* CF_DEBUG */

ret0:
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (prochost) */


static int process(pip,cop,cfp,calp)
PROGINFO	*pip ;
CM		*cop ;
struct conninfo	*cfp ;
const char	*calp ;
{
	struct havemsg_request	m0 ;
	struct havemsg_report	m1 ;
	int		rs = SR_OK ;
	int		n, i ;
	int		mbl ;
	int		to = TO_READ ;
	int		f = FALSE ;
	char		msgbuf[MSGBUFLEN + 1] ;

	if ((calp == 0) || (calp[0] == '\0'))
	    return SR_INVALID ;

	n = (cfp->co) ? 1 : RETRIES ;
	memset(&m0,0,sizeof(struct havemsg_request)) ;

	m0.type = havemsgtype_request ;
	m0.tag = pip->hostid ;
	sncpy1(m0.calendar,HAVEMSG_LCALENDAR,calp) ;

	for (i = 0 ; i < n ; i += 1) {

	    m0.seq = i ;
	    m0.timestamp = time(NULL) ;

	    rs = havemsg_request(msgbuf,MSGBUFLEN,0,&m0) ;
	    mbl = rs ;
	    if (rs >= 0) {

	        rs = cm_write(cop,msgbuf,mbl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("process: cm_write() rs=%d\n",rs) ;
#endif

	    }

	    if (rs >= 0) {

	        rs = cm_reade(cop,msgbuf,MSGBUFLEN,to,FM_TIMED) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("process: cm_reade() rs=%d\n",rs) ;
#endif

	        if (rs > 0) {

	            rs = havemsg_report(msgbuf,MSGBUFLEN,1,&m1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("process: havemsg_report() rs=%d\n",rs) ;
#endif

	            if (rs >= 0) {
	                f = (m1.rc == 0) ;
	                break ;
	            }

	        }
	    }

	} /* end for */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (process) */


