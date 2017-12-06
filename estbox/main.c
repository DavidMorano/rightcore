/* main (REXEC) */

/* program to INET REXEC a remote command */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_REXECL	1
#define	CF_RCMDU	1


/* revision history:

	= 96/11/21, David A­D­ Morano

	This program was started by copying from the RSLOW program.


	= 96/12/12, David A­D­ Morano

	I modified the program to take the username and password
	from a specified file (for better security).


*/


/**************************************************************************

	This is the front-end of the ESTBOX program.



**************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */
#include	<time.h>
#include	<netdb.h>
#include	<stropts.h>
#include	<poll.h>
#include	<errno.h>

#include	<bfile.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<vecitem.h>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"netfile.h"


/* local defines */

#define	NPARG		2
#define	NFDS		5
#define	USERBUFLEN	(NODENAMELEN + (2 * 1024))


/* define command option words */

static char *argopts[] = {
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"ROOT",
	"LOGFILE",
	NULL,
} ;


#define	ARGOPT_TMPDIR	0
#define	ARGOPT_VERSION	1
#define	ARGOPT_VERBOSE	2
#define	ARGOPT_ROOT	3
#define	ARGOPT_LOGFILE	4


/* external subroutines */

extern int	getnodedomain() ;
extern int	authfile() ;
extern int	cfdec() ;
extern int	isdigitlatin(int) ;

extern int	hostequiv() ;

extern char	*timestr_log() ;
extern char	*getlogin() ;
extern char	*reventstr() ;
extern char	*strshrink(), *strbasename() ;
extern char	*malloc_str(char *) ;


/* forward subroutines */

static int	copymachines() ;


/* external variables */

extern int	errno ;


/* local variables */

struct global	g ;




int main(argc,argv)
int	argc ;
char	**argv ;
{
	struct ustat	sb, isb, osb, esb ;

	struct tm	*timep ;

	struct passwd	*pp ;

	struct group	*gp ;

	struct servent	*sp, se ;

	struct pollfd	fds[NFDS] ;

	struct userinfo	u ;

	struct netrc	*mp ;

	bfile		errfile, *efp = &errfile ;

	vecitem		ne, tmp ;

	time_t	t_sanity ;

	int	argr, argl, aol, akl, avl ;
	int	kwi, npa, ai = 0, i, j ;
	int	rs, len, l ;
	int	argnum = 0 ;
	int	f_optplus, f_optminus, f_optequal ;
	int	f_exitargs = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_noinput = FALSE ;
	int	f_initlist = FALSE ;
	int	f_in0, f_out1, f_out2, f_in3, f_out3, f_in4 ;
	int	f_final0, f_final1, f_final2, f_final3, f_final4 ;
	int	ifd = -1, ofd = 1, efd = -1 ;
	int	f_eof0, f_eof3, f_eof4 ;
	int	f_exit ;
	int	f_euid ;
	int	rfd, rfd2 ;
	int	childstat ;
	int	port = -1 ;
	int	pollinput = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;
	int	polloutput = POLLWRNORM | POLLWRBAND ;

	char	*argp, *aop, *akp, *avp ;
	char	*infname = NULL ;
	char	*afname = NULL ;
	char	buf[BUFLEN + 1], *bp ;
	char	userbuf[USERBUFLEN + 1] ;
	char	un_buf[33] ;
	char	pd_buf[33] ;
	char	*username = NULL ;
	char	*password = NULL ;
	char	*authorization = "any" ;
	char	*cmdname = NULL ;
	char	*cp, *cp1, *cp2 ;
	char	cmdbuf[CMDBUFLEN + 1] ;
	char	*hostname = NULL ;
	char	ahostname[2048 + 1] ;
	char	*ahost = ahostname ;


	g.progname = strbasename(argv[0]) ;

	efd = 3 ;
	if (fstat(efd,&esb) < 0) {

	    rs = errno ;
#if	CF_DEBUGS
	    nprintf("/tmp/open.log","rexec: EFD errno=%d\n",rs) ;
#endif
	    efd = 2 ;
	}

	debugsetfd(efd) ;

	if (bopen(efp,(char *) efd,"wca",0666) < 0) return BAD ;

	bcontrol(efp,BC_LINEBUF,0) ;

#if	CF_DEBUG
	if (efd == 3) debugprintf("main: we are erroring to 3\n") ;
#endif

/* miscellaneous early stuff */

	g.tmpdir = NULL ;
	g.programroot = NULL ;
	g.efp = efp ;
	g.logfname = NULL ;
	g.netfname = NULL ;
	g.debuglevel = 0 ;
	g.keeptime = (time_t) DEFKEEPTIME ;
	g.euid = geteuid() ;

	g.f.log = FALSE ;
	g.f.verbose = FALSE ;
	g.f.keepalive = FALSE ;
	g.f.quiet = FALSE ;

/* parse arguments */

	npa = 0 ;			/* number of positional so far */
	i = 0 ;
	argr = argc - 1 ;
	while ((! f_exitargs) && (argr > 0)) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optplus || f_optminus)) {

	        if (argl > 1) {
		    const int	ach = MKCHAR(argp[1]) ;

	            if (isdigitlatin(ach)) {

	                if (cfdec(argp + 1,argl - 1,&argnum) < 0)
	                    goto badarg ;

	                if (argnum < 0) argnum = 0 ;

	            } else {

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

	                if ((kwi = matstr(argopts,akp,akl)) >= 0) {

	                    switch (kwi) {

/* temporary directory */
	                    case ARGOPT_TMPDIR:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.tmpdir = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.tmpdir = argp ;

	                        }

	                        break ;

/* version */
	                    case ARGOPT_VERSION:
#if	CF_DEBUGS
	                        debugprintf("main: version key-word\n") ;
#endif
	                        f_version = TRUE ;
	                        if (f_optequal) goto badargextra ;

	                        break ;

/* verbose mode */
	                    case ARGOPT_VERBOSE:
	                        g.f.verbose = TRUE ;
	                        break ;

/* program root */
	                    case ARGOPT_ROOT:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.programroot = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.programroot = argp ;

	                        }

	                        break ;

/* LOGFILE */
	                    case ARGOPT_LOGFILE:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.logfname = avp ;

	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
	                        bprintf(efp,
	                            "%s: unknown argument keyword \"%s\"\n",
	                            g.progname,akp) ;

	                        f_exitargs = TRUE ;
	                        f_usage = TRUE ;

	                    } /* end switch */

	                } else {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        debugprintf("main: option key letters >%c<\n",
	                            *akp) ;
#endif

	                        switch ((int) *akp) {

/* debug */
	                        case 'D':
	                            g.debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if ((avl > 0) &&
	                                    (cfdec(avp,avl,&g.debuglevel) < 0))
	                                    goto badargvalue ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* alternate user's NETRC file */
	                        case 'N':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    g.netfname = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    g.netfname = argp ;

	                            }

	                            break ;

/* input file name */
	                        case 'i':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    infname = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    infname = argp ;

	                            }

	                            break ;

/* specify target host */
	                        case 'h':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    hostname = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    hostname = argp ;

	                            }

	                            break ;

/* authorization file */
	                        case 'a':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    afname = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    afname = argp ;

	                            }

	                            break ;

/* specify user */
	                        case 'l':
	                        case 'u':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    username = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    username = argp ;

	                            }

	                            break ;

/* specify password */
	                        case 'p':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    password = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    password = argp ;

	                            }

	                            break ;

/* authorization restriction */
	                        case 'A':
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    authorization = avp ;

	                            } else {

	                                if (argr <= 0) goto badargnum ;

	                                argp = argv[++i] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    authorization = argp ;

	                            }

	                            break ;

/* no input mode (command only) */
	                        case 'n':
#if	CF_DEBUGS
	                            debugprintf("main: at option key 'n'\n") ;
#endif

	                            f_noinput = TRUE ;
	                            break ;

	                        case 'k':
	                            g.f.keepalive = TRUE ;
	                            break ;

	                        case 'q':
	                            g.f.quiet = TRUE ;
	                            break ;

	                        case 's':
	                            g.f.sanity = TRUE ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if ((avl > 0) &&
	                                    (cfdec(avp,avl,&g.keeptime) < 0))
	                                    goto badargvalue ;

	                            }

	                            break ;

	                        case 'v':
	                            g.f.verbose = TRUE ;
	                            break ;

	                        default:
	                            bprintf(efp,"%s: unknown option - %c\n",
	                                g.progname,*aop) ;

	                        case '?':
	                            f_usage = TRUE ;

	                        } /* end switch */

#if	CF_DEBUGS
	                        debugprintf("main: out of option key letters switch\n") ;
#endif

	                        akp += 1 ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minus sign character alone on the command line */

	            npa += 1 ;

	        } /* end if */

	    } else {

	        switch (npa) {

	        case 0:
	            if (argl > 0)
	                hostname = argp ;

	            break ;

	        case 1:
	            cmdname = argp ;
	            ai = i + 1 ;
	            f_exitargs = TRUE ;
	            break ;

	        default:
	            bprintf(efp,
	                "%s: extra positional arguments ignored\n",
	                g.progname) ;

	        } /* end switch */

	        npa += 1 ;

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


#if	CF_DEBUGS
	debugprintf("main: out of argument processing loop\n") ;
#endif

/* try writing over our 'argv[0]' argument */

#ifdef	COMMENT
	for (i = 0 ; argv[0][i] != '\0' ; i += 1)
	    argv[0][i] = 'X' ;
#endif


/* pop out the password if it was given (if was on invocation line) */

	if (password != NULL) {

	    cp = malloc_str(password) ;

/* try to write over the password a passed down in the arguments */

	    for (i = 0 ; password[i] != '\0' ; i += 1)
	        password[i] = 'X' ;

	    password = cp ;

	} /* end if (overwriting the password) */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: about to make a log entry \n") ;
#endif



	if (g.debuglevel > 0)
	    bprintf(efp,"%s: finished parsing arguments\n",
	        g.progname) ;

	if (g.debuglevel > 0)
	    bprintf(efp,"%s: debug level %d\n",
	        g.progname,g.debuglevel) ;

	rs = OK ;
	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        g.progname,VERSION) ;

	    goto earlyret ;
	}

	if (f_usage) goto usage ;



#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: f_exitargs=%d ai=%d\n",f_exitargs,ai) ;
#endif

/* get a TMPDIR */

	if (g.tmpdir == NULL) {

	    if ((g.tmpdir = getenv("TMPDIR")) == NULL)
	        g.tmpdir = TMPDIR ;

	}

/* initialize some other common user stuff */

	if (userinfo(&u,userbuf,USERBUFLEN,NULL) < 0)
	    if (u.domainname == NULL) u.domainname = "" ;

/* set our effective UID to the real user's UID (for safety) */

	g.uid = u.uid ;
	if (g.uid != g.euid) seteuid(g.uid) ;


/* get our program root directory */

	if (g.programroot == NULL) {

	    if ((g.programroot = getenv("PROGRAMROOT")) == NULL)
	        g.programroot = PROGRAMROOT ;

	}

/* get the current time-of-day */

	time(&g.daytime) ;

	timep = localtime(&g.daytime) ;


/* set some flags */


/* find the log file */

	if (g.logfname == NULL) {

	    sprintf(buf,"%s/%s",g.programroot,LOGFNAME) ;

	    g.logfname = malloc_str(buf) ;

	}

/* open the log file (if we can) */

	if ((g.logfname != NULL) &&
	    (logfile_open(&g.lh,g.logfname,0,0666,u.logid) >= 0)) {

	    g.f.log = TRUE ;

	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

	    logfile_printf(&g.lh,"%02d%02d%02d %02d%02d:%02d %-14s %s/%s\n",
	        timep->tm_year,
	        timep->tm_mon + 1,
	        timep->tm_mday,
	        timep->tm_hour,
	        timep->tm_min,
	        timep->tm_sec,
	        g.progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	    logfile_printf(&g.lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;

	    logfile_printf(&g.lh,"os=%s domain=%s\n",
	        (u.f.sysv_rt) ? "SYSV" : "BSD",
	        u.domainname) ;

	} /* end if (opened log file) */


/* check arguments */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: about to check arguments\n",
	        g.progname) ;
#endif

	if ((hostname == NULL) || (*hostname == '\0'))
	    goto badhost ;

	if ((cmdname == NULL) || (*cmdname == '\0'))
	    goto badprog ;


/* process the 'hostname' to see if there is a port also */

	if ((cp = strchr(hostname,':')) != NULL) {

	    *cp++ = '\0' ;
	    if (cfdec(cp,-1,&port) < 0) {

	        port = -1 ;

#if	SYSV
	        sp = getservbyname_r(cp, "tcp",
	            &se,cmdbuf,CMDBUFLEN) ;
#else
	        sp = getservbyname(cp, "tcp") ;
#endif

	        if (sp != NULL) {

	            port = (int) ntohs(sp->s_port) ;

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("main: service specified port=%d\n",
	                    port) ;
#endif

	        }

	    } /* end if (bad decimal conversion) */

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: specified port=%d\n",
	            port) ;
#endif

	} /* end if (hostname had a port) */


/* do we have an authorization file ? */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: authroization file \"%s\"\n",
	        (afname != NULL) ? afname : "*not there*") ;
#endif

	if (afname != NULL) {

	    if (g.f.verbose)
	        bprintf(efp,"%s: authorization file \"%s\"\n",
	            g.progname,afname) ;

	    f_euid = FALSE ;
	    if (access(afname,R_OK) < 0) {

	        f_euid = TRUE ;
	        seteuid(g.euid) ;

	    }

	    un_buf[0] = '\0' ;
	    pd_buf[0] = '\0' ;
	    rs = authfile(afname,un_buf,pd_buf) ;

	    if (f_euid) seteuid(g.uid) ;

	    if (rs >= 0) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: got a good file return\n") ;
#endif

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: ub=\"%s\" pb=\"%s\"\n",un_buf, pd_buf) ;
#endif


	        if ((un_buf[0] != '\0') && 
	            ((username == NULL) || (username[0] == '\0')))
	            username = un_buf ;

	        if ((pd_buf[0] != '\0') && 
	            ((password == NULL) || (password[0] == '\0')))
	            password = pd_buf ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: u=\"%s\" p=\"%s\"\n",
	                username, password) ;
#endif

	    } else {

	        bprintf(efp,
	            "%s: error in authroization file (rs %d)\n",
	            g.progname,rs) ;

	    }

	} /* end if (authorization file processing) */


	if ((username == NULL) || (username[0] == '\0'))
	    username = u.username ;


/* find the user's NETRC file */

	if (g.netfname == NULL) {

	    sprintf(buf,"%s/.netrc",u.homedname) ;

	    g.netfname = malloc_str(buf) ;

	}


/* make some log entries */

#ifdef	COMMENT
	logfile_printf(&g.lh,"host=%s.%s user=%s(%d)\n",
	    (u.nodename != NULL) ? u.nodename : "*unknown*",
	    (u.domainname != NULL) ? u.domainname : "*unknown*",
	    (u.username != NULL) ? u.username : "*unknown*",u.uid) ;
#endif

/* get the INET service port for the REXEC service */

	if (port < 0) {

#if	SYSV
	    sp = getservbyname_r("exec", "tcp",
	        &se,cmdbuf,CMDBUFLEN) ;
#else
	    sp = getservbyname("exec", "tcp") ;
#endif

	    if (sp == NULL) {

#if	SYSV
	        sp = getservbyname_r("rexec", "tcp",
	            &se,cmdbuf,CMDBUFLEN) ;
#else
	        sp = getservbyname_r("rexec", "tcp") ;
#endif

	    }

	    if (sp != NULL) {
	        port = (int) ntohs(sp->s_port) ;
	    } else {
	        port = DEFEXECSERVICE ;
	    }

	} /* end if (default port) */

/* open files (if necessary) */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: about to open input file\n") ;
#endif

	if (! f_noinput) {
	    if (infname != NULL) {
	        u_close(0) ;
	        rs = uc_open(infname,O_RDONLY,0666) ;
		ifd = rs ;
	    } else {
	        ifd = FD_STDIN ;
 	    }
	    if (rs >= 0) {
	        if ((rs = fstat(ifd,&isb)) < 0) f_noinput = TRUE ;
	    }
	} /* end if (we have input) */

/* create the remote command */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: creating remote command\n") ;
#endif

	len = sprintf(cmdbuf,"%s",cmdname) ;

	for (i = ai ; argv[i] != NULL ; i += 1) {

#if	CF_DEBUG
	    if (g.debuglevel > 2)
	        debugprintf("main: arg%d> %s\n",i,argv[i]) ;
#endif

	    l = quoteshellarg(argv[i],-1,buf,BUFLEN,&bp) ;

	    if ((l < 0) || (l > (BUFLEN - len))) goto badtoomuch ;

#if	CF_DEBUG
	    if (g.debuglevel > 2)
	        debugprintf("main: sharg%d> %W\n",i,bp,l) ;
#endif

	    cmdbuf[len++] = ' ' ;
	    strwcpy(cmdbuf + len,bp,l) ;

	    len += l ;

	} /* end while */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: cmd=\"%s\"\n",cmdbuf) ;
#endif

/* get a canonical name for our target hostname */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: about to get canonical on h=\"%s\"\n",
	        hostname) ;
#endif

	rs = getchostname(hostname,buf) ;

	if (buf[0] != '\0')
	    hostname = malloc_str(buf) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: canonical h=\"%s\"\n",hostname) ;
#endif


/* make a log about where we want to go */

	if (((cp = strchr(hostname,'.')) != NULL) && 
	    (strcasecmp(cp + 1,u.domainname) == 0)) {
	    logfile_printf(&g.lh,
	        "target h=%W u=%s\n",hostname,cp - hostname,username) ;

	} else
	    logfile_printf(&g.lh,"target h=%s u=%s\n",hostname,username) ;

	if (rs < 0) {

	    logfile_printf(&g.lh,"host is unreachable\n") ;

	    goto badunreach ;
	}

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: done with canonical business\n") ;
#endif


/* pop off the remote command */

	if (password != NULL) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: executing REXEC on supplied password\n") ;
#endif

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: supplied m=\"%s\" u=\"%s\" p=\"%s\"\n",
	            hostname,username,password) ;
#endif

	    ahost = ahostname ;
	    strcpy(ahostname,hostname) ;

#if	CF_REXECL
	    rs = rexecl(&ahost,(unsigned short) port,username,
	        password, cmdbuf,&rfd2) ;
#else
	    if (password == NULL) password = "" ;

	    rs = rexec(&ahost,(unsigned short) port,username,
	        password, cmdbuf,&rfd2) ;
#endif

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: REXECL rs=%d\n", rs) ;
#endif

	    rfd = rs ;
	    if (rs >= 0) goto goodrexec ;

	} /* end if (tried supplied password) */

	if (strncmp(authorization,"supplied",1) == 0) goto badrexec ;


/* process any NETRC files that we can find */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: about to process the NETRC files\n") ;
#endif

	rs = vecitem_start(&ne,20,0) ;

	if (rs < 0)
		goto badinit ;

	f_initlist = TRUE ;

/* process the local user's NETRC file */

	if (g.netfname != NULL)
	    copymachines(&ne,g.netfname,hostname,u.domainname) ;

/* process the system-wide NETRC file */

	sprintf(buf,"%s/%s",g.programroot,NETFILE1) ;

	copymachines(&ne,buf,hostname,u.domainname) ;

	sprintf(buf,"%s/%s",g.programroot,NETFILE2) ;

	copymachines(&ne,buf,hostname,u.domainname) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: processed NETRC files\n") ;
#endif

/* try to find a matching NETRC entry for the host/user pair that we have */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: m=%s u=%s\n",hostname,username) ;
#endif

	rs = -1 ;
	for (i = 0 ; vecitem_get(&ne,i,&mp) >= 0 ; i += 1) {

	    if (mp == NULL) continue ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) {

	        debugprintf("main: entry i=%d\n",
	            i) ;

	        if (mp->machine != NULL)
	            debugprintf("main: m=\"%s\"\n",
	                mp->machine) ;

	        if (mp->login != NULL)
	            debugprintf("main: u=\"%s\"\n",
	                mp->login) ;

	        if (mp->password != NULL)
	            debugprintf("main: p=\"%s\"\n",
	                mp->password) ;

	    }
#endif

	    if ((mp->machine == NULL) ||
	        (mp->login == NULL) ||
	        (mp->password == NULL)) continue ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) {

	        debugprintf("main: looking at entry m=\"%s\"\n",
	            mp->machine) ;

	    }
#endif

	    if ((hostequiv(hostname,mp->machine,u.domainname)) &&
	        (strcmp(mp->login,username) == 0)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: trying entry m=\"%s\" u=\"%s\"\n",
	                mp->machine,mp->login) ;
#endif

	        ahost = ahostname ;
	        strcpy(ahostname,hostname) ;

#if	CF_REXECL
	        rs = rexecl(&ahost,(unsigned short) port,username,
	            mp->password, cmdbuf,&rfd2) ;
#else
	        password = mp->password ;
	        if (password == NULL) password = "" ;

	        rs = rexec(&ahost,(unsigned short) port,username,
	            password, cmdbuf,&rfd2) ;
#endif

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: REXECL rs=%d\n", rs) ;
#endif

	        rfd = rs ;
	        if (rs >= 0) break ;

	    }

	} /* end for */

	if (rs >= 0) goto goodrexec ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: no matching NETRC entries\n") ;
#endif

	if (strncmp(authorization,"password",1) == 0) goto badrexec ;


#if	CF_RCMDU

/* try RCMDU with the supplied username */

	if (! g.f.keepalive) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: executing RCMDU u=%s\n",username) ;
#endif

	    rs = rcmdu(hostname,username,
	        cmdbuf,&rfd2) ;

	    rfd = rs ;
	    if (rs >= 0) {

	        logfile_printf(&g.lh,"connected w/ RCMDU\n") ;

	        goto goodrexec ;
	    }

	    if ((rs == SR_HOSTDOWN) || (rs == SR_HOSTUNREACH))
	        goto hostdown ;

	} /* end if */

#endif

	if (strncmp(authorization,"user",1) == 0) goto badrexec ;


/* try no password on the specified username */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: executing REXEC with blank password\n") ;
#endif

	ahost = ahostname ;
	strcpy(ahostname,hostname) ;

#if	CF_REXECL
	rs = rexecl(&ahost,(unsigned short) port,username,
	    "", cmdbuf,&rfd2) ;
#else
	rs = rexec(&ahost,(unsigned short) port,username,
	    "", cmdbuf,&rfd2) ;
#endif

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: REXECL rs=%d\n", rs) ;
#endif

	rfd = rs ;
	if (rs >= 0) goto goodrexec ;

/* we couldn't get through with a NULL password */


/* try to find a matching NETRC entry for the host only */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: autologin m=%s\n",hostname) ;
#endif

	rs = -1 ;
	for (i = 0 ; vecitem_get(&ne,i,&mp) >= 0 ; i += 1) {

	    if (mp == NULL) continue ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) {

	        debugprintf("main: entry i=%d\n",
	            i) ;

	        if (mp->machine != NULL)
	            debugprintf("main: m=\"%s\"\n",
	                mp->machine) ;

	        if (mp->login != NULL)
	            debugprintf("main: u=\"%s\"\n",
	                mp->login) ;

	        if (mp->password != NULL)
	            debugprintf("main: p=\"%s\"\n",
	                mp->password) ;

	    }
#endif

	    if ((mp->machine == NULL) ||
	        (mp->login == NULL)) continue ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) {

	        debugprintf("main: looking at entry m=\"%s\"\n",
	            mp->machine) ;

	    }
#endif

	    if (hostequiv(hostname,mp->machine,u.domainname) &&
	        (strcasecmp(mp->login,username) != 0)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: trying entry m=\"%s\" u=\"%s\"\n",
	                mp->machine,mp->login) ;
#endif

	        ahost = ahostname ;
	        strcpy(ahostname,hostname) ;

#if	CF_REXECL
	        rs = rexecl(&ahost,(unsigned short) port,mp->login,
	            mp->password, cmdbuf,&rfd2) ;
#else
	        password = mp->password ;
	        if (password == NULL) password = "" ;

	        rs = rexec(&ahost,(unsigned short) port,mp->login,
	            password, cmdbuf,&rfd2) ;
#endif

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: REXECL rs=%d\n", rs) ;
#endif

	        rfd = rs ;
	        if (rs >= 0) break ;

	    }

	} /* end for */

	if (rs >= 0) {

	    username = mp->login ;
	    goto goodrexec ;
	}

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: couldn't find a NETRC entry \n") ;
#endif


#if	CF_RCMDU

/* finally we try to connect using RCMDU with our own username */

	if ((! g.f.keepalive) && (strcmp(username,u.username) != 0)) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: use our own username w/ RCMDU\n") ;
#endif

	    rs = rcmdu(hostname,u.username,
	        cmdbuf,&rfd2) ;

	    rfd = rs ;
	    if (rs >= 0) {

	        logfile_printf(&g.lh,"connected w/ RCMDU\n") ;

	        username = u.username ;
	        goto goodrexec ;
	    }

	    if ((rs == SR_HOSTDOWN) || (rs == SR_HOSTUNREACH))
	        goto hostdown ;

	} /* end if */

#endif


/* we failed all attempts */

#ifdef	COMMENT
	logfile_printf(&g.lh,"failed all authorization attempts\n") ;
#endif

	goto badrexec ;



/* go ahead with an REXEC */
goodrexec:
	logfile_printf(&g.lh,"connected u=%s\n",username) ;

	if (f_initlist)
		vecitem_finish(&ne) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: about to go into the processing loop\n") ;
#endif

/* setup the 'poll(2)' structures for use later */

	f_in0 = FALSE ;
	f_out1 = FALSE ;
	f_out2 = FALSE ;
	f_in3 = FALSE ;
	f_out3 = FALSE ;
	f_in4 = FALSE ;
	f_eof0 = f_eof3 = f_eof4 = FALSE ;
	f_final0 = f_final1 = f_final2 = f_final3 = f_final4 = FALSE ;

/* standard input */

	fds[0].fd = -1 ;
	if (! f_noinput) {

	    fds[0].fd = ifd ;
	    fds[0].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;

	} else {

	    f_eof0 = TRUE ;
	    if ((fstat(rfd,&sb) >= 0) && 
	        (S_ISSOCK(sb.st_mode) || S_ISCHR(sb.st_mode)))
	        shutdown(rfd,1) ;

	    else
	        write(rfd,buf,0) ;

	}

/* standard output */

	fds[1].fd = -1 ;
	if ((rs = fstat(ofd,&osb)) >= 0) {

	    fds[1].fd = ofd ;
	    fds[1].events = POLLWRNORM | POLLWRBAND ;

	}

/* standard error */

	fds[2].fd = -1 ;
	if (fstat(2,&esb) >= 0) {

	    bflush(efp) ;

	    fds[2].fd = 2 ;
	    fds[2].events = POLLWRNORM | POLLWRBAND ;

	}

/* remote socket */

	fds[3].fd = rfd ;
	fds[3].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;
	fds[3].events |= (fds[3].events | POLLWRNORM | POLLWRBAND) ;

/* remote error socket */

	fds[4].fd = rfd2 ;
	fds[4].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI ;


/* do it */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("nain: we got connected\n") ;
#endif

	bflush(efp) ;

/* do the copy data function */

	f_exit = FALSE ;
	while (! f_exit) {

	    if (poll(fds,NFDS,POLLTIME) < 0) rs = (- errno) ;

#if	CF_DEBUG
	    if (g.debuglevel > 2)
	        debugprintf("main: back from POLL w/ rs=%d\n",
	            rs) ;
#endif

	    if (rs < 0) {

	        if (errno == EAGAIN) {

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("main: back from POLL w/ EAGAIN\n") ;
#endif

	            sleep(2) ;

	            continue ;

	        } else if (errno == EINTR) {

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("main: back from POLL w/ EINTR\n") ;
#endif

	            continue ;

	        } else {

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("main: back from POLL w/ BADPOLL\n") ;
#endif

	            goto badpoll ;
	        }

	    } /* end if (poll got an error) */

#if	CF_DEBUG
	    if (g.debuglevel > 2) {

	        for (i = 0 ; i < NFDS ; i += 1) {

	            debugprintf("main: fds%d %s\n",i,
	                reventstr(fds[i].revents,buf,BUFLEN)) ;

	        }
	    }
#endif

/* check the actual low level events */

	    if (fds[0].revents & pollinput) {

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("main: IN0\n") ;
#endif

	        f_in0 = TRUE ;
	        fds[0].events &= (~ pollinput) ;

	    }

	    if (fds[0].revents & POLLHUP) {

	        f_in0 = TRUE ;
	        f_final0 = TRUE ;
	        fds[0].events &= (~ pollinput) ;
	        fds[0].fd = -1 ;

	    }

	    if (fds[1].revents & polloutput) {

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("main: OUT1\n") ;
#endif

	        f_out1 = TRUE ;
	        fds[1].events &= (~ polloutput) ;

	    }

	    if (fds[2].revents & polloutput) {

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("main: OUT2\n") ;
#endif

	        f_out2 = TRUE ;
	        fds[2].events &= (~ polloutput) ;

	    }

	    if (fds[3].revents & pollinput) {

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("main: IN3\n") ;
#endif

	        f_in3 = TRUE ;
	        fds[3].events &= (~ pollinput) ;

	    }

	    if ((fds[3].revents & POLLHUP) ||
	        (fds[3].revents & POLLERR) ||
	        (fds[3].revents & POLLNVAL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 2) {

	            if (fds[3].revents & POLLHUP)
	                debugprintf("main: 3 POLLHUP\n") ;

	            if (fds[3].revents & POLLERR)
	                debugprintf("main: 3 POLLERR\n") ;

	            if (fds[3].revents & POLLNVAL)
	                debugprintf("main: 3 POLLNVAL\n") ;

	        }
#endif

	        f_in3 = TRUE ;
	        f_final3 = TRUE ;
	        fds[3].fd = -1 ;

	    }

	    if (fds[3].revents & polloutput) {

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("main: OUT3\n") ;
#endif

	        f_out3 = TRUE ;
	        fds[3].events &= (~ polloutput) ;

	    }

	    if (fds[4].revents & pollinput) {

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("main: IN4\n") ;
#endif

	        f_in4 = TRUE ;
	        fds[4].events &= (~ pollinput) ;

	    }

	    if (fds[4].revents & POLLHUP) {

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("main: HUP4\n") ;
#endif

	        f_final4 = TRUE ;
	        f_in4 = TRUE ;
	        fds[4].fd = -1 ;

	    }

/* what did we not look at */

#if	! CF_DEBUG
	    if ((! f_in0) && (! f_out1) && (! f_out2) && (! f_in3)
	        && (! f_in4)) {

	        if (g.debuglevel > 2) {

	            debugprintf("main: fd0=%08X\n",fds[0].revents) ;

	            debugprintf("main: fd1=%08X\n",fds[1].revents) ;

	            debugprintf("main: fd2=%08X\n",fds[2].revents) ;

	            debugprintf("main: fd3=%08X\n",fds[3].revents) ;

	            debugprintf("main: fd4=%08X\n",fds[4].revents) ;

	        }

	        break ;
	    }
#else
	    if (g.debuglevel > 2) {

	        debugprintf("main: fd0=%08X\n",fds[0].revents) ;

	        debugprintf("main: fd1=%08X\n",fds[1].revents) ;

	        debugprintf("main: fd2=%08X\n",fds[2].revents) ;

	        debugprintf("main: fd3=%08X\n",fds[3].revents) ;

	        debugprintf("main: fd4=%08X\n",fds[4].revents) ;

	    }
#endif

/* now we are ready to check for the events that we really want */

/* output from remote command to our standard output */

	    if (f_in3 && f_out1) {

	        len = read(rfd,buf,BUFLEN) ;

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("main: IN3 -> OUT1 len=%d errno=%d\n",
	                len,errno) ;
#endif
	        if (len > 0) {

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("main: IN3 -> OUT1 \"%W\"\n",buf,len) ;
#endif

	            writen(fds[1].fd,buf,len) ;

	        } else {

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("main: IN3 EOF\n") ;
#endif
	            f_eof3 = TRUE ;
	            if (f_eof0) {

	                fds[3].fd = -1 ;

	            } else {

	                fds[3].events &= (~ pollinput) ;

	            }

	        }

	        if (! f_final3) {

	            f_in3 = FALSE ;
	            fds[3].events |= pollinput ;

	        }

	        f_out1 = FALSE ;
	        fds[1].events |= polloutput ;
	    }

/* input from our standard input out to the remote command */

	    if (f_in0 && f_out3) {

	        len = read(ifd,buf,BUFLEN) ;

#if	CF_DEBUG
	        if (g.debuglevel > 2)
	            debugprintf("main: IN0 -> OUT3  len=%d\n",len) ;
#endif

	        if (len > 0)
	            writen(fds[3].fd,buf,len) ;

	        else {

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("main: IN0 EOF\n") ;
#endif
	            f_eof0 = TRUE ;
	            f_in0 = FALSE ;
	            fds[0].fd = -1 ;
#if	CF_DEBUG
	            if (g.debuglevel > 2) {
	                rs = fstat(rfd,&sb) ;

	                debugprintf("main: RFD stat rs=%d mode=%08X\n",
	                    rs,sb.st_mode) ;
	            }
#endif

	            if ((fstat(rfd,&sb) >= 0) && 
	                (S_ISSOCK(sb.st_mode) || S_ISCHR(sb.st_mode))) {

	                shutdown(rfd,1) ;

#if	CF_DEBUG
	                if (g.debuglevel > 2)
	                    debugprintf("main: we shutdown the remote FD\n") ;
#endif
	            } else
	                write(rfd,buf,0) ;
	        }

	        if (! f_final0) {

	            f_in0 = FALSE ;
	            fds[0].events |= pollinput ;
	        }

	        f_out3 = FALSE ;
	        fds[3].events |= polloutput ;
	    }

/* output from remote command error channel to our standard error output */

	    if (f_in4 && f_out2) {

	        len = read(rfd2,buf,BUFLEN) ;

	        if (len > 0)
	            writen(fds[2].fd,buf,len) ;

	        else {

	            f_eof4 = TRUE ;

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("main: IN4 EOF\n") ;
#endif
	        }

	        if (! f_final4) {

	            f_in4 = FALSE ;
	            fds[4].events |= pollinput ;

	        }

	        f_out2 = FALSE ;
	        fds[2].events |= polloutput ;
	    }

	    if (f_eof3 && f_eof4) break ;

/* miscellaneous functions */

	    if (g.f.sanity) {

	        time(&g.daytime) ;

	        if (ping(hostname) >= 0) t_sanity = g.daytime ;

	        if (((g.daytime - t_sanity) > g.keeptime) &&
	            (ping(hostname) < 0)) goto badping ;

	    } /* end if (sanity check) */

	} /* end while */

	close(rfd) ;

	close(rfd2) ;

#ifdef	COMMENT
	if (g.f.verbose) bprintf(efp,
	    "%s: remote command completed (rs %d)\n",
	    g.progname,rs) ;
#endif

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: finishing up\n") ;
#endif

/* finish up */
done:
	rs = OK ;
	close(ofd) ;

	logfile_close(&g.lh) ;

/* take the early return here */
earlyret:
	bclose(efp) ;

	return rs ;

/* bad returns come here */
badret:

#if	CF_DEBUG
	bflush(efp) ;
#endif

	close(ofd) ;

	logfile_close(&g.lh) ;

	bclose(efp) ;

	return BAD ;

usage:
	bprintf(efp,
	    "%s: USAGE> rexec [-?VD] hostname\n",
	    g.progname) ;

	bprintf(efp,"\t-l username -p password") ;

	bprintf(efp,"command [arguments ...]\n") ;

	goto badret ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",g.progname) ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument given\n",g.progname) ;

	goto badret ;

badtoomuch:
	bprintf(g.efp,"%s: arguments are too long\n",
	    g.progname) ;

	goto badret ;

badinfile:
	bprintf(efp,"%s: cannot open the input file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badhost:
	bprintf(efp,"%s: bad or no host specification was given\n",
	    g.progname) ;

	goto badret ;

badprog:
	bprintf(efp,"%s: bad or no service specification was given\n",
	    g.progname) ;

	goto badret ;

badinit:
	bprintf(efp,"%s: could not initialize internal structures (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badunreach:
	bprintf(efp,"%s: host is unreachable (maybe doesn't exist)\n",
	    g.progname) ;

	goto badret ;

badrexec:
	logfile_printf(&g.lh,"failed authorization\n") ;

	if (g.f.quiet) goto badret ;

	bprintf(efp,"%s: could not find authorization\n",
	    g.progname) ;

	goto badret ;

hostdown:
	logfile_printf(&g.lh,"host is down\n") ;

	if (! g.f.quiet)
	    bprintf(efp,"%s: host is down (or unreachable)\n",
	        g.progname) ;

	goto badret ;

badpoll:
	bprintf(efp,"%s: bad error return from a poll (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badping:
	time(&g.daytime) ;

	if (((cp = strchr(hostname,'.')) != NULL) &&
	    (strcasecmp(cp + 1,u.domainname) == 0))
	    l = cp - hostname ;

	logfile_printf(&g.lh,
	    "%s host \"%W\" went down\n",
	    timestr_log(g.daytime,buf),
	    hostname,l) ;

	if (! g.f.quiet)
	    bprintf(efp,"%s: %s host \"%W\" went down\n",
	        g.progname,
	        timestr_log(g.daytime,buf),
	        hostname,l) ;

	goto badret ;

}
/* end subroutine (main) */


/* copy machine entries from a NETRC file to a vector element list */
static int copymachines(netrcp,filename,machine,localdomain)
VECITEM		*netrcp ;
char		filename[] ;
char		machine[] ;
char		localdomain[] ;
{
	VECITEM		tmp ;

	struct netrc	*mp ;

	int		rs, i ;
	int		f_euid = FALSE ;

	char		*hnp, hostname[4096 + 1] ;


	if (access(filename,R_OK) < 0) {

	    f_euid = TRUE ;
	    seteuid(g.euid) ;

	}

	if ((rs = netfileopen(&tmp,filename)) >= 0) {

	    if (f_euid) {

	        f_euid = FALSE ;
	        seteuid(g.uid) ;

	    }

	    for (i = 0 ; vecitem_get(&tmp,i,&mp) >= 0 ; i += 1) {

	        if (mp == NULL) continue ;

	        if (mp->machine == NULL) continue ;

/* convert all machine names to canonical form */

	        hnp = mp->machine ;
	        if (getchostname(mp->machine,hostname) >= 0)
	            hnp = hostname ;

/* copy over only those machines that match our target machine */

	        if (! hostequiv(hnp,machine,localdomain)) continue ;

	        if (hnp == hostname) {

	            free(mp->machine) ;

	            mp->machine = malloc_str(hostname) ;

	        }

	        vecitem_add(netrcp,mp,sizeof(struct netrc)) ;

	        mp->machine = NULL ;
	        mp->login = NULL ;
	        mp->password = NULL ;
	        mp->account = NULL ;

	    } /* end for */

	    netfileclose(&tmp) ;

	} /* end if */

	if (f_euid) seteuid(g.uid) ;

	return rs ;
}
/* end subroutine (copymachines) */



