/* main */

/* main subroutine for several programs */


#define	CF_DEBUGS	1
#define	CF_DEBUG	1
#define	CF_SIGNAL	0


/* revision history:

	= 88/02/01, David A­D­ Morano

	This subroutine was originally written.


	= 88/02/01, David A­D­ Morano

	This subroutine was modified to not write out anything
	to standard output if the access time of the associated
	terminal has not been changed in 10 minutes.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	This is a pretty much generic subroutine for several program.


*************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mkdev.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		2	/* number of positional arguments */
#define	MAXARGINDEX	100

#define	NARGPRESENT	(MAXARGINDEX/8 + 1)

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	USERNAMELEN
#define	USERNAMELEN	32
#endif

#ifndef	LINENAMELEN
#define	LINENAMELEN	MAXPATHLEN
#endif

#define	TO		10

#ifndef	LINELEN
#define	LINELEN		100
#endif


/* external subroutines */

extern int	matstr() ;
extern int	cfdeci(char *,int,int *) ;
extern int	daytimer() ;

extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	makedate[] ;


/* local structures */


/* forward references */

static int	anyformat() ;
static int	e1(bfile *,int) ;
static int	e2(bfile *,int) ;
static int	e3(bfile *,int) ;

static void	helpfile(const char *,bfile *) ;
static void	int_all() ;


/* global data */

struct proginfo	g ;


/* local data */

/* define command option words */

static char *argopts[] = {
	        "ROOT",
	        "DEBUG",
	        "VERSION",
	        "VERBOSE",
	        "HELP",
	        "LOG",
	        "MAKEDATE",
	        NULL
} ;

#define	ARGOPT_ROOT		0
#define	ARGOPT_DEBUG		1
#define	ARGOPT_VERSION		2
#define	ARGOPT_VERBOSE		3
#define	ARGOPT_HELP		4
#define	ARGOPT_LOG		5
#define	ARGOPT_MAKEDATE		6


static char	*dialers[] = {
	        "TCP",
	        "TCPMUX",
	        "TCPNLS",
	        NULL
} ;

#define	DIALER_TCP		0
#define	DIALER_TCPMUX		1
#define	DIALER_TCPNLS		2





int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		infile, *ifp = &infile ;

	struct sigaction	sigs ;

	sigset_t	signalmask ;

	struct ustat	sb ;

	struct proginfo	*gp = &g ;

	struct userinfo	u ;

	time_t	daytime ;

	int	argr, argl, aol, avl ;
	int	maxai, pan, npa, kwi, i ;
	int	len ;
	int	argnum ;
	int	dialer ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_anyformat = FALSE ;
	int	f_help ;
	int	l, rs, es ;
	int	fd_debug ;

	char	*argp, *aop, *avp ;
	char	argpresent[NARGPRESENT] ;
	char	buf[BUFLEN + 1] ;
	char	linebuf[LINELEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	char	timebuf[TIMEBUFLEN] ;
	char	*logfname = NULL ;
	char	*mqidspec = NULL ;
	char	*srvspec = "daytime" ;
	char	*cp ;


	if (((cp = getenv(ERRORFDVAR)) != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;

#if	CF_DEBUGS
	debugprintf("main: errfd=%d\n",fd_debug) ;
#endif


	g.banner = BANNER ;
	g.progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0) {

#if	COMMENT
	    u_close(2) ;
#endif

	    bcontrol(efp,BC_LINEBUF,0) ;

	}

	g.efp = efp ;
	g.ofp = ofp ;
	g.debuglevel = 0 ;
	g.programroot = NULL ;
	g.helpfile = NULL ;

	g.f_exit = FALSE ;

	g.f.verbose = FALSE ;
	g.f.quiet = FALSE ;
	g.f.server = FALSE ;

	f_help = FALSE ;


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
	                    goto badargvalue ;

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

/* do we have a keyword match or should we assume only key letters ? */

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

/* debug level */
	                    case ARGOPT_DEBUG:
	                        g.debuglevel = 1 ;
	                        if (f_optequal) {

#if	CF_DEBUGS
	                            debugprintf("main: debug flag, avp=\"%W\"\n",
	                                avp,avl) ;
#endif

	                            f_optequal = FALSE ;
	                            if ((avl > 0) &&
	                                (cfdec(avp,avl,&g.debuglevel) < 0))
	                                goto badargvalue ;

	                        }

	                        break ;

	                    case ARGOPT_VERSION:
	                        f_version = TRUE ;
	                        break ;

	                    case ARGOPT_VERBOSE:
	                        g.f.verbose = TRUE ;
	                        break ;

/* help file */
	                    case ARGOPT_HELP:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.helpfile = avp ;

	                        }

	                        f_help  = TRUE ;
	                        break ;

/* log file */
	                    case ARGOPT_LOG:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) logfname = avp ;

	                        }

	                        break ;

/* display the time this program was last "made" */
	                    case ARGOPT_MAKEDATE:
	                        f_makedate = TRUE ;
	                        break ;

	                    } /* end switch (key words) */

	                } else {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        debugprintf("main: option key letters\n") ;
#endif

	                        switch (*aop) {

	                        case 'D':
	                            g.debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (cfdec(avp,avl, &g.debuglevel) !=
	                                    OK)
	                                    goto badargvalue ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'd':
					gp->f.server = TRUE ;
					break ;

/* message ID */
				case 'm':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                mqidspec = argp ;

	                            break ;

	                        case 'q':
	                            g.f.quiet = TRUE ;
	                            break ;

	                        case 's':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                srvspec = argp ;

	                            break ;

	                        case 'v':
	                            g.f.verbose = TRUE ;
	                            break ;

	                        case 'x':
	                            f_anyformat = TRUE ;
	                            break ;

	                        default:
	                            bprintf(efp,"%s : unknown option - %c\n",
	                                g.progname,*aop) ;

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
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    g.progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


	if (g.debuglevel > 0)
	    bprintf(efp,"%s: finished parsing arguments\n",
	        g.progname) ;


/* continue w/ the trivia argument processing stuff */

	if (f_version)
	    bprintf(efp,"%s: version %s\n",
	        g.progname,VERSION) ;

	if (f_makedate)
	    bprintf(efp,"%s: built %s\n",
	        g.progname,makedate) ;

	if (f_usage) goto usage ;

	if (f_version + f_makedate) 
		goto exit ;


	if (f_help) {

	    if (g.helpfile == NULL) {

	        l = bufprintf(buf,BUFLEN,"%s/%s",
	            g.programroot,HELPFNAME) ;

	        g.helpfile = (char *) mallocstrw(buf,l) ;

	    }

	    helpfile(g.helpfile,g.efp) ;

	    goto exit ;

	} /* end if */


	if (g.debuglevel > 0)
	    bprintf(efp,"%s: debuglevel %d\n",
	        g.progname,g.debuglevel) ;


/* get our program root (if we have one) */

	if (g.programroot == NULL)
	    g.programroot = getenv(VARPROGRAMROOT1) ;

	if (g.programroot == NULL)
	    g.programroot = getenv(VARPROGRAMROOT2) ;

	if (g.programroot == NULL)
	    g.programroot = getenv(VARPROGRAMROOT3) ;

	if (g.programroot == NULL)
	    g.programroot = PROGRAMROOT ;


	if (g.debuglevel > 0)
	    bprintf(efp,"%s: programroot=%s\n",
	        g.progname,g.programroot) ;



/* initial check arguments that we have so far */



/* who are we ? */

	if ((rs = userinfo(&u,userinfobuf,USERINFO_LEN,NULL)) < 0)
	    goto baduser ;

	g.pid = u.pid ;

	(void) time(&daytime) ;

/* do we have a log file ? */

#ifdef	COMMENT
	if (logfname == NULL)
	    logfname = LOGFNAME ;

	if (logfname[0] != '/') {

	    len = bufprintf(buf,BUFLEN,"%s/%s",g.programroot,logfname) ;

	    logfname = mallocstrw(buf,len) ;

	}

/* make a log entry */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: logfile=%s\n",
	        logfname) ;
#endif

	if ((rs = logfile_open(&g.lh,logfname,0,0666,u.logid)) >= 0) {

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
	    if (g.debuglevel > 2)
	        debugprintf("main: about to do 'logfile_printf'\n") ;
#endif

	    logfile_printf(&g.lh,"%s %-14s %s/%s\n",
	        timestr_log(daytime,timebuf),
	        g.progname,
	        VERSION,(u.f.sysv_ct ? "SYSV" : "BSD")) ;

	    logfile_printf(&g.lh,"os=%s %s!%s %s\n",
	        (u.f.sysv_rt ? "SYSV" : "BSD"),u.nodename,u.username,buf) ;

	} else {

	    if (g.f.verbose || (g.debuglevel > 0)) {

	        bprintf(g.efp,
	            "%s: logfile=%s\n",
	            g.progname,logfname) ;

	        bprintf(g.efp,
	            "%s: could not open the log file (rs %d)\n",
	            g.progname,rs) ;

	    }

	} /* end if (opened a log) */

#endif /* COMMENT */


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: checking for positional arguments\n") ;
#endif

	    if (npa > 0) {

	        pan = 0 ;
	        for (i = 0 ; i <= maxai ; i += 1) {

	            if (BATST(argpresent,i)) {

			switch (pan) {

			case 0:

#if	CF_DEBUGS
		debugprintf("main: parameter MQID=%s\n",argv[pan]) ;
#endif

				mqidspec = argv[i] ;
				break ;

			} /* end switch */

	                pan += 1 ;

	            } /* end if */

	        } /* end for */

	    } /* end if (we have positional arguments) */



/* initial check arguments that we have so far */



/* do the main thing */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: performing main processing\n") ;
#endif

	g.f_signal = FALSE ;
	g.signal_num = -1 ;

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


	bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	bopen(ofp,BFILE_STDIN,"r",0666) ;


	bprintf(ofp,"enter numbers\n") ;

	len = breadline(ofp,linebuf,LINELEN) ;

	if (len > 0) {

		int	sl, cl ;
		int	e[3] ;

		char	*sp ;


	linebuf[len] = '\0' ;

	sp = linebuf ;
	sl = len ;
	for (i = 0 ; i < 3 ; i += 1) {

		cl = nextfield(sp,sl,&cp) ;

		cfdeci(cp,cl,&e[i]) ;

		sp = cp + cl + 1 ;
		sl = linebuf + len - sp ;

	} /* end for */


		if (e1(ofp,e[0]) || e2(ofp,e[1]) && e3(ofp,e[2]))
			bprintf(ofp,"condition was true\n") ;

		else
			bprintf(ofp,"condition was false\n") ;

	}

	bclose(ofp) ;

	bclose(ofp) ;



/* close off and get out ! */
exit:
	bclose(efp) ;

	return rs ;

/* what are we about ? */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [host [...]] [-d dialer_specification]\n",
	    g.progname,g.progname) ;

	bprintf(efp,
	    "\t[-s service] [-x] [-v]\n",
	    g.progname,g.progname) ;

	es = -1 ;
	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",g.progname) ;

	es = -2 ;
	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	es = -3 ;
	goto badret ;

baddialer:
	bprintf(efp,
	    "%s: unknown dialer specification given\n",
	    g.progname) ;

	es = -4 ;
	goto badret ;

badsrv:
	bprintf(efp,
	    "%s: unknown service specification given\n",
	    g.progname) ;

	es = -4 ;
	goto badret ;

baduser:
	if (! g.f.quiet)
	    bprintf(efp,
	        "%s: could not get user information, rs=%d\n",
	        g.progname,rs) ;

	es = -5 ;
	goto badret ;


badret:
	bclose(efp) ;

	return es ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static void helpfile(f,ofp)
char	f[] ;
bfile	*ofp ;
{
	bfile	file, *ifp = &file ;


	if ((f == NULL) || (f[0] == '\0'))
	    return ;

	if (bopen(ifp,f,"r",0666) >= 0) {

	    bcopyblock(ifp,ofp,-1) ;

	    bclose(ifp) ;

	}

}
/* end subroutine (helpfile) */


void int_all(signum)
int	signum ;
{


	g.f_signal = TRUE ;
	g.signal_num = signum ;

}


static int anyformat(ofp,s)
bfile	*ofp ;
int	s ;
{
	int	rlen, tlen = 0 ;

	char	buf[BUFLEN + 1] ;


	while ((rlen = uc_readlinetimed(s,buf,BUFLEN,TO)) > 0) {

	    bwrite(ofp,buf,rlen) ;

	    tlen += rlen ;
	}

	return ((rlen < 0) ? rlen : tlen) ;
}
/* end subroutine (anyformat) */


static int e1(ofp,n)
bfile	*ofp ;
int	n ;
{

	bprintf(ofp,"e1 has %d\n",n) ;

	return n ;
}

static int e2(ofp,n)
bfile	*ofp ;
int	n ;
{

	bprintf(ofp,"e2 has %d\n",n) ;

	return n ;
}

static int e3(ofp,n)
bfile	*ofp ;
int	n ;
{

	bprintf(ofp,"e3 has %d\n",n) ;

	return n ;
}



