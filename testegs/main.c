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



/************************************************************************

	This is a pretty much generic subroutine for several program.


*************************************************************************/




#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mkdev.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
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
#include	<exitcodes.h>
#include	<mallocstuff.h>

#include	"localmisc.h"
#include	"lfm.h"
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

extern int	cfdeci(const char *,int,int *) ;
extern int	matstr() ;
extern int	dialudp(char *,char *,int,int,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	dialtcpnls(char *,char *,int,char *,int,int) ;
extern int	dialtcpmux(char *,char *,int,char *,char **,int,int) ;

extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	makedate[] ;


/* local structures */


/* forward references */

static int	anyformat() ;

static void	helpfile(const char *,bfile *) ;
static void	int_all() ;


/* global data */

static int	f_signal = FALSE ;
static int	signal_num = -1 ;


/* local data */

/* define command option words */

static char *const argopts[] = {
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

static char	*const dialers[] = {
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
	struct proginfo	g, *pip = &g ;
	struct sigaction	sigs ;
	struct ustat	sb ;

	USERINFO	u ;

	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		pidfile ;

	sigset_t	signalmask ;

	time_t	daytime = time(NULL) ;

	int	argr, argl, aol, avl ;
	int	maxai, pan, npa, kwi, i ;
	int	ex = EX_INFO ;
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
	int	l, rs ;
	int	fd_debug ;

	char	*argp, *aop, *avp ;
	char	argpresent[NARGPRESENT] ;
	char	buf[BUFLEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	char	timebuf[TIMEBUFLEN] ;
	char	*logfname = NULL ;
	char	*pathname = NULL ;
	char	*srvspec = "daytime" ;
	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
		cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;

#if	CF_DEBUGS
	debugprintf("main: fd_debug=%d\n",fd_debug) ;
#endif


	pip->banner = BANNER ;
	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

	pip->programroot = NULL ;
	pip->helpfile = NULL ;
	pip->efp = efp ;
	pip->ofp = ofp ;
	pip->verboselevel = 1 ;

	pip->f_exit = FALSE ;
	pip->f.quiet = FALSE ;
	pip->f.server = FALSE ;

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
	                    case argopt_root:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->programroot = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) pip->programroot = argp ;

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
	                            if (avl) {

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
					pip->helpfile = avp ;

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

#if	CF_DEBUGS
	                    debugprintf("main: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        debugprintf("main: option key letters\n") ;
#endif

	                        switch (*aop) {

	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                rs = cfdeci(avp,avl, &pip->debuglevel) ;

					if (rs < 0)
	                                    goto badargval ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'd':
					pip->f.server = TRUE ;
					break ;

	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

	                        case 'p':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pathname = argp ;

	                            break ;

	                        case 's':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                srvspec = argp ;

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

	                        case '?':
	                            f_usage = TRUE ;
					break ;

	                        default:
	                            f_usage = TRUE ;
					ex = EX_USAGE ;
	                            bprintf(efp,"%s : unknown option - %c\n",
	                                pip->progname,*aop) ;

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
			f_usage = TRUE ;
				ex = EX_USAGE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: finished parsing arguments\n",
	        pip->progname) ;


/* continue w/ the trivia argument processing stuff */

	if (f_version)
	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_makedate)
	    bprintf(efp,"%s: built %s\n",
	        pip->progname,makedate) ;

	if (f_usage) 
		goto usage ;

	if (f_version + f_makedate) 
		goto earlyret ;


	if (f_help) {

	    if (pip->helpfile == NULL) {

	        l = bufprintf(buf,BUFLEN,"%s/%s",
	            pip->programroot,HELPFNAME) ;

	        pip->helpfile = (char *) mallocstrw(buf,l) ;

	    }

	    helpfile(pip->helpfile,pip->efp) ;

	    goto earlyret ;

	} /* end if */


	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: debuglevel %d\n",
	        pip->progname,pip->debuglevel) ;


/* get our program root (if we have one) */

	if (pip->programroot == NULL)
	    pip->programroot = getenv(VARPROGRAMROOT1) ;

	if (pip->programroot == NULL)
	    pip->programroot = getenv(VARPROGRAMROOT2) ;

	if (pip->programroot == NULL)
	    pip->programroot = getenv(VARPROGRAMROOT3) ;

	if (pip->programroot == NULL)
	    pip->programroot = PROGRAMROOT ;


	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: programroot=%s\n",
	        pip->progname,pip->programroot) ;

/* initial check arguments that we have so far */

/* who are we? */

	if ((rs = userinfo(&u,userinfobuf,USERINFO_LEN,NULL)) < 0)
	    goto baduser ;

	pip->pid = u.pid ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
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

				pathname = argv[i] ;
				break ;

			} /* end switch */

	                pan += 1 ;

	            } /* end if */

	        } /* end for */

	    } /* end if (we have positional arguments) */



	ex = EX_DATAERR ;


/* do the main thing */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: performing main processing\n") ;
#endif

	f_signal = FALSE ;
	signal_num = -1 ;

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

	if ((rs = bopen(ofp,BFILE_STDOUT,"dwct",0666)) >= 0) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("main: process() pathname=%s\n",pathname) ;
#endif

	rs = process(pip,pathname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("main: process() rs=%d\n",rs) ;
#endif



	bclose(ofp) ;

	} /* end block */



/* close off and get out ! */
ret2:
exit:
earlyret:
ret1:
	bclose(efp) ;

ret0:
	return ex ;

/* what are we about ? */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [host [...]] [-d dialer_specification]\n",
	    pip->progname,pip->progname) ;

	bprintf(efp,
	    "\t[-s service] [-x] [-v]\n",
	    pip->progname,pip->progname) ;

	goto earlyret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",pip->progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

baddialer:
	bprintf(efp,
	    "%s: unknown dialer specification given\n",
	    pip->progname) ;

	goto badarg ;

badsrv:
	bprintf(efp,
	    "%s: unknown service specification given\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto earlyret ;

baduser:
	if (! pip->f.quiet)
	    bprintf(efp,
	        "%s: could not get user information, rs=%d\n",
	        pip->progname,rs) ;

	ex = EX_NOUSER ;
	goto badret ;


badret:
	goto ret2 ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static void helpfile(f,ofp)
const char	f[] ;
bfile		*ofp ;
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


	f_signal = TRUE ;
	signal_num = signum ;

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



