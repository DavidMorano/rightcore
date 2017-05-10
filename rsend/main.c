/* main */

/* small (rather generic) front-end subroutine */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */
#define	CF_SIGNAL	0


/* revision history:

	= 1988-02-01, David A­D­ Morano

	This subroutine was originally written.


	= 1988-02-01, David A­D­ Morano

	This subroutine was modified to not write out anything
	to standard output if the access time of the associated
	terminal has not been changed in 10 minutes.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	Call as :

	$ rfinger user@host [-d dialerspec]


*************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
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
#include	<userinfo.h>
#include	<logfile.h>
#include	<schedvar.h>
#include	<exitcodes.h>
#include	<mallocstuff.h>
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

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	USERPARTLEN	100
#define	HOSTPARTLEN	1024

#define	TO_CONNECT	5
#define	TO_READ		10


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	dialudp(char *,char *,int,int,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	dialtcpnls(char *,char *,int,char *,int,int) ;
extern int	dialtcpmux(char *,char *,int,char *,char **,int,int) ;
extern int	dialuss(char *,int,int) ;
extern int	dialussnls(char *,char *,int,int) ;
extern int	dialussmux(char *,char *,char **,int,int) ;
extern int	dialticotsord(char *,int,int,int) ;
extern int	dialticotsordnls(char *,int,char *,int,int) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strcpylc(char *,const char *,int) ;
extern char	*strcpyuc(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_gmlog(time_t,char *) ;


/* external variables */

extern char	makedate[] ;


/* local structures */


/* forward references */

static int	anyformat() ;
static int	isok(int) ;

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
	argopt_overlast
} ;

static const char	*sysfiles[] = {
	"%p/etc/%n/%n.%f",
	"%p/etc/%n/%f",
	"%p/etc/%n.%f",
	"%p/%n.%f",
	"%n.%f",
	NULL
} ;

static const char	*dialers[] = {
	"TCP",
	"TCPMUX",
	"TCPNLS",
	"USS",
	"USSMUX",
	"USSNLS",
	"TICOTSORD",
	"TICOTSORDNLS",
	NULL
} ;

enum dialers {
	dialer_tcp,
	dialer_tcpmux,
	dialer_tcpnls,
	dialer_uss,
	dialer_ussmux,
	dialer_ussnls,
	dialer_ticotsord,
	dialer_ticotsordnls,
	dialer_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct sigaction	sigs ;

	struct ustat	sb ;

	struct proginfo	g, *pip = &g ;

	struct userinfo	u ;

	SYSTEMS		sysdb ;

	DIALER		d ;

	CM_ARGS		ca ;

	sigset_t	signalmask ;

	bfile		errfile ;
	bfile		pidfile ;

	time_t	daytime ;

	int	argr, argl, aol, avl ;
	int	maxai, pan, npa, kwi, i, j ;
	int	rs, argnum, len ;
	int	ex = EX_INFO ;
	int	dialer, af, timeout = -1 ;
	int	sl, cl ;
	int	fd_debug ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_anyformat = TRUE ;
	int	f_help = FALSE ;

	char	*argp, *aop, *avp ;
	char	argpresent[NARGPRESENT] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	linebuf[LINEBUFLEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	char	userpart[USERPARTLEN + 3] ;
	char	hostpart[HOSTPARTLEN + 1] ;
	char	timebuf[TIMEBUFLEN] ;
	char	*pr = NULL ;
	char	*logfname = NULL ;
	char	*dialerspec = NULL ;
	char	*portspec = NULL ;
	char	*svcspec = NULL ;
	char	*sp, *cp, *cp2 ;


	cp = getenv(VARDEBUGFD1) ;

	if (cp == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if (cp == NULL)
	    cp = getenv(VARDEBUGFD3) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;

#if	CF_DEBUGS
	debugprintf("main: errfd=%d\n",fd_debug) ;
#endif


	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;


	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {

#if	COMMENT
	    u_close(2) ;
#endif

	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;

	}

	pip->helpfname = NULL ;

	pip->debuglevel = 0 ;
	pip->verboselevel = 1 ;

	pip->f_exit = FALSE ;
	pip->f.quiet = FALSE ;


/* process program arguments */

	for (i = 0 ; i < NARGPRESENT ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                if (cfdeci(argp + 1,argl - 1,&argnum))
	                    goto badargval ;

	            } else {

#if	CF_DEBUGS
	                debugprintf("main: got an option\n") ;
#endif

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key w/ a value\n") ;
#endif

	                    aol = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + argl - 1 - avp ;
	                    f_optequal = TRUE ;

	                } else
	                    avl = 0 ;

/* do we have a keyword match or should we assume only key letters ?

										    */

#if	CF_DEBUGS
	                debugprintf("main: about to check for a key word match\n") ;
#endif

	                if ((kwi = matstr(argopts,aop,aol)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option keyword, kwi=%d\n",
	                        kwi) ;
#endif

	                    switch (kwi) {

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                pr = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
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

#if	CF_DEBUGS
	                            debugprintf("main: debug flag, avp=\"%W\"\n",
	                                avp,avl) ;
#endif

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,&pip->debuglevel) ;

	                                if (rs < 0)
	                                    goto badargval ;

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
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                                if (rs < 0)
	                                    goto badargval ;

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

/* log file */
	                    case argopt_log:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                logfname = avp ;

	                        }

	                        break ;

/* display the time this program was last "made" */
	                    case argopt_makedate:
	                        f_makedate = TRUE ;
	                        break ;

	                    } /* end switch (key words) */

	                } else {

	                    while (akl--) {

	                        switch ((int) *aop) {

	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {

	                                    rs = cfdeci(avp,avl, 
	                                        &pip->debuglevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'd':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                dialerspec = argp ;

	                            break ;

/* port specification */
	                        case 'p':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                portspec = argp ;

	                            break ;

	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* service name */
	                        case 's':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                svcspec = argp ;

	                            break ;

/* timeout */
	                        case 't':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdeci(argp,argl,&timeout) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }

	                            break ;

	                        case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
	                                        &pip->verboselevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

	                        case 'x':
	                            f_anyformat = TRUE ;
	                            break ;

	                        default:
	                            bprintf(pip->efp,
	                                "%s : unknown option - %c\n",
	                                pip->progname,*aop) ;

/* fall through from above */
	                        case '?':
	                            f_usage = TRUE ;

	                        } /* end switch */

	                        akp += 1 ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                bprintf(pip->efp,"%s: extra arguments specified\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: finished parsing arguments\n",
	        pip->progname) ;


/* continue w/ the trivia argument processing stuff */

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_makedate)
	    bprintf(pip->efp,"%s: built %s\n",
	        pip->progname,makedate) ;

	if (f_usage)
	    goto usage ;

	if (f_version + f_makedate)
	    goto done ;


/* program root */

	if (pr == NULL) {

	    pr = getenv(VARPROGRAMROOT1) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT2) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT3) ;

/* try to see if a path was given at invocation */

	    if ((pr == NULL) && (pip->progdname != NULL))
	        proginfo_rootprogdname(pip) ;

/* do the special thing */

#if	CF_GETEXECNAME && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	    if ((pr == NULL) && (pip->pr == NULL)) {

	        const char	*pp ;


	        pp = getexecname() ;

	        if (pp != NULL)
	            proginfo_execname(pip,pp) ;

	    }
#endif /* SOLARIS */

	} /* end if (getting a program root) */

	if (pip->pr == NULL) {

	    if (pr == NULL)
	        pr = PROGRAMROOT ;

	    proginfo_setprogroot(pip,pr,-1) ;

	}


/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;


/* help */

	if (f_help) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: printhelp() pr=%s sn=%s\n",
	            pip->pr,pip->searchname) ;
#endif

	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	    goto retearly ;

	} /* end if */


	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;


/* check arguments */

#ifdef	COMMENT
	if ((dialerspec == NULL) || (dialerspec[0] == '\0'))
	    goto baddialer ;
#endif

	if (dialerspec != NULL) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: dialerspec=%s\n",dialerspec) ;
#endif

/* does this dialer specification have a port-like part */

	    if ((cp = strchr(dialerspec,':')) != NULL) {

	        portspec = cp + 1 ;
	        strwcpyuc(buf,dialerspec,(cp - dialerspec)) ;

	        buf[cp - dialerspec] = '\0' ;

	    } else
	        strcpyuc(buf,dialerspec) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("main: 2 dialerspec=%s\n",buf) ;
	        debugprintf("main: 2 portspec=%s\n",portspec) ;
	    }
#endif

	    rs = matstr(dialers,buf,-1) ;

	    if ((rs < 0) && (strcasecmp(buf,"unix") == 0))
	        rs = dialer_uss ;

	    if ((rs < 0) && (strcasecmp(buf,"unixstream") == 0))
	        rs = dialer_uss ;

	    if (rs < 0)
	        goto baddialer ;

	    dialer = rs ;

	} /* end if (dialerspec) */


	if ((svcspec == NULL) || (svcspec[0] == '\0'))
	    svcspec = SVCSPEC_RSEND ;

	if (timeout <= 0)
	    timeout = TO_READ ;

/* who are we ? */

	rs = userinfo(&u,userinfobuf,USERINFO_LEN,NULL) ;

	if (rs < 0)
	    goto baduser ;

	pip->pid = u.pid ;

	daytime = time(NULL) ;

/* do we have a log file ? */

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

	    struct utsname	un ;


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
	        timestr_log(daytime,timebuf),
	        pip->progname,
	        VERSION,(u.f.sysv_ct ? "SYSV" : "BSD")) ;

	    (void) u_uname(&un) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s) domain=%s\n",
	        (u.f.sysv_rt ? "SYSV" : "BSD"),
	        un.sysname,un.release,
	        u.domainname) ;

	    logfile_printf(&pip->lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;

	} else {

	    if (pip->debuglevel > 0) {

	        bprintf(pip->efp,
	            "%s: logfile=%s\n",
	            pip->progname,logfname) ;

	        bprintf(pip->efp,
	            "%s: could not open the log file (rs %d)\n",
	            pip->progname,rs) ;

	    }

	} /* end if (opened a log) */


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking for positional arguments\n") ;
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    debugprintf("main: do we have any ?\n") ;

	    if (npa > 0) {

	        debugprintf("main: we have some positional arguments\n") ;

	        pan = 0 ;
	        for (i = 0 ; i <= maxai ; i += 1) {

	            if (BATST(argpresent,i)) {

	                debugprintf("main: positional arg i=%d pan=%d arg=%s\n",
	                    i,pan,argv[i]) ;

	                pan += 1 ;

	            } /* end if */

	        } /* end for */

	    } /* end if (we have positional arguments) */

	} /* end if (debug) */
#endif /* CF_DEBUG */


/* do the main thing */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: performing main processing\n") ;
#endif

	pip->f_signal = FALSE ;
	pip->signal_num = -1 ;

	(void) sigemptyset(&signalmask) ;

#if	CF_SIGNAL
	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGHUP,&sigs,NULL) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGPIPE,&sigs,NULL) ;
#endif /* CF_SIGNAL */


/* setup for looping */

	rs = systems_init(&sysdb,NULL) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main: systems_init() rs=%d\n",rs) ;
#endif

	{
	    SCHEDVAR	sf ;

	    char	sysfname[MAXPATHLEN + 1] ;


	    schedvar_start(&sf) ;

	    schedvar_add(&sf,"p",pip->pr) ;

	    schedvar_add(&sf,"n",pip->searchname) ;

	    for (j = 0 ; j < 2 ; j += 1) {

	        if (j == 0)
	            schedvar_add(&sf,"f",SYSFNAME1) ;

	        else
	            schedvar_add(&sf,"f",SYSFNAME2) ;

	        for (i = 0 ; sysfiles[i] != NULL ; i += 1) {

	            rs = schedvar_expand(&sf,
			tmpfname,MAXPATHLEN,
			sysfiles[i],-1) ;

	            if (rs >= 0)
	                rs = u_access(tmpfname,R_OK) ;

	            if (rs >= 0) {

	                rs = systems_fileadd(&sysdb,tmpfname) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(5)) {
	                    debugprintf("main: systems_fileadd() rs=%d\n",
	                    debugprintf("main: fname=%s\n", tmpfname) ;
			}
#endif

	            }

	        } /* end for */

	    } /* end for */

	    schedvar_finish(&sf) ;

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


	dialer_init(&d,pip->pr,NULL) ;

	memset(&ca,0,sizeof(CM_ARGS)) ;

	ca.pr = pip->pr ;
	ca.sp = &sysdb ;
	ca.dp = &d ;
	ca.timeout = TO_CONNECT ;


/* loop through the arguments */

	if (npa > 0) {

	    bfile	outfile ;


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: npa=%d\n",npa) ;
#endif

	    rs = bopen(&outfile,BFILE_STDOUT,"dwct",0666) ;


	    pan = 0 ;
	    for (i = 0 ; i <= maxai ; i += 1) {

	        if (BATST(argpresent,i)) {

	            int	s, uplen ;


#if	CF_DEBUG
	            if (pip->debuglevel > 1) {
	                debugprintf("main: 2 pos_arg i=%d pan=%d arg=%s\n",
	                    i,pan,argv[i]) ;
	                debugprintf("main: dialer=%d\n",dialer) ;
	                debugprintf("main: portspec=%s\n",portspec) ;
	                debugprintf("main: svcspec=%s\n",svcspec) ;
	            }
#endif /* CF_DEBUG */

/* break the argument into username and hostname */

	            hostpart[0] = '\0' ;
	            userpart[0] = '\0' ;
	            if ((cp = strchr(argv[i],'@')) != NULL) {

	                sl = sfshrink(argv[i],(cp - argv[i]),&sp) ;

	                cp2 = strwcpy(userpart,sp,MIN(sl,USERPARTLEN)) ;

	                sl = sfshrink((cp + 1),-1,&sp) ;

	                strwcpy(hostpart,sp,MIN(sl,HOSTPARTLEN)) ;

	            } else {

	                sl = sfshrink(argv[i],-1,&sp) ;

	                cp2 = strwcpy(userpart,sp,MIN(sl,USERPARTLEN)) ;

	            }

	            *cp2++ = '\r' ;
	            *cp2++ = '\n' ;
	            *cp2 = '\0' ;
	            uplen = cp2 - userpart ;

/* try to make the connection */

	            if (dialerspec != NULL) {

	                rs = SR_INVALID ;
	                switch (dialer) {

	                case dialer_tcp:
	                    af = AF_UNSPEC ;
	                    if ((portspec != NULL) && (portspec[0] != '\0'))
	                        rs = dialtcp(hostpart,portspec,af,timeout,0) ;

	                    else
	                        rs = dialtcp(hostpart,svcspec,af,timeout,0) ;

	                    break ;

	                case dialer_tcpnls:
	                    af = AF_UNSPEC ;
	                    rs = dialtcpnls(hostpart,portspec,af,svcspec,
	                        timeout,0) ;

	                    break ;

	                case dialer_tcpmux:
	                    af = AF_UNSPEC ;
	                    rs = dialtcpmux(hostpart,portspec,af,svcspec,NULL,
	                        timeout,0) ;

	                    break ;

	                case dialer_uss:
	                    cp = portspec ;
	                    if ((cp == NULL) || (cp[0] == '\0'))
	                        cp = svcspec ;

	                    rs = dialuss(cp,
	                        timeout,0) ;

	                    break ;

	                case dialer_ussmux:
	                    rs = dialussmux(portspec,svcspec,NULL,
	                        timeout,0) ;

	                    break ;

	                case dialer_ussnls:
	                    rs = dialussnls(portspec,svcspec,
	                        timeout,0) ;

	                    break ;

	                case dialer_ticotsord:
	                    cp = portspec ;
	                    if ((cp == NULL) || (cp[0] == '\0'))
	                        cp = svcspec ;

	                    rs = dialticotsord(cp,-1, timeout,0) ;

	                    break ;

	                case dialer_ticotsordnls:
	                    rs = SR_INVALID ;
	                    if ((portspec != NULL) && (portspec[0] != '\0'))
	                        rs = dialticotsordnls(portspec,-1,svcspec,
	                            timeout,0) ;

	                    break ;

	                } /* end switch */

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf( "main: dial rs=%d\n", rs) ;
#endif /* CF_DEBUG */

	                s = rs ;
	                if (rs >= 0) {

/* write the username to the server */

	                    uc_writen(s,userpart,uplen) ;


/* get the results */

	                    if (! f_anyformat) {

	                        len = uc_readlinetimed(s,buf,BUFLEN,timeout) ;

#if	CF_DEBUG
	                        if (pip->debuglevel > 1)
	                            debugprintf("main: uc_readlinetimed() len=%d\n",
	                                len) ;
#endif /* CF_DEBUG */

	                        if (len > 24) {

	                            if (buf[len - 1] == '\n') {

	                                len -= 1 ;
	                                if (buf[len - 1] == '\r')
	                                    len -= 1 ;

	                            }

	                            buf[len] = '\0' ;
	                            bprintf(&outfile,"%-24s %s\n",
	                                buf,argv[i]) ;

	                        } else if (len > 0)
	                            bprintf(&outfile,"%-24s %s\n",
	                                "** short read **",argv[i]) ;

	                        else if (len < 0)
	                            bprintf(&outfile,
	                                "rs=%4d                  %s\n",
	                                len,argv[i]) ;

	                    } else
	                        rs = anyformat(&outfile,s,timeout) ;

	                    u_close(s) ;

	                } else
	                    bprintf(&outfile,"** no connection (%d) **\n",rs) ;

	            } else {

	                CM	con ;


#if	CF_DEBUG
	                if (DEBUGLEVEL(5))
	                    debugprintf("main: cm_open()\n") ;
#endif /* CF_DEBUG */

	                rs = cm_open(&con,&ca,hostpart,svcspec) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(5))
	                    debugprintf("main: cm_open() rs=%d\n",rs) ;
#endif /* CF_DEBUG */

	                if (rs >= 0) {

	                    rs = cm_write(&con,userpart,uplen) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(5))
	                        debugprintf("main: cm_write() rs=%d\n",rs) ;
#endif /* CF_DEBUG */

	                    while (TRUE) {

	                        rs = cm_reade(&con,linebuf,LINEBUFLEN,
	                            TO_READ,0) ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(5))
	                            debugprintf("main: cm_reade() rs=%d\n",rs) ;
#endif /* CF_DEBUG */

	                        if (rs <= 0)
	                            break ;

	                        len = rs ;
	                        uc_writen(FD_STDOUT,linebuf,len) ;

	                    } /* end while */

	                    cm_close(&con) ;

	                } /* end if (opened connection) */

	            } /* end if (dialerspec) */

	            pan += 1 ;

	        } /* end if (got a positional argument) */

	    } /* end for (loading positional arguments) */

	    bclose(&outfile) ;

	} else {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: npa=0\n") ;
#endif

	    daytime = time(NULL) ;

	    cp = timestr_gmlog(daytime,timebuf) ;

	    cp += 24 ;
	    *cp++ = '\n' ;
	    uc_writen(FD_STDOUT,timebuf,25) ;

	} /* end if (processing) */

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    tzset() ;

	    debugprintf("main: finishing, f_dst=%d\n",
	        daylight) ;
	    debugprintf("main: finishing, timezone=%ld (%d mins)\n",
	        timezone,(timezone / 60)) ;
	}
#endif /* CF_DEBUGS */


	dialer_free(&d) ;


	systems_free(&sysdb) ;


	u_close(FD_STDOUT) ;


	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;


/* close off and get out ! */
done:
ret2:
	logfile_close(&pip->lh) ;

retearly:
ret1:
	bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

	return ex ;

/* what are we about ? */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [host [...]] [-d dialer_specification]\n",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp,
	    "%s: \t[-s service] [-x] [-v]\n",
	    pip->progname) ;

	goto retearly ;

help:
	    ex = EX_INFO ;
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	    goto retearly ;

/* bad argument stuff */
badargnum:
	bprintf(pip->efp,"%s: not enough arguments specified\n",pip->progname) ;

	goto badarg ;

badargval:
	bprintf(pip->efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

baddialer:
	bprintf(pip->efp,
	    "%s: unknown dialer specification given\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto retearly ;

baduser:
	if (! pip->f.quiet)
	    bprintf(pip->efp,
	        "%s: could not get user information (%d)\n",
	        pip->progname,rs) ;

	ex = EX_NOUSER ;
	goto retearly ;

badret:
	goto done ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



void int_all(signum)
int	signum ;
{


	f_signal = TRUE ;
	signal_num = signum ;

}


static int anyformat(ofp,s,timeout)
bfile	*ofp ;
int	s ;
int	timeout ;
{
	bfile	tmpfile ;

	int	rs, i, len ;
	int	tlen = 0 ;
	int	olen ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	outbuf[LINEBUFLEN + 1] ;


	rs = bopen(&tmpfile,(char *) s,"rd",0666) ;

	if (rs < 0)
	    return rs ;

	while ((rs = breadline(&tmpfile,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    olen = 0 ;
	    for (i = 0 ; i < len ; i += 1) {

	        if (isprint(linebuf[i]) || isok(linebuf[i]))
	            outbuf[olen++] = linebuf[i] ;

	    } /* end for */

	    if (olen > 0) {

	        bwrite(ofp,outbuf,olen) ;

	        tlen += olen ;

	    }

	} /* end while (reading lines) */

	bclose(&tmpfile) ;

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (anyformat) */


static int isok(ch)
int	ch ;
{


	return ((ch == '\t') || (ch == '\n')) ;
}



