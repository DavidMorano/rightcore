/* main */

/* testing */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1
#define	CF_DIALTCPMUX	0
#define	CF_SLEEP	0


/* revision history:

	= 88/02/01, David A­D­ Morano

	This subroutine was originally written.


	= 88/02/10, David A­D­ Morano

	This subroutine was modified to not write out anything
	to standard output if the access time of the associated
	terminal has not been changed in 10 minutes.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	Call as :

	$ testdialtcp


*************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<libgen.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<lfm.h>
#include	<bfile.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		2	/* number of positional arguments */
#define	MAXARGINDEX	100

#define	NARGPRESENT	(MAXARGINDEX/8 + 1)
#define	BUFLEN		MAXPATHLEN

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
extern int	cfdeci(const char *,int,int *) ;
extern int	dialudp(char *,char *,int,int,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	dialtcpnls(char *,char *,int,char *,int,int) ;
extern int	dialtcpmux(char *,char *,int,char *,char **,int,int) ;

extern char	*strbasename() ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	makedate[] ;


/* local structures */


/* forward references */

static int	helpfile(const char *,bfile *) ;


/* global data */


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
	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp ;

	struct ustat	sb ;

	LFM		lk ;

	struct userinfo	u ;

	struct proginfo	pi, *pip = &pi ;

	time_t	daytime ;

	int	argr, argl, aol, avl ;
	int	maxai, pan, npa, kwi, i ;
	int	len ;
	int	argnum ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_help ;
	int	l, rs, srs ;
	int	fd_debug ;

	char	*argp, *aop, *avp ;
	char	argpresent[NARGPRESENT] ;
	char	buf[BUFLEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	char	timebuf[TIMEBUFLEN] ;
	char	*logfname = NULL ;
	char	*hostname = NULL ;
	char	*portspec = NULL ;
	char	*cp ;



	if (((cp = getenv(VARDEBUGFD)) != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;

#if	CF_DEBUGS
	debugprintf("main: fd_debug=%d\n",fd_debug) ;
#endif


	pip->banner = BANNER ;
	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0) {

	    u_close(2) ;

	    bcontrol(efp,BC_LINEBUF,0) ;

	}

	pip->efp = efp ;
	pip->debuglevel = 0 ;
	pip->programroot = NULL ;
	pip->helpfile = NULL ;

	pip->f_exit = FALSE ;

	pip->f.verbose = FALSE ;
	pip->f.quiet = FALSE ;

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
	                            if (avl) 
					pip->programroot = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pip->programroot = argp ;

	                        }

	                        break ;

/* debug level */
	                    case ARGOPT_DEBUG:
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

#if	CF_DEBUGS
	                            debugprintf(
	                                "main: debug flag, avp=\"%W\"\n",
	                                avp,avl) ;
#endif

	                            f_optequal = FALSE ;
	                            if (avl) {

	                                rs = cfdec(avp,avl,
	                                	&pip->debuglevel) ;

					if (rs < 0)
	                                goto badargvalue ;

				    }
	                        }

	                        break ;

	                    case ARGOPT_VERSION:
	                        f_version = TRUE ;
	                        break ;

	                    case ARGOPT_VERBOSE:
	                        pip->f.verbose = TRUE ;
	                        break ;

/* help file */
	                    case ARGOPT_HELP:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->helpfile = avp ;

	                        }

	                        f_help  = TRUE ;
	                        break ;

/* log file */
	                    case ARGOPT_LOG:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					logfname = avp ;

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
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                rs = cfdec(avp,avl, &pip->debuglevel) ;

					if (rs < 0)
	                                    goto badargvalue ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

	                        case 'v':
	                            pip->f.verbose = TRUE ;
	                            break ;

	                        default:
	                            bprintf(efp,"%s : unknown option - %c\n",
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

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	    bprintf(efp,"%s: built %s\n",
	        pip->progname,makedate) ;

	}

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto exit ;


	if (f_help) {

	    if (pip->helpfile == NULL) {

	        l = bufprintf(buf,BUFLEN,"%s/%s",
	            pip->programroot,HELPFNAME) ;

	        pip->helpfile = (char *) mallocstrw(buf,l) ;

	    }

	    helpfile(pip->helpfile,pip->efp) ;

	    goto exit ;

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
	    pip->programroot = PROGRAMROOT ;


	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: programroot=%s\n",
	        pip->progname,pip->programroot) ;

/* who are we? */

	if ((rs = userinfo(&u,userinfobuf,USERINFO_LEN,NULL)) < 0)
	    goto baduser ;

	pip->pid = u.pid ;

	(void) time(&daytime) ;

/* do we have a log file ? */

	if (logfname == NULL)
	    logfname = LOGFNAME ;

	if (logfname[0] != '/') {

	    len = bufprintf(buf,BUFLEN,"%s/%s",pip->programroot,logfname) ;

	    logfname = mallocstrw(buf,len) ;

	}

/* make a log entry */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: logfile=%s\n",
	        logfname) ;
#endif

	if ((rs = logfile_open(&pip->lh,logfname,0,0666,u.logid)) >= 0) {

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

	    logfile_printf(&pip->lh,"os=%s %s!%s %s\n",
	        (u.f.sysv_rt ? "SYSV" : "BSD"),u.nodename,u.username,buf) ;

	} else {

	    if (pip->f.verbose || (pip->debuglevel > 0)) {

	        bprintf(pip->efp,
	            "%s: logfile=%s\n",
	            pip->progname,logfname) ;

	        bprintf(pip->efp,
	            "%s: could not open the log file (rs %d)\n",
	            pip->progname,rs) ;

	    }

	} /* end if (opened a log) */


/* do the main thing */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if (BATST(argpresent,i)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf(
	                    "main: got positional arg i=%d pan=%d arg=%s\n",
	                    i,pan,argv[i]) ;
#endif

			switch (pan) {

			case 0:
				hostname = argv[i] ;
				break ;

			case 1:
				portspec = argv[i] ;

			} /* end switch */

	            pan += 1 ;

	        } /* end if (got a positional argument) */

	    } /* end for (loading positional arguments) */

	} /* end if (getting arguments) */




	ofp = NULL ;
	if ((bopen(&outfile,BFILE_STDOUT,"wct",0666)) >= 0) {

		ofp = &outfile ;
		bcontrol(&outfile,BC_UNBUF,0) ;

	}

	bopen(&infile,BFILE_STDIN,"r",0666) ;


	if ((hostname == NULL) || (hostname[0] == '\0'))
		hostname = "rca" ;

	if ((portspec == NULL) || (portspec[0] == '\0'))
		portspec = "echo" ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: hostname=%s portspec=%s\n",
		hostname,portspec) ;
#endif

#if	CF_DIALTCPMUX
		rs = dialtcpmux(hostname,portspec,AF_INET,"echo",NULL,TO,0) ;
#else
		rs = dialtcp(hostname,portspec,AF_INET,TO,0) ;
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: dial rs=%d\n",rs) ;
#endif

		if (rs >= 0) {

	                int	s ;


	                s = rs ;


			while ((len = breadline(ifp,buf,BUFLEN)) > 0) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
			debugprintf("main: writing> %W",buf,len) ;
#endif

				rs = uc_writen(s,buf,len) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
			debugprintf("main: rs=%d wrote> %W",rs,buf,len) ;
#endif

#if	CF_SLEEP
			sleep(1) ;
#endif

	                len = uc_readlinetimed(s,buf,BUFLEN,TO) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
			debugprintf("main: len=%d read> %W",
				len,buf,((len > 0) ? len : 0)) ;
#endif

	                if ((len > 0) && (ofp != NULL))
				bwrite(ofp,buf,len) ;

			} /* end while */


	                u_close(s) ;

	            } else {

			if (ofp != NULL)
	                bprintf(ofp,"** no connect (%d) ** ^%s\n",
				rs,hostname) ;

			}



	bclose(&infile) ;

	if (ofp != NULL)
	    bclose(ofp) ;


	u_close(FD_STDOUT) ;


/* close off and get out ! */
exit:
	bclose(efp) ;

	return rs ;

/* what are we about ? */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [mailfile [offset]] [-sV] [-t timeout] [-offset]\n",
	    pip->progname,pip->progname) ;

	bprintf(efp,
	    "\t[-d terminal_device]\n",
	    pip->progname,pip->progname) ;

	srs = -1 ;
	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",pip->progname) ;

	srs = -2 ;
	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	srs = -3 ;
	goto badret ;

baduser:
	if (! pip->f.quiet)
	    bprintf(efp,
	        "%s: could not get user information, rs=%d\n",
	        pip->progname,rs) ;

	srs = -5 ;
	goto badret ;


badret:
	bclose(efp) ;

	return srs ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int helpfile(f,ofp)
const char	f[] ;
bfile		*ofp ;
{
	bfile	file, *ifp = &file ;

	int	rs ;


	if ((f == NULL) || (f[0] == '\0')) 
		return ;

	if ((rs = bopen(ifp,f,"r",0666)) >= 0) {

	    rs = bcopyblock(ifp,ofp,-1) ;

	    bclose(ifp) ;

	}

	return rs ;
}
/* end subroutine (helpfile) */



