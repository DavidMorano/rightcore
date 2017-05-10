/* main */

/* for LISTENER */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ LISTNER_ADDR=<tliadda>
	$ LISTENER_PROGRAM=<program_to_spawn>
	$ export LISTENER_ADDR LISTENER_PROGRAM
	$ listener


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/utsname.h>
#include	<sys/uio.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<lfm.h>
#include	<sockaddress.h>
#include	<exitcodes.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"connection.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		2	/* number of positional arguments */
#define	MAXARGINDEX	100

#define	NARGPRESENT	(MAXARGINDEX/8 + 1)

#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif

#define	ADDRBUFLEN	(2 * MAXHOSTNAMELEN)

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	USERNAMELEN
#define	USERNAMELEN	32
#endif

#ifndef	JOBIDLEN
#define	JOBIDLEN	32
#endif

#define	TO		10
#define	NIOVECS		10
#define	POLLINT		10

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	listenusd(const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	makedate[] ;


/* local structures */


/* forward references */

static int	helpfile(const char *,bfile *) ;


/* global data */


/* local variables */

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


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	struct msghdr	mh ;
	struct pollfd	fds[3] ;
	struct iovec	vecs[NIOVECS] ;
	struct ustat	sb ;
	PROGINFO	pi, *pip = &pi ;
	USERINFO	u ;
	LFM		lk ;
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		infile, *ifp = &infile ;
	bfile		pidfile ;

	time_t		daytime = time(NULL) ;

	int	argr, argl, aol, avl ;
	int	maxai, pan, npa, kwi, i ;
	int	rs, srs, len, argnum ;
	int	ex = EX_USAGE ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	nfds, nfd, to_poll ;
	int	size, blen, sl, re ;
	int	loglen ;
	int	flen ;
	int	cl ;
	int	fd_debug ;
	int	fd_stdout, fd_listen ;

	cchar	*argp, *aop, *avp ;
	char	argpresent[NARGPRESENT] ;
	char	buf[BUFLEN + 1] ;
	cchar	tmpbuf[MAXPATHLEN + 1] ;
	cchar	tmpfname[MAXPATHLEN + 1] ;
	cchar	userinfobuf[USERINFO_LEN + 1] ;
	cchar	addrbuf[ADDRBUFLEN + 1] ;
	cchar	peername[SOCKADDRESS_NAMELEN + 1] ;
	cchar	jobidbuf[JOBIDLEN + 1] ;
	cchar	timebuf[TIMEBUFLEN] ;
	cchar	*jobid = NULL ;
	cchar	*pidfname = NULL ;
	cchar	*logfname = NULL ;
	cchar	*addrspec = NULL ;
	cchar	*hostname = NULL ;
	cchar	*portspec = NULL ;
	cchar	*progpath = NULL ;
	cchar	*username = NULL ;
	cchar	*cp, *cp1, *cp2 ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	memset(pip,0,sizeof(struct proginfo)) ;

	pip->banner = BANNER ;
	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0) {
	    u_close(2) ;
	    pip->efp = efp ;
	    bcontrol(efp,BC_LINEBUF,0) ;
	}

	pip->programroot = NULL ;
	pip->helpfname = NULL ;

	pip->debuglevel = 0 ;
	pip->verboselevel = 1 ;

	pip->f_exit = FALSE ;

	pip->f.quiet = FALSE ;

/* process program arguments */

	rs = SR_OK ;
	for (i = 0 ; i < NARGPRESENT ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

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

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

	                    aol = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + argl - 1 - avp ;
	                    f_optequal = TRUE ;

	                } else
	                    avl = 0 ;

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

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

	                            if (argl)
	                                pip->programroot = argp ;

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

	                                rs = cfdeci(avp,avl,
	                                    &pip->debuglevel) ;

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

	                        switch ((int) *akp) {

	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                rs = cfdeci(avp,avl, 
	                                    &pip->debuglevel) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'q':
	                            pip->f.quiet = TRUE ;
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

	                        case '?':
	                            f_usage = TRUE ;
				    break ;

	                        default:
				    rs = SR_INVALID ;
	                            bprintf(efp,
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

#ifdef	COMMENT
	    bprintf(efp,"%s: built %s\n",
	        pip->progname,makedate) ;
#endif

	}

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;



/* get our program root */

	if (pip->programroot == NULL)
	    pip->programroot = getenv(VARPROGRAMROOT1) ;

	if (pip->programroot == NULL)
	    pip->programroot = getenv(VARPROGRAMROOT2) ;

	if (pip->programroot == NULL)
	    pip->programroot = getenv(VARPROGRAMROOT3) ;

	if (pip->programroot == NULL)
	    pip->programroot = PROGRAMROOT ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: initial programroot=%s\n",pip->programroot) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: programroot=%s\n",
	        pip->progname,pip->programroot) ;


	if (pip->searchname == NULL)
	    pip->searchname = getenv(VARSEARCHNAME) ;

	if (pip->searchname == NULL)
	    pip->searchname = SEARCHNAME ;


	if (f_help) {

	    if (pip->helpfname == NULL) {

	        blen = bufprintf(buf,BUFLEN,"%s/%s",
	            pip->programroot,HELPFNAME) ;

	        pip->helpfname = (char *) mallocstrw(buf,blen) ;

	    }

	    helpfile(pip->helpfname,pip->efp) ;

	    goto retearly ;

	} /* end if */


/* who are we ? */

	if ((rs = userinfo(&u,userinfobuf,USERINFO_LEN,NULL)) < 0)
	    goto baduser ;

	pip->up = &u ;
	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;

	pip->pid = u.pid ;

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
	                progpath = argv[i] ;
	                break ;

	            } /* end switch */

	            pan += 1 ;

	        } /* end if (got a positional argument) */

	    } /* end for (loading positional arguments) */

	} /* end if (getting arguments) */



	fd_stdout = FD_STDOUT ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: portspec=%s\n", portspec) ;
#endif

/* some initialization */


/* possibly clean up the user specified JOB ID */

	if (jobid != NULL) {

	    cp = jobid ;
	    i = 0 ;
	    while ((*cp != '\0') && (i < LOGFILE_LOGIDLEN)) {

	        if (isalnum(*cp) || (*cp == '_'))
	            jobidbuf[i++] = *cp ;

	        cp += 1 ;

	    } /* end for */

	    jobid = jobidbuf ;

	} else
	    jobid = u.logid ;


/* do we have an activity log file ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 0 logfname=%s\n",logfname) ;
#endif

	rs = BAD ;
	if ((logfname == NULL) || (logfname[0] == '\0'))
	    logfname = getenv(LOGVAR) ;

	if ((logfname == NULL) || (logfname[0] == '\0'))
	    logfname = LOGFNAME ;

	if ((logfname[0] == '/') || (u_access(logfname,W_OK) >= 0))
	    rs = logfile_open(&pip->lh,logfname,0,0666,jobid) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if ((rs < 0) && (logfname[0] != '/')) {

	    mkpath2(tmpfname, pip->programroot,logfname) ;

	    rs = logfile_open(&pip->lh,tmpfname,0,0666,jobid) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 2 logfname=%s rs=%d\n",tmpfname,rs) ;
#endif

	} /* end if (we tried to open another log file) */

	if (rs >= 0) {
	    struct utsname	un ;

	    pip->f.log = TRUE ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we opened a logfile\n") ;
#endif

/* we opened it, maintenance this log file if we have to */

	    if (loglen < 0)
	        loglen = LOGSIZE ;

	    logfile_checksize(&pip->lh,loglen) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we checked its length\n") ;
#endif

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: making log entry\n") ;
#endif

	    logfile_printf(&pip->lh,"%s %s\n",
	        timestr_log(daytime,timebuf),
	        BANNER) ;

	    logfile_printf(&pip->lh,"%-14s %s/%s\n",
	        pip->progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;


	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

	    (void) u_uname(&un) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s)\n",
	        (u.f.sysv_rt ? "SYSV" : "BSD"),
	        un.sysname,un.release) ;

	    logfile_printf(&pip->lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;

	} /* end if (we have a log file or not) */


/* other initialization */

	if (pidfname == NULL)
		pidfname = getenv(PIDVAR) ;

	if (pidfname == NULL)
		pidfname = "/var/run/listener.pid" ;


	if (addrspec == NULL)
		addrspec = getenv(ADDRVAR) ;

	if ((addrspec != NULL) && (addrspec[0] != '\0')) {

	if ((cp = strchr(addrspec,':')) != NULL) {

		strwcpy(addrbuf,addrspec,ADDRBUFLEN) ;

		cl = cp - addrspec ;
		addrbuf[cl] = '\0' ;
		hostname = addrbuf ;
		portspec = addrbuf + cl + 1 ;

	} else
		hostname = addrbuf ;

	}

	if ((hostname == NULL) || (hostname[0] == '\0'))
	    hostname = "rca" ;

	if ((portspec == NULL) || (portspec[0] == '\0'))
	    portspec = "printer" ;


	if (progpath == NULL)
		progpath = getenv(PROGVAR) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main: 1 progpath=%s\n",progpath) ;
#endif

	if (progpath == NULL)
	    progpath = PROG_ECHOD ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main: 2 progpath=%s\n",progpath) ;
#endif


	if (username == NULL)
		username = getenv(USERNAMEVAR) ;

	if (username == NULL)
		username = "daemon" ;


/* can we listen as expected ? */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main: hostname=%s portspec=%s\n",hostname,portspec) ;
#endif

	rs = listentcp(hostname,portspec,0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main: listentcp() rs=%d\n",rs) ;
#endif

	fd_listen = rs ;
	if (rs < 0)
		goto badlisten ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main: fd_listen=%d\n",fd_listen) ;
#endif


/* write our PID out */

	rs = bopen(&pidfile,pidfname,"wct",0644) ;

	if (rs >= 0) {

		bprintf(&pidfile,"%d\n",pip->pid) ;

		bclose(&pidfile) ;

	} /* end if (wriring PID file) */


/* OK, loop waiting */

	rs = watch(pip,fd_listen,progpath,username) ;


	u_close(fd_listen) ;


	if (pidfname != NULL)
		u_unlink(pidfname) ;


#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: done\n") ;
#endif

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;


done:
	u_close(fd_stdout) ;

ret2:
	if (pip->f.log)
	    logfile_close(&pip->lh) ;


/* close off and get out ! */
retearly:
ret1:
	bclose(efp) ;

ret0:

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: ret ex=%d\n",ex) ;
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* what are we about ? */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [progpath] [-V] \n",
	    pip->progname,pip->progname) ;

	goto retearly ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",pip->progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto retearly ;

baduser:
	if (! pip->f.quiet)
	    bprintf(efp,
	        "%s: could not get user information, rs=%d\n",
	        pip->progname,rs) ;

	ex = EX_NOUSER ;
	goto badret ;

badret:
	goto ret2 ;

badlisten:
	ex = EX_DATAERR ;
	goto done ;

}
/* end subroutine (main) */


/* local subroutines */


static int helpfile(f,ofp)
char	f[] ;
bfile	*ofp ;
{
	bfile		file, *ifp = &file ;
	int		rs ;

	if ((f == NULL) || (f[0] == '\0'))
	    return SR_NOENT ;

	if ((rs = bopen(ifp,f,"r",0666)) >= 0) {

	    rs = bcopyblock(ifp,ofp,-1) ;

	    bclose(ifp) ;

	}

	return rs ;
}
/* end subroutine (helpfile) */


