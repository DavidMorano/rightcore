/* main */

/* main subroutine for several programs */


#define	CF_DEBUGS	1
#define	CF_DEBUG	1


/* revision history:

	= 88/02/01, David A­D­ Morano

	This subroutine was originally written.


	= 88/02/01, David A­D­ Morano

	This subroutine was modified to not write out anything
	to standard output if the access time of the associated
	terminal has not been changed in 10 minutes.


*/


/************************************************************************

	This is a pretty much generic subroutine for several program.


*************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<time.h>
#include	<string.h>
#include	<ctype.h>
#include	<libgen.h>
#include	<signal.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<mallocstuff.h>

#include	"localmisc.h"
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



/* external subroutines */

extern int	matstr() ;
extern int	cfnumi(char *,int,int *) ;

extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	makedate[] ;


/* local structures */


/* forward references */

static int	anyformat() ;

static void	helpfile(const char *,bfile *) ;
static void	int_all() ;


/* local variables */

static const char *argopts[] = {
	        "ROOT",
	        "DEBUG",
	        "VERSION",
	        "VERBOSE",
	        "HELP",
	        "LOG",
	        "MAKEDATE",
	        NULL,
} ;

#define	ARGOPT_ROOT		0
#define	ARGOPT_DEBUG		1
#define	ARGOPT_VERSION		2
#define	ARGOPT_VERBOSE		3
#define	ARGOPT_HELP		4
#define	ARGOPT_LOG		5
#define	ARGOPT_MAKEDATE		6







int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	struct sigaction	sigs ;

	sigset_t	signalmask ;

	struct ustat	sb ;

	struct proginfo	g, *gp = &g ;

	struct userinfo	u ;

	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		pidfile ;

	time_t	daytime ;

	int	argr, argl, aol, avl ;
	int	maxai, pan, npa, kwi, i ;
	int	es = ES_INFO ;
	int	len, sl ;
	int	argnum ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_help ;
	int	rs ;
	int	err_fd ;

	char	*argp, *aop, *avp ;
	char	argpresent[NARGPRESENT] ;
	char	buf[BUFLEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	char	timebuf[TIMEBUFLEN] ;
	char	*logfname = NULL ;
	char	*mqidspec = NULL ;
	char	*srvspec = "daytime" ;
	char	*cp ;


	if (((cp = getenv(ERRORFDVAR)) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;

#if	CF_DEBUGS
	debugprintf("main: errfd=%d\n",err_fd) ;
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
	g.debuglevel = 0 ;
	g.programroot = NULL ;
	g.helpfile = NULL ;

	g.f.verbose = FALSE ;
	g.f.quiet = FALSE ;

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

	                        case 'q':
	                            g.f.quiet = TRUE ;
	                            break ;

	                        case 'v':
	                            g.f.verbose = TRUE ;
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


#if	CF_DEBUGS
	debugprintf("main: finished w/ arguments\n") ;
#endif

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

	es = ES_BADARG ;
	if (f_usage) 
		goto usage ;

	if (f_version + f_makedate) 
		goto exit ;


	if (f_help) {

	    if (g.helpfile == NULL) {

	        sl = bufprintf(buf,BUFLEN,"%s/%s",
	            g.programroot,HELPFNAME) ;

	        g.helpfile = (char *) mallocstrw(buf,sl) ;

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

/* do we have a log file? */

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



/* initial check arguments that we have so far */



#if	CF_DEBUGS
	debugprintf("main: main thing\n") ;
#endif


/* do the main thing */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: performing main processing\n") ;
#endif


	bopen(ofp,BFILE_STDOUT,"dwct",0666) ;


/* OK, we do it */

	pan = 0 ;
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) &&
	            (argv[i][0] != '\0')) continue ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: calling process name=\"%s\"\n",argv[i]) ;
#endif

	        if ((rs = process(&g,ofp,argv[i])) < 0)
	            break ;

	        pan += 1 ;

	    } /* end for (looping through requested circuits) */

	} else {

	        rs = process(&g,ofp,"-") ;

	        pan += 1 ;

	} /* end if */


	bclose(ofp) ;


#if	CF_DEBUGS
	debugprintf("main: done\n") ;
#endif


	es = ES_OK ;


/* close off and get out ! */
exit:
earlyret:
	bclose(efp) ;

	return es ;

/* what are we about ? */
usage:
	bprintf(efp,
	    "%s: USAGE> %s infile \n",
	    g.progname,g.progname) ;

	es = ES_INFO ;
	goto earlyret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

baduser:
	if (! g.f.quiet)
	    bprintf(efp,
	        "%s: could not get user information, rs=%d\n",
	        g.progname,rs) ;

	goto badret ;


badret:
	es = ES_BADARG ;
	bclose(efp) ;

	goto earlyret ;
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



